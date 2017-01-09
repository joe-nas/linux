/*
* cxd2880_spi_device.c
* Sony CXD2880 DVB-T2/T tuner + demodulator driver
* The SPI I/O access implemantation.
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

#include <linux/spi/spi.h>

#include "cxd2880_spi_device.h"


static enum cxd2880_ret cxd2880_spi_device_Read (struct cxd2880_spi * pSpi,
									 uint8_t * pData,
									 uint32_t size)
{
	struct cxd2880_spi_device* pSpiDevice = NULL;

	CXD2880_TRACE_IO_ENTER ("cxd2880_spi_device_Read");

	if ((!pSpi) || (!pSpi->user) || (!pData) || (size == 0)) {
		return CXD2880_RESULT_ERROR_ARG;
	}

	pSpiDevice = (struct cxd2880_spi_device*)(pSpi->user);

	return CXD2880_RESULT_OK;
}

static enum cxd2880_ret cxd2880_spi_device_Write (struct cxd2880_spi * pSpi, const uint8_t * pData, uint32_t size)
{
	struct cxd2880_spi_device* pSpiDevice = NULL;
	struct spi_message msg;
	struct spi_transfer tx;
	int result = 0;

	CXD2880_TRACE_IO_ENTER ("cxd2880_spi_device_Write");

	if ((!pSpi) || (!pSpi->user) || (!pData) || (size == 0)) {
		CXD2880_TRACE_IO_RETURN (CXD2880_RESULT_ERROR_ARG);
	}

	pSpiDevice = (struct cxd2880_spi_device*)(pSpi->user);

	memset(&tx, 0, sizeof(tx));
	tx.tx_buf = pData;
	tx.len = size;
    
	spi_message_init(&msg);
	spi_message_add_tail(&tx, &msg);
	result = spi_sync(pSpiDevice->spi, &msg);

	if (result < 0) {
		return CXD2880_RESULT_ERROR_IO;
	}

	return CXD2880_RESULT_OK;
}

static enum cxd2880_ret cxd2880_spi_device_WriteRead (struct cxd2880_spi * pSpi,
										const uint8_t * pTxData,
										uint32_t txSize,
										uint8_t * pRxData,
										uint32_t rxSize)
{
	struct cxd2880_spi_device* pSpiDevice = NULL;
	int result = 0;

	CXD2880_TRACE_IO_ENTER ("cxd2880_spi_device_WriteRead");

	if ((!pSpi) || (!pSpi->user) || (!pTxData) || (txSize == 0) || (!pRxData) || (rxSize == 0)) {
		return CXD2880_RESULT_ERROR_ARG;
	}

	pSpiDevice = (struct cxd2880_spi_device*)(pSpi->user);

	result = spi_write_then_read(pSpiDevice->spi, pTxData, txSize, pRxData, rxSize);

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_spi_device_Initialize (
									struct cxd2880_spi_device * pSpiSpidev, 
				                                        enum cxd2880_spi_mode mode,
									uint32_t speedHz)
{
	int result = 0;
	struct spi_device *spi = pSpiSpidev->spi;

	switch (mode) {
	case CXD2880_SPI_MODE_0:
		spi->mode = SPI_MODE_0;
		break;
	case CXD2880_SPI_MODE_1:
		spi->mode = SPI_MODE_1;
		break;
	case CXD2880_SPI_MODE_2:
		spi->mode = SPI_MODE_2;
		break;
	case CXD2880_SPI_MODE_3:
		spi->mode = SPI_MODE_3;
		break;
	default:
		CXD2880_TRACE_IO_RETURN (CXD2880_RESULT_ERROR_ARG);
	}

	spi->max_speed_hz = speedHz;
	spi->bits_per_word = 8;
	result = spi_setup(spi);
	if (result != 0) {
		printk("spi_setup failed %d\n", result);
	}

	return result;
}

enum cxd2880_ret cxd2880_spi_device_CreateSpi (
									struct cxd2880_spi * pSpi,
									struct cxd2880_spi_device* pSpiDevice)
{
	if ((!pSpi) || (!pSpiDevice)) {
		return CXD2880_RESULT_ERROR_ARG;
	}

	pSpi->Read = cxd2880_spi_device_Read;
	pSpi->Write = cxd2880_spi_device_Write;
	pSpi->WriteRead = cxd2880_spi_device_WriteRead;
	pSpi->flags = 0;
	pSpi->user = pSpiDevice;

	return CXD2880_RESULT_OK;
}
