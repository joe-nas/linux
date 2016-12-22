/*
* cxd2880_tnrdmd_dvbt.c
* Sony CXD2880 DVB-T2/T tuner + demodulator driver
* The tuner and demodulator control interface implementation for DVB-T
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

#include "cxd2880_tnrdmd_dvbt.h"
#include "cxd2880_tnrdmd_dvbt_mon.h"

static enum cxd2880_ret X_tune_dvbt_DemodSetting(struct cxd2880_tnrdmd *pTnrDmd,
						 enum cxd2880_dtv_bandwidth
						 bandwidth,
						 enum cxd2880_tnrdmd_clockmode
						 clockMode)
{

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x31,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	{
		u8 dataA[2] = { 0x52, 0x49 };
		u8 dataB[2] = { 0x5D, 0x55 };
		u8 dataC[2] = { 0x60, 0x00 };
		u8 *pData = NULL;

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x04) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		switch (clockMode) {
		case CXD2880_TNRDMD_CLOCKMODE_A:
			pData = dataA;
			break;
		case CXD2880_TNRDMD_CLOCKMODE_B:
			pData = dataB;
			break;
		case CXD2880_TNRDMD_CLOCKMODE_C:
			pData = dataC;
			break;
		default:
			return CXD2880_RESULT_ERROR_SW_STATE;
		}

		if (pTnrDmd->pIo->
		    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x65, pData,
				  2) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
	}

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x5D,
			     0x07) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->diverMode != CXD2880_TNRDMD_DIVERMODE_SUB) {
		u8 data[2] = { 0x01, 0x01 };

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x00) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xCE, data,
				  2) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
	}

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x04) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x5C,
			     0xFB) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x10) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xA4,
			     0x03) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x14) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xB0,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x25) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	{
		u8 data[2] = { 0x01, 0xF0 };

		if (pTnrDmd->pIo->
		    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xF0, data,
				  2) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
	}

	if ((pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN)
	    || (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)) {
		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x12) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x44,
				     0x00) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
	}

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB) {
		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x11) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x87,
				     0xD2) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
	}

	if (pTnrDmd->diverMode != CXD2880_TNRDMD_DIVERMODE_SUB) {
		u8 dataA[3] = { 0x73, 0xCA, 0x49 };
		u8 dataB[3] = { 0xC8, 0x13, 0xAA };
		u8 dataC[3] = { 0xDC, 0x6C, 0x00 };
		u8 *pData = NULL;

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x04) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		switch (clockMode) {
		case CXD2880_TNRDMD_CLOCKMODE_A:
			pData = dataA;
			break;
		case CXD2880_TNRDMD_CLOCKMODE_B:
			pData = dataB;
			break;
		case CXD2880_TNRDMD_CLOCKMODE_C:
			pData = dataC;
			break;
		default:
			return CXD2880_RESULT_ERROR_SW_STATE;
		}

		if (pTnrDmd->pIo->
		    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x68, pData,
				  3) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
	}

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x04) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	switch (bandwidth) {
	case CXD2880_DTV_BW_8_MHZ:

		{
			u8 dataAC[5] = { 0x15, 0x00, 0x00, 0x00, 0x00 };
			u8 dataB[5] = { 0x14, 0x6A, 0xAA, 0xAA, 0xAA };
			u8 *pData = NULL;

			switch (clockMode) {
			case CXD2880_TNRDMD_CLOCKMODE_A:
			case CXD2880_TNRDMD_CLOCKMODE_C:
				pData = dataAC;
				break;
			case CXD2880_TNRDMD_CLOCKMODE_B:
				pData = dataB;
				break;
			default:
				return CXD2880_RESULT_ERROR_SW_STATE;
			}

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x60, pData, 5) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x4A,
				     0x00) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		{
			u8 dataA[2] = { 0x01, 0x28 };
			u8 dataB[2] = { 0x11, 0x44 };
			u8 dataC[2] = { 0x15, 0x28 };
			u8 *pData = NULL;

			switch (clockMode) {
			case CXD2880_TNRDMD_CLOCKMODE_A:
				pData = dataA;
				break;
			case CXD2880_TNRDMD_CLOCKMODE_B:
				pData = dataB;
				break;
			case CXD2880_TNRDMD_CLOCKMODE_C:
				pData = dataC;
				break;
			default:
				return CXD2880_RESULT_ERROR_SW_STATE;
			}

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x7D, pData, 2) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		{
			u8 data = 0;

			switch (clockMode) {
			case CXD2880_TNRDMD_CLOCKMODE_A:
			case CXD2880_TNRDMD_CLOCKMODE_B:
				data = 0x35;
				break;
			case CXD2880_TNRDMD_CLOCKMODE_C:
				data = 0x34;
				break;
			default:
				return CXD2880_RESULT_ERROR_SW_STATE;
			}

			if (pTnrDmd->pIo->
			    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					     0x71, data) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
			u8 dataA[5] = { 0x30, 0x00, 0x00, 0x90, 0x00 };
			u8 dataB[5] = { 0x36, 0x71, 0x00, 0xA3, 0x55 };
			u8 dataC[5] = { 0x38, 0x00, 0x00, 0xA8, 0x00 };
			u8 *pData = NULL;

			switch (clockMode) {
			case CXD2880_TNRDMD_CLOCKMODE_A:
				pData = dataA;
				break;
			case CXD2880_TNRDMD_CLOCKMODE_B:
				pData = dataB;
				break;
			case CXD2880_TNRDMD_CLOCKMODE_C:
				pData = dataC;
				break;
			default:
				return CXD2880_RESULT_ERROR_SW_STATE;
			}

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x4B, &pData[0],
					  2) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x51, &pData[2],
					  3) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		{
			u8 data[4] = { 0xB3, 0x00, 0x01, 0x02 };

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x72, &data[0],
					  2) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x6B, &data[2],
					  2) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}
		break;

	case CXD2880_DTV_BW_7_MHZ:

		{
			u8 dataAC[5] = { 0x18, 0x00, 0x00, 0x00, 0x00 };
			u8 dataB[5] = { 0x17, 0x55, 0x55, 0x55, 0x55 };
			u8 *pData = NULL;

			switch (clockMode) {
			case CXD2880_TNRDMD_CLOCKMODE_A:
			case CXD2880_TNRDMD_CLOCKMODE_C:
				pData = dataAC;
				break;
			case CXD2880_TNRDMD_CLOCKMODE_B:
				pData = dataB;
				break;
			default:
				return CXD2880_RESULT_ERROR_SW_STATE;
			}

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x60, pData, 5) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x4A,
				     0x02) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		{
			u8 dataA[2] = { 0x12, 0x4C };
			u8 dataB[2] = { 0x1F, 0x15 };
			u8 dataC[2] = { 0x1F, 0xF8 };
			u8 *pData = NULL;

			switch (clockMode) {
			case CXD2880_TNRDMD_CLOCKMODE_A:
				pData = dataA;
				break;
			case CXD2880_TNRDMD_CLOCKMODE_B:
				pData = dataB;
				break;
			case CXD2880_TNRDMD_CLOCKMODE_C:
				pData = dataC;
				break;
			default:
				return CXD2880_RESULT_ERROR_SW_STATE;
			}

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x7D, pData, 2) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		{
			u8 data = 0;

			switch (clockMode) {
			case CXD2880_TNRDMD_CLOCKMODE_A:
			case CXD2880_TNRDMD_CLOCKMODE_B:
				data = 0x2F;
				break;
			case CXD2880_TNRDMD_CLOCKMODE_C:
				data = 0x2E;
				break;
			default:
				return CXD2880_RESULT_ERROR_SW_STATE;
			}

			if (pTnrDmd->pIo->
			    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					     0x71, data) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
			u8 dataA[5] = { 0x36, 0xDB, 0x00, 0xA4, 0x92 };
			u8 dataB[5] = { 0x3E, 0x38, 0x00, 0xBA, 0xAA };
			u8 dataC[5] = { 0x40, 0x00, 0x00, 0xC0, 0x00 };
			u8 *pData = NULL;

			switch (clockMode) {
			case CXD2880_TNRDMD_CLOCKMODE_A:
				pData = dataA;
				break;
			case CXD2880_TNRDMD_CLOCKMODE_B:
				pData = dataB;
				break;
			case CXD2880_TNRDMD_CLOCKMODE_C:
				pData = dataC;
				break;
			default:
				return CXD2880_RESULT_ERROR_SW_STATE;
			}

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x4B, &pData[0],
					  2) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x51, &pData[2],
					  3) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		{
			u8 data[4] = { 0xB8, 0x00, 0x00, 0x03 };

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x72, &data[0],
					  2) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x6B, &data[2],
					  2) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}
		break;

	case CXD2880_DTV_BW_6_MHZ:

		{
			u8 dataAC[5] = { 0x1C, 0x00, 0x00, 0x00, 0x00 };
			u8 dataB[5] = { 0x1B, 0x38, 0xE3, 0x8E, 0x38 };
			u8 *pData = NULL;

			switch (clockMode) {
			case CXD2880_TNRDMD_CLOCKMODE_A:
			case CXD2880_TNRDMD_CLOCKMODE_C:
				pData = dataAC;
				break;
			case CXD2880_TNRDMD_CLOCKMODE_B:
				pData = dataB;
				break;
			default:
				return CXD2880_RESULT_ERROR_SW_STATE;
			}

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x60, pData, 5) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x4A,
				     0x04) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		{
			u8 dataA[2] = { 0x1F, 0xF8 };
			u8 dataB[2] = { 0x24, 0x43 };
			u8 dataC[2] = { 0x25, 0x4C };
			u8 *pData = NULL;

			switch (clockMode) {
			case CXD2880_TNRDMD_CLOCKMODE_A:
				pData = dataA;
				break;
			case CXD2880_TNRDMD_CLOCKMODE_B:
				pData = dataB;
				break;
			case CXD2880_TNRDMD_CLOCKMODE_C:
				pData = dataC;
				break;
			default:
				return CXD2880_RESULT_ERROR_SW_STATE;
			}

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x7D, pData, 2) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		{
			u8 data = 0;

			switch (clockMode) {
			case CXD2880_TNRDMD_CLOCKMODE_A:
			case CXD2880_TNRDMD_CLOCKMODE_C:
				data = 0x29;
				break;
			case CXD2880_TNRDMD_CLOCKMODE_B:
				data = 0x2A;
				break;
			default:
				return CXD2880_RESULT_ERROR_SW_STATE;
			}

			if (pTnrDmd->pIo->
			    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					     0x71, data) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
			u8 dataA[5] = { 0x40, 0x00, 0x00, 0xC0, 0x00 };
			u8 dataB[5] = { 0x48, 0x97, 0x00, 0xD9, 0xC7 };
			u8 dataC[5] = { 0x4A, 0xAA, 0x00, 0xDF, 0xFF };
			u8 *pData = NULL;

			switch (clockMode) {
			case CXD2880_TNRDMD_CLOCKMODE_A:
				pData = dataA;
				break;
			case CXD2880_TNRDMD_CLOCKMODE_B:
				pData = dataB;
				break;
			case CXD2880_TNRDMD_CLOCKMODE_C:
				pData = dataC;
				break;
			default:
				return CXD2880_RESULT_ERROR_SW_STATE;
			}

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x4B, &pData[0],
					  2) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x51, &pData[2],
					  3) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		{
			u8 data[4] = { 0xBE, 0xAB, 0x00, 0x03 };

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x72, &data[0],
					  2) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x6B, &data[2],
					  2) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}
		break;

	case CXD2880_DTV_BW_5_MHZ:

		{
			u8 dataAC[5] = { 0x21, 0x99, 0x99, 0x99, 0x99 };
			u8 dataB[5] = { 0x20, 0xAA, 0xAA, 0xAA, 0xAA };
			u8 *pData = NULL;

			switch (clockMode) {
			case CXD2880_TNRDMD_CLOCKMODE_A:
			case CXD2880_TNRDMD_CLOCKMODE_C:
				pData = dataAC;
				break;
			case CXD2880_TNRDMD_CLOCKMODE_B:
				pData = dataB;
				break;
			default:
				return CXD2880_RESULT_ERROR_SW_STATE;
			}

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x60, pData, 5) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x4A,
				     0x06) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		{
			u8 dataA[2] = { 0x26, 0x5D };
			u8 dataB[2] = { 0x2B, 0x84 };
			u8 dataC[2] = { 0x2C, 0xC2 };
			u8 *pData = NULL;

			switch (clockMode) {
			case CXD2880_TNRDMD_CLOCKMODE_A:
				pData = dataA;
				break;
			case CXD2880_TNRDMD_CLOCKMODE_B:
				pData = dataB;
				break;
			case CXD2880_TNRDMD_CLOCKMODE_C:
				pData = dataC;
				break;
			default:
				return CXD2880_RESULT_ERROR_SW_STATE;
			}

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x7D, pData, 2) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		{
			u8 data = 0;

			switch (clockMode) {
			case CXD2880_TNRDMD_CLOCKMODE_A:
			case CXD2880_TNRDMD_CLOCKMODE_B:
				data = 0x24;
				break;
			case CXD2880_TNRDMD_CLOCKMODE_C:
				data = 0x23;
				break;
			default:
				return CXD2880_RESULT_ERROR_SW_STATE;
			}

			if (pTnrDmd->pIo->
			    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					     0x71, data) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
			u8 dataA[5] = { 0x4C, 0xCC, 0x00, 0xE6, 0x66 };
			u8 dataB[5] = { 0x57, 0x1C, 0x01, 0x05, 0x55 };
			u8 dataC[5] = { 0x59, 0x99, 0x01, 0x0C, 0xCC };
			u8 *pData = NULL;

			switch (clockMode) {
			case CXD2880_TNRDMD_CLOCKMODE_A:
				pData = dataA;
				break;
			case CXD2880_TNRDMD_CLOCKMODE_B:
				pData = dataB;
				break;
			case CXD2880_TNRDMD_CLOCKMODE_C:
				pData = dataC;
				break;
			default:
				return CXD2880_RESULT_ERROR_SW_STATE;
			}

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x4B, &pData[0],
					  2) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x51, &pData[2],
					  3) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		{
			u8 data[4] = { 0xC8, 0x01, 0x00, 0x03 };

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x72, &data[0],
					  2) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x6B, &data[2],
					  2) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}
		break;

	default:
		return CXD2880_RESULT_ERROR_SW_STATE;
	}

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xFD,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	return CXD2880_RESULT_OK;
}

static enum cxd2880_ret X_sleep_dvbt_DemodSetting(struct cxd2880_tnrdmd
						  *pTnrDmd)
{

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x04) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x5C,
			     0xD8) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x10) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xA4,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB) {
		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x11) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x87,
				     0x04) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
	}

	return CXD2880_RESULT_OK;
}

static enum cxd2880_ret dvbt_SetProfile(struct cxd2880_tnrdmd *pTnrDmd,
					enum cxd2880_dvbt_profile profile)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x10) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x67,
			     (profile ==
			      CXD2880_DVBT_PROFILE_HP) ? 0x00 : 0x01) !=
	    CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt_Tune1(struct cxd2880_tnrdmd *pTnrDmd,
					   struct cxd2880_dvbt_tune_param
					   *pTuneParam)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pTuneParam))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if ((pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
	    && (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE))
		return CXD2880_RESULT_ERROR_SW_STATE;

	ret =
	    cxd2880_tnrdmd_CommonTuneSetting1(pTnrDmd, CXD2880_DTV_SYS_DVBT,
					      pTuneParam->centerFreqKHz,
					      pTuneParam->bandwidth, 0, 0);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	ret =
	    X_tune_dvbt_DemodSetting(pTnrDmd, pTuneParam->bandwidth,
				     pTnrDmd->clockMode);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
		ret =
		    X_tune_dvbt_DemodSetting(pTnrDmd->pDiverSub,
					     pTuneParam->bandwidth,
					     pTnrDmd->pDiverSub->clockMode);
		if (ret != CXD2880_RESULT_OK)
			return ret;
	}

	ret = dvbt_SetProfile(pTnrDmd, pTuneParam->profile);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt_Tune2(struct cxd2880_tnrdmd *pTnrDmd,
					   struct cxd2880_dvbt_tune_param
					   *pTuneParam)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pTuneParam))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if ((pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
	    && (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE))
		return CXD2880_RESULT_ERROR_SW_STATE;

	ret =
	    cxd2880_tnrdmd_CommonTuneSetting2(pTnrDmd, CXD2880_DTV_SYS_DVBT, 0);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	pTnrDmd->state = CXD2880_TNRDMD_STATE_ACTIVE;
	pTnrDmd->frequencyKHz = pTuneParam->centerFreqKHz;
	pTnrDmd->sys = CXD2880_DTV_SYS_DVBT;
	pTnrDmd->bandwidth = pTuneParam->bandwidth;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
		pTnrDmd->pDiverSub->state = CXD2880_TNRDMD_STATE_ACTIVE;
		pTnrDmd->pDiverSub->frequencyKHz = pTuneParam->centerFreqKHz;
		pTnrDmd->pDiverSub->sys = CXD2880_DTV_SYS_DVBT;
		pTnrDmd->pDiverSub->bandwidth = pTuneParam->bandwidth;
	}

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt_SleepSetting(struct cxd2880_tnrdmd
						  *pTnrDmd)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if ((pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
	    && (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE))
		return CXD2880_RESULT_ERROR_SW_STATE;

	ret = X_sleep_dvbt_DemodSetting(pTnrDmd);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
		ret = X_sleep_dvbt_DemodSetting(pTnrDmd->pDiverSub);
		if (ret != CXD2880_RESULT_OK)
			return ret;
	}

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt_CheckDemodLock(struct cxd2880_tnrdmd
						    *pTnrDmd,
						    enum
						    cxd2880_tnrdmd_lock_result
						    *pLock)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	u8 syncStat = 0;
	u8 tsLock = 0;
	u8 unlockDetected = 0;
	u8 unlockDetectedSub = 0;

	if ((!pTnrDmd) || (!pLock))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	ret =
	    cxd2880_tnrdmd_dvbt_mon_SyncStat(pTnrDmd, &syncStat, &tsLock,
					     &unlockDetected);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SINGLE) {
		if (syncStat == 6)
			*pLock = CXD2880_TNRDMD_LOCK_RESULT_LOCKED;
		else if (unlockDetected)
			*pLock = CXD2880_TNRDMD_LOCK_RESULT_UNLOCKED;
		else
			*pLock = CXD2880_TNRDMD_LOCK_RESULT_NOTDETECT;

		return ret;
	}

	if (syncStat == 6) {
		*pLock = CXD2880_TNRDMD_LOCK_RESULT_LOCKED;
		return ret;
	}

	ret =
	    cxd2880_tnrdmd_dvbt_mon_SyncStat_sub(pTnrDmd, &syncStat,
						 &unlockDetectedSub);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	if (syncStat == 6)
		*pLock = CXD2880_TNRDMD_LOCK_RESULT_LOCKED;
	else if (unlockDetected && unlockDetectedSub)
		*pLock = CXD2880_TNRDMD_LOCK_RESULT_UNLOCKED;
	else
		*pLock = CXD2880_TNRDMD_LOCK_RESULT_NOTDETECT;

	return ret;

}

enum cxd2880_ret cxd2880_tnrdmd_dvbt_CheckTSLock(struct cxd2880_tnrdmd *pTnrDmd,
						 enum cxd2880_tnrdmd_lock_result
						 *pLock)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	u8 syncStat = 0;
	u8 tsLock = 0;
	u8 unlockDetected = 0;
	u8 unlockDetectedSub = 0;

	if ((!pTnrDmd) || (!pLock))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	ret =
	    cxd2880_tnrdmd_dvbt_mon_SyncStat(pTnrDmd, &syncStat, &tsLock,
					     &unlockDetected);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SINGLE) {
		if (tsLock)
			*pLock = CXD2880_TNRDMD_LOCK_RESULT_LOCKED;
		else if (unlockDetected)
			*pLock = CXD2880_TNRDMD_LOCK_RESULT_UNLOCKED;
		else
			*pLock = CXD2880_TNRDMD_LOCK_RESULT_NOTDETECT;

		return ret;
	}

	if (tsLock) {
		*pLock = CXD2880_TNRDMD_LOCK_RESULT_LOCKED;
		return ret;
	} else if (!unlockDetected) {
		*pLock = CXD2880_TNRDMD_LOCK_RESULT_NOTDETECT;
		return ret;
	}

	ret =
	    cxd2880_tnrdmd_dvbt_mon_SyncStat_sub(pTnrDmd, &syncStat,
						 &unlockDetectedSub);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	if (unlockDetected && unlockDetectedSub)
		*pLock = CXD2880_TNRDMD_LOCK_RESULT_UNLOCKED;
	else
		*pLock = CXD2880_TNRDMD_LOCK_RESULT_NOTDETECT;

	return ret;

}
