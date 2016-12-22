/*
* cxd2880_spi.h
* Sony CXD2880 DVB-T2/T tuner + demodulator driver
* The SPI I/O access interface.
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

#ifndef CXD2880_SPI_H
#define CXD2880_SPI_H

#include "cxd2880_common.h"

enum cxd2880_spi_mode {
	CXD2880_SPI_MODE_0,
	CXD2880_SPI_MODE_1,
	CXD2880_SPI_MODE_2,
	CXD2880_SPI_MODE_3
};

struct cxd2880_spi {

	enum cxd2880_ret (*Read)(struct cxd2880_spi *pSpi, u8 *pData,
				  u32 size);

	enum cxd2880_ret (*Write)(struct cxd2880_spi *pSpi, const u8 *pData,
				   u32 size);

	enum cxd2880_ret (*WriteRead)(struct cxd2880_spi *pSpi,
				       const u8 *pTxData, u32 txSize,
				       u8 *pRxData, u32 rxSize);

	u32 flags;
	void *user;
};

#endif
