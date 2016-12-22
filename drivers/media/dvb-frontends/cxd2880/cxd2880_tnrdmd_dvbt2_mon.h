/*
* cxd2880_tnrdmd_dvbt2_mon.h
* Sony CXD2880 DVB-T2/T tuner + demodulator driver
* The DVB-T2 tuner and demodulator monitor interface.
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

#ifndef CXD2880_TNRDMD_DVBT2_MON_H
#define CXD2880_TNRDMD_DVBT2_MON_H

#include "cxd2880_tnrdmd.h"
#include "cxd2880_dvbt2.h"

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_SyncStat(struct cxd2880_tnrdmd
						   *pTnrDmd, u8 *pSyncStat,
						   u8 *pTSLockStat,
						   u8 *pUnlockDetected);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_SyncStat_sub(struct cxd2880_tnrdmd
						       *pTnrDmd, u8 *pSyncStat,
						       u8 *pUnlockDetected);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_CarrierOffset(struct cxd2880_tnrdmd
							*pTnrDmd, int *pOffset);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_CarrierOffset_sub(struct
							    cxd2880_tnrdmd
							    *pTnrDmd,
							    int *pOffset);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_L1Pre(struct cxd2880_tnrdmd *pTnrDmd,
						struct cxd2880_dvbt2_l1pre
						*pL1Pre);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_Version(struct cxd2880_tnrdmd
						  *pTnrDmd,
						  enum cxd2880_dvbt2_version
						  *pVersion);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_OFDM(struct cxd2880_tnrdmd *pTnrDmd,
					       struct cxd2880_dvbt2_ofdm
					       *pOFDM);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_DataPLPs(struct cxd2880_tnrdmd
						   *pTnrDmd, u8 *pPLPIds,
						   u8 *pNumPLPs);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_ActivePLP(struct cxd2880_tnrdmd
						    *pTnrDmd,
						    enum cxd2880_dvbt2_plp_btype
						    type,
						    struct cxd2880_dvbt2_plp
						    *pPLPInfo);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_DataPLPError(struct cxd2880_tnrdmd
						       *pTnrDmd,
						       u8 *pPLPError);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_L1Change(struct cxd2880_tnrdmd
						   *pTnrDmd, u8 *pL1Change);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_L1Post(struct cxd2880_tnrdmd *pTnrDmd,
						 struct cxd2880_dvbt2_l1post
						 *pL1Post);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_BBHeader(struct cxd2880_tnrdmd
						   *pTnrDmd,
						   enum cxd2880_dvbt2_plp_btype
						   type,
						   struct cxd2880_dvbt2_bbheader
						   *pBBHeader);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_InBandBTSRate(struct cxd2880_tnrdmd
							*pTnrDmd,
							enum
							cxd2880_dvbt2_plp_btype
							type, u32 *pTSRateBps);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_SpectrumSense(struct cxd2880_tnrdmd
						*pTnrDmd,
						enum
						cxd2880_tnrdmd_spectrum_sense
						*pSense);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_SNR(struct cxd2880_tnrdmd *pTnrDmd,
					      int *pSNR);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_SNR_diver(struct cxd2880_tnrdmd
						    *pTnrDmd, int *pSNR,
						    int *pSNRMain,
						    int *pSNRSub);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_PreLDPCBER(struct cxd2880_tnrdmd
						     *pTnrDmd, u32 *pBER);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_PostBCHFER(struct cxd2880_tnrdmd
						     *pTnrDmd, u32 *pFER);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_PreBCHBER(struct cxd2880_tnrdmd
						    *pTnrDmd, u32 *pBER);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_PacketErrorNumber(struct
							    cxd2880_tnrdmd
							    *pTnrDmd,
							    u32 *pPEN);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_SamplingOffset(struct cxd2880_tnrdmd
							 *pTnrDmd, int *pPPM);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_SamplingOffset_sub(struct
							     cxd2880_tnrdmd
							     *pTnrDmd,
							     int *pPPM);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_TSRate(struct cxd2880_tnrdmd *pTnrDmd,
						 u32 *pTSRateKbps);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_Quality(struct cxd2880_tnrdmd
						  *pTnrDmd, u8 *pQuality);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_PER(struct cxd2880_tnrdmd *pTnrDmd,
					      u32 *pPER);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_QAM(struct cxd2880_tnrdmd *pTnrDmd,
					      enum cxd2880_dvbt2_plp_btype type,
					      enum cxd2880_dvbt2_plp_constell
					      *pQAM);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_CodeRate(struct cxd2880_tnrdmd
						   *pTnrDmd,
						   enum cxd2880_dvbt2_plp_btype
						   type,
						   enum
						   cxd2880_dvbt2_plp_code_rate
						   *pCodeRate);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_Profile(struct cxd2880_tnrdmd
						  *pTnrDmd,
						  enum cxd2880_dvbt2_profile
						  *pProfile);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_SSI(struct cxd2880_tnrdmd *pTnrDmd,
					      u8 *pSSI);

enum cxd2880_ret cxd2880_tnrdmd_dvbt2_mon_SSI_sub(struct cxd2880_tnrdmd
						  *pTnrDmd, u8 *pSSI);

#endif
