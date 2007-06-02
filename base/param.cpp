/**************************************************************************
 *Copyright (C) 2002-2006 Intel Corporation
 *
 *This program is free software; you can redistribute it and/or
 *modify it under the terms of the GNU General Public License
 *as published by the Free Software Foundation; either version 2
 *of the License, or (at your option) any later version.
 *
 *This program is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *GNU General Public License for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with this program; if not, write to the Free Software
 *Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**************************************************
 * This source file is automatically generated
 * by ModelBuilder for defining all parameters.
 *************************************************/

#include "asim/param.h"
#include "asim/atoi.h"

UINT64 STOP_THREAD = 0;
UINT64 LIMIT_PTHREADS = 0;
UINT64 SIMULATED_REGION_WEIGHT = 10000;
UINT64 SIMULATED_REGION_INSTRS_REPRESENTED = 0;
string ADF_DEFAULT = "$built-in$";
UINT64 ENABLE_WARMUP = 1;
UINT64 CHECK_RUNNABLE = 10;
UINT64 CHECK_NEW_STREAM = 5;
UINT64 CHECK_EXIT = 100;
UINT64 INTERVAL_TIMER = 0;
UINT64 CYCLES_PER_PICOSECOND = 2200;
UINT64 ENABLE_INST_PROFILE = 0;
UINT64 MAX_NUM_INSTS = 100000;
UINT64 APE_ERROR_CHECKING = 0;
UINT64 USE_TRANSLATION_CACHE = 1;
UINT64 USE_UNALIAS_CACHE = 0;
UINT64 USE_UOP_DIS_CACHE = 1;
string pt = "";
string pt_dump = "";
UINT64 pipetrace_dump_max = 20000;
string stats = "";
UINT64 pt_ascii = 0;
UINT64 nthread = 4;
UINT64 ncore = 16;
UINT64 nproc = 1;
UINT64 perfchecker = 0;
UINT64 WILLY_STYLE_STATS = 0;
UINT64 instrs_retired = 0;
UINT64 ENABLE_CACHE_LOCK = 0;
UINT64 ASSERT_NON_READY_REG = 0;
UINT64 LEGACY_FCW = 0;
UINT64 CHANGE_XAPIC_ID_TO_STREAM_ID = 0;
UINT64 SHARE_IADDR_SPACE = 0;
UINT64 SHARE_DADDR_SPACE = 0;
UINT64 FEEDER_REPLICATE = 0;
UINT64 FEEDER_SKIP = 0;
UINT64 FEEDER_NTHREADS = 1;
UINT64 SKIP_WITH_WARMUP = 0;
UINT64 CHECKER_ON = 1;
UINT64 ASSERT_ON_CHECKER_ERROR = 1;
UINT64 CHECKER_MASK = 63;
UINT64 USE_SEPARATE_IMAGES = 1;
UINT64 DO_NOT_MANGLE_LOWER_BITS = 0;
UINT64 MAX_SKIP_UOP_PER_MOP_THRESHOLD = 10000000;
UINT64 REC_INST_STAT = 1;
UINT64 STRICT_ORDERING = 1;
UINT64 PROCESS_HISTORY_BUCKETS = 5000;
UINT64 LIVENESS_TIMEOUT = 10000;
UINT64 PGSDEBUG = 0;
UINT64 HACK_DOWRITE_AT_COMMIT = 0;
UINT64 WRONG_PATH = 1;
UINT64 INST_BW = 4;
UINT64 COMMIT_WIDTH = 4;
UINT64 RMT_ENABLE_NHM_SYNC = 1;
UINT64 SLACK_VALUE = 0;
UINT64 ALLOW_LOAD_STORE_PORT_SHARING = 1;
UINT64 INCREASE_LOAD_BW = 0;
UINT64 INCREASE_STORE_BW = 0;
UINT64 LDB_SIZE = 48;
UINT64 STB_SIZE = 32;
UINT64 ENABLE_MLC_AVF = 0;
UINT64 ENABLE_MLC_SCRUB_AVF = 0;
UINT64 ENABLE_RAT_AVF = 0;
UINT64 ENABLE_RAT_REC_AVF = 0;
UINT64 ENABLE_DCU_AVF = 0;
UINT64 ENABLE_DCU_SCRUB_AVF = 0;
UINT64 ENABLE_DCUBUFF_AVF = 0;
UINT64 ENABLE_LDB_AVF = 0;
UINT64 ENABLE_STB_AVF = 0;
UINT64 ENABLE_IDQ_AVF = 0;
UINT64 ENABLE_IQ_AVF = 0;
UINT64 ENABLE_BAC_AVF = 0;
UINT64 ENABLE_ID_AVF = 0;
UINT64 ENABLE_RRF_AVF = 0;
UINT64 ENABLE_RS_AVF = 0;
UINT64 ENABLE_EXEC_AVF = 0;
UINT64 ENABLE_ROB_AVF = 0;
UINT64 ENABLE_IC_AVF = 0;
UINT64 ENABLE_ISB_AVF = 0;
UINT64 ENABLE_IVC_AVF = 0;
UINT64 ENABLE_AVF = 0;
UINT64 ENABLE_AVF_TRACE = 0;
UINT64 AVF_RS_TRACE = 0;
UINT64 IN_ORDER_MEM_DISPATCH = 0;
UINT64 SKIP_RRSB_RETIRE_CHECKS = 0;
UINT64 USE_PARTIAL_AWIN = 1;
UINT64 CONTROL_REGISTER_WRITE_LATENCY = 3;
UINT64 ENABLE_LOAD_TO_STD_OPT = 0;
UINT64 CORRECT_PB_DIV_REPRATE = 1;
UINT64 TRACE_TINY_INST_REFCOUNT = 0;
UINT64 ENABLE_MEMORY_DISAMBIGUATOR = 0;
UINT64 ENHANCED_LOOSENET_CHECKING = 1;
UINT64 ENABLE_PERFECT_STLB = 0;
UINT64 STLB_PER_THREAD = 0;
UINT64 STLB_LOG2_PAGE_SIZE = 12;
UINT64 STLB_WARM_PERCENT = 0;
UINT64 STLB_ONLY_FILL_4K_PAGES = 1;
UINT64 ENABLE_PERFECT_PDE = 0;
UINT64 PDE_PER_THREAD = 0;
UINT64 PDE_LOG2_PAGE_SIZE = 12;
UINT64 PDE_WARM_PERCENT = 0;
UINT64 ENABLE_PERFECT_PDP = 0;
UINT64 PDP_PER_THREAD = 0;
UINT64 PDP_LOG2_PAGE_SIZE = 12;
UINT64 PDP_WARM_PERCENT = 0;
UINT64 PRED_MEM_LATENCY = 9;
UINT64 ENABLE_STREAMER_PREFETCH = 1;
UINT64 ENABLE_STRIDE_PREFETCH = 1;
UINT64 MD_COUNTER_SATURATION = 15;
UINT64 MD_NUM_ENTRIES = 256;
UINT64 MD_IDX_MASK = 255;
UINT64 MD_IDX_HWC_POS = 7;
UINT64 MD_IDX_HWC_MASK = 1;
UINT64 ALLOW_STORE_FORWARDING_WITH_VIRTUAL_ADDRESS = 1;
UINT64 ENABLE_PERFECT_DTLB = 0;
UINT64 FORCE_XAPIC_UC = 1;
UINT64 DTLB_SMALL_PER_THREAD = 0;
UINT64 DTLB_SMALL_WARM_PERCENT = 0;
UINT64 DTLB_SMALL_TAG_BITS = 32;
UINT64 DTLB_SMALL_EXTRA_BITS_PER_WAY = 3;
UINT64 DTLB_SMALL_EXTRA_BITS = 3;
UINT64 DTLB_SMALL_READ_PORTS = 2;
UINT64 DTLB_SMALL_WRITE_PORTS = 1;
UINT64 DTLB_LARGE_PER_THREAD = 0;
UINT64 DTLB_LARGE_WARM_PERCENT = 0;
UINT64 DTLB_LARGE_TAG_BITS = 32;
UINT64 DTLB_LARGE_EXTRA_BITS_PER_WAY = 3;
UINT64 DTLB_LARGE_EXTRA_BITS = 3;
UINT64 DTLB_LARGE_READ_PORTS = 2;
UINT64 DTLB_LARGE_WRITE_PORTS = 1;
UINT64 MLC_SUPER_QUEUE_SIZE = 16;
UINT64 MLC_SNOOP_QUEUE_SIZE = 16;
UINT64 SNOOPQ_CREDITS = 8;
UINT64 IMQ_CREDITS = 8;
UINT64 SQ_ENTRIES = 16;
UINT64 WOB_ENTRIES = 16;
UINT64 ENABLE_MLC_PFC_STREAMER = 0;
UINT64 ENABLE_MLC_PFC_SPATIAL = 0;
UINT64 MLC_PFC_STREAMER_FWD_THRESH = 1;
UINT64 MLC_PFC_STREAMER_BWD_THRESH = 2;
UINT64 MLC_PFC_STREAMER_FAR_DIST = 32;
UINT64 MLC_PFC_STREAMER_GLOBAL_TRAIN_BITS = 6;
UINT64 MLC_PFC_STREAMER_PFDIST_MAX = 12;
UINT64 MLC_PFC_STREAMER_PFDIST_MIN = 2;
UINT64 MLC_PFC_STREAMER_DETECTOR_TRAIN_MAX = 3;
UINT64 MLC_PFC_STREAMER_DETECTOR_CONFIDENCE_INC = 4;
UINT64 MLC_PFC_STREAMER_DETECTOR_CONFIDENCE_DEC = 1;
UINT64 SQ_WOB_ENTRY_LIVENESS_TIMEOUT = 200000;
UINT64 SQ_MLC_MAX_REJECTS = 10000;
UINT64 IDI_LLC_UC_LD_LATENCY = 228;
UINT64 IDI_LLC_UC_ST_LATENCY = 155;
UINT64 IDI_LLC_HIT_LATENCY = 29;
UINT64 IDI_LLC_GO_AHEAD_OF_DATA = 3;
UINT64 IDI_LLC_ALWAYS_EXLCUSIVE = 1;
UINT64 USE_BP = 1;
UINT64 USE_uBP = 1;
UINT64 IQ_ARRAY_BW = 136;
UINT64 NUM_INST_INSERTED_PER_CYCLE = 6;
UINT64 IDQUED_WIDTH = 135;
UINT64 IDQCTLM_WIDTH = 9;
UINT64 IDQIMMD_WIDTH = 92;
UINT64 IDQ_WRITE_PTS = 7;
UINT64 ID_2_ID_BW = 4;
UINT64 SERIAL_ON_UID = 0;
UINT64 SERIALIZE = 0;
UINT64 AUOP_BIT_WIDTH = 137;
UINT64 MICRO_SEQUENCER_BW = 308;
UINT64 MICRO_SEQUENCER_ENTRIES = 6144;
UINT64 FUSE_ESP_FOLD = 1;
UINT64 FUSE_TJS = 0;
UINT64 FUSE_P6 = 1;
UINT64 FUSE_MRM_LD_OP = 1;
UINT64 FUSE_MRM_LD_JMP = 1;
UINT64 FUSE_MRM_IMM_DISP = 1;
UINT64 FUSE_NHM_MS_STORE = 1;
UINT64 FUSE_NHM = 1;
UINT64 FUSE_MM_IMM8 = 0;
UINT64 ENABLE_PERFECT_ITLB = 0;
UINT64 ITLB_SMALL_PER_THREAD = 0;
UINT64 ITLB_SMALL_WARM_PERCENT = 0;
UINT64 ITLB_LARGE_PER_THREAD = 0;
UINT64 ITLB_LARGE_WARM_PERCENT = 0;
UINT64 ENABLE_OPP_FETCHES = 0;
UINT64 BIT_ARRAY_WRITE_LATENCY = 2;
UINT64 BIT_ARRAY_READ_LATENCY = 1;
UINT64 UPDATE_STEW_ON_RETURNS = 1;
UINT64 UPDATE_STEW_ON_UNCONDITIONALS = 1;
UINT64 CLEAR_IBTB_ON_BOGUS = 0;
UINT64 BAC_RSB_PER_THREAD = 1;
UINT64 STATIC_PREDICTION_DISPLACEMENT = 256;
UINT64 ROB_SIZE = 128;
UINT64 RS_SIZE = 36;
UINT64 NUM_BCAST = 0;
UINT64 BLOCK_ROB = 0;
UINT64 FUSED_ROB = 0;
UINT64 BLOCK_RS = 0;
UINT64 FUSED_RS = 0;
UINT64 MOVE_ELIM = 0;
UINT64 ZERO_ELIM = 0;
string RS_PARTITION_STEERING = "0";
string RS_PARTITION_SIZES = "0";
UINT64 RS_ELIGIBLE = 0;
UINT64 RS_LOAD_DEP_TIMER = 0;
UINT64 NO_SPEC_MEM_ISSUE = 1;
UINT64 ENABLE_VAR_EXEC_FIXED = 0;
UINT64 ADVANCED_RS = 0;
UINT64 ENFORCE_PARTIAL_FLAG_STALLS = 1;
UINT64 ENFORCE_PARTIAL_READ_STALLS = 0;
UINT64 ALLOC_MAX_BRANCHES = 0;
UINT64 ALLOC_FXCHG_CUT = 1;
UINT64 RS_2_EXEC_LATENCY = 1;
UINT64 RS_2_ROB_LATENCY = 1;
UINT64 EXEC_2_DIVIDER_LATENCY = 1;
UINT64 MS_2_BE_DISPATCHED_LD_LATENCY = 3;
UINT64 MS_2_BE_INVALID_LD_LATENCY = 0;
UINT64 FE_2_BE_FETCHED_INST_LATENCY = 1;
UINT64 EXEC_2_ROB_LATENCY = 3;
UINT64 ROB_2_ALLOC_RAT1_COMMITTED_INST_LATENCY = 1;
UINT64 ROB_2_ALLOC_RAT1_DISPATCHED_INST_LATENCY = 1;
UINT64 ROB_2_ALLOC_RAT1_WB_INST_LATENCY = 1;
UINT64 EH_2_BE_SIGNALED_EXCEPTIONS_LATENCY = 1;
UINT64 RS_2_ALLOC_STATE_LATENCY = 1;
UINT64 ROB_2_ALLOC_STATE_LATENCY = 1;
UINT64 STB_2_ALLOC_STATE_LATENCY = 1;
UINT64 LDB_2_ALLOC_STATE_LATENCY = 1;
UINT64 BE_2_EH_EXCEPTION_LATENCY = 1;
UINT64 EXCEPTION_2_ALLOC_RAT1_LATENCY = 1;
UINT64 CHECK_PORTBINDING_VS_FEEDER = 0;
UINT64 ESPR_BIT_WIDTH = 11;
UINT64 ESPR_READ_PORTS = 1;
UINT64 ESPR_JECLEAR_LATENCY = 4;
UINT64 ESPR_RONUKE_LATENCY = 3;
UINT64 ESPR_ALLOC_LATENCY = 1;
UINT64 ALLOC_RAT1_2_RAT2_LATENCY = 1;
UINT64 RAT2_2_RS_LATENCY = 1;
UINT64 EH_2_RS_LATENCY = 1;
UINT64 RAT2_2_ROB_LATENCY = 1;
UINT64 EH_2_ROB_LATENCY = 1;
UINT64 MS_2_BE_COMPLETED_MEM_OP_LATENCY = 1;
UINT64 ENABLE_ANALYSIS_WINDOW = 0;
UINT64 CAPS_STALL_THRESHOLD = 100;
UINT64 CAPS_SECONDARY_STALL_THRESHOLD = 0;
UINT64 ENABLE_CAPS_FLUSH = 0;
UINT64 RONUKE_STYLE_CAPS_FLUSH = 0;
UINT64 COMMIT_TIME_CAPS = 0;
UINT64 ENABLE_PRIMARY = 1;
UINT64 ENABLE_LOGICAL = 0;
UINT64 ENABLE_COLLAPSING = 1;
UINT64 ENABLE_BACK_MASKING = 0;
UINT64 ENABLE_STORE_ANALYSIS_1 = 0;
UINT64 ENABLE_STORE_ANALYSIS_2 = 0;
UINT64 ENABLE_INST_PORT = 0;
UINT64 ENABLE_PENRYN_INTEGER_DIVIDER = 1;
UINT64 ENABLE_MEROM_INTEGER_DIVIDER = 0;
UINT64 IDIV_QUOTIENT_BITS_PER_CYCLE = 4;
UINT64 IDIV_MIN_LATENCY = 9;
UINT64 IDIV_POSTPROCESSING_LATENCY = 4;
UINT64 IDIV_PREPROCESSING_LATENCY = 2;
UINT64 ENABLE_EARLY_OUT_FDIV0 = 1;
UINT64 ENABLE_EARLY_OUT_FDIV_BY_POW2 = 1;
UINT64 ENABLE_EARLY_OUT_FSQRT0 = 1;
UINT64 ENABLE_EARLY_OUT_FSQRT_BY_POW4 = 1;
UINT64 ENABLE_EARLY_OUT_SSE0 = 1;
UINT64 ENABLE_EARLY_OUT_SSE_DIV_BY_POW2 = 1;
UINT64 ENABLE_EARLY_OUT_SSE_SQRT_BY_POW4 = 1;
UINT64 ENABLE_FORCE_IDIV = 0;
UINT64 ENABLE_FORCE_FDIV = 0;
UINT64 ENABLE_FORCE_SDIV = 0;
UINT64 ENABLE_FORCE_FSQRT = 0;
UINT64 ENABLE_FORCE_SSQRT = 0;
UINT64 ENABLE_FORCE_FREM = 0;
UINT64 FORCE_IDIV_LATENCY = 10;
UINT64 FORCE_FDIV_LATENCY = 10;
UINT64 FORCE_SDIV_LATENCY = 10;
UINT64 FORCE_FSQRT_LATENCY = 10;
UINT64 FORCE_SSQRT_LATENCY = 10;
UINT64 FORCE_FREM_LATENCY = 10;
UINT64 EXEC_2_RE_PREDICTOR_LATENCY = 1;
UINT64 MS_2_RE_PREDICTOR_LATENCY = 1;
UINT64 ROB_2_EH_LATENCY = 0;
UINT64 EXEC_2_EH_LATENCY = 0;
UINT64 RAT_AVF_SIMULATE_COUNT = 10000000;
UINT64 RAT_AVF_COOLDOWN_COUNT = 0;
UINT64 RAT_AVF_WARMUP_COUNT = 0;
UINT64 RAT_AVF_GC_INTERVAL = 50000;
UINT64 RAT_REC_AVF_SIMULATE_COUNT = 10000000;
UINT64 RAT_REC_AVF_COOLDOWN_COUNT = 0;
UINT64 RAT_REC_AVF_WARMUP_COUNT = 0;
UINT64 RAT_REC_AVF_GC_INTERVAL = 50000;
UINT64 RRF_AVF_SIMULATE_COUNT = 10000000;
UINT64 RRF_AVF_WARMUP_COUNT = 10000000;
UINT64 RRF_AVF_COOLDOWN_COUNT = 10000000;


bool SetParam (char * name, char * value)
{
    bool found = true;
    if (strcmp(name, "STOP_THREAD") == 0) {
        STOP_THREAD = atoi_general(value);
    }
    else if (strcmp(name, "LIMIT_PTHREADS") == 0) {
        LIMIT_PTHREADS = atoi_general(value);
    }
    else if (strcmp(name, "SIMULATED_REGION_WEIGHT") == 0) {
        SIMULATED_REGION_WEIGHT = atoi_general(value);
    }
    else if (strcmp(name, "SIMULATED_REGION_INSTRS_REPRESENTED") == 0) {
        SIMULATED_REGION_INSTRS_REPRESENTED = atoi_general(value);
    }
    else if (strcmp(name, "ADF_DEFAULT") == 0) {
        ADF_DEFAULT = value;
    }
    else if (strcmp(name, "ENABLE_WARMUP") == 0) {
        ENABLE_WARMUP = atoi_general(value);
    }
    else if (strcmp(name, "CHECK_RUNNABLE") == 0) {
        CHECK_RUNNABLE = atoi_general(value);
    }
    else if (strcmp(name, "CHECK_NEW_STREAM") == 0) {
        CHECK_NEW_STREAM = atoi_general(value);
    }
    else if (strcmp(name, "CHECK_EXIT") == 0) {
        CHECK_EXIT = atoi_general(value);
    }
    else if (strcmp(name, "INTERVAL_TIMER") == 0) {
        INTERVAL_TIMER = atoi_general(value);
    }
    else if (strcmp(name, "CYCLES_PER_PICOSECOND") == 0) {
        CYCLES_PER_PICOSECOND = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_INST_PROFILE") == 0) {
        ENABLE_INST_PROFILE = atoi_general(value);
    }
    else if (strcmp(name, "MAX_NUM_INSTS") == 0) {
        MAX_NUM_INSTS = atoi_general(value);
    }
    else if (strcmp(name, "APE_ERROR_CHECKING") == 0) {
        APE_ERROR_CHECKING = atoi_general(value);
    }
    else if (strcmp(name, "USE_TRANSLATION_CACHE") == 0) {
        USE_TRANSLATION_CACHE = atoi_general(value);
    }
    else if (strcmp(name, "USE_UNALIAS_CACHE") == 0) {
        USE_UNALIAS_CACHE = atoi_general(value);
    }
    else if (strcmp(name, "USE_UOP_DIS_CACHE") == 0) {
        USE_UOP_DIS_CACHE = atoi_general(value);
    }
    else if (strcmp(name, "pt") == 0) {
        pt = value;
    }
    else if (strcmp(name, "pt_dump") == 0) {
        pt_dump = value;
    }
    else if (strcmp(name, "pipetrace_dump_max") == 0) {
        pipetrace_dump_max = atoi_general(value);
    }
    else if (strcmp(name, "stats") == 0) {
        stats = value;
    }
    else if (strcmp(name, "pt_ascii") == 0) {
        pt_ascii = atoi_general(value);
    }
    else if (strcmp(name, "nthread") == 0) {
        nthread = atoi_general(value);
    }
    else if (strcmp(name, "ncore") == 0) {
        ncore = atoi_general(value);
    }
    else if (strcmp(name, "nproc") == 0) {
        nproc = atoi_general(value);
    }
    else if (strcmp(name, "perfchecker") == 0) {
        perfchecker = atoi_general(value);
    }
    else if (strcmp(name, "WILLY_STYLE_STATS") == 0) {
        WILLY_STYLE_STATS = atoi_general(value);
    }
    else if (strcmp(name, "instrs_retired") == 0) {
        instrs_retired = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_CACHE_LOCK") == 0) {
        ENABLE_CACHE_LOCK = atoi_general(value);
    }
    else if (strcmp(name, "ASSERT_NON_READY_REG") == 0) {
        ASSERT_NON_READY_REG = atoi_general(value);
    }
    else if (strcmp(name, "LEGACY_FCW") == 0) {
        LEGACY_FCW = atoi_general(value);
    }
    else if (strcmp(name, "CHANGE_XAPIC_ID_TO_STREAM_ID") == 0) {
        CHANGE_XAPIC_ID_TO_STREAM_ID = atoi_general(value);
    }
    else if (strcmp(name, "SHARE_IADDR_SPACE") == 0) {
        SHARE_IADDR_SPACE = atoi_general(value);
    }
    else if (strcmp(name, "SHARE_DADDR_SPACE") == 0) {
        SHARE_DADDR_SPACE = atoi_general(value);
    }
    else if (strcmp(name, "FEEDER_REPLICATE") == 0) {
        FEEDER_REPLICATE = atoi_general(value);
    }
    else if (strcmp(name, "FEEDER_SKIP") == 0) {
        FEEDER_SKIP = atoi_general(value);
    }
    else if (strcmp(name, "FEEDER_NTHREADS") == 0) {
        FEEDER_NTHREADS = atoi_general(value);
    }
    else if (strcmp(name, "SKIP_WITH_WARMUP") == 0) {
        SKIP_WITH_WARMUP = atoi_general(value);
    }
    else if (strcmp(name, "CHECKER_ON") == 0) {
        CHECKER_ON = atoi_general(value);
    }
    else if (strcmp(name, "ASSERT_ON_CHECKER_ERROR") == 0) {
        ASSERT_ON_CHECKER_ERROR = atoi_general(value);
    }
    else if (strcmp(name, "CHECKER_MASK") == 0) {
        CHECKER_MASK = atoi_general(value);
    }
    else if (strcmp(name, "USE_SEPARATE_IMAGES") == 0) {
        USE_SEPARATE_IMAGES = atoi_general(value);
    }
    else if (strcmp(name, "DO_NOT_MANGLE_LOWER_BITS") == 0) {
        DO_NOT_MANGLE_LOWER_BITS = atoi_general(value);
    }
    else if (strcmp(name, "MAX_SKIP_UOP_PER_MOP_THRESHOLD") == 0) {
        MAX_SKIP_UOP_PER_MOP_THRESHOLD = atoi_general(value);
    }
    else if (strcmp(name, "REC_INST_STAT") == 0) {
        REC_INST_STAT = atoi_general(value);
    }
    else if (strcmp(name, "STRICT_ORDERING") == 0) {
        STRICT_ORDERING = atoi_general(value);
    }
    else if (strcmp(name, "PROCESS_HISTORY_BUCKETS") == 0) {
        PROCESS_HISTORY_BUCKETS = atoi_general(value);
    }
    else if (strcmp(name, "LIVENESS_TIMEOUT") == 0) {
        LIVENESS_TIMEOUT = atoi_general(value);
    }
    else if (strcmp(name, "PGSDEBUG") == 0) {
        PGSDEBUG = atoi_general(value);
    }
    else if (strcmp(name, "HACK_DOWRITE_AT_COMMIT") == 0) {
        HACK_DOWRITE_AT_COMMIT = atoi_general(value);
    }
    else if (strcmp(name, "WRONG_PATH") == 0) {
        WRONG_PATH = atoi_general(value);
    }
    else if (strcmp(name, "INST_BW") == 0) {
        INST_BW = atoi_general(value);
    }
    else if (strcmp(name, "COMMIT_WIDTH") == 0) {
        COMMIT_WIDTH = atoi_general(value);
    }
    else if (strcmp(name, "RMT_ENABLE_NHM_SYNC") == 0) {
        RMT_ENABLE_NHM_SYNC = atoi_general(value);
    }
    else if (strcmp(name, "SLACK_VALUE") == 0) {
        SLACK_VALUE = atoi_general(value);
    }
    else if (strcmp(name, "ALLOW_LOAD_STORE_PORT_SHARING") == 0) {
        ALLOW_LOAD_STORE_PORT_SHARING = atoi_general(value);
    }
    else if (strcmp(name, "INCREASE_LOAD_BW") == 0) {
        INCREASE_LOAD_BW = atoi_general(value);
    }
    else if (strcmp(name, "INCREASE_STORE_BW") == 0) {
        INCREASE_STORE_BW = atoi_general(value);
    }
    else if (strcmp(name, "LDB_SIZE") == 0) {
        LDB_SIZE = atoi_general(value);
    }
    else if (strcmp(name, "STB_SIZE") == 0) {
        STB_SIZE = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_MLC_AVF") == 0) {
        ENABLE_MLC_AVF = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_MLC_SCRUB_AVF") == 0) {
        ENABLE_MLC_SCRUB_AVF = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_RAT_AVF") == 0) {
        ENABLE_RAT_AVF = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_RAT_REC_AVF") == 0) {
        ENABLE_RAT_REC_AVF = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_DCU_AVF") == 0) {
        ENABLE_DCU_AVF = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_DCU_SCRUB_AVF") == 0) {
        ENABLE_DCU_SCRUB_AVF = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_DCUBUFF_AVF") == 0) {
        ENABLE_DCUBUFF_AVF = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_LDB_AVF") == 0) {
        ENABLE_LDB_AVF = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_STB_AVF") == 0) {
        ENABLE_STB_AVF = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_IDQ_AVF") == 0) {
        ENABLE_IDQ_AVF = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_IQ_AVF") == 0) {
        ENABLE_IQ_AVF = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_BAC_AVF") == 0) {
        ENABLE_BAC_AVF = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_ID_AVF") == 0) {
        ENABLE_ID_AVF = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_RRF_AVF") == 0) {
        ENABLE_RRF_AVF = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_RS_AVF") == 0) {
        ENABLE_RS_AVF = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_EXEC_AVF") == 0) {
        ENABLE_EXEC_AVF = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_ROB_AVF") == 0) {
        ENABLE_ROB_AVF = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_IC_AVF") == 0) {
        ENABLE_IC_AVF = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_ISB_AVF") == 0) {
        ENABLE_ISB_AVF = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_IVC_AVF") == 0) {
        ENABLE_IVC_AVF = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_AVF") == 0) {
        ENABLE_AVF = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_AVF_TRACE") == 0) {
        ENABLE_AVF_TRACE = atoi_general(value);
    }
    else if (strcmp(name, "AVF_RS_TRACE") == 0) {
        AVF_RS_TRACE = atoi_general(value);
    }
    else if (strcmp(name, "IN_ORDER_MEM_DISPATCH") == 0) {
        IN_ORDER_MEM_DISPATCH = atoi_general(value);
    }
    else if (strcmp(name, "SKIP_RRSB_RETIRE_CHECKS") == 0) {
        SKIP_RRSB_RETIRE_CHECKS = atoi_general(value);
    }
    else if (strcmp(name, "USE_PARTIAL_AWIN") == 0) {
        USE_PARTIAL_AWIN = atoi_general(value);
    }
    else if (strcmp(name, "CONTROL_REGISTER_WRITE_LATENCY") == 0) {
        CONTROL_REGISTER_WRITE_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_LOAD_TO_STD_OPT") == 0) {
        ENABLE_LOAD_TO_STD_OPT = atoi_general(value);
    }
    else if (strcmp(name, "CORRECT_PB_DIV_REPRATE") == 0) {
        CORRECT_PB_DIV_REPRATE = atoi_general(value);
    }
    else if (strcmp(name, "TRACE_TINY_INST_REFCOUNT") == 0) {
        TRACE_TINY_INST_REFCOUNT = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_MEMORY_DISAMBIGUATOR") == 0) {
        ENABLE_MEMORY_DISAMBIGUATOR = atoi_general(value);
    }
    else if (strcmp(name, "ENHANCED_LOOSENET_CHECKING") == 0) {
        ENHANCED_LOOSENET_CHECKING = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_PERFECT_STLB") == 0) {
        ENABLE_PERFECT_STLB = atoi_general(value);
    }
    else if (strcmp(name, "STLB_PER_THREAD") == 0) {
        STLB_PER_THREAD = atoi_general(value);
    }
    else if (strcmp(name, "STLB_LOG2_PAGE_SIZE") == 0) {
        STLB_LOG2_PAGE_SIZE = atoi_general(value);
    }
    else if (strcmp(name, "STLB_WARM_PERCENT") == 0) {
        STLB_WARM_PERCENT = atoi_general(value);
    }
    else if (strcmp(name, "STLB_ONLY_FILL_4K_PAGES") == 0) {
        STLB_ONLY_FILL_4K_PAGES = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_PERFECT_PDE") == 0) {
        ENABLE_PERFECT_PDE = atoi_general(value);
    }
    else if (strcmp(name, "PDE_PER_THREAD") == 0) {
        PDE_PER_THREAD = atoi_general(value);
    }
    else if (strcmp(name, "PDE_LOG2_PAGE_SIZE") == 0) {
        PDE_LOG2_PAGE_SIZE = atoi_general(value);
    }
    else if (strcmp(name, "PDE_WARM_PERCENT") == 0) {
        PDE_WARM_PERCENT = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_PERFECT_PDP") == 0) {
        ENABLE_PERFECT_PDP = atoi_general(value);
    }
    else if (strcmp(name, "PDP_PER_THREAD") == 0) {
        PDP_PER_THREAD = atoi_general(value);
    }
    else if (strcmp(name, "PDP_LOG2_PAGE_SIZE") == 0) {
        PDP_LOG2_PAGE_SIZE = atoi_general(value);
    }
    else if (strcmp(name, "PDP_WARM_PERCENT") == 0) {
        PDP_WARM_PERCENT = atoi_general(value);
    }
    else if (strcmp(name, "PRED_MEM_LATENCY") == 0) {
        PRED_MEM_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_STREAMER_PREFETCH") == 0) {
        ENABLE_STREAMER_PREFETCH = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_STRIDE_PREFETCH") == 0) {
        ENABLE_STRIDE_PREFETCH = atoi_general(value);
    }
    else if (strcmp(name, "MD_COUNTER_SATURATION") == 0) {
        MD_COUNTER_SATURATION = atoi_general(value);
    }
    else if (strcmp(name, "MD_NUM_ENTRIES") == 0) {
        MD_NUM_ENTRIES = atoi_general(value);
    }
    else if (strcmp(name, "MD_IDX_MASK") == 0) {
        MD_IDX_MASK = atoi_general(value);
    }
    else if (strcmp(name, "MD_IDX_HWC_POS") == 0) {
        MD_IDX_HWC_POS = atoi_general(value);
    }
    else if (strcmp(name, "MD_IDX_HWC_MASK") == 0) {
        MD_IDX_HWC_MASK = atoi_general(value);
    }
    else if (strcmp(name, "ALLOW_STORE_FORWARDING_WITH_VIRTUAL_ADDRESS") == 0) {
        ALLOW_STORE_FORWARDING_WITH_VIRTUAL_ADDRESS = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_PERFECT_DTLB") == 0) {
        ENABLE_PERFECT_DTLB = atoi_general(value);
    }
    else if (strcmp(name, "FORCE_XAPIC_UC") == 0) {
        FORCE_XAPIC_UC = atoi_general(value);
    }
    else if (strcmp(name, "DTLB_SMALL_PER_THREAD") == 0) {
        DTLB_SMALL_PER_THREAD = atoi_general(value);
    }
    else if (strcmp(name, "DTLB_SMALL_WARM_PERCENT") == 0) {
        DTLB_SMALL_WARM_PERCENT = atoi_general(value);
    }
    else if (strcmp(name, "DTLB_SMALL_TAG_BITS") == 0) {
        DTLB_SMALL_TAG_BITS = atoi_general(value);
    }
    else if (strcmp(name, "DTLB_SMALL_EXTRA_BITS_PER_WAY") == 0) {
        DTLB_SMALL_EXTRA_BITS_PER_WAY = atoi_general(value);
    }
    else if (strcmp(name, "DTLB_SMALL_EXTRA_BITS") == 0) {
        DTLB_SMALL_EXTRA_BITS = atoi_general(value);
    }
    else if (strcmp(name, "DTLB_SMALL_READ_PORTS") == 0) {
        DTLB_SMALL_READ_PORTS = atoi_general(value);
    }
    else if (strcmp(name, "DTLB_SMALL_WRITE_PORTS") == 0) {
        DTLB_SMALL_WRITE_PORTS = atoi_general(value);
    }
    else if (strcmp(name, "DTLB_LARGE_PER_THREAD") == 0) {
        DTLB_LARGE_PER_THREAD = atoi_general(value);
    }
    else if (strcmp(name, "DTLB_LARGE_WARM_PERCENT") == 0) {
        DTLB_LARGE_WARM_PERCENT = atoi_general(value);
    }
    else if (strcmp(name, "DTLB_LARGE_TAG_BITS") == 0) {
        DTLB_LARGE_TAG_BITS = atoi_general(value);
    }
    else if (strcmp(name, "DTLB_LARGE_EXTRA_BITS_PER_WAY") == 0) {
        DTLB_LARGE_EXTRA_BITS_PER_WAY = atoi_general(value);
    }
    else if (strcmp(name, "DTLB_LARGE_EXTRA_BITS") == 0) {
        DTLB_LARGE_EXTRA_BITS = atoi_general(value);
    }
    else if (strcmp(name, "DTLB_LARGE_READ_PORTS") == 0) {
        DTLB_LARGE_READ_PORTS = atoi_general(value);
    }
    else if (strcmp(name, "DTLB_LARGE_WRITE_PORTS") == 0) {
        DTLB_LARGE_WRITE_PORTS = atoi_general(value);
    }
    else if (strcmp(name, "MLC_SUPER_QUEUE_SIZE") == 0) {
        MLC_SUPER_QUEUE_SIZE = atoi_general(value);
    }
    else if (strcmp(name, "MLC_SNOOP_QUEUE_SIZE") == 0) {
        MLC_SNOOP_QUEUE_SIZE = atoi_general(value);
    }
    else if (strcmp(name, "SNOOPQ_CREDITS") == 0) {
        SNOOPQ_CREDITS = atoi_general(value);
    }
    else if (strcmp(name, "IMQ_CREDITS") == 0) {
        IMQ_CREDITS = atoi_general(value);
    }
    else if (strcmp(name, "SQ_ENTRIES") == 0) {
        SQ_ENTRIES = atoi_general(value);
    }
    else if (strcmp(name, "WOB_ENTRIES") == 0) {
        WOB_ENTRIES = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_MLC_PFC_STREAMER") == 0) {
        ENABLE_MLC_PFC_STREAMER = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_MLC_PFC_SPATIAL") == 0) {
        ENABLE_MLC_PFC_SPATIAL = atoi_general(value);
    }
    else if (strcmp(name, "MLC_PFC_STREAMER_FWD_THRESH") == 0) {
        MLC_PFC_STREAMER_FWD_THRESH = atoi_general(value);
    }
    else if (strcmp(name, "MLC_PFC_STREAMER_BWD_THRESH") == 0) {
        MLC_PFC_STREAMER_BWD_THRESH = atoi_general(value);
    }
    else if (strcmp(name, "MLC_PFC_STREAMER_FAR_DIST") == 0) {
        MLC_PFC_STREAMER_FAR_DIST = atoi_general(value);
    }
    else if (strcmp(name, "MLC_PFC_STREAMER_GLOBAL_TRAIN_BITS") == 0) {
        MLC_PFC_STREAMER_GLOBAL_TRAIN_BITS = atoi_general(value);
    }
    else if (strcmp(name, "MLC_PFC_STREAMER_PFDIST_MAX") == 0) {
        MLC_PFC_STREAMER_PFDIST_MAX = atoi_general(value);
    }
    else if (strcmp(name, "MLC_PFC_STREAMER_PFDIST_MIN") == 0) {
        MLC_PFC_STREAMER_PFDIST_MIN = atoi_general(value);
    }
    else if (strcmp(name, "MLC_PFC_STREAMER_DETECTOR_TRAIN_MAX") == 0) {
        MLC_PFC_STREAMER_DETECTOR_TRAIN_MAX = atoi_general(value);
    }
    else if (strcmp(name, "MLC_PFC_STREAMER_DETECTOR_CONFIDENCE_INC") == 0) {
        MLC_PFC_STREAMER_DETECTOR_CONFIDENCE_INC = atoi_general(value);
    }
    else if (strcmp(name, "MLC_PFC_STREAMER_DETECTOR_CONFIDENCE_DEC") == 0) {
        MLC_PFC_STREAMER_DETECTOR_CONFIDENCE_DEC = atoi_general(value);
    }
    else if (strcmp(name, "SQ_WOB_ENTRY_LIVENESS_TIMEOUT") == 0) {
        SQ_WOB_ENTRY_LIVENESS_TIMEOUT = atoi_general(value);
    }
    else if (strcmp(name, "SQ_MLC_MAX_REJECTS") == 0) {
        SQ_MLC_MAX_REJECTS = atoi_general(value);
    }
    else if (strcmp(name, "IDI_LLC_UC_LD_LATENCY") == 0) {
        IDI_LLC_UC_LD_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "IDI_LLC_UC_ST_LATENCY") == 0) {
        IDI_LLC_UC_ST_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "IDI_LLC_HIT_LATENCY") == 0) {
        IDI_LLC_HIT_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "IDI_LLC_GO_AHEAD_OF_DATA") == 0) {
        IDI_LLC_GO_AHEAD_OF_DATA = atoi_general(value);
    }
    else if (strcmp(name, "IDI_LLC_ALWAYS_EXLCUSIVE") == 0) {
        IDI_LLC_ALWAYS_EXLCUSIVE = atoi_general(value);
    }
    else if (strcmp(name, "USE_BP") == 0) {
        USE_BP = atoi_general(value);
    }
    else if (strcmp(name, "USE_uBP") == 0) {
        USE_uBP = atoi_general(value);
    }
    else if (strcmp(name, "IQ_ARRAY_BW") == 0) {
        IQ_ARRAY_BW = atoi_general(value);
    }
    else if (strcmp(name, "NUM_INST_INSERTED_PER_CYCLE") == 0) {
        NUM_INST_INSERTED_PER_CYCLE = atoi_general(value);
    }
    else if (strcmp(name, "IDQUED_WIDTH") == 0) {
        IDQUED_WIDTH = atoi_general(value);
    }
    else if (strcmp(name, "IDQCTLM_WIDTH") == 0) {
        IDQCTLM_WIDTH = atoi_general(value);
    }
    else if (strcmp(name, "IDQIMMD_WIDTH") == 0) {
        IDQIMMD_WIDTH = atoi_general(value);
    }
    else if (strcmp(name, "IDQ_WRITE_PTS") == 0) {
        IDQ_WRITE_PTS = atoi_general(value);
    }
    else if (strcmp(name, "ID_2_ID_BW") == 0) {
        ID_2_ID_BW = atoi_general(value);
    }
    else if (strcmp(name, "SERIAL_ON_UID") == 0) {
        SERIAL_ON_UID = atoi_general(value);
    }
    else if (strcmp(name, "SERIALIZE") == 0) {
        SERIALIZE = atoi_general(value);
    }
    else if (strcmp(name, "AUOP_BIT_WIDTH") == 0) {
        AUOP_BIT_WIDTH = atoi_general(value);
    }
    else if (strcmp(name, "MICRO_SEQUENCER_BW") == 0) {
        MICRO_SEQUENCER_BW = atoi_general(value);
    }
    else if (strcmp(name, "MICRO_SEQUENCER_ENTRIES") == 0) {
        MICRO_SEQUENCER_ENTRIES = atoi_general(value);
    }
    else if (strcmp(name, "FUSE_ESP_FOLD") == 0) {
        FUSE_ESP_FOLD = atoi_general(value);
    }
    else if (strcmp(name, "FUSE_TJS") == 0) {
        FUSE_TJS = atoi_general(value);
    }
    else if (strcmp(name, "FUSE_P6") == 0) {
        FUSE_P6 = atoi_general(value);
    }
    else if (strcmp(name, "FUSE_MRM_LD_OP") == 0) {
        FUSE_MRM_LD_OP = atoi_general(value);
    }
    else if (strcmp(name, "FUSE_MRM_LD_JMP") == 0) {
        FUSE_MRM_LD_JMP = atoi_general(value);
    }
    else if (strcmp(name, "FUSE_MRM_IMM_DISP") == 0) {
        FUSE_MRM_IMM_DISP = atoi_general(value);
    }
    else if (strcmp(name, "FUSE_NHM_MS_STORE") == 0) {
        FUSE_NHM_MS_STORE = atoi_general(value);
    }
    else if (strcmp(name, "FUSE_NHM") == 0) {
        FUSE_NHM = atoi_general(value);
    }
    else if (strcmp(name, "FUSE_MM_IMM8") == 0) {
        FUSE_MM_IMM8 = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_PERFECT_ITLB") == 0) {
        ENABLE_PERFECT_ITLB = atoi_general(value);
    }
    else if (strcmp(name, "ITLB_SMALL_PER_THREAD") == 0) {
        ITLB_SMALL_PER_THREAD = atoi_general(value);
    }
    else if (strcmp(name, "ITLB_SMALL_WARM_PERCENT") == 0) {
        ITLB_SMALL_WARM_PERCENT = atoi_general(value);
    }
    else if (strcmp(name, "ITLB_LARGE_PER_THREAD") == 0) {
        ITLB_LARGE_PER_THREAD = atoi_general(value);
    }
    else if (strcmp(name, "ITLB_LARGE_WARM_PERCENT") == 0) {
        ITLB_LARGE_WARM_PERCENT = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_OPP_FETCHES") == 0) {
        ENABLE_OPP_FETCHES = atoi_general(value);
    }
    else if (strcmp(name, "BIT_ARRAY_WRITE_LATENCY") == 0) {
        BIT_ARRAY_WRITE_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "BIT_ARRAY_READ_LATENCY") == 0) {
        BIT_ARRAY_READ_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "UPDATE_STEW_ON_RETURNS") == 0) {
        UPDATE_STEW_ON_RETURNS = atoi_general(value);
    }
    else if (strcmp(name, "UPDATE_STEW_ON_UNCONDITIONALS") == 0) {
        UPDATE_STEW_ON_UNCONDITIONALS = atoi_general(value);
    }
    else if (strcmp(name, "CLEAR_IBTB_ON_BOGUS") == 0) {
        CLEAR_IBTB_ON_BOGUS = atoi_general(value);
    }
    else if (strcmp(name, "BAC_RSB_PER_THREAD") == 0) {
        BAC_RSB_PER_THREAD = atoi_general(value);
    }
    else if (strcmp(name, "STATIC_PREDICTION_DISPLACEMENT") == 0) {
        STATIC_PREDICTION_DISPLACEMENT = atoi_general(value);
    }
    else if (strcmp(name, "ROB_SIZE") == 0) {
        ROB_SIZE = atoi_general(value);
    }
    else if (strcmp(name, "RS_SIZE") == 0) {
        RS_SIZE = atoi_general(value);
    }
    else if (strcmp(name, "NUM_BCAST") == 0) {
        NUM_BCAST = atoi_general(value);
    }
    else if (strcmp(name, "BLOCK_ROB") == 0) {
        BLOCK_ROB = atoi_general(value);
    }
    else if (strcmp(name, "FUSED_ROB") == 0) {
        FUSED_ROB = atoi_general(value);
    }
    else if (strcmp(name, "BLOCK_RS") == 0) {
        BLOCK_RS = atoi_general(value);
    }
    else if (strcmp(name, "FUSED_RS") == 0) {
        FUSED_RS = atoi_general(value);
    }
    else if (strcmp(name, "MOVE_ELIM") == 0) {
        MOVE_ELIM = atoi_general(value);
    }
    else if (strcmp(name, "ZERO_ELIM") == 0) {
        ZERO_ELIM = atoi_general(value);
    }
    else if (strcmp(name, "RS_PARTITION_STEERING") == 0) {
        RS_PARTITION_STEERING = value;
    }
    else if (strcmp(name, "RS_PARTITION_SIZES") == 0) {
        RS_PARTITION_SIZES = value;
    }
    else if (strcmp(name, "RS_ELIGIBLE") == 0) {
        RS_ELIGIBLE = atoi_general(value);
    }
    else if (strcmp(name, "RS_LOAD_DEP_TIMER") == 0) {
        RS_LOAD_DEP_TIMER = atoi_general(value);
    }
    else if (strcmp(name, "NO_SPEC_MEM_ISSUE") == 0) {
        NO_SPEC_MEM_ISSUE = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_VAR_EXEC_FIXED") == 0) {
        ENABLE_VAR_EXEC_FIXED = atoi_general(value);
    }
    else if (strcmp(name, "ADVANCED_RS") == 0) {
        ADVANCED_RS = atoi_general(value);
    }
    else if (strcmp(name, "ENFORCE_PARTIAL_FLAG_STALLS") == 0) {
        ENFORCE_PARTIAL_FLAG_STALLS = atoi_general(value);
    }
    else if (strcmp(name, "ENFORCE_PARTIAL_READ_STALLS") == 0) {
        ENFORCE_PARTIAL_READ_STALLS = atoi_general(value);
    }
    else if (strcmp(name, "ALLOC_MAX_BRANCHES") == 0) {
        ALLOC_MAX_BRANCHES = atoi_general(value);
    }
    else if (strcmp(name, "ALLOC_FXCHG_CUT") == 0) {
        ALLOC_FXCHG_CUT = atoi_general(value);
    }
    else if (strcmp(name, "RS_2_EXEC_LATENCY") == 0) {
        RS_2_EXEC_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "RS_2_ROB_LATENCY") == 0) {
        RS_2_ROB_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "EXEC_2_DIVIDER_LATENCY") == 0) {
        EXEC_2_DIVIDER_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "MS_2_BE_DISPATCHED_LD_LATENCY") == 0) {
        MS_2_BE_DISPATCHED_LD_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "MS_2_BE_INVALID_LD_LATENCY") == 0) {
        MS_2_BE_INVALID_LD_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "FE_2_BE_FETCHED_INST_LATENCY") == 0) {
        FE_2_BE_FETCHED_INST_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "EXEC_2_ROB_LATENCY") == 0) {
        EXEC_2_ROB_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "ROB_2_ALLOC_RAT1_COMMITTED_INST_LATENCY") == 0) {
        ROB_2_ALLOC_RAT1_COMMITTED_INST_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "ROB_2_ALLOC_RAT1_DISPATCHED_INST_LATENCY") == 0) {
        ROB_2_ALLOC_RAT1_DISPATCHED_INST_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "ROB_2_ALLOC_RAT1_WB_INST_LATENCY") == 0) {
        ROB_2_ALLOC_RAT1_WB_INST_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "EH_2_BE_SIGNALED_EXCEPTIONS_LATENCY") == 0) {
        EH_2_BE_SIGNALED_EXCEPTIONS_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "RS_2_ALLOC_STATE_LATENCY") == 0) {
        RS_2_ALLOC_STATE_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "ROB_2_ALLOC_STATE_LATENCY") == 0) {
        ROB_2_ALLOC_STATE_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "STB_2_ALLOC_STATE_LATENCY") == 0) {
        STB_2_ALLOC_STATE_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "LDB_2_ALLOC_STATE_LATENCY") == 0) {
        LDB_2_ALLOC_STATE_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "BE_2_EH_EXCEPTION_LATENCY") == 0) {
        BE_2_EH_EXCEPTION_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "EXCEPTION_2_ALLOC_RAT1_LATENCY") == 0) {
        EXCEPTION_2_ALLOC_RAT1_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "CHECK_PORTBINDING_VS_FEEDER") == 0) {
        CHECK_PORTBINDING_VS_FEEDER = atoi_general(value);
    }
    else if (strcmp(name, "ESPR_BIT_WIDTH") == 0) {
        ESPR_BIT_WIDTH = atoi_general(value);
    }
    else if (strcmp(name, "ESPR_READ_PORTS") == 0) {
        ESPR_READ_PORTS = atoi_general(value);
    }
    else if (strcmp(name, "ESPR_JECLEAR_LATENCY") == 0) {
        ESPR_JECLEAR_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "ESPR_RONUKE_LATENCY") == 0) {
        ESPR_RONUKE_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "ESPR_ALLOC_LATENCY") == 0) {
        ESPR_ALLOC_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "ALLOC_RAT1_2_RAT2_LATENCY") == 0) {
        ALLOC_RAT1_2_RAT2_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "RAT2_2_RS_LATENCY") == 0) {
        RAT2_2_RS_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "EH_2_RS_LATENCY") == 0) {
        EH_2_RS_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "RAT2_2_ROB_LATENCY") == 0) {
        RAT2_2_ROB_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "EH_2_ROB_LATENCY") == 0) {
        EH_2_ROB_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "MS_2_BE_COMPLETED_MEM_OP_LATENCY") == 0) {
        MS_2_BE_COMPLETED_MEM_OP_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_ANALYSIS_WINDOW") == 0) {
        ENABLE_ANALYSIS_WINDOW = atoi_general(value);
    }
    else if (strcmp(name, "CAPS_STALL_THRESHOLD") == 0) {
        CAPS_STALL_THRESHOLD = atoi_general(value);
    }
    else if (strcmp(name, "CAPS_SECONDARY_STALL_THRESHOLD") == 0) {
        CAPS_SECONDARY_STALL_THRESHOLD = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_CAPS_FLUSH") == 0) {
        ENABLE_CAPS_FLUSH = atoi_general(value);
    }
    else if (strcmp(name, "RONUKE_STYLE_CAPS_FLUSH") == 0) {
        RONUKE_STYLE_CAPS_FLUSH = atoi_general(value);
    }
    else if (strcmp(name, "COMMIT_TIME_CAPS") == 0) {
        COMMIT_TIME_CAPS = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_PRIMARY") == 0) {
        ENABLE_PRIMARY = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_LOGICAL") == 0) {
        ENABLE_LOGICAL = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_COLLAPSING") == 0) {
        ENABLE_COLLAPSING = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_BACK_MASKING") == 0) {
        ENABLE_BACK_MASKING = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_STORE_ANALYSIS_1") == 0) {
        ENABLE_STORE_ANALYSIS_1 = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_STORE_ANALYSIS_2") == 0) {
        ENABLE_STORE_ANALYSIS_2 = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_INST_PORT") == 0) {
        ENABLE_INST_PORT = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_PENRYN_INTEGER_DIVIDER") == 0) {
        ENABLE_PENRYN_INTEGER_DIVIDER = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_MEROM_INTEGER_DIVIDER") == 0) {
        ENABLE_MEROM_INTEGER_DIVIDER = atoi_general(value);
    }
    else if (strcmp(name, "IDIV_QUOTIENT_BITS_PER_CYCLE") == 0) {
        IDIV_QUOTIENT_BITS_PER_CYCLE = atoi_general(value);
    }
    else if (strcmp(name, "IDIV_MIN_LATENCY") == 0) {
        IDIV_MIN_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "IDIV_POSTPROCESSING_LATENCY") == 0) {
        IDIV_POSTPROCESSING_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "IDIV_PREPROCESSING_LATENCY") == 0) {
        IDIV_PREPROCESSING_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_EARLY_OUT_FDIV0") == 0) {
        ENABLE_EARLY_OUT_FDIV0 = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_EARLY_OUT_FDIV_BY_POW2") == 0) {
        ENABLE_EARLY_OUT_FDIV_BY_POW2 = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_EARLY_OUT_FSQRT0") == 0) {
        ENABLE_EARLY_OUT_FSQRT0 = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_EARLY_OUT_FSQRT_BY_POW4") == 0) {
        ENABLE_EARLY_OUT_FSQRT_BY_POW4 = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_EARLY_OUT_SSE0") == 0) {
        ENABLE_EARLY_OUT_SSE0 = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_EARLY_OUT_SSE_DIV_BY_POW2") == 0) {
        ENABLE_EARLY_OUT_SSE_DIV_BY_POW2 = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_EARLY_OUT_SSE_SQRT_BY_POW4") == 0) {
        ENABLE_EARLY_OUT_SSE_SQRT_BY_POW4 = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_FORCE_IDIV") == 0) {
        ENABLE_FORCE_IDIV = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_FORCE_FDIV") == 0) {
        ENABLE_FORCE_FDIV = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_FORCE_SDIV") == 0) {
        ENABLE_FORCE_SDIV = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_FORCE_FSQRT") == 0) {
        ENABLE_FORCE_FSQRT = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_FORCE_SSQRT") == 0) {
        ENABLE_FORCE_SSQRT = atoi_general(value);
    }
    else if (strcmp(name, "ENABLE_FORCE_FREM") == 0) {
        ENABLE_FORCE_FREM = atoi_general(value);
    }
    else if (strcmp(name, "FORCE_IDIV_LATENCY") == 0) {
        FORCE_IDIV_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "FORCE_FDIV_LATENCY") == 0) {
        FORCE_FDIV_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "FORCE_SDIV_LATENCY") == 0) {
        FORCE_SDIV_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "FORCE_FSQRT_LATENCY") == 0) {
        FORCE_FSQRT_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "FORCE_SSQRT_LATENCY") == 0) {
        FORCE_SSQRT_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "FORCE_FREM_LATENCY") == 0) {
        FORCE_FREM_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "EXEC_2_RE_PREDICTOR_LATENCY") == 0) {
        EXEC_2_RE_PREDICTOR_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "MS_2_RE_PREDICTOR_LATENCY") == 0) {
        MS_2_RE_PREDICTOR_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "ROB_2_EH_LATENCY") == 0) {
        ROB_2_EH_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "EXEC_2_EH_LATENCY") == 0) {
        EXEC_2_EH_LATENCY = atoi_general(value);
    }
    else if (strcmp(name, "RAT_AVF_SIMULATE_COUNT") == 0) {
        RAT_AVF_SIMULATE_COUNT = atoi_general(value);
    }
    else if (strcmp(name, "RAT_AVF_COOLDOWN_COUNT") == 0) {
        RAT_AVF_COOLDOWN_COUNT = atoi_general(value);
    }
    else if (strcmp(name, "RAT_AVF_WARMUP_COUNT") == 0) {
        RAT_AVF_WARMUP_COUNT = atoi_general(value);
    }
    else if (strcmp(name, "RAT_AVF_GC_INTERVAL") == 0) {
        RAT_AVF_GC_INTERVAL = atoi_general(value);
    }
    else if (strcmp(name, "RAT_REC_AVF_SIMULATE_COUNT") == 0) {
        RAT_REC_AVF_SIMULATE_COUNT = atoi_general(value);
    }
    else if (strcmp(name, "RAT_REC_AVF_COOLDOWN_COUNT") == 0) {
        RAT_REC_AVF_COOLDOWN_COUNT = atoi_general(value);
    }
    else if (strcmp(name, "RAT_REC_AVF_WARMUP_COUNT") == 0) {
        RAT_REC_AVF_WARMUP_COUNT = atoi_general(value);
    }
    else if (strcmp(name, "RAT_REC_AVF_GC_INTERVAL") == 0) {
        RAT_REC_AVF_GC_INTERVAL = atoi_general(value);
    }
    else if (strcmp(name, "RRF_AVF_SIMULATE_COUNT") == 0) {
        RRF_AVF_SIMULATE_COUNT = atoi_general(value);
    }
    else if (strcmp(name, "RRF_AVF_WARMUP_COUNT") == 0) {
        RRF_AVF_WARMUP_COUNT = atoi_general(value);
    }
    else if (strcmp(name, "RRF_AVF_COOLDOWN_COUNT") == 0) {
        RRF_AVF_COOLDOWN_COUNT = atoi_general(value);
    }
    else if (1) {
        found = false;
    }
    return found;
}


void ListParams (void)
{
    cout << "The following dynamic parameters are registered:" << endl;
    cout << "    STOP_THREAD" << " = " << "0" << endl;
    cout << "    LIMIT_PTHREADS" << " = " << "0" << endl;
    cout << "    SIMULATED_REGION_WEIGHT" << " = " << "10000" << endl;
    cout << "    SIMULATED_REGION_INSTRS_REPRESENTED" << " = " << "0" << endl;
    cout << "    ADF_DEFAULT" << " = " << "\"$built-in$\"" << endl;
    cout << "    ENABLE_WARMUP" << " = " << "1" << endl;
    cout << "    CHECK_RUNNABLE" << " = " << "10" << endl;
    cout << "    CHECK_NEW_STREAM" << " = " << "5" << endl;
    cout << "    CHECK_EXIT" << " = " << "100" << endl;
    cout << "    INTERVAL_TIMER" << " = " << "0" << endl;
    cout << "    CYCLES_PER_PICOSECOND" << " = " << "2200" << endl;
    cout << "    ENABLE_INST_PROFILE" << " = " << "0" << endl;
    cout << "    MAX_NUM_INSTS" << " = " << "100000" << endl;
    cout << "    APE_ERROR_CHECKING" << " = " << "0" << endl;
    cout << "    USE_TRANSLATION_CACHE" << " = " << "1" << endl;
    cout << "    USE_UNALIAS_CACHE" << " = " << "0" << endl;
    cout << "    USE_UOP_DIS_CACHE" << " = " << "1" << endl;
    cout << "    pt" << " = " << "\"\"" << endl;
    cout << "    pt_dump" << " = " << "\"\"" << endl;
    cout << "    pipetrace_dump_max" << " = " << "20000" << endl;
    cout << "    stats" << " = " << "\"\"" << endl;
    cout << "    pt_ascii" << " = " << "0" << endl;
    cout << "    nthread" << " = " << "4" << endl;
    cout << "    ncore" << " = " << "16" << endl;
    cout << "    nproc" << " = " << "1" << endl;
    cout << "    perfchecker" << " = " << "0" << endl;
    cout << "    WILLY_STYLE_STATS" << " = " << "0" << endl;
    cout << "    instrs_retired" << " = " << "0" << endl;
    cout << "    ENABLE_CACHE_LOCK" << " = " << "0" << endl;
    cout << "    ASSERT_NON_READY_REG" << " = " << "0" << endl;
    cout << "    LEGACY_FCW" << " = " << "0" << endl;
    cout << "    CHANGE_XAPIC_ID_TO_STREAM_ID" << " = " << "0" << endl;
    cout << "    SHARE_IADDR_SPACE" << " = " << "0" << endl;
    cout << "    SHARE_DADDR_SPACE" << " = " << "0" << endl;
    cout << "    FEEDER_REPLICATE" << " = " << "0" << endl;
    cout << "    FEEDER_SKIP" << " = " << "0" << endl;
    cout << "    FEEDER_NTHREADS" << " = " << "1" << endl;
    cout << "    SKIP_WITH_WARMUP" << " = " << "0" << endl;
    cout << "    CHECKER_ON" << " = " << "1" << endl;
    cout << "    ASSERT_ON_CHECKER_ERROR" << " = " << "1" << endl;
    cout << "    CHECKER_MASK" << " = " << "63" << endl;
    cout << "    USE_SEPARATE_IMAGES" << " = " << "1" << endl;
    cout << "    DO_NOT_MANGLE_LOWER_BITS" << " = " << "0" << endl;
    cout << "    MAX_SKIP_UOP_PER_MOP_THRESHOLD" << " = " << "10000000" << endl;
    cout << "    REC_INST_STAT" << " = " << "1" << endl;
    cout << "    STRICT_ORDERING" << " = " << "1" << endl;
    cout << "    PROCESS_HISTORY_BUCKETS" << " = " << "5000" << endl;
    cout << "    LIVENESS_TIMEOUT" << " = " << "10000" << endl;
    cout << "    PGSDEBUG" << " = " << "0" << endl;
    cout << "    HACK_DOWRITE_AT_COMMIT" << " = " << "0" << endl;
    cout << "    WRONG_PATH" << " = " << "1" << endl;
    cout << "    INST_BW" << " = " << "4" << endl;
    cout << "    COMMIT_WIDTH" << " = " << "4" << endl;
    cout << "    RMT_ENABLE_NHM_SYNC" << " = " << "1" << endl;
    cout << "    SLACK_VALUE" << " = " << "0" << endl;
    cout << "    ALLOW_LOAD_STORE_PORT_SHARING" << " = " << "1" << endl;
    cout << "    INCREASE_LOAD_BW" << " = " << "0" << endl;
    cout << "    INCREASE_STORE_BW" << " = " << "0" << endl;
    cout << "    LDB_SIZE" << " = " << "48" << endl;
    cout << "    STB_SIZE" << " = " << "32" << endl;
    cout << "    ENABLE_MLC_AVF" << " = " << "0" << endl;
    cout << "    ENABLE_MLC_SCRUB_AVF" << " = " << "0" << endl;
    cout << "    ENABLE_RAT_AVF" << " = " << "0" << endl;
    cout << "    ENABLE_RAT_REC_AVF" << " = " << "0" << endl;
    cout << "    ENABLE_DCU_AVF" << " = " << "0" << endl;
    cout << "    ENABLE_DCU_SCRUB_AVF" << " = " << "0" << endl;
    cout << "    ENABLE_DCUBUFF_AVF" << " = " << "0" << endl;
    cout << "    ENABLE_LDB_AVF" << " = " << "0" << endl;
    cout << "    ENABLE_STB_AVF" << " = " << "0" << endl;
    cout << "    ENABLE_IDQ_AVF" << " = " << "0" << endl;
    cout << "    ENABLE_IQ_AVF" << " = " << "0" << endl;
    cout << "    ENABLE_BAC_AVF" << " = " << "0" << endl;
    cout << "    ENABLE_ID_AVF" << " = " << "0" << endl;
    cout << "    ENABLE_RRF_AVF" << " = " << "0" << endl;
    cout << "    ENABLE_RS_AVF" << " = " << "0" << endl;
    cout << "    ENABLE_EXEC_AVF" << " = " << "0" << endl;
    cout << "    ENABLE_ROB_AVF" << " = " << "0" << endl;
    cout << "    ENABLE_IC_AVF" << " = " << "0" << endl;
    cout << "    ENABLE_ISB_AVF" << " = " << "0" << endl;
    cout << "    ENABLE_IVC_AVF" << " = " << "0" << endl;
    cout << "    ENABLE_AVF" << " = " << "0" << endl;
    cout << "    ENABLE_AVF_TRACE" << " = " << "0" << endl;
    cout << "    AVF_RS_TRACE" << " = " << "0" << endl;
    cout << "    IN_ORDER_MEM_DISPATCH" << " = " << "0" << endl;
    cout << "    SKIP_RRSB_RETIRE_CHECKS" << " = " << "0" << endl;
    cout << "    USE_PARTIAL_AWIN" << " = " << "1" << endl;
    cout << "    CONTROL_REGISTER_WRITE_LATENCY" << " = " << "3" << endl;
    cout << "    ENABLE_LOAD_TO_STD_OPT" << " = " << "0" << endl;
    cout << "    CORRECT_PB_DIV_REPRATE" << " = " << "1" << endl;
    cout << "    TRACE_TINY_INST_REFCOUNT" << " = " << "0" << endl;
    cout << "    ENABLE_MEMORY_DISAMBIGUATOR" << " = " << "0" << endl;
    cout << "    ENHANCED_LOOSENET_CHECKING" << " = " << "1" << endl;
    cout << "    ENABLE_PERFECT_STLB" << " = " << "0" << endl;
    cout << "    STLB_PER_THREAD" << " = " << "0" << endl;
    cout << "    STLB_LOG2_PAGE_SIZE" << " = " << "12" << endl;
    cout << "    STLB_WARM_PERCENT" << " = " << "0" << endl;
    cout << "    STLB_ONLY_FILL_4K_PAGES" << " = " << "1" << endl;
    cout << "    ENABLE_PERFECT_PDE" << " = " << "0" << endl;
    cout << "    PDE_PER_THREAD" << " = " << "0" << endl;
    cout << "    PDE_LOG2_PAGE_SIZE" << " = " << "12" << endl;
    cout << "    PDE_WARM_PERCENT" << " = " << "0" << endl;
    cout << "    ENABLE_PERFECT_PDP" << " = " << "0" << endl;
    cout << "    PDP_PER_THREAD" << " = " << "0" << endl;
    cout << "    PDP_LOG2_PAGE_SIZE" << " = " << "12" << endl;
    cout << "    PDP_WARM_PERCENT" << " = " << "0" << endl;
    cout << "    PRED_MEM_LATENCY" << " = " << "9" << endl;
    cout << "    ENABLE_STREAMER_PREFETCH" << " = " << "1" << endl;
    cout << "    ENABLE_STRIDE_PREFETCH" << " = " << "1" << endl;
    cout << "    MD_COUNTER_SATURATION" << " = " << "15" << endl;
    cout << "    MD_NUM_ENTRIES" << " = " << "256" << endl;
    cout << "    MD_IDX_MASK" << " = " << "255" << endl;
    cout << "    MD_IDX_HWC_POS" << " = " << "7" << endl;
    cout << "    MD_IDX_HWC_MASK" << " = " << "1" << endl;
    cout << "    ALLOW_STORE_FORWARDING_WITH_VIRTUAL_ADDRESS" << " = " << "1" << endl;
    cout << "    ENABLE_PERFECT_DTLB" << " = " << "0" << endl;
    cout << "    FORCE_XAPIC_UC" << " = " << "1" << endl;
    cout << "    DTLB_SMALL_PER_THREAD" << " = " << "0" << endl;
    cout << "    DTLB_SMALL_WARM_PERCENT" << " = " << "0" << endl;
    cout << "    DTLB_SMALL_TAG_BITS" << " = " << "32" << endl;
    cout << "    DTLB_SMALL_EXTRA_BITS_PER_WAY" << " = " << "3" << endl;
    cout << "    DTLB_SMALL_EXTRA_BITS" << " = " << "3" << endl;
    cout << "    DTLB_SMALL_READ_PORTS" << " = " << "2" << endl;
    cout << "    DTLB_SMALL_WRITE_PORTS" << " = " << "1" << endl;
    cout << "    DTLB_LARGE_PER_THREAD" << " = " << "0" << endl;
    cout << "    DTLB_LARGE_WARM_PERCENT" << " = " << "0" << endl;
    cout << "    DTLB_LARGE_TAG_BITS" << " = " << "32" << endl;
    cout << "    DTLB_LARGE_EXTRA_BITS_PER_WAY" << " = " << "3" << endl;
    cout << "    DTLB_LARGE_EXTRA_BITS" << " = " << "3" << endl;
    cout << "    DTLB_LARGE_READ_PORTS" << " = " << "2" << endl;
    cout << "    DTLB_LARGE_WRITE_PORTS" << " = " << "1" << endl;
    cout << "    MLC_SUPER_QUEUE_SIZE" << " = " << "16" << endl;
    cout << "    MLC_SNOOP_QUEUE_SIZE" << " = " << "16" << endl;
    cout << "    SNOOPQ_CREDITS" << " = " << "8" << endl;
    cout << "    IMQ_CREDITS" << " = " << "8" << endl;
    cout << "    SQ_ENTRIES" << " = " << "16" << endl;
    cout << "    WOB_ENTRIES" << " = " << "16" << endl;
    cout << "    ENABLE_MLC_PFC_STREAMER" << " = " << "0" << endl;
    cout << "    ENABLE_MLC_PFC_SPATIAL" << " = " << "0" << endl;
    cout << "    MLC_PFC_STREAMER_FWD_THRESH" << " = " << "1" << endl;
    cout << "    MLC_PFC_STREAMER_BWD_THRESH" << " = " << "2" << endl;
    cout << "    MLC_PFC_STREAMER_FAR_DIST" << " = " << "32" << endl;
    cout << "    MLC_PFC_STREAMER_GLOBAL_TRAIN_BITS" << " = " << "6" << endl;
    cout << "    MLC_PFC_STREAMER_PFDIST_MAX" << " = " << "12" << endl;
    cout << "    MLC_PFC_STREAMER_PFDIST_MIN" << " = " << "2" << endl;
    cout << "    MLC_PFC_STREAMER_DETECTOR_TRAIN_MAX" << " = " << "3" << endl;
    cout << "    MLC_PFC_STREAMER_DETECTOR_CONFIDENCE_INC" << " = " << "4" << endl;
    cout << "    MLC_PFC_STREAMER_DETECTOR_CONFIDENCE_DEC" << " = " << "1" << endl;
    cout << "    SQ_WOB_ENTRY_LIVENESS_TIMEOUT" << " = " << "200000" << endl;
    cout << "    SQ_MLC_MAX_REJECTS" << " = " << "10000" << endl;
    cout << "    IDI_LLC_UC_LD_LATENCY" << " = " << "228" << endl;
    cout << "    IDI_LLC_UC_ST_LATENCY" << " = " << "155" << endl;
    cout << "    IDI_LLC_HIT_LATENCY" << " = " << "29" << endl;
    cout << "    IDI_LLC_GO_AHEAD_OF_DATA" << " = " << "3" << endl;
    cout << "    IDI_LLC_ALWAYS_EXLCUSIVE" << " = " << "1" << endl;
    cout << "    USE_BP" << " = " << "1" << endl;
    cout << "    USE_uBP" << " = " << "1" << endl;
    cout << "    IQ_ARRAY_BW" << " = " << "136" << endl;
    cout << "    NUM_INST_INSERTED_PER_CYCLE" << " = " << "6" << endl;
    cout << "    IDQUED_WIDTH" << " = " << "135" << endl;
    cout << "    IDQCTLM_WIDTH" << " = " << "9" << endl;
    cout << "    IDQIMMD_WIDTH" << " = " << "92" << endl;
    cout << "    IDQ_WRITE_PTS" << " = " << "7" << endl;
    cout << "    ID_2_ID_BW" << " = " << "4" << endl;
    cout << "    SERIAL_ON_UID" << " = " << "0" << endl;
    cout << "    SERIALIZE" << " = " << "0" << endl;
    cout << "    AUOP_BIT_WIDTH" << " = " << "137" << endl;
    cout << "    MICRO_SEQUENCER_BW" << " = " << "308" << endl;
    cout << "    MICRO_SEQUENCER_ENTRIES" << " = " << "6144" << endl;
    cout << "    FUSE_ESP_FOLD" << " = " << "1" << endl;
    cout << "    FUSE_TJS" << " = " << "0" << endl;
    cout << "    FUSE_P6" << " = " << "1" << endl;
    cout << "    FUSE_MRM_LD_OP" << " = " << "1" << endl;
    cout << "    FUSE_MRM_LD_JMP" << " = " << "1" << endl;
    cout << "    FUSE_MRM_IMM_DISP" << " = " << "1" << endl;
    cout << "    FUSE_NHM_MS_STORE" << " = " << "1" << endl;
    cout << "    FUSE_NHM" << " = " << "1" << endl;
    cout << "    FUSE_MM_IMM8" << " = " << "0" << endl;
    cout << "    ENABLE_PERFECT_ITLB" << " = " << "0" << endl;
    cout << "    ITLB_SMALL_PER_THREAD" << " = " << "0" << endl;
    cout << "    ITLB_SMALL_WARM_PERCENT" << " = " << "0" << endl;
    cout << "    ITLB_LARGE_PER_THREAD" << " = " << "0" << endl;
    cout << "    ITLB_LARGE_WARM_PERCENT" << " = " << "0" << endl;
    cout << "    ENABLE_OPP_FETCHES" << " = " << "0" << endl;
    cout << "    BIT_ARRAY_WRITE_LATENCY" << " = " << "2" << endl;
    cout << "    BIT_ARRAY_READ_LATENCY" << " = " << "1" << endl;
    cout << "    UPDATE_STEW_ON_RETURNS" << " = " << "1" << endl;
    cout << "    UPDATE_STEW_ON_UNCONDITIONALS" << " = " << "1" << endl;
    cout << "    CLEAR_IBTB_ON_BOGUS" << " = " << "0" << endl;
    cout << "    BAC_RSB_PER_THREAD" << " = " << "1" << endl;
    cout << "    STATIC_PREDICTION_DISPLACEMENT" << " = " << "256" << endl;
    cout << "    ROB_SIZE" << " = " << "128" << endl;
    cout << "    RS_SIZE" << " = " << "36" << endl;
    cout << "    NUM_BCAST" << " = " << "0" << endl;
    cout << "    BLOCK_ROB" << " = " << "0" << endl;
    cout << "    FUSED_ROB" << " = " << "0" << endl;
    cout << "    BLOCK_RS" << " = " << "0" << endl;
    cout << "    FUSED_RS" << " = " << "0" << endl;
    cout << "    MOVE_ELIM" << " = " << "0" << endl;
    cout << "    ZERO_ELIM" << " = " << "0" << endl;
    cout << "    RS_PARTITION_STEERING" << " = " << "\"0\"" << endl;
    cout << "    RS_PARTITION_SIZES" << " = " << "\"0\"" << endl;
    cout << "    RS_ELIGIBLE" << " = " << "0" << endl;
    cout << "    RS_LOAD_DEP_TIMER" << " = " << "0" << endl;
    cout << "    NO_SPEC_MEM_ISSUE" << " = " << "1" << endl;
    cout << "    ENABLE_VAR_EXEC_FIXED" << " = " << "0" << endl;
    cout << "    ADVANCED_RS" << " = " << "0" << endl;
    cout << "    ENFORCE_PARTIAL_FLAG_STALLS" << " = " << "1" << endl;
    cout << "    ENFORCE_PARTIAL_READ_STALLS" << " = " << "0" << endl;
    cout << "    ALLOC_MAX_BRANCHES" << " = " << "0" << endl;
    cout << "    ALLOC_FXCHG_CUT" << " = " << "1" << endl;
    cout << "    RS_2_EXEC_LATENCY" << " = " << "1" << endl;
    cout << "    RS_2_ROB_LATENCY" << " = " << "1" << endl;
    cout << "    EXEC_2_DIVIDER_LATENCY" << " = " << "1" << endl;
    cout << "    MS_2_BE_DISPATCHED_LD_LATENCY" << " = " << "3" << endl;
    cout << "    MS_2_BE_INVALID_LD_LATENCY" << " = " << "0" << endl;
    cout << "    FE_2_BE_FETCHED_INST_LATENCY" << " = " << "1" << endl;
    cout << "    EXEC_2_ROB_LATENCY" << " = " << "3" << endl;
    cout << "    ROB_2_ALLOC_RAT1_COMMITTED_INST_LATENCY" << " = " << "1" << endl;
    cout << "    ROB_2_ALLOC_RAT1_DISPATCHED_INST_LATENCY" << " = " << "1" << endl;
    cout << "    ROB_2_ALLOC_RAT1_WB_INST_LATENCY" << " = " << "1" << endl;
    cout << "    EH_2_BE_SIGNALED_EXCEPTIONS_LATENCY" << " = " << "1" << endl;
    cout << "    RS_2_ALLOC_STATE_LATENCY" << " = " << "1" << endl;
    cout << "    ROB_2_ALLOC_STATE_LATENCY" << " = " << "1" << endl;
    cout << "    STB_2_ALLOC_STATE_LATENCY" << " = " << "1" << endl;
    cout << "    LDB_2_ALLOC_STATE_LATENCY" << " = " << "1" << endl;
    cout << "    BE_2_EH_EXCEPTION_LATENCY" << " = " << "1" << endl;
    cout << "    EXCEPTION_2_ALLOC_RAT1_LATENCY" << " = " << "1" << endl;
    cout << "    CHECK_PORTBINDING_VS_FEEDER" << " = " << "0" << endl;
    cout << "    ESPR_BIT_WIDTH" << " = " << "11" << endl;
    cout << "    ESPR_READ_PORTS" << " = " << "1" << endl;
    cout << "    ESPR_JECLEAR_LATENCY" << " = " << "4" << endl;
    cout << "    ESPR_RONUKE_LATENCY" << " = " << "3" << endl;
    cout << "    ESPR_ALLOC_LATENCY" << " = " << "1" << endl;
    cout << "    ALLOC_RAT1_2_RAT2_LATENCY" << " = " << "1" << endl;
    cout << "    RAT2_2_RS_LATENCY" << " = " << "1" << endl;
    cout << "    EH_2_RS_LATENCY" << " = " << "1" << endl;
    cout << "    RAT2_2_ROB_LATENCY" << " = " << "1" << endl;
    cout << "    EH_2_ROB_LATENCY" << " = " << "1" << endl;
    cout << "    MS_2_BE_COMPLETED_MEM_OP_LATENCY" << " = " << "1" << endl;
    cout << "    ENABLE_ANALYSIS_WINDOW" << " = " << "0" << endl;
    cout << "    CAPS_STALL_THRESHOLD" << " = " << "100" << endl;
    cout << "    CAPS_SECONDARY_STALL_THRESHOLD" << " = " << "0" << endl;
    cout << "    ENABLE_CAPS_FLUSH" << " = " << "0" << endl;
    cout << "    RONUKE_STYLE_CAPS_FLUSH" << " = " << "0" << endl;
    cout << "    COMMIT_TIME_CAPS" << " = " << "0" << endl;
    cout << "    ENABLE_PRIMARY" << " = " << "1" << endl;
    cout << "    ENABLE_LOGICAL" << " = " << "0" << endl;
    cout << "    ENABLE_COLLAPSING" << " = " << "1" << endl;
    cout << "    ENABLE_BACK_MASKING" << " = " << "0" << endl;
    cout << "    ENABLE_STORE_ANALYSIS_1" << " = " << "0" << endl;
    cout << "    ENABLE_STORE_ANALYSIS_2" << " = " << "0" << endl;
    cout << "    ENABLE_INST_PORT" << " = " << "0" << endl;
    cout << "    ENABLE_PENRYN_INTEGER_DIVIDER" << " = " << "1" << endl;
    cout << "    ENABLE_MEROM_INTEGER_DIVIDER" << " = " << "0" << endl;
    cout << "    IDIV_QUOTIENT_BITS_PER_CYCLE" << " = " << "4" << endl;
    cout << "    IDIV_MIN_LATENCY" << " = " << "9" << endl;
    cout << "    IDIV_POSTPROCESSING_LATENCY" << " = " << "4" << endl;
    cout << "    IDIV_PREPROCESSING_LATENCY" << " = " << "2" << endl;
    cout << "    ENABLE_EARLY_OUT_FDIV0" << " = " << "1" << endl;
    cout << "    ENABLE_EARLY_OUT_FDIV_BY_POW2" << " = " << "1" << endl;
    cout << "    ENABLE_EARLY_OUT_FSQRT0" << " = " << "1" << endl;
    cout << "    ENABLE_EARLY_OUT_FSQRT_BY_POW4" << " = " << "1" << endl;
    cout << "    ENABLE_EARLY_OUT_SSE0" << " = " << "1" << endl;
    cout << "    ENABLE_EARLY_OUT_SSE_DIV_BY_POW2" << " = " << "1" << endl;
    cout << "    ENABLE_EARLY_OUT_SSE_SQRT_BY_POW4" << " = " << "1" << endl;
    cout << "    ENABLE_FORCE_IDIV" << " = " << "0" << endl;
    cout << "    ENABLE_FORCE_FDIV" << " = " << "0" << endl;
    cout << "    ENABLE_FORCE_SDIV" << " = " << "0" << endl;
    cout << "    ENABLE_FORCE_FSQRT" << " = " << "0" << endl;
    cout << "    ENABLE_FORCE_SSQRT" << " = " << "0" << endl;
    cout << "    ENABLE_FORCE_FREM" << " = " << "0" << endl;
    cout << "    FORCE_IDIV_LATENCY" << " = " << "10" << endl;
    cout << "    FORCE_FDIV_LATENCY" << " = " << "10" << endl;
    cout << "    FORCE_SDIV_LATENCY" << " = " << "10" << endl;
    cout << "    FORCE_FSQRT_LATENCY" << " = " << "10" << endl;
    cout << "    FORCE_SSQRT_LATENCY" << " = " << "10" << endl;
    cout << "    FORCE_FREM_LATENCY" << " = " << "10" << endl;
    cout << "    EXEC_2_RE_PREDICTOR_LATENCY" << " = " << "1" << endl;
    cout << "    MS_2_RE_PREDICTOR_LATENCY" << " = " << "1" << endl;
    cout << "    ROB_2_EH_LATENCY" << " = " << "0" << endl;
    cout << "    EXEC_2_EH_LATENCY" << " = " << "0" << endl;
    cout << "    RAT_AVF_SIMULATE_COUNT" << " = " << "10000000" << endl;
    cout << "    RAT_AVF_COOLDOWN_COUNT" << " = " << "0" << endl;
    cout << "    RAT_AVF_WARMUP_COUNT" << " = " << "0" << endl;
    cout << "    RAT_AVF_GC_INTERVAL" << " = " << "50000" << endl;
    cout << "    RAT_REC_AVF_SIMULATE_COUNT" << " = " << "10000000" << endl;
    cout << "    RAT_REC_AVF_COOLDOWN_COUNT" << " = " << "0" << endl;
    cout << "    RAT_REC_AVF_WARMUP_COUNT" << " = " << "0" << endl;
    cout << "    RAT_REC_AVF_GC_INTERVAL" << " = " << "50000" << endl;
    cout << "    RRF_AVF_SIMULATE_COUNT" << " = " << "10000000" << endl;
    cout << "    RRF_AVF_WARMUP_COUNT" << " = " << "10000000" << endl;
    cout << "    RRF_AVF_COOLDOWN_COUNT" << " = " << "10000000" << endl;
}
