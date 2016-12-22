/*
* cxd2880_common.c
* Sony CXD2880 DVB-T2/T tuner + demodulator driver
* common functions.
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

#define MASKUPPER(n) (((n) == 0) ? 0 : (0xFFFFFFFFU << (32 - (n))))
#define MASKLOWER(n) (((n) == 0) ? 0 : (0xFFFFFFFFU >> (32 - (n))))

int cxd2880_Convert2SComplement(u32 value, u32 bitlen)
{
	if ((bitlen == 0) || (bitlen >= 32))
		return (int)value;

	if (value & (u32) (1 << (bitlen - 1)))
		return (int)(MASKUPPER(32 - bitlen) | value);
	else
		return (int)(MASKLOWER(bitlen) & value);
}

u32 cxd2880_BitSplitFromByteArray(u8 *pArray, u32 startBit, u32 bitNum)
{
	u32 value = 0;
	u8 *pArrayRead;
	u8 bitRead;
	u32 lenRemain;

	if (!pArray)
		return 0;
	if ((bitNum == 0) || (bitNum > 32))
		return 0;

	pArrayRead = pArray + (startBit / 8);
	bitRead = (u8) (startBit % 8);
	lenRemain = bitNum;

	if (bitRead != 0) {
		if (((int)lenRemain) <= 8 - bitRead) {
			value = (*pArrayRead) >> ((8 - bitRead) - lenRemain);
			lenRemain = 0;
		} else {
			value = *pArrayRead++;
			lenRemain -= 8 - bitRead;
		}
	}

	while (lenRemain > 0) {
		if (lenRemain < 8) {
			value <<= lenRemain;
			value |= (*pArrayRead++ >> (8 - lenRemain));
			lenRemain = 0;
		} else {
			value <<= 8;
			value |= (u32) (*pArrayRead++);
			lenRemain -= 8;
		}
	}

	value &= MASKLOWER(bitNum);

	return value;
}
