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
};


class BASE_POWER_MACRO_CLASS : public ASIM_MODULE_CLASS
{
 public:
  //virtual void InitializeMacro() = 0;
  void ComputePower(){};

  // Constructor
  BASE_POWER_MACRO_CLASS(ASIM_MODULE parent,
			 const char * const name)
    : ASIM_MODULE_CLASS(parent, name) {};
  
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
			     const char * const name) :
    BASE_POWER_MACRO_CLASS(parent, name) {};

  // Destructor
  ~BASE_MEM_POWER_MACRO_CLASS(){};
};

class BASE_CTRL_POWER_MACRO_CLASS : public BASE_POWER_MACRO_CLASS
{
 public:
  void Access(UINT64 offset = 0){};

  // Constructor
  BASE_CTRL_POWER_MACRO_CLASS(ASIM_MODULE parent,
			     const char * const name) :
    BASE_POWER_MACRO_CLASS(parent, name) {};

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
			     const char * const name) :
    BASE_MEM_POWER_MACRO_CLASS(parent, name) {};

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
			     const char * const name) :
    BASE_MEM_POWER_MACRO_CLASS(parent, name) {};

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
			       const char * const name) :
    BASE_MEM_POWER_MACRO_CLASS(parent, name) {};

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
			const char * const name) :
    BASE_CTRL_POWER_MACRO_CLASS(parent, name) {};

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
			    const char * const name) :
      BASE_CTRL_POWER_MACRO_CLASS(parent, name) {};

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
			const char * const name) :
    BASE_CTRL_POWER_MACRO_CLASS(parent, name) {};

    // Destructor
  ~DECODER_POWER_MACRO_CLASS(){};
};


class BASE_EXE_POWER_MACRO_CLASS : public BASE_POWER_MACRO_CLASS
{

  public:

  void Access(UINT64 offset, UINT32 access_class) {};

  // Constructor
  BASE_EXE_POWER_MACRO_CLASS(ASIM_MODULE parent,
                              const char * const name) :
    BASE_POWER_MACRO_CLASS(parent, name) {};
 
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
                             const char * const name) :
    BASE_MEM_POWER_MACRO_CLASS(parent, name) {};

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
                       const char * const name) :
   BASE_EXE_POWER_MACRO_CLASS(parent, name) {};
  
 // Desstructor
 ~EXE_POWER_MACRO_CLASS() {};
 
};

class NO_ACTIVITY_POWER_MACRO_CLASS : public BASE_POWER_MACRO_CLASS
{
 public:
  void InitializeMacro(VF_DOMAIN_CLASS *domain) {};

  // Constructor
  NO_ACTIVITY_POWER_MACRO_CLASS(ASIM_MODULE parent,
				const char * const name) :
    BASE_POWER_MACRO_CLASS(parent, name) {};

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
#endif
