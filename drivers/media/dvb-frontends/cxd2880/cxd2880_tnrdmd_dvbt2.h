/*
* cxd2880_tnrdmd_dvbt2.h
* Sony CXD2880 DVB-T2/T tuner + demodulator driver
* The tuner and demodulator control interface specific to DVB-T2.
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

#ifndef CXD2880_TNRDMD_DVBT2_H
#define CXD2880_TNRDMD_DVBT2_H

#include "cxd2880_common.h"
#include "cxd2880_tnrdmd.h"

enum cxd2880_tnrdmd_dvbt2_tune_info {

	CXD2880_TNRDMD_DVBT2_TUNE_INFO_OK,

	CXD2880_TNRDMD_DVBT2_TUNE_INFO_INVALID_PLP_ID
};

struct cxd2880_dvbt2_tune_param {

	u32 centerFreqKHz;

	enum cxd2880_dtv_bandwidth bandwidth;

	u16 dataPLPID;

	enum cxd2880_dvbt2_profile profile;

	enum cxd2880_tnrdmd_dvbt2_tune_info tuneInfo;
};

#define CXD2880_DVBT2_TUNE_PARAM_PLPID_AUTO  0xFFFF

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_Tune1(struct cxd2880_tnrdmd *pTnrDmd,
					    struct cxd2880_dvbt2_tune_param
					    *pTuneParam);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_Tune2(struct cxd2880_tnrdmd *pTnrDmd,
					    struct cxd2880_dvbt2_tune_param
					    *pTuneParam);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_SleepSetting(struct cxd2880_tnrdmd
						   *pTnrDmd);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_CheckDemodLock(struct cxd2880_tnrdmd
						     *pTnrDmd,
						     enum
						     cxd2880_tnrdmd_lock_result
						     *pLock);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_CheckTSLock(struct cxd2880_tnrdmd
						  *pTnrDmd,
						  enum
						  cxd2880_tnrdmd_lock_result
						  *pLock);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_SetPLPCfg(struct cxd2880_tnrdmd *pTnrDmd,
						u8 autoPLP, u8 plpId);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_DiverFEFSetting(struct cxd2880_tnrdmd
						      *pTnrDmd);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_CheckL1PostValid(struct cxd2880_tnrdmd
						       *pTnrDmd,
						       u8 *pL1PostValid);

#endif
