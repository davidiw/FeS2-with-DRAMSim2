/* This file is part of the Ruby Multiprocessor Memory System
   Simulator, a component of the Multifacet GEMS (General
   Execution-driven Multiprocessor Simulator) software toolset
   originally developed at the University of Wisconsin-Madison.

   Ruby was originally developed primarily by Milo Martin and Daniel
   Sorin with contributions from Ross Dickson, Carl Mauer, and Manoj
   Plakal.

   Substantial further development of Multifacet GEMS at the
   University of Wisconsin was performed by Alaa Alameldeen, Brad
   Beckmann, Ross Dickson, Pacia Harper, Milo Martin, Michael Marty,
   Carl Mauer, Kevin Moore, Manoj Plakal, Daniel Sorin, Min Xu, and
   Luke Yen.

   Additional development was performed at the University of
   Pennsylvania by Milo Martin.

   --------------------------------------------------------------------

   Copyright (C) 1999-2005 by Mark D. Hill and David A. Wood for the
   Wisconsin Multifacet Project.  Contact: gems@cs.wisc.edu
   http://www.cs.wisc.edu/gems/

   This file is based upon a pre-release version of Multifacet GEMS
   from 2004 and may also contain additional modifications and code
   Copyright (C) 2004-2007 by Milo Martin for the Penn Architecture
   and Compilers Group.  Contact: acg@lists.seas.upenn.edu
   http://www.cis.upenn.edu/acg/

   --------------------------------------------------------------------

   Multifacet GEMS is free software; you can redistribute it and/or
   modify it under the terms of version 2 of the GNU General Public
   License as published by the Free Software Foundation.  This 
   software comes with ABSOLUTELY NO WARRANTY.  For details see the 
   file LICENSE included with the distribution.
*/
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

#include "Global.h"
#include "EventQueue.h"
#include "System.h"
#include "Debug.h"
// #include "Debug.h"
#include "Profiler.h"
#include "Tester.h"
// #include "OpalInterface.h"
#include "init.h"
// #include "interface.h"
#include "Param.h"

//***************************************************************************
void init_variables()
{
  if (!g_param_ptr) {
    cout << "Creating parameter object..." << endl;
    g_param_ptr = new Param();
    cout << "Creating parameter object...done" << endl;
  }
}

void init_simulator()
{
  // Set things to NULL to make sure we don't de-reference them
  // without a seg. fault.
  g_system_ptr = NULL;
  // g_debug_ptr = NULL;
  g_eventQueue_ptr = NULL;
  
  cout << "Ruby Timing Mode" << endl;
#ifndef MULTIFACET_NO_OPT_WARN
  cerr << "Warning: optimizations not enabled." << endl;
#endif

  init_variables();

  //  if (g_param_ptr->SIMICS()) {
  //    g_param_ptr->set_NUM_NODES(SIMICS_number_processors());
  //  }

  if (g_param_ptr->NUM_NODES() == 0) {
    ERROR_MSG("The NUM_NODES parameter must be non-zero");
  }

  if (!is_power_of_2(g_param_ptr->NUM_NODES())) {
    ERROR_MSG("The NUM_NODES parameter must be a power of 2");
  }

  // Fill in NUM_NODES_BITS
  if (g_param_ptr->NUM_NODES_BITS() != 0) {
    ERROR_MSG("The NUM_NODES_BITS parameter must default to zero.  The correct value will be substituted at runtime");
  }
  g_param_ptr->set_NUM_NODES_BITS(log_int(g_param_ptr->NUM_NODES()));

  // Freeze the parameters: disallow any future changes to the parameters
  g_param_ptr->freeze();

  // Randomize the execution
  if (g_param_ptr->RANDOM_SEED() == "random") {
    srandom(time(NULL) * getpid());
  } else {
    srandom(string_to_int(g_param_ptr->RANDOM_SEED()));
  }

  cout << "Setting up debugging..." << endl;
  // g_debug_ptr = new Debug();
  cout << "Setting up debugging...done" << endl;

  cout << "Creating event queue..." << endl;
  g_eventQueue_ptr = new EventQueue;
  cout << "Creating event queue...done" << endl;

  cout << "Creating system..." << endl;
  cout << "  Processors: " << g_param_ptr->NUM_NODES() << endl;

  g_system_ptr = new System;
  cout << "Creating system...done" << endl;

  // FIXME: Replay the trace here, if filename is specific
}

// void init_opal_interface(mf_ruby_api_t *api)
// {
//   //  OpalInterface::installInterface(api);
// }

void destroy_simulator()
{
  cout << "Deleting system..." << endl;
  delete g_system_ptr;
  cout << "Deleting system...done" << endl;

  cout << "Deleting event queue..." << endl;
  delete g_eventQueue_ptr;
  cout << "Deleting event queue...done" << endl;

  // delete g_debug_ptr;
  delete g_param_ptr;
}
