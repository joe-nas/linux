/*
* cxd2880-spi.c
* Sony CXD2880 DVB-T2/T tuner + demodulator driver
*
* Copyright (C) 2016 Sony Semiconductor Solutions Corporation
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; version 2 of the License.
*
* THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
* NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
* USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/spi/spi.h>
#include <linux/types.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/mutex.h>
#include <linux/ktime.h>
#include <linux/time.h>
#include <linux/timekeeping.h>

#include <dvb_demux.h>
#include <dmxdev.h>
#include <dvb_frontend.h>
#include <linux/spi/spi.h>
#include <cxd2880.h>

#define CXD2880_MAX_FILTER_SIZE 32
#define  BURST_WRITE_MAX 128

static LIST_HEAD(pidlist_head);

struct cxd2880_ts_buf_info {
	u8 readReady;
	u8 almostFull;
	u8 almostEmpty;
	u8 overflow;
	u8 underflow;
	u16 packetNum;
};

struct cxd2880_tuner_demod_pid_config {
	u8 isEnable;
	u16 pid;
};

struct cxd2880_tuner_demod_pid_filter_config {
	u8 isNegative;
	struct cxd2880_tuner_demod_pid_config pidConfig[32];
};

struct cxd2880_pid_filter {
	struct cxd2880_tuner_demod_pid_config config;
	struct list_head pidlist;
};

struct cxd2880_dvb {
	struct dvb_frontend fe;
	struct dvb_adapter adapter;
	struct dvb_demux demux;
	struct dmxdev dmxdev;
	struct dmx_frontend fe_hw;
	struct task_struct* cxd2880_ts_read_thread;
	struct spi_device *spi;
	spinlock_t spi_lock;
	struct mutex spi_mutex;
	int feed_count;
	spinlock_t feed_lock;
	u8* ts_buf;
};

static struct cxd2880_dvb *dvb = 0;


DVB_DEFINE_MOD_OPT_ADAPTER_NR(adapter_nr);

static int cxd2880_write_spi(u8* data, u32 size)
{
	struct spi_message msg;
	struct spi_transfer tx;
	int result = 0;

	memset(&tx, 0, sizeof(tx));
	tx.tx_buf = data;
	tx.len = size;

	spi_message_init(&msg);
	spi_message_add_tail(&tx, &msg);
	result = spi_sync(dvb->spi, &msg);

	return result;
}

static int cxd2880_write_reg (u8 subAddress, const u8 * pData, u32 size)
{
	u8 sendData[BURST_WRITE_MAX + 4];
	const u8* pWriteDataTop = pData;
	int result = 0;

	if (!pData) {
		return -EINVAL;
	}
	if (size > BURST_WRITE_MAX) {
		return -EINVAL;
	}

	if (subAddress + size > 0x100) {
	return -EINVAL;
	}

	sendData[0] = 0x0E; 

	while (size > 0) {
		sendData[1] = subAddress;
		if (size > 255) {
			sendData[2] = 255;
 		} else {
			sendData[2] = (uint8_t)size;
		}

		memcpy (&sendData[3], pWriteDataTop, sendData[2]);

		result = cxd2880_write_spi(sendData, sendData[2] + 3);
		if (result != 0)
			break;
		subAddress += sendData[2];
		pWriteDataTop += sendData[2];
		size -= sendData[2];
	}

	return result;
}

static int cxd2880_devio_spi_ReadTS (
								uint8_t * pData,
								uint32_t packetNum)
{
	u8 data[3];

	if (!pData || packetNum == 0) {
		return -EINVAL;
	}
	if (packetNum > 0xFFFF) {
		return -EINVAL;
	}

	if (dvb->spi == NULL) {
		printk("spi = NULL\n");
		return -EINVAL;
	}
	data[0] = 0x10;
	data[1] = packetNum >> 8 & 0xFf;
	data[2] = packetNum & 0xFf;

	spi_write_then_read(dvb->spi, data, sizeof(data), pData, packetNum * 188);
	return 0;
}

static int cxd2880_devio_spi_ReadTSBufferInfo (
						struct cxd2880_ts_buf_info * pInfo)
{
	u8 sendData = 0x20;
	u8 recvData[2];

	if (!pInfo) {
		return -EINVAL;
	}

	spi_write_then_read(dvb->spi, &sendData, 1, recvData, sizeof(recvData));

	pInfo->readReady = (u8)((recvData[0] & 0x80) ? 1 : 0);
	pInfo->almostFull = (u8)((recvData[0] & 0x40) ? 1 : 0);
	pInfo->almostEmpty = (u8)((recvData[0] & 0x20) ? 1 : 0);
	pInfo->overflow = (u8)((recvData[0] & 0x10) ? 1 : 0);
	pInfo->underflow = (u8)((recvData[0] & 0x08) ? 1 : 0);
	pInfo->packetNum = (u16)(((recvData[0] & 0x07) << 8) | recvData[1]);
	return 0;
}

static int cxd2880_devio_spi_ClearTSBuffer (void)
{
	return 0;
}

static int cxd2880_SetPIDFilter(struct cxd2880_tuner_demod_pid_filter_config
						     *pPIDFilterConfig)
{
	u8 data[65];
	data[0] = 0x00;
	if (cxd2880_write_reg (0x00, &data[0], 1) != 0)
		return -1;

	if (!pPIDFilterConfig) {
		data[0] = 0x02;
		if (cxd2880_write_reg (0x50, &data[0], 1) != 0)
			return -1;
	} else {
		data[0] = (u8) (pPIDFilterConfig->isNegative ? 0x01 : 0x00);
		{
			int i = 0;

			for (i = 0; i < 32; i++) {
				if (pPIDFilterConfig->pidConfig[i].isEnable) {
					data[1 + (i * 2)] =
					    (u8) ((u8)
						  (pPIDFilterConfig->
						   pidConfig[i].
						   pid >> 8) | 0x20);
					data[2 + (i * 2)] =
					    (u8) (pPIDFilterConfig->
						  pidConfig[i].pid & 0xFF);
				} else {
					data[1 + (i * 2)] = 0x00;
					data[2 + (i * 2)] = 0x00;
				}
			}
		}
		if (cxd2880_write_reg (0x50, data, 65) != 0)
			return -1;
	}

	return 0;
}

static int cxd2880_ts_read(void* arg)
{
	struct cxd2880_ts_buf_info info;
	struct timespec ts;
	u32 elapsed = 0;
	u32 starttime = 0;

	starttime = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
	while (!kthread_should_stop()) {
		getnstimeofday(&ts);
		elapsed = ((ts.tv_sec * 1000) + (ts.tv_nsec / 1000000)) - starttime;
		cxd2880_devio_spi_ReadTSBufferInfo(&info);
		if (info.packetNum != 0) {
			if (info.packetNum > 300 ||
				(info.packetNum <= 300 && elapsed >= 500)) {
				printk("packet num %d elapsed %d", info.packetNum, elapsed);
				printk("send \n");
				cxd2880_devio_spi_ReadTS(dvb->ts_buf, info.packetNum);
				dvb_dmx_swfilter(&dvb->demux,
								dvb->ts_buf,
								188*info.packetNum);
				starttime = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
			}
		}
	}

	return 0;
}

static int cxd2880_dvb_bus_ctrl(struct dvb_frontend *fe, int acquire)
{
	printk("cxd2880_dvb_bus_ctrl\n");

	return 0;
}

static int cxd2880_start_feed(struct dvb_demux_feed *feed)
{
	int result = 0;
	int i;
	struct cxd2880_pid_filter* filter;
	struct list_head* list;
	struct cxd2880_tuner_demod_pid_filter_config filter_config;

	mutex_lock(&dvb->spi_mutex);

	dvb->feed_count++;
	memset(&filter_config,
			0,
			 sizeof(struct cxd2880_tuner_demod_pid_filter_config));

	printk("feed filtering %d %d\n", feed->pid, feed->feed.ts.is_filtering);

	filter = kmalloc(sizeof(struct cxd2880_pid_filter), GFP_KERNEL);
	if (filter != NULL) {
		filter->config.isEnable = 1;
		filter->config.pid = feed->pid;
		list_add_tail(&filter->pidlist, &pidlist_head);
	}
	i = 0;
	list_for_each(list, &pidlist_head) {
		filter = list_entry(list, struct cxd2880_pid_filter, pidlist);
		filter_config.pidConfig[i].isEnable = filter->config.isEnable;
		filter_config.pidConfig[i].pid = filter->config.pid;
		printk("set pid %d enable %d\n", filter_config.pidConfig[i].pid,
									filter_config.pidConfig[i].isEnable);
		i++;
		if (i > 32)
			printk("pid filter over flow\n");
	}
	filter_config.isNegative = 0;

	result = cxd2880_SetPIDFilter(&filter_config);

	if (result != 0) {
		printk("set pid filter failed\n");
	}

	if (dvb->feed_count == 1) {
		dvb->ts_buf = kzalloc(sizeof(u8)*2008 * 108, GFP_KERNEL);;
		dvb->cxd2880_ts_read_thread = kthread_run(cxd2880_ts_read,
											NULL,
											"cxd2880_ts_read");
	}

	result = dvb->feed_count;

	mutex_unlock(&dvb->spi_mutex);

	return result;
}

static int cxd2880_stop_feed(struct dvb_demux_feed *feed)
{
	struct cxd2880_tuner_demod_pid_filter_config filter_config;
	struct cxd2880_pid_filter* filter;
	struct list_head* list;

	mutex_lock(&dvb->spi_mutex);
	dvb->feed_count--;
	printk("stop feed (count %d)\n", dvb->feed_count);

	list_for_each(list, &pidlist_head) {
		filter = list_entry(list, struct cxd2880_pid_filter, pidlist);
		if (filter == NULL) {
			printk("filter is null\n");
			continue;
		}
		if (feed->pid == filter->config.pid) {
			printk("remove pid %d %d\n", feed->pid, filter->config.pid);
			list_del(&filter->pidlist);
			kfree(filter);
			break;
		}
	}

	if (dvb->feed_count == 0) {
		printk("all stop feed\n");
		memset(&filter_config, 0, 
				sizeof(struct	cxd2880_tuner_demod_pid_filter_config));
		cxd2880_SetPIDFilter(&filter_config);
		kthread_stop(dvb->cxd2880_ts_read_thread);
	}
	mutex_unlock(&dvb->spi_mutex);

	return 0;
}

static const struct of_device_id cxd2880_spi_of_match[] = {
	{ .compatible = "sony,cxd2880" },
	{ /* sentinel */ }
};

MODULE_DEVICE_TABLE(of, cxd2880_spi_of_match);

static int
cxd2880_spi_probe(struct spi_device *spi)
{
	int result = 0;
	struct cxd2880_config config;

	printk("cxd2880_spi_probe\n");
	printk("spi mode %d\n", spi->mode);

	dvb = kzalloc(sizeof(struct cxd2880_dvb), GFP_KERNEL);
	if (dvb == NULL) {
		printk("struct dvb alloc failed\n");
		return -ENOMEM;
	}
	dvb->feed_count =0;
	dvb->spi = spi;
	spin_lock_init(&dvb->spi_lock);
	spin_lock_init(&dvb->feed_lock);
	mutex_init(&dvb->spi_mutex);

	config.spi = spi;
	config.slock = &dvb->spi_lock;
	config.spi_mutex = &dvb->spi_mutex;

	cxd2880_attach(&dvb->fe, &config);

	result = dvb_register_adapter(&dvb->adapter,
								"CXD2880",
								THIS_MODULE,
								&spi->dev,
								adapter_nr);
	if (result < 0) {
		printk ("dvb_register_adapter() failed\n");
	}
	dvb->fe.ops.ts_bus_ctrl = cxd2880_dvb_bus_ctrl;
	result = dvb_register_frontend(&dvb->adapter,
								&dvb->fe);
	if (result < 0) {
		printk ("dvb_register_frontend() failed\n");
	}

	dvb->demux.dmx.capabilities = DMX_TS_FILTERING;
	dvb->demux.priv = dvb;
	dvb->demux.filternum = 32;
	dvb->demux.feednum = 32;
	dvb->demux.start_feed = cxd2880_start_feed;
	dvb->demux.stop_feed = cxd2880_stop_feed;

	result = dvb_dmx_init(&dvb->demux);
	if (result < 0) {
		printk ("dvb_dmx_init() failed\n");
	}

	dvb->dmxdev.filternum = 32;
	dvb->dmxdev.demux = &dvb->demux.dmx;
	dvb->dmxdev.capabilities = 0;
	result = dvb_dmxdev_init(&dvb->dmxdev,
							&dvb->adapter);
	if (result < 0) {
		printk ("dvb_dmxdev_init() failed\n");
	}

	dvb->fe_hw.source = DMX_FRONTEND_0;
	result = dvb->demux.dmx.add_frontend(&dvb->demux.dmx,
									&dvb->fe_hw);
	if (result < 0) {
		printk ("add_frontend() failed\n");
	}

	result = dvb->demux.dmx.connect_frontend(
										&dvb->demux.dmx,
										&dvb->fe_hw);
	if (result < 0) {
		printk ("dvb_register_frontend() failed\n");
	}
	return 0;
}


static int
cxd2880_spi_remove(struct spi_device *spi)
{
	struct cxd2880_pid_filter* filter;

	kfree(dvb->ts_buf);

	while(!list_empty(&pidlist_head)) {
		filter = list_entry(pidlist_head.next,
						struct cxd2880_pid_filter,
						pidlist);
		printk("dell pid %d\n", filter->config.pid);
		list_del(&filter->pidlist);
		kfree(filter);
	}

	dvb->demux.dmx.remove_frontend(&dvb->demux.dmx,
											&dvb->fe_hw);
	dvb_dmxdev_release(&dvb->dmxdev);
	dvb_dmx_release(&dvb->demux);
	dvb_unregister_frontend(&dvb->fe);
	dvb_unregister_adapter(&dvb->adapter);

	return 0;
}

static const struct spi_device_id cxd2880_spi_id[] = {
	{ "cxd2880", 0 },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(spi, cxd2880_spi_id);

static struct spi_driver cxd2880_spi_driver = {
	.driver	= {
		.name	= "cxd2880",
		.of_match_table = cxd2880_spi_of_match,
	},
	.id_table = cxd2880_spi_id,
	.probe    = cxd2880_spi_probe,
	.remove   = cxd2880_spi_remove,
};
module_spi_driver(cxd2880_spi_driver);

MODULE_DESCRIPTION("Sony CXD2880 DVB-T2/T tuner + demodulator drvier");
MODULE_AUTHOR("Sony Semiconductor Solutions Corporation");
MODULE_LICENSE("GPL v2");

