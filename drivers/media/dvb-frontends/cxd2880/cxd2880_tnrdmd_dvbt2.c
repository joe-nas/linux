/*
* cxd2880_tnrdmd_dvbt2.c
* Sony CXD2880 DVB-T2/T tuner + demodulator driver
* The tuner and demodulator control interface implementation for DVB-T2.
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

#include "cxd2880_tnrdmd_dvbt2.h"
#include "cxd2880_tnrdmd_dvbt2_mon.h"

static enum cxd2880_ret X_tune_dvbt2_DemodSetting(struct cxd2880_tnrdmd
						  *pTnrDmd,
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
			     0x02) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x04) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x5D,
			     0x0B) != CXD2880_RESULT_OK)
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

	{
		u8 data[14] = { 0x07, 0x06, 0x01, 0xF0, 0x00, 0x00, 0x04, 0xB0,
				0x00, 0x00, 0x09, 0x9C, 0x0E, 0x4C };

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x20) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x8A,
				     data[0]) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x90,
				     data[1]) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x25) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xF0,
				  &data[2], 2) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x2A) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xDC,
				     data[4]) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xDE,
				     data[5]) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x2D) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x73,
				  &data[6], 4) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x8F,
				  &data[10], 4) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
	}

	{
		u8 dataA_1[9] = { 0x52, 0x49, 0x2C, 0x51, 0x51, 0x3D, 0x15,
				  0x29, 0x0C };
		u8 dataB_1[9] = { 0x5D, 0x55, 0x32, 0x5C, 0x5C, 0x45, 0x17,
				  0x2E, 0x0D };
		u8 dataC_1[9] = { 0x60, 0x00, 0x34, 0x5E, 0x5E, 0x47, 0x18,
				  0x2F, 0x0E };

		u8 dataA_2[13] = { 0x04, 0xE7, 0x94, 0x92, 0x09, 0xCF, 0x7E,
				   0xD0, 0x49, 0xCD, 0xCD, 0x1F, 0x5B };
		u8 dataB_2[13] = { 0x05, 0x90, 0x27, 0x55, 0x0B, 0x20, 0x8F,
				   0xD6, 0xEA, 0xC8, 0xC8, 0x23, 0x91 };
		u8 dataC_2[13] = { 0x05, 0xB8, 0xD8, 0x00, 0x0B, 0x72, 0x93,
				   0xF3, 0x00, 0xCD, 0xCD, 0x24, 0x95 };
		u8 dataA_3[5] = { 0x0B, 0x6A, 0xC9, 0x03, 0x33 };
		u8 dataB_3[5] = { 0x01, 0x02, 0xE4, 0x03, 0x39 };
		u8 dataC_3[5] = { 0x01, 0x02, 0xEB, 0x03, 0x3B };

		u8 *pData_1 = NULL;
		u8 *pData_2 = NULL;
		u8 *pData_3 = NULL;

		switch (clockMode) {
		case CXD2880_TNRDMD_CLOCKMODE_A:
			pData_1 = dataA_1;
			pData_2 = dataA_2;
			pData_3 = dataA_3;
			break;
		case CXD2880_TNRDMD_CLOCKMODE_B:
			pData_1 = dataB_1;
			pData_2 = dataB_2;
			pData_3 = dataB_3;
			break;
		case CXD2880_TNRDMD_CLOCKMODE_C:
			pData_1 = dataC_1;
			pData_2 = dataC_2;
			pData_3 = dataC_3;
			break;
		default:
			return CXD2880_RESULT_ERROR_SW_STATE;
		}

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x04) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x1D,
				  &pData_1[0], 3) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x22,
				     pData_1[3]) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x24,
				     pData_1[4]) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x26,
				     pData_1[5]) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x29,
				  &pData_1[6], 2) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x2D,
				     pData_1[8]) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->diverMode != CXD2880_TNRDMD_DIVERMODE_SUB) {
			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x2E, &pData_2[0],
					  6) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x35, &pData_2[6],
					  7) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->pIo->
		    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x3C,
				  &pData_3[0], 2) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x56,
				  &pData_3[2], 3) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
	}

	switch (bandwidth) {
	case CXD2880_DTV_BW_8_MHZ:

		{
			u8 dataAC[6] = { 0x15, 0x00, 0x00, 0x00, 0x00, 0x00 };
			u8 dataB[6] = { 0x14, 0x6A, 0xAA, 0xAA, 0xAB, 0x00 };
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
					  0x10, pData, 6) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x4A,
				     0x00) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		{
			u8 dataA[2] = { 0x19, 0xD2 };
			u8 dataBC[2] = { 0x3F, 0xFF };
			u8 *pData = NULL;

			switch (clockMode) {
			case CXD2880_TNRDMD_CLOCKMODE_A:
				pData = dataA;
				break;
			case CXD2880_TNRDMD_CLOCKMODE_B:
			case CXD2880_TNRDMD_CLOCKMODE_C:
				pData = dataBC;
				break;
			default:
				return CXD2880_RESULT_ERROR_SW_STATE;
			}

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x19, pData, 2) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		{
			u8 dataA[2] = { 0x06, 0x2A };
			u8 dataB[2] = { 0x06, 0x29 };
			u8 dataC[2] = { 0x06, 0x28 };
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
					  0x1B, pData, 2) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
			u8 dataA[9] = { 0x28, 0x00, 0x50, 0x00, 0x60, 0x00,
					0x00, 0x90, 0x00 };
			u8 dataB[9] = { 0x2D, 0x5E, 0x5A, 0xBD, 0x6C, 0xE3,
					0x00, 0xA3, 0x55 };
			u8 dataC[9] = { 0x2E, 0xAA, 0x5D, 0x55, 0x70, 0x00,
					0x00, 0xA8, 0x00 };
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
					  0x4B, pData, 9) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}
		break;

	case CXD2880_DTV_BW_7_MHZ:

		{
			u8 dataAC[6] = { 0x18, 0x00, 0x00, 0x00, 0x00, 0x00 };
			u8 dataB[6] = { 0x17, 0x55, 0x55, 0x55, 0x55, 0x00 };
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
					  0x10, pData, 6) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x4A,
				     0x02) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		{
			u8 data[2] = { 0x3F, 0xFF };

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x19, data, 2) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		{
			u8 dataA[2] = { 0x06, 0x23 };
			u8 dataB[2] = { 0x06, 0x22 };
			u8 dataC[2] = { 0x06, 0x21 };
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
					  0x1B, pData, 2) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
			u8 dataA[9] = { 0x2D, 0xB6, 0x5B, 0x6D, 0x6D, 0xB6,
					0x00, 0xA4, 0x92 };
			u8 dataB[9] = { 0x33, 0xDA, 0x67, 0xB4, 0x7C, 0x71,
					0x00, 0xBA, 0xAA };
			u8 dataC[9] = { 0x35, 0x55, 0x6A, 0xAA, 0x80, 0x00,
					0x00, 0xC0, 0x00 };
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
					  0x4B, pData, 9) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}
		break;

	case CXD2880_DTV_BW_6_MHZ:

		{
			u8 dataAC[6] = { 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00 };
			u8 dataB[6] = { 0x1B, 0x38, 0xE3, 0x8E, 0x39, 0x00 };
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
					  0x10, pData, 6) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x4A,
				     0x04) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		{
			u8 data[2] = { 0x3F, 0xFF };

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x19, data, 2) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		{
			u8 dataA[2] = { 0x06, 0x1C };
			u8 dataB[2] = { 0x06, 0x1B };
			u8 dataC[2] = { 0x06, 0x1A };
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
					  0x1B, pData, 2) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
			u8 dataA[9] = { 0x35, 0x55, 0x6A, 0xAA, 0x80, 0x00,
					0x00, 0xC0, 0x00 };
			u8 dataB[9] = { 0x3C, 0x7E, 0x78, 0xFC, 0x91, 0x2F,
					0x00, 0xD9, 0xC7 };
			u8 dataC[9] = { 0x3E, 0x38, 0x7C, 0x71, 0x95, 0x55,
					0x00, 0xDF, 0xFF };
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
					  0x4B, pData, 9) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}
		break;

	case CXD2880_DTV_BW_5_MHZ:

		{
			u8 dataAC[6] = { 0x21, 0x99, 0x99, 0x99, 0x9A, 0x00 };
			u8 dataB[6] = { 0x20, 0xAA, 0xAA, 0xAA, 0xAB, 0x00 };
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
					  0x10, pData, 6) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x4A,
				     0x06) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		{
			u8 data[2] = { 0x3F, 0xFF };

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x19, data, 2) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		{
			u8 dataA[2] = { 0x06, 0x15 };
			u8 dataB[2] = { 0x06, 0x15 };
			u8 dataC[2] = { 0x06, 0x14 };
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
					  0x1B, pData, 2) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
			u8 dataA[9] = { 0x40, 0x00, 0x6A, 0xAA, 0x80, 0x00,
					0x00, 0xE6, 0x66 };
			u8 dataB[9] = { 0x48, 0x97, 0x78, 0xFC, 0x91, 0x2F,
					0x01, 0x05, 0x55 };
			u8 dataC[9] = { 0x4A, 0xAA, 0x7C, 0x71, 0x95, 0x55,
					0x01, 0x0C, 0xCC };
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
					  0x4B, pData, 9) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}
		break;

	case CXD2880_DTV_BW_1_7_MHZ:

		{
			u8 dataA[6] = { 0x68, 0x0F, 0xA2, 0x32, 0xCF, 0x03 };
			u8 dataC[6] = { 0x68, 0x0F, 0xA2, 0x32, 0xCF, 0x03 };
			u8 dataB[6] = { 0x65, 0x2B, 0xA4, 0xCD, 0xD8, 0x03 };
			u8 *pData = NULL;

			switch (clockMode) {
			case CXD2880_TNRDMD_CLOCKMODE_A:
				pData = dataA;
				break;
			case CXD2880_TNRDMD_CLOCKMODE_C:
				pData = dataC;
				break;
			case CXD2880_TNRDMD_CLOCKMODE_B:
				pData = dataB;
				break;
			default:
				return CXD2880_RESULT_ERROR_SW_STATE;
			}

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x10, pData, 6) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x4A,
				     0x03) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		{
			u8 data[2] = { 0x3F, 0xFF };

			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					  0x19, data, 2) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		{
			u8 dataA[2] = { 0x06, 0x0C };
			u8 dataB[2] = { 0x06, 0x0C };
			u8 dataC[2] = { 0x06, 0x0B };
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
					  0x1B, pData, 2) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
			u8 dataA[9] = { 0x40, 0x00, 0x6A, 0xAA, 0x80, 0x00,
					0x02, 0xC9, 0x8F };
			u8 dataB[9] = { 0x48, 0x97, 0x78, 0xFC, 0x91, 0x2F,
					0x03, 0x29, 0x5D };
			u8 dataC[9] = { 0x4A, 0xAA, 0x7C, 0x71, 0x95, 0x55,
					0x03, 0x40, 0x7D };
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
					  0x4B, pData, 9) != CXD2880_RESULT_OK)
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

static enum cxd2880_ret X_sleep_dvbt2_DemodSetting(struct cxd2880_tnrdmd
						   *pTnrDmd)
{

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
		u8 data[] = { 0, 1, 0, 2, 0, 4, 0, 8, 0, 16, 0, 32 };

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x1D) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x47, data,
				  12) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
	}

	return CXD2880_RESULT_OK;
}

static enum cxd2880_ret dvbt2_SetProfile(struct cxd2880_tnrdmd *pTnrDmd,
					 enum cxd2880_dvbt2_profile profile)
{
	u8 t2Mode_tuneMode = 0;
	u8 seqNot2Dtime = 0;
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	{
		u8 dtime1 = 0;
		u8 dtime2 = 0;

		switch (pTnrDmd->clockMode) {
		case CXD2880_TNRDMD_CLOCKMODE_A:
			dtime1 = 0x27;
			dtime2 = 0x0C;
			break;
		case CXD2880_TNRDMD_CLOCKMODE_B:
			dtime1 = 0x2C;
			dtime2 = 0x0D;
			break;
		case CXD2880_TNRDMD_CLOCKMODE_C:
			dtime1 = 0x2E;
			dtime2 = 0x0E;
			break;
		default:
			return CXD2880_RESULT_ERROR_SW_STATE;
		}

		switch (profile) {
		case CXD2880_DVBT2_PROFILE_BASE:
			t2Mode_tuneMode = 0x01;
			seqNot2Dtime = dtime2;
			break;

		case CXD2880_DVBT2_PROFILE_LITE:
			t2Mode_tuneMode = 0x05;
			seqNot2Dtime = dtime1;
			break;

		case CXD2880_DVBT2_PROFILE_ANY:
			t2Mode_tuneMode = 0x00;
			seqNot2Dtime = dtime1;
			break;

		default:
			return CXD2880_RESULT_ERROR_ARG;
		}
	}

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x2E) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x10,
			     t2Mode_tuneMode) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x04) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x2C,
			     seqNot2Dtime) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_Tune1(struct cxd2880_tnrdmd *pTnrDmd,
					    struct cxd2880_dvbt2_tune_param
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

	if ((pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN)
	    && (pTuneParam->profile == CXD2880_DVBT2_PROFILE_ANY))
		return CXD2880_RESULT_ERROR_NOSUPPORT;

	ret =
	    cxd2880_tnrdmd_CommonTuneSetting1(pTnrDmd, CXD2880_DTV_SYS_DVBT2,
					      pTuneParam->centerFreqKHz,
					      pTuneParam->bandwidth, 0, 0);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	ret =
	    X_tune_dvbt2_DemodSetting(pTnrDmd, pTuneParam->bandwidth,
				      pTnrDmd->clockMode);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
		ret =
		    X_tune_dvbt2_DemodSetting(pTnrDmd->pDiverSub,
					      pTuneParam->bandwidth,
					      pTnrDmd->pDiverSub->clockMode);
		if (ret != CXD2880_RESULT_OK)
			return ret;
	}

	ret = dvbt2_SetProfile(pTnrDmd, pTuneParam->profile);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
		ret = dvbt2_SetProfile(pTnrDmd->pDiverSub, pTuneParam->profile);
		if (ret != CXD2880_RESULT_OK)
			return ret;
	}

	if (pTuneParam->dataPLPID == CXD2880_DVBT2_TUNE_PARAM_PLPID_AUTO) {
		ret = cxd2880_tnrdmd_dvbt2_SetPLPCfg(pTnrDmd, 1, 0);
		if (ret != CXD2880_RESULT_OK)
			return ret;
	} else {
		ret =
		    cxd2880_tnrdmd_dvbt2_SetPLPCfg(pTnrDmd, 0,
						   (u8) (pTuneParam->
							 dataPLPID));
		if (ret != CXD2880_RESULT_OK)
			return ret;
	}

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_Tune2(struct cxd2880_tnrdmd *pTnrDmd,
					    struct cxd2880_dvbt2_tune_param
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

	{
		u8 enFEFIntmtntCtrl = 1;

		switch (pTuneParam->profile) {
		case CXD2880_DVBT2_PROFILE_BASE:
			enFEFIntmtntCtrl = pTnrDmd->enFEFIntmtntBase;
			break;
		case CXD2880_DVBT2_PROFILE_LITE:
			enFEFIntmtntCtrl = pTnrDmd->enFEFIntmtntLite;
			break;
		case CXD2880_DVBT2_PROFILE_ANY:
			if (pTnrDmd->enFEFIntmtntBase
			    && pTnrDmd->enFEFIntmtntLite)
				enFEFIntmtntCtrl = 1;
			else
				enFEFIntmtntCtrl = 0;
			break;
		default:
			return CXD2880_RESULT_ERROR_ARG;
		}

		ret =
		    cxd2880_tnrdmd_CommonTuneSetting2(pTnrDmd,
						      CXD2880_DTV_SYS_DVBT2,
						      enFEFIntmtntCtrl);
		if (ret != CXD2880_RESULT_OK)
			return ret;
	}

	pTnrDmd->state = CXD2880_TNRDMD_STATE_ACTIVE;
	pTnrDmd->frequencyKHz = pTuneParam->centerFreqKHz;
	pTnrDmd->sys = CXD2880_DTV_SYS_DVBT2;
	pTnrDmd->bandwidth = pTuneParam->bandwidth;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
		pTnrDmd->pDiverSub->state = CXD2880_TNRDMD_STATE_ACTIVE;
		pTnrDmd->pDiverSub->frequencyKHz = pTuneParam->centerFreqKHz;
		pTnrDmd->pDiverSub->sys = CXD2880_DTV_SYS_DVBT2;
		pTnrDmd->pDiverSub->bandwidth = pTuneParam->bandwidth;
	}

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_SleepSetting(struct cxd2880_tnrdmd
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

	ret = X_sleep_dvbt2_DemodSetting(pTnrDmd);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
		ret = X_sleep_dvbt2_DemodSetting(pTnrDmd->pDiverSub);
		if (ret != CXD2880_RESULT_OK)
			return ret;
	}

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_CheckDemodLock(struct cxd2880_tnrdmd
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
	    cxd2880_tnrdmd_dvbt2_mon_SyncStat(pTnrDmd, &syncStat, &tsLock,
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
	    cxd2880_tnrdmd_dvbt2_mon_SyncStat_sub(pTnrDmd, &syncStat,
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

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_CheckTSLock(struct cxd2880_tnrdmd
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
	    cxd2880_tnrdmd_dvbt2_mon_SyncStat(pTnrDmd, &syncStat, &tsLock,
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
	    cxd2880_tnrdmd_dvbt2_mon_SyncStat_sub(pTnrDmd, &syncStat,
						  &unlockDetectedSub);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	if (unlockDetected && unlockDetectedSub)
		*pLock = CXD2880_TNRDMD_LOCK_RESULT_UNLOCKED;
	else
		*pLock = CXD2880_TNRDMD_LOCK_RESULT_NOTDETECT;

	return ret;

}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_SetPLPCfg(struct cxd2880_tnrdmd *pTnrDmd,
						u8 autoPLP, u8 plpId)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if ((pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
	    && (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE))
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x23) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (!autoPLP) {

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xAF,
				     plpId) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
	}

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xAD,
			     autoPLP ? 0x00 : 0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_DiverFEFSetting(struct cxd2880_tnrdmd
						      *pTnrDmd)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SINGLE)
		return CXD2880_RESULT_OK;

	{
		struct cxd2880_dvbt2_ofdm ofdm;

		ret = cxd2880_tnrdmd_dvbt2_mon_OFDM(pTnrDmd, &ofdm);
		if (ret != CXD2880_RESULT_OK)
			return ret;

		if (!ofdm.mixed)
			return CXD2880_RESULT_OK;
	}

	{
		u8 data[] = { 0, 8, 0, 16, 0, 32, 0, 64, 0, 128, 1, 0 };

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0x1D) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x47, data,
				  12) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
	}

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_CheckL1PostValid(struct cxd2880_tnrdmd
						       *pTnrDmd,
						       u8 *pL1PostValid)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	u8 data;

	if ((!pTnrDmd) || (!pL1PostValid))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if ((pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
	    && (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE))
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x0B) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x86, &data,
			 1) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	*pL1PostValid = data & 0x01;

	return ret;
}
