/*
* cxd2880_integ_dvbt2.h
* Sony CXD2880 DVB-T2/T tuner + demodulator driver
* The integration layer interface for DVB-T2.
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

#ifndef CXD2880_INTEG_DVBT2_H
#define CXD2880_INTEG_DVBT2_H

#include "cxd2880_tnrdmd.h"
#include "cxd2880_tnrdmd_dvbt2.h"
#include "cxd2880_integ.h"

#define CXD2880_DVBT2_BASE_WAIT_DMD_LOCK     3500
#define CXD2880_DVBT2_BASE_WAIT_TS_LOCK	1500
#define CXD2880_DVBT2_LITE_WAIT_DMD_LOCK     5000
#define CXD2880_DVBT2_LITE_WAIT_TS_LOCK	2300
#define CXD2880_DVBT2_WAIT_LOCK_INTVL       10
#define CXD2880_DVBT2_L1POST_TIMEOUT	   500

struct cxd2880_integ_dvbt2_scan_param {

	u32 startFrequencyKHz;

	u32 endFrequencyKHz;

	u32 stepFrequencyKHz;

	enum cxd2880_dtv_bandwidth bandwidth;

	enum cxd2880_dvbt2_profile t2Profile;
};

struct cxd2880_integ_dvbt2_scan_result {

	u32 centerFreqKHz;

	enum cxd2880_ret tuneResult;

	struct cxd2880_dvbt2_tune_param dvbt2TuneParam;
};

enum cxd2880_ret cxd2880_integ_dvbt2_Tune(struct cxd2880_tnrdmd *pTnrDmd,
					  struct cxd2880_dvbt2_tune_param
					  *pTuneParam);

enum cxd2880_ret cxd2880_integ_dvbt2_WaitTSLock(struct cxd2880_tnrdmd *pTnrDmd,
						enum cxd2880_dvbt2_profile
						profile);

#endif
