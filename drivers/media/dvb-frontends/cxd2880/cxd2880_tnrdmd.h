/*
* cxd2880_tnrdmd.h
* Sony CXD2880 DVB-T2/T tuner + demodulator driver
* The common tuner and demodulator control interface.
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

#ifndef CXD2880_TNRDMD_H
#define CXD2880_TNRDMD_H

#include "cxd2880_common.h"
#include "cxd2880_io.h"
#include "cxd2880_dtv.h"
#include "cxd2880_dvbt.h"
#include "cxd2880_dvbt2.h"

#define CXD2880_TNRDMD_MAX_CFG_MEMORY_COUNT 100

#define SLVT_UnFreezeReg(pTnrDmd) ((void)((pTnrDmd)->pIo->WriteOneRegister\
((pTnrDmd)->pIo, CXD2880_IO_TGT_DMD, 0x01, 0x00)))

#define CXD2880_TNRDMD_INTERRUPT_TYPE_BUF_UNDERFLOW     0x0001
#define CXD2880_TNRDMD_INTERRUPT_TYPE_BUF_OVERFLOW      0x0002
#define CXD2880_TNRDMD_INTERRUPT_TYPE_BUF_ALMOST_EMPTY  0x0004
#define CXD2880_TNRDMD_INTERRUPT_TYPE_BUF_ALMOST_FULL   0x0008
#define CXD2880_TNRDMD_INTERRUPT_TYPE_BUF_RRDY	  0x0010
#define CXD2880_TNRDMD_INTERRUPT_TYPE_ILLEGAL_COMMAND      0x0020
#define CXD2880_TNRDMD_INTERRUPT_TYPE_ILLEGAL_ACCESS       0x0040
#define CXD2880_TNRDMD_INTERRUPT_TYPE_CPU_ERROR	    0x0100
#define CXD2880_TNRDMD_INTERRUPT_TYPE_LOCK		 0x0200
#define CXD2880_TNRDMD_INTERRUPT_TYPE_INV_LOCK	     0x0400
#define CXD2880_TNRDMD_INTERRUPT_TYPE_NOOFDM	       0x0800
#define CXD2880_TNRDMD_INTERRUPT_TYPE_EWS		  0x1000
#define CXD2880_TNRDMD_INTERRUPT_TYPE_EEW		  0x2000
#define CXD2880_TNRDMD_INTERRUPT_TYPE_FEC_FAIL	     0x4000

#define CXD2880_TNRDMD_INTERRUPT_LOCK_SEL_L1POST_OK	0x01
#define CXD2880_TNRDMD_INTERRUPT_LOCK_SEL_DMD_LOCK	 0x02
#define CXD2880_TNRDMD_INTERRUPT_LOCK_SEL_TS_LOCK	  0x04

enum cxd2880_tnrdmd_chip_id {
	CXD2880_TNRDMD_CHIP_ID_UNKNOWN = 0x00,
	CXD2880_TNRDMD_CHIP_ID_CXD2880_ES1_0X = 0x62,
	CXD2880_TNRDMD_CHIP_ID_CXD2880_ES1_11 = 0x6A
};

#define CXD2880_TNRDMD_CHIP_ID_VALID(chipID) (((chipID) == \
CXD2880_TNRDMD_CHIP_ID_CXD2880_ES1_0X) || \
((chipID) == CXD2880_TNRDMD_CHIP_ID_CXD2880_ES1_11))

enum cxd2880_tnrdmd_state {
	CXD2880_TNRDMD_STATE_UNKNOWN,
	CXD2880_TNRDMD_STATE_SLEEP,
	CXD2880_TNRDMD_STATE_ACTIVE,
	CXD2880_TNRDMD_STATE_INVALID
};

enum cxd2880_tnrdmd_divermode {
	CXD2880_TNRDMD_DIVERMODE_SINGLE,
	CXD2880_TNRDMD_DIVERMODE_MAIN,
	CXD2880_TNRDMD_DIVERMODE_SUB
};

enum cxd2880_tnrdmd_clockmode {
	CXD2880_TNRDMD_CLOCKMODE_UNKNOWN,
	CXD2880_TNRDMD_CLOCKMODE_A,
	CXD2880_TNRDMD_CLOCKMODE_B,
	CXD2880_TNRDMD_CLOCKMODE_C
};

enum cxd2880_tnrdmd_tsout_if {
	CXD2880_TNRDMD_TSOUT_IF_TS,
	CXD2880_TNRDMD_TSOUT_IF_SPI,
	CXD2880_TNRDMD_TSOUT_IF_SDIO
};

enum cxd2880_tnrdmd_xtal_share {
	CXD2880_TNRDMD_XTAL_SHARE_NONE,
	CXD2880_TNRDMD_XTAL_SHARE_EXTREF,
	CXD2880_TNRDMD_XTAL_SHARE_MASTER,
	CXD2880_TNRDMD_XTAL_SHARE_SLAVE
};

enum cxd2880_tnrdmd_spectrum_sense {
	CXD2880_TNRDMD_SPECTRUM_NORMAL,
	CXD2880_TNRDMD_SPECTRUM_INV
};

enum cxd2880_tnrdmd_cfg_id {

	CXD2880_TNRDMD_CFG_OUTPUT_SEL_MSB,

	CXD2880_TNRDMD_CFG_TSVALID_ACTIVE_HI,

	CXD2880_TNRDMD_CFG_TSSYNC_ACTIVE_HI,

	CXD2880_TNRDMD_CFG_TSERR_ACTIVE_HI,

	CXD2880_TNRDMD_CFG_LATCH_ON_POSEDGE,

	CXD2880_TNRDMD_CFG_TSCLK_CONT,

	CXD2880_TNRDMD_CFG_TSCLK_MASK,

	CXD2880_TNRDMD_CFG_TSVALID_MASK,

	CXD2880_TNRDMD_CFG_TSERR_MASK,

	CXD2880_TNRDMD_CFG_TSERR_VALID_DIS,

	CXD2880_TNRDMD_CFG_TSPIN_CURRENT,

	CXD2880_TNRDMD_CFG_TSPIN_PULLUP_MANUAL,

	CXD2880_TNRDMD_CFG_TSPIN_PULLUP,

	CXD2880_TNRDMD_CFG_TSCLK_FREQ,

	CXD2880_TNRDMD_CFG_TSBYTECLK_MANUAL,

	CXD2880_TNRDMD_CFG_TS_PACKET_GAP,

	CXD2880_TNRDMD_CFG_TS_BACKWARDS_COMPATIBLE,

	CXD2880_TNRDMD_CFG_PWM_VALUE,

	CXD2880_TNRDMD_CFG_INTERRUPT,

	CXD2880_TNRDMD_CFG_INTERRUPT_LOCK_SEL,

	CXD2880_TNRDMD_CFG_INTERRUPT_INV_LOCK_SEL,

	CXD2880_TNRDMD_CFG_TS_BUF_ALMOST_EMPTY_THRS,

	CXD2880_TNRDMD_CFG_TS_BUF_ALMOST_FULL_THRS,

	CXD2880_TNRDMD_CFG_TS_BUF_RRDY_THRS,

	CXD2880_TNRDMD_CFG_FIXED_CLOCKMODE,

	CXD2880_TNRDMD_CFG_CABLE_INPUT,

	CXD2880_TNRDMD_CFG_DVBT2_FEF_INTERMITTENT_BASE,

	CXD2880_TNRDMD_CFG_DVBT2_FEF_INTERMITTENT_LITE,

	CXD2880_TNRDMD_CFG_BLINDTUNE_DVBT2_FIRST,

	CXD2880_TNRDMD_CFG_DVBT_BERN_PERIOD,

	CXD2880_TNRDMD_CFG_DVBT_VBER_PERIOD,

	CXD2880_TNRDMD_CFG_DVBT_PER_MES,

	CXD2880_TNRDMD_CFG_DVBT2_BBER_MES,

	CXD2880_TNRDMD_CFG_DVBT2_LBER_MES,

	CXD2880_TNRDMD_CFG_DVBT2_PER_MES,

	CXD2880_TNRDMD_CFG_ISDBT_BERPER_PERIOD
};

enum cxd2880_tnrdmd_lock_result {
	CXD2880_TNRDMD_LOCK_RESULT_NOTDETECT,
	CXD2880_TNRDMD_LOCK_RESULT_LOCKED,
	CXD2880_TNRDMD_LOCK_RESULT_UNLOCKED
};

enum cxd2880_tnrdmd_gpio_mode {

	CXD2880_TNRDMD_GPIO_MODE_OUTPUT = 0x00,

	CXD2880_TNRDMD_GPIO_MODE_INPUT = 0x01,

	CXD2880_TNRDMD_GPIO_MODE_INT = 0x02,

	CXD2880_TNRDMD_GPIO_MODE_FEC_FAIL = 0x03,

	CXD2880_TNRDMD_GPIO_MODE_PWM = 0x04,

	CXD2880_TNRDMD_GPIO_MODE_EWS = 0x05,

	CXD2880_TNRDMD_GPIO_MODE_EEW = 0x06
};

enum cxd2880_tnrdmd_serial_ts_clk {
	CXD2880_TNRDMD_SERIAL_TS_CLK_FULL,
	CXD2880_TNRDMD_SERIAL_TS_CLK_HALF
};

struct cxd2880_tnrdmd_cfg_memory {
	enum cxd2880_io_tgt tgt;
	u8 bank;
	u8 address;
	u8 value;
	u8 bitMask;
};

struct cxd2880_tnrdmd_pid_cfg {
	u8 isEn;
	u16 pid;
};

struct cxd2880_tnrdmd_pid_filter_cfg {

	u8 isNegative;

	struct cxd2880_tnrdmd_pid_cfg pidCfg[32];
};

struct cxd2880_tnrdmd_lna_thrs {
	u8 off_on;
	u8 on_off;
};

struct cxd2880_tnrdmd_lna_thrs_tbl_air {

	struct cxd2880_tnrdmd_lna_thrs thrs[24];
};

struct cxd2880_tnrdmd_lna_thrs_tbl_cable {

	struct cxd2880_tnrdmd_lna_thrs thrs[32];
};

struct cxd2880_tnrdmd_create_param {

	enum cxd2880_tnrdmd_tsout_if tsOutputIF;

	u8 enInternalLDO;

	enum cxd2880_tnrdmd_xtal_share xtalShareType;

	u8 xosc_cap;

	u8 xosc_i;

	u8 isCXD2881GG;

	u8 stationaryUse;

};

struct cxd2880_tnrdmd_diver_create_param {

	enum cxd2880_tnrdmd_tsout_if tsOutputIF;

	u8 enInternalLDO;

	u8 xosc_cap_main;

	u8 xosc_i_main;

	u8 xosc_i_sub;

	u8 isCXD2881GG;

	u8 stationaryUse;

};

struct cxd2880_tnrdmd {

	struct cxd2880_tnrdmd *pDiverSub;

	struct cxd2880_io *pIo;

	struct cxd2880_tnrdmd_create_param createParam;

	enum cxd2880_tnrdmd_divermode diverMode;

	enum cxd2880_tnrdmd_clockmode fixedClockMode;

	u8 isCableInput;

	u8 enFEFIntmtntBase;

	u8 enFEFIntmtntLite;

	u8 blindTuneDvbt2First;

	enum cxd2880_ret (*RFLvlCmpstn)(struct cxd2880_tnrdmd *pTnrDmd,
					 int *pRFLvldB);

	struct cxd2880_tnrdmd_lna_thrs_tbl_air *pLNAThrsTableAir;

	struct cxd2880_tnrdmd_lna_thrs_tbl_cable *pLNAThrsTableCable;

	u8 serialTsClockModeContinuous;

	enum cxd2880_tnrdmd_serial_ts_clk serialTsClkFreq;

	u8 tsByteClkManualSetting;

	u8 isTsBackwardsCompatibleMode;

	struct cxd2880_tnrdmd_cfg_memory
	    cfgMemory[CXD2880_TNRDMD_MAX_CFG_MEMORY_COUNT];

	u8 cfgMemoryLastEntry;

	struct cxd2880_tnrdmd_pid_filter_cfg pidFilterCfg;

	u8 pidFilterCfgEn;

	void *user;

	enum cxd2880_tnrdmd_chip_id chipID;

	enum cxd2880_tnrdmd_state state;

	enum cxd2880_tnrdmd_clockmode clockMode;

	u32 frequencyKHz;

	enum cxd2880_dtv_sys sys;

	enum cxd2880_dtv_bandwidth bandwidth;

	u8 scanMode;

	struct cxd2880_atomic cancel;

};

enum cxd2880_ret cxd2880_tnrdmd_Create(struct cxd2880_tnrdmd *pTnrDmd,
				       struct cxd2880_io *pIo,
				       struct cxd2880_tnrdmd_create_param
				       *pCreateParam);

enum cxd2880_ret cxd2880_tnrdmd_diver_Create(struct cxd2880_tnrdmd *pTnrDmdMain,
					     struct cxd2880_io *pIoMain,
					     struct cxd2880_tnrdmd *pTnrDmdSub,
					     struct cxd2880_io *pIoSub,
					     struct
					     cxd2880_tnrdmd_diver_create_param
					     *pCreateParam);

enum cxd2880_ret cxd2880_tnrdmd_Init1(struct cxd2880_tnrdmd *pTnrDmd);

enum cxd2880_ret cxd2880_tnrdmd_Init2(struct cxd2880_tnrdmd *pTnrDmd);

enum cxd2880_ret cxd2880_tnrdmd_CheckInternalCPUStatus(struct cxd2880_tnrdmd
						       *pTnrDmd,
						       u8 *pTaskCompleted);

enum cxd2880_ret cxd2880_tnrdmd_CommonTuneSetting1(struct cxd2880_tnrdmd
						   *pTnrDmd,
						   enum cxd2880_dtv_sys sys,
						   u32 frequencyKHz,
						   enum cxd2880_dtv_bandwidth
						   bandwidth,
						   u8 oneSegmentOptimize,
						   u8 oneSegOptShftDrctn);

enum cxd2880_ret cxd2880_tnrdmd_CommonTuneSetting2(struct cxd2880_tnrdmd
						   *pTnrDmd,
						   enum cxd2880_dtv_sys sys,
						   u8 enFEFIntmtntCtrl);

enum cxd2880_ret cxd2880_tnrdmd_Sleep(struct cxd2880_tnrdmd *pTnrDmd);

enum cxd2880_ret cxd2880_tnrdmd_SetCfg(struct cxd2880_tnrdmd *pTnrDmd,
				       enum cxd2880_tnrdmd_cfg_id id,
				       int value);

enum cxd2880_ret cxd2880_tnrdmd_GPIOSetCfg(struct cxd2880_tnrdmd *pTnrDmd,
					   u8 id,
					   u8 en,
					   enum cxd2880_tnrdmd_gpio_mode mode,
					   u8 openDrain, u8 invert);

enum cxd2880_ret cxd2880_tnrdmd_GPIOSetCfg_sub(struct cxd2880_tnrdmd *pTnrDmd,
					       u8 id,
					       u8 en,
					       enum cxd2880_tnrdmd_gpio_mode
					       mode, u8 openDrain, u8 invert);

enum cxd2880_ret cxd2880_tnrdmd_GPIORead(struct cxd2880_tnrdmd *pTnrDmd,
					 u8 id, u8 *pValue);

enum cxd2880_ret cxd2880_tnrdmd_GPIORead_sub(struct cxd2880_tnrdmd *pTnrDmd,
					     u8 id, u8 *pValue);

enum cxd2880_ret cxd2880_tnrdmd_GPIOWrite(struct cxd2880_tnrdmd *pTnrDmd,
					  u8 id, u8 value);

enum cxd2880_ret cxd2880_tnrdmd_GPIOWrite_sub(struct cxd2880_tnrdmd *pTnrDmd,
					      u8 id, u8 value);

enum cxd2880_ret cxd2880_tnrdmd_InterruptRead(struct cxd2880_tnrdmd *pTnrDmd,
					      u16 *pValue);

enum cxd2880_ret cxd2880_tnrdmd_InterruptClear(struct cxd2880_tnrdmd *pTnrDmd,
					       u16 value);

enum cxd2880_ret cxd2880_tnrdmd_TSBufClear(struct cxd2880_tnrdmd *pTnrDmd,
					   u8 clearOverflowFlag,
					   u8 clearUnderflowFlag, u8 clearBuf);

enum cxd2880_ret cxd2880_tnrdmd_ChipID(struct cxd2880_tnrdmd *pTnrDmd,
				       enum cxd2880_tnrdmd_chip_id *pChipID);

enum cxd2880_ret cxd2880_tnrdmd_SetAndSaveRegisterBits(struct cxd2880_tnrdmd
						       *pTnrDmd,
						       enum cxd2880_io_tgt tgt,
						       u8 bank, u8 address,
						       u8 value, u8 bitMask);

enum cxd2880_ret cxd2880_tnrdmd_SetScanMode(struct cxd2880_tnrdmd *pTnrDmd,
					    enum cxd2880_dtv_sys sys,
					    u8 scanModeEnd);

enum cxd2880_ret cxd2880_tnrdmd_SetPIDFilter(struct cxd2880_tnrdmd *pTnrDmd,
					     struct
					     cxd2880_tnrdmd_pid_filter_cfg
					     *pPIDFilterCfg);

enum cxd2880_ret cxd2880_tnrdmd_SetRFLvlCmpstn(struct cxd2880_tnrdmd *pTnrDmd,
					       enum
					       cxd2880_ret(*pRFLvlCmpstn)
					       (struct cxd2880_tnrdmd *,
						int *));

enum cxd2880_ret cxd2880_tnrdmd_SetRFLvlCmpstn_sub(struct cxd2880_tnrdmd
						   *pTnrDmd,
						   enum
						   cxd2880_ret(*pRFLvlCmpstn)
						   (struct cxd2880_tnrdmd *,
						    int *));

enum cxd2880_ret cxd2880_tnrdmd_SetLNAThrs(struct cxd2880_tnrdmd *pTnrDmd,
					   struct
					   cxd2880_tnrdmd_lna_thrs_tbl_air
					   *pTableAir,
					   struct
					   cxd2880_tnrdmd_lna_thrs_tbl_cable
					   *pTableCable);

enum cxd2880_ret cxd2880_tnrdmd_SetLNAThrs_sub(struct cxd2880_tnrdmd *pTnrDmd,
					       struct
					       cxd2880_tnrdmd_lna_thrs_tbl_air
					       *pTableAir,
					       struct
					       cxd2880_tnrdmd_lna_thrs_tbl_cable
					       *pTableCable);

enum cxd2880_ret cxd2880_tnrdmd_SetTSPinHighLow(struct cxd2880_tnrdmd *pTnrDmd,
						u8 en, u8 value);

enum cxd2880_ret cxd2880_tnrdmd_SetTSOutput(struct cxd2880_tnrdmd *pTnrDmd,
					    u8 en);

enum cxd2880_ret SLVT_FreezeReg(struct cxd2880_tnrdmd *pTnrDmd);

#endif
