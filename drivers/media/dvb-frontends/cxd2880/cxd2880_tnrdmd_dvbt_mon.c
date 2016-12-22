/*
* cxd2880_tnrdmd_dvbt_mon.c
* Sony CXD2880 DVB-T2/T tuner + demodulator driver
* The DVB-T tuner and demodulator monitor functions.
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

#include "cxd2880_tnrdmd_mon.h"
#include "cxd2880_tnrdmd_dvbt.h"
#include "cxd2880_tnrdmd_dvbt_mon.h"
#include "cxd2880_math.h"

static enum cxd2880_ret IsTPSLocked(struct cxd2880_tnrdmd *pTnrDmd);

enum cxd2880_ret cxd2880_tnrdmd_dvbt_mon_SyncStat(struct cxd2880_tnrdmd
						  *pTnrDmd, u8 *pSyncStat,
						  u8 *pTSLockStat,
						  u8 *pUnlockDetected)
{
	u8 rdata = 0x00;
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pSyncStat) || (!pTSLockStat) || (!pUnlockDetected))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;
	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x0D) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x10, &rdata,
			 1) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	*pUnlockDetected = (u8) ((rdata & 0x10) ? 1 : 0);
	*pSyncStat = (u8) (rdata & 0x07);
	*pTSLockStat = (u8) ((rdata & 0x20) ? 1 : 0);

	if (*pSyncStat == 0x07)
		return CXD2880_RESULT_ERROR_HW_STATE;

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt_mon_SyncStat_sub(struct cxd2880_tnrdmd
						      *pTnrDmd, u8 *pSyncStat,
						      u8 *pUnlockDetected)
{
	u8 tsLockStat = 0;

	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pSyncStat) || (!pUnlockDetected))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode != CXD2880_TNRDMD_DIVERMODE_MAIN)
		return CXD2880_RESULT_ERROR_ARG;

	ret =
	    cxd2880_tnrdmd_dvbt_mon_SyncStat(pTnrDmd->pDiverSub, pSyncStat,
					     &tsLockStat, pUnlockDetected);

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt_mon_ModeGuard(struct cxd2880_tnrdmd
						   *pTnrDmd,
						   enum cxd2880_dvbt_mode
						   *pMode,
						   enum cxd2880_dvbt_guard
						   *pGuard)
{
	u8 rdata = 0x00;

	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pMode) || (!pGuard))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (SLVT_FreezeReg(pTnrDmd) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	ret = IsTPSLocked(pTnrDmd);
	if (ret != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);

		if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN)
			ret =
			    cxd2880_tnrdmd_dvbt_mon_ModeGuard(pTnrDmd->
							      pDiverSub, pMode,
							      pGuard);

		return ret;
	}

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x0D) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x1B, &rdata,
			 1) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	SLVT_UnFreezeReg(pTnrDmd);

	*pMode = (enum cxd2880_dvbt_mode)((rdata >> 2) & 0x03);
	*pGuard = (enum cxd2880_dvbt_guard)(rdata & 0x03);

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt_mon_CarrierOffset(struct cxd2880_tnrdmd
						       *pTnrDmd, int *pOffset)
{
	u8 rdata[4];
	u32 ctlVal = 0;

	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pOffset))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (SLVT_FreezeReg(pTnrDmd) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	ret = IsTPSLocked(pTnrDmd);
	if (ret != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return ret;
	}

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x0D) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x1D, rdata,
			 4) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	SLVT_UnFreezeReg(pTnrDmd);

	ctlVal =
	    ((rdata[0] & 0x1F) << 24) | (rdata[1] << 16) | (rdata[2] << 8) |
	    (rdata[3]);
	*pOffset = cxd2880_Convert2SComplement(ctlVal, 29);
	*pOffset = -1 * ((*pOffset) * (u8) pTnrDmd->bandwidth / 235);

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt_mon_CarrierOffset_sub(struct cxd2880_tnrdmd
							   *pTnrDmd,
							   int *pOffset)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pOffset))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode != CXD2880_TNRDMD_DIVERMODE_MAIN)
		return CXD2880_RESULT_ERROR_ARG;

	ret =
	    cxd2880_tnrdmd_dvbt_mon_CarrierOffset(pTnrDmd->pDiverSub, pOffset);

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt_mon_PreViterbiBER(struct cxd2880_tnrdmd
						       *pTnrDmd, u32 *pBER)
{
	u8 rdata[2];
	u32 bitError = 0;
	u32 period = 0;

	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pBER))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (SLVT_FreezeReg(pTnrDmd) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x10) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x39, rdata,
			 1) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	if ((rdata[0] & 0x01) == 0) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_HW_STATE;
	}

	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x22, rdata,
			 2) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	bitError = (rdata[0] << 8) | rdata[1];

	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x6F, rdata,
			 1) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	SLVT_UnFreezeReg(pTnrDmd);

	period = ((rdata[0] & 0x07) == 0) ? 256 : (0x1000 << (rdata[0] & 0x07));

	if ((period == 0) || (bitError > period))
		return CXD2880_RESULT_ERROR_HW_STATE;

	{
		u32 div = 0;
		u32 Q = 0;
		u32 R = 0;

		div = period / 128;

		Q = (bitError * 3125) / div;
		R = (bitError * 3125) % div;

		R *= 25;
		Q = Q * 25 + R / div;
		R = R % div;

		if (div / 2 <= R)
			*pBER = Q + 1;
		else
			*pBER = Q;
	}
	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt_mon_PreRSBER(struct cxd2880_tnrdmd
						  *pTnrDmd, u32 *pBER)
{
	u8 rdata[3];
	u32 bitError = 0;
	u32 periodExp = 0;

	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pBER))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x0D) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x15, rdata,
			 3) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if ((rdata[0] & 0x40) == 0)
		return CXD2880_RESULT_ERROR_HW_STATE;

	bitError = ((rdata[0] & 0x3F) << 16) | (rdata[1] << 8) | rdata[2];

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x10) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x60, rdata,
			 1) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	periodExp = (rdata[0] & 0x1F);

	if ((periodExp <= 11) && (bitError > (1U << periodExp) * 204 * 8))
		return CXD2880_RESULT_ERROR_HW_STATE;

	{
		u32 div = 0;
		u32 Q = 0;
		u32 R = 0;

		if (periodExp <= 8)
			div = (1U << periodExp) * 51;
		else
			div = (1U << 8) * 51;

		Q = (bitError * 250) / div;
		R = (bitError * 250) % div;

		R *= 1250;
		Q = Q * 1250 + R / div;
		R = R % div;

		if (periodExp > 8) {
			*pBER = (Q + (1 << (periodExp - 9))) >> (periodExp - 8);
		} else {
			if (div / 2 <= R)
				*pBER = Q + 1;
			else
				*pBER = Q;
		}
	}

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt_mon_TPSInfo(struct cxd2880_tnrdmd *pTnrDmd,
						 struct cxd2880_dvbt_tpsinfo
						 *pInfo)
{
	u8 rdata[7];
	u8 cellIDOK = 0;

	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pInfo))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (SLVT_FreezeReg(pTnrDmd) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	ret = IsTPSLocked(pTnrDmd);
	if (ret != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);

		if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN)
			ret =
			    cxd2880_tnrdmd_dvbt_mon_TPSInfo(pTnrDmd->pDiverSub,
							    pInfo);

		return ret;
	}

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x0D) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x29, rdata,
			 7) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x11) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xD5, &cellIDOK,
			 1) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	SLVT_UnFreezeReg(pTnrDmd);

	pInfo->constellation =
	    (enum cxd2880_dvbt_constellation)((rdata[0] >> 6) & 0x03);
	pInfo->hierarchy =
	    (enum cxd2880_dvbt_hierarchy)((rdata[0] >> 3) & 0x07);
	pInfo->rateHP = (enum cxd2880_dvbt_coderate)(rdata[0] & 0x07);
	pInfo->rateLP = (enum cxd2880_dvbt_coderate)((rdata[1] >> 5) & 0x07);
	pInfo->guard = (enum cxd2880_dvbt_guard)((rdata[1] >> 3) & 0x03);
	pInfo->mode = (enum cxd2880_dvbt_mode)((rdata[1] >> 1) & 0x03);
	pInfo->fnum = (rdata[2] >> 6) & 0x03;
	pInfo->lengthIndicator = rdata[2] & 0x3F;
	pInfo->cellID = (u16) ((rdata[3] << 8) | rdata[4]);
	pInfo->reservedEven = rdata[5] & 0x3F;
	pInfo->reservedOdd = rdata[6] & 0x3F;

	pInfo->cellIDOK = cellIDOK & 0x01;

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt_mon_PacketErrorNumber(struct cxd2880_tnrdmd
							   *pTnrDmd, u32 *pPEN)
{
	u8 rdata[3];

	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pPEN))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x0D) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x26, rdata,
			 3) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (!(rdata[0] & 0x01))
		return CXD2880_RESULT_ERROR_HW_STATE;

	*pPEN = (rdata[1] << 8) | rdata[2];

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt_mon_SpectrumSense(struct cxd2880_tnrdmd
					       *pTnrDmd,
					       enum
					       cxd2880_tnrdmd_spectrum_sense
					       *pSense)
{
	u8 data = 0;

	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pSense))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (SLVT_FreezeReg(pTnrDmd) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	ret = IsTPSLocked(pTnrDmd);
	if (ret != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);

		if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN)
			ret =
			    cxd2880_tnrdmd_dvbt_mon_SpectrumSense(pTnrDmd->
								  pDiverSub,
								  pSense);

		return ret;
	}

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x0D) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x1C, &data,
			 sizeof(data)) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	SLVT_UnFreezeReg(pTnrDmd);

	*pSense =
	    (data & 0x01) ? CXD2880_TNRDMD_SPECTRUM_INV :
	    CXD2880_TNRDMD_SPECTRUM_NORMAL;

	return ret;
}

static enum cxd2880_ret dvbt_readSNRReg(struct cxd2880_tnrdmd *pTnrDmd,
					u16 *pRegValue)
{
	u8 rdata[2];

	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pRegValue))
		return CXD2880_RESULT_ERROR_ARG;

	if (SLVT_FreezeReg(pTnrDmd) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	ret = IsTPSLocked(pTnrDmd);
	if (ret != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return ret;
	}

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x0D) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x13, rdata,
			 2) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	SLVT_UnFreezeReg(pTnrDmd);

	*pRegValue = (rdata[0] << 8) | rdata[1];

	return ret;
}

static enum cxd2880_ret dvbt_calcSNR(struct cxd2880_tnrdmd *pTnrDmd,
				     u32 regValue, int *pSNR)
{

	if ((!pTnrDmd) || (!pSNR))
		return CXD2880_RESULT_ERROR_ARG;

	if (regValue == 0)
		return CXD2880_RESULT_ERROR_HW_STATE;

	if (regValue > 4996)
		regValue = 4996;

	*pSNR =
	    10 * 10 * ((int)cxd2880_math_log10(regValue) -
		       (int)cxd2880_math_log10(5350 - regValue));
	*pSNR += 28500;

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt_mon_SNR(struct cxd2880_tnrdmd *pTnrDmd,
					     int *pSNR)
{
	u16 regValue = 0;
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pSNR))
		return CXD2880_RESULT_ERROR_ARG;

	*pSNR = -1000 * 1000;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SINGLE) {
		ret = dvbt_readSNRReg(pTnrDmd, &regValue);
		if (ret != CXD2880_RESULT_OK)
			return ret;

		ret = dvbt_calcSNR(pTnrDmd, regValue, pSNR);
		if (ret != CXD2880_RESULT_OK)
			return ret;
	} else {
		int snrMain = 0;
		int snrSub = 0;

		ret =
		    cxd2880_tnrdmd_dvbt_mon_SNR_diver(pTnrDmd, pSNR, &snrMain,
						      &snrSub);
		if (ret != CXD2880_RESULT_OK)
			return ret;
	}

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt_mon_SNR_diver(struct cxd2880_tnrdmd
						   *pTnrDmd, int *pSNR,
						   int *pSNRMain, int *pSNRSub)
{
	u16 regValue = 0;
	u32 regValueSum = 0;
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pSNR) || (!pSNRMain) || (!pSNRSub))
		return CXD2880_RESULT_ERROR_ARG;

	*pSNR = -1000 * 1000;
	*pSNRMain = -1000 * 1000;
	*pSNRSub = -1000 * 1000;

	if (pTnrDmd->diverMode != CXD2880_TNRDMD_DIVERMODE_MAIN)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT)
		return CXD2880_RESULT_ERROR_SW_STATE;

	ret = dvbt_readSNRReg(pTnrDmd, &regValue);
	if (ret == CXD2880_RESULT_OK) {
		ret = dvbt_calcSNR(pTnrDmd, regValue, pSNRMain);
		if (ret != CXD2880_RESULT_OK)
			regValue = 0;
	} else if (ret == CXD2880_RESULT_ERROR_HW_STATE) {
		regValue = 0;
	} else {
		return ret;
	}

	regValueSum += regValue;

	ret = dvbt_readSNRReg(pTnrDmd->pDiverSub, &regValue);
	if (ret == CXD2880_RESULT_OK) {
		ret = dvbt_calcSNR(pTnrDmd->pDiverSub, regValue, pSNRSub);
		if (ret != CXD2880_RESULT_OK)
			regValue = 0;
	} else if (ret == CXD2880_RESULT_ERROR_HW_STATE) {
		regValue = 0;
	} else {
		return ret;
	}

	regValueSum += regValue;

	ret = dvbt_calcSNR(pTnrDmd, regValueSum, pSNR);

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt_mon_SamplingOffset(struct cxd2880_tnrdmd
							*pTnrDmd, int *pPPM)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pPPM))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT)
		return CXD2880_RESULT_ERROR_SW_STATE;

	{
		u8 ctlValReg[5];
		u8 nominalRateReg[5];
		u32 trlCtlVal = 0;
		u32 trcgNominalRate = 0;
		int num;
		int den;
		char diffUpper = 0;

		if (SLVT_FreezeReg(pTnrDmd) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		ret = IsTPSLocked(pTnrDmd);
		if (ret != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return ret;
		}

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x0D) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x21,
				 ctlValReg,
				 sizeof(ctlValReg)) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x04) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x60,
				 nominalRateReg,
				 sizeof(nominalRateReg)) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		SLVT_UnFreezeReg(pTnrDmd);

		diffUpper = (ctlValReg[0] & 0x7F) - (nominalRateReg[0] & 0x7F);

		if ((diffUpper < -1) || (diffUpper > 1))
			return CXD2880_RESULT_ERROR_HW_STATE;

		trlCtlVal = ctlValReg[1] << 24;
		trlCtlVal |= ctlValReg[2] << 16;
		trlCtlVal |= ctlValReg[3] << 8;
		trlCtlVal |= ctlValReg[4];

		trcgNominalRate = nominalRateReg[1] << 24;
		trcgNominalRate |= nominalRateReg[2] << 16;
		trcgNominalRate |= nominalRateReg[3] << 8;
		trcgNominalRate |= nominalRateReg[4];

		trlCtlVal >>= 1;
		trcgNominalRate >>= 1;

		if (diffUpper == 1)
			num =
			    (int)((trlCtlVal + 0x80000000u) - trcgNominalRate);
		else if (diffUpper == -1)
			num =
			    -(int)((trcgNominalRate + 0x80000000u) - trlCtlVal);
		else
			num = (int)(trlCtlVal - trcgNominalRate);

		den = (nominalRateReg[0] & 0x7F) << 24;
		den |= nominalRateReg[1] << 16;
		den |= nominalRateReg[2] << 8;
		den |= nominalRateReg[3];
		den = (den + (390625 / 2)) / 390625;

		den >>= 1;

		if (num >= 0)
			*pPPM = (num + (den / 2)) / den;
		else
			*pPPM = (num - (den / 2)) / den;
	}

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt_mon_SamplingOffset_sub(struct
							    cxd2880_tnrdmd
							    *pTnrDmd, int *pPPM)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pPPM))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode != CXD2880_TNRDMD_DIVERMODE_MAIN)
		return CXD2880_RESULT_ERROR_ARG;

	ret = cxd2880_tnrdmd_dvbt_mon_SamplingOffset(pTnrDmd->pDiverSub, pPPM);

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt_mon_Quality(struct cxd2880_tnrdmd *pTnrDmd,
						 u8 *pQuality)
{
	struct cxd2880_dvbt_tpsinfo tps;
	enum cxd2880_dvbt_profile profile = CXD2880_DVBT_PROFILE_HP;
	u32 ber = 0;
	int sn = 0;
	int snRel = 0;
	int berSQI = 0;

	static const int nordigNonHDVBTdB1000[3][5] = {
		{5100, 6900, 7900, 8900, 9700},
		{10800, 13100, 14600, 15600, 16000},
		{16500, 18700, 20200, 21600, 22500}
	};

	static const int nordigHierHpDVBTdB1000[3][2][5] = {
		{
		 {9100, 12000, 13600, 15000, 16600},
		 {10900, 14100, 15700, 19400, 20600}
		 },
		{
		 {6800, 9100, 10400, 11900, 12700},
		 {8500, 11000, 12800, 15000, 16000}
		 },
		{
		 {5800, 7900, 9100, 10300, 12100},
		 {8000, 9300, 11600, 13000, 12900}
		 }
	};

	static const int nordigHierLpDVBTdB1000[3][2][5] = {
		{
		 {12500, 14300, 15300, 16300, 16900},
		 {16700, 19100, 20900, 22500, 23700}
		 },
		{
		 {15000, 17200, 18400, 19100, 20100},
		 {18500, 21200, 23600, 24700, 25900}
		 },
		{
		 {19500, 21400, 22500, 23700, 24700},
		 {21900, 24200, 25600, 26900, 27800}
		 }
	};

	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pQuality))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT)
		return CXD2880_RESULT_ERROR_SW_STATE;

	ret = cxd2880_tnrdmd_dvbt_mon_TPSInfo(pTnrDmd, &tps);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	if (tps.hierarchy != CXD2880_DVBT_HIERARCHY_NON) {
		u8 data = 0;

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x10) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x67, &data,
				 1) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		profile =
		    ((data & 0x01) ==
		     0x01) ? CXD2880_DVBT_PROFILE_LP : CXD2880_DVBT_PROFILE_HP;
	}

	ret = cxd2880_tnrdmd_dvbt_mon_PreRSBER(pTnrDmd, &ber);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	ret = cxd2880_tnrdmd_dvbt_mon_SNR(pTnrDmd, &sn);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	if ((tps.constellation >= CXD2880_DVBT_CONSTELLATION_RESERVED_3) ||
	    (tps.rateHP >= CXD2880_DVBT_CODERATE_RESERVED_5) ||
	    (tps.rateLP >= CXD2880_DVBT_CODERATE_RESERVED_5) ||
	    (tps.hierarchy > CXD2880_DVBT_HIERARCHY_4)) {
		return CXD2880_RESULT_ERROR_OTHER;
	}

	if ((tps.hierarchy != CXD2880_DVBT_HIERARCHY_NON)
	    && (tps.constellation == CXD2880_DVBT_CONSTELLATION_QPSK))
		return CXD2880_RESULT_ERROR_OTHER;

	if (tps.hierarchy == CXD2880_DVBT_HIERARCHY_NON)
		snRel =
		    sn - nordigNonHDVBTdB1000[tps.constellation][tps.rateHP];
	else if (profile == CXD2880_DVBT_PROFILE_LP)
		snRel =
		    sn - nordigHierLpDVBTdB1000[(int)tps.hierarchy -
						1][(int)tps.constellation -
						   1][tps.rateLP];
	else
		snRel =
		    sn - nordigHierHpDVBTdB1000[(int)tps.hierarchy -
						1][(int)tps.constellation -
						   1][tps.rateHP];

	if (ber > 10000) {
		berSQI = 0;
	} else if (ber > 1) {

		berSQI = (int)(10 * cxd2880_math_log10(ber));
		berSQI = 20 * (7 * 1000 - (berSQI)) - 40 * 1000;
	} else {
		berSQI = 100 * 1000;
	}

	if (snRel < -7 * 1000) {
		*pQuality = 0;
	} else if (snRel < 3 * 1000) {
		int tmpSQI = (((snRel - (3 * 1000)) / 10) + 1000);
		*pQuality =
		    (u8) (((tmpSQI * berSQI) +
			   (1000000 / 2)) / (1000000)) & 0xFF;
	} else {
		*pQuality = (u8) ((berSQI + 500) / 1000);
	}

	if (*pQuality > 100)
		*pQuality = 100;

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt_mon_PER(struct cxd2880_tnrdmd *pTnrDmd,
					     u32 *pPER)
{
	u32 packetError = 0;
	u32 period = 0;
	u8 rdata[3];

	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pPER))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x0D) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x18, rdata,
			 3) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if ((rdata[0] & 0x01) == 0)
		return CXD2880_RESULT_ERROR_HW_STATE;

	packetError = (rdata[1] << 8) | rdata[2];

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x10) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x5C, rdata,
			 1) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	period = 1U << (rdata[0] & 0x0F);

	if ((period == 0) || (packetError > period))
		return CXD2880_RESULT_ERROR_HW_STATE;

	{
		u32 div = 0;
		u32 Q = 0;
		u32 R = 0;

		div = period;

		Q = (packetError * 1000) / div;
		R = (packetError * 1000) % div;

		R *= 1000;
		Q = Q * 1000 + R / div;
		R = R % div;

		if ((div != 1) && (div / 2 <= R))
			*pPER = Q + 1;
		else
			*pPER = Q;
	}

	return ret;
}

static enum cxd2880_ret dvbt_CalcSSI(struct cxd2880_tnrdmd *pTnrDmd, int rfLvl,
				     u8 *pSSI)
{
	struct cxd2880_dvbt_tpsinfo tps;
	int prel;
	int tempSSI = 0;
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	static const int pRefdBm1000[3][5] = {
		{-93000, -91000, -90000, -89000, -88000},
		{-87000, -85000, -84000, -83000, -82000},
		{-82000, -80000, -78000, -77000, -76000},
	};

	if ((!pTnrDmd) || (!pSSI))
		return CXD2880_RESULT_ERROR_ARG;

	ret = cxd2880_tnrdmd_dvbt_mon_TPSInfo(pTnrDmd, &tps);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	if ((tps.constellation >= CXD2880_DVBT_CONSTELLATION_RESERVED_3)
	    || (tps.rateHP >= CXD2880_DVBT_CODERATE_RESERVED_5))
		return CXD2880_RESULT_ERROR_OTHER;

	prel = rfLvl - pRefdBm1000[tps.constellation][tps.rateHP];

	if (prel < -15000)
		tempSSI = 0;
	else if (prel < 0)
		tempSSI = ((2 * (prel + 15000)) + 1500) / 3000;
	else if (prel < 20000)
		tempSSI = (((4 * prel) + 500) / 1000) + 10;
	else if (prel < 35000)
		tempSSI = (((2 * (prel - 20000)) + 1500) / 3000) + 90;
	else
		tempSSI = 100;

	*pSSI = (tempSSI > 100) ? 100 : (u8) tempSSI;

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt_mon_SSI(struct cxd2880_tnrdmd *pTnrDmd,
					     u8 *pSSI)
{
	int rfLvl = 0;
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pSSI))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT)
		return CXD2880_RESULT_ERROR_SW_STATE;

	ret = cxd2880_tnrdmd_mon_RFLvl(pTnrDmd, &rfLvl);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	ret = dvbt_CalcSSI(pTnrDmd, rfLvl, pSSI);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt_mon_SSI_sub(struct cxd2880_tnrdmd *pTnrDmd,
						 u8 *pSSI)
{
	int rfLvl = 0;
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pSSI))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode != CXD2880_TNRDMD_DIVERMODE_MAIN)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT)
		return CXD2880_RESULT_ERROR_SW_STATE;

	ret = cxd2880_tnrdmd_mon_RFLvl(pTnrDmd->pDiverSub, &rfLvl);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	ret = dvbt_CalcSSI(pTnrDmd, rfLvl, pSSI);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	return ret;
}

static enum cxd2880_ret IsTPSLocked(struct cxd2880_tnrdmd *pTnrDmd)
{
	u8 sync = 0;
	u8 tslock = 0;
	u8 earlyUnlock = 0;
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	ret =
	    cxd2880_tnrdmd_dvbt_mon_SyncStat(pTnrDmd, &sync, &tslock,
					     &earlyUnlock);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	if (sync != 6)
		return CXD2880_RESULT_ERROR_HW_STATE;

	return CXD2880_RESULT_OK;
}
