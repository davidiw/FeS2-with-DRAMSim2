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
#include "Debug.h"
#include "EventQueue.h"
#include "Param.h"
//#include <unistd.h>

class Debug;
extern Debug* g_debug_ptr;

// component character list
const char DEFINE_COMP_CHAR[] =
{
#undef DEFINE_COMP
#define DEFINE_COMP(component, character, description) character,
#include "Debug.def"
};

// component description list
const char* DEFINE_COMP_DESCRIPTION[] =
{
#undef DEFINE_COMP
#define DEFINE_COMP(component, character, description) description,
#include "Debug.def"
};

Debug::Debug()
{
  m_verbosityLevel = No_Verb;
  clearFilter();
  m_starting_cycle = 0;

  assert(g_param_ptr != NULL);
  setDebugTime(g_param_ptr->DEBUG_START_TIME());
  setVerbosity(g_param_ptr->DEBUG_VERBOSITY());
  setFilter(g_param_ptr->DEBUG_FILTER());
}

Debug::~Debug()
{
}

void Debug::printVerbosity(ostream& out) const
{
  switch (getVerbosity()) {
  case No_Verb:
    out << "verbosity = No_Verb" << endl;
    break;
  case Low_Verb:
    out << "verbosity = Low_Verb" << endl;
    break;
  case Med_Verb:
    out << "verbosity = Med_Verb" << endl;
    break;
  case High_Verb:
    out << "verbosity = High_Verb" << endl;
    break;
  default:
    out << "verbosity = unknown" << endl;
  }
}

bool Debug::validDebug(DebugComponents module, PriorityLevel priority) const
{
  assert(this != NULL);
  int local_module = (1 << module);
  if(m_filter & local_module) {
    if (g_eventQueue_ptr == NULL || g_eventQueue_ptr->getTime() >= m_starting_cycle) {
      switch(m_verbosityLevel) {
      case No_Verb:
        return false;
      case Low_Verb:
        return (priority == HighPrio);
      case Med_Verb:
        return (priority == HighPrio || priority == MedPrio);
      case High_Verb:
        return true;
      }
    }
  }
  return false;
}

void Debug::setVerbosity(const string& verb_str)
{
  if (string(verb_str) == "none") {
    setVerbosity(No_Verb);
  } else if (string(verb_str) == "low") {
    setVerbosity(Low_Verb);
  } else if (string(verb_str) == "med") {
    setVerbosity(Med_Verb);
  } else if (string(verb_str) == "high") {
    setVerbosity(High_Verb);
  } else {
    ERROR_MSG("Error: unrecognized verbosity (use none, low, med, high): " + verb_str);
  }
}

void Debug::setFilter(const string& filter_str)
{
  if (filter_str == "all" ) {
    allFilter();
  } else if (filter_str == "none") {
    clearFilter();
  } else {
    // scan string adding to bit mask for each component which is present
    for (unsigned int i=0; i < filter_str.length(); i++) {
      addFilter(filter_str[i]);
    }
  }
}

void Debug::addFilter(char ch)
{
  for (int i=0; i<NUMBER_OF_COMPS; i++) {
    // Look at all components to find a character match
    if (DEFINE_COMP_CHAR[i] == ch) {
      // We found a match - update the filter bit mask
      cout << "  Debug: Adding to filter: '" << ch << "' (" << DEFINE_COMP_DESCRIPTION[i] << ")" << endl;
      m_filter |= (1 << i);
      return;
    }
  }
  
  // We didn't find the character
  ERROR_MSG("Error: unrecognized component filter: " + ch);
}

void Debug::usageInstructions()
{
  cerr << "Debug components: " << endl;
  for (int i=0; i<NUMBER_OF_COMPS; i++) {
    cerr << "  " << DEFINE_COMP_CHAR[i] << ": " << DEFINE_COMP_DESCRIPTION[i] << endl;
  }
}

void pre_abort()
{
  // Before aborting, give a chance to connect gdb
  if (isatty(STDIN_FILENO)) {
    cerr << "At this point you might want to attach a debug to ";
    cerr << "the running and get to the" << endl;
    cerr << "crash site; otherwise press enter to continue" << endl;
    cerr << "PID: " << getpid();
    cerr << endl << flush;
    char c;
    cin.get(c);
  }
}
