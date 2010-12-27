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

#include "Tracer.h"
#include "TraceRecord.h"
#include "EventQueue.h"
#include "PrioHeap.h"
#include "System.h"
#include "Param.h"

Tracer::Tracer()
{
  m_enabled = false;
}

Tracer::~Tracer()
{
}

void Tracer::startTrace(string filename)
{
  if (m_enabled) {
    stopTrace();
  }

  if (filename != "") {
    m_trace_file.open(filename.c_str());
    if (m_trace_file.fail()) {
      cout << "Error: error opening file '" << filename << "'" << endl;
      cout << "Trace not enabled." << endl;
      return;
    }
    cout << "Request trace enabled to output file '" << filename << "'" << endl;
    m_enabled = true;
  }
}

void Tracer::stopTrace()
{
  assert(m_enabled == true);
  m_trace_file.close();
  cout << "Request trace file closed." << endl;
  m_enabled = false;
}

void Tracer::traceRequest(NodeID id, const Address& data_addr, const Address& pc_addr, CacheRequestType type, Time time)
{
  assert(m_enabled == true);
  TraceRecord tr(id, data_addr, pc_addr, type, time);
  tr.output(m_trace_file);
}

// Class method
int Tracer::playbackTrace(string filename)
{
  igzstream in(filename.c_str());
  if (in.fail()) {
    cout << "Error: error opening file '" << filename << "'" << endl;
    return 0;
  }
  
  time_t start_time = time(NULL);

  TraceRecord record;
  int counter = 0;
  // Read in the next TraceRecord
  bool ok = record.input(in);
  while (ok) {
    // Put it in the right cache
    record.issueRequest();
    counter++;

    // Read in the next TraceRecord
    ok = record.input(in);

    // Clear the statistics after warmup
    if (counter == g_param_ptr->TRACE_WARMUP_LENGTH()) {
      cout << "Clearing stats after warmup of length " << g_param_ptr->TRACE_WARMUP_LENGTH() << endl; 
      g_system_ptr->clearStats();
    }
  }
  
  // Flush the prefetches through the system
  g_eventQueue_ptr->triggerEvents(g_eventQueue_ptr->getTime() + 1000);  // FIXME - should be smarter

  time_t stop_time = time(NULL);
  double seconds = difftime(stop_time, start_time);
  double minutes = seconds / 60.0;
  cout << "playbackTrace: " << minutes << " minutes" << endl;

  return counter;
}
