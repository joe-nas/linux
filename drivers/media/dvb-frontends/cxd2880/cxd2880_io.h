/*
* cxd2880_io.h
* Sony CXD2880 DVB-T2/T tuner + demodulator driver
* The abstract register I/O interface definition.
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

#ifndef CXD2880_IO_H
#define CXD2880_IO_H

#include "cxd2880_common.h"

enum cxd2880_io_tgt {
	CXD2880_IO_TGT_SYS,
	CXD2880_IO_TGT_DMD
};

struct cxd2880_io {

	enum cxd2880_ret (*ReadRegister)(struct cxd2880_io *pIo,
					  enum cxd2880_io_tgt tgt,
					  u8 subAddress, u8 *pData, u32 size);

	enum cxd2880_ret (*WriteRegister)(struct cxd2880_io *pIo,
					   enum cxd2880_io_tgt tgt,
					   u8 subAddress, const u8 *pData,
					   u32 size);

	enum cxd2880_ret (*WriteOneRegister)(struct cxd2880_io *pIo,
					      enum cxd2880_io_tgt tgt,
					      u8 subAddress, u8 data);

	void *pIfObject;

	u8 i2cAddressSys;
	u8 i2cAddressDemod;

	u8 slaveSelect;

	void *user;
};

enum cxd2880_ret cxd2880_io_CommonWriteOneRegister(struct cxd2880_io *pIo,
						   enum cxd2880_io_tgt tgt,
						   u8 subAddress, u8 data);

enum cxd2880_ret cxd2880_io_SetRegisterBits(struct cxd2880_io *pIo,
					    enum cxd2880_io_tgt tgt,
					    u8 subAddress, u8 data, u8 mask);

#endif
