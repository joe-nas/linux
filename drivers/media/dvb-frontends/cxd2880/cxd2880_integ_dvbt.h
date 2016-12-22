/*
* cxd2880_integ_dvbt.h
* Sony CXD2880 DVB-T2/T tuner + demodulator driver
* The integration layer interface for DVB-T
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

#ifndef CXD2880_INTEG_DVBT_H
#define CXD2880_INTEG_DVBT_H

#include "cxd2880_tnrdmd.h"
#include "cxd2880_tnrdmd_dvbt.h"
#include "cxd2880_integ.h"

#define CXD2880_DVBT_WAIT_DMD_LOCK	   1000
#define CXD2880_DVBT_WAIT_TS_LOCK	      1000
#define CXD2880_DVBT_WAIT_LOCK_INTVL	10

struct cxd2880_integ_dvbt_scan_param {

	u32 startFrequencyKHz;

	u32 endFrequencyKHz;

	u32 stepFrequencyKHz;

	enum cxd2880_dtv_bandwidth bandwidth;

};

struct cxd2880_integ_dvbt_scan_result {

	u32 centerFreqKHz;

	enum cxd2880_ret tuneResult;

	struct cxd2880_dvbt_tune_param dvbtTuneParam;

};

enum cxd2880_ret cxd2880_integ_dvbt_Tune(struct cxd2880_tnrdmd *pTnrDmd,
					 struct cxd2880_dvbt_tune_param
					 *pTuneParam);

enum cxd2880_ret cxd2880_integ_dvbt_WaitTSLock(struct cxd2880_tnrdmd *pTnrDmd);

#endif
