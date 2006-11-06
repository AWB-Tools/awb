/*
 *Copyright (C) 2000-2006 Intel Corporation
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

/**
 * @file
 * @author Artur Klauser
 * @brief Register simulator configuration as stats so the configuration
 * will end up being put into the stats output file.
 */

#ifndef _CONFIG_
#define _CONFIG_

// generic C++
#include <string>

// ASIM core
#include "asim/syntax.h"
#include "asim/registry.h"
#include "asim/module.h"


// setup extern declarations of dynamic parameters
#define Register(NAME,DESC,TYPE,VAR,VAL)
#define RegisterDyn(NAME,DESC,TYPE,VAR)
#define Declare(DECL) \
  DECL;
//
// ASIM public modules - This is OK here, since AWB guarantees that
// this header file is synthesized for all configurations.
#include "asim/provides/sim_config.h"
#undef Register
#undef RegisterDyn
#undef Declare

/*
 * Class ASIM_CONFIG
 *
 * ASIM simulator configuration module
 *
 */
typedef class ASIM_CONFIG_CLASS *ASIM_CONFIG;
class ASIM_CONFIG_CLASS : public ASIM_MODULE_CLASS
{
    private:
#define Register(NAME,DESC,TYPE,VAR,VAL) \
      TYPE VAR;
#define RegisterDyn(NAME,DESC,TYPE,VAR) \
      TYPE VAR;
#define Declare(DECL)
//
// ASIM public modules - This is OK here, since AWB guarantees that
// this header file is synthesized for all configurations.
#include "asim/provides/sim_config.h"
#undef Register
#undef RegisterDyn
#undef Declare

    public:
        ASIM_CONFIG_CLASS (ASIM_MODULE p);
	void RegisterSimulatorConfiguration(void);

};

#endif
