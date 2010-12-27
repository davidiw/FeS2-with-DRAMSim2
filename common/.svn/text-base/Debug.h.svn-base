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

#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>
#include <string>

using namespace std;

// component enumeration
enum DebugComponents
{
#undef DEFINE_COMP
#define DEFINE_COMP(component, character, description) component,
#include "Debug.def"
  NUMBER_OF_COMPS
};

enum PriorityLevel {HighPrio, MedPrio, LowPrio};
enum VerbosityLevel {No_Verb, Low_Verb, Med_Verb, High_Verb};

class Debug {
public:
  // Constructors
  Debug();

  // Destructor
  ~Debug();
  
  // Public Methods
  bool validDebug(DebugComponents module, PriorityLevel priority) const;

  void setVerbosity(const string& verb_str);
  void setFilter(const string& filter_str);
  void setDebugTime(Time t) { m_starting_cycle = t; }
  Time getDebugTime() const { return m_starting_cycle; }

  void print(ostream& out) const {};
  void printVerbosity(ostream& out) const;
  static void usageInstructions(void);

private:
  // Private Methods
  void clearFilter() { m_filter = 0; }
  void allFilter() { m_filter = ~0; }
  void setVerbosity(VerbosityLevel vb) { m_verbosityLevel = vb; }
  void addFilter(char ch);
  VerbosityLevel getVerbosity() const { return m_verbosityLevel; }

  // Private copy constructor and assignment operator
  Debug(const Debug& obj);
  Debug& operator=(const Debug& obj);
  
  // Data Members (m_ prefix)
  VerbosityLevel m_verbosityLevel;
  int m_filter;
  Time m_starting_cycle;
};

// Output operator declaration
ostream& operator<<(ostream& out, const Debug& obj);

// ******************* Definitions *******************

// Output operator definition
extern inline 
ostream& operator<<(ostream& out, const Debug& obj)
{
  obj.print(out);
  out << flush;
  return out;
}

void pre_abort();

const bool ERROR_MESSAGE_FLAG = true;
const bool WARNING_MESSAGE_FLAG = true;

#ifdef RUBY_NO_ASSERT
const bool ASSERT_FLAG = false;
#else
const bool ASSERT_FLAG = true;
#endif

#undef assert
#define assert(EXPR) ASSERT(EXPR)
#undef ASSERT
#define ASSERT(EXPR)\
{\
  if (ASSERT_FLAG) {\
    if (!(EXPR)) {\
      cerr << "failed assertion '"\
           << #EXPR << "' at fn "\
           << __PRETTY_FUNCTION__ << " in "\
           << __FILE__ << ":"\
           << __LINE__ << endl << flush;\
      cout << "failed assertion '"\
           << #EXPR << "' at fn "\
           << __PRETTY_FUNCTION__ << " in "\
           << __FILE__ << ":"\
           << __LINE__ << endl << flush;\
      pre_abort();\
      abort();\
    }\
  }\
}

#undef assert_msg
#define assert_msg(EXPR, MSG) ASSERT_MSG(EXPR, MSG)
#undef ASSERT_MSG
/** assert() with a message attached */
#define ASSERT_MSG(EXPR, MESSAGE)\
{\
  if (ASSERT_FLAG) {\
    if (!(EXPR)) {\
      cerr << "failed assertion '"\
           << #EXPR << "' at fn "\
           << __PRETTY_FUNCTION__ << " in "\
           << __FILE__ << ":"\
           << __LINE__ << ": "\
           << (MESSAGE) << endl << flush;\
      cout << "failed assertion '"\
           << #EXPR << "' at fn "\
           << __PRETTY_FUNCTION__ << " in "\
           << __FILE__ << ":"\
           << __LINE__ << ": "\
           << (MESSAGE) << endl << flush;\
      std::abort();\
    }\
  }\
}

#define ERROR_MSG(MESSAGE)\
{\
  if (ERROR_MESSAGE_FLAG) {\
    cerr << "Fatal Error: in fn "\
         << __PRETTY_FUNCTION__ << " in "\
         << __FILE__ << ":"\
         << __LINE__ << ": "\
         << (MESSAGE) << endl << flush;\
    cout << "Fatal Error: in fn "\
         << __PRETTY_FUNCTION__ << " in "\
         << __FILE__ << ":"\
         << __LINE__ << ": "\
         << (MESSAGE) << endl << flush;\
    pre_abort();\
    abort();\
  }\
}

#define WARN_MSG(MESSAGE)\
{\
  if (WARNING_MESSAGE_FLAG) {\
    cerr << "Warning: in fn "\
         << __PRETTY_FUNCTION__ << " in "\
         << __FILE__ << ":"\
         << __LINE__ << ": "\
         << (MESSAGE) << endl << flush;\
    cout << "Warning: in fn "\
         << __PRETTY_FUNCTION__ << " in "\
         << __FILE__ << ":"\
         << __LINE__ << ": "\
         << (MESSAGE) << endl << flush;\
  }\
}

#define WARN_EXPR(EXPR)\
{\
  if (WARNING_MESSAGE_FLAG) {\
    cerr << "Warning: in fn "\
         << __PRETTY_FUNCTION__ << " in "\
         << __FILE__ << ":"\
         << __LINE__ << ": "\
         << #EXPR << " is "\
         << (EXPR) << endl << flush;\
    cout << "Warning: in fn "\
         << __PRETTY_FUNCTION__ << " in "\
         << __FILE__ << ":"\
         << __LINE__ << ": "\
         << #EXPR << " is "\
         << (EXPR) << endl << flush;\
  }\
}

#define DEBUG_MSG(module, priority, MESSAGE)\
{\
  if (RUBY_DEBUG) {\
    if (g_debug_ptr != NULL && g_debug_ptr->validDebug(module, priority)) {\
      cout << "Debug: in fn "\
           << __PRETTY_FUNCTION__\
           << " in " << __FILE__ << ":"\
           << __LINE__ << ": "\
           << (MESSAGE) << endl << flush;\
    }\
  }\
}

#define DEBUG_EXPR(module, priority, EXPR)\
{\
  if (RUBY_DEBUG) {\
    if (g_debug_ptr != NULL && g_debug_ptr->validDebug(module, priority)) {\
      cout << "Debug: in fn "\
           << __PRETTY_FUNCTION__\
           << " in " << __FILE__ << ":"\
           << __LINE__ << ": "\
           << #EXPR << " is "\
           << (EXPR) << endl << flush;\
    }\
  }\
}

#define DEBUG_NEWLINE(module, priority)\
{\
  if (RUBY_DEBUG) {\
    if (g_debug_ptr != NULL && g_debug_ptr->validDebug(module, priority)) {\
      cout << endl << flush;\
    }\
  }\
}

#define DEBUG_SLICC(priority, LINE, MESSAGE)\
{\
  if (RUBY_DEBUG) {\
    if (g_debug_ptr != NULL && g_debug_ptr->validDebug(SLICC_COMP, priority)) {\
      cout << (LINE) << (MESSAGE) << endl << flush;\
    }\
  }\
}

#endif //DEBUG_H

