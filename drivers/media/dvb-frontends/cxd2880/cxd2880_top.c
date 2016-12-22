/*
* cxd2880_top.c
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

#include <dvb_frontend.h>

#include "cxd2880.h"

#include "cxd2880_tnrdmd.h"
#include "cxd2880_io.h"
#include "cxd2880_devio_spi.h"
#include "cxd2880_spi_device.h"
#include "cxd2880_integ.h"

static struct return_value return_value_tbl[] = {
	{0, CXD2880_RESULT_OK},
	{EINVAL, CXD2880_RESULT_ERROR_ARG},
	{EIO, CXD2880_RESULT_ERROR_IO},
	{EBUSY, CXD2880_RESULT_ERROR_SW_STATE},
	{EBUSY, CXD2880_RESULT_ERROR_HW_STATE},
	{FE_TIMEDOUT, CXD2880_RESULT_ERROR_TIMEOUT},
	{EINVAL, CXD2880_RESULT_ERROR_RANGE},
	{EPERM, CXD2880_RESULT_ERROR_NOSUPPORT},
	{EFAULT, CXD2880_RESULT_ERROR_CANCEL},
	{EFAULT, CXD2880_RESULT_ERROR_OTHER},
	{EFAULT, CXD2880_RESULT_ERROR_OVERFLOW},
};

static struct bw_tbl bw_tbl[6] = {
	{0, CXD2880_DTV_BW_UNKNOWN},
	{1712000, CXD2880_DTV_BW_1_7_MHZ},
	{5000000, CXD2880_DTV_BW_5_MHZ},
	{6000000, CXD2880_DTV_BW_6_MHZ},
	{7000000, CXD2880_DTV_BW_7_MHZ},
	{8000000, CXD2880_DTV_BW_8_MHZ},
};

static struct dvbt_modulation dvbt_modulation_tbl[] = {
	{QPSK, CXD2880_DVBT_CONSTELLATION_QPSK},
	{QAM_16, CXD2880_DVBT_CONSTELLATION_16QAM},
	{QAM_64, CXD2880_DVBT_CONSTELLATION_64QAM},
};

static struct dvbt_guard dvbt_guard_tbl[] = {
	{GUARD_INTERVAL_1_32, CXD2880_DVBT_GUARD_1_32},
	{GUARD_INTERVAL_1_16, CXD2880_DVBT_GUARD_1_16},
	{GUARD_INTERVAL_1_8, CXD2880_DVBT_GUARD_1_8},
	{GUARD_INTERVAL_1_4, CXD2880_DVBT_GUARD_1_4},
};

static struct cxd2880_spi_device *spi_device;
static struct cxd2880_spi *cxd2880_spi;
static struct cxd2880_io *regio;
static struct cxd2880_tnrdmd *tuner_demod;
static struct cxd2880_priv *priv;

#define conv_from_cxd2880_to_dvb(value, table) ({	\
	int i = 0;	\
	while (table[i].cxd2880 != value) {	 \
		i++;	\
	}	\
	table[i].dvb;	\
})


static void cxd2880_release(struct dvb_frontend* fe)
{

}

static int cxd2880_init(struct dvb_frontend* fe)
{
	return 0;
}

static int cxd2880_sleep(struct dvb_frontend* fe)
{
	int result = 0;
	struct cxd2880_priv* priv = (struct cxd2880_priv*)fe->demodulator_priv;

	printk("cxd2880_sleep\n");
	return result;
}

static int cxd2880_tune(struct dvb_frontend* fe,
					bool retune,
					unsigned int mode_flags,
					unsigned int* delay,
					enum fe_status* status)
{
	int result = 0;
	printk("cxd2880_tune\n");
	return result;
}

static int cxd2880_set_frontend(struct dvb_frontend* fe)
{
	int result = 0;
	int i;
	struct dtv_frontend_properties * properties;
	struct cxd2880_priv* priv = fe->demodulator_priv;

	properties = &fe->dtv_property_cache;

	if (properties->delivery_system == SYS_DVBT) {
		priv->tuner_demod->sys = CXD2880_DTV_SYS_DVBT;
		priv->dvbt_tune_param.centerFreqKHz = properties->frequency / 1000;
		for (i = 0;i < 6;i++) {
			if (properties->bandwidth_hz == bw_tbl[i].dvb) {
				priv->dvbt_tune_param.bandwidth = bw_tbl[i].cxd2880;
				printk("bw = %d\n", i);
				break;
			}
		}
		priv->dvbt_tune_param.profile = CXD2880_DVBT_PROFILE_HP;
		printk("cxd_2880_set_frontend dvbt %d %d %d\n",
									properties->frequency /1000,
									properties->bandwidth_hz,
									priv->dvbt_tune_param.bandwidth);

		result = cxd2880_integ_dvbt_Tune(priv->tuner_demod,
							&priv->dvbt_tune_param);
		if (CXD2880_RESULT_OK != result)
			return conv_from_cxd2880_to_dvb(result, return_value_tbl);

	} else if (properties->delivery_system == SYS_DVBT2) {
		priv->tuner_demod->sys = CXD2880_DTV_SYS_DVBT2;
		priv->dvbt2_tune_param.centerFreqKHz = properties->frequency / 1000;
		for (i = 0;i < 6;i++) {
			if (properties->bandwidth_hz == bw_tbl[i].dvb) {
				priv->dvbt2_tune_param.bandwidth = bw_tbl[i].cxd2880;
				printk("bw = %d\n", i);
				break;
			}
		}
		priv->dvbt2_tune_param.dataPLPID = 0;
		result = cxd2880_integ_dvbt2_Tune(priv->tuner_demod,
							&priv->dvbt2_tune_param);
		if (CXD2880_RESULT_OK != result)
			return conv_from_cxd2880_to_dvb(result, return_value_tbl);

	}

	return result;
}

static int cxd2880_get_tune_settings(struct dvb_frontend* fe,
							struct dvb_frontend_tune_settings* settings)
{
	int result = 0;

	return result;
}

static int cxd2880_get_frontend(struct dvb_frontend* fe)
{
	int result = 0;
	struct cxd2880_priv* priv = (struct cxd2880_priv*)fe->demodulator_priv;
	struct dtv_frontend_properties* properties;
	struct cxd2880_tnrdmd* td;
	enum cxd2880_dvbt_mode mode;
	enum cxd2880_dvbt_guard guard;
	struct cxd2880_dvbt_tpsinfo tps;
	enum cxd2880_tnrdmd_spectrum_sense sense;

	enum cxd2880_dvbt2_plp_constell qam;
	struct cxd2880_dvbt2_ofdm ofdm;
	
	int i;

	properties = &fe->dtv_property_cache;
	td = priv->tuner_demod;

	if (CXD2880_DTV_SYS_DVBT == td->sys) {

		result = cxd2880_tnrdmd_dvbt_mon_TPSInfo(td, &tps);
		if (CXD2880_RESULT_OK != result)
			return conv_from_cxd2880_to_dvb(result, return_value_tbl);

		result = cxd2880_tnrdmd_dvbt_mon_ModeGuard(td, 
													&mode,
													&guard);
		if (CXD2880_RESULT_OK != result)
			return conv_from_cxd2880_to_dvb(result, return_value_tbl);	

		properties->frequency = priv->dvbt_tune_param.centerFreqKHz*1000;
		properties->delivery_system = SYS_DVBT;
		for (i = 0;i < 6;i++) {
			if (priv->dvbt_tune_param.bandwidth == bw_tbl[i].cxd2880) {
				properties->bandwidth_hz = bw_tbl[i].dvb * 1000;
				break;
			}
		}
		for (i = 0;i < 4;i++) {
			if (dvbt_guard_tbl[i].sony_guard == guard) {
				properties->guard_interval = dvbt_guard_tbl[i].dvb_guard;
				break;
			}
		}
		for (i = 0;i < 3;i++) {
			if (dvbt_modulation_tbl[i].sony_modulation == tps.constellation) {
				properties->modulation =
					dvbt_modulation_tbl[i].dvb_modulation;
				printk("conste %d\n", i);
				break;
			}
		}
	} else if (CXD2880_DTV_SYS_DVBT2 == td->sys) {
		properties->frequency = priv->dvbt2_tune_param.centerFreqKHz*1000;
		properties->delivery_system = SYS_DVBT2;

		result = cxd2880_tnrdmd_dvbt2_mon_QAM(td,
											CXD2880_DVBT2_PLP_DATA,
											&qam);
		if (CXD2880_RESULT_OK != result)
			return conv_from_cxd2880_to_dvb(result, return_value_tbl);

		result = cxd2880_tnrdmd_dvbt2_mon_OFDM(td, &ofdm);
		if (CXD2880_RESULT_OK != result)
			return conv_from_cxd2880_to_dvb(result, return_value_tbl);
	}

	return result;
}

static int cxd2880_read_status(struct dvb_frontend* fe, enum fe_status* status)
{
	int result = 0;
	u8 sync = 0;
	u8 lock = 0;
	u8 unlock = 0;
	struct cxd2880_priv* priv = (struct cxd2880_priv*)fe->demodulator_priv;
	struct cxd2880_tnrdmd* td;

	td = priv->tuner_demod;
	if (priv->tuner_demod->sys == CXD2880_DTV_SYS_DVBT) {
		result = cxd2880_tnrdmd_dvbt_mon_SyncStat(td,
												&sync,
												&lock,
												&unlock);
	} else if (priv->tuner_demod->sys == CXD2880_DTV_SYS_DVBT2) {
		result = cxd2880_tnrdmd_dvbt2_mon_SyncStat(td,
												&sync,
												&lock,
												&unlock);
	}

	if (lock == 1) { 
		*status = FE_HAS_LOCK |
				FE_HAS_SIGNAL |
				FE_HAS_CARRIER |
				FE_HAS_VITERBI |
				FE_HAS_SYNC;
	} else {
		if (unlock == 1) {
			*status = FE_REINIT;
		} else if (sync > 0) {
		*status = FE_HAS_SIGNAL |
				FE_HAS_CARRIER |
				FE_HAS_VITERBI |
				FE_HAS_SYNC;
		}
	}

	result =  conv_from_cxd2880_to_dvb(result, return_value_tbl);
	return result;
}

static int cxd2880_read_ber(struct dvb_frontend* fe, u32* ber)
{
	int result = 0;
	struct cxd2880_priv* priv = (struct cxd2880_priv*)fe->demodulator_priv;

	if (priv->tuner_demod->sys == CXD2880_DTV_SYS_DVBT) {
		result = cxd2880_tnrdmd_dvbt_mon_PreViterbiBER(priv->tuner_demod,
														ber);
	} else if (priv->tuner_demod->sys == CXD2880_DTV_SYS_DVBT2) {
		result = cxd2880_tnrdmd_dvbt2_mon_PreBCHBER(priv->tuner_demod,
													ber);
	}

	result =  conv_from_cxd2880_to_dvb(result, return_value_tbl);
	return result;
}

static int cxd2880_read_signal_strength(struct dvb_frontend* fe, u16* strength)
{
	int result = 0;
	int32_t rflevel;

	struct cxd2880_priv* priv = (struct cxd2880_priv*)fe->demodulator_priv;

	result = cxd2880_tnrdmd_mon_RFLvl(priv->tuner_demod, &rflevel);
	*strength = rflevel & 0xffff;

	result =  conv_from_cxd2880_to_dvb(result, return_value_tbl);
	return result;
}

static int cxd2880_read_snr(struct dvb_frontend* fe, u16* snr)
{
	int result = 0;
	struct cxd2880_priv* priv = (struct cxd2880_priv*)fe->demodulator_priv;

	if (priv->tuner_demod->sys == CXD2880_DTV_SYS_DVBT) {
		result = cxd2880_tnrdmd_dvbt_mon_SNR(priv->tuner_demod,
													snr);
	} else if (priv->tuner_demod->sys == CXD2880_DTV_SYS_DVBT2) {
		result = cxd2880_tnrdmd_dvbt2_mon_SNR(priv->tuner_demod,
													snr);
	} 

	result =  conv_from_cxd2880_to_dvb(result, return_value_tbl);
	return result;
}

static int cxd2880_read_ucblocks(struct dvb_frontend* fe, u32* ucblocks)
{
	int result = 0;
	struct cxd2880_priv* priv = (struct cxd2880_priv*)fe->demodulator_priv;

	if (priv->tuner_demod->sys == CXD2880_DTV_SYS_DVBT) {
		result = cxd2880_tnrdmd_dvbt_mon_PacketErrorNumber(
									priv->tuner_demod,
									ucblocks);
	} else if (priv->tuner_demod->sys == CXD2880_DTV_SYS_DVBT2) {
		result = cxd2880_tnrdmd_dvbt2_mon_PacketErrorNumber(
									priv->tuner_demod,
									ucblocks);
	}

	result =  conv_from_cxd2880_to_dvb(result, return_value_tbl);

	return result;
}

static int cxd2880_set_property(struct dvb_frontend* fe,
							struct dtv_property* tvp)
{
	int result = 0;
	struct cxd2880_priv* priv = (struct cxd2880_priv*)fe->demodulator_priv;
	int i;


	mutex_lock(priv->spi_mutex);
	printk("cxd2880_set_property %d\n", tvp->cmd);
	switch(tvp->cmd) {
	case DTV_CLEAR:
		printk("set property: CLEAR\n");
		break;
	case DTV_TUNE:
		if (priv->tuner_demod->sys == CXD2880_DTV_SYS_DVBT) {
			printk("system = dvb-t\n");
			cxd2880_integ_dvbt_Tune(priv->tuner_demod,
								&priv->dvbt_tune_param);
		} else if (priv->tuner_demod->sys ==
									CXD2880_DTV_SYS_DVBT2) {
			printk("system = dvb-t2\n");
			cxd2880_integ_dvbt2_Tune(priv->tuner_demod,
								&priv->dvbt2_tune_param);
		} else {
			printk("system = unknown\n");
		}
		break;
	case DTV_DELIVERY_SYSTEM:
		if (tvp->u.data == SYS_DVBT) {
			priv->tuner_demod->sys = CXD2880_DTV_SYS_DVBT;
		} else if (tvp->u.data == SYS_DVBT2) {
			priv->tuner_demod->sys = CXD2880_DTV_SYS_DVBT2;
		}
		break;
	case DTV_FREQUENCY:
		printk("set property: FREQUENCY");
		if (priv->tuner_demod->sys == CXD2880_DTV_SYS_DVBT) {
			priv->dvbt_tune_param.centerFreqKHz = tvp->u.data / 1000;
			printk("%d\n", priv->dvbt_tune_param.centerFreqKHz);
		} else if(priv->tuner_demod->sys == CXD2880_DTV_SYS_DVBT2){
			priv->dvbt2_tune_param.centerFreqKHz = tvp->u.data / 1000;
			printk("%d\n", priv->dvbt2_tune_param.centerFreqKHz);
		}
		break;
	case DTV_BANDWIDTH_HZ:

		printk("set property: BANDWIDTH %d\n", tvp->u.data);

		for (i = 0;i < 6;i++) {
			if (tvp->u.data == bw_tbl[i].dvb) {
				if (priv->tuner_demod->sys ==
										CXD2880_DTV_SYS_DVBT) {
					priv->dvbt_tune_param.bandwidth = bw_tbl[i].cxd2880;
					printk("%d\n", priv->dvbt_tune_param.bandwidth);
				} else {
					priv->dvbt2_tune_param.bandwidth = bw_tbl[i].cxd2880;
					printk("%d\n", priv->dvbt2_tune_param.bandwidth);
				}
			}
		}
		break;
	default:
		printk("set property ::%d\n", tvp->cmd);
		break;
	}

	mutex_unlock(priv->spi_mutex);

	return result;
}

static int cxd2880_get_property(struct dvb_frontend* fe,
							struct dtv_property* tvp)
{
	int result = 0;
	struct cxd2880_priv* priv = (struct cxd2880_priv*)fe->demodulator_priv;
	struct dtv_frontend_properties* properties;
	struct cxd2880_tnrdmd* td;
	u8 u8data;
	u32 u32data;

	properties = &fe->dtv_property_cache;
	td = priv->tuner_demod;

	switch(tvp->cmd) {
	case DTV_CLEAR:
		printk("get property: CLEAR\n");
		break;
	case DTV_TUNE:
		printk("get property: DTV_TUNE ");
		result = -EINVAL;
		break;
	case DTV_DELIVERY_SYSTEM:
		printk("get property: DTV_DELIVERY_SYSTEM\n");
		if (CXD2880_DTV_SYS_DVBT == td->sys) {
			tvp->u.data = SYS_DVBT;
		} else if (CXD2880_DTV_SYS_DVBT2 == td->sys) {
			tvp->u.data = SYS_DVBT2;
		}
		break;
	case DTV_FREQUENCY:
		printk("get property: FREQUENCY");
		if (td->sys == CXD2880_DTV_SYS_DVBT) {
			tvp->u.data = priv->dvbt_tune_param.centerFreqKHz * 1000;
		} else if(td->sys == CXD2880_DTV_SYS_DVBT2){
			tvp->u.data = priv->dvbt2_tune_param.centerFreqKHz * 1000;
		}
		break;
	case DTV_BANDWIDTH_HZ:
		printk("get property: BANDWIDTH %d\n", tvp->u.data);
		mutex_lock(priv->spi_mutex);
		if (td->sys == CXD2880_DTV_SYS_DVBT) {
			tvp->u.data = 
			conv_from_cxd2880_to_dvb(priv->dvbt_tune_param.bandwidth,
									bw_tbl);
		} else if (td->sys == CXD2880_DTV_SYS_DVBT2) {
			tvp->u.data = 
			conv_from_cxd2880_to_dvb(priv->dvbt2_tune_param.bandwidth,
									bw_tbl);
		}
		mutex_unlock(priv->spi_mutex);
		break;
	case DTV_STAT_SIGNAL_STRENGTH:
		printk("get property: SSI\n");
		mutex_lock(priv->spi_mutex);
		if (td->sys == CXD2880_DTV_SYS_DVBT) {
			result = 
				cxd2880_tnrdmd_dvbt_mon_SSI(priv->tuner_demod,
													&u8data);
		} else if (td->sys == CXD2880_DTV_SYS_DVBT2) {
			result = 
				cxd2880_tnrdmd_dvbt2_mon_SSI(priv->tuner_demod,
													&u8data);
		} 
		mutex_unlock(priv->spi_mutex);
		if (CXD2880_RESULT_OK != result) {
			properties->strength.stat[0].scale = FE_SCALE_NOT_AVAILABLE;
		} else {
			properties->strength.stat[0].scale = FE_SCALE_RELATIVE;
			properties->strength.stat[0].uvalue = 6554 * u8data;
			printk("ssi %d\n", u8data);
		}
		break;
	case DTV_STAT_CNR:
		printk("get property: CNR\n");
		mutex_lock(priv->spi_mutex);
		if (td->sys == CXD2880_DTV_SYS_DVBT) {
			result = cxd2880_tnrdmd_dvbt_mon_SNR(td,	&u32data);
		} else if (td->sys == CXD2880_DTV_SYS_DVBT2) {
			result = cxd2880_tnrdmd_dvbt2_mon_SNR(td, &u32data);
		} 
		mutex_unlock(priv->spi_mutex);
		if (CXD2880_RESULT_OK != result) {
			printk("get property: CNR failed\n");
			properties->cnr.stat[0].scale = FE_SCALE_NOT_AVAILABLE;
		} else {
			properties->cnr.stat[0].scale = FE_SCALE_DECIBEL;
			properties->cnr.stat[0].svalue = u32data;
		}
		break;
	case DTV_STAT_PRE_ERROR_BIT_COUNT:
		printk("get property: PRE BER\n");
		mutex_lock(priv->spi_mutex);
		if (td->sys == CXD2880_DTV_SYS_DVBT) {
			result = cxd2880_tnrdmd_dvbt_mon_PreViterbiBER(td,
														&u32data);
		} else if (td->sys == CXD2880_DTV_SYS_DVBT2) {
			result = cxd2880_tnrdmd_dvbt2_mon_PreLDPCBER(td,
														&u32data);
		} 
		mutex_unlock(priv->spi_mutex);
		if (CXD2880_RESULT_OK != result) {
			properties->pre_bit_error.stat[0].scale = FE_SCALE_NOT_AVAILABLE;
		} else {
			properties->pre_bit_error.stat[0].scale = FE_SCALE_DECIBEL;
			properties->pre_bit_error.stat[0].svalue = u32data;
		}
		break;
	case DTV_STAT_PRE_TOTAL_BIT_COUNT:
		break;
	case DTV_STAT_POST_ERROR_BIT_COUNT:
		mutex_lock(priv->spi_mutex);
		if (td->sys == CXD2880_DTV_SYS_DVBT) {
			result = cxd2880_tnrdmd_dvbt_mon_PreRSBER(td,
														&u32data);
		} else if (td->sys == CXD2880_DTV_SYS_DVBT2) {
			result = cxd2880_tnrdmd_dvbt2_mon_PreBCHBER(td,
														&u32data);
		} 
		mutex_unlock(priv->spi_mutex);
		if (CXD2880_RESULT_OK != result) {
			properties->post_bit_error.stat[0].scale = FE_SCALE_NOT_AVAILABLE;
		} else {
			properties->post_bit_error.stat[0].scale = FE_SCALE_DECIBEL;
			properties->post_bit_error.stat[0].svalue = u32data;
		}
		break;
	case DTV_STAT_POST_TOTAL_BIT_COUNT:
		break;
	case DTV_STAT_ERROR_BLOCK_COUNT:
		mutex_lock(priv->spi_mutex);
		if (td->sys == CXD2880_DTV_SYS_DVBT) {
			result = cxd2880_tnrdmd_dvbt_mon_PacketErrorNumber(td,
															&u32data);
		} else if (td->sys == CXD2880_DTV_SYS_DVBT2) {
			result = cxd2880_tnrdmd_dvbt2_mon_PacketErrorNumber(td,
															&u32data);
		} 
		mutex_unlock(priv->spi_mutex);
		if (CXD2880_RESULT_OK != result) {
			properties->block_error.stat[0].scale = FE_SCALE_NOT_AVAILABLE;
		} else {
			properties->block_error.stat[0].scale = FE_SCALE_DECIBEL;
			properties->block_error.stat[0].svalue = u32data;
		}
		break;
	case DTV_STAT_TOTAL_BLOCK_COUNT:
		break;
	default:
		break;
	}
	return result;
}

static struct dvb_frontend_ops cxd2880_dvbt_t2_ops;

struct dvb_frontend* cxd2880_attach(struct dvb_frontend *fe,
									struct cxd2880_config *cfg)
{
	enum cxd2880_ret result;
	struct cxd2880_tnrdmd_create_param createParam;

	printk("cxd2880_attach\n");
	if (!fe) {
		printk("cxd2880_attach invalid arg fe\n");
		return NULL;
	}
	printk("ops %p table %p\n", &fe->ops, &cxd2880_dvbt_t2_ops);

	spi_device = kzalloc(sizeof(struct cxd2880_spi_device), GFP_KERNEL);;
	if (NULL == spi_device) {
		printk("spi_device alloc failed\n");
		return NULL;
	}
	cxd2880_spi = kzalloc(sizeof(struct cxd2880_spi), GFP_KERNEL);;
	if (NULL == cxd2880_spi) {
		printk("cxd2880_spi alloc failed\n");
		return NULL;
	}
	regio = kzalloc(sizeof(struct cxd2880_io), GFP_KERNEL);;
	if (NULL == regio) {
		printk("cxd2880_spi alloc failed\n");
		return NULL;
	}
	tuner_demod = kzalloc(sizeof(struct cxd2880_tnrdmd), GFP_KERNEL);
	if (NULL == tuner_demod) {
		printk("cxd2880_tuner_demod alloc failed\n");
		return NULL;
	}
	priv = kzalloc(sizeof(struct cxd2880_priv), GFP_KERNEL);
	if (NULL == priv) {
		return NULL;
	}

	priv->tuner_demod = tuner_demod;
	priv->spi = cfg->spi;
	priv->slock = cfg->slock;
	priv->spi_mutex = cfg->spi_mutex;
	fe->demodulator_priv = priv;

	spi_device->spi = cfg->spi;

	result = cxd2880_spi_device_Initialize(spi_device,
									CXD2880_SPI_MODE_0,
									50000000);
	result = cxd2880_spi_device_CreateSpi(cxd2880_spi, spi_device);
	result = cxd2880_io_spi_Create(regio, cxd2880_spi, 0);

	createParam.tsOutputIF = CXD2880_TNRDMD_TSOUT_IF_SPI;
	createParam.xtalShareType = CXD2880_TNRDMD_XTAL_SHARE_NONE;
	createParam.enInternalLDO = 1;
	createParam.xosc_cap = 18;
	createParam.xosc_i = 8;
	createParam.stationaryUse = 1;

	result = cxd2880_tnrdmd_Create(tuner_demod, regio, &createParam);
	if (result != CXD2880_RESULT_OK) {
		printk("cxd2880_tuner_demod_Create failed\n");
		return NULL;
	}

	result = cxd2880_integ_Init(tuner_demod);
	if (result != 0) {
		printk("cxd2880_integ_initialize failed %d\n", result);
		return NULL;
	}

	memcpy(&fe->ops, &cxd2880_dvbt_t2_ops,
			sizeof(struct dvb_frontend_ops));

	return fe;
}
EXPORT_SYMBOL(cxd2880_attach);

static struct dvb_frontend_ops cxd2880_dvbt_t2_ops = {
	.info = {
		.name = "Sony CXD2880",
		.frequency_min =  174000000,
		.frequency_max = 862000000,
		.frequency_stepsize = 1000,
		.caps = 	FE_CAN_INVERSION_AUTO |
				FE_CAN_FEC_1_2 |
				FE_CAN_FEC_2_3 |
				FE_CAN_FEC_3_4 |
				FE_CAN_FEC_4_5 |
				FE_CAN_FEC_5_6	|
				FE_CAN_FEC_7_8	|
				FE_CAN_FEC_AUTO |
				FE_CAN_QPSK |
				FE_CAN_QAM_16 |
				FE_CAN_QAM_32 |
				FE_CAN_QAM_64 |
				FE_CAN_QAM_128 |
				FE_CAN_QAM_256 |
				FE_CAN_QAM_AUTO |
				FE_CAN_TRANSMISSION_MODE_AUTO |
				FE_CAN_GUARD_INTERVAL_AUTO |
				FE_CAN_2G_MODULATION |
				FE_CAN_RECOVER |
				FE_CAN_MUTE_TS,
	},
	.delsys = { SYS_DVBT, SYS_DVBT2 },
	.release = cxd2880_release,
	.init = cxd2880_init,
	.sleep = cxd2880_sleep,
	.tune = cxd2880_tune,
	.set_frontend = cxd2880_set_frontend,
	.get_tune_settings = cxd2880_get_tune_settings,
	.get_frontend = cxd2880_get_frontend,
	.read_status = cxd2880_read_status,
	.read_ber = cxd2880_read_ber,
	.read_signal_strength = cxd2880_read_signal_strength,
	.read_snr = cxd2880_read_snr,
	.read_ucblocks = cxd2880_read_ucblocks,
	.read_ucblocks = cxd2880_read_ucblocks,
	.set_property = cxd2880_set_property,
	.get_property = cxd2880_get_property,
};

MODULE_DESCRIPTION("Sony CXD2880 DVB-T2/T tuner + demodulator drvier");
MODULE_AUTHOR("Sony Semiconductor Solutions Corporation");
MODULE_LICENSE("GPL v2");
