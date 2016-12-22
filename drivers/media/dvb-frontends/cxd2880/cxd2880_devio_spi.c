/*
* cxd2880_devio_spi.c
* Sony CXD2880 DVB-T2/T tuner + demodulator driver
* The I/O interface via SPI.
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

#include "cxd2880_devio_spi.h"

#include "cxd2880_stdlib.h"

#define BURST_WRITE_MAX 128

static enum cxd2880_ret cxd2880_io_spi_ReadRegister(struct cxd2880_io *pIo,
						    enum cxd2880_io_tgt tgt,
						    u8 subAddress, u8 *pData,
						    u32 size)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;
	struct cxd2880_spi *pSpi = NULL;
	u8 sendData[6];
	u8 *pReadDataTop = pData;

	if ((!pIo) || (!pIo->pIfObject) || (!pData))
		return CXD2880_RESULT_ERROR_ARG;

	if (subAddress + size > 0x100)
		return CXD2880_RESULT_ERROR_RANGE;

	pSpi = (struct cxd2880_spi *)(pIo->pIfObject);

	if (tgt == CXD2880_IO_TGT_SYS)
		sendData[0] = 0x0B;
	else
		sendData[0] = 0x0A;

	sendData[3] = sendData[4] = sendData[5] = 0;

	while (size > 0) {
		sendData[1] = subAddress;
		if (size > 255)
			sendData[2] = 255;
		else
			sendData[2] = (u8) size;

		ret =
		    pSpi->WriteRead(pSpi, sendData, sizeof(sendData),
				    pReadDataTop, sendData[2]);
		if (ret != CXD2880_RESULT_OK)
			return ret;

		subAddress += sendData[2];
		pReadDataTop += sendData[2];
		size -= sendData[2];
	}

	return ret;
}

static enum cxd2880_ret cxd2880_io_spi_WriteRegister(struct cxd2880_io *pIo,
						     enum cxd2880_io_tgt tgt,
						     u8 subAddress,
						     const u8 *pData, u32 size)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;
	struct cxd2880_spi *pSpi = NULL;
	u8 sendData[BURST_WRITE_MAX + 4];
	const u8 *pWriteDataTop = pData;

	if ((!pIo) || (!pIo->pIfObject) || (!pData))
		return CXD2880_RESULT_ERROR_ARG;

	if (size > BURST_WRITE_MAX)
		CXD2880_TRACE_IO_RETURN(CXD2880_RESULT_ERROR_OVERFLOW);

	if (subAddress + size > 0x100)
		return CXD2880_RESULT_ERROR_RANGE;

	pSpi = (struct cxd2880_spi *)(pIo->pIfObject);

	if (tgt == CXD2880_IO_TGT_SYS)
		sendData[0] = 0x0F;
	else
		sendData[0] = 0x0E;

	while (size > 0) {
		sendData[1] = subAddress;
		if (size > 255)
			sendData[2] = 255;
		else
			sendData[2] = (u8) size;

		cxd2880_memcpy(&sendData[3], pWriteDataTop, sendData[2]);

		if (tgt == CXD2880_IO_TGT_SYS) {
			sendData[3 + sendData[2]] = 0x00;
			ret = pSpi->Write(pSpi, sendData, sendData[2] + 4);
		} else {
			ret = pSpi->Write(pSpi, sendData, sendData[2] + 3);
		}
		if (ret != CXD2880_RESULT_OK)
			return ret;

		subAddress += sendData[2];
		pWriteDataTop += sendData[2];
		size -= sendData[2];
	}

	return ret;
}

enum cxd2880_ret cxd2880_io_spi_Create(struct cxd2880_io *pIo,
				       struct cxd2880_spi *pSpi, u8 slaveSelect)
{

	if ((!pIo) || (!pSpi))
		CXD2880_TRACE_IO_RETURN(CXD2880_RESULT_ERROR_ARG);

	pIo->ReadRegister = cxd2880_io_spi_ReadRegister;
	pIo->WriteRegister = cxd2880_io_spi_WriteRegister;
	pIo->WriteOneRegister = cxd2880_io_CommonWriteOneRegister;
	pIo->pIfObject = pSpi;
	pIo->i2cAddressSys = 0;
	pIo->i2cAddressDemod = 0;
	pIo->slaveSelect = slaveSelect;

	return CXD2880_RESULT_OK;
}


