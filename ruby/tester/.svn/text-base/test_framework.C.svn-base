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

#include "test_framework.h"
#include "protocol_name.h"
#include "System.h"
#include "init.h"
#include "Tester.h"
#include "EventQueue.h"
#include "CacheRecorder.h"
#include "Tracer.h"
#include "Param.h"

static void tester_record_cache();
static void tester_playback_trace();
static void tester_destroy();

static string trace_filename;

void tester_main(int argc, char **argv)
{
  if (!g_param_ptr) {
    cout << "Creating parameter object..." << endl;
    g_param_ptr = new Param();
    cout << "Creating parameter object done" << endl;
  }

  // Tester specific parameters
  g_param_ptr->set_SIMICS(false);
  g_param_ptr->set_RANDOMIZATION(true);
  g_param_ptr->set_DATA_BLOCK(true);
  g_param_ptr->set_DEADLOCK_THRESHOLD(200000);
  g_param_ptr->set_DESTSET_PREDICTOR(DestPredType_Random);
  g_param_ptr->set_L1_CACHE_ASSOC(2);
  g_param_ptr->set_L1_CACHE_NUM_SETS_BITS(2);
  g_param_ptr->set_L2_CACHE_ASSOC(2);
  g_param_ptr->set_L2_CACHE_NUM_SETS_BITS(3);
  g_param_ptr->set_MEMORY_SIZE_BITS(20);

  if (argc <= 1) {
    cout << endl;
    cout << "Usage: tester.exec PARAM1=value1 PARAM2=value2..." << endl;
    cout << "  The NUM_NODES and TESTER_LENGTH parameters must be set." << endl;
    cout << endl;
    g_param_ptr->printOptions(cout);
    exit(1);
  }

  cout << "Parsing command line arguments:" << endl;

  for(int i=1; i<argc; i++) {
    string current_arg = string(argv[i]);
    string param = string_split(current_arg, '=');
    g_param_ptr->setParam(param, current_arg);        
  }

  init_simulator();

  g_system_ptr->printConfig(cout);
  cout << "Testing clear stats...";
  g_system_ptr->clearStats();
  cout << "Done." << endl;

  if (trace_filename != "") {
    // playback a trace (for multicast-mask prediction)
    tester_playback_trace();
  } else {
    // test code to create a trace
    if (!(g_param_ptr->SYNTHETIC_DRIVER()) && trace_filename == "") {
      g_system_ptr->getTracer()->startTrace("ruby.trace.gz");
      g_eventQueue_ptr->triggerEvents(g_eventQueue_ptr->getTime() + 10000);
      g_system_ptr->getTracer()->stopTrace();
    }
    g_eventQueue_ptr->triggerAllEvents();
    
    // This call is placed here to make sure the cache dump code doesn't fall victim to code rot
    if (!g_param_ptr->SYNTHETIC_DRIVER()) {
      tester_record_cache();
    }
  }
  tester_destroy();
}

void tester_destroy()
{
  g_system_ptr->printStats(cout);

  // Clean up
  destroy_simulator();
  cerr << "Success: " << CURRENT_PROTOCOL << endl;
}

void tester_record_cache()
{
  cout << "Testing recording of cache contents" << endl;
  CacheRecorder recorder;
  g_system_ptr->recordCacheContents(recorder);
  int written = recorder.dumpRecords("ruby.caches.gz");
  int read = Tracer::playbackTrace("ruby.caches.gz");
  assert(read == written);
  cout << "Testing recording of cache contents completed" << endl;
}

void tester_playback_trace()
{
  assert(trace_filename != "");
  cout << "Reading trace from file '" << trace_filename << "'..." << endl;
  int read = Tracer::playbackTrace(trace_filename);
  cout << "(" << read << " requests read)" << endl;
  if (read == 0) {
    ERROR_MSG("Zero items read from tracefile.");
  }
}

//    if ((trace_filename != "") || (g_param_ptr->TESTER_LENGTH() != 0)) {
//      if ((trace_filename != "") && (g_param_ptr->TESTER_LENGTH() != 0)) {
//        cerr << "Error: both a run length (-l) and a trace file (-z) have been specified." << endl;
//        usageInstructions();
//      }
//    }
