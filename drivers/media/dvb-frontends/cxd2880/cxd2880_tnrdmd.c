/*
* cxd2880_tnrdmd.c
* Sony CXD2880 DVB-T2/T tuner + demodulator driver
* The common tuner and demodulator control interface implementation.
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

#include "cxd2880_common.h"
#include "cxd2880_stdlib.h"
#include "cxd2880_tnrdmd.h"
#include "cxd2880_tnrdmd_mon.h"
#include "cxd2880_tnrdmd_dvbt.h"
#include "cxd2880_tnrdmd_dvbt2.h"

static enum cxd2880_ret P_init1(struct cxd2880_tnrdmd *pTnrDmd)
{
	u8 data = 0;

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if ((pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SINGLE)
	    || (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN)) {
		switch (pTnrDmd->createParam.tsOutputIF) {
		case CXD2880_TNRDMD_TSOUT_IF_TS:
			data = 0x00;
			break;
		case CXD2880_TNRDMD_TSOUT_IF_SPI:
			data = 0x01;
			break;
		case CXD2880_TNRDMD_TSOUT_IF_SDIO:
			data = 0x02;
			break;
		default:
			return CXD2880_RESULT_ERROR_ARG;
		}
		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x10,
				     data) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
	}

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x11,
			     0x16) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x10) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	switch (pTnrDmd->chipID) {
	case CXD2880_TNRDMD_CHIP_ID_CXD2880_ES1_0X:
		data = 0x1A;
		break;
	case CXD2880_TNRDMD_CHIP_ID_CXD2880_ES1_11:
		data = 0x16;
		break;
	default:
		return CXD2880_RESULT_ERROR_NOSUPPORT;
	}

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x10,
			     data) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->createParam.enInternalLDO)
		data = 0x01;
	else
		data = 0x00;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x11,
			     data) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x13,
			     data) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x12,
			     data) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x10) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	switch (pTnrDmd->chipID) {
	case CXD2880_TNRDMD_CHIP_ID_CXD2880_ES1_0X:
		data = 0x01;
		break;
	case CXD2880_TNRDMD_CHIP_ID_CXD2880_ES1_11:
		data = 0x00;
		break;
	default:
		return CXD2880_RESULT_ERROR_NOSUPPORT;
	}

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x69,
			     data) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	return CXD2880_RESULT_OK;
}

static enum cxd2880_ret P_init2(struct cxd2880_tnrdmd *pTnrDmd)
{
	u8 data[6] = { 0 };

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	data[0] = pTnrDmd->createParam.xosc_cap;
	data[1] = pTnrDmd->createParam.xosc_i;
	switch (pTnrDmd->createParam.xtalShareType) {
	case CXD2880_TNRDMD_XTAL_SHARE_NONE:
		data[2] = 0x01;
		data[3] = 0x00;
		break;
	case CXD2880_TNRDMD_XTAL_SHARE_EXTREF:
		data[2] = 0x00;
		data[3] = 0x00;
		break;
	case CXD2880_TNRDMD_XTAL_SHARE_MASTER:
		data[2] = 0x01;
		data[3] = 0x01;
		break;
	case CXD2880_TNRDMD_XTAL_SHARE_SLAVE:
		data[2] = 0x00;
		data[3] = 0x01;
		break;
	default:
		return CXD2880_RESULT_ERROR_ARG;
	}
	data[4] = 0x06;
	data[5] = 0x00;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x13, data,
			  6) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	return CXD2880_RESULT_OK;
}

static enum cxd2880_ret P_init3(struct cxd2880_tnrdmd *pTnrDmd)
{
	u8 data[2] = { 0 };

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	switch (pTnrDmd->diverMode) {
	case CXD2880_TNRDMD_DIVERMODE_SINGLE:
		data[0] = 0x00;
		break;
	case CXD2880_TNRDMD_DIVERMODE_MAIN:
		data[0] = 0x03;
		break;
	case CXD2880_TNRDMD_DIVERMODE_SUB:
		data[0] = 0x02;
		break;
	default:
		return CXD2880_RESULT_ERROR_ARG;
	}

	data[1] = 0x01;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x1F, data,
			  2) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	return CXD2880_RESULT_OK;
}

static enum cxd2880_ret RF_init1(struct cxd2880_tnrdmd *pTnrDmd)
{
	u8 data[80] = { 0 };

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	data[0] = 0x01;
	data[1] = 0x00;
	data[2] = 0x01;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x21, data,
			  3) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x10) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	data[0] = 0x01;
	data[1] = 0x01;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x17, data,
			  2) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->createParam.stationaryUse) {
		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x1A,
				     0x06) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
	}

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x4F,
			     0x18) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x61,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x71,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x9D,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x7D,
			     0x02) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x8F,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x8B,
			     0xC6) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x9A,
			     0x03) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x1C,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	data[0] = 0x00;
	if ((pTnrDmd->createParam.isCXD2881GG)
	    && (pTnrDmd->createParam.xtalShareType ==
		CXD2880_TNRDMD_XTAL_SHARE_SLAVE))
		data[1] = 0x00;
	else
		data[1] = 0x1F;
	data[2] = 0x0A;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0xB5, data,
			  3) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0xB9,
			     0x07) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x33,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0xC1,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0xC4,
			     0x1E) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->chipID == CXD2880_TNRDMD_CHIP_ID_CXD2880_ES1_0X) {
		data[0] = 0x34;
		data[1] = 0x2C;
	} else {
		data[0] = 0x2F;
		data[1] = 0x25;
	}
	data[2] = 0x15;
	data[3] = 0x19;
	data[4] = 0x1B;
	data[5] = 0x15;
	data[6] = 0x19;
	data[7] = 0x1B;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0xD9, data,
			  8) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x11) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	data[0] = 0x6C;
	data[1] = 0x10;
	data[2] = 0xA6;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x44, data,
			  3) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	data[0] = 0x16;
	data[1] = 0xA8;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x50, data,
			  2) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	data[0] = 0x00;
	data[1] = 0x22;
	data[2] = 0x00;
	data[3] = 0x88;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x62, data,
			  4) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x74,
			     0x75) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	data[0] = 0x05;
	data[1] = 0x05;
	data[2] = 0x05;
	data[3] = 0x05;
	data[4] = 0x05;
	data[5] = 0x05;
	data[6] = 0x05;
	data[7] = 0x05;
	data[8] = 0x05;
	data[9] = 0x04;
	data[10] = 0x04;
	data[11] = 0x04;
	data[12] = 0x03;
	data[13] = 0x03;
	data[14] = 0x03;
	data[15] = 0x04;
	data[16] = 0x04;
	data[17] = 0x05;
	data[18] = 0x05;
	data[19] = 0x05;
	data[20] = 0x02;
	data[21] = 0x02;
	data[22] = 0x02;
	data[23] = 0x02;
	data[24] = 0x02;
	data[25] = 0x02;
	data[26] = 0x02;
	data[27] = 0x02;
	data[28] = 0x02;
	data[29] = 0x03;
	data[30] = 0x02;
	data[31] = 0x01;
	data[32] = 0x01;
	data[33] = 0x01;
	data[34] = 0x02;
	data[35] = 0x02;
	data[36] = 0x03;
	data[37] = 0x04;
	data[38] = 0x04;
	data[39] = 0x04;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x7F, data,
			  40) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x16) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	data[0] = 0x00;
	data[1] = 0x71;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x10, data,
			  2) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x23,
			     0x89) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	data[0] = 0xFF;
	data[1] = 0x00;
	data[2] = 0x00;
	data[3] = 0x00;
	data[4] = 0x00;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x27, data,
			  5) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	data[0] = 0x00;
	data[1] = 0x00;
	data[2] = 0x00;
	data[3] = 0x00;
	data[4] = 0x00;
	data[5] = 0x01;
	data[6] = 0x00;
	data[7] = 0x01;
	data[8] = 0x00;
	data[9] = 0x02;
	data[10] = 0x00;
	data[11] = 0x63;
	data[12] = 0x00;
	data[13] = 0x00;
	data[14] = 0x00;
	data[15] = 0x03;
	data[16] = 0x00;
	data[17] = 0x04;
	data[18] = 0x00;
	data[19] = 0x04;
	data[20] = 0x00;
	data[21] = 0x06;
	data[22] = 0x00;
	data[23] = 0x06;
	data[24] = 0x00;
	data[25] = 0x08;
	data[26] = 0x00;
	data[27] = 0x09;
	data[28] = 0x00;
	data[29] = 0x0B;
	data[30] = 0x00;
	data[31] = 0x0B;
	data[32] = 0x00;
	data[33] = 0x0D;
	data[34] = 0x00;
	data[35] = 0x0D;
	data[36] = 0x00;
	data[37] = 0x0F;
	data[38] = 0x00;
	data[39] = 0x0F;
	data[40] = 0x00;
	data[41] = 0x0F;
	data[42] = 0x00;
	data[43] = 0x10;
	data[44] = 0x00;
	data[45] = 0x79;
	data[46] = 0x00;
	data[47] = 0x00;
	data[48] = 0x00;
	data[49] = 0x02;
	data[50] = 0x00;
	data[51] = 0x00;
	data[52] = 0x00;
	data[53] = 0x03;
	data[54] = 0x00;
	data[55] = 0x01;
	data[56] = 0x00;
	data[57] = 0x03;
	data[58] = 0x00;
	data[59] = 0x03;
	data[60] = 0x00;
	data[61] = 0x03;
	data[62] = 0x00;
	data[63] = 0x04;
	data[64] = 0x00;
	data[65] = 0x04;
	data[66] = 0x00;
	data[67] = 0x06;
	data[68] = 0x00;
	data[69] = 0x05;
	data[70] = 0x00;
	data[71] = 0x07;
	data[72] = 0x00;
	data[73] = 0x07;
	data[74] = 0x00;
	data[75] = 0x08;
	data[76] = 0x00;
	data[77] = 0x0A;
	data[78] = 0x03;
	data[79] = 0xE0;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x3A, data,
			  80) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	data[0] = 0x03;
	data[1] = 0xE0;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0xBC, data,
			  2) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x10) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x51,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0xC5,
			     0x07) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x11) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x70,
			     0xE9) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x76,
			     0x0A) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x78,
			     0x32) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x7A,
			     0x46) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x7C,
			     0x86) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x7E,
			     0xA4) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x10) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0xE1,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	{
		u8 addr = 0;

		if (pTnrDmd->createParam.stationaryUse) {
			data[0] = 0x06;
			data[1] = 0x07;
			data[2] = 0x1A;
		} else {
			data[0] = 0x00;
			data[1] = 0x08;
			data[2] = 0x19;
		}
		data[3] = 0x0E;
		data[4] = 0x09;
		data[5] = 0x0E;

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
				     0x12) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
		for (addr = 0x10; addr < 0x9F; addr += 6) {
			if (pTnrDmd->pLNAThrsTableAir) {
				data[0] =
				    pTnrDmd->pLNAThrsTableAir->
				    thrs[(addr - 0x10) / 6].off_on;
				data[1] =
				    pTnrDmd->pLNAThrsTableAir->
				    thrs[(addr - 0x10) / 6].on_off;
			}
			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS,
					  addr, data, 6) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}

		data[0] = 0x00;
		data[1] = 0x08;
		if (pTnrDmd->createParam.stationaryUse)
			data[2] = 0x1A;
		else
			data[2] = 0x19;
		data[3] = 0x0E;
		data[4] = 0x09;
		data[5] = 0x0E;

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
				     0x13) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
		for (addr = 0x10; addr < 0xCF; addr += 6) {
			if (pTnrDmd->pLNAThrsTableCable) {
				data[0] =
				    pTnrDmd->pLNAThrsTableCable->
				    thrs[(addr - 0x10) / 6].off_on;
				data[1] =
				    pTnrDmd->pLNAThrsTableCable->
				    thrs[(addr - 0x10) / 6].on_off;
			}
			if (pTnrDmd->pIo->
			    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS,
					  addr, data, 6) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}
	}

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x11) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	data[0] = 0x08;
	data[1] = 0x09;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0xBD, data,
			  2) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	data[0] = 0x08;
	data[1] = 0x09;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0xC4, data,
			  2) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	data[0] = 0x20;
	data[1] = 0x20;
	data[2] = 0x30;
	data[3] = 0x41;
	data[4] = 0x50;
	data[5] = 0x5F;
	data[6] = 0x6F;
	data[7] = 0x80;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0xC9, data,
			  8) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x14) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	data[0] = 0x15;
	data[1] = 0x18;
	data[2] = 0x00;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x10, data,
			  3) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x15,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x16) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	data[0] = 0x00;
	data[1] = 0x09;
	data[2] = 0x00;
	data[3] = 0x08;
	data[4] = 0x00;
	data[5] = 0x07;
	data[6] = 0x00;
	data[7] = 0x06;
	data[8] = 0x00;
	data[9] = 0x05;
	data[10] = 0x00;
	data[11] = 0x03;
	data[12] = 0x00;
	data[13] = 0x02;
	data[14] = 0x00;
	data[15] = 0x00;
	data[16] = 0x00;
	data[17] = 0x78;
	data[18] = 0x00;
	data[19] = 0x00;
	data[20] = 0x00;
	data[21] = 0x06;
	data[22] = 0x00;
	data[23] = 0x08;
	data[24] = 0x00;
	data[25] = 0x08;
	data[26] = 0x00;
	data[27] = 0x0C;
	data[28] = 0x00;
	data[29] = 0x0C;
	data[30] = 0x00;
	data[31] = 0x0D;
	data[32] = 0x00;
	data[33] = 0x0F;
	data[34] = 0x00;
	data[35] = 0x0E;
	data[36] = 0x00;
	data[37] = 0x0E;
	data[38] = 0x00;
	data[39] = 0x10;
	data[40] = 0x00;
	data[41] = 0x0F;
	data[42] = 0x00;
	data[43] = 0x0E;
	data[44] = 0x00;
	data[45] = 0x10;
	data[46] = 0x00;
	data[47] = 0x0F;
	data[48] = 0x00;
	data[49] = 0x0E;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x12, data,
			  50) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	CXD2880_SLEEP(1);

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x0A) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x10, data,
			 1) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if ((data[0] & 0x01) == 0x00)
		return CXD2880_RESULT_ERROR_HW_STATE;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x25,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	CXD2880_SLEEP(1);

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x0A) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x11, data,
			 1) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if ((data[0] & 0x01) == 0x00)
		return CXD2880_RESULT_ERROR_HW_STATE;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x02,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x21,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0xE1) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x8F,
			     0x16) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x67,
			     0x60) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x6A,
			     0x0F) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x6C,
			     0x17) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	data[0] = 0x00;
	data[1] = 0xFE;
	data[2] = 0xEE;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x6E, data,
			  3) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	data[0] = 0xA1;
	data[1] = 0x8B;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x8D, data,
			  2) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	data[0] = 0x08;
	data[1] = 0x09;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x77, data,
			  2) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->createParam.stationaryUse) {
		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x80,
				     0xAA) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
	}

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0xE2) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x41,
			     0xA0) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x4B,
			     0x68) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x21,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x10,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x10) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x25,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	CXD2880_SLEEP(1);

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x1A) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x10, data,
			 1) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if ((data[0] & 0x01) == 0x00)
		return CXD2880_RESULT_ERROR_HW_STATE;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x10) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x14,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x26,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	return CXD2880_RESULT_OK;
}

static enum cxd2880_ret RF_init2(struct cxd2880_tnrdmd *pTnrDmd)
{
	u8 data[5] = { 0 };

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x10) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	data[0] = 0x40;
	data[1] = 0x40;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0xEA, data,
			  2) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	CXD2880_SLEEP(1);

	data[0] = 0x00;
	if (pTnrDmd->chipID == CXD2880_TNRDMD_CHIP_ID_CXD2880_ES1_0X)
		data[1] = 0x00;
	else
		data[1] = 0x01;
	data[2] = 0x01;
	data[3] = 0x03;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x30, data,
			  4) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x14) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x1B,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x21,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0xE1) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xD3,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x21,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	return CXD2880_RESULT_OK;
}

static enum cxd2880_ret X_tune1(struct cxd2880_tnrdmd *pTnrDmd,
				enum cxd2880_dtv_sys sys, u32 freqKHz,
				enum cxd2880_dtv_bandwidth bandwidth,
				u8 isCable, int shiftFrequencyKHz)
{
	u8 data[11] = { 0 };

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x10,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x10) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	data[0] = 0x00;
	data[1] = 0x00;
	data[2] = 0x0E;
	data[3] = 0x00;
	data[4] = 0x03;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0xE7, data,
			  5) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	data[0] = 0x1F;
	data[1] = 0x80;
	data[2] = 0x18;
	data[3] = 0x00;
	data[4] = 0x07;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0xE7, data,
			  5) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	CXD2880_SLEEP(1);

	data[0] = 0x72;
	data[1] = 0x81;
	data[3] = 0x1D;
	data[4] = 0x6F;
	data[5] = 0x7E;
	data[7] = 0x1C;
	switch (sys) {
	case CXD2880_DTV_SYS_DVBT:
	case CXD2880_DTV_SYS_ISDBT:
	case CXD2880_DTV_SYS_ISDBTSB:
	case CXD2880_DTV_SYS_ISDBTMM_A:
	case CXD2880_DTV_SYS_ISDBTMM_B:
		data[2] = 0x94;
		data[6] = 0x91;
		break;
	case CXD2880_DTV_SYS_DVBT2:
		data[2] = 0x96;
		data[6] = 0x93;
		break;
	default:
		return CXD2880_RESULT_ERROR_ARG;
	}
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x44, data,
			  8) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x62,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x15) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	data[0] = 0x03;
	data[1] = 0xE2;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x1E, data,
			  2) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x10) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	data[0] = (u8) (isCable ? 0x01 : 0x00);
	data[1] = 0x00;
	data[2] = 0x6B;
	data[3] = 0x4D;

	switch (bandwidth) {
	case CXD2880_DTV_BW_1_7_MHZ:
		data[4] = 0x03;
		break;
	case CXD2880_DTV_BW_5_MHZ:
	case CXD2880_DTV_BW_6_MHZ:
		data[4] = 0x00;
		break;
	case CXD2880_DTV_BW_7_MHZ:
		data[4] = 0x01;
		break;
	case CXD2880_DTV_BW_8_MHZ:
		data[4] = 0x02;
		break;
	default:
		return CXD2880_RESULT_ERROR_ARG;
	}

	data[5] = 0x00;

	freqKHz += shiftFrequencyKHz;

	data[6] = (u8) ((freqKHz >> 16) & 0x0F);
	data[7] = (u8) ((freqKHz >> 8) & 0xFF);
	data[8] = (u8) (freqKHz & 0xFF);
	data[9] = 0xFF;
	data[10] = 0xFE;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x52, data,
			  11) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	return CXD2880_RESULT_OK;
}

static enum cxd2880_ret X_tune2(struct cxd2880_tnrdmd *pTnrDmd,
				enum cxd2880_dtv_bandwidth bandwidth,
				enum cxd2880_tnrdmd_clockmode clockMode,
				int shiftFrequencyKHz)
{
	u8 data[3] = { 0 };

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x11) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	data[0] = 0x01;
	data[1] = 0x0E;
	data[2] = 0x01;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x2D, data,
			  3) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x1A) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x29,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x2C, data,
			 1) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x10) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x60,
			     data[0]) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x62,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x11) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x2D,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x2F,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x10,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x21,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (shiftFrequencyKHz != 0) {
		int shiftFreq = 0;

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0xE1) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x60, data,
				 2) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		shiftFreq = shiftFrequencyKHz * 1000;

		switch (clockMode) {
		case CXD2880_TNRDMD_CLOCKMODE_A:
		case CXD2880_TNRDMD_CLOCKMODE_C:
		default:
			if (shiftFreq >= 0)
				shiftFreq = (shiftFreq + 183 / 2) / 183;
			else
				shiftFreq = (shiftFreq - 183 / 2) / 183;
			break;
		case CXD2880_TNRDMD_CLOCKMODE_B:
			if (shiftFreq >= 0)
				shiftFreq = (shiftFreq + 178 / 2) / 178;
			else
				shiftFreq = (shiftFreq - 178 / 2) / 178;
			break;
		}

		shiftFreq +=
		    cxd2880_Convert2SComplement((data[0] << 8) | data[1], 16);

		if (shiftFreq > 32767)
			shiftFreq = 32767;
		else if (shiftFreq < -32768)
			shiftFreq = -32768;

		data[0] = (u8) (((u32) shiftFreq >> 8) & 0xFF);
		data[1] = (u8) ((u32) shiftFreq & 0xFF);

		if (pTnrDmd->pIo->
		    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x60, data,
				  2) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x69, data,
				 1) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		shiftFreq = -shiftFrequencyKHz;

		if (bandwidth == CXD2880_DTV_BW_1_7_MHZ) {
			switch (clockMode) {
			case CXD2880_TNRDMD_CLOCKMODE_A:
			case CXD2880_TNRDMD_CLOCKMODE_C:
			default:
				if (shiftFreq >= 0)
					shiftFreq =
					    (shiftFreq * 1000 +
					     17578 / 2) / 17578;
				else
					shiftFreq =
					    (shiftFreq * 1000 -
					     17578 / 2) / 17578;
				break;
			case CXD2880_TNRDMD_CLOCKMODE_B:
				if (shiftFreq >= 0)
					shiftFreq =
					    (shiftFreq * 1000 +
					     17090 / 2) / 17090;
				else
					shiftFreq =
					    (shiftFreq * 1000 -
					     17090 / 2) / 17090;
				break;
			}
		} else {
			switch (clockMode) {
			case CXD2880_TNRDMD_CLOCKMODE_A:
			case CXD2880_TNRDMD_CLOCKMODE_C:
			default:
				if (shiftFreq >= 0)
					shiftFreq =
					    (shiftFreq * 1000 +
					     35156 / 2) / 35156;
				else
					shiftFreq =
					    (shiftFreq * 1000 -
					     35156 / 2) / 35156;
				break;
			case CXD2880_TNRDMD_CLOCKMODE_B:
				if (shiftFreq >= 0)
					shiftFreq =
					    (shiftFreq * 1000 +
					     34180 / 2) / 34180;
				else
					shiftFreq =
					    (shiftFreq * 1000 -
					     34180 / 2) / 34180;
				break;
			}
		}

		shiftFreq += cxd2880_Convert2SComplement(data[0], 8);

		if (shiftFreq > 127)
			shiftFreq = 127;
		else if (shiftFreq < -128)
			shiftFreq = -128;

		data[0] = (u8) ((u32) shiftFreq & 0xFF);

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x69,
				     data[0]) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
	}

	if (pTnrDmd->createParam.stationaryUse) {
		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
				     0xE1) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x8A,
				     0x87) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
	}

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x21,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	return CXD2880_RESULT_OK;
}

static enum cxd2880_ret X_tune3(struct cxd2880_tnrdmd *pTnrDmd,
				enum cxd2880_dtv_sys sys, u8 enFEFIntmtntCtrl)
{
	u8 data[6] = { 0 };

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x21,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0xE2) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x41,
			     0xA0) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x21,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xFE,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x10) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if ((sys == CXD2880_DTV_SYS_DVBT2) && enFEFIntmtntCtrl) {
		data[0] = 0x01;
		data[1] = 0x01;
		data[2] = 0x01;
		data[3] = 0x01;
		data[4] = 0x01;
		data[5] = 0x01;
	} else {
		data[0] = 0x00;
		data[1] = 0x00;
		data[2] = 0x00;
		data[3] = 0x00;
		data[4] = 0x00;
		data[5] = 0x00;
	}
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0xEF, data,
			  6) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x2D) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if ((sys == CXD2880_DTV_SYS_DVBT2) && enFEFIntmtntCtrl)
		data[0] = 0x00;
	else
		data[0] = 0x01;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xB1,
			     data[0]) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	return CXD2880_RESULT_OK;
}

static enum cxd2880_ret X_tune4(struct cxd2880_tnrdmd *pTnrDmd)
{
	u8 data[2] = { 0 };

	if (pTnrDmd->diverMode != CXD2880_TNRDMD_DIVERMODE_MAIN)
		return CXD2880_RESULT_ERROR_ARG;

	{
		if (pTnrDmd->pDiverSub->pIo->
		    WriteOneRegister(pTnrDmd->pDiverSub->pIo,
				     CXD2880_IO_TGT_SYS, 0x00,
				     0x00) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
		data[0] = 0x14;
		data[1] = 0x00;
		if (pTnrDmd->pDiverSub->pIo->
		    WriteRegister(pTnrDmd->pDiverSub->pIo, CXD2880_IO_TGT_SYS,
				  0x55, data, 2) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
	}

	{
		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
				     0x00) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
		data[0] = 0x0B;
		data[1] = 0xFF;
		if (pTnrDmd->pIo->
		    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x53, data,
				  2) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x57,
				     0x01) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
		data[0] = 0x0B;
		data[1] = 0xFF;
		if (pTnrDmd->pIo->
		    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x55, data,
				  2) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
	}

	{
		if (pTnrDmd->pDiverSub->pIo->
		    WriteOneRegister(pTnrDmd->pDiverSub->pIo,
				     CXD2880_IO_TGT_SYS, 0x00,
				     0x00) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
		data[0] = 0x14;
		data[1] = 0x00;
		if (pTnrDmd->pDiverSub->pIo->
		    WriteRegister(pTnrDmd->pDiverSub->pIo, CXD2880_IO_TGT_SYS,
				  0x53, data, 2) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
		if (pTnrDmd->pDiverSub->pIo->
		    WriteOneRegister(pTnrDmd->pDiverSub->pIo,
				     CXD2880_IO_TGT_SYS, 0x57,
				     0x02) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
	}

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xFE,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pDiverSub->pIo->
	    WriteOneRegister(pTnrDmd->pDiverSub->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pDiverSub->pIo->
	    WriteOneRegister(pTnrDmd->pDiverSub->pIo, CXD2880_IO_TGT_DMD, 0xFE,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	return CXD2880_RESULT_OK;
}

static enum cxd2880_ret X_sleep1(struct cxd2880_tnrdmd *pTnrDmd)
{
	u8 data[3] = { 0 };

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode != CXD2880_TNRDMD_DIVERMODE_MAIN)
		return CXD2880_RESULT_ERROR_ARG;

	{
		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
				     0x00) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x57,
				     0x03) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
		data[0] = 0x00;
		data[1] = 0x00;
		if (pTnrDmd->pIo->
		    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x53, data,
				  2) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
	}

	{
		if (pTnrDmd->pDiverSub->pIo->
		    WriteOneRegister(pTnrDmd->pDiverSub->pIo,
				     CXD2880_IO_TGT_SYS, 0x00,
				     0x00) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
		data[0] = 0x1F;
		data[1] = 0xFF;
		data[2] = 0x03;
		if (pTnrDmd->pDiverSub->pIo->
		    WriteRegister(pTnrDmd->pDiverSub->pIo, CXD2880_IO_TGT_SYS,
				  0x55, data, 3) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
		data[0] = 0x00;
		data[1] = 0x00;
		if (pTnrDmd->pDiverSub->pIo->
		    WriteRegister(pTnrDmd->pDiverSub->pIo, CXD2880_IO_TGT_SYS,
				  0x53, data, 2) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
	}

	{
		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
				     0x00) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
		data[0] = 0x1F;
		data[1] = 0xFF;
		if (pTnrDmd->pIo->
		    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x55, data,
				  2) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
	}

	return CXD2880_RESULT_OK;
}

static enum cxd2880_ret X_sleep2(struct cxd2880_tnrdmd *pTnrDmd)
{
	u8 data = 0;

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x2D) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xB1,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	CXD2880_SLEEP(1);

	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xB2, &data,
			 1) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if ((data & 0x01) == 0x00)
		return CXD2880_RESULT_ERROR_HW_STATE;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x10) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0xF4,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0xF3,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0xF2,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0xF1,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0xF0,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0xEF,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	return CXD2880_RESULT_OK;
}

static enum cxd2880_ret X_sleep3(struct cxd2880_tnrdmd *pTnrDmd)
{

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xFD,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	return CXD2880_RESULT_OK;
}

static enum cxd2880_ret X_sleep4(struct cxd2880_tnrdmd *pTnrDmd)
{

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x21,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0xE2) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x41,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x21,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	return CXD2880_RESULT_OK;
}

static enum cxd2880_ret SPLL_reset(struct cxd2880_tnrdmd *pTnrDmd,
				   enum cxd2880_tnrdmd_clockmode clockmode)
{
	u8 data[4] = { 0 };

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x10) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x29,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x28,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x27,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x26,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x10,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x27,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x22,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	switch (clockmode) {
	case CXD2880_TNRDMD_CLOCKMODE_A:
		data[0] = 0x00;
		break;

	case CXD2880_TNRDMD_CLOCKMODE_B:
		data[0] = 0x01;
		break;

	case CXD2880_TNRDMD_CLOCKMODE_C:
		data[0] = 0x02;
		break;

	default:
		return CXD2880_RESULT_ERROR_ARG;
	}
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x30,
			     data[0]) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x22,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	CXD2880_SLEEP(2);

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x0A) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x10, data,
			 1) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if ((data[0] & 0x01) == 0x00)
		return CXD2880_RESULT_ERROR_HW_STATE;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x27,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	CXD2880_SLEEP(1);

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x10,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x10) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	data[0] = 0x00;
	data[1] = 0x00;
	data[2] = 0x00;
	data[3] = 0x00;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x26, data,
			  4) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	return CXD2880_RESULT_OK;
}

static enum cxd2880_ret T_powerX(struct cxd2880_tnrdmd *pTnrDmd, u8 on)
{
	u8 data[3] = { 0 };

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x10) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x29,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x28,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x27,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x10,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x27,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x25,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (on) {
		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x2B,
				     0x01) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		CXD2880_SLEEP(1);

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
				     0x0A) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x12, data,
				 1) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
		if ((data[0] & 0x01) == 0)
			return CXD2880_RESULT_ERROR_HW_STATE;
		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
				     0x00) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x2A,
				     0x00) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
	} else {
		data[0] = 0x03;
		data[1] = 0x00;
		if (pTnrDmd->pIo->
		    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x2A, data,
				  2) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		CXD2880_SLEEP(1);

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
				     0x0A) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x13, data,
				 1) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
		if ((data[0] & 0x01) == 0)
			return CXD2880_RESULT_ERROR_HW_STATE;
	}

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x25,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	CXD2880_SLEEP(1);

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x0A) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x11, data,
			 1) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if ((data[0] & 0x01) == 0)
		return CXD2880_RESULT_ERROR_HW_STATE;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x27,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	CXD2880_SLEEP(1);

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x10,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x10) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	data[0] = 0x00;
	data[1] = 0x00;
	data[2] = 0x00;
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x27, data,
			  3) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	return CXD2880_RESULT_OK;
}

struct cxd2880_tnrdmd_ts_clk_cfg {
	u8 serialClkMode;
	u8 serialDutyMode;
	u8 tsClkPeriod;
};

static enum cxd2880_ret setTSClockModeAndFreq(struct cxd2880_tnrdmd *pTnrDmd,
					      enum cxd2880_dtv_sys sys)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;
	u8 backwardsCompatible = 0;
	struct cxd2880_tnrdmd_ts_clk_cfg tsClkCfg;

	const struct cxd2880_tnrdmd_ts_clk_cfg serialTsClkSettings[2][6] = { {
	 {3, 1, 8,},
	 {0, 2, 16,}
	},
	{
	 {1, 1, 8,},
	 {2, 2, 16,}
	 }
	};

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	{
		u8 tsRateCtrlOff = 0;
		u8 tsInOff = 0;
		u8 tsClkManaulOn = 0;

		if ((sys == CXD2880_DTV_SYS_ISDBT)
		    || (sys == CXD2880_DTV_SYS_ISDBTSB)
		    || (sys == CXD2880_DTV_SYS_ISDBTMM_A)
		    || (sys == CXD2880_DTV_SYS_ISDBTMM_B)) {
			backwardsCompatible = 0;
			tsRateCtrlOff = 1;
			tsInOff = 0;
		} else if (pTnrDmd->isTsBackwardsCompatibleMode) {
			backwardsCompatible = 1;
			tsRateCtrlOff = 1;
			tsInOff = 1;
		} else {
			backwardsCompatible = 0;
			tsRateCtrlOff = 0;
			tsInOff = 0;
		}

		if (pTnrDmd->tsByteClkManualSetting) {
			tsClkManaulOn = 1;
			tsRateCtrlOff = 0;
		}

		ret =
		    cxd2880_io_SetRegisterBits(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					       0xD3, tsRateCtrlOff, 0x01);
		if (ret != CXD2880_RESULT_OK)
			return ret;

		ret =
		    cxd2880_io_SetRegisterBits(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					       0xDE, tsInOff, 0x01);
		if (ret != CXD2880_RESULT_OK)
			return ret;

		ret =
		    cxd2880_io_SetRegisterBits(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					       0xDA, tsClkManaulOn, 0x01);
		if (ret != CXD2880_RESULT_OK)
			return ret;
	}

	tsClkCfg =
	    serialTsClkSettings[pTnrDmd->
				serialTsClockModeContinuous][(u8) pTnrDmd->
							     serialTsClkFreq];

	if (pTnrDmd->tsByteClkManualSetting)
		tsClkCfg.tsClkPeriod = pTnrDmd->tsByteClkManualSetting;

	ret =
	    cxd2880_io_SetRegisterBits(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xC4,
				       tsClkCfg.serialClkMode, 0x03);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	ret =
	    cxd2880_io_SetRegisterBits(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0xD1,
				       tsClkCfg.serialDutyMode, 0x03);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	ret =
	    pTnrDmd->pIo->WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					   0xD9, tsClkCfg.tsClkPeriod);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	{
		u8 data = (u8) (backwardsCompatible ? 0x00 : 0x01);

		if (sys == CXD2880_DTV_SYS_DVBT) {
			if (pTnrDmd->pIo->
			    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					     0x00, 0x10) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;

			ret =
			    cxd2880_io_SetRegisterBits(pTnrDmd->pIo,
						       CXD2880_IO_TGT_DMD, 0x66,
						       data, 0x01);
			if (ret != CXD2880_RESULT_OK)
				return ret;
		}
	}

	return ret;
}

static enum cxd2880_ret clearCfgMemory(struct cxd2880_tnrdmd *pTnrDmd)
{

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	pTnrDmd->cfgMemoryLastEntry = 0;
	cxd2880_memset(pTnrDmd->cfgMemory, 0x00, sizeof(pTnrDmd->cfgMemory));

	return CXD2880_RESULT_OK;
}

static enum cxd2880_ret loadCfgMemory(struct cxd2880_tnrdmd *pTnrDmd)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;
	u8 i;

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	for (i = 0; i < pTnrDmd->cfgMemoryLastEntry; i++) {
		ret = pTnrDmd->pIo->WriteOneRegister(pTnrDmd->pIo,
						     pTnrDmd->cfgMemory[i].tgt,
						     0x00,
						     pTnrDmd->cfgMemory[i].
						     bank);
		if (ret != CXD2880_RESULT_OK)
			return ret;

		ret = cxd2880_io_SetRegisterBits(pTnrDmd->pIo,
						 pTnrDmd->cfgMemory[i].tgt,
						 pTnrDmd->cfgMemory[i].address,
						 pTnrDmd->cfgMemory[i].value,
						 pTnrDmd->cfgMemory[i].bitMask);
		if (ret != CXD2880_RESULT_OK)
			return ret;
	}

	return ret;
}

static enum cxd2880_ret setCfgMemory(struct cxd2880_tnrdmd *pTnrDmd,
				     enum cxd2880_io_tgt tgt,
				     u8 bank, u8 address, u8 value, u8 bitMask)
{
	u8 i;
	u8 valueStored = 0;

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	for (i = 0; i < pTnrDmd->cfgMemoryLastEntry; i++) {
		if ((valueStored == 0) &&
		    (pTnrDmd->cfgMemory[i].tgt == tgt) &&
		    (pTnrDmd->cfgMemory[i].bank == bank) &&
		    (pTnrDmd->cfgMemory[i].address == address)) {
			pTnrDmd->cfgMemory[i].value &= ~bitMask;
			pTnrDmd->cfgMemory[i].value |= (value & bitMask);

			pTnrDmd->cfgMemory[i].bitMask |= bitMask;

			valueStored = 1;
		}
	}

	if (valueStored == 0) {
		if (pTnrDmd->cfgMemoryLastEntry <
		    CXD2880_TNRDMD_MAX_CFG_MEMORY_COUNT) {
			pTnrDmd->cfgMemory[pTnrDmd->cfgMemoryLastEntry].tgt =
			    tgt;
			pTnrDmd->cfgMemory[pTnrDmd->cfgMemoryLastEntry].bank =
			    bank;
			pTnrDmd->cfgMemory[pTnrDmd->cfgMemoryLastEntry].
			    address = address;
			pTnrDmd->cfgMemory[pTnrDmd->cfgMemoryLastEntry].value =
			    (value & bitMask);
			pTnrDmd->cfgMemory[pTnrDmd->cfgMemoryLastEntry].
			    bitMask = bitMask;
			pTnrDmd->cfgMemoryLastEntry++;
		} else {
			return CXD2880_RESULT_ERROR_OVERFLOW;
		}
	}

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_Create(struct cxd2880_tnrdmd *pTnrDmd,
				       struct cxd2880_io *pIo,
				       struct cxd2880_tnrdmd_create_param
				       *pCreateParam)
{

	if ((!pTnrDmd) || (!pIo) || (!pCreateParam))
		return CXD2880_RESULT_ERROR_ARG;

	cxd2880_memset(pTnrDmd, 0, sizeof(struct cxd2880_tnrdmd));

	pTnrDmd->pIo = pIo;
	pTnrDmd->createParam = *pCreateParam;

	pTnrDmd->diverMode = CXD2880_TNRDMD_DIVERMODE_SINGLE;
	pTnrDmd->pDiverSub = NULL;

	pTnrDmd->serialTsClockModeContinuous = 1;
	pTnrDmd->enFEFIntmtntBase = 1;
	pTnrDmd->enFEFIntmtntLite = 1;
	pTnrDmd->RFLvlCmpstn = NULL;
	pTnrDmd->pLNAThrsTableAir = NULL;
	pTnrDmd->pLNAThrsTableCable = NULL;

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_diver_Create(struct cxd2880_tnrdmd *pTnrDmdMain,
					     struct cxd2880_io *pIoMain,
					     struct cxd2880_tnrdmd *pTnrDmdSub,
					     struct cxd2880_io *pIoSub,
					     struct
					     cxd2880_tnrdmd_diver_create_param
					     *pCreateParam)
{

	if ((!pTnrDmdMain) || (!pIoMain) || (!pTnrDmdSub) || (!pIoSub)
	    || (!pCreateParam))
		return CXD2880_RESULT_ERROR_ARG;

	cxd2880_memset(pTnrDmdMain, 0, sizeof(struct cxd2880_tnrdmd));
	cxd2880_memset(pTnrDmdSub, 0, sizeof(struct cxd2880_tnrdmd));

	pTnrDmdMain->pIo = pIoMain;
	pTnrDmdMain->diverMode = CXD2880_TNRDMD_DIVERMODE_MAIN;
	pTnrDmdMain->pDiverSub = pTnrDmdSub;
	pTnrDmdMain->createParam.enInternalLDO = pCreateParam->enInternalLDO;
	pTnrDmdMain->createParam.tsOutputIF = pCreateParam->tsOutputIF;
	pTnrDmdMain->createParam.xtalShareType =
	    CXD2880_TNRDMD_XTAL_SHARE_MASTER;
	pTnrDmdMain->createParam.xosc_cap = pCreateParam->xosc_cap_main;
	pTnrDmdMain->createParam.xosc_i = pCreateParam->xosc_i_main;
	pTnrDmdMain->createParam.isCXD2881GG = pCreateParam->isCXD2881GG;
	pTnrDmdMain->createParam.stationaryUse = pCreateParam->stationaryUse;

	pTnrDmdSub->pIo = pIoSub;
	pTnrDmdSub->diverMode = CXD2880_TNRDMD_DIVERMODE_SUB;
	pTnrDmdSub->pDiverSub = NULL;
	pTnrDmdSub->createParam.enInternalLDO = pCreateParam->enInternalLDO;
	pTnrDmdSub->createParam.tsOutputIF = pCreateParam->tsOutputIF;
	pTnrDmdSub->createParam.xtalShareType = CXD2880_TNRDMD_XTAL_SHARE_SLAVE;
	pTnrDmdSub->createParam.xosc_cap = 0;
	pTnrDmdSub->createParam.xosc_i = pCreateParam->xosc_i_sub;
	pTnrDmdSub->createParam.isCXD2881GG = pCreateParam->isCXD2881GG;
	pTnrDmdSub->createParam.stationaryUse = pCreateParam->stationaryUse;

	pTnrDmdMain->serialTsClockModeContinuous = 1;
	pTnrDmdMain->enFEFIntmtntBase = 1;
	pTnrDmdMain->enFEFIntmtntLite = 1;
	pTnrDmdMain->RFLvlCmpstn = NULL;
	pTnrDmdMain->pLNAThrsTableAir = NULL;
	pTnrDmdMain->pLNAThrsTableCable = NULL;

	pTnrDmdSub->serialTsClockModeContinuous = 1;
	pTnrDmdSub->enFEFIntmtntBase = 1;
	pTnrDmdSub->enFEFIntmtntLite = 1;
	pTnrDmdSub->RFLvlCmpstn = NULL;
	pTnrDmdSub->pLNAThrsTableAir = NULL;
	pTnrDmdSub->pLNAThrsTableCable = NULL;

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_Init1(struct cxd2880_tnrdmd *pTnrDmd)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB))
		return CXD2880_RESULT_ERROR_ARG;

	pTnrDmd->chipID = CXD2880_TNRDMD_CHIP_ID_UNKNOWN;
	pTnrDmd->state = CXD2880_TNRDMD_STATE_UNKNOWN;
	pTnrDmd->clockMode = CXD2880_TNRDMD_CLOCKMODE_UNKNOWN;
	pTnrDmd->frequencyKHz = 0;
	pTnrDmd->sys = CXD2880_DTV_SYS_UNKNOWN;
	pTnrDmd->bandwidth = CXD2880_DTV_BW_UNKNOWN;
	pTnrDmd->scanMode = 0;
	cxd2880_atomic_set(&(pTnrDmd->cancel), 0);

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
		pTnrDmd->pDiverSub->chipID = CXD2880_TNRDMD_CHIP_ID_UNKNOWN;
		pTnrDmd->pDiverSub->state = CXD2880_TNRDMD_STATE_UNKNOWN;
		pTnrDmd->pDiverSub->clockMode =
		    CXD2880_TNRDMD_CLOCKMODE_UNKNOWN;
		pTnrDmd->pDiverSub->frequencyKHz = 0;
		pTnrDmd->pDiverSub->sys = CXD2880_DTV_SYS_UNKNOWN;
		pTnrDmd->pDiverSub->bandwidth = CXD2880_DTV_BW_UNKNOWN;
		pTnrDmd->pDiverSub->scanMode = 0;
		cxd2880_atomic_set(&(pTnrDmd->pDiverSub->cancel), 0);
	}

	ret = cxd2880_tnrdmd_ChipID(pTnrDmd, &pTnrDmd->chipID);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	if (!CXD2880_TNRDMD_CHIP_ID_VALID(pTnrDmd->chipID))
		return CXD2880_RESULT_ERROR_NOSUPPORT;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
		ret =
		    cxd2880_tnrdmd_ChipID(pTnrDmd->pDiverSub,
					  &pTnrDmd->pDiverSub->chipID);
		if (ret != CXD2880_RESULT_OK)
			return ret;

		if (!CXD2880_TNRDMD_CHIP_ID_VALID(pTnrDmd->pDiverSub->chipID))
			return CXD2880_RESULT_ERROR_NOSUPPORT;
	}

	ret = P_init1(pTnrDmd);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
		ret = P_init1(pTnrDmd->pDiverSub);
		if (ret != CXD2880_RESULT_OK)
			return ret;
	}

	CXD2880_SLEEP(1);

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
		ret = P_init2(pTnrDmd->pDiverSub);
		if (ret != CXD2880_RESULT_OK)
			return ret;
	}

	ret = P_init2(pTnrDmd);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	CXD2880_SLEEP(5);

	ret = P_init3(pTnrDmd);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
		ret = P_init3(pTnrDmd->pDiverSub);
		if (ret != CXD2880_RESULT_OK)
			return ret;
	}

	ret = RF_init1(pTnrDmd);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
		ret = RF_init1(pTnrDmd->pDiverSub);
		if (ret != CXD2880_RESULT_OK)
			return ret;
	}

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_Init2(struct cxd2880_tnrdmd *pTnrDmd)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	{
		u8 cpuTaskCompleted = 0;

		ret =
		    cxd2880_tnrdmd_CheckInternalCPUStatus(pTnrDmd,
							  &cpuTaskCompleted);
		if (ret != CXD2880_RESULT_OK)
			return ret;

		if (!cpuTaskCompleted)
			return CXD2880_RESULT_ERROR_HW_STATE;
	}

	ret = RF_init2(pTnrDmd);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
		ret = RF_init2(pTnrDmd->pDiverSub);
		if (ret != CXD2880_RESULT_OK)
			return ret;
	}

	ret = loadCfgMemory(pTnrDmd);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
		ret = loadCfgMemory(pTnrDmd->pDiverSub);
		if (ret != CXD2880_RESULT_OK)
			return ret;
	}

	pTnrDmd->state = CXD2880_TNRDMD_STATE_SLEEP;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN)
		pTnrDmd->pDiverSub->state = CXD2880_TNRDMD_STATE_SLEEP;

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_CheckInternalCPUStatus(struct cxd2880_tnrdmd
						       *pTnrDmd,
						       u8 *pTaskCompleted)
{
	u16 cpuStatus = 0;
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pTaskCompleted))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	ret = cxd2880_tnrdmd_mon_InternalCPUStatus(pTnrDmd, &cpuStatus);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SINGLE) {
		if (cpuStatus == 0)
			*pTaskCompleted = 1;
		else
			*pTaskCompleted = 0;

		return ret;
	}

	if (cpuStatus != 0) {
		*pTaskCompleted = 0;
		return ret;
	}

	ret =
	    cxd2880_tnrdmd_mon_InternalCPUStatus_sub(pTnrDmd,
						     &cpuStatus);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	if (cpuStatus == 0)
		*pTaskCompleted = 1;
	else
		*pTaskCompleted = 0;

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_CommonTuneSetting1(struct cxd2880_tnrdmd
						   *pTnrDmd,
						   enum cxd2880_dtv_sys sys,
						   u32 frequencyKHz,
						   enum cxd2880_dtv_bandwidth
						   bandwidth,
						   u8 oneSegmentOptimize,
						   u8 oneSegOptShftDrctn)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;
	int shiftFrequencyKHz = 0;

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if ((pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
	    && (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE))
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (frequencyKHz < 4000)
		return CXD2880_RESULT_ERROR_RANGE;

	ret = cxd2880_tnrdmd_Sleep(pTnrDmd);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	{
		u8 data = 0;

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
				     0x00) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x2B, &data,
				 1) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		switch (sys) {
		case CXD2880_DTV_SYS_DVBT:
		case CXD2880_DTV_SYS_ISDBT:
		case CXD2880_DTV_SYS_ISDBTSB:
		case CXD2880_DTV_SYS_ISDBTMM_A:
		case CXD2880_DTV_SYS_ISDBTMM_B:
			if (data == 0x00) {
				ret = T_powerX(pTnrDmd, 1);
				if (ret != CXD2880_RESULT_OK)
					return ret;

				if (pTnrDmd->diverMode ==
				    CXD2880_TNRDMD_DIVERMODE_MAIN) {
					ret = T_powerX(pTnrDmd->pDiverSub, 1);
					if (ret != CXD2880_RESULT_OK)
						return ret;
				}

			}
			break;

		case CXD2880_DTV_SYS_DVBT2:
			if (data == 0x01) {
				ret = T_powerX(pTnrDmd, 0);
				if (ret != CXD2880_RESULT_OK)
					return ret;

				if (pTnrDmd->diverMode ==
				    CXD2880_TNRDMD_DIVERMODE_MAIN) {
					ret = T_powerX(pTnrDmd->pDiverSub, 0);
					if (ret != CXD2880_RESULT_OK)
						return ret;
				}
			}
			break;

		default:
			return CXD2880_RESULT_ERROR_ARG;
		}
	}

	{
		enum cxd2880_tnrdmd_clockmode newClockMode =
		    CXD2880_TNRDMD_CLOCKMODE_A;

		ret = SPLL_reset(pTnrDmd, newClockMode);
		if (ret != CXD2880_RESULT_OK)
			return ret;

		pTnrDmd->clockMode = newClockMode;

		if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
			ret = SPLL_reset(pTnrDmd->pDiverSub, newClockMode);
			if (ret != CXD2880_RESULT_OK)
				return ret;

			pTnrDmd->pDiverSub->clockMode = newClockMode;
		}

		ret = loadCfgMemory(pTnrDmd);
		if (ret != CXD2880_RESULT_OK)
			return ret;

		if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
			ret = loadCfgMemory(pTnrDmd->pDiverSub);
			if (ret != CXD2880_RESULT_OK)
				return ret;
		}
	}

	if (oneSegmentOptimize) {
		if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
			shiftFrequencyKHz = 350;
		} else {
			if (oneSegOptShftDrctn)
				shiftFrequencyKHz = 350;
			else
				shiftFrequencyKHz = -350;

			if (pTnrDmd->createParam.xtalShareType ==
			    CXD2880_TNRDMD_XTAL_SHARE_SLAVE)
				shiftFrequencyKHz *= -1;
		}
	} else {
		if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
			shiftFrequencyKHz = 150;
		} else {
			switch (pTnrDmd->createParam.xtalShareType) {
			case CXD2880_TNRDMD_XTAL_SHARE_NONE:
			case CXD2880_TNRDMD_XTAL_SHARE_EXTREF:
			default:
				shiftFrequencyKHz = 0;
				break;
			case CXD2880_TNRDMD_XTAL_SHARE_MASTER:
				shiftFrequencyKHz = 150;
				break;
			case CXD2880_TNRDMD_XTAL_SHARE_SLAVE:
				shiftFrequencyKHz = -150;
				break;
			}
		}
	}

	ret =
	    X_tune1(pTnrDmd, sys, frequencyKHz, bandwidth,
		    pTnrDmd->isCableInput, shiftFrequencyKHz);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
		ret =
		    X_tune1(pTnrDmd->pDiverSub, sys, frequencyKHz,
			    bandwidth, pTnrDmd->isCableInput,
			    -shiftFrequencyKHz);
		if (ret != CXD2880_RESULT_OK)
			return ret;
	}

	CXD2880_SLEEP(10);

	{
		u8 cpuTaskCompleted = 0;

		ret =
		    cxd2880_tnrdmd_CheckInternalCPUStatus(pTnrDmd,
							  &cpuTaskCompleted);
		if (ret != CXD2880_RESULT_OK)
			return ret;

		if (!cpuTaskCompleted)
			return CXD2880_RESULT_ERROR_HW_STATE;
	}

	ret =
	    X_tune2(pTnrDmd, bandwidth, pTnrDmd->clockMode,
		    shiftFrequencyKHz);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
		ret =
		    X_tune2(pTnrDmd->pDiverSub, bandwidth,
			    pTnrDmd->pDiverSub->clockMode,
			    -shiftFrequencyKHz);
		if (ret != CXD2880_RESULT_OK)
			return ret;
	}


	if (pTnrDmd->createParam.tsOutputIF == CXD2880_TNRDMD_TSOUT_IF_TS) {
		ret = setTSClockModeAndFreq(pTnrDmd, sys);
		if (ret != CXD2880_RESULT_OK)
			return ret;
	}

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_CommonTuneSetting2(struct cxd2880_tnrdmd
						   *pTnrDmd,
						   enum cxd2880_dtv_sys sys,
						   u8 enFEFIntmtntCtrl)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if ((pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
	    && (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE))
		return CXD2880_RESULT_ERROR_SW_STATE;

	ret = X_tune3(pTnrDmd, sys, enFEFIntmtntCtrl);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
		ret = X_tune3(pTnrDmd->pDiverSub, sys, enFEFIntmtntCtrl);
		if (ret != CXD2880_RESULT_OK)
			return ret;
	}

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
		ret = X_tune4(pTnrDmd);
		if (ret != CXD2880_RESULT_OK)
			return ret;
	}

	ret = cxd2880_tnrdmd_SetTSOutput(pTnrDmd, 1);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_Sleep(struct cxd2880_tnrdmd *pTnrDmd)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state == CXD2880_TNRDMD_STATE_SLEEP) {
	} else if (pTnrDmd->state == CXD2880_TNRDMD_STATE_ACTIVE) {
		ret = cxd2880_tnrdmd_SetTSOutput(pTnrDmd, 0);
		if (ret != CXD2880_RESULT_OK)
			return ret;

		if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
			ret = X_sleep1(pTnrDmd);
			if (ret != CXD2880_RESULT_OK)
				return ret;
		}

		ret = X_sleep2(pTnrDmd);
		if (ret != CXD2880_RESULT_OK)
			return ret;

		if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
			ret = X_sleep2(pTnrDmd->pDiverSub);
			if (ret != CXD2880_RESULT_OK)
				return ret;
		}

		switch (pTnrDmd->sys) {
		case CXD2880_DTV_SYS_DVBT:
			ret = cxd2880_tnrdmd_dvbt_SleepSetting(pTnrDmd);
			if (ret != CXD2880_RESULT_OK)
				return ret;
			break;

		case CXD2880_DTV_SYS_DVBT2:
			ret = cxd2880_tnrdmd_dvbt2_SleepSetting(pTnrDmd);
			if (ret != CXD2880_RESULT_OK)
				return ret;
			break;

		default:
			return CXD2880_RESULT_ERROR_SW_STATE;
		}

		ret = X_sleep3(pTnrDmd);
		if (ret != CXD2880_RESULT_OK)
			return ret;

		if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
			ret = X_sleep3(pTnrDmd->pDiverSub);
			if (ret != CXD2880_RESULT_OK)
				return ret;
		}

		ret = X_sleep4(pTnrDmd);
		if (ret != CXD2880_RESULT_OK)
			return ret;

		if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
			ret = X_sleep4(pTnrDmd->pDiverSub);
			if (ret != CXD2880_RESULT_OK)
				return ret;
		}

		pTnrDmd->state = CXD2880_TNRDMD_STATE_SLEEP;
		pTnrDmd->frequencyKHz = 0;
		pTnrDmd->sys = CXD2880_DTV_SYS_UNKNOWN;
		pTnrDmd->bandwidth = CXD2880_DTV_BW_UNKNOWN;

		if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
			pTnrDmd->pDiverSub->state = CXD2880_TNRDMD_STATE_SLEEP;
			pTnrDmd->pDiverSub->frequencyKHz = 0;
			pTnrDmd->pDiverSub->sys = CXD2880_DTV_SYS_UNKNOWN;
			pTnrDmd->pDiverSub->bandwidth = CXD2880_DTV_BW_UNKNOWN;
		}
	} else {
		return CXD2880_RESULT_ERROR_SW_STATE;
	}

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_SetCfg(struct cxd2880_tnrdmd *pTnrDmd,
				       enum cxd2880_tnrdmd_cfg_id id, int value)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;
	u8 data[2] = { 0 };
	u8 needSubSetting = 0;

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if ((pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
	    && (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE))
		return CXD2880_RESULT_ERROR_SW_STATE;

	switch (id) {
	case CXD2880_TNRDMD_CFG_OUTPUT_SEL_MSB:
		if (pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
			return CXD2880_RESULT_ERROR_SW_STATE;

		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_DMD,
							  0x00, 0xC4,
							  (u8) (value ? 0x00 :
								0x10), 0x10);
		if (ret != CXD2880_RESULT_OK)
			return ret;
		break;

	case CXD2880_TNRDMD_CFG_TSVALID_ACTIVE_HI:
		if (pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
			return CXD2880_RESULT_ERROR_SW_STATE;

		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_DMD,
							  0x00, 0xC5,
							  (u8) (value ? 0x00 :
								0x02), 0x02);
		if (ret != CXD2880_RESULT_OK)
			return ret;
		break;

	case CXD2880_TNRDMD_CFG_TSSYNC_ACTIVE_HI:
		if (pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
			return CXD2880_RESULT_ERROR_SW_STATE;

		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_DMD,
							  0x00, 0xC5,
							  (u8) (value ? 0x00 :
								0x04), 0x04);
		if (ret != CXD2880_RESULT_OK)
			return ret;
		break;

	case CXD2880_TNRDMD_CFG_TSERR_ACTIVE_HI:
		if (pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
			return CXD2880_RESULT_ERROR_SW_STATE;

		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_DMD,
							  0x00, 0xCB,
							  (u8) (value ? 0x00 :
								0x01), 0x01);
		if (ret != CXD2880_RESULT_OK)
			return ret;
		break;

	case CXD2880_TNRDMD_CFG_LATCH_ON_POSEDGE:
		if (pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
			return CXD2880_RESULT_ERROR_SW_STATE;

		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_DMD,
							  0x00, 0xC5,
							  (u8) (value ? 0x01 :
								0x00), 0x01);
		if (ret != CXD2880_RESULT_OK)
			return ret;
		break;

	case CXD2880_TNRDMD_CFG_TSCLK_CONT:
		if (pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
			return CXD2880_RESULT_ERROR_SW_STATE;

		pTnrDmd->serialTsClockModeContinuous =
		    (u8) (value ? 0x01 : 0x00);
		break;

	case CXD2880_TNRDMD_CFG_TSCLK_MASK:
		if (pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
			return CXD2880_RESULT_ERROR_SW_STATE;

		if ((value < 0) || (value > 0x1F))
			return CXD2880_RESULT_ERROR_RANGE;

		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_DMD,
							  0x00, 0xC6,
							  (u8) value, 0x1F);
		if (ret != CXD2880_RESULT_OK)
			return ret;
		break;

	case CXD2880_TNRDMD_CFG_TSVALID_MASK:
		if (pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
			return CXD2880_RESULT_ERROR_SW_STATE;

		if ((value < 0) || (value > 0x1F))
			return CXD2880_RESULT_ERROR_RANGE;

		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_DMD,
							  0x00, 0xC8,
							  (u8) value, 0x1F);
		if (ret != CXD2880_RESULT_OK)
			return ret;
		break;

	case CXD2880_TNRDMD_CFG_TSERR_MASK:
		if (pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
			return CXD2880_RESULT_ERROR_SW_STATE;

		if ((value < 0) || (value > 0x1F))
			return CXD2880_RESULT_ERROR_RANGE;

		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_DMD,
							  0x00, 0xC9,
							  (u8) value, 0x1F);
		if (ret != CXD2880_RESULT_OK)
			return ret;
		break;

	case CXD2880_TNRDMD_CFG_TSERR_VALID_DIS:
		if (pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
			return CXD2880_RESULT_ERROR_SW_STATE;

		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_DMD,
							  0x00, 0x91,
							  (u8) (value ? 0x01 :
								0x00), 0x01);
		if (ret != CXD2880_RESULT_OK)
			return ret;
		break;

	case CXD2880_TNRDMD_CFG_TSPIN_CURRENT:
		if (pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
			return CXD2880_RESULT_ERROR_SW_STATE;

		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_SYS,
							  0x00, 0x51,
							  (u8) value, 0x3F);
		if (ret != CXD2880_RESULT_OK)
			return ret;
		break;

	case CXD2880_TNRDMD_CFG_TSPIN_PULLUP_MANUAL:
		if (pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
			return CXD2880_RESULT_ERROR_SW_STATE;

		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_SYS,
							  0x00, 0x50,
							  (u8) (value ? 0x80 :
								0x00), 0x80);
		if (ret != CXD2880_RESULT_OK)
			return ret;
		break;

	case CXD2880_TNRDMD_CFG_TSPIN_PULLUP:
		if (pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
			return CXD2880_RESULT_ERROR_SW_STATE;

		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_SYS,
							  0x00, 0x50,
							  (u8) value, 0x3F);
		if (ret != CXD2880_RESULT_OK)
			return ret;
		break;

	case CXD2880_TNRDMD_CFG_TSCLK_FREQ:
		if (pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
			return CXD2880_RESULT_ERROR_SW_STATE;

		if ((value < 0) || (value > 1))
			return CXD2880_RESULT_ERROR_RANGE;

		pTnrDmd->serialTsClkFreq =
		    (enum cxd2880_tnrdmd_serial_ts_clk)value;
		break;

	case CXD2880_TNRDMD_CFG_TSBYTECLK_MANUAL:
		if (pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
			return CXD2880_RESULT_ERROR_SW_STATE;

		if ((value < 0) || (value > 0xFF))
			return CXD2880_RESULT_ERROR_RANGE;

		pTnrDmd->tsByteClkManualSetting = (u8) value;

		break;

	case CXD2880_TNRDMD_CFG_TS_PACKET_GAP:
		if (pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
			return CXD2880_RESULT_ERROR_SW_STATE;

		if ((value < 0) || (value > 7))
			return CXD2880_RESULT_ERROR_RANGE;

		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_DMD,
							  0x00, 0xD6,
							  (u8) value, 0x07);
		if (ret != CXD2880_RESULT_OK)
			return ret;

		break;

	case CXD2880_TNRDMD_CFG_TS_BACKWARDS_COMPATIBLE:
		if (pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
			return CXD2880_RESULT_ERROR_SW_STATE;

		pTnrDmd->isTsBackwardsCompatibleMode = (u8) (value ? 1 : 0);

		break;

	case CXD2880_TNRDMD_CFG_PWM_VALUE:
		if ((value < 0) || (value > 0x1000))
			return CXD2880_RESULT_ERROR_RANGE;

		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_DMD,
							  0x00, 0x22,
							  (u8) (value ? 0x01 :
								0x00), 0x01);
		if (ret != CXD2880_RESULT_OK)
			return ret;

		data[0] = (u8) (((u16) value >> 8) & 0x1F);
		data[1] = (u8) ((u16) value & 0xFF);

		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_DMD,
							  0x00, 0x23,
							  data[0],
							  0x1F);
		if (ret != CXD2880_RESULT_OK)
			return ret;

		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_DMD,
							  0x00, 0x24,
							  data[1],
							  0xFF);
		if (ret != CXD2880_RESULT_OK)
			return ret;

		break;

	case CXD2880_TNRDMD_CFG_INTERRUPT:
		data[0] = (u8) ((value >> 8) & 0xFF);
		data[1] = (u8) (value & 0xFF);
		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_SYS,
							  0x00, 0x48, data[0],
							  0xFF);
		if (ret != CXD2880_RESULT_OK)
			return ret;
		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_SYS,
							  0x00, 0x49, data[1],
							  0xFF);
		if (ret != CXD2880_RESULT_OK)
			return ret;
		break;

	case CXD2880_TNRDMD_CFG_INTERRUPT_LOCK_SEL:
		data[0] = (u8) (value & 0x07);
		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_SYS,
							  0x00, 0x4A, data[0],
							  0x07);
		if (ret != CXD2880_RESULT_OK)
			return ret;
		break;

	case CXD2880_TNRDMD_CFG_INTERRUPT_INV_LOCK_SEL:
		data[0] = (u8) ((value & 0x07) << 3);
		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_SYS,
							  0x00, 0x4A, data[0],
							  0x38);
		if (ret != CXD2880_RESULT_OK)
			return ret;
		break;

	case CXD2880_TNRDMD_CFG_FIXED_CLOCKMODE:
		if ((value < (int)CXD2880_TNRDMD_CLOCKMODE_UNKNOWN)
		    || (value > (int)CXD2880_TNRDMD_CLOCKMODE_C))
			return CXD2880_RESULT_ERROR_RANGE;
		pTnrDmd->fixedClockMode = (enum cxd2880_tnrdmd_clockmode)value;
		break;

	case CXD2880_TNRDMD_CFG_CABLE_INPUT:
		pTnrDmd->isCableInput = (u8) (value ? 1 : 0);
		break;

	case CXD2880_TNRDMD_CFG_DVBT2_FEF_INTERMITTENT_BASE:
		pTnrDmd->enFEFIntmtntBase = (u8) (value ? 1 : 0);
		break;

	case CXD2880_TNRDMD_CFG_DVBT2_FEF_INTERMITTENT_LITE:
		pTnrDmd->enFEFIntmtntLite = (u8) (value ? 1 : 0);
		break;

	case CXD2880_TNRDMD_CFG_TS_BUF_ALMOST_EMPTY_THRS:
		data[0] = (u8) ((value >> 8) & 0x07);
		data[1] = (u8) (value & 0xFF);
		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_DMD,
							  0x00, 0x99, data[0],
							  0x07);
		if (ret != CXD2880_RESULT_OK)
			return ret;
		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_DMD,
							  0x00, 0x9A, data[1],
							  0xFF);
		if (ret != CXD2880_RESULT_OK)
			return ret;
		break;

	case CXD2880_TNRDMD_CFG_TS_BUF_ALMOST_FULL_THRS:
		data[0] = (u8) ((value >> 8) & 0x07);
		data[1] = (u8) (value & 0xFF);
		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_DMD,
							  0x00, 0x9B, data[0],
							  0x07);
		if (ret != CXD2880_RESULT_OK)
			return ret;
		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_DMD,
							  0x00, 0x9C, data[1],
							  0xFF);
		if (ret != CXD2880_RESULT_OK)
			return ret;
		break;

	case CXD2880_TNRDMD_CFG_TS_BUF_RRDY_THRS:
		data[0] = (u8) ((value >> 8) & 0x07);
		data[1] = (u8) (value & 0xFF);
		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_DMD,
							  0x00, 0x9D, data[0],
							  0x07);
		if (ret != CXD2880_RESULT_OK)
			return ret;
		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_DMD,
							  0x00, 0x9E, data[1],
							  0xFF);
		if (ret != CXD2880_RESULT_OK)
			return ret;
		break;

	case CXD2880_TNRDMD_CFG_BLINDTUNE_DVBT2_FIRST:
		pTnrDmd->blindTuneDvbt2First = (u8) (value ? 1 : 0);
		break;

	case CXD2880_TNRDMD_CFG_DVBT_BERN_PERIOD:
		if ((value < 0) || (value > 31))
			return CXD2880_RESULT_ERROR_RANGE;

		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_DMD,
							  0x10, 0x60,
							  (u8) (value & 0x1F),
							  0x1F);
		if (ret != CXD2880_RESULT_OK)
			return ret;
		break;

	case CXD2880_TNRDMD_CFG_DVBT_VBER_PERIOD:
		if ((value < 0) || (value > 7))
			return CXD2880_RESULT_ERROR_RANGE;

		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_DMD,
							  0x10, 0x6F,
							  (u8) (value & 0x07),
							  0x07);
		if (ret != CXD2880_RESULT_OK)
			return ret;
		break;

	case CXD2880_TNRDMD_CFG_DVBT2_BBER_MES:
		if ((value < 0) || (value > 15))
			return CXD2880_RESULT_ERROR_RANGE;

		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_DMD,
							  0x20, 0x72,
							  (u8) (value & 0x0F),
							  0x0F);
		if (ret != CXD2880_RESULT_OK)
			return ret;
		break;

	case CXD2880_TNRDMD_CFG_DVBT2_LBER_MES:
		if ((value < 0) || (value > 15))
			return CXD2880_RESULT_ERROR_RANGE;

		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_DMD,
							  0x20, 0x6F,
							  (u8) (value & 0x0F),
							  0x0F);
		if (ret != CXD2880_RESULT_OK)
			return ret;
		break;

	case CXD2880_TNRDMD_CFG_DVBT_PER_MES:
		if ((value < 0) || (value > 15))
			return CXD2880_RESULT_ERROR_RANGE;

		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_DMD,
							  0x10, 0x5C,
							  (u8) (value & 0x0F),
							  0x0F);
		if (ret != CXD2880_RESULT_OK)
			return ret;
		break;

	case CXD2880_TNRDMD_CFG_DVBT2_PER_MES:
		if ((value < 0) || (value > 15))
			return CXD2880_RESULT_ERROR_RANGE;

		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_DMD,
							  0x24, 0xDC,
							  (u8) (value & 0x0F),
							  0x0F);
		if (ret != CXD2880_RESULT_OK)
			return ret;
		break;

	case CXD2880_TNRDMD_CFG_ISDBT_BERPER_PERIOD:
		data[0] = (u8) ((value & 0x00007F00) >> 8);
		data[1] = (u8) (value & 0x000000FF);

		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_DMD,
							  0x60, 0x5B,
							  data[0],
							  0x7F);
		if (ret != CXD2880_RESULT_OK)
			CXD2880_TRACE_RETURN(ret);
		ret =
		    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd,
							  CXD2880_IO_TGT_DMD,
							  0x60, 0x5C,
							  data[1],
							  0xFF);
		if (ret != CXD2880_RESULT_OK)
			CXD2880_TRACE_RETURN(ret);
		break;

	default:
		return CXD2880_RESULT_ERROR_ARG;
	}

	if (needSubSetting
	    && (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN)) {
		ret = cxd2880_tnrdmd_SetCfg(pTnrDmd->pDiverSub, id, value);
		if (ret != CXD2880_RESULT_OK)
			return ret;
	}

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_GPIOSetCfg(struct cxd2880_tnrdmd *pTnrDmd,
					   u8 id,
					   u8 en,
					   enum cxd2880_tnrdmd_gpio_mode mode,
					   u8 openDrain, u8 invert)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (id > 2)
		return CXD2880_RESULT_ERROR_ARG;

	if (mode > CXD2880_TNRDMD_GPIO_MODE_EEW)
		return CXD2880_RESULT_ERROR_ARG;

	if ((pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
	    && (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE))
		return CXD2880_RESULT_ERROR_SW_STATE;

	ret =
	    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd, CXD2880_IO_TGT_SYS,
						  0x00, 0x40 + id, (u8) mode,
						  0x0F);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	ret =
	    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd, CXD2880_IO_TGT_SYS,
						  0x00, 0x43,
						  (u8) (openDrain ? (1 << id) :
							0), (u8) (1 << id));
	if (ret != CXD2880_RESULT_OK)
		return ret;

	ret =
	    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd, CXD2880_IO_TGT_SYS,
						  0x00, 0x44,
						  (u8) (invert ? (1 << id) : 0),
						  (u8) (1 << id));
	if (ret != CXD2880_RESULT_OK)
		return ret;

	ret =
	    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd, CXD2880_IO_TGT_SYS,
						  0x00, 0x45,
						  (u8) (en ? 0 : (1 << id)),
						  (u8) (1 << id));
	if (ret != CXD2880_RESULT_OK)
		return ret;

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_GPIOSetCfg_sub(struct cxd2880_tnrdmd *pTnrDmd,
					       u8 id,
					       u8 en,
					       enum cxd2880_tnrdmd_gpio_mode
					       mode, u8 openDrain, u8 invert)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode != CXD2880_TNRDMD_DIVERMODE_MAIN)
		return CXD2880_RESULT_ERROR_ARG;

	ret =
	    cxd2880_tnrdmd_GPIOSetCfg(pTnrDmd->pDiverSub, id, en, mode,
				      openDrain, invert);

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_GPIORead(struct cxd2880_tnrdmd *pTnrDmd,
					 u8 id, u8 *pValue)
{
	u8 data = 0;

	if ((!pTnrDmd) || (!pValue))
		return CXD2880_RESULT_ERROR_ARG;

	if (id > 2)
		return CXD2880_RESULT_ERROR_ARG;

	if ((pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
	    && (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE))
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x0A) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x20, &data,
			 1) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	*pValue = (u8) ((data >> id) & 0x01);

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_GPIORead_sub(struct cxd2880_tnrdmd *pTnrDmd,
					     u8 id, u8 *pValue)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode != CXD2880_TNRDMD_DIVERMODE_MAIN)
		return CXD2880_RESULT_ERROR_ARG;

	ret = cxd2880_tnrdmd_GPIORead(pTnrDmd->pDiverSub, id, pValue);

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_GPIOWrite(struct cxd2880_tnrdmd *pTnrDmd,
					  u8 id, u8 value)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (id > 2)
		return CXD2880_RESULT_ERROR_ARG;

	if ((pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
	    && (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE))
		return CXD2880_RESULT_ERROR_SW_STATE;

	ret =
	    cxd2880_tnrdmd_SetAndSaveRegisterBits(pTnrDmd, CXD2880_IO_TGT_SYS,
						  0x00, 0x46,
						  (u8) (value ? (1 << id) : 0),
						  (u8) (1 << id));
	if (ret != CXD2880_RESULT_OK)
		return ret;

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_GPIOWrite_sub(struct cxd2880_tnrdmd *pTnrDmd,
					      u8 id, u8 value)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode != CXD2880_TNRDMD_DIVERMODE_MAIN)
		return CXD2880_RESULT_ERROR_ARG;

	ret = cxd2880_tnrdmd_GPIOWrite(pTnrDmd->pDiverSub, id, value);

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_InterruptRead(struct cxd2880_tnrdmd *pTnrDmd,
					      u16 *pValue)
{
	u8 data[2] = { 0 };

	if ((!pTnrDmd) || (!pValue))
		return CXD2880_RESULT_ERROR_ARG;

	if ((pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
	    && (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE))
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x0A) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x15, data,
			 2) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	*pValue = (u16) (((u16) data[0] << 8) | (data[1]));

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_InterruptClear(struct cxd2880_tnrdmd *pTnrDmd,
					       u16 value)
{
	u8 data[2] = { 0 };

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if ((pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
	    && (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE))
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	data[0] = (u8) ((value >> 8) & 0xFF);
	data[1] = (u8) (value & 0xFF);
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x3C, data,
			  2) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_TSBufClear(struct cxd2880_tnrdmd *pTnrDmd,
					   u8 clearOverflowFlag,
					   u8 clearUnderflowFlag, u8 clearBuf)
{
	u8 data[2] = { 0 };

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if ((pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
	    && (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE))
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	data[0] = (u8) (clearOverflowFlag ? 0x02 : 0x00);
	data[0] |= (u8) (clearUnderflowFlag ? 0x01 : 0x00);
	data[1] = (u8) (clearBuf ? 0x01 : 0x00);
	if (pTnrDmd->pIo->
	    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x9F, data,
			  2) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_ChipID(struct cxd2880_tnrdmd *pTnrDmd,
				       enum cxd2880_tnrdmd_chip_id *pChipID)
{
	u8 data = 0;

	if ((!pTnrDmd) || (!pChipID))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0xFD, &data,
			 1) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	*pChipID = (enum cxd2880_tnrdmd_chip_id)data;

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_SetAndSaveRegisterBits(struct cxd2880_tnrdmd
						       *pTnrDmd,
						       enum cxd2880_io_tgt tgt,
						       u8 bank, u8 address,
						       u8 value, u8 bitMask)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if ((pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
	    && (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE))
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->pIo->WriteOneRegister(pTnrDmd->pIo, tgt, 0x00, bank) !=
	    CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (cxd2880_io_SetRegisterBits
	    (pTnrDmd->pIo, tgt, address, value, bitMask) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	ret = setCfgMemory(pTnrDmd, tgt, bank, address, value, bitMask);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_SetScanMode(struct cxd2880_tnrdmd *pTnrDmd,
					    enum cxd2880_dtv_sys sys,
					    u8 scanModeEnd)
{

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if ((pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
	    && (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE))
		return CXD2880_RESULT_ERROR_SW_STATE;

	CXD2880_ARG_UNUSED(sys);

	pTnrDmd->scanMode = scanModeEnd;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_MAIN) {
		enum cxd2880_ret ret = CXD2880_RESULT_OK;

		ret =
		    cxd2880_tnrdmd_SetScanMode(pTnrDmd->pDiverSub, sys,
					       scanModeEnd);
		if (ret != CXD2880_RESULT_OK)
			return ret;
	}

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_SetRFLvlCmpstn(struct cxd2880_tnrdmd *pTnrDmd,
					       enum
					       cxd2880_ret(*pRFLvlCmpstn)
					       (struct cxd2880_tnrdmd *, int *))
{

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	pTnrDmd->RFLvlCmpstn = pRFLvlCmpstn;

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_SetRFLvlCmpstn_sub(struct cxd2880_tnrdmd
						   *pTnrDmd,
						   enum
						   cxd2880_ret(*pRFLvlCmpstn)
						   (struct cxd2880_tnrdmd *,
						    int *))
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode != CXD2880_TNRDMD_DIVERMODE_MAIN)
		return CXD2880_RESULT_ERROR_ARG;

	ret = cxd2880_tnrdmd_SetRFLvlCmpstn(pTnrDmd->pDiverSub, pRFLvlCmpstn);

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_SetLNAThrs(struct cxd2880_tnrdmd *pTnrDmd,
					   struct
					   cxd2880_tnrdmd_lna_thrs_tbl_air
					   *pTableAir,
					   struct
					   cxd2880_tnrdmd_lna_thrs_tbl_cable
					   *pTableCable)
{

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	pTnrDmd->pLNAThrsTableAir = pTableAir;
	pTnrDmd->pLNAThrsTableCable = pTableCable;

	return CXD2880_RESULT_OK;

}

enum cxd2880_ret cxd2880_tnrdmd_SetLNAThrs_sub(struct cxd2880_tnrdmd *pTnrDmd,
					       struct
					       cxd2880_tnrdmd_lna_thrs_tbl_air
					       *pTableAir,
					       struct
					       cxd2880_tnrdmd_lna_thrs_tbl_cable
					       *pTableCable)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode != CXD2880_TNRDMD_DIVERMODE_MAIN)
		return CXD2880_RESULT_ERROR_ARG;

	ret =
	    cxd2880_tnrdmd_SetLNAThrs(pTnrDmd->pDiverSub, pTableAir,
				      pTableCable);

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_SetTSPinHighLow(struct cxd2880_tnrdmd *pTnrDmd,
						u8 en, u8 value)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
		return CXD2880_RESULT_ERROR_SW_STATE;

	if (pTnrDmd->createParam.tsOutputIF != CXD2880_TNRDMD_TSOUT_IF_TS)
		return CXD2880_RESULT_ERROR_NOSUPPORT;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (en) {
		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x50,
				     ((value & 0x1F) | 0x80)) !=
		    CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x52,
				     (value & 0x1F)) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
	} else {
		ret =
		    pTnrDmd->pIo->WriteOneRegister(pTnrDmd->pIo,
						   CXD2880_IO_TGT_SYS, 0x50,
						   0x3F);
		if (ret != CXD2880_RESULT_OK)
			return ret;

		if (pTnrDmd->pIo->
		    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x52,
				     0x1F) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		ret = loadCfgMemory(pTnrDmd);
		if (ret != CXD2880_RESULT_OK)
			return ret;
	}

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_SetTSOutput(struct cxd2880_tnrdmd *pTnrDmd,
					    u8 en)
{

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if ((pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
	    && (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE))
		return CXD2880_RESULT_ERROR_SW_STATE;

	switch (pTnrDmd->createParam.tsOutputIF) {
	case CXD2880_TNRDMD_TSOUT_IF_TS:
		if (en) {
			if (pTnrDmd->pIo->
			    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS,
					     0x00, 0x00) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
			if (pTnrDmd->pIo->
			    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS,
					     0x52, 0x00) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
			if (pTnrDmd->pIo->
			    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					     0x00, 0x00) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
			if (pTnrDmd->pIo->
			    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					     0xC3, 0x00) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		} else {
			if (pTnrDmd->pIo->
			    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					     0x00, 0x00) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
			if (pTnrDmd->pIo->
			    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD,
					     0xC3, 0x01) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
			if (pTnrDmd->pIo->
			    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS,
					     0x00, 0x00) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
			if (pTnrDmd->pIo->
			    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS,
					     0x52, 0x1F) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}
		break;

	case CXD2880_TNRDMD_TSOUT_IF_SPI:
		break;

	case CXD2880_TNRDMD_TSOUT_IF_SDIO:
		break;

	default:
		return CXD2880_RESULT_ERROR_SW_STATE;
	}

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret SLVT_FreezeReg(struct cxd2880_tnrdmd *pTnrDmd)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	switch (pTnrDmd->createParam.tsOutputIF) {
	case CXD2880_TNRDMD_TSOUT_IF_SPI:
	case CXD2880_TNRDMD_TSOUT_IF_SDIO:
		{
			u8 data = 0;

			if (pTnrDmd->pIo->
			    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
					 &data, 1) != CXD2880_RESULT_OK)
				return CXD2880_RESULT_ERROR_IO;
		}
		break;
	case CXD2880_TNRDMD_TSOUT_IF_TS:
	default:
		break;
	}

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x01,
			     0x01) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	return ret;
}
