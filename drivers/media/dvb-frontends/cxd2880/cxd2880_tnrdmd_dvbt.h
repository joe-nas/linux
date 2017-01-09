/*
* cxd2880_tnrdmd_dvbt.h
* Sony CXD2880 DVB-T2/T tuner + demodulator driver
* The tuner and demodulator control interface specific to DVB-T.
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

#ifndef CXD2880_TNRDMD_DVBT_H
#define CXD2880_TNRDMD_DVBT_H

#include "cxd2880_common.h"
#include "cxd2880_tnrdmd.h"

struct cxd2880_dvbt_tune_param {
	u32 centerFreqKHz;
	enum cxd2880_dtv_bandwidth bandwidth;
	enum cxd2880_dvbt_profile profile;
};

enum cxd2880_ret cxd2880_tnrdmd_dvbt_Tune1(struct cxd2880_tnrdmd *pTnrDmd,
					   struct cxd2880_dvbt_tune_param
					   *pTuneParam);

enum cxd2880_ret cxd2880_tnrdmd_dvbt_Tune2(struct cxd2880_tnrdmd *pTnrDmd,
					   struct cxd2880_dvbt_tune_param
					   *pTuneParam);

enum cxd2880_ret cxd2880_tnrdmd_dvbt_SleepSetting(struct cxd2880_tnrdmd
						  *pTnrDmd);

enum cxd2880_ret cxd2880_tnrdmd_dvbt_CheckDemodLock(struct cxd2880_tnrdmd
						    *pTnrDmd,
						    enum
						    cxd2880_tnrdmd_lock_result
						    *pLock);

enum cxd2880_ret cxd2880_tnrdmd_dvbt_CheckTSLock(struct cxd2880_tnrdmd *pTnrDmd,
						 enum cxd2880_tnrdmd_lock_result
						 *pLock);

#endif
