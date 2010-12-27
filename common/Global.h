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

#ifndef GLOBAL_H
#define GLOBAL_H

typedef unsigned char uint8;
typedef unsigned int uint32;
typedef unsigned long long uint64;

typedef signed char int8;
typedef int int32;
typedef long long int64;

typedef long long integer_t;
typedef unsigned long long uinteger_t;

typedef int64 Time;
typedef uint64 physical_address_t;
typedef uint64 la_t;
typedef uint64 pa_t;

// external includes for all classes
#include "std-includes.h"
#include "Debug.h"

// simple type declarations
typedef int64 Index;            // what the address bit ripper returns
typedef int SwitchID;
typedef int LinkID;

class EventQueue;
extern EventQueue* g_eventQueue_ptr;

class System;
extern System* g_system_ptr;

class Debug;
extern Debug* g_debug_ptr;

// The following function is use only by Milo's token protocol.  This
// location isn't the best place for this function, but it is included
// here so all the generated SLICC files can find it.
extern inline int max_tokens()
{
  return 1024;
}

bool string_to_bool(const string& str);
int string_to_int(const string& str);
int64 string_to_int64(const string& str);
double string_to_double(const string& str);
Time string_to_Time(const string& str);

#endif //GLOBAL_H

