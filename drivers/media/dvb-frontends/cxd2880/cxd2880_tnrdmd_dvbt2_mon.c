/*
* cxd2880_tnrdmd_dvbt2_mon.c
* Sony CXD2880 DVB-T2/T tuner + demodulator driver
* The DVB-T2 tuner and demodulator monitor functions.
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
#include "cxd2880_tnrdmd_dvbt2.h"
#include "cxd2880_tnrdmd_dvbt2_mon.h"
#include "cxd2880_math.h"

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_SyncStat(struct cxd2880_tnrdmd
						   *pTnrDmd, u8 *pSyncStat,
						   u8 *pTSLockStat,
						   u8 *pUnlockDetected)
{

	if ((!pTnrDmd) || (!pSyncStat) || (!pTSLockStat) || (!pUnlockDetected))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT2)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x0B) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	{
		u8 data;

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x10, &data,
				 sizeof(data)) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		*pSyncStat = data & 0x07;
		*pTSLockStat = ((data & 0x20) ? 1 : 0);
		*pUnlockDetected = ((data & 0x10) ? 1 : 0);
	}

	if (*pSyncStat == 0x07)
		return CXD2880_RESULT_ERROR_HW_STATE;

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_SyncStat_sub(struct cxd2880_tnrdmd
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
	    cxd2880_tnrdmd_dvbt2_mon_SyncStat(pTnrDmd->pDiverSub, pSyncStat,
					      &tsLockStat, pUnlockDetected);

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_CarrierOffset(struct cxd2880_tnrdmd
							*pTnrDmd, int *pOffset)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pOffset))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT2)
		return CXD2880_RESULT_ERROR_SW_STATE;

	{
		u8 data[4];
		u32 ctlVal = 0;
		u8 syncState = 0;
		u8 tsLock = 0;
		u8 unlockDetected = 0;

		if (SLVT_FreezeReg(pTnrDmd) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		ret =
		    cxd2880_tnrdmd_dvbt2_mon_SyncStat(pTnrDmd, &syncState,
						      &tsLock, &unlockDetected);
		if (ret != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (syncState != 6) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_HW_STATE;
		}

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x0B) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x30, data,
				 sizeof(data)) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		SLVT_UnFreezeReg(pTnrDmd);

		ctlVal =
		    ((data[0] & 0x0F) << 24) | (data[1] << 16) | (data[2] << 8)
		    | (data[3]);
		*pOffset = cxd2880_Convert2SComplement(ctlVal, 28);

		switch (pTnrDmd->bandwidth) {
		case CXD2880_DTV_BW_1_7_MHZ:
			*pOffset = -1 * ((*pOffset) / 582);
			break;
		case CXD2880_DTV_BW_5_MHZ:
		case CXD2880_DTV_BW_6_MHZ:
		case CXD2880_DTV_BW_7_MHZ:
		case CXD2880_DTV_BW_8_MHZ:
			*pOffset =
			    -1 * ((*pOffset) * (u8) pTnrDmd->bandwidth / 940);
			break;
		default:
			return CXD2880_RESULT_ERROR_SW_STATE;
		}
	}

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_CarrierOffset_sub(struct
							    cxd2880_tnrdmd
							    *pTnrDmd,
							    int *pOffset)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pOffset))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode != CXD2880_TNRDMD_DIVERMODE_MAIN)
		return CXD2880_RESULT_ERROR_ARG;

	ret =
	    cxd2880_tnrdmd_dvbt2_mon_CarrierOffset(pTnrDmd->pDiverSub, pOffset);

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_L1Pre(struct cxd2880_tnrdmd *pTnrDmd,
						struct cxd2880_dvbt2_l1pre
						*pL1Pre)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;
	u8 data[37];
	u8 syncState = 0;
	u8 tsLock = 0;
	u8 unlockDetected = 0;
	u8 version = 0;
	enum cxd2880_dvbt2_profile profile;

	if ((!pTnrDmd) || (!pL1Pre))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT2)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (SLVT_FreezeReg(pTnrDmd) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	ret =
	    cxd2880_tnrdmd_dvbt2_mon_SyncStat(pTnrDmd, &syncState,
					      &tsLock, &unlockDetected);
	if (ret != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return ret;
	}

	if (syncState < 5) {
		if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
			ret =
			    cxd2880_tnrdmd_dvbt2_mon_SyncStat_sub
			    (pTnrDmd, &syncState, &unlockDetected);
			if (ret != CXD2880_RESULT_OK) {
				SLVT_UnFreezeReg(pTnrDmd);
				return ret;
			}

			if (syncState < 5) {
				SLVT_UnFreezeReg(pTnrDmd);
				return CXD2880_RESULT_ERROR_HW_STATE;
			}
		} else {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_HW_STATE;
		}
	}

	ret = cxd2880_tnrdmd_dvbt2_mon_Profile(pTnrDmd, &profile);
	if (ret != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return ret;
	}

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x0B) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x61, data,
			 sizeof(data)) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}
	SLVT_UnFreezeReg(pTnrDmd);

	pL1Pre->type = (enum cxd2880_dvbt2_l1pre_type)data[0];
	pL1Pre->bwExt = data[1] & 0x01;
	pL1Pre->s1 = (enum cxd2880_dvbt2_s1)(data[2] & 0x07);
	pL1Pre->s2 = data[3] & 0x0F;
	pL1Pre->l1Rep = data[4] & 0x01;
	pL1Pre->gi = (enum cxd2880_dvbt2_guard)(data[5] & 0x07);
	pL1Pre->papr = (enum cxd2880_dvbt2_papr)(data[6] & 0x0F);
	pL1Pre->mod =
	    (enum cxd2880_dvbt2_l1post_constell)(data[7] & 0x0F);
	pL1Pre->cr = (enum cxd2880_dvbt2_l1post_cr)(data[8] & 0x03);
	pL1Pre->fec =
	    (enum cxd2880_dvbt2_l1post_fec_type)(data[9] & 0x03);
	pL1Pre->l1PostSize = (data[10] & 0x03) << 16;
	pL1Pre->l1PostSize |= (data[11]) << 8;
	pL1Pre->l1PostSize |= (data[12]);
	pL1Pre->l1PostInfoSize = (data[13] & 0x03) << 16;
	pL1Pre->l1PostInfoSize |= (data[14]) << 8;
	pL1Pre->l1PostInfoSize |= (data[15]);
	pL1Pre->pp = (enum cxd2880_dvbt2_pp)(data[16] & 0x0F);
	pL1Pre->txIdAvailability = data[17];
	pL1Pre->cellId = (data[18] << 8);
	pL1Pre->cellId |= (data[19]);
	pL1Pre->networkId = (data[20] << 8);
	pL1Pre->networkId |= (data[21]);
	pL1Pre->sysId = (data[22] << 8);
	pL1Pre->sysId |= (data[23]);
	pL1Pre->numFrames = data[24];
	pL1Pre->numSymbols = (data[25] & 0x0F) << 8;
	pL1Pre->numSymbols |= data[26];
	pL1Pre->regen = data[27] & 0x07;
	pL1Pre->postExt = data[28] & 0x01;
	pL1Pre->numRfFreqs = data[29] & 0x07;
	pL1Pre->rfIdx = data[30] & 0x07;
	version = (data[31] & 0x03) << 2;
	version |= (data[32] & 0xC0) >> 6;
	pL1Pre->t2Version = (enum cxd2880_dvbt2_version)version;
	pL1Pre->l1PostScrambled = (data[32] & 0x20) >> 5;
	pL1Pre->t2BaseLite = (data[32] & 0x10) >> 4;
	pL1Pre->crc32 = (data[33] << 24);
	pL1Pre->crc32 |= (data[34] << 16);
	pL1Pre->crc32 |= (data[35] << 8);
	pL1Pre->crc32 |= data[36];

	if (profile == CXD2880_DVBT2_PROFILE_BASE) {
		switch ((pL1Pre->s2 >> 1)) {
		case CXD2880_DVBT2_BASE_S2_M1K_G_ANY:
			pL1Pre->fftMode = CXD2880_DVBT2_M1K;
			break;
		case CXD2880_DVBT2_BASE_S2_M2K_G_ANY:
			pL1Pre->fftMode = CXD2880_DVBT2_M2K;
			break;
		case CXD2880_DVBT2_BASE_S2_M4K_G_ANY:
			pL1Pre->fftMode = CXD2880_DVBT2_M4K;
			break;
		case CXD2880_DVBT2_BASE_S2_M8K_G_DVBT:
		case CXD2880_DVBT2_BASE_S2_M8K_G_DVBT2:
			pL1Pre->fftMode = CXD2880_DVBT2_M8K;
			break;
		case CXD2880_DVBT2_BASE_S2_M16K_G_ANY:
			pL1Pre->fftMode = CXD2880_DVBT2_M16K;
			break;
		case CXD2880_DVBT2_BASE_S2_M32K_G_DVBT:
		case CXD2880_DVBT2_BASE_S2_M32K_G_DVBT2:
			pL1Pre->fftMode = CXD2880_DVBT2_M32K;
			break;
		default:
			return CXD2880_RESULT_ERROR_HW_STATE;
		}
	} else if (profile == CXD2880_DVBT2_PROFILE_LITE) {
		switch ((pL1Pre->s2 >> 1)) {
		case CXD2880_DVBT2_LITE_S2_M2K_G_ANY:
			pL1Pre->fftMode = CXD2880_DVBT2_M2K;
			break;
		case CXD2880_DVBT2_LITE_S2_M4K_G_ANY:
			pL1Pre->fftMode = CXD2880_DVBT2_M4K;
			break;
		case CXD2880_DVBT2_LITE_S2_M8K_G_DVBT:
		case CXD2880_DVBT2_LITE_S2_M8K_G_DVBT2:
			pL1Pre->fftMode = CXD2880_DVBT2_M8K;
			break;
		case CXD2880_DVBT2_LITE_S2_M16K_G_DVBT:
		case CXD2880_DVBT2_LITE_S2_M16K_G_DVBT2:
			pL1Pre->fftMode = CXD2880_DVBT2_M16K;
			break;
		default:
			return CXD2880_RESULT_ERROR_HW_STATE;
		}
	} else {
		return CXD2880_RESULT_ERROR_HW_STATE;
	}

	pL1Pre->mixed = pL1Pre->s2 & 0x01;


	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_Version(struct cxd2880_tnrdmd
						  *pTnrDmd,
						  enum cxd2880_dvbt2_version
						  *pVersion)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;
	u8 version = 0;
	u8 data[2];
	u8 syncState = 0;
	u8 tsLock = 0;
	u8 unlockDetected = 0;


	if ((!pTnrDmd) || (!pVersion))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT2)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (SLVT_FreezeReg(pTnrDmd) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	ret =
	    cxd2880_tnrdmd_dvbt2_mon_SyncStat(pTnrDmd, &syncState,
					      &tsLock, &unlockDetected);
	if (ret != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	if (syncState < 5) {
		if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
			ret =
			    cxd2880_tnrdmd_dvbt2_mon_SyncStat_sub
			    (pTnrDmd, &syncState, &unlockDetected);
			if (ret != CXD2880_RESULT_OK) {
				SLVT_UnFreezeReg(pTnrDmd);
				return ret;
			}

			if (syncState < 5) {
				SLVT_UnFreezeReg(pTnrDmd);
				return CXD2880_RESULT_ERROR_HW_STATE;
			}
		} else {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_HW_STATE;
		}
	}

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x0B) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x80, data,
			 sizeof(data)) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	SLVT_UnFreezeReg(pTnrDmd);

	version = ((data[0] & 0x03) << 2);
	version |= ((data[1] & 0xC0) >> 6);
	*pVersion = (enum cxd2880_dvbt2_version)version;


	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_OFDM(struct cxd2880_tnrdmd *pTnrDmd,
					       struct cxd2880_dvbt2_ofdm *pOfdm)
{

	if ((!pTnrDmd) || (!pOfdm))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT2)
		return CXD2880_RESULT_ERROR_SW_STATE;

	{
		u8 data[5];
		u8 syncState = 0;
		u8 tsLock = 0;
		u8 unlockDetected = 0;
		enum cxd2880_ret ret = CXD2880_RESULT_OK;

		if (SLVT_FreezeReg(pTnrDmd) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		ret =
		    cxd2880_tnrdmd_dvbt2_mon_SyncStat(pTnrDmd, &syncState,
						      &tsLock, &unlockDetected);
		if (ret != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (syncState != 6) {
			SLVT_UnFreezeReg(pTnrDmd);

			ret = CXD2880_RESULT_ERROR_HW_STATE;

			if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN)
				ret =
				    cxd2880_tnrdmd_dvbt2_mon_OFDM(pTnrDmd->
								  pDiverSub,
								  pOfdm);

			return ret;
		}

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x0B) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x1D, data,
				 sizeof(data)) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		SLVT_UnFreezeReg(pTnrDmd);

		pOfdm->mixed = ((data[0] & 0x20) ? 1 : 0);
		pOfdm->isMiso = ((data[0] & 0x10) >> 4);
		pOfdm->mode = (enum cxd2880_dvbt2_mode)(data[0] & 0x07);
		pOfdm->gi = (enum cxd2880_dvbt2_guard)((data[1] & 0x70) >> 4);
		pOfdm->pp = (enum cxd2880_dvbt2_pp)(data[1] & 0x07);
		pOfdm->bwExt = (data[2] & 0x10) >> 4;
		pOfdm->papr = (enum cxd2880_dvbt2_papr)(data[2] & 0x0F);
		pOfdm->numSymbols = (data[3] << 8) | data[4];
	}

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_DataPLPs(struct cxd2880_tnrdmd
						   *pTnrDmd, u8 *pPLPIds,
						   u8 *pNumPLPs)
{

	if ((!pTnrDmd) || (!pNumPLPs))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT2)
		return CXD2880_RESULT_ERROR_SW_STATE;

	{
		u8 l1PostOK = 0;

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x0B) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (SLVT_FreezeReg(pTnrDmd) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x86,
				 &l1PostOK, 1) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (!(l1PostOK & 0x01)) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_HW_STATE;
		}

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xC1,
				 pNumPLPs, 1) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (*pNumPLPs == 0) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_OTHER;
		}

		if (!pPLPIds) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_OK;
		}

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xC2,
				 pPLPIds,
				 ((*pNumPLPs > 62) ? 62 : *pNumPLPs)) !=
		    CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (*pNumPLPs > 62) {
			if (pTnrDmd->pIo->
			    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					     0x00, 0x0C) != CXD2880_RESULT_OK) {
				SLVT_UnFreezeReg(pTnrDmd);
				return CXD2880_RESULT_ERROR_IO;
			}

			if (pTnrDmd->pIo->
			    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x10,
					 pPLPIds + 62,
					 *pNumPLPs - 62) != CXD2880_RESULT_OK) {
				SLVT_UnFreezeReg(pTnrDmd);
				return CXD2880_RESULT_ERROR_IO;
			}
		}

		SLVT_UnFreezeReg(pTnrDmd);
	}
	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_ActivePLP(struct cxd2880_tnrdmd
						    *pTnrDmd,
						    enum cxd2880_dvbt2_plp_btype
						    type,
						    struct cxd2880_dvbt2_plp
						    *pPLPInfo)
{

	if ((!pTnrDmd) || (!pPLPInfo))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT2)
		return CXD2880_RESULT_ERROR_SW_STATE;

	{
		u8 data[20];
		u8 addr = 0;
		u8 index = 0;
		u8 l1PostOk = 0;

		if (SLVT_FreezeReg(pTnrDmd) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x0B) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x86,
				 &l1PostOk, 1) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (!l1PostOk) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_HW_STATE;
		}

		if (type == CXD2880_DVBT2_PLP_COMMON)
			addr = 0xA9;
		else
			addr = 0x96;

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, addr, data,
				 sizeof(data)) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		SLVT_UnFreezeReg(pTnrDmd);

		if ((type == CXD2880_DVBT2_PLP_COMMON) && (data[13] == 0))
			return CXD2880_RESULT_ERROR_HW_STATE;

		pPLPInfo->id = data[index++];
		pPLPInfo->type =
		    (enum cxd2880_dvbt2_plp_type)(data[index++] & 0x07);
		pPLPInfo->payload =
		    (enum cxd2880_dvbt2_plp_payload)(data[index++] & 0x1F);
		pPLPInfo->ff = data[index++] & 0x01;
		pPLPInfo->firstRfIdx = data[index++] & 0x07;
		pPLPInfo->firstFrmIdx = data[index++];
		pPLPInfo->groupId = data[index++];
		pPLPInfo->plpCr =
		    (enum cxd2880_dvbt2_plp_code_rate)(data[index++] & 0x07);
		pPLPInfo->constell =
		    (enum cxd2880_dvbt2_plp_constell)(data[index++] & 0x07);
		pPLPInfo->rot = data[index++] & 0x01;
		pPLPInfo->fec =
		    (enum cxd2880_dvbt2_plp_fec)(data[index++] & 0x03);
		pPLPInfo->numBlocksMax = (u16) ((data[index++] & 0x03)) << 8;
		pPLPInfo->numBlocksMax |= data[index++];
		pPLPInfo->frmInt = data[index++];
		pPLPInfo->tilLen = data[index++];
		pPLPInfo->tilType = data[index++] & 0x01;

		pPLPInfo->inBandAFlag = data[index++] & 0x01;
		pPLPInfo->rsvd = data[index++] << 8;
		pPLPInfo->rsvd |= data[index++];

		pPLPInfo->inBandBFlag = (u8) ((pPLPInfo->rsvd & 0x8000) >> 15);
		pPLPInfo->plpMode =
		    (enum cxd2880_dvbt2_plp_mode)((pPLPInfo->rsvd & 0x000C) >>
						  2);
		pPLPInfo->staticFlag = (u8) ((pPLPInfo->rsvd & 0x0002) >> 1);
		pPLPInfo->staticPaddingFlag = (u8) (pPLPInfo->rsvd & 0x0001);
		pPLPInfo->rsvd = (u16) ((pPLPInfo->rsvd & 0x7FF0) >> 4);
	}

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_DataPLPError(struct cxd2880_tnrdmd
						       *pTnrDmd, u8 *pPLPError)
{

	if ((!pTnrDmd) || (!pPLPError))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT2)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (SLVT_FreezeReg(pTnrDmd) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	{
		u8 data;

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x0B) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x86, &data,
				 1) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if ((data & 0x01) == 0x00) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_HW_STATE;
		}

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xC0, &data,
				 1) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		SLVT_UnFreezeReg(pTnrDmd);

		*pPLPError = data & 0x01;
	}

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_L1Change(struct cxd2880_tnrdmd
						   *pTnrDmd, u8 *pL1Change)
{
	u8 data;
	u8 syncState = 0;
	u8 tsLock = 0;
	u8 unlockDetected = 0;
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pL1Change))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT2)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (SLVT_FreezeReg(pTnrDmd) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	ret =
	    cxd2880_tnrdmd_dvbt2_mon_SyncStat(pTnrDmd, &syncState,
					      &tsLock, &unlockDetected);
	if (ret != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	if (syncState < 5) {
		if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
			ret =
			    cxd2880_tnrdmd_dvbt2_mon_SyncStat_sub
			    (pTnrDmd, &syncState, &unlockDetected);
			if (ret != CXD2880_RESULT_OK) {
				SLVT_UnFreezeReg(pTnrDmd);
				return ret;
			}

			if (syncState < 5) {
				SLVT_UnFreezeReg(pTnrDmd);
				return CXD2880_RESULT_ERROR_HW_STATE;
			}
		} else {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_HW_STATE;
		}
	}

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x0B) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x5F, &data,
			 sizeof(data)) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	*pL1Change = data & 0x01;
	if (*pL1Change) {
		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
				     0x00, 0x22) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
				     0x16, 0x01) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}
	}
	SLVT_UnFreezeReg(pTnrDmd);


	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_L1Post(struct cxd2880_tnrdmd *pTnrDmd,
						 struct cxd2880_dvbt2_l1post
						 *pL1Post)
{

	if ((!pTnrDmd) || (!pL1Post))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT2)
		return CXD2880_RESULT_ERROR_SW_STATE;

	{
		u8 data[16];

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x0B) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x86, data,
				 sizeof(data)) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (!(data[0] & 0x01))
			return CXD2880_RESULT_ERROR_HW_STATE;

		pL1Post->subSlicesPerFrame = (data[1] & 0x7F) << 8;
		pL1Post->subSlicesPerFrame |= data[2];
		pL1Post->numPLPs = data[3];
		pL1Post->numAux = data[4] & 0x0F;
		pL1Post->auxCfgRFU = data[5];
		pL1Post->rfIdx = data[6] & 0x07;
		pL1Post->freq = data[7] << 24;
		pL1Post->freq |= data[8] << 16;
		pL1Post->freq |= data[9] << 8;
		pL1Post->freq |= data[10];
		pL1Post->fefType = data[11] & 0x0F;
		pL1Post->fefLength = data[12] << 16;
		pL1Post->fefLength |= data[13] << 8;
		pL1Post->fefLength |= data[14];
		pL1Post->fefIntvl = data[15];
	}

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_BBHeader(struct cxd2880_tnrdmd
						   *pTnrDmd,
						   enum cxd2880_dvbt2_plp_btype
						   type,
						   struct cxd2880_dvbt2_bbheader
						   *pBBHeader)
{

	if ((!pTnrDmd) || (!pBBHeader))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT2)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (SLVT_FreezeReg(pTnrDmd) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	{
		enum cxd2880_ret ret = CXD2880_RESULT_OK;
		u8 syncState = 0;
		u8 tsLock = 0;
		u8 unlockDetected = 0;

		ret =
		    cxd2880_tnrdmd_dvbt2_mon_SyncStat(pTnrDmd, &syncState,
						      &tsLock, &unlockDetected);
		if (ret != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return ret;
		}

		if (!tsLock) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_HW_STATE;
		}
	}

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x0B) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	if (type == CXD2880_DVBT2_PLP_COMMON) {
		u8 l1PostOk;
		u8 data;

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x86,
				 &l1PostOk, 1) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (!(l1PostOk & 0x01)) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_HW_STATE;
		}

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xB6, &data,
				 1) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (data == 0) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_HW_STATE;
		}
	}

	{
		u8 data[14];
		u8 addr = 0;

		if (type == CXD2880_DVBT2_PLP_COMMON)
			addr = 0x51;
		else
			addr = 0x42;

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, addr, data,
				 sizeof(data)) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		SLVT_UnFreezeReg(pTnrDmd);

		pBBHeader->streamInput =
		    (enum cxd2880_dvbt2_stream)((data[0] >> 6) & 0x03);
		pBBHeader->isSingleInputStream = (u8) ((data[0] >> 5) & 0x01);
		pBBHeader->isConstantCodingModulation =
		    (u8) ((data[0] >> 4) & 0x01);
		pBBHeader->issyIndicator = (u8) ((data[0] >> 3) & 0x01);
		pBBHeader->nullPacketDeletion = (u8) ((data[0] >> 2) & 0x01);
		pBBHeader->ext = (u8) (data[0] & 0x03);

		pBBHeader->inputStreamIdentifier = data[1];
		pBBHeader->plpMode =
		    (data[3] & 0x01) ? CXD2880_DVBT2_PLP_MODE_HEM :
		    CXD2880_DVBT2_PLP_MODE_NM;
		pBBHeader->dataFieldLength = (u16) ((data[4] << 8) | data[5]);

		if (pBBHeader->plpMode == CXD2880_DVBT2_PLP_MODE_NM) {
			pBBHeader->userPacketLength =
			    (u16) ((data[6] << 8) | data[7]);
			pBBHeader->syncByte = data[8];
			pBBHeader->issy = 0;
		} else {
			pBBHeader->userPacketLength = 0;
			pBBHeader->syncByte = 0;
			pBBHeader->issy =
			    (u32) ((data[11] << 16) | (data[12] << 8) |
				   data[13]);
		}
	}

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_InBandBTSRate(struct cxd2880_tnrdmd
							*pTnrDmd,
							enum
							cxd2880_dvbt2_plp_btype
							type, u32 *pTSRateBps)
{

	if ((!pTnrDmd) || (!pTSRateBps))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT2)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (SLVT_FreezeReg(pTnrDmd) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	{
		enum cxd2880_ret ret = CXD2880_RESULT_OK;
		u8 syncState = 0;
		u8 tsLock = 0;
		u8 unlockDetected = 0;

		ret =
		    cxd2880_tnrdmd_dvbt2_mon_SyncStat(pTnrDmd, &syncState,
						      &tsLock, &unlockDetected);
		if (ret != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return ret;
		}

		if (!tsLock) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_HW_STATE;
		}
	}

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x0B) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	{
		u8 l1PostOk = 0;
		u8 addr = 0;
		u8 data = 0;

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x86,
				 &l1PostOk, 1) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (!(l1PostOk & 0x01)) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_HW_STATE;
		}

		if (type == CXD2880_DVBT2_PLP_COMMON)
			addr = 0xBA;
		else
			addr = 0xA7;

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, addr, &data,
				 1) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if ((data & 0x80) == 0x00) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_HW_STATE;
		}
	}

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x25) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	{
		u8 data[4];
		u8 addr = 0;

		if (type == CXD2880_DVBT2_PLP_COMMON)
			addr = 0xA6;
		else
			addr = 0xAA;

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, addr,
				 &data[0], 4) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		*pTSRateBps =
		    (u32) (((data[0] & 0x07) << 24) | (data[1] << 16) |
			   (data[2] << 8) | data[3]);
	}

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_SpectrumSense(struct cxd2880_tnrdmd
						*pTnrDmd,
						enum
						cxd2880_tnrdmd_spectrum_sense
						*pSense)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;
	u8 syncState = 0;
	u8 tsLock = 0;
	u8 earlyUnlock = 0;

	if ((!pTnrDmd) || (!pSense))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT2)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (SLVT_FreezeReg(pTnrDmd) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	ret =
	    cxd2880_tnrdmd_dvbt2_mon_SyncStat(pTnrDmd, &syncState, &tsLock,
					      &earlyUnlock);
	if (ret != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return ret;
	}

	if (syncState != 6) {
		SLVT_UnFreezeReg(pTnrDmd);

		ret = CXD2880_RESULT_ERROR_HW_STATE;

		if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN)
			ret =
			    cxd2880_tnrdmd_dvbt2_mon_SpectrumSense(pTnrDmd->
								   pDiverSub,
								   pSense);

		return ret;
	}

	{
		u8 data = 0;

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x0B) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x2F, &data,
				 sizeof(data)) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		SLVT_UnFreezeReg(pTnrDmd);

		*pSense =
		    (data & 0x01) ? CXD2880_TNRDMD_SPECTRUM_INV :
		    CXD2880_TNRDMD_SPECTRUM_NORMAL;
	}

	return CXD2880_RESULT_OK;
}

static enum cxd2880_ret dvbt2_readSNRReg(struct cxd2880_tnrdmd *pTnrDmd,
					 u16 *pRegValue)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pRegValue))
		return CXD2880_RESULT_ERROR_ARG;

	{
		u8 syncState = 0;
		u8 tsLock = 0;
		u8 unlockDetected = 0;
		u8 data[2];

		if (SLVT_FreezeReg(pTnrDmd) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		ret =
		    cxd2880_tnrdmd_dvbt2_mon_SyncStat(pTnrDmd, &syncState,
						      &tsLock, &unlockDetected);
		if (ret != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (syncState != 6) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_HW_STATE;
		}

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x0B) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x13, data,
				 sizeof(data)) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		SLVT_UnFreezeReg(pTnrDmd);

		*pRegValue = (data[0] << 8) | data[1];
	}

	return ret;
}

static enum cxd2880_ret dvbt2_calcSNR(struct cxd2880_tnrdmd *pTnrDmd,
				      u32 regValue, int *pSNR)
{

	if ((!pTnrDmd) || (!pSNR))
		return CXD2880_RESULT_ERROR_ARG;

	if (regValue == 0)
		return CXD2880_RESULT_ERROR_HW_STATE;

	if (regValue > 10876)
		regValue = 10876;

	*pSNR =
	    10 * 10 * ((int)cxd2880_math_log10(regValue) -
		       (int)cxd2880_math_log10(12600 - regValue));
	*pSNR += 32000;

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_SNR(struct cxd2880_tnrdmd *pTnrDmd,
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

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT2)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SINGLE) {
		ret = dvbt2_readSNRReg(pTnrDmd, &regValue);
		if (ret != CXD2880_RESULT_OK)
			return ret;

		ret = dvbt2_calcSNR(pTnrDmd, regValue, pSNR);
		if (ret != CXD2880_RESULT_OK)
			return ret;
	} else {
		int snrMain = 0;
		int snrSub = 0;

		ret =
		    cxd2880_tnrdmd_dvbt2_mon_SNR_diver(pTnrDmd, pSNR, &snrMain,
						       &snrSub);
		if (ret != CXD2880_RESULT_OK)
			return ret;
	}

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_SNR_diver(struct cxd2880_tnrdmd
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

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT2)
		return CXD2880_RESULT_ERROR_SW_STATE;

	ret = dvbt2_readSNRReg(pTnrDmd, &regValue);
	if (ret == CXD2880_RESULT_OK) {
		ret = dvbt2_calcSNR(pTnrDmd, regValue, pSNRMain);
		if (ret != CXD2880_RESULT_OK)
			regValue = 0;
	} else if (ret == CXD2880_RESULT_ERROR_HW_STATE) {
		regValue = 0;
	} else {
		return ret;
	}

	regValueSum += regValue;

	ret = dvbt2_readSNRReg(pTnrDmd->pDiverSub, &regValue);
	if (ret == CXD2880_RESULT_OK) {
		ret = dvbt2_calcSNR(pTnrDmd->pDiverSub, regValue, pSNRSub);
		if (ret != CXD2880_RESULT_OK)
			regValue = 0;
	} else if (ret == CXD2880_RESULT_ERROR_HW_STATE) {
		regValue = 0;
	} else {
		return ret;
	}

	regValueSum += regValue;

	ret = dvbt2_calcSNR(pTnrDmd, regValueSum, pSNR);

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_PreLDPCBER(struct cxd2880_tnrdmd
						     *pTnrDmd, u32 *pBER)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;
	u32 bitError = 0;
	u32 periodExp = 0;
	u32 n_ldpc = 0;

	if ((!pTnrDmd) || (!pBER))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT2)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (SLVT_FreezeReg(pTnrDmd) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x0B) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	{

		u8 data[5];

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x3C, data,
				 sizeof(data)) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (!(data[0] & 0x01)) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_HW_STATE;
		}

		bitError =
		    ((data[1] & 0x0F) << 24) | (data[2] << 16) | (data[3] << 8)
		    | data[4];

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xA0, data,
				 1) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (((enum cxd2880_dvbt2_plp_fec)(data[0] & 0x03)) ==
		    CXD2880_DVBT2_FEC_LDPC_16K)
			n_ldpc = 16200;
		else
			n_ldpc = 64800;

		SLVT_UnFreezeReg(pTnrDmd);

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x20) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x6F, data,
				 1) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		periodExp = data[0] & 0x0F;
	}

	if (bitError > ((1U << periodExp) * n_ldpc))
		return CXD2880_RESULT_ERROR_HW_STATE;

	{
		u32 div = 0;
		u32 Q = 0;
		u32 R = 0;

		if (periodExp >= 4) {

			div = (1U << (periodExp - 4)) * (n_ldpc / 200);

			Q = (bitError * 5) / div;
			R = (bitError * 5) % div;

			R *= 625;
			Q = Q * 625 + R / div;
			R = R % div;
		} else {

			div = (1U << periodExp) * (n_ldpc / 200);

			Q = (bitError * 10) / div;
			R = (bitError * 10) % div;

			R *= 5000;
			Q = Q * 5000 + R / div;
			R = R % div;
		}

		if (div / 2 <= R)
			*pBER = Q + 1;
		else
			*pBER = Q;
	}

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_PostBCHFER(struct cxd2880_tnrdmd
						     *pTnrDmd, u32 *pFER)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;
	u32 fecError = 0;
	u32 period = 0;

	if ((!pTnrDmd) || (!pFER))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT2)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x0B) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	{
		u8 data[2];

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x1B, data,
				 2) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (!(data[0] & 0x80))
			return CXD2880_RESULT_ERROR_HW_STATE;

		fecError = ((data[0] & 0x7F) << 8) | (data[1]);

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x20) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x72, data,
				 1) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		period = (1 << (data[0] & 0x0F));
	}

	if ((period == 0) || (fecError > period))
		return CXD2880_RESULT_ERROR_HW_STATE;

	{
		u32 div = 0;
		u32 Q = 0;
		u32 R = 0;

		div = period;

		Q = (fecError * 1000) / div;
		R = (fecError * 1000) % div;

		R *= 1000;
		Q = Q * 1000 + R / div;
		R = R % div;

		if ((div != 1) && (div / 2 <= R))
			*pFER = Q + 1;
		else
			*pFER = Q;
	}

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_PreBCHBER(struct cxd2880_tnrdmd
						    *pTnrDmd, u32 *pBER)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;
	u32 bitError = 0;
	u32 periodExp = 0;
	u32 n_bch = 0;

	if ((!pTnrDmd) || (!pBER))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT2)
		return CXD2880_RESULT_ERROR_SW_STATE;

	{
		u8 data[3];
		enum cxd2880_dvbt2_plp_fec plpFecType =
		    CXD2880_DVBT2_FEC_LDPC_16K;
		enum cxd2880_dvbt2_plp_code_rate plpCr = CXD2880_DVBT2_R1_2;

		static const u16 nBCHBitsLookup[2][8] = {
			{7200, 9720, 10800, 11880, 12600, 13320, 5400, 6480},
			{32400, 38880, 43200, 48600, 51840, 54000, 21600, 25920}
		};

		if (SLVT_FreezeReg(pTnrDmd) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x0B) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x15, data,
				 3) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (!(data[0] & 0x40)) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_HW_STATE;
		}

		bitError = ((data[0] & 0x3F) << 16) | (data[1] << 8) | data[2];

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x9D, data,
				 1) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		plpCr = (enum cxd2880_dvbt2_plp_code_rate)(data[0] & 0x07);

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xA0, data,
				 1) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		plpFecType = (enum cxd2880_dvbt2_plp_fec)(data[0] & 0x03);

		SLVT_UnFreezeReg(pTnrDmd);

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x20) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x72, data,
				 1) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		periodExp = data[0] & 0x0F;

		if ((plpFecType > CXD2880_DVBT2_FEC_LDPC_64K)
		    || (plpCr > CXD2880_DVBT2_R2_5))
			return CXD2880_RESULT_ERROR_HW_STATE;

		n_bch = nBCHBitsLookup[plpFecType][plpCr];
	}

	if (bitError > ((1U << periodExp) * n_bch))
		return CXD2880_RESULT_ERROR_HW_STATE;

	{
		u32 div = 0;
		u32 Q = 0;
		u32 R = 0;

		if (periodExp >= 6) {

			div = (1U << (periodExp - 6)) * (n_bch / 40);

			Q = (bitError * 625) / div;
			R = (bitError * 625) % div;

			R *= 625;
			Q = Q * 625 + R / div;
			R = R % div;
		} else {

			div = (1U << periodExp) * (n_bch / 40);

			Q = (bitError * 1000) / div;
			R = (bitError * 1000) % div;

			R *= 25000;
			Q = Q * 25000 + R / div;
			R = R % div;
		}

		if (div / 2 <= R)
			*pBER = Q + 1;
		else
			*pBER = Q;
	}

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_PacketErrorNumber(struct
							    cxd2880_tnrdmd
							    *pTnrDmd,
							    u32 *pPEN)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	u8 data[3];

	if ((!pTnrDmd) || (!pPEN))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT2)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x0B) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x39, data,
			 sizeof(data)) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (!(data[0] & 0x01))
		return CXD2880_RESULT_ERROR_HW_STATE;

	*pPEN = ((data[1] << 8) | data[2]);

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_SamplingOffset(struct cxd2880_tnrdmd
							 *pTnrDmd, int *pPPM)
{

	if ((!pTnrDmd) || (!pPPM))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT2)
		return CXD2880_RESULT_ERROR_SW_STATE;

	{
		u8 ctlValReg[5];
		u8 nominalRateReg[5];
		u32 trlCtlVal = 0;
		u32 trcgNominalRate = 0;
		int num;
		int den;
		enum cxd2880_ret ret = CXD2880_RESULT_OK;
		u8 syncState = 0;
		u8 tsLock = 0;
		u8 unlockDetected = 0;
		char diffUpper = 0;

		if (SLVT_FreezeReg(pTnrDmd) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		ret =
		    cxd2880_tnrdmd_dvbt2_mon_SyncStat(pTnrDmd, &syncState,
						      &tsLock, &unlockDetected);
		if (ret != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (syncState != 6) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_HW_STATE;
		}

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x0B) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x34,
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
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x10,
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

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_SamplingOffset_sub(struct
							     cxd2880_tnrdmd
							     *pTnrDmd,
							     int *pPPM)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pPPM))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode != CXD2880_TNRDMD_DIVERMODE_MAIN)
		return CXD2880_RESULT_ERROR_ARG;

	ret = cxd2880_tnrdmd_dvbt2_mon_SamplingOffset(pTnrDmd->pDiverSub, pPPM);

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_Quality(struct cxd2880_tnrdmd
						  *pTnrDmd, u8 *pQuality)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;
	int snr = 0;
	int snrRel = 0;
	u32 ber = 0;
	u32 berSQI = 0;
	enum cxd2880_dvbt2_plp_constell qam;
	enum cxd2880_dvbt2_plp_code_rate codeRate;

	static const int snrNordigP1dB1000[4][8] = {
		{3500, 4700, 5600, 6600, 7200, 7700, 1300, 2200},
		{8700, 10100, 11400, 12500, 13300, 13800, 6000, 7200},
		{13000, 14800, 16200, 17700, 18700, 19400, 9800, 11100},
		{17000, 19400, 20800, 22900, 24300, 25100, 13200, 14800},
	};

	if ((!pTnrDmd) || (!pQuality))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT2)
		return CXD2880_RESULT_ERROR_SW_STATE;

	ret = cxd2880_tnrdmd_dvbt2_mon_PreBCHBER(pTnrDmd, &ber);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	ret = cxd2880_tnrdmd_dvbt2_mon_SNR(pTnrDmd, &snr);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	ret =
	    cxd2880_tnrdmd_dvbt2_mon_QAM(pTnrDmd, CXD2880_DVBT2_PLP_DATA, &qam);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	ret =
	    cxd2880_tnrdmd_dvbt2_mon_CodeRate(pTnrDmd, CXD2880_DVBT2_PLP_DATA,
					      &codeRate);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	if ((codeRate > CXD2880_DVBT2_R2_5) || (qam > CXD2880_DVBT2_QAM256))
		return CXD2880_RESULT_ERROR_OTHER;

	if (ber > 100000)
		berSQI = 0;
	else if (ber >= 100)
		berSQI = 6667;
	else
		berSQI = 16667;

	snrRel = snr - snrNordigP1dB1000[qam][codeRate];

	if (snrRel < -3000) {
		*pQuality = 0;
	} else if (snrRel <= 3000) {
		u32 tempSQI = (((snrRel + 3000) * berSQI) + 500000) / 1000000;
		*pQuality = (tempSQI > 100) ? 100 : (u8) tempSQI;
	} else {
		*pQuality = 100;
	}

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_TSRate(struct cxd2880_tnrdmd *pTnrDmd,
						 u32 *pTSRateKbps)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;
	u32 rd_smooth_dp = 0;
	u32 ep_ck_nume = 0;
	u32 ep_ck_deno = 0;
	u8 issy_on_data = 0;

	if ((!pTnrDmd) || (!pTSRateKbps))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT2)
		return CXD2880_RESULT_ERROR_SW_STATE;

	{
		u8 data[12];
		u8 syncState = 0;
		u8 tsLock = 0;
		u8 unlockDetected = 0;

		if (SLVT_FreezeReg(pTnrDmd) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		ret =
		    cxd2880_tnrdmd_dvbt2_mon_SyncStat(pTnrDmd, &syncState,
						      &tsLock, &unlockDetected);
		if (ret != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (!tsLock) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_HW_STATE;
		}

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x0B) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x23, data,
				 12) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		rd_smooth_dp = (u32) ((data[0] & 0x1F) << 24);
		rd_smooth_dp |= (u32) (data[1] << 16);
		rd_smooth_dp |= (u32) (data[2] << 8);
		rd_smooth_dp |= (u32) data[3];

		if (rd_smooth_dp < 214958) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_HW_STATE;
		}

		ep_ck_nume = (u32) ((data[4] & 0x3F) << 24);
		ep_ck_nume |= (u32) (data[5] << 16);
		ep_ck_nume |= (u32) (data[6] << 8);
		ep_ck_nume |= (u32) data[7];

		ep_ck_deno = (u32) ((data[8] & 0x3F) << 24);
		ep_ck_deno |= (u32) (data[9] << 16);
		ep_ck_deno |= (u32) (data[10] << 8);
		ep_ck_deno |= (u32) data[11];

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x41, data,
				 1) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		issy_on_data = data[0] & 0x01;

		SLVT_UnFreezeReg(pTnrDmd);
	}

	if (issy_on_data) {
		if ((ep_ck_deno == 0) || (ep_ck_nume == 0)
		    || (ep_ck_deno >= ep_ck_nume))
			return CXD2880_RESULT_ERROR_HW_STATE;
	}

	{
		u32 ick_x100;
		u32 div = 0;
		u32 Q = 0;
		u32 R = 0;

		switch (pTnrDmd->clockMode) {
		case CXD2880_TNRDMD_CLOCKMODE_A:
			ick_x100 = 8228;
			break;
		case CXD2880_TNRDMD_CLOCKMODE_B:
			ick_x100 = 9330;
			break;
		case CXD2880_TNRDMD_CLOCKMODE_C:
			ick_x100 = 9600;
			break;
		default:
			return CXD2880_RESULT_ERROR_SW_STATE;
		}

		div = rd_smooth_dp;

		Q = ick_x100 * 262144U / div;
		R = ick_x100 * 262144U % div;

		R *= 5U;
		Q = Q * 5 + R / div;
		R = R % div;

		R *= 2U;
		Q = Q * 2 + R / div;
		R = R % div;

		if (div / 2 <= R)
			*pTSRateKbps = Q + 1;
		else
			*pTSRateKbps = Q;
	}

	if (issy_on_data) {

		u32 diff = ep_ck_nume - ep_ck_deno;

		while (diff > 0x7FFF) {
			diff >>= 1;
			ep_ck_nume >>= 1;
		}

		*pTSRateKbps -=
		    (*pTSRateKbps * diff + ep_ck_nume / 2) / ep_ck_nume;
	}

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_PER(struct cxd2880_tnrdmd *pTnrDmd,
					      u32 *pPER)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;
	u32 packetError = 0;
	u32 period = 0;

	if (!pTnrDmd || !pPER)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;
	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT2)
		return CXD2880_RESULT_ERROR_SW_STATE;

	{
		u8 rdata[3];

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x0B) != CXD2880_RESULT_OK)
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
				     0x24) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xDC, rdata,
				 1) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		period = 1U << (rdata[0] & 0x0F);
	}

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

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_QAM(struct cxd2880_tnrdmd *pTnrDmd,
					      enum cxd2880_dvbt2_plp_btype type,
					      enum cxd2880_dvbt2_plp_constell
					      *pQAM)
{
	u8 data;
	u8 l1PostOk = 0;
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pQAM))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT2)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (SLVT_FreezeReg(pTnrDmd) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x0B) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x86, &l1PostOk,
			 1) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	if (!(l1PostOk & 0x01)) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_HW_STATE;
	}

	if (type == CXD2880_DVBT2_PLP_COMMON) {

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xB6, &data,
				 1) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (data == 0) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_HW_STATE;
		}

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xB1, &data,
				 1) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}
	} else {

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x9E, &data,
				 1) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}
	}

	SLVT_UnFreezeReg(pTnrDmd);

	*pQAM = (enum cxd2880_dvbt2_plp_constell)(data & 0x07);

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_CodeRate(struct cxd2880_tnrdmd
						   *pTnrDmd,
						   enum cxd2880_dvbt2_plp_btype
						   type,
						   enum
						   cxd2880_dvbt2_plp_code_rate
						   *pCodeRate)
{
	u8 data;
	u8 l1PostOk = 0;
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pCodeRate))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT2)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (SLVT_FreezeReg(pTnrDmd) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x0B) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x86, &l1PostOk,
			 1) != CXD2880_RESULT_OK) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_IO;
	}

	if (!(l1PostOk & 0x01)) {
		SLVT_UnFreezeReg(pTnrDmd);
		return CXD2880_RESULT_ERROR_HW_STATE;
	}

	if (type == CXD2880_DVBT2_PLP_COMMON) {

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xB6, &data,
				 1) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}

		if (data == 0) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_HW_STATE;
		}

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xB0, &data,
				 1) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}
	} else {

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x9D, &data,
				 1) != CXD2880_RESULT_OK) {
			SLVT_UnFreezeReg(pTnrDmd);
			return CXD2880_RESULT_ERROR_IO;
		}
	}

	SLVT_UnFreezeReg(pTnrDmd);

	*pCodeRate = (enum cxd2880_dvbt2_plp_code_rate)(data & 0x07);

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_Profile(struct cxd2880_tnrdmd
						  *pTnrDmd,
						  enum cxd2880_dvbt2_profile
						  *pProfile)
{

	if ((!pTnrDmd) || (!pProfile))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT2)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x0B) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	{
		u8 data;

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x22, &data,
				 sizeof(data)) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (data & 0x02) {
			if (data & 0x01)
				*pProfile = CXD2880_DVBT2_PROFILE_LITE;
			else
				*pProfile = CXD2880_DVBT2_PROFILE_BASE;
		} else {
			enum cxd2880_ret ret = CXD2880_RESULT_ERROR_HW_STATE;

			if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN)
				ret =
				    cxd2880_tnrdmd_dvbt2_mon_Profile(pTnrDmd->
								     pDiverSub,
								     pProfile);

			return ret;
		}
	}

	return CXD2880_RESULT_OK;
}

static enum cxd2880_ret dvbt2_CalcSSI(struct cxd2880_tnrdmd *pTnrDmd, int rfLvl,
				      u8 *pSSI)
{
	enum cxd2880_dvbt2_plp_constell qam;
	enum cxd2880_dvbt2_plp_code_rate codeRate;
	int prel;
	int tempSSI = 0;
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	static const int pRefdBm1000[4][8] = {
		{-96000, -95000, -94000, -93000, -92000, -92000, -98000,
		 -97000},
		{-91000, -89000, -88000, -87000, -86000, -86000, -93000,
		 -92000},
		{-86000, -85000, -83000, -82000, -81000, -80000, -89000,
		 -88000},
		{-82000, -80000, -78000, -76000, -75000, -74000, -86000,
		 -84000},
	};

	if ((!pTnrDmd) || (!pSSI))
		return CXD2880_RESULT_ERROR_ARG;

	ret =
	    cxd2880_tnrdmd_dvbt2_mon_QAM(pTnrDmd, CXD2880_DVBT2_PLP_DATA, &qam);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	ret =
	    cxd2880_tnrdmd_dvbt2_mon_CodeRate(pTnrDmd, CXD2880_DVBT2_PLP_DATA,
					      &codeRate);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	if ((codeRate > CXD2880_DVBT2_R2_5) || (qam > CXD2880_DVBT2_QAM256))
		return CXD2880_RESULT_ERROR_OTHER;

	prel = rfLvl - pRefdBm1000[qam][codeRate];

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

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_SSI(struct cxd2880_tnrdmd *pTnrDmd,
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

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT2)
		return CXD2880_RESULT_ERROR_SW_STATE;

	ret = cxd2880_tnrdmd_mon_RFLvl(pTnrDmd, &rfLvl);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	ret = dvbt2_CalcSSI(pTnrDmd, rfLvl, pSSI);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_SSI_sub(struct cxd2880_tnrdmd
						  *pTnrDmd, u8 *pSSI)
{
	int rfLvl = 0;
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pSSI))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode != CXD2880_TNRDMD_DIVERMODE_MAIN)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->sys != CXD2880_DTV_SYS_DVBT2)
		return CXD2880_RESULT_ERROR_SW_STATE;

	ret = cxd2880_tnrdmd_mon_RFLvl(pTnrDmd->pDiverSub, &rfLvl);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	ret = dvbt2_CalcSSI(pTnrDmd, rfLvl, pSSI);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	return ret;
}
