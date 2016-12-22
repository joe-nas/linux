/*
* cxd2880_tnrdmd_mon.h
* Sony CXD2880 DVB-T2/T tuner + demodulator driver
* The common tuner and demodulator monitor interface.
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

#ifndef CXD2880_TNRDMD_MON_H
#define CXD2880_TNRDMD_MON_H

#include "cxd2880_common.h"
#include "cxd2880_tnrdmd.h"

enum cxd2880_ret cxd2880_tnrdmd_mon_RFLvl(struct cxd2880_tnrdmd *pTnrDmd,
					  int *pRFLvldB);

enum cxd2880_ret cxd2880_tnrdmd_mon_RFLvl_sub(struct cxd2880_tnrdmd *pTnrDmd,
					      int *pRFLvldB);

enum cxd2880_ret cxd2880_tnrdmd_mon_InternalCPUStatus(struct cxd2880_tnrdmd
						      *pTnrDmd, u16 *pStatus);

enum cxd2880_ret cxd2880_tnrdmd_mon_InternalCPUStatus_sub(struct cxd2880_tnrdmd
							  *pTnrDmd,
							  u16 *pStatus);

#endif
