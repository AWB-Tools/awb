/*****************************************************************************
 *
 * @author Sari Coumeri
 *
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

#ifndef _NULL_POWER_MODEL_
#define _NULL_POWER_MODEL_

// added for power ports
#include "asim/port.h"


#define POWER_MODEL_ENABLED 0
#define THERMAL_MODEL_ENABLED 0
#define DUMMY_THERMAL_MODEL_ENABLED 0

class VF_DOMAIN_CLASS;


enum CACHE_ARRAY_TYPE {
  CACHE_DATA = 0,
  CACHE_TAG,
  CACHE_BOTH
};

class POWER_MODEL_CLASS
{
 public:
  POWER_MODEL_CLASS(){};
  ~POWER_MODEL_CLASS(){};

  void PowerPostProcessing(){};
};


class BASE_POWER_MACRO_CLASS
{
 public:
  //virtual void InitializeMacro() = 0;
  void ComputePower(){};

  // Constructor
  BASE_POWER_MACRO_CLASS(ASIM_MODULE parent,
			 const char * const name,
			 const char * const tname,
			 bool enable) {};
  
  // Destructor
  ~BASE_POWER_MACRO_CLASS(){};
};

class BASE_MEM_POWER_MACRO_CLASS : public BASE_POWER_MACRO_CLASS
{
 public:
  void Read(UINT64 offet = 0){};
  void Write(UINT64 offset = 0){};  

  // Constructor
  BASE_MEM_POWER_MACRO_CLASS(ASIM_MODULE parent,
			     const char * const name,
			     const char * const tname,
			     bool enable) :
    BASE_POWER_MACRO_CLASS(parent, name, tname, enable) {};

  // Destructor
  ~BASE_MEM_POWER_MACRO_CLASS(){};
};

class BASE_CTRL_POWER_MACRO_CLASS : public BASE_POWER_MACRO_CLASS
{
 public:
  void Access(UINT64 offset = 0){};

  // Constructor
  BASE_CTRL_POWER_MACRO_CLASS(ASIM_MODULE parent,
			      const char * const name,
			      const char * const tname,
			      bool enable) :
    BASE_POWER_MACRO_CLASS(parent, name, tname, enable) {};

    // Destructor
  ~BASE_CTRL_POWER_MACRO_CLASS(){};
};

class REG_FILE_POWER_MACRO_CLASS : public BASE_MEM_POWER_MACRO_CLASS
{
 public:
  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       UINT32 bw, 
		       UINT32 entries,
		       UINT32 read_ports, 
		       UINT32 write_ports,
		       bool group = false) {};

  // Constructor
  REG_FILE_POWER_MACRO_CLASS(ASIM_MODULE parent,
			     const char * const name,
			     const char * const tname="",
			     bool enable = true) :
    BASE_MEM_POWER_MACRO_CLASS(parent, name, tname, enable) {};

    // Destructor
  ~REG_FILE_POWER_MACRO_CLASS(){};
};


class ROM_POWER_MACRO_CLASS : public BASE_MEM_POWER_MACRO_CLASS
{
 public:
  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       UINT32 bw, 
		       UINT32 entries,
		       UINT32 read_ports) {};

  // Constructor
  ROM_POWER_MACRO_CLASS(ASIM_MODULE parent,
			const char * const name,
			const char * const tname="",
			bool enable = true) :
    BASE_MEM_POWER_MACRO_CLASS(parent, name, tname, enable) {};

    // Destructor
  ~ROM_POWER_MACRO_CLASS(){};
};


class CACHE_POWER_MACRO_CLASS : public BASE_MEM_POWER_MACRO_CLASS
{
 public:
  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       CACHE_ARRAY_TYPE atype, 
		       UINT32 assoc,
		       UINT32 sets,
		       UINT32 bw,
		       UINT32 read_ports, 
		       UINT32 write_ports,
		       UINT32 extra_bits_per_way = 0,
		       UINT32 extra_bits = 0,
		       bool parallel = false,
		       UINT32 tag_bw = 0,
		       UINT32 tag_read_ports = 0,
		       UINT32 tag_write_ports = 0) {};
  
  void Snoop(UINT32 offset){};
  void Evict(UINT32 offset){};
  void Partial(UINT32 offset){};


  // Constructor
  CACHE_POWER_MACRO_CLASS(ASIM_MODULE parent,
			  const char * const name,
			  const char * const tname="",
			  bool enable = true) :
    BASE_MEM_POWER_MACRO_CLASS(parent, name, tname, enable) {};

  // Destructor
  ~CACHE_POWER_MACRO_CLASS(){};
};


class MUX_POWER_MACRO_CLASS : public BASE_CTRL_POWER_MACRO_CLASS
{
 public:
  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       UINT32 bw, 
		       UINT32 num_inputs, 
		       UINT32 num_outputs = 1) {};

  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       const char * const aname,
		       UINT32 bw, 
		       UINT32 num_inputs, 
		       UINT32 num_outputs = 1) {};

  // Constructor
  MUX_POWER_MACRO_CLASS(ASIM_MODULE parent,
			const char * const name,
			const char * const tname="",
			bool enable = true) :
    BASE_CTRL_POWER_MACRO_CLASS(parent, name, tname, enable) {};

    // Destructor
  ~MUX_POWER_MACRO_CLASS(){};
};


class GENERAL_POWER_MACRO_CLASS : public BASE_CTRL_POWER_MACRO_CLASS
{
 public:
  void InitializeMacro(VF_DOMAIN_CLASS *domain) {};

  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       const char * const aname) {};

  // Constructor
  GENERAL_POWER_MACRO_CLASS(ASIM_MODULE parent,
			    const char * const name,
			    const char * const tname="",
			    bool enable = true) :
    BASE_CTRL_POWER_MACRO_CLASS(parent, name, tname, enable) {};

  // Destructor
  ~GENERAL_POWER_MACRO_CLASS(){};
};



class DECODER_POWER_MACRO_CLASS : public BASE_CTRL_POWER_MACRO_CLASS
{
 public:
  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       UINT32 num_inputs, 
		       UINT32 num_outputs,
		       UINT32 input_bw,
		       UINT32 output_bw = 0) {};

  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       const char * const aname,
		       UINT32 num_inputs, 
		       UINT32 num_outputs,
		       UINT32 input_bw,
		       UINT32 output_bw = 0) {};

  // Constructor
  DECODER_POWER_MACRO_CLASS(ASIM_MODULE parent,
			    const char * const name,
			    const char * const tname="",
			    bool enable = true) :
    BASE_CTRL_POWER_MACRO_CLASS(parent, name, tname, enable) {};

    // Destructor
  ~DECODER_POWER_MACRO_CLASS(){};
};

class BUS_POWER_MACRO_CLASS : public BASE_CTRL_POWER_MACRO_CLASS
{
 public:
  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       UINT32 bw) {};

  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       const char * const aname,
		       UINT32 bw) {};

  // Constructor
  BUS_POWER_MACRO_CLASS(ASIM_MODULE parent,
			const char * const name,
			const char * const tname="",
			bool enable = true) :
    BASE_CTRL_POWER_MACRO_CLASS(parent, name, tname, enable) {};

    // Destructor
  ~BUS_POWER_MACRO_CLASS(){};
};


class BASE_EXE_POWER_MACRO_CLASS : public BASE_POWER_MACRO_CLASS
{

  public:

  void Access(UINT64 offset, UINT32 access_class) {};

  // Constructor
  BASE_EXE_POWER_MACRO_CLASS(ASIM_MODULE parent,
			     const char * const name,
			     const char * const tname="",
			     bool enable = true) :
    BASE_POWER_MACRO_CLASS(parent, name, tname, enable) {};
 
  // Destructor
  virtual ~BASE_EXE_POWER_MACRO_CLASS(){};
};

class CAM_POWER_MACRO_CLASS : public BASE_MEM_POWER_MACRO_CLASS
{
  friend class POWER_CALCULATOR_CLASS;
 private:
  UINT32 bit_width;
  UINT32 num_entries;
  UINT32 num_read_ports;
  UINT32 num_write_ports;
  UINT32 cam0_num_ports;
  UINT32 cam0_bw;
  UINT32 cam1_num_ports;
  UINT32 cam1_bw;
  UINT32 cam2_num_ports;
  UINT32 cam2_bw;

 public:
  void InitializeMacro(VF_DOMAIN_CLASS *domain,
                       UINT32 bw,
                       UINT32 entries,
                       UINT32 read_ports,
                       UINT32 write_ports,
                       UINT32 cam0_ports, UINT32 cam0_bits,
                       UINT32 cam1_ports, UINT32 cam1_bits,
                       UINT32 cam2_ports, UINT32 cam2_bits) {};

  void Cam0(UINT64 offset = 0) {};
  void Cam1(UINT64 offset = 0) {};
  void Cam2(UINT64 offset = 0) {};


  // Constructor
  CAM_POWER_MACRO_CLASS(ASIM_MODULE parent,
			const char * const name,
			const char * const tname="",
			bool enable = true) :
    BASE_MEM_POWER_MACRO_CLASS(parent, name, tname, enable) {};

    // Destructor
  ~CAM_POWER_MACRO_CLASS(){};
};


class EXE_POWER_MACRO_CLASS : public BASE_EXE_POWER_MACRO_CLASS
{

 public:
   void InitializeMacro(VF_DOMAIN_CLASS *domain,
                        UINT32 input_bw,
                        UINT32 output_bw) {};

 // Constructor
 EXE_POWER_MACRO_CLASS(ASIM_MODULE parent,
                       const char * const name,
		       const char * const tname="",
		       bool enable = true) :
   BASE_EXE_POWER_MACRO_CLASS(parent, name, tname, enable) {};
  
 // Desstructor
 ~EXE_POWER_MACRO_CLASS() {};
 
};

class NO_ACTIVITY_POWER_MACRO_CLASS : public BASE_POWER_MACRO_CLASS
{
 public:
  void InitializeMacro(VF_DOMAIN_CLASS *domain) {};

  // Constructor
  NO_ACTIVITY_POWER_MACRO_CLASS(ASIM_MODULE parent,
				const char * const name,
				const char * const tname="",
				bool enable = true) :
    BASE_POWER_MACRO_CLASS(parent, name, tname, enable) {};

    // Destructor
  ~NO_ACTIVITY_POWER_MACRO_CLASS(){};
};



enum CPU_STATE_TYPE { 
  ACTIVE = 0, //normal operation  
  UNCLOCKED, //clock to cpu gated -- only leakage pwr -- no idle
  SLEEP //no pwr (sleep transistor) -- no leakage pwr or idle pwr
};


class VF_DOMAIN_CLASS {
public:
  // Constructor
  VF_DOMAIN_CLASS(ASIM_MODULE parent,
		  const char * const name,
		  ASIM_CLOCK_SERVER clock_server,
		  double volt = 0.0,
		  CPU_STATE_TYPE state = ACTIVE) {};
  // Destructor
  ~VF_DOMAIN_CLASS(){};
  
  void SetVoltage(double volt) {};
  void SetState(CPU_STATE_TYPE state) {};
  void SetClockRegistry(CLOCK_REGISTRY registry) {};
  void RegisterVFClock(string domainName, UINT32 skew) {};
  
  void Clock(UINT64 cycle) {};
};

// empty thermal class for null_thermal_model. It's here because we have a use of 
// the thermal model in single_chip_clockserver_system.h
class THERMAL_MODEL_CLASS
{
    
 protected:
  THERMAL_MODEL_CLASS(){};   
  ~THERMAL_MODEL_CLASS(){};

 public:
  static THERMAL_MODEL_CLASS * Instance() {return new THERMAL_MODEL_CLASS();};
  void UpdateTemperature(UINT64 system_clock) {};
};

// Power Write port inherited from Write Port in port.h
// The only real difference is the fact that there is pointer to the
// appropriate power macro class
// Note that the power macro class is assumed to be derived from the
// BASE_CTRL_POWER_MACRO_CLASS so the only method is Access()
// Note also that the Write function had to be virtualized in the WritePort
// class in port.h. The array Buffer is also changed to protected

template<class T, class POWER_MACRO_CLASS, int F = 1>
  class PowerWritePort : public WritePort <T, F>
{
  private:
  /* pointer to power macro class */
  POWER_MACRO_CLASS *powerMacro;

  public:

  /* default constructor -- calls default constructor from write port as well*/
  PowerWritePort(){}

  /* constructor -- calls default constructor from write port as well */
  PowerWritePort(const char * const name, const char * const tname = "") { }

  /* Make the power macro point to a specific object of type POWER_MACRO_CLASS */
  void AssignMacro(POWER_MACRO_CLASS *objectPointer){ }

  /* Initialize power macro calls for GENERAL_POWER_MACRO_CLASS */
  void InitializeMacro(VF_DOMAIN_CLASS *domain){ }
  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       const char * const aname){} //used to name activity something other than "Access"
 

  /* Initialize power macro calls for MUX_POWER_MACRO_CLASS */
  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       UINT32 bw,
		       UINT32 num_inputs,
		       UINT32 num_outputs = 1){ }

  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       const char * const aname, //used to name activity something other than "Access"
		       UINT32 bw,
		       UINT32 num_inputs,
		       UINT32 num_outputs = 1){ }

  /* Initialize power macro calls for DECODER_POWER_MACRO_CLASS */
  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       UINT32 num_inputs,
		       UINT32 num_outputs,
		       UINT32 input_bw,
		       UINT32 output_bw = 0){ }

  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       const char * const aname, //used to name activity something other than "Access"
		       UINT32 num_inputs,
		       UINT32 num_outputs,
		       UINT32 input_bw,
		       UINT32 output_bw = 0){ }

  /* Initialize power macro calls for BUS_POWER_MACRO_CLASS */
  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       UINT32 bw){ }

  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       const char * const aname, //used to name activity something other than "Access"
		       UINT32 bw){ }

  /* Write method is the same as in port.h */
  bool Write(T data, UINT64 cycle);

  /* Desctrutor */
  ~PowerWritePort(){
    delete powerMacro;
  }
};

// Power Write Skid port inherited from WriteSkidPort in port.h
// Same concept as above. Write function is virtualized in port.h
// and Buffer is changed to protected from private


template<class T, class POWER_MACRO_CLASS, int S = 0>
  class PowerWriteSkidPort : public WriteSkidPort <T, S>
{
  private:
  /* power macro */
  POWER_MACRO_CLASS *powerMacro;

  public:

  /* default constructor -- calls default constructor from writeskid port as well*/
  PowerWriteSkidPort(){}

  /* constructor -- calls default constructor of WriteSkidPort*/
  PowerWriteSkidPort(const char * const name, const char * const tname = ""){ }

  /* Make the power macro point to a specific object of type POWER_MACRO_CLASS */
  void AssignMacro(POWER_MACRO_CLASS *objectPointer){ }

  /* Initialize power macro calls for GENERAL_POWER_MACRO_CLASS */
  void InitializeMacro(VF_DOMAIN_CLASS *domain){ }

  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       const char * const aname){ }//used to name activity something other than "Access"

  /* Initialize power macro calls for MUX_POWER_MACRO_CLASS */
  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       UINT32 bw,
		       UINT32 num_inputs,
		       UINT32 num_outputs = 1){ }

  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       const char * const aname, //used to name activity something other than "Access"
		       UINT32 bw,
		       UINT32 num_inputs,
		       UINT32 num_outputs = 1){ }

  /* Initialize power macro calls for DECODER_POWER_MACRO_CLASS */
  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       UINT32 num_inputs,
		       UINT32 num_outputs,
		       UINT32 input_bw,
		       UINT32 output_bw = 0){ }

  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       const char * const aname, //used to name activity something other than "Access"
		       UINT32 num_inputs,
		       UINT32 num_outputs,
		       UINT32 input_bw,
		       UINT32 output_bw = 0){ }

  /* Initialize power macro calls for BUS_POWER_MACRO_CLASS */
  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       UINT32 bw){ }

  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       const char * const aname, //used to name activity something other than "Access"
		       UINT32 bw){ }

  ~PowerWriteSkidPort(){
    delete powerMacro;
  }
  
  /* Write method is the same as port.h */
  bool Write(T data, UINT64 cycle);
};

// Power Write Stall port inherited from PowerWriteStallPort in port.h
// Buffer is again changed to protected from private and Write is virtualized
template<class T, class POWER_MACRO_CLASS>
class PowerWriteStallPort : public WriteStallPort <T>
{

 private:
  /* power macro */
  POWER_MACRO_CLASS *powerMacro;

 public:

  /* default constructor -- calls default powerwritestall port*/
  PowerWriteStallPort(){}

  /* constructor -- calls default constructor from WriteStallPort */
  PowerWriteStallPort(const char * const name, const char * const tname = ""){ }

  /* Make the power macro point to a specific object of type POWER_MACRO_CLASS */
  void AssignMacro(POWER_MACRO_CLASS *objectPointer){ }

  /* Initialize power macro calls for GENERAL_POWER_MACRO_CLASS */
  void InitializeMacro(VF_DOMAIN_CLASS *domain){ }

  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       const char * const aname){ }//used to name activity something other than "Access"

  /* Initialize power macro calls for MUX_POWER_MACRO_CLASS */
  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       UINT32 bw,
		       UINT32 num_inputs,
		       UINT32 num_outputs = 1){  }

  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       const char * const aname, //used to name activity something other than "Access"
		       UINT32 bw,
		       UINT32 num_inputs,
		       UINT32 num_outputs = 1){ }

  /* Initialize power macro calls for DECODER_POWER_MACRO_CLASS */
  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       UINT32 num_inputs,
		       UINT32 num_outputs,
		       UINT32 input_bw,
		       UINT32 output_bw = 0){ }

  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       const char * const aname, //used to name activity something other than "Access"
		       UINT32 num_inputs,
		       UINT32 num_outputs,
		       UINT32 input_bw,
		       UINT32 output_bw = 0){ }

  /* Initialize power macro calls for BUS_POWER_MACRO_CLASS */
  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       UINT32 bw){}

  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       const char * const aname, //used to name activity something other than "Access"
		       UINT32 bw){}

  /* destructor */
  ~PowerWriteStallPort(){
    delete powerMacro;
  }

  /* Write method is the same as port.h */
  bool Write(T data, UINT64 cycle);
};


// Power Read port inherited from Read Port in port.h
// The only real difference is the fact that there is pointer to the
// appropriate power macro class
// Note that the power macro class is assumed to be derived from the
// BASE_CTRL_POWER_MACRO_CLASS so the only method is Access()
template<class T, class POWER_MACRO_CLASS>
  class PowerReadPort : public ReadPort <T>
{
 private:
  /* pointer to the power macro class */
  POWER_MACRO_CLASS *powerMacro;

 public:

  /* default constructor */
  PowerReadPort(){}

  /* constructor */
  PowerReadPort(const char * const name, const char * const tname = "") {  }

 /* Make the power macro point to a specific object of type POWER_MACRO_CLASS */
  void AssignMacro(POWER_MACRO_CLASS *objectPointer){  }

  /* Initialize power macro calls for GENERAL_POWER_MACRO_CLASS */
  void InitializeMacro(VF_DOMAIN_CLASS *domain){ }

  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       const char * const aname){ } //used to name activity something other than "Access"
   

  /* Initialize power macro calls for MUX_POWER_MACRO_CLASS */
  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       UINT32 bw,
		       UINT32 num_inputs,
		       UINT32 num_outputs = 1){  }

  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       const char * const aname, //used to name activity something other than "Access"
		       UINT32 bw,
		       UINT32 num_inputs,
		       UINT32 num_outputs = 1){ }

 /* Initialize power macro calls for DECODER_POWER_MACRO_CLASS */
  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       UINT32 num_inputs,
		       UINT32 num_outputs,
		       UINT32 input_bw,
		       UINT32 output_bw = 0){ }

  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       const char * const aname, //used to name activity something other than "Access"
		       UINT32 num_inputs,
		       UINT32 num_outputs,
		       UINT32 input_bw,
		       UINT32 output_bw = 0){  }

  /* Initialize power macro calls for BUS_POWER_MACRO_CLASS */
  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       UINT32 bw){  }

  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       const char * const aname, //used to name activity something other than "Access"
		       UINT32 bw){  }

  /* Read method which is the same as in port.h */
  bool Read(T& data, UINT64 cycle);
 /* destructor */
  ~PowerReadPort(){
    delete powerMacro;
  }
};

// PowerReadSkidPort is inherited from ReadSkiPort in port.h
// Buffer is changed to protected and Read() is virtualized in port.h
template<class T, class POWER_MACRO_CLASS, int S = 0>
  class PowerReadSkidPort : public ReadSkidPort <T, S>
{
  private:
 /* power macro */
 POWER_MACRO_CLASS *powerMacro;

  public:

 /* default constructor */
 PowerReadSkidPort(){}

 /* constructor */
 PowerReadSkidPort(const char * const name, const char * const tname = ""){ }

  /* Make the power macro point to a specific object of type POWER_MACRO_CLASS */
  void AssignMacro(POWER_MACRO_CLASS *objectPointer){  }

  /* Initialize power macro calls for GENERAL_POWER_MACRO_CLASS */
 void InitializeMacro(VF_DOMAIN_CLASS *domain){ }

  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       const char * const aname){ }//used to name activity something other than "Access"
   
  

  /* Initialize power macro calls for MUX_POWER_MACRO_CLASS */
  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       UINT32 bw,
		       UINT32 num_inputs,
		       UINT32 num_outputs = 1){  }

  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       const char * const aname, //used to name activity something other than "Access"
		       UINT32 bw,
		       UINT32 num_inputs,
		       UINT32 num_outputs = 1){ }

  /* Initialize power macro calls for DECODER_POWER_MACRO_CLASS */
  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       UINT32 num_inputs,
		       UINT32 num_outputs,
		       UINT32 input_bw,
		       UINT32 output_bw = 0){  }

  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       const char * const aname, //used to name activity something other than "Access"
		       UINT32 num_inputs,
		       UINT32 num_outputs,
		       UINT32 input_bw,
		       UINT32 output_bw = 0){  }

  /* Initialize power macro calls for BUS_POWER_MACRO_CLASS */
  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       UINT32 bw){  }

  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       const char * const aname, //used to name activity something other than "Access"
		       UINT32 bw){  }

 /* destructor */
 ~PowerReadSkidPort(){
   delete powerMacro;
 }

  /* Read method which is the same as in port.h */
  bool Read(T& data, UINT64 cycle);
};
// PowerReadStallPort is inherited from ReadStallPort in port.h
template<class T, class POWER_MACRO_CLASS>
class PowerReadStallPort : public ReadStallPort <T>
{

 private:
  /* power macro */
  POWER_MACRO_CLASS *powerMacro;

 public:

  /* default constructor */
  PowerReadStallPort(){}

  /* constructor */
  PowerReadStallPort(const char * const name, const char * const tname = ""){  }

  /* Make the power macro point to a specific object of type POWER_MACRO_CLASS */
  void AssignMacro(POWER_MACRO_CLASS *objectPointer){  }

  /* Initialize power macro calls for GENERAL_POWER_MACRO_CLASS */
  void InitializeMacro(VF_DOMAIN_CLASS *domain){  }

  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       const char * const aname){ }//used to name activity something other than "Access"

 /* Initialize power macro calls for MUX_POWER_MACRO_CLASS */
  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       UINT32 bw,
		       UINT32 num_inputs,
		       UINT32 num_outputs = 1){  }

  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       const char * const aname, //used to name activity something other than "Access"
		       UINT32 bw,
		       UINT32 num_inputs,
		       UINT32 num_outputs = 1){  }

  /* Initialize power macro calls for DECODER_POWER_MACRO_CLASS */
  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       UINT32 num_inputs,
		       UINT32 num_outputs,
		       UINT32 input_bw,
		       UINT32 output_bw = 0){  }

  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       const char * const aname, //used to name activity something other than "Access"
		       UINT32 num_inputs,
		       UINT32 num_outputs,
		       UINT32 input_bw,
		       UINT32 output_bw = 0){  }

  /* Initialize power macro calls for BUS_POWER_MACRO_CLASS */
  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       UINT32 bw){  }

  void InitializeMacro(VF_DOMAIN_CLASS *domain,
		       const char * const aname, //used to name activity something other than "Access"
		       UINT32 bw){  }

  /* destructor */
  ~PowerReadStallPort(){
    delete powerMacro;
  }

  /* Read function from port.h */
  bool Read(T& data, UINT64 cycle);
};

// Write method for PowerWritePort
// Assumes power macro class is derived from ctrl base class

template <class T, class POWER_MACRO_CLASS, int F>
inline bool
PowerWritePort<T,POWER_MACRO_CLASS,F>::Write(T data, UINT64 cycle)
{ 
  VERIFYX(this->IsConnected()); 
  for (int f = 0; f < this->Fanout; f++) {
    this->Buffer[f]->Write(data, cycle, this->GetName()); 
  }  

  // assertions above and in Write method will never allow them to
  // return false, so...return true, or else make this a void function
  return true;
}

// PowerReadPort Read method which is based on the virtual Read method 
// in port.h

template <class T, class POWER_MACRO_CLASS>
inline bool
PowerReadPort<T, POWER_MACRO_CLASS>::Read(T& data, UINT64 cycle)
{ 
  return (this->Buffer.Read(data, cycle, this->GetName())); 
}


// PowerReadSkidPort Read method which is based on the virtual Read method 
// in port.h

template <class T, class POWER_MACRO_CLASS, int S>
inline bool
PowerReadSkidPort<T,POWER_MACRO_CLASS, S>::Read(T& data, UINT64 cycle)
{ 
  return (this->Buffer.Read(data, cycle, this->GetName(), true)); 
}

// PowerReadStallPort Read method which is based on the virtual Read method
// in port.h

template <class T, class POWER_MACRO_CLASS>
inline bool
PowerReadStallPort<T, POWER_MACRO_CLASS>::Read(T& data, UINT64 cycle)
{
     //  we need to relax the asserts here since this port
    //  does not need to be read every cycle
  return (this->Buffer.Read(data, cycle, this->GetName(), true)); 
}


// PowerWriteSkidPort Write method which is based on the virtual Write method
// in port.h

template <class T, class POWER_MACRO_CLASS, int S>
inline bool
PowerWriteSkidPort<T,POWER_MACRO_CLASS, S>::Write(T data, UINT64 cycle)
{
  return (this->IsConnected()) && (this->Buffer->Write(data, cycle, this->GetName())); 
}

// PowerWriteStallPort Write method which is based on the virtual Write method
// in port.h

template <class T, class POWER_MACRO_CLASS>
inline bool
PowerWriteStallPort<T, POWER_MACRO_CLASS>::Write(T data, UINT64 cycle)
{
  return ((this->IsConnected()) && (this->Buffer->Write(data, cycle, this->GetName()))); }

#endif
