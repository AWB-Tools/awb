/**************************************************************************
 * INTEL CONFIDENTIAL Copyright (c) 2007 Intel Corp.
 *
 * Recipient is granted a non-sublicensable copyright license under
 * Intel copyrights to copy and distribute this code internally only.
 * This code is provided "AS IS" with no support and with no
 * warranties of any kind, including warranties of MERCHANTABILITY,
 * FITNESS FOR ANY PARTICULAR PURPOSE or INTELLECTUAL PROPERTY
 * INFRINGEMENT. By making any use of this code, Recipient agrees that
 * no other licenses to any Intel patents, trade secrets, copyrights
 * or other intellectual property rights are granted herein, and no
 * other licenses shall arise by estoppel, implication or by operation
 * of law. Recipient accepts all risks of use.
 **************************************************************************/

#include "asim/ptlib.h"

#if COMPILE_DRAL_WITH_PTV

void pipe_init(uint32_t min_records) { return; }
void pipe_enable_collection() { return; }
void pipe_open_file(const char *filename, PT_Format format, Pipe_RecId first, int pdl, int * pipetrace_id) { return; }
void pipe_close_file(Pipe_RecId last, int pTraceId) { return; }
Pipe_Recordtype *pipe_new_recordtype(const char *name, const char *desc) { return 0; }
Pipe_Datatype *pipe_new_datatype(const char *name, Base_Data_Type bdt, const char *desc, int len, char **strs) { return PIPE_NO_DATATYPE; }
Pipe_Eventtype *pipe_new_eventtype(const char *name, int letter, Pipe_Color color, const char *desc,  uint32_t pdl, uint32_t options, char knob[64]) { return 0; }
Pipe_RecId pipe_open_record_inst(Pipe_Recordtype *rt, int32_t thread_id, Pipe_RecId parent, int pTraceId, const char *filename, int linenum) { return PIPE_NO_RECORD; }
Pipe_RecId pipe_reference_record_inst(Pipe_RecId recid, const char *filename, int linenum) { return PIPE_NO_RECORD; }
void pipe_close_record_inst(Pipe_RecId recid, const char *filename, int linenum) { return; }
void pipe_close_record_inst_completely(Pipe_RecId recid, const char *filename, int linenum) { return; }
void pipe_record_data_va(Pipe_RecId recid, Pipe_Datatype *dt, va_list* ap) { return; }
void pipe_record_event(Pipe_RecId recid, Pipe_Eventtype *et, uint32_t duration, Pipe_Datatype *dt, ...) { return; }
void pipe_record_event(Pipe_RecId recid, Pipe_Eventtype *et, uint32_t duration, Pipe_Datatype *dt, va_list* ap) { return; }
Pipe_Cookie *pipe_record_event_begin(Pipe_RecId recid, Pipe_Eventtype *et) { return 0; }
void pipe_record_event_end(Pipe_Cookie *cookie) { return; }
void pipe_comment(char *comment, char newline_delimiter) { return; }
void pipe_record_event_time(Pipe_RecId recid, Pipe_Eventtype *et, uint32_t time, uint32_t duration, Pipe_Datatype *dt, va_list* ap) { return; }
bool pipe_record_exists(Pipe_RecId recid) { return false; }
void pipe_threads_per_cores_per_procs(int nt, int nc, int np) { return; }
void pipe_threads_per_proc(int n) { return; }
void pipe_threads_total(int n) { return; }
void thread_initialize(int init_size) { return; }
void pipe_disable_implicit_stats() { return; }
void pipe_dump_buffer(const char *filename, uint32_t max_records, PT_Format format) { return; }
void pipe_set_will_dump() { return; }

#endif
