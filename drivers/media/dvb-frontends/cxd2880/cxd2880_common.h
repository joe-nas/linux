/*
* cxd2880_common.h
* Sony CXD2880 DVB-T2/T tuner + demodulator driver common definition.
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

#ifndef CXD2880_COMMON_H
#define CXD2880_COMMON_H

#include <linux/types.h>

#ifndef NULL
#ifdef __cplusplus

#define NULL 0
#else

#define NULL ((void *)0)
#endif
#endif

#include <linux/delay.h>
#define CXD2880_SLEEP(n) msleep(n)

#ifndef CXD2880_SLEEP_IN_MON

#define CXD2880_SLEEP_IN_MON(n, obj) CXD2880_SLEEP(n)
#endif

#define CXD2880_ARG_UNUSED(arg) ((void)(arg))

enum cxd2880_ret {
	CXD2880_RESULT_OK,
	CXD2880_RESULT_ERROR_ARG,
	CXD2880_RESULT_ERROR_IO,
	CXD2880_RESULT_ERROR_SW_STATE,
	CXD2880_RESULT_ERROR_HW_STATE,
	CXD2880_RESULT_ERROR_TIMEOUT,
	CXD2880_RESULT_ERROR_UNLOCK,
	CXD2880_RESULT_ERROR_RANGE,
	CXD2880_RESULT_ERROR_NOSUPPORT,
	CXD2880_RESULT_ERROR_CANCEL,
	CXD2880_RESULT_ERROR_OTHER,
	CXD2880_RESULT_ERROR_OVERFLOW,
	CXD2880_RESULT_OK_CONFIRM
};

int cxd2880_Convert2SComplement(u32 value, u32 bitlen);

u32 cxd2880_BitSplitFromByteArray(u8 *pArray, u32 startBit, u32 bitNum);

#ifdef CXD2880_TRACE_EN

void cxd2880_trace_log_enter(const char *funcname, const char *filename,
			     unsigned int linenum);
void cxd2880_trace_log_return(enum cxd2880_ret ret, const char *filename,
			      unsigned int linenum);
#define CXD2880_TRACE_ENTER(func)
#define CXD2880_TRACE_RETURN(ret)
#else

#define CXD2880_TRACE_ENTER(func)

#define CXD2880_TRACE_RETURN(ret)

#define CXD2880_TRACE_IO_ENTER(func)

#define CXD2880_TRACE_IO_RETURN(ret)
#endif

#ifdef CXD2880_TRACE_IO_EN

void cxd2880_trace_io_log_enter(const char *funcname, const char *filename,
				unsigned int linenum);
void cxd2880_trace_io_log_return(enum cxd2880_ret ret, const char *filename,
				 unsigned int linenum);
#define CXD2880_TRACE_IO_ENTER(func)
#define CXD2880_TRACE_IO_RETURN(ret)
#else
#define CXD2880_TRACE_IO_ENTER(func)
#define CXD2880_TRACE_IO_RETURN(ret)
#endif

struct cxd2880_atomic {

	int counter;
};
#define cxd2880_atomic_set(a, i) ((a)->counter = i)
#define cxd2880_atomic_read(a) ((a)->counter)

struct cxd2880_stopwatch {

	u32 startTime;

};

enum cxd2880_ret cxd2880_stopwatch_start(struct cxd2880_stopwatch *pStopwatch);

enum cxd2880_ret cxd2880_stopwatch_sleep(struct cxd2880_stopwatch *pStopwatch,
					 u32 ms);

enum cxd2880_ret cxd2880_stopwatch_elapsed(struct cxd2880_stopwatch *pStopwatch,
					   u32 *pElapsed);

#endif
