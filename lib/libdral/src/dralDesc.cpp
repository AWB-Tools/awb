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
 * @file dralDesc.cpp
 * @author Brian Slechta
 * @brief dral descriptors
 */

using namespace std;

#include "asim/dralDesc.h"

#define ENABLE_VALIST_CODE COMPILE_DRAL_WITH_PTV

// ----------------------------------------------------------------------------
char *
hack_ptv_s(
    const char * n)
{
    // create a new string "chars" that contains postfix appended to "n"
    static const size_t post_size = strlen(HACK_PTV_NAME_POSTFIX_STR);
    const size_t n_size = strlen(n);
    
    char *chars = new char[post_size+n_size+1];

    strcpy(&chars[0], n);
    strcpy(&chars[n_size], HACK_PTV_NAME_POSTFIX_STR);

    return chars;
}

// ----------------------------------------------------------------------------
DRAL_ITEM_DESC_CLASS::DRAL_ITEM_DESC_CLASS()
  : name("none"),
    desc("none"),
    ptvRecType(NULL)
{
    return;
}

// ----------------------------------------------------------------------------
DRAL_ITEM_DESC_CLASS::DRAL_ITEM_DESC_CLASS(
    const char *n, 
    const char *d)
  : name(n),
    desc(d),
    ptvRecType(NULL)
{
#if COMPILE_DRAL_WITH_PTV
    ptvRecType = pipe_new_recordtype(hack_ptv_s(n), d);
#endif
    
    return;
}

// ----------------------------------------------------------------------------
DRAL_ITEM_DESC_CLASS::~DRAL_ITEM_DESC_CLASS()
{
    return;
}

// ----------------------------------------------------------------------------
void
DRAL_ITEM_DESC_CLASS::Construct(
    const char *n, 
    const char *d)
{
#if COMPILE_DRAL_WITH_PTV
    ptvRecType = pipe_new_recordtype(hack_ptv_s(n), d);
#endif
    
    name = n;
    desc = d;
    
    return;
}

// ----------------------------------------------------------------------------
std::ostream & operator << (std::ostream & os, const DRAL_ITEM_DESC_CLASS& x)
{
    os <<  "N:" << x.GetName()
       << " D:" << x.GetDesc()
      ;

    return os;    
}

// ----------------------------------------------------------------------------
DRAL_EVENT_DESC_CLASS::DRAL_EVENT_DESC_CLASS()
  : name("none"),
    desc("none"),
    letter('?'),
    color(DRAL_COLOR_NONE),
    pdl(0),
    options(0),
    ptvEventType(NULL)
{
    return;
}

// ----------------------------------------------------------------------------
DRAL_EVENT_DESC_CLASS::DRAL_EVENT_DESC_CLASS(
    const char *n, 
    UINT32 l, 
    DRAL_COLOR_T c, 
    const char *d, 
    UINT32 p, 
    UINT32 o, 
    char k[64])
  : name(n),
    desc(d),
    letter(l),
    color(c),
    pdl(p),
    options(o),
    ptvEventType(NULL)
{
#if COMPILE_DRAL_WITH_PTV
    ptvEventType = pipe_new_eventtype(hack_ptv_s(n), l, CvtColorDralToPtv(c), d, p, o, k);
#endif    

    memcpy(knob, k, 64);

    return;
}

// ----------------------------------------------------------------------------
DRAL_EVENT_DESC_CLASS::~DRAL_EVENT_DESC_CLASS()
{
    return;
}

// ----------------------------------------------------------------------------
void
DRAL_EVENT_DESC_CLASS::Construct(
    const char *n, 
    UINT32 l, 
    DRAL_COLOR_T c, 
    const char *d, 
    UINT32 p, 
    UINT32 o, 
    char k[64])
{
#if COMPILE_DRAL_WITH_PTV
    ptvEventType = pipe_new_eventtype(hack_ptv_s(n), l, CvtColorDralToPtv(c), d, p, o, k);
#endif    

    name = n;
    desc = d;
    letter = l;
    color = c;
    pdl = p;
    options = o;
    memcpy(knob, k, 64);
    
    return;
}

// ----------------------------------------------------------------------------
PTV_COLOR_T  
DRAL_EVENT_DESC_CLASS::CvtColorDralToPtv(
    const DRAL_COLOR_T dral_color) 
{
#if COMPILE_DRAL_WITH_PTV
    switch(dral_color)
    {
    case DRAL_COLOR_NONE       : return Pipe_No_Color  ;
    case DRAL_COLOR_DARK_BLUE  : return Pipe_Dark_Blue ;
    case DRAL_COLOR_BLUE       : return Pipe_Blue      ;
    case DRAL_COLOR_CYAN       : return Pipe_Cyan      ;
    case DRAL_COLOR_DARK_GREEN : return Pipe_Dark_Green;
    case DRAL_COLOR_GREEN      : return Pipe_Green     ;
    case DRAL_COLOR_YELLOW     : return Pipe_Yellow    ;
    case DRAL_COLOR_ORANGE     : return Pipe_Orange    ;
    case DRAL_COLOR_RED        : return Pipe_Red       ;
    case DRAL_COLOR_PINK       : return Pipe_Pink      ;
    case DRAL_COLOR_MAGENTA    : return Pipe_Magenta   ;
    case DRAL_COLOR_PURPLE     : return Pipe_Purple    ;
    case DRAL_COLOR_BROWN      : return Pipe_Brown     ;
    case DRAL_COLOR_GRAY       : return Pipe_Gray      ;
    case DRAL_COLOR_LIGHT_GRAY : return Pipe_Light_Gray;
    case DRAL_COLOR_WHITE      : return Pipe_White     ;
    case DRAL_COLOR_BLACK      : return Pipe_Black     ;
    default                    : return Pipe_No_Color  ;
    }
#else
    return dral_color;
#endif
}

// ----------------------------------------------------------------------------
DRAL_COLOR_T 
DRAL_EVENT_DESC_CLASS::CvtColorPtvToDral(
    const PTV_COLOR_T ptv_color) 
{
#if COMPILE_DRAL_WITH_PTV
    switch(ptv_color)
    {
    case Pipe_No_Color   : return DRAL_COLOR_NONE      ;
    case Pipe_Dark_Blue  : return DRAL_COLOR_DARK_BLUE ;
    case Pipe_Blue       : return DRAL_COLOR_BLUE      ;
    case Pipe_Cyan       : return DRAL_COLOR_CYAN      ;
    case Pipe_Dark_Green : return DRAL_COLOR_DARK_GREEN;
    case Pipe_Green      : return DRAL_COLOR_GREEN     ;
    case Pipe_Yellow     : return DRAL_COLOR_YELLOW    ;
    case Pipe_Orange     : return DRAL_COLOR_ORANGE    ;
    case Pipe_Red        : return DRAL_COLOR_RED       ;
    case Pipe_Pink       : return DRAL_COLOR_PINK      ;
    case Pipe_Magenta    : return DRAL_COLOR_MAGENTA   ;
    case Pipe_Purple     : return DRAL_COLOR_PURPLE    ;
    case Pipe_Brown      : return DRAL_COLOR_BROWN     ;
    case Pipe_Gray       : return DRAL_COLOR_GRAY      ;
    case Pipe_Light_Gray : return DRAL_COLOR_LIGHT_GRAY;
    case Pipe_White      : return DRAL_COLOR_WHITE     ;
    case Pipe_Black      : return DRAL_COLOR_BLACK     ;
    default              : return DRAL_COLOR_NONE      ;
    }
#else
    return ptv_color;
#endif
}

// ----------------------------------------------------------------------------
const char * 
DRAL_EVENT_DESC_CLASS::GetDralColorStr(
    const DRAL_COLOR_T dral_color)
{
    if ((dral_color < DRAL_COLOR_NONE) || (dral_color > DRAL_COLOR_END))
    {
        return "?ERR?";
    }
    else
    {
        return DRAL_COLOR_STR[dral_color];
    }
}

// ----------------------------------------------------------------------------
std::ostream & operator << (std::ostream & os, const DRAL_EVENT_DESC_CLASS& x)
{
    os << dec
       << "N:" << x.GetName()
       << " D:" << x.GetDesc()
       << " L:" << (char) x.GetLetter()
       << " C:" << x.GetColorStr()
       << " P:" << hex << x.GetPdl()
       << " O:" << hex << x.GetOptions()
       << " K:" << x.GetKnob()
       << dec;

    return os;
}

// ----------------------------------------------------------------------------
DRAL_DATA_DESC_CLASS::DRAL_DATA_DESC_CLASS()
  : name("none"),
    desc("none"),
    bdt(DRAL_BASE_DATA_INVALID),
    len(0),
    strs(0),
    ptvDataType(NULL)
{
    return;
}

// ----------------------------------------------------------------------------
DRAL_DATA_DESC_CLASS::DRAL_DATA_DESC_CLASS(
    const char *n, 
    DRAL_BASE_DATA_T b,
    const char *d, 
    UINT32 l, 
    char **s)
  : name(n),
    desc(d),
    bdt(b),
    len(l),
    strs(s),
    ptvDataType(NULL)
{
#if COMPILE_DRAL_WITH_PTV
    ptvDataType = pipe_new_datatype(hack_ptv_s(n), CvtBaseDataDralToPtv(b), d, l, s);
#endif

    return;
}

// ----------------------------------------------------------------------------
DRAL_DATA_DESC_CLASS::~DRAL_DATA_DESC_CLASS()
{
    return;
}

// ----------------------------------------------------------------------------
void
DRAL_DATA_DESC_CLASS::Construct(
    const char *n, 
    DRAL_BASE_DATA_T b,
    const char *d, 
    UINT32 l, 
    char **s)
{
#if COMPILE_DRAL_WITH_PTV
    ptvDataType = pipe_new_datatype(hack_ptv_s(n), CvtBaseDataDralToPtv(b), d, l, s);
#endif

    name = n;
    desc = d;
    bdt = b;
    len = l;
    strs = s;

    return;
}

// ----------------------------------------------------------------------------
// given the var-args list pointer to the data value, return the string
// representation of the data.
std::string 
DRAL_DATA_DESC_CLASS::GetDralDataStr(
    va_list & ap,
    bool advance_ap)
{
    std::string data_str("");

#if ENABLE_VALIST_CODE

    va_list tmpap = ap;
    DRAL_BASE_DATA_VAL_T val;

    switch (bdt)
    {
    case DRAL_BASE_DATA_INVALID:
      break;

    case DRAL_BASE_DATA_BOOL:
    case DRAL_BASE_DATA_INT32:
      {
          val.i32 = va_arg(tmpap, INT32);
          char s[30];
          sprintf(s, "%d", val.i32);
          data_str +=s;
      }
      break;

    case DRAL_BASE_DATA_INT32H:
    case DRAL_BASE_DATA_ASSOCIATION:
      {
          val.u32 = va_arg(tmpap, UINT32);
          char s[30];
          sprintf(s, "%X", val.u32);
          data_str +=s;
      }                                               
      break;

    case DRAL_BASE_DATA_ENUM:
      {
          val.i32 = va_arg(tmpap, INT32);
          char s[30];
          sprintf(s, "%d", val.i32);
          data_str +=s;
      }
      break;

    case DRAL_BASE_DATA_STRING:
      {
          val.chars = va_arg(tmpap, char *);
          data_str += val.chars;
      }
      break;

    case DRAL_BASE_DATA_INT8_ARRAY:
      {
          val.data = va_arg(tmpap, INT8 *);
          char *s = new char[len+1];
          for (UINT32 n = 0; n < len; ++n)
          {
              sprintf(&s[n], "%X", val.data[n]);
          }
          s[len] = '\0';
          data_str += s;
          delete[] s;
      }
      break;

    case DRAL_BASE_DATA_INT64H:
      {
          val.u64 = va_arg(tmpap, UINT64);
          char s[30];
          sprintf(s, "%llX", val.u64);
          data_str +=s;
      }
      break;

    case DRAL_BASE_DATA_END:
      break;

    default:
      break;

    }    

    if (advance_ap)
    {
        ap = tmpap;
    }

#endif

    return data_str;
}

// ----------------------------------------------------------------------------
// given the var-args list pointer to the data value, return the string
// representation of the data.
UINT32
DRAL_DATA_DESC_CLASS::GetVaSize(
    va_list ap)
{
    switch (bdt)
    {
    case DRAL_BASE_DATA_INVALID:
      return 0;

    case DRAL_BASE_DATA_BOOL:
    case DRAL_BASE_DATA_INT32:
      return sizeof(INT32);

    case DRAL_BASE_DATA_INT32H:
    case DRAL_BASE_DATA_ASSOCIATION:
      return sizeof(UINT32);

    case DRAL_BASE_DATA_ENUM:
      return sizeof(INT32);

    case DRAL_BASE_DATA_STRING:
      return sizeof(char *);

    case DRAL_BASE_DATA_INT8_ARRAY:
      return sizeof(INT8 *);

    case DRAL_BASE_DATA_INT64H:
      return sizeof(UINT64);

    case DRAL_BASE_DATA_END:
    default:
      return 0;
    }
}

// ----------------------------------------------------------------------------
PTV_BASE_DATA_T
DRAL_DATA_DESC_CLASS::CvtBaseDataDralToPtv(
    const DRAL_BASE_DATA_T dral_data) 
{
#if COMPILE_DRAL_WITH_PTV
    switch (dral_data)
    {
    case DRAL_BASE_DATA_INVALID     : return Pipe_Invalid_Data  ;
    case DRAL_BASE_DATA_BOOL        : return Pipe_Boolean       ;
    case DRAL_BASE_DATA_INT32       : return Pipe_Integer       ;
    case DRAL_BASE_DATA_INT32H      : return Pipe_Hex_Integer   ;
    case DRAL_BASE_DATA_ASSOCIATION : return Pipe_Association   ;
    case DRAL_BASE_DATA_ENUM        : return Pipe_Enumeration   ;
    case DRAL_BASE_DATA_STRING      : return Pipe_String        ;
    case DRAL_BASE_DATA_INT8_ARRAY  : return Pipe_ByteArray     ;
    case DRAL_BASE_DATA_INT64H      : return Pipe_Hex_Integer64 ;
    case DRAL_BASE_DATA_END         : return Pipe_Max_Data_Types;
    default                         : return Pipe_Invalid_Data  ;
    }
#else
    return dral_data;
#endif
}

// ----------------------------------------------------------------------------
DRAL_BASE_DATA_T
DRAL_DATA_DESC_CLASS::CvtBaseDataPtvToDral(
    const PTV_BASE_DATA_T ptv_data)  
{
#if COMPILE_DRAL_WITH_PTV
    switch (ptv_data)
    {
    case Pipe_Invalid_Data   : return DRAL_BASE_DATA_INVALID    ;
    case Pipe_Boolean        : return DRAL_BASE_DATA_BOOL       ;
    case Pipe_Integer        : return DRAL_BASE_DATA_INT32      ;
    case Pipe_Hex_Integer    : return DRAL_BASE_DATA_INT32H     ;
    case Pipe_Association    : return DRAL_BASE_DATA_ASSOCIATION;
    case Pipe_Enumeration    : return DRAL_BASE_DATA_ENUM       ;
    case Pipe_String         : return DRAL_BASE_DATA_STRING     ;
    case Pipe_ByteArray      : return DRAL_BASE_DATA_INT8_ARRAY ;
    case Pipe_Hex_Integer64  : return DRAL_BASE_DATA_INT64H     ;
    case Pipe_Max_Data_Types : return DRAL_BASE_DATA_END        ;
    default                  : return DRAL_BASE_DATA_INVALID    ;
    }
#else
    return ptv_data;
#endif
}

// ----------------------------------------------------------------------------
// get the var-arg size of the base data
UINT32 
DRAL_DATA_DESC_CLASS::GetDralBaseDataSize(
    const DRAL_BASE_DATA_T dral_data)
{
    switch (dral_data)
    {
    case DRAL_BASE_DATA_INVALID     : return 0;
    case DRAL_BASE_DATA_BOOL        : return sizeof(INT32);
    case DRAL_BASE_DATA_INT32       : return sizeof(INT32);
    case DRAL_BASE_DATA_INT32H      : return sizeof(UINT32);
    case DRAL_BASE_DATA_ASSOCIATION : return sizeof(UINT32);
    case DRAL_BASE_DATA_ENUM        : return sizeof(INT32);
    case DRAL_BASE_DATA_STRING      : return sizeof(char *);
    case DRAL_BASE_DATA_INT8_ARRAY  : return sizeof(INT8 *);
    case DRAL_BASE_DATA_INT64H      : return sizeof(UINT64);
    case DRAL_BASE_DATA_END         : return 0;
    default                         : return 0;
    }    
}

// ----------------------------------------------------------------------------
const char *
DRAL_DATA_DESC_CLASS::GetDralBaseDataStr(
    const DRAL_BASE_DATA_T dral_data)
{
    if ((dral_data < DRAL_BASE_DATA_INVALID) || (dral_data > DRAL_BASE_DATA_END))
    {
        return "?ERR?";
    }
    else
    {
        return DRAL_BASE_DATA_STR[dral_data];
    }
}

// ----------------------------------------------------------------------------
std::ostream & operator << (std::ostream & os, const DRAL_DATA_DESC_CLASS& x)
{
    os << dec 
       <<  "N:" << x.GetName()
       << " D:" << x.GetDesc()
       << " B:" << x.GetBaseDataStr()
       << " O:" << x.GetLen()
       << " FINISH ME"
       << dec;

    return os;    
}
