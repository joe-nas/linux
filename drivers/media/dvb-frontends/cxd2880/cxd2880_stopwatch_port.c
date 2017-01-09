/*
* cxd2880_stopwatch_port.c
* Sony CXD2880 DVB-T2/T tuner + demodulator driver
* Implementation stopwatch fucntions.
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

#include <linux/ktime.h>
#include <linux/time.h>
#include <linux/timekeeping.h>

static u32 GetTimeCount(void)
{
	struct timespec tp;

	getnstimeofday(&tp);

	return (u32) ((tp.tv_sec * 1000) + (tp.tv_nsec / 1000000));
}

enum cxd2880_ret cxd2880_stopwatch_start(struct cxd2880_stopwatch *pStopwatch)
{
	CXD2880_TRACE_ENTER("cxd2880_stopwatch_start");

	if (!pStopwatch)
		return CXD2880_RESULT_ERROR_ARG;

	pStopwatch->startTime = GetTimeCount();

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_stopwatch_sleep(struct cxd2880_stopwatch *pStopwatch,
					 u32 ms)
{
	if (!pStopwatch)
		return CXD2880_RESULT_ERROR_ARG;

	CXD2880_ARG_UNUSED(*pStopwatch);
	CXD2880_SLEEP(ms);

	return CXD2880_RESULT_OK;
}

enum cxd2880_ret cxd2880_stopwatch_elapsed(struct cxd2880_stopwatch *pStopwatch,
					   u32 *pElapsed)
{
	if (!pStopwatch || !pElapsed)
		return CXD2880_RESULT_ERROR_ARG
;
	*pElapsed = GetTimeCount() - pStopwatch->startTime;

	return CXD2880_RESULT_OK;
}
