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

/**
 * @file dralDesc.h
 * @author Brian Slechta
 * @brief dral descriptors
 */

#ifndef __DRAL_DESC__H__
#define __DRAL_DESC__H__

#include <ostream>
#include <string>

// for var args
#include <stdarg.h>

#include "asim/dral_syntax.h"

// there are "uniqueness" issues when you link in with coho libraries.  we can 
// around that by appending all name strings with this string.
static const char * HACK_PTV_NAME_POSTFIX_STR = "_";

#if COMPILE_DRAL_WITH_PTV
// ----------------------------------------------------------------------------
// -----------------------------BEGIN UGLY HACK -------------------------------
// ----------------------------------------------------------------------------
// FIXME: ALL OF THIS WILL BE COMPLETELY REMOVED.  WE HAVE CERTAIN ISSUES WITH
// DRAL BEING DEPENDANT ON PTV WHILE PTV IS NOT IN SIMCORE.  ONCE PTV IS IN
// SIMCORE, WE CAN REMOVE THIS UGLY HORRIBLE SECTION OF CODE
/*HACK*/enum Pipe_Color {
/*HACK*/        Pipe_No_Color,
/*HACK*/        Pipe_Dark_Blue,
/*HACK*/        Pipe_Blue,
/*HACK*/        Pipe_Cyan,
/*HACK*/        Pipe_Dark_Green,
/*HACK*/        Pipe_Green,
/*HACK*/        Pipe_Yellow,
/*HACK*/        Pipe_Orange,
/*HACK*/        Pipe_Red,
/*HACK*/        Pipe_Pink,
/*HACK*/        Pipe_Magenta,
/*HACK*/        Pipe_Purple,
/*HACK*/        Pipe_Brown,
/*HACK*/        Pipe_Gray,
/*HACK*/        Pipe_Light_Gray,
/*HACK*/        Pipe_White,
/*HACK*/        Pipe_Black
/*HACK*/};
/*HACK*/
/*HACK*/typedef enum Pipe_Color Pipe_Color;
/*HACK*/
/*HACK*/enum Base_Data_Type {
/*HACK*/        Pipe_Invalid_Data=0,
/*HACK*/        Pipe_Boolean,
/*HACK*/        Pipe_Integer,
/*HACK*/        Pipe_Hex_Integer,
/*HACK*/        Pipe_String,
/*HACK*/        Pipe_Association,
/*HACK*/        Pipe_Enumeration,
/*HACK*/        Pipe_ByteArray,
/*HACK*/        Pipe_Hex_Integer64,
/*HACK*/        Pipe_Max_Data_Types
/*HACK*/};
/*HACK*/typedef enum Base_Data_Type Base_Data_Type;
/*HACK*/struct Pipe_Record;
/*HACK*/struct Pipe_Event;
/*HACK*/struct Pipe_Data;
/*HACK*/typedef UINT32 Pipe_RecId;
/*HACK*/typedef class Pipe_Recordtype Pipe_Recordtype;
/*HACK*/typedef class Pipe_Datatype Pipe_Datatype;
/*HACK*/typedef class Pipe_Eventtype Pipe_Eventtype;
/*HACK*/typedef struct Pipe_Cookie Pipe_Cookie;
/*HACK*/#define PIPE_PDL_MIN 0x0
/*HACK*/#define PIPE_PDL_MAX 0xff 
/*HACK*/#define NO_TRACE_FILE  0
/*HACK*/#define TRACE_FILE_DEFAULT 1
/*HACK*/#define PIPE_NO_DATATYPE	((Pipe_Datatype *) -3)
/*HACK*/#define PIPE_NO_RECORD		((Pipe_RecId) 0)
/*HACK*/#define PIPE_NO_THREADID	(-1)
/*HACK*/typedef enum {
/*HACK*/        PT_Binary,
/*HACK*/        PT_ASCII,
/*HACK*/        PT_Unknown
/*HACK*/} PT_Format;
/*HACK*/extern void pipe_init(UINT32 min_records);
/*HACK*/extern void pipe_enable_collection();
/*HACK*/extern void pipe_open_file(const char *filename, 
/*HACK*/                           PT_Format format,
/*HACK*/                           Pipe_RecId first, 
/*HACK*/                           int pdl = PIPE_PDL_MAX,
/*HACK*/			   int * pipetrace_id = NULL);
/*HACK*/extern void pipe_close_file(Pipe_RecId last, int pTraceId = TRACE_FILE_DEFAULT );
/*HACK*/extern Pipe_Recordtype *pipe_new_recordtype(const char *name, const char *desc);
/*HACK*/extern Pipe_Datatype *pipe_new_datatype(const char *name, Base_Data_Type bdt,
/*HACK*/					const char *desc, int len = 0, char **strs = NULL);
/*HACK*/extern Pipe_Eventtype *pipe_new_eventtype(const char *name, int letter,
/*HACK*/					  Pipe_Color color, const char *desc,  UINT32 pdl = 0xff, UINT32 options = 0, char knob[64] = "");
/*HACK*/extern Pipe_RecId pipe_open_record_inst(Pipe_Recordtype *rt, INT32 thread_id, Pipe_RecId parent, int pTraceId = TRACE_FILE_DEFAULT, const char *filename = 0, int linenum = 0 );
/*HACK*/extern Pipe_RecId pipe_reference_record_inst(Pipe_RecId recid, const char *filename = 0, int linenum = 0);
/*HACK*/extern void pipe_close_record_inst(Pipe_RecId recid, const char *filename = 0, int linenum = 0);
/*HACK*/extern void pipe_close_record_inst_completely(Pipe_RecId recid, const char *filename = 0, int linenum = 0);
/*HACK*/
/*HACK*///extern void pipe_record_data(Pipe_RecId recid, Pipe_Datatype *dt, ...);
/*HACK*/extern void pipe_record_data_va(Pipe_RecId recid, Pipe_Datatype *dt, va_list* ap);
/*HACK*/extern void pipe_record_event(Pipe_RecId recid, Pipe_Eventtype *et,
/*HACK*/			      UINT32 duration, Pipe_Datatype *dt = PIPE_NO_DATATYPE, ...);
/*HACK*/extern void pipe_record_event(Pipe_RecId recid, Pipe_Eventtype *et,
/*HACK*/			      UINT32 duration, Pipe_Datatype *dt, va_list* ap);
/*HACK*/extern Pipe_Cookie *pipe_record_event_begin(Pipe_RecId recid, Pipe_Eventtype *et);
/*HACK*/extern void pipe_record_event_end(Pipe_Cookie *cookie);
/*HACK*/extern void pipe_comment(char *comment, char newline_delimiter = 0);
/*HACK*/
/*HACK*/// Please only use this next one if ABSOLUTELY NEED to specify a different time
/*HACK*///  than the current thread_cycle.  The time cannot be before the opening time
/*HACK*///  of the record it is posted to.
/*HACK*///extern void pipe_record_event_time(Pipe_RecId recid, Pipe_Eventtype *et,
/*HACK*///				   UINT32 time, UINT32 duration,
/*HACK*///				   Pipe_Datatype *dt = PIPE_NO_DATATYPE, ...);
/*HACK*/extern void pipe_record_event_time(Pipe_RecId recid, Pipe_Eventtype *et,
/*HACK*/				   UINT32 time, UINT32 duration,
/*HACK*/				   Pipe_Datatype *dt, va_list* ap);
/*HACK*/extern bool pipe_record_exists(Pipe_RecId recid);
/*HACK*/extern void pipe_threads_per_cores_per_procs(int nt, int nc, int np);
/*HACK*/extern void pipe_threads_per_proc(int n);
/*HACK*/extern void pipe_threads_total(int n);
/*HACK*/// ----------------------------------------------------------------------------
/*HACK*/// -------------------------------END UGLY HACK -------------------------------
/*HACK*/// ----------------------------------------------------------------------------
/*HAXK*/extern void thread_initialize(int init_size = 0);
/*HACK*/extern void pipe_disable_implicit_stats();
/*HACK*/extern void pipe_dump_buffer(const char *filename, UINT32 max_records, PT_Format format);
/*HACK*/extern void pipe_set_will_dump();
#endif

// ----------------------------------------------------------------------------
// these are display colors for the viewer.  they are originally derived from
// PTV colors, but we plan to support them in DRAL as well.
enum DRAL_COLOR_ENUM
{
    DRAL_COLOR_NONE=0,
    DRAL_COLOR_DARK_BLUE,
    DRAL_COLOR_BLUE,
    DRAL_COLOR_CYAN,
    DRAL_COLOR_DARK_GREEN,
    DRAL_COLOR_GREEN,
    DRAL_COLOR_YELLOW,
    DRAL_COLOR_ORANGE,
    DRAL_COLOR_RED,
    DRAL_COLOR_PINK,
    DRAL_COLOR_MAGENTA,
    DRAL_COLOR_PURPLE,
    DRAL_COLOR_BROWN,
    DRAL_COLOR_GRAY,
    DRAL_COLOR_LIGHT_GRAY,
    DRAL_COLOR_WHITE,
    DRAL_COLOR_BLACK,
    DRAL_COLOR_END
};
typedef enum DRAL_COLOR_ENUM DRAL_COLOR_T;

// ----------------------------------------------------------------------------
// these are the print strings for the colors above.  it is much safer to use
// the function DRAL_EVENT_DESC_CLASS::GetDralColorStr() because it has array
// bounds checking.
static const char * DRAL_COLOR_STR[DRAL_COLOR_END-DRAL_COLOR_NONE+1] =
{
    "NONE",
    "DARK_BLUE",
    "BLUE",
    "CYAN",
    "DARK_GREEN",
    "GREEN",
    "YELLOW",
    "ORANGE",
    "RED",
    "PINK",
    "MAGENTA",
    "PURPLE",
    "BROWN",
    "GRAY",
    "LIGHT_GRAY",
    "WHITE",
    "BLACK",
    "END"
};

// ----------------------------------------------------------------------------
// these are display data types for the viewer.  they are originally derived from
// PTV data types, but we plan to support them in DRAL as well.  they are also
// used for the var-args call to SetEvent() for indicating size of the value
// arguments.
enum DRAL_BASE_DATA_ENUM
{
    DRAL_BASE_DATA_INVALID=0,
    DRAL_BASE_DATA_BOOL,
    DRAL_BASE_DATA_INT32,
    DRAL_BASE_DATA_INT32H,
    DRAL_BASE_DATA_ASSOCIATION,
    DRAL_BASE_DATA_ENUM,
    DRAL_BASE_DATA_STRING,
    DRAL_BASE_DATA_INT8_ARRAY,
    DRAL_BASE_DATA_INT64H,
    DRAL_BASE_DATA_END
};
typedef enum DRAL_BASE_DATA_ENUM DRAL_BASE_DATA_T;

// ----------------------------------------------------------------------------
// this are the print strings for the base data types above  it is much safer 
// to use the function DRAL_DATA_DESC_CLASS::GetDralBaseDataStr() because 
// it has array bounds checking.
static const char * DRAL_BASE_DATA_STR[DRAL_BASE_DATA_END-DRAL_BASE_DATA_INVALID+1] =
{
    "INVALID",
    "BOOL",
    "INT32",
    "INT32H",
    "ASSOCIATION",
    "ENUM",
    "STRING",
    "INT8_ARRAY",
    "INT64H",
    "END"
};

// ----------------------------------------------------------------------------
union DRAL_BASE_DATA_VAL_UNION
{
    UINT32 u32;
    INT32  i32;
    UINT64 u64;
    INT8*  data;
    char*  chars;
};
typedef union DRAL_BASE_DATA_VAL_UNION DRAL_BASE_DATA_VAL_T;

// ----------------------------------------------------------------------------
// we currently need the ability to compile with and without PTV dependance.
// these declarations help resolve that.
#if COMPILE_DRAL_WITH_PTV
typedef class Pipe_Recordtype PTV_REC_TYPE_CLASS;
typedef class Pipe_Datatype PTV_DATA_TYPE_CLASS;
typedef class Pipe_Eventtype PTV_EVENT_TYPE_CLASS;
typedef enum Pipe_Color PTV_COLOR_T;
typedef enum Base_Data_Type PTV_BASE_DATA_T;
#else
typedef void PTV_REC_TYPE_CLASS;
typedef void PTV_DATA_TYPE_CLASS;
typedef void PTV_EVENT_TYPE_CLASS;
typedef DRAL_COLOR_T PTV_COLOR_T;
typedef DRAL_BASE_DATA_T PTV_BASE_DATA_T;
#endif

// ----------------------------------------------------------------------------
// The DRAL Item Descriptor (or PTV record type) by providing a  name and a desc.
class DRAL_ITEM_DESC_CLASS
{
  public:
    // we really don't have a use for this... we leave it public in case
    // someone has an array of these.
    DRAL_ITEM_DESC_CLASS();

    // this constuctor should look like the pipe_new_eventtype() function call
    // from the PTV library.  this is the main contructor.  See Construct() for 
    // more details.
    DRAL_ITEM_DESC_CLASS(const char *name, const char *desc);

    ~DRAL_ITEM_DESC_CLASS();

    // this is the main constuction routine.  you can either call the full
    // constructor or use the default constructor along with this function.
    // @param name - name of the record
    // @param desc - description of the record
    void Construct(const char *name, const char *desc);

    // direct queries
    std::string GetName() const { return name; };
    std::string GetDesc() const { return desc; };

    // we may change this class to contain "Pipe_Eventtype" rather than derive
    // from it.  Use this function for returning the PTV event object contained
    // within this class.
    PTV_REC_TYPE_CLASS *GetPtvRecType() const { return ptvRecType; };

  private:
    
    // name and description of the event
    std::string  name; 
    std::string  desc; 

    // encapsulated PTV event
    PTV_REC_TYPE_CLASS *ptvRecType;

  public:
    friend std::ostream & operator << (std::ostream & os, const DRAL_ITEM_DESC_CLASS& x);
};

std::ostream & operator << (std::ostream & os, const DRAL_ITEM_DESC_CLASS& x);

// ----------------------------------------------------------------------------
// The DRAL Event Descriptor.  It defines the "event" (or PTV event) by
// providing a name, a display letter, a display color, a description, and
// some other option fields.
class DRAL_EVENT_DESC_CLASS
{
  public:
    // we really don't have a use for this... we leave it public in case
    // someone has an array of these.
    DRAL_EVENT_DESC_CLASS();

    // this constuctor should look like the pipe_new_eventtype() function call
    // from the PTV library.  this is the main contructor.  See Construct() for 
    // more details.
    DRAL_EVENT_DESC_CLASS(const char *name, UINT32 letter, DRAL_COLOR_T color, 
                           const char *desc, UINT32 pdl = 0xff, 
                           UINT32 options = 0, char knob[64] = "");

    ~DRAL_EVENT_DESC_CLASS();

    // this is the main constuction routine.  you can either call the full
    // constructor or use the default constructor along with this function.
    // @param name - name of the "event"
    // @param letter - character to display in the viewer for this event
    // @param color - color to display in the viewer for this event
    // @param desc - description of the data member
    // @param pdl - ?
    // @param options - ?
    // @param strs - ?
    void Construct(const char *name, UINT32 letter, DRAL_COLOR_T color, 
                   const char *desc, UINT32 pdl = 0xff, 
                   UINT32 options = 0, char knob[64] = "");

    // direct queries
    std::string      GetName()    const { return name;  };
    std::string      GetDesc()    const { return desc;  };
    UINT32           GetLetter()  const { return letter; };
    DRAL_COLOR_T     GetColor()   const { return color; };
    UINT32           GetPdl()     const { return pdl; };
    UINT32           GetOptions() const { return options; };
    const char*      GetKnob()    const { return knob; };

    // we may change this class to contain "Pipe_Eventtype" rather than derive
    // from it.  Use this function for returning the PTV event object contained
    // within this class.
    PTV_EVENT_TYPE_CLASS *GetPtvEventType() const { return ptvEventType; };

    // indirect queries
    const char *GetColorStr() const { return GetDralColorStr(color); };

    // conversion functions from PTV to DRAL enumerations
    static PTV_COLOR_T  CvtColorDralToPtv(const DRAL_COLOR_T dral_color);
    static DRAL_COLOR_T CvtColorPtvToDral(const PTV_COLOR_T  ptv_color);

    // safe print string function for dral color
    static const char * GetDralColorStr(const DRAL_COLOR_T dral_color);

  private:
    
    // name and description of the event
    std::string  name; 
    std::string  desc; 

    // event attributes
    UINT32       letter; 
    DRAL_COLOR_T color; 
    UINT32       pdl; 
    UINT32       options; 
    char         knob[64];

    // encapsulated PTV event
    PTV_EVENT_TYPE_CLASS *ptvEventType;

  public:
    friend std::ostream & operator << (std::ostream & os, const DRAL_EVENT_DESC_CLASS& x);
};

std::ostream & operator << (std::ostream & os, const DRAL_EVENT_DESC_CLASS& x);

// ----------------------------------------------------------------------------
// The DRAL Data Descriptor class.  It defines a data line for an event or item
// (or PTV event) by providing a name, description, and type information to be
// used in the display and logging of the data.  The main function is in the
// SetEvent() var-args function, and it is used as an indication for the size
// and type of the values in the var-args list.
class DRAL_DATA_DESC_CLASS
{
  public:
    // we really don't have a use for this... we leave it public in case
    // someone has an array of these.
    DRAL_DATA_DESC_CLASS();

    // this constuctor should look like the pipe_new_datatype() function call
    // from the PTV library.  this is the main contructor.  See Construct() for
    // more details
    DRAL_DATA_DESC_CLASS(const char *name, DRAL_BASE_DATA_T bdt,
                           const char *desc, UINT32 len = 0, char **strs = NULL);

    ~DRAL_DATA_DESC_CLASS();

    // this is the main constuction routine.  you can either call the full
    // constructor or use the default constructor along with this function.
    // @param name - name of the data member
    // @param bdt - basic data type of the data member
    // @param desc - description of the data member
    // @param len - option length of bdt in the case of an array
    // @param strs - ?
    void Construct(const char *name, DRAL_BASE_DATA_T bdt,
                   const char *desc, UINT32 len = 0, char **strs = NULL);

    // direct queries
    std::string      GetName()     const { return name; };
    std::string      GetDesc()     const { return desc; };
    DRAL_BASE_DATA_T GetBaseData() const { return bdt;  };
    UINT32           GetLen()      const { return len;  };
    char**           GetStrs()     const { return strs; };

    // we may change this class to contain "Pipe_Eventtype" rather than derive
    // from it.  Use this function for returning the PTV event object contained
    // within this class.
    PTV_DATA_TYPE_CLASS *GetPtvDataType() const { return ptvDataType; };

    // indirect queries
    const char *GetBaseDataStr()  const { return GetDralBaseDataStr(bdt);  };
    UINT32      GetBaseDataSize() const { return GetDralBaseDataSize(bdt); };

    // given the var-args list pointer to the data value, return the string
    // representation of the data.  will not advance va_list
    std::string GetDralDataStr(va_list & ap, bool advance_ap = false);

    // the size of the va list in number of bytes
    UINT32 GetVaSize(va_list ap);

    // conversion functions from PTV to DRAL enumerations
    static PTV_BASE_DATA_T  CvtBaseDataDralToPtv(const DRAL_BASE_DATA_T dral_data);
    static DRAL_BASE_DATA_T CvtBaseDataPtvToDral(const PTV_BASE_DATA_T  ptv_data);
    
    // get the var-arg size of the base data
    static UINT32 GetDralBaseDataSize(const DRAL_BASE_DATA_T dral_data);

    // safe print string function for dral base data
    static const char * GetDralBaseDataStr(const DRAL_BASE_DATA_T dral_data);

  private:

    // name and description of the data
    std::string name;
    std::string desc;
    
    // attributes of the data
    DRAL_BASE_DATA_T bdt;
    UINT32 len;
    char **strs;

    // encapsulated PTV data type
    PTV_DATA_TYPE_CLASS *ptvDataType;
  public:
    friend std::ostream & operator << (std::ostream & os, const DRAL_DATA_DESC_CLASS& x);
};

std::ostream & operator << (std::ostream & os, const DRAL_DATA_DESC_CLASS& x);


#endif
