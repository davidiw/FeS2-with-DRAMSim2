// -----------------------------------------------------------------------------
//
//  This file is part of FeS2.
//
//  FeS2 is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  FeS2 is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with FeS2.  If not, see <http://www.gnu.org/licenses/>.
//
// -----------------------------------------------------------------------------

#ifndef PYRITE_H
#define PYRITE_H

#include "globals.h"

#define CHECK_SIM_EXCEPTION()  if (SIM_clear_exception() != SimExc_No_Exception){ ERROR_MSG(SIM_last_error()); }

/* struct for the pyrite class */
typedef struct {
        conf_object_t obj;
        char *trace_file_name;
        FILE *trace_file;
        W64 count;/* counts number of mem. accesses made while tracing */
	int stall_time;
	conf_object_t *next_timing_model;/*tracer to fwd ops to*/
	timing_model_interface_t next_timing_iface;/*interface for above*/
} pyrite_object_t;

void step_cycle(void);
void print(void);
void myMagicInsnHandler(void* userDate, conf_object_t* cpu, integer_t
                        dontCare);

extern bool g_break_simulation;
// extern SimicsDriver g_simics_driver;
#endif // PYRITE_H
