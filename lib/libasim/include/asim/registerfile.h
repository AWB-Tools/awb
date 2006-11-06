/*
 *Copyright (C) 2005-2006 Intel Corporation
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

#ifndef _REGISTER_FILE_H
#define _REGISTER_FILE_H
//
// A REGISTER_FILE is data structure used to
// mimic hardware arrays (RegFiles, FIFOs, etc...) of a somewhat "tabular"
// nature. The goal of a REGISTER_FILE structure is to make the programmer
// happier by tracking fundamental stats about number of reads/writes and
// CAMs performed on the table. Additionally, the REGISTER_FILE generates
// automatic DRAL events for all the basic actions (writing something new
// into the table, readign from the table, etc..).
//
// Fundamentally, a REGISTER_FILE is an array that can hold arbitrary entries.
// Each entry is composed of fields and, just like in real structures, 
// each field can have a different number of RD/WR/CAM ports. Methods
// are provided to read/write/cam from fields on a particular port. The
// class will sanity check that the user does not exceed the number of
// valid reads/writes/cams per cycle in a given port/field pair.
//
// For programming convenience, REGISTER_FILE provides two types of fields: real
// bit-fields (64 bit max) and OBJECT fields, which hold an arbitrary C++
// type. They behave almost identically except that
//   - no "CAM"ing is allowed on OBJECT fields 
//   - the method arguments are slightly different.
//
// [TODO] On top of normal ports, we provide a "multiPort" capability, so that
// you can write N entries into the table using a single port (wider ports
// tend to be more power effective than multiple independent ports, hence
// then need to be able to track them separately).
// 
#include "asim/mm.h"
#include "asim/mpointer.h"
#include "asim/phase.h"
#include "asim/event.h"
#include "asim/item.h"
#include "asim/module.h"

#ifdef LIBSUPPORT_STATS
 #include "stats.h"
#else
 #define STAT2(E) 
#endif

#define BACKDOOR_PORT 31
#define ANY_PORT BACKDOOR_PORT 
#define INVALID_PORT 32



// This INIT_REGISTER_FILE should be included in one and only one .cpp file as a 
// global variable of the models whishing to use RegisterFiles.

#define INIT_REGISTER_FILE ASIM_MM_DEFINE(InternalState, 128);

typedef enum {
  MATCH_INVALID,
  MATCH_FIRST_ONLY,
  MATCH_ALL,
} MATCH_POLICY;

typedef struct {
UINT32 idx;
UINT64 value;
} LatchedData;

struct InternalState : public ASIM_MM_CLASS<InternalState>
{
  UINT32       ref;
  void         *pt;
  INT32        fld;
  INT32        start;
  INT32        end;
  UINT32       capacity;
  UINT32       nelem;
  LatchedData  *data;

  InternalState() : ASIM_MM_CLASS<InternalState>(0, 0) { };
  ~InternalState()
  {
   if ( data != NULL ) delete [] data;
  }
};
typedef class mmptr<InternalState> IS;

// Name of the field that holds the object of type T when T is a basic type
// Used to display in dreams.
#define ITEM_TAG_NAME "Item"

template <class T>
class RegisterFile : public ASIM_MODULE_CLASS
{

  public:

    class  FieldIterator
    {
         friend class RegisterFile<T>;
         
     public:

         FieldIterator();
         FieldIterator(const FieldIterator& it);
         FieldIterator(RegisterFile<T> *pt, UINT32 fld, UINT32 start = 0xffffffff, UINT32 end = 0xffffffff);

         // TODO: Delete copy constructor
         FieldIterator&                operator=(const FieldIterator& it);

         void                          operator++();
         void                          operator--();


         UINT64                        operator*();
         UINT64                        current();
         UINT64                        toFirst();
         UINT64                        toLast();
         INT32                         index();
         bool                          more();
         bool                          end();
         void                          latchMe();
         void                          dump() const;

     private:

         FieldIterator(RegisterFile<T> *pt);
         void                          Add(UINT32 idx, UINT64 value);

     private:

         IS             state;
         INT32          curr;
    };



 ///////////////////////////////////////////////////////////////////////////////////////////
 ///////////////////////////////////////////////////////////////////////////////////////////
 ///////////////////////////////////////////////////////////////////////////////////////////
 ////////////
 ////////////
 ////////////  REGISTER_FILE PUBLIC INTERFACE
 ////////////
 ////////////
 ///////////////////////////////////////////////////////////////////////////////////////////
 ///////////////////////////////////////////////////////////////////////////////////////////
 ///////////////////////////////////////////////////////////////////////////////////////////


        //
        // Constructor
        //
        RegisterFile(char *name, UINT32 entries, ASIM_MODULE parent);

        //
        // Destructor
        //
        ~RegisterFile();

        //
        // Populate Table with "fields": each "field" has a bitwidth (max
        // 64 bits), or, ALTERNATIVELY, can hold exactly one object of
        // type T.  A field has and number of RD, WR and CAM ports. A
        // RegisterFile may have a maximum of BACKDOOR_PORT ports of each type. Each type
        // of port  is indepedently numbered, from 0 through BACKDOOR_PORT-1. Thus, for
        // a RegisterFile with 2 read ports and 2 write ports, the valid read
        // ports are 0 and 1 and the valid write ports are also numbered 0
        // and 1. Port BACKDOOR_PORT is special and is reserved for indicating a
        // RD/WR/CAM operation performed WITHOUT using a port (i.e.,
        // cheating :-)). IN other words, an infinite number of RD/WR/CAMs
        // can be performed on the same cycle using port BACKDOOR_PORT.
        //
        UINT32        NewField   (char *name, UINT32 rdp, UINT32 wrp, UINT32 camp, UINT32 bits, UINT64 default_val = 0xdeadbeefdeadbeefULL, MATCH_POLICY p = MATCH_ALL);
        void          NewFieldObj(char *name, UINT32 rdp = 0, UINT32 wrp = 0);

        //
        // Associate "pointers" (like head/tail and read/write pointers to
        // the table. This association will automatically take care of
        // generating DRAL events to manage the pointer contents.
        //
        MPointer      NewPointer(char *name);

        //
        // Clock to re-set the usage for each port
        //
        void          Clock(PHASE p);

        //
        // Field Write: Writes the value "val" (which is limited to 64 bits) into
        // position "idx" in the table using write port "port". If this
        // write port has already been used during this "clock period",
        // the function ASSERTs.
        //
        void          FieldWrite        (UINT32 fld, UINT32 idx, UINT64 val, UINT32 port = BACKDOOR_PORT);

        //
        // Field "MultiWrite": Writes the value "val" (which is limited to 64
        // bits) into MULTIPLE positions specified by the Iterator "it".
        // All writes happen simultaneously using port "port". If this
        // write port has already been used during this "clock period",
        // the function ASSERTs.
        //
        void          FieldWrite        (UINT32 fld, FieldIterator&  it, UINT64 val, UINT32 port = BACKDOOR_PORT);
        
        //
        // Field Read: Reads and returns the bits in position "idx" of the
        // table for field "fld". Uses the read port "port", and the
        // function ASSERTs if this port has already been used during this
        // "clock period".
        //
        UINT64        FieldRead         (UINT32 fld, UINT32 idx, UINT32 port = BACKDOOR_PORT);

        //
        // FieldReadLateral provides a way to emulate "lateral"
        // reads/copies typically found in many hardware structures.
        // Lateral reads are typically assumed to use no read port because
        // they have special wires to compute logic associated with the
        // field. It returns all indices and associated values for a
        // particular field. If no start/end points are provided, indices
        // are ordered starting at 0 and ending at the table capacity. If
        // only a "start" point is provided, then the returned iterator
        // will have as its first element the starting point indicated and
        // will contain all elements of the table field wrapping at the
        // table end. If both "start" and "end" are provided, then the
        // lateral read is limited  to the specified range (again, start
        // and end can wrap around the table capacity to ease modelling
        // circular buffers).
        //
        // IMPORTANT: End is beyond the last valid entry in the Table. In
        // other words, if you are using the RegisterFile as a circular buffer,
        // the assumption is that when the buffer is full then start == end. 
        // This also means that the "full" and "empty" conditions are
        // indistinguishable from the point of view of this class.
        //
        FieldIterator    FieldReadLateral  (UINT32 fld, UINT32 start = 0xffffffff, UINT32 end = 0xffffffff);

        //
        // Field CAM: Scans all entries in field "fld" in the table and
        // compares them (equality comparison) to the supplied value
        // "val".  This function returns a Latched Iterator with a COPY of
        // all indices (and values) that match the "val" value supplied.
        // The user must specify which CAM port is being used for this
        // operation. If the CAM port "port" provided has already been
        // used during this "clock period", then the function will ASSERT.
        // If no start/end points are provided, the indices returned in
        // the iterator are ordered starting at 0 and ending at the table
        // capacity. If only a "start" point is provided, then the
        // returned iterator will have as its first element the starting
        // point indicated and will contain all elements of the table
        // field wrapping at the table end. If both "start" and "end" are
        // provided, then the lateral read is limited  to the specified
        // range (again, start and end can wrap around the table capacity
        // to ease modelling circular buffers).
        //
        // IMPORTANT: End is beyond the last valid entry in the Table. In
        // other words, if you are using the RegisterFile as a circular buffer,
        // the assumption is that when the buffer is full then start == end. 
        // This also means that the "full" and "empty" conditions are
        // indistinguishable from the point of view of this class.
        //
        FieldIterator   FieldCAM          (UINT32 fld, UINT64 val, UINT32 port = BACKDOOR_PORT, UINT32 disablefld = MAX_FIELDS);
        FieldIterator   FieldCAMRange     (UINT32 fld, UINT64 val, UINT32 start, UINT32 end, UINT32 port = BACKDOOR_PORT, UINT32 disablefld = MAX_FIELDS);

        //
        // Accessors and Modifiers for Object Fields. Note the lack of
        // Matching on the object fields.
        //
        void          FieldWriteObj  (UINT32 idx, T obj, UINT32 port = BACKDOOR_PORT);
        T             FieldReadObj   (UINT32 idx,        UINT32 port = BACKDOOR_PORT);

	    //
	    // Maximum number of rows in the table
	    //
	    UINT32        Capacity();

        EVENT(string      GetWatchWindowADF();)
        EVENT(ostringstream adfPointers;);
        EVENT(ostringstream watchwindow_adf_userdefined_fields);

        void AddWatchWindowField(char *tagname); 

        // OPcio 1:
        //void FieldOperate(PTOper op, UINT32  dfld, UINT32  sfld1, UINT32  sfld2);
        //void FieldOperate(PTOper op, UINT32  dfld, UINT32  sfld1, PTIter& sfld2);
        //void FieldOperate(PTOper op, UINT32  dfld, PTIter& sfld1, UINT32  sfld2);
        //void FieldOperate(PTOper op, UINT32  dfld, PTIter& sfld1, PTIter& sfld2);
        //
        //void FieldOperate(PTOper op, PTIter& dfld, UINT32  sfld1, UINT32  sfld2);
        //void FieldOperate(PTOper op, PTIter& dfld, UINT32  sfld1, PTIter& sfld2);
        //void FieldOperate(PTOper op, PTIter& dfld, PTIter& sfld1, UINT32  sfld2);
        //
        //
        // Opcio2
        //
        //void FieldOperate(Iter&          itd, PTOper op, Iter&        it1, Iter&        it2);
        //void FieldOperate(IterLatched&   itd, PTOper op, IterLatched& it1, IterLatched& it2);
        //
        //Iter it;
        //it->Operate(AND, it1, it2);
        //it->Operate(AND, it, it2);
        //
        // it = itvalid & itsrc1rdy;
        //
        //void FieldOperate(UINT32         fld, PTOper op, Iter&        it1, Iter&        it2);
        //void FieldOperate(UINT32         fld, PTOper op, IterLatched& it1, IterLatched& it2);

 private:

        static const UINT32 MAX_FIELDS = 16;

        //
        // Name for this table
        //
        char *name;

        //
        // Array describing all fields in the table
        //
        typedef struct {
          char            *name;
          UINT32          width;
          UINT32          rdports;
          UINT32          wrports;
          UINT32          camports;
          UINT32          rused;
          UINT32          wused;
          UINT32          cused;
          UINT64          mask;
          MATCH_POLICY    policy;
          STAT2(
            Stat_Index *si_num_cam_ports;
            Stat<UINT32> *stat_num_cam;
            Stat<UINT32> *stat_num_cam_backdoor;
            Stat<UINT32> *stat_num_detailed_cam;
            Stat_Index *si_num_read_ports;
            Stat<UINT32> *stat_num_read;
            Stat<UINT32> *stat_num_read_backdoor;
            Stat<UINT32> *stat_num_read_lateral;
            Stat_Index *si_num_write_ports;
            Stat<UINT32> *stat_num_write;
            Stat<UINT32> *stat_num_write_backdoor;
          )  
        } FieldDescriptor;
        FieldDescriptor  fields[MAX_FIELDS];

        //
        //  This structure is a matrix of "MAX_FIELDS x Entries"
        //
        typedef struct {
         UINT64     value;
         T          obj;
         bool       occupied;       ///< Boolean that says, it the field is the 
                                    ///  fieldObj, if the entry is occupied or not
         STAT2( UINT64 last_read_cycle;)
         STAT2( UINT64 last_write_cycle;)
        } FieldData;
        FieldData *data[MAX_FIELDS];

        STAT2(UINT64 current_cycle);
        STAT2(UINT64 total_vulnerable_bits_cycle);
        STAT2(UINT64 array_bits);
        STAT2(Stat<float> *stat_avf);

        //
        // "Object" field. 
        //
        UINT32 objfld;

        //
        // Next Field index
        //
        UINT32 fidx;

        //
        // Table capacity
        //
        UINT32 capacity;

        //
        // Computes the Range to be scanned given "start" and "end"
        //
        UINT32 Range(UINT32& start, UINT32& end);

        // This functions are used to automatically generate events when an ASIM_ITEM
        // moves through a port
        void NotifyExit(UINT32 idx, const ASIM_ITEM& data);
        void NotifyExit(UINT32 idx, const ASIM_ITEM_CLASS& data);
        void NotifyExit(UINT32 idx, const ASIM_SILENT_ITEM& data);
        void NotifyExit(UINT32 idx, const ASIM_SILENT_ITEM_CLASS& data);
        void NotifyExit(UINT32 idx, ...);

        void NotifyEnter(UINT32 idx, const ASIM_ITEM& data);
        void NotifyEnter(UINT32 idx, const ASIM_ITEM_CLASS& data);
        void NotifyEnter(UINT32 idx, const ASIM_SILENT_ITEM& data);
        void NotifyEnter(UINT32 idx, const ASIM_SILENT_ITEM_CLASS& data);
        void NotifyEnter(UINT32 idx, ...);
};

template <class T>
inline
RegisterFile<T>::RegisterFile(char *name, UINT32 entries, ASIM_MODULE parent) : ASIM_MODULE_CLASS(parent, name)
{
 capacity = entries;
 this->name = strdup(name);

 EVENT(SetNodeLayout(capacity, true));

 fidx = 0;
 objfld = (UINT32)-1;
 for ( UINT32 i = 0; i < MAX_FIELDS; i++ ) {
  data[i] = NULL;
  fields[i].name = NULL;
  fields[i].width = 0;
  fields[i].rdports = 0;
  fields[i].wrports = 0;
  fields[i].camports = 0;
  fields[i].rused = 0;
  fields[i].wused = 0;
  fields[i].cused = 0;
  fields[i].mask = 0;
 }
 STAT2(current_cycle = 0;)
 STAT2(total_vulnerable_bits_cycle = 0);
 STAT2(array_bits = 0);
 STAT2(
  ostringstream stat_name;
  stat_name << "avf." << name;
  stat_avf = new Stat<float>(stat_name.str(),"RegisterFile automatic avf");
 ) 
}

template <class T>
inline
RegisterFile<T>::~RegisterFile()
{
 for ( UINT32 i = 0; i < fidx; i++ ) {
  VERIFYX(data[i] != NULL);
  delete []data[i];
 }
 STAT2(if ((current_cycle * array_bits) > 0) (*stat_avf)() = (float)total_vulnerable_bits_cycle / (float)(current_cycle * array_bits);)
}

template <class T>
inline void       
RegisterFile<T>::Clock(PHASE p)
{
 for ( UINT32 i = 0; i < fidx; i++ ) {
  fields[i].rused = 0;
  fields[i].wused = 0;
  fields[i].cused = 0;
 }
 STAT2(current_cycle = p.cycle;)
}

template <class T>
inline UINT32       
RegisterFile<T>::NewField(char *name, UINT32 rdp, UINT32 wrp, UINT32 camp, UINT32 bits, UINT64 default_val, MATCH_POLICY p)
{
 VERIFY(fidx < MAX_FIELDS, "Exceeded number of fields");
 VERIFY(bits >   0, "Number of bits can not be 0 in field");
 VERIFY(bits <= 64, "Exceeded number of bits (64) in field");
 //
 // We limit user ports to a total BACKDOOR_PORT because the last port (port "BACKDOOR_PORT") is
 // always reserved as the "NO-PORT" and used in the masks/stats to count
 // read/wr/cam accesses performed using the API that DOES NOT specify a
 // port.
 //
 VERIFY(rdp  <= BACKDOOR_PORT, "Exceeded number of read ports");
 VERIFY(wrp  <= BACKDOOR_PORT, "Exceeded number of write ports");
 VERIFY(camp <= BACKDOOR_PORT, "Exceeded number of cam ports");
 
 STAT2(
  ostringstream stat_name_read;
  stat_name_read << "read_" << name << "_" << this->name;
  if (rdp > 0) {
    fields[fidx].si_num_read_ports = new Stat_Index(rdp, "port_");
    fields[fidx].stat_num_read = new Stat<UINT32>(stat_name_read.str(),"RegisterFile Automatic read port stat",*(fields[fidx].si_num_read_ports));
  }   
  ostringstream stat_name_lateral;
  stat_name_lateral << stat_name_read.str() << "_lateral";
  fields[fidx].stat_num_read_lateral = new Stat<UINT32>(stat_name_lateral.str(),"RegisterFile Automatic read lateral stat");
  stat_name_read << "_backdoor";
  fields[fidx].stat_num_read_backdoor = new Stat<UINT32>(stat_name_read.str(),"RegisterFile Automatic backdoor read stat");

  ostringstream stat_name_write;
  stat_name_write << "write_" << name << "_" << this->name;
  if (wrp > 0) {
    fields[fidx].si_num_write_ports = new Stat_Index(wrp, "port_");
    fields[fidx].stat_num_write = new Stat<UINT32>(stat_name_write.str(),"RegisterFile Automatic write port stat",*(fields[fidx].si_num_write_ports));
  }   
  stat_name_write << "_backdoor";
  fields[fidx].stat_num_write_backdoor = new Stat<UINT32>(stat_name_write.str(),"RegisterFile Automatic backdoor write stat");

  ostringstream stat_name_cam;
  stat_name_cam << "cam_" << name << "_" << this->name;
  if (camp > 0) {
    fields[fidx].si_num_cam_ports = new Stat_Index(camp, "port_");
    fields[fidx].stat_num_cam = new Stat<UINT32>(stat_name_cam.str(),"RegisterFile Automatic cam port stat",*(fields[fidx].si_num_cam_ports));
  }   
  stat_name_cam << "_backdoor";
  fields[fidx].stat_num_cam_backdoor = new Stat<UINT32>(stat_name_cam.str(),"RegisterFile Automatic cam backdoor stat");
  ostringstream stat_name_detailed_cam;
  stat_name_detailed_cam << "detailed_cam_" << name << "_" << this->name;
  fields[fidx].stat_num_detailed_cam = new Stat<UINT32>(stat_name_detailed_cam.str(),"RegisterFile Automatic detailed cam stat");
 ) 
 STAT2(array_bits += bits);

 //
 // Initialize field descriptor
 //
 fields[fidx].name = strdup(name);
 fields[fidx].width = bits;
 fields[fidx].rdports = rdp;
 fields[fidx].wrports = wrp;
 fields[fidx].camports = camp;
 fields[fidx].rused = 0;
 fields[fidx].wused = 0;
 fields[fidx].cused = 0;
 fields[fidx].mask = (UINT64)((1ULL << bits) - 1);
 if ( bits == 64) fields[fidx].mask = UINT64_MAX;
 fields[fidx].policy = p;

 //
 // Allocate memory for holding this field data
 //
 data[fidx] = new FieldData[capacity];
 for (UINT32 i = 0; i < capacity; i++ )
 {
  data[fidx][i].value = default_val;
  STAT2(data[fidx][i].last_read_cycle = 0;)
  STAT2(data[fidx][i].last_write_cycle = 0;)
 }

 EVENT(stagList.insert(name));
 //
 // Increase field index
 //
 fidx++;

 return (fidx - 1);
}

template <class T>
inline void       
RegisterFile<T>::NewFieldObj(char *name, UINT32 rdp, UINT32 wrp)
{
 VERIFY(fidx < MAX_FIELDS, "Exceeded number of fields");
 VERIFY(objfld == (UINT32)-1, "Sorry, only one OBJECT field supported.");
 //
 // We limit user ports to a total BACKDOOR_PORT because the last port (port "BACKDOOR_PORT") is
 // always reserved as the "NO-PORT" and used in the masks/stats to count
 // read/wr/cam accesses performed using the API that DOES NOT specify a
 // port.
 //
 VERIFY(rdp  <= BACKDOOR_PORT, "Exceeded number of read ports");
 VERIFY(wrp  <= BACKDOOR_PORT, "Exceeded number of write ports");
 
 STAT2(
  ostringstream stat_name_read;
  stat_name_read << "read." << name << "." << this->name;
  if (rdp > 0) {
    fields[fidx].si_num_read_ports = new Stat_Index(rdp, "port_");
    fields[fidx].stat_num_read = new Stat<UINT32>(stat_name_read.str(),"RegisterFile Automatic read port stat",*(fields[fidx].si_num_read_ports));
  }   
  stat_name_read << "_backdoor";
  fields[fidx].stat_num_read_backdoor = new Stat<UINT32>(stat_name_read.str(),"RegisterFile Automatic backdoor read stat");

  ostringstream stat_name_write;
  stat_name_write << "write." << name << "." << this->name;
  if (wrp > 0) {
    fields[fidx].si_num_write_ports = new Stat_Index(wrp, "port_");
    fields[fidx].stat_num_write = new Stat<UINT32>(stat_name_write.str(),"RegisterFile Automatic write port stat",*(fields[fidx].si_num_write_ports));
  }   
  stat_name_write << "_backdoor";
  fields[fidx].stat_num_write_backdoor = new Stat<UINT32>(stat_name_write.str(),"RegisterFile Automatic backdoor write stat");
 ) 


 //
 // Initialized field descriptor
 //
 fields[fidx].name = strdup(name);
 fields[fidx].width = 0;
 fields[fidx].rdports = rdp;
 fields[fidx].wrports = wrp;
 fields[fidx].camports = 0;
 fields[fidx].rused = 0;
 fields[fidx].wused = 0;
 fields[fidx].cused = 0;
 fields[fidx].mask = 0;
 fields[fidx].policy = MATCH_INVALID;

 //
 // Allocate memory for holding this field data
 //
 data[fidx] = new FieldData[capacity];
 for (UINT32 i = 0; i < capacity; i++ )
 {
  data[fidx][i].value = 0;
  data[fidx][i].occupied = false;
  STAT2(data[fidx][i].last_read_cycle = 0;)
  STAT2(data[fidx][i].last_write_cycle = 0;)
 }

 //
 // Remember position of this OBJ field
 //
 objfld = fidx;

 //
 // Increase field index
 //
 fidx++;
}

template <class T>
inline MPointer       
RegisterFile<T>::NewPointer(char *name)
{
 EVENT(adfPointers << "<pointer ntag=\"" << name << "\" name=\"" << name << "\"/>" << endl);
 EVENT(stagList.insert(name));
 MPointer p(capacity, name, GetUniqueId());
 return p;
}

template <class T>
inline void       
RegisterFile<T>::FieldWrite(UINT32 fld, UINT32 idx, UINT64 val, UINT32 port)
{
 ASSERT(fld < fidx, "Invalid field number: " << this->name << "[" << fields[fld].name << "]");
 ASSERT(idx < capacity, "Index out of RegisterFile bounds: " << this->name << "[" << fields[fld].name << "]");
 ASSERT(fields[fld].mask != 0, "Field size improperly defined: " << this->name << "[" << fields[fld].name << "]");
 ASSERT(port == BACKDOOR_PORT || port < fields[fld].wrports, "Invalid port(" << port << "): " << this->name << "[" << fields[fld].name << "]");

 STAT2(
  if (port < BACKDOOR_PORT) (*(fields[fld].stat_num_write))(port)++; 
  else  (*(fields[fld].stat_num_write_backdoor))()++; 
 )   
 STAT2(
  if (port < BACKDOOR_PORT) {
   if (data[fld][idx].last_read_cycle >= data[fld][idx].last_write_cycle) {
    total_vulnerable_bits_cycle += (data[fld][idx].last_read_cycle - data[fld][idx].last_write_cycle + 1);
   }
    data[fld][idx].last_write_cycle = current_cycle;
  }  
 )
 //
 // Compute port mask making sure that port "BACKDOOR_PORT" is not considered "used"
 //
 UINT32 pmask = (1 << port) & 0x7fffffff;

 //
 // Check the port has not been used since last "Clock" invocation
 //
 VERIFY((pmask & fields[fld].wused) == 0, "Write Port Conflict(" << port << "): " << this->name << "[" << fields[fld].name << "]");

 //
 // Mark port as now used
 //
 fields[fld].wused |= pmask;

 //
 // Write data: for sanity, we only write the data up to the bit width
 // specified by the user.
 //
 data[fld][idx].value = val & fields[fld].mask;

 EVENT(SetNodeTag(fields[fld].name, data[fld][idx].value, idx));
}

//
// FieldWrite w/ Iterator
//
// This variant of field writes performs a "multi-write" operation. It
// writes value "val" to every index indicated by the field iterator "it"
// onto field "fld". Optionally, the user can specify a "port" (or use
// port BACKDOOR_PORT to avoid being limited by port constraints)
//
template <class T>
inline void       
RegisterFile<T>::FieldWrite(UINT32 fld, FieldIterator& it, UINT64 val, UINT32 port)
{
 // printf("Begin FieldWrite\n");
 ASSERT(fld < fidx, "Invalid field number: " << this->name << "[" << fields[fld].name << "]");
 // ASSERT(port < fields[fld].wrports, "Invalid port");
 ASSERT(fields[fld].mask != 0, "Field size improperly defined: " << this->name << "[" << fields[fld].name << "]");
 ASSERT(port == BACKDOOR_PORT || port < fields[fld].wrports, "Invalid port(" << port << "): " << this->name << "[" << fields[fld].name << "]");

 STAT2(
  if (port < BACKDOOR_PORT) (*(fields[fld].stat_num_write))(port)++; 
  else  (*(fields[fld].stat_num_write_backdoor))()++; 
 )   
 //
 // Compute port mask making sure that port "BACKDOOR_PORT" is not considered "used"
 //
 UINT32 pmask = (1 << port) & 0x7fffffff;

 //
 // Check the port has not been used since last "Clock" invocation
 //
 VERIFY((pmask & fields[fld].wused) == 0, "Write Port Conflict(" << port << "): " << this->name << "[" << fields[fld].name << "]");

 //
 // Mark port as now used
 //
 fields[fld].wused |= pmask;

 //
 // For sanity, clean up upper data bits
 //
 val &= fields[fld].mask;

 //
 // Write data for all elements in list. We first make a copy of the
 // iterator to preserve the user state.
 //
 FieldIterator myit;
 myit = it;
 for (  ; myit.more() ; ++myit ) {
  UINT32 idx = myit.index();
  data[fld][idx].value = val;  
  EVENT(SetNodeTag(fields[fld].name, val, idx));
  STAT2(
    if (port < BACKDOOR_PORT) {
        if (data[fld][idx].last_read_cycle >= data[fld][idx].last_write_cycle) {
            total_vulnerable_bits_cycle += (data[fld][idx].last_read_cycle - data[fld][idx].last_write_cycle + 1);
        }
        data[fld][idx].last_write_cycle = current_cycle;
    }  
  )
 }
 // printf("End FieldWrite\n");
}

template <class T>
inline UINT64       
RegisterFile<T>::FieldRead(UINT32 fld, UINT32 idx, UINT32 port)
{
 ASSERT(fld < fidx, "Invalid field number: " << this->name << "[" << fields[fld].name << "]");
 ASSERT(idx < capacity, "Index out of RegisterFile bounds: " << this->name << "[" << fields[fld].name << "]");
 ASSERT(fields[fld].mask != 0, "Field size improperly defined: " << this->name << "[" << fields[fld].name << "]");
 ASSERT(port == BACKDOOR_PORT || port < fields[fld].rdports, "Invalid port(" << port << "): " << this->name << "[" << fields[fld].name << "]");

 STAT2(
  if (port < BACKDOOR_PORT) (*(fields[fld].stat_num_read))(port)++; 
  else  (*(fields[fld].stat_num_read_backdoor))()++; 
 )   

 STAT2(if (port < BACKDOOR_PORT) data[fld][idx].last_read_cycle = current_cycle;)
 //
 // Compute port mask making sure that port "BACKDOOR_PORT" is not considered "used"
 //
 UINT32 pmask = (1 << port) & 0x7fffffff;

 //
 // Check the port has not been used since last "Clock" invocation
 //
 VERIFY((pmask & fields[fld].rused) == 0, "Read Port Conflict(" << port << "): " << this->name << "[" << fields[fld].name << "]");

 //
 // Mark port as now used
 //
 fields[fld].rused |= pmask;

 //
 // Return Data. For Sanity we double check that upper bits are all zero.
 //
 ASSERT(((data[fld][idx].value >> fields[fld].width) == 0) || (fields[fld].width == 64), "High bits not zero: " << this->name << "[" << fields[fld].name << "]");
 return data[fld][idx].value;
}

template <class T>
inline typename RegisterFile<T>::FieldIterator
RegisterFile<T>::FieldReadLateral(UINT32 fld, UINT32 start, UINT32 end)
{
 // printf("Begin FieldReadLateral\n");
 ASSERT(fld < fidx, "Invalid field number: " << this->name << "[" << fields[fld].name << "]");
 ASSERT(fields[fld].mask != 0, "Field size improperly defined: " << this->name << "[" << fields[fld].name << "]");

 STAT2(
  (*(fields[fld].stat_num_read_lateral))()++; 
 )   

 STAT2(
  FieldIterator myit(this, fld, start, end);
  for (  ; myit.more() ; ++myit ) {
   UINT32 idx = myit.index();
   data[fld][idx].last_read_cycle = current_cycle;
  }
 )

 // printf("End FieldReadLateral\n");
 return FieldIterator (this, fld, start, end);
}

template <class T>
inline typename RegisterFile<T>::FieldIterator
RegisterFile<T>::FieldCAM(UINT32 fld, UINT64 val, UINT32 port, UINT32 disablefld)
{
 return FieldCAMRange(fld,val,0,0,port,disablefld); 
}

template <class T>
inline typename RegisterFile<T>::FieldIterator
RegisterFile<T>::FieldCAMRange(UINT32 fld, UINT64 val, UINT32 start, UINT32 end, UINT32 port, UINT32 disablefld)
{ 
 ASSERT(fld < fidx, "Invalid field number: " << this->name << "[" << fields[fld].name << "]");
 ASSERT(fields[fld].mask != 0, "Field size improperly defined: " << this->name << "[" << fields[fld].name << "]");
 ASSERT(port == BACKDOOR_PORT || port < fields[fld].camports, "Invalid port(" << port << "): " << this->name << "[" << fields[fld].name << "]");

 STAT2(
  if (port < BACKDOOR_PORT) (*(fields[fld].stat_num_cam))(port)++; 
  else  (*(fields[fld].stat_num_cam_backdoor))()++; 
 )   

 UINT32 trip = 0;
 FieldIterator it(this);

 //
 // Compute port mask making sure that port "BACKDOOR_PORT" is not considered "used"
 //
 UINT32 pmask = (1 << port) & 0x7fffffff;

 //
 // Check the port has not been used since last "Clock" invocation
 //
 VERIFY((pmask & fields[fld].cused) == 0, "CAM Port Conflict(" << port << "): " << this->name << "[" << fields[fld].name << "]");

 //
 // Mark port as now used
 //
 fields[fld].cused |= pmask;

 //
 // Get policy to be usef for matching
 //
 MATCH_POLICY p = fields[fld].policy;

 //
 // Clean up value: make sure upper bits are zero
 //
 val &= fields[fld].mask;

 //
 // Establish range to be Matched against. Note function Range() will
 // modifiy the start/end values to fit them into the valid table capacity
 //
 trip = Range(start, end);
 
 //
 // Compare against all values in Table starting at "start". Return indices of all entries
 // that match the given value.
 //
 if ( p == MATCH_FIRST_ONLY ) { // Stop at first match
  for (UINT32 i = start, j = 0 ; j < trip; j++, i = ((i + 1) % capacity) ) {
   if (disablefld == MAX_FIELDS) {
     STAT2((*(fields[fld].stat_num_detailed_cam))()++);
   } else {
    if ((data[disablefld][i].value == port) || (data[disablefld][i].value == ANY_PORT)) {
     STAT2((*(fields[fld].stat_num_detailed_cam))()++);
    } else {
     continue;
    }
   }
   if ( data[fld][i].value == val ) {
    it.Add(i,val);
    break;
   }
  }
 }
 else if ( p == MATCH_ALL ) {  // Scan full trip count for all possible matches
  for (UINT32 i = start, j = 0 ; j < trip; j++, i = ((i + 1) % capacity) ) {
   if (disablefld == MAX_FIELDS) {
     STAT2((*(fields[fld].stat_num_detailed_cam))()++);
   } else {
    if ((data[disablefld][i].value == port) || (data[disablefld][i].value == ANY_PORT)) {
     STAT2((*(fields[fld].stat_num_detailed_cam))()++);
    } else {
     continue;
    }
   }
   if ( data[fld][i].value == val ) {
    it.Add(i,val);
   }
  }
 }
 else {
  VERIFYX(0);
 }

 return it;
}

template <class T>
inline void       
RegisterFile<T>::FieldWriteObj(UINT32 idx, T obj, UINT32 port)
{
 ASSERT(idx < capacity, "Index out of RegisterFile bounds: " << this->name);
 ASSERT(port == BACKDOOR_PORT || port < fields[objfld].wrports, "Invalid port(" << port << "): " << this->name);

 STAT2(
  if (port < BACKDOOR_PORT) (*(fields[objfld].stat_num_write))(port)++; 
  else  (*(fields[objfld].stat_num_write_backdoor))()++; 
 )   
 //
 // Compute port mask making sure that port "BACKDOOR_PORT" is not considered "used"
 //
 UINT32 pmask = (1 << port) & 0x7fffffff;

 //
 // Check the port has not been used since last "Clock" invocation
 //
 VERIFY((pmask & fields[objfld].wused) == 0, "Write Port Conflict(" << port << ") on Object Field: " << this->name);

 //
 // Mark port as now used
 //
 fields[objfld].wused |= pmask;

 // 
 // Generate the ExitNode Dral event.
 //
 NotifyExit(idx, data[objfld][idx].obj);

 //
 // Write Object
 //
 data[objfld][idx].obj = obj;
 data[objfld][idx].occupied = true;

 // 
 // Generate the EnterNode Dral event.
 //
 NotifyEnter(idx, obj);

}

template <class T>
inline T       
RegisterFile<T>::FieldReadObj(UINT32 idx, UINT32 port)
{
 ASSERT(idx < capacity, "Index out of RegisterFile bounds: " << this->name);
 ASSERT(port == BACKDOOR_PORT || port < fields[objfld].rdports, "Invalid port(" << port << "): " << this->name);

 STAT2(
  if (port < BACKDOOR_PORT) (*(fields[objfld].stat_num_read))(port)++; 
  else  (*(fields[objfld].stat_num_read_backdoor))()++; 
 )   
 //
 // Compute port mask making sure that port "BACKDOOR_PORT" is not considered "used"
 //
 UINT32 pmask = (1 << port) & 0x7fffffff;

 //
 // Check the port has not been used since last "Clock" invocation
 //
 VERIFY((pmask & fields[objfld].rused) == 0, "Read Port Conflict(" << port << "): " << this->name);

 //
 // Mark port as now used
 //
 fields[objfld].rused |= pmask;

 //
 // Return Object. 
 //
 return data[objfld][idx].obj;
}

template <class T>
inline UINT32       
RegisterFile<T>::Capacity()
{
 return capacity;
}

template <class T>
void 
RegisterFile<T>::AddWatchWindowField(char *tagname)
{
    EVENT(watchwindow_adf_userdefined_fields << "  <show itag=\"" << tagname << "\" align=\"left\"/>" << endl);
}

EVENT(
template <class T>
string        
RegisterFile<T>::GetWatchWindowADF()
{
    watchwindow_adf << "<watchwindow node=\"" << name << "\">" << endl;
    watchwindow_adf << "  <rule> if(item_inside()) { color=\"lightblue\"; } </rule>" << endl;
    watchwindow_adf << watchwindow_adf_userdefined_fields.str();
    for(UINT32 i=0; i<fidx; i++) {
        if (i != objfld ) watchwindow_adf << "  <show stag=\"" << fields[i].name << "\" align=\"left\"/>" << endl;
    }
    watchwindow_adf << adfPointers.str();
    watchwindow_adf << "</watchwindow>" << endl;
 return watchwindow_adf.str();
}
)



//
// Establish range to be Read against. If end is -1, then we assume
// we must scan the whole table. Otherwise, we must have both 
// start and end and the trip count should be "end-start". In any case,
// keep an eye on wrapping behavior to get the right "end-start" or
// "start-end" comuptation.
//
template <class T>
inline UINT32
RegisterFile<T>::Range(UINT32& start, UINT32& end)
{
 UINT32 trip = 0; 
 //
 // First case: neither  start nor end are specified. Range is then
 // the full table, with both start/end pointing to element 0
 //
 if ( start == (UINT32)-1 && end == (UINT32)-1 ) {
  start = 0;
  end   = 0;
  return capacity;
 }

 //
 // Second case: we do have "start", but "end" is unspecified. Again, the
 // range is the full table.
 //
 if ( start != (UINT32)-1 && end == (UINT32)-1 ) {
  ASSERT(start < capacity, "Start Index out of RegisterFile bounds");
  end  = start;
  return capacity;
 }

 // 
 // Third case: both start and and are specified. Compute the trip count
 // taking into account the wrapping behavior of the table
 //
 ASSERT(start < capacity, "Start Index out of RegisterFile bounds");
 ASSERT(end   < capacity, "End Index out of RegisterFile bounds");
 trip = (( start <= end ) ? end - start : (capacity - (start - end)) );
 trip = ( trip == 0    ) ? capacity : trip;
 ASSERTX(trip <= capacity);
 return trip;
}

template <class T>
inline void
RegisterFile<T>::NotifyExit(UINT32 idx, const ASIM_ITEM& obj)
{
 if (data[objfld][idx].occupied && obj->GetEventsEnabled())
 {
  // If the entry is occupied, then we perform the exit node of
  // the entry
  EVENT(ExitNode(obj, idx));
 }
}

template <class T>
inline void
RegisterFile<T>::NotifyExit(UINT32 idx, const ASIM_ITEM_CLASS& obj)
{
 if (data[objfld][idx].occupied && obj.GetEventsEnabled())
 {
  EVENT(ExitNode(obj, idx));
 }
}

template <class T>
inline void
RegisterFile<T>::NotifyExit(UINT32 idx, const ASIM_SILENT_ITEM& obj) 
{ 
}

template <class T>
inline void
RegisterFile<T>::NotifyExit(UINT32 idx, const ASIM_SILENT_ITEM_CLASS& obj) 
{ 
}

// Other-wise we set a tag in the node, given that the element is a basic one.
template <class T>
inline void
RegisterFile<T>::NotifyExit(UINT32 idx, ...)
{
 // Note: If you get a compile warning on this line with something like:
 //
 //      warning: cannot pass objects of non-POD type xxxxxx through `...'
 //
 // The class xxxxx MUST inherit from ASIM_ITEM_CLASS if you want to
 // obtain events or inherit from ASIM_SILENT_ITEM_CLASS if you
 // don't want events.
 // This error may also manifest itself by a SEGFLT.
}

template <class T>
inline void
RegisterFile<T>::NotifyEnter(UINT32 idx, const ASIM_ITEM& obj)
{
 if (obj->GetEventsEnabled())
 {
  EVENT(EnterNode(obj, idx));
 }
}

template <class T>
inline void
RegisterFile<T>::NotifyEnter(UINT32 idx, const ASIM_ITEM_CLASS& obj)
{
 if (obj.GetEventsEnabled())
 {
  EVENT(EnterNode(obj, idx));
 }
}

template <class T>
inline void
RegisterFile<T>::NotifyEnter(UINT32 idx, const ASIM_SILENT_ITEM& obj) 
{ 
}

template <class T>
inline void
RegisterFile<T>::NotifyEnter(UINT32 idx, const ASIM_SILENT_ITEM_CLASS& obj) 
{ 
}

// Other-wise we set a tag in the node, given that the element is a basic one.
template <class T>
inline void
RegisterFile<T>::NotifyEnter(UINT32 idx, ...)
{
 // Note: If you get a compile warning on this line with something like:
 //
 //      warning: cannot pass objects of non-POD type xxxxxx through `...'
 //
 // The class xxxxx MUST inherit from ASIM_ITEM_CLASS if you want to
 // obtain events or inherit from ASIM_SILENT_ITEM_CLASS if you
 // don't want events.
 // This error may also manifest itself by a SEGFLT.
  EVENT(SetNodeTag(ITEM_TAG_NAME, data[objfld][idx].value, idx));
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
////////////
////////////
////////////  FIELD ITERATOR 
////////////
////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
template <class T>
inline
RegisterFile<T>::FieldIterator::FieldIterator()
{
 //printf("Standrad constructor: RegisterFile<T>::FieldIterator::FieldIterator(FieldIterator& it): this %p\n",this);
 state = NULL;
 curr  = -1;
}

template <class T>
inline
RegisterFile<T>::FieldIterator::FieldIterator(const FieldIterator& it)
{
 //printf("Copy constructor: RegisterFile<T>::FieldIterator::FieldIterator(FieldIterator& it): this %p it %p\n",this,&it);
 state = it.state;
 curr  = it.curr;
 state->ref++;
}

template <class T>
inline typename RegisterFile<T>::FieldIterator&
RegisterFile<T>::FieldIterator::operator=(const FieldIterator& it)
{
 //printf("Copy operator: RegisterFile<T>::FieldIterator::operator=(FieldIterator& it) this %p it %p\n",this,it);
 it.dump();
 if ( state ) state->ref--;
 state = it.state;
 curr  = it.curr;
 state->ref++;
 //printf("End Copy operator\n");
 return *this;
}

template <class T>
inline
RegisterFile<T>::FieldIterator::FieldIterator(RegisterFile<T> *pt, UINT32 fld, UINT32 start, UINT32 end)
{
 ASSERT(fld < pt->fidx, "Invalid field number");
 ASSERT(pt->fields[fld].mask != 0, "Field size improperly defined");

 //
 // Invoke Range function to "fix" start/end and put them into bounds
 //
 pt->Range(start, end);

 state = new InternalState();

 state->ref      = 1;
 state->pt       = pt;
 state->fld      = fld;
 state->start    = start;
 state->end      = end;
 state->capacity = pt->capacity;
 state->nelem    = 0;
 state->data     = NULL;
 curr            = start;
}

template <class T>
inline
RegisterFile<T>::FieldIterator::FieldIterator(RegisterFile<T> *pt)
{
 //
 // Allocate state for iterator, including an area where to copy the field
 // data (after all, we *are* a LATCHED iterator :-)).
 //
 state = new InternalState();
 state->ref      = 1;
 state->pt       = pt;
 state->capacity = pt->capacity;
 state->nelem    = 0;
 state->data     = new LatchedData [state->capacity];

 //
 // The following fields are undefined,  because the user is expected
 // to "build" this iterator using the "Add" method
 //
 state->fld      = -1;
 state->start    = -1;
 state->end      = -1;
 curr            = -1;
}

template <class T>
inline void
RegisterFile<T>::FieldIterator::Add(UINT32 idx, UINT64 value)
{
 ASSERTX(state != NULL);
 ASSERTX(state->data != NULL);
 ASSERT(state->nelem < state->capacity, "Added too many pairs to FieldIterator");
 state->data[state->nelem].idx = idx;
 state->data[state->nelem].value = value;
 if  ( state->start == -1 ) {
  state->start = 0;
  curr         = 0;
 }
 state->nelem++;
 state->end = state->nelem % state->capacity;
}

template <class T>
inline void
RegisterFile<T>::FieldIterator::latchMe()
{
 //
 // Establish range to be Read against. If end is -1, then we assume
 // we must scan the whole table. Otherwise, we must have both 
 // start and end and the trip count should be "end-start". In any case,
 // keep an eye on wrapping behavior to get the right "end-start" or
 // "start-end" comuptation.
 //
 //start = ( start == -1  ) ? 0 : start;
 //end   = ( end   == -1  ) ? 0 : end;
 //trip  = ( start <= end ) ? end - start : start - end;
 //trip  = ( trip == 0    ) ? pt->capacity : trip;
 //ASSERT(start >= 0 && start < (INT32)pt->capacity, "FieldIterator: Start Index out of RegisterFile bounds");
 //ASSERT(end   >= 0 && end   < (INT32)pt->capacity, "FieldIterator: End Index out of RegisterFile bounds");
 //ASSERTX(trip <= pt->capacity);

 //
 // Copy field from table into our latched data structure
 // For Extra Sanity we double check that upper bits of the bit field are all zero.
 //
 //for (UINT32 i = start, j = 0 ; j < trip; j++, i = ((i + 1) % pt->capacity) ) {
  //ASSERT((pt->data[fld][i].value >> pt->fields[fld].width) == 0, "High bits not zero");
  //state->data[j].idx   = i;
  //state->data[j].value = pt->data[fld][i].value;
 //}

 //curr         = 0;
 //state->start = 0;
 //state->end   = (trip % pt->capacity);
}

template <class T>
inline void
RegisterFile<T>::FieldIterator::operator++()
{
 if ( curr != -1 ) {
  ASSERTX(state     != NULL);
  curr++;
  if ( curr >= (INT32)state->capacity ) curr = 0;
  if ( curr == state->end      ) curr = -1;
 }
}

template <class T>
inline void
RegisterFile<T>::FieldIterator::operator--()
{
 if ( curr != -1 ) {
  ASSERTX(state     != NULL);
  curr--;
  if ( curr < 0             ) curr = state->capacity - 1;
  if ( curr == state->start ) curr = -1;
 }
}


template <class T>
inline UINT64 
RegisterFile<T>::FieldIterator::current()
{
 ASSERT(curr != -1, "FieldIterator::current: out of range");
 ASSERTX(state != NULL);
 ASSERTX(curr < (INT32)state->capacity);
 if ( state->data != NULL ) {
  return state->data[curr].value;
 }
 return ((RegisterFile<T> *)state->pt)->data[state->fld][curr].value;
}

template <class T>
inline UINT64
RegisterFile<T>::FieldIterator::operator*()
{
 return current();
}

template <class T>
inline INT32
RegisterFile<T>::FieldIterator::index()
{
 ASSERTX(state != NULL);
 if ( state->data != NULL ) {
  return state->data[curr].idx;
 }
 return curr;
}

template <class T>
inline bool
RegisterFile<T>::FieldIterator::more()
{
 return ( curr != -1);
}

template <class T>
inline bool
RegisterFile<T>::FieldIterator::end()
{
 return ( curr == -1);
}

template <class T>
inline void
RegisterFile<T>::FieldIterator::dump() const
{
 // printf("FieldIterator::curr     %d\n",curr);
 // printf("FieldIterator::state    %p {\n",&state);
 // if ( state != NULL ) {
  // printf("                  ref      %d\n",state->ref);
  // printf("                  pt       %p\n",state->pt);
  // printf("                  fld      %d\n",state->fld);
  // printf("                  start    %d\n",state->start);
  // printf("                  end      %d\n",state->end);
  // printf("                  capacity %d\n",state->capacity);
  // printf("                  nelem    %d\n",state->nelem);
  // printf("                  data     %p\n",state->data);
 // }
 // printf("               }\n");
}

#endif // _REGISTER_FILE_H

