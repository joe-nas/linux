/*
* cxd2880_tnrdmd_mon.c
* Sony CXD2880 DVB-T2/T tuner + demodulator driver
* The common tuner and demodulator monitor functions.
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
#include "cxd2880_tnrdmd_mon.h"

enum cxd2880_ret cxd2880_tnrdmd_mon_RFLvl(struct cxd2880_tnrdmd *pTnrDmd,
					  int *pRFLvldB)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pRFLvldB))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

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

	{
		u8 data[2] = { 0x80, 0x00 };

		if (pTnrDmd->pIo->
		    WriteRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x5B, data,
				  2) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;
	}

	CXD2880_SLEEP_IN_MON(2, pTnrDmd);

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x1A) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	{
		u8 data[2];

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x15, data,
				 2) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		if ((data[0] != 0) || (data[1] != 0))
			return CXD2880_RESULT_ERROR_OTHER;

		if (pTnrDmd->pIo->
		    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x11, data,
				 2) != CXD2880_RESULT_OK)
			return CXD2880_RESULT_ERROR_IO;

		*pRFLvldB =
		    cxd2880_Convert2SComplement((data[0] << 3) |
						((data[1] & 0xE0) >> 5), 11);
	}

	*pRFLvldB *= 125;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x00,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_DMD, 0x10,
			     0x00) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	if (pTnrDmd->RFLvlCmpstn) {
		ret = pTnrDmd->RFLvlCmpstn(pTnrDmd, pRFLvldB);
		if (ret != CXD2880_RESULT_OK)
			return ret;
	}

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_mon_RFLvl_sub(struct cxd2880_tnrdmd *pTnrDmd,
					      int *pRFLvldB)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pRFLvldB))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode != CXD2880_TNRDMD_DIVERMODE_MAIN)
		return CXD2880_RESULT_ERROR_ARG;

	ret = cxd2880_tnrdmd_mon_RFLvl(pTnrDmd->pDiverSub, pRFLvldB);

	return ret;
}

enum cxd2880_ret cxd2880_tnrdmd_mon_InternalCPUStatus(struct cxd2880_tnrdmd
						      *pTnrDmd, u16 *pStatus)
{
	u8 data[2] = { 0 };

	if ((!pTnrDmd) || (!pStatus))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->pIo->
	    WriteOneRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x00,
			     0x1A) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;
	if (pTnrDmd->pIo->
	    ReadRegister(pTnrDmd->pIo, CXD2880_IO_TGT_SYS, 0x15, data,
			 2) != CXD2880_RESULT_OK)
		return CXD2880_RESULT_ERROR_IO;

	*pStatus = (u16) (((u16) data[0] << 8) | data[1]);

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_tnrdmd_mon_InternalCPUStatus_sub(struct cxd2880_tnrdmd
							  *pTnrDmd,
							  u16 *pStatus)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pStatus))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode != CXD2880_TNRDMD_DIVERMODE_MAIN)
		return CXD2880_RESULT_ERROR_ARG;

	ret = cxd2880_tnrdmd_mon_InternalCPUStatus(pTnrDmd->pDiverSub, pStatus);

	return ret;
}

