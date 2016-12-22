/*
* cxd2880_io.c
* Sony CXD2880 DVB-T2/T tuner + demodulator driver
* The abstract register I/O interface impementation.
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

#include "cxd2880_io.h"

enum cxd2880_ret cxd2880_io_CommonWriteOneRegister(struct cxd2880_io *pIo,
						   enum cxd2880_io_tgt tgt,
						   u8 subAddress, u8 data)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if (!pIo)
		CXD2880_TRACE_IO_RETURN(CXD2880_RESULT_ERROR_ARG);

	ret = pIo->WriteRegister(pIo, tgt, subAddress, &data, 1);

	return ret;
}

enum cxd2880_ret cxd2880_io_SetRegisterBits(struct cxd2880_io *pIo,
					    enum cxd2880_io_tgt tgt,
					    u8 subAddress, u8 data, u8 mask)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if (!pIo)
		return CXD2880_RESULT_ERROR_ARG;

	if (mask == 0x00)
		return CXD2880_RESULT_OK;

	if (mask != 0xFF) {
		u8 rdata = 0x00;

		ret = pIo->ReadRegister(pIo, tgt, subAddress, &rdata, 1);
		if (ret != CXD2880_RESULT_OK)
			return ret;

		data = (u8) ((data & mask) | (rdata & (mask ^ 0xFF)));
	}

	ret = pIo->WriteOneRegister(pIo, tgt, subAddress, data);

	return ret;
}

