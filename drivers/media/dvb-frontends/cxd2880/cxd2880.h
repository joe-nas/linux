/*
* cxd2880.h
* Sony CXD2880 DVB-T2/T tuner + demodulator driver
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

#ifndef CXD2880_H
#define CXD2880_H

#include <linux/spinlock_types.h>

#include "cxd2880_integ.h"
#include "cxd2880_integ_dvbt.h"
#include "cxd2880_dvbt.h"
#include "cxd2880_tnrdmd_mon.h"
#include "cxd2880_tnrdmd_dvbt_mon.h"
#include "cxd2880_dvbt2.h"
#include "cxd2880_integ_dvbt2.h"
#include "cxd2880_tnrdmd_dvbt2_mon.h"

struct cxd2880_priv {
	struct cxd2880_tnrdmd* tuner_demod;
	struct spi_device* spi;
	struct cxd2880_dvbt_tune_param dvbt_tune_param;
	struct cxd2880_dvbt2_tune_param dvbt2_tune_param;
	spinlock_t* slock;
	struct mutex* spi_mutex;
};

struct cxd2880_config {
	struct spi_device* spi;
	spinlock_t* slock;
	struct mutex* spi_mutex;
};

struct return_value {
	u32 dvb;
	enum cxd2880_ret cxd2880;
};

struct bw_tbl {
	u32 dvb;
	enum cxd2880_dtv_bandwidth cxd2880;
};

struct tmp_bw {
	u32 dvb;
	enum cxd2880_dtv_bandwidth cxd2880;
};

struct inversion_tbl {
	enum fe_spectral_inversion dvb_inv;
	enum cxd2880_tnrdmd_spectrum_sense sony_inv;
};

struct dvbt_modulation {
	enum fe_modulation dvb_modulation;
	enum cxd2880_dvbt_constellation sony_modulation;
};

struct dvbt_code_rate {
	enum fe_code_rate dvb_code_rate;
	enum cxd2880_dvbt_coderate sony_code_rate;
};

struct dvbt_mode {
	enum fe_transmit_mode dvb_mode;
	enum cxd2880_dvbt_mode sony_mode;
};

struct dvbt_guard {
	enum fe_guard_interval dvb_guard;
	enum cxd2880_dvbt_guard sony_guard;
};

struct dvbt_hierarchy {
	enum fe_hierarchy dvb_hierarchy;
	enum cxd2880_dvbt_hierarchy sony_hierarchy;
};

struct dvbt2_guard {
	enum fe_guard_interval dvb;
	enum cxd2880_dvbt2_guard cxd2880;
};

struct dvbt2_transmit_mode {
	enum fe_transmit_mode dvb;
	enum cxd2880_dvbt2_mode cxd2880;
};

extern struct dvb_frontend* cxd2880_attach(struct dvb_frontend* fe,
									struct cxd2880_config* cfg);

#endif /* CXD2880_H */
