/*
* cxd2880_integ_dvbt.c
* Sony CXD2880 DVB-T2/T tuner + demodulator driver
* The integration layer implementation for DVB-T.
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

#include "cxd2880_tnrdmd_dvbt.h"
#include "cxd2880_integ_dvbt.h"

static enum cxd2880_ret dvbt_WaitDemodLock(struct cxd2880_tnrdmd *pTnrDmd);

enum cxd2880_ret cxd2880_integ_dvbt_Tune(struct cxd2880_tnrdmd *pTnrDmd,
					 struct cxd2880_dvbt_tune_param
					 *pTuneParam)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;

	if ((!pTnrDmd) || (!pTuneParam))
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if ((pTnrDmd->state != CXD2880_TNRDMD_STATE_SLEEP)
	    && (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE))
		return CXD2880_RESULT_ERROR_SW_STATE;

	cxd2880_atomic_set(&(pTnrDmd->cancel), 0);

	if ((pTuneParam->bandwidth != CXD2880_DTV_BW_5_MHZ)
	    && (pTuneParam->bandwidth != CXD2880_DTV_BW_6_MHZ)
	    && (pTuneParam->bandwidth != CXD2880_DTV_BW_7_MHZ)
	    && (pTuneParam->bandwidth != CXD2880_DTV_BW_8_MHZ)) {
		return CXD2880_RESULT_ERROR_NOSUPPORT;
	}

	ret = cxd2880_tnrdmd_dvbt_Tune1(pTnrDmd, pTuneParam);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	CXD2880_SLEEP(CXD2880_TNRDMD_WAIT_AGC_STABLE);

	ret = cxd2880_tnrdmd_dvbt_Tune2(pTnrDmd, pTuneParam);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	ret = dvbt_WaitDemodLock(pTnrDmd);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	return ret;
}

enum cxd2880_ret cxd2880_integ_dvbt_WaitTSLock(struct cxd2880_tnrdmd *pTnrDmd)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;
	enum cxd2880_tnrdmd_lock_result lock =
	    CXD2880_TNRDMD_LOCK_RESULT_NOTDETECT;
	struct cxd2880_stopwatch timer;
	u8 continueWait = 1;
	u32 elapsed = 0;

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->diverMode == CXD2880_TNRDMD_DIVERMODE_SUB)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	ret = cxd2880_stopwatch_start(&timer);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	for (;;) {
		ret = cxd2880_stopwatch_elapsed(&timer, &elapsed);
		if (ret != CXD2880_RESULT_OK)
			return ret;

		if (elapsed >= CXD2880_DVBT_WAIT_TS_LOCK)
			continueWait = 0;

		ret = cxd2880_tnrdmd_dvbt_CheckTSLock(pTnrDmd, &lock);
		if (ret != CXD2880_RESULT_OK)
			return ret;

		switch (lock) {
		case CXD2880_TNRDMD_LOCK_RESULT_LOCKED:
			return CXD2880_RESULT_OK;

		case CXD2880_TNRDMD_LOCK_RESULT_UNLOCKED:
			return CXD2880_RESULT_ERROR_UNLOCK;

		default:
			break;
		}

		ret = cxd2880_integ_CheckCancellation(pTnrDmd);
		if (ret != CXD2880_RESULT_OK)
			return ret;

		if (continueWait) {
			ret =
			    cxd2880_stopwatch_sleep(&timer,
					    CXD2880_DVBT_WAIT_LOCK_INTVL);
			if (ret != CXD2880_RESULT_OK)
				return ret;
		} else {
			ret = CXD2880_RESULT_ERROR_TIMEOUT;
			break;
		}
	}

	return ret;
}

static enum cxd2880_ret dvbt_WaitDemodLock(struct cxd2880_tnrdmd *pTnrDmd)
{
	enum cxd2880_ret ret = CXD2880_RESULT_OK;
	enum cxd2880_tnrdmd_lock_result lock =
	    CXD2880_TNRDMD_LOCK_RESULT_NOTDETECT;
	struct cxd2880_stopwatch timer;
	u8 continueWait = 1;
	u32 elapsed = 0;

	if (!pTnrDmd)
		return CXD2880_RESULT_ERROR_ARG;

	if (pTnrDmd->state != CXD2880_TNRDMD_STATE_ACTIVE)
		return CXD2880_RESULT_ERROR_SW_STATE;

	ret = cxd2880_stopwatch_start(&timer);
	if (ret != CXD2880_RESULT_OK)
		return ret;

	for (;;) {
		ret = cxd2880_stopwatch_elapsed(&timer, &elapsed);
		if (ret != CXD2880_RESULT_OK)
			return ret;

		if (elapsed >= CXD2880_DVBT_WAIT_DMD_LOCK)
			continueWait = 0;

		ret = cxd2880_tnrdmd_dvbt_CheckDemodLock(pTnrDmd, &lock);
		if (ret != CXD2880_RESULT_OK)
			return ret;

		switch (lock) {
		case CXD2880_TNRDMD_LOCK_RESULT_LOCKED:
			return CXD2880_RESULT_OK;

		case CXD2880_TNRDMD_LOCK_RESULT_UNLOCKED:
			return CXD2880_RESULT_ERROR_UNLOCK;

		default:
			break;
		}

		ret = cxd2880_integ_CheckCancellation(pTnrDmd);
		if (ret != CXD2880_RESULT_OK)
			return ret;

		if (continueWait) {
			ret =
			    cxd2880_stopwatch_sleep(&timer,
					    CXD2880_DVBT_WAIT_LOCK_INTVL);
			if (ret != CXD2880_RESULT_OK)
				return ret;
		} else {
			ret = CXD2880_RESULT_ERROR_TIMEOUT;
			break;
		}
	}

	return ret;
}
