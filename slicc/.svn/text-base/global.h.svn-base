/* This file is part of SLICC (Specification Language for Implementing
   Cache Coherence), a component of the Multifacet GEMS (General
   Execution-driven Multiprocessor Simulator) software toolset
   originally developed at the University of Wisconsin-Madison.

   SLICC was originally developed by Milo Martin with substantial
   contributions from Daniel Sorin.

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

#include <assert.h> /* slicc needs to include this in order to use classes in
                     * ../common directory.
                     */

typedef unsigned char uint8;
typedef unsigned int uint32;
typedef unsigned long long uint64;

typedef signed char int8;
typedef int int32;
typedef long long int64;

typedef int64 Time;
typedef long long integer_t;
typedef unsigned long long uinteger_t;

#include "std-includes.h"
#include "Map.h"
#include "util.h"

const bool ASSERT_FLAG = true;

#undef assert
#define assert(EXPR) ASSERT(EXPR)

#define ASSERT(EXPR)\
{\
  if (ASSERT_FLAG) {\
    if (!(EXPR)) {\
      cerr << "failed assertion '"\
           << #EXPR << "' at fn "\
           << __PRETTY_FUNCTION__ << " in "\
           << __FILE__ << ":"\
           << __LINE__ << endl;\
      abort();\
    }\
  }\
}

class SymbolTable;
class Symbol;
class Var;
class Func;
class Type;
class Enum;
class Struct;
class ExternType;
class StateMachine;
class State;
class Event;
class Action;
class Transition;

namespace __gnu_cxx {
  template<>
  struct hash<State*>
  {
    size_t operator()(State* s) const { return (size_t) s; }
  };
  template<>
  struct hash<Event*>
  {
    size_t operator()(Event* s) const { return (size_t) s; }
  };
  template<>
  struct hash<Symbol*>
  {
    size_t operator()(Symbol* s) const { return (size_t) s; }
  };
  template<>
  struct hash<Var*>
  {
    size_t operator()(Var* s) const { return (size_t) s; }
  };
}

namespace std {
  template<>
  struct equal_to<Event*>
  {
    bool operator()(Event* s1, Event* s2) const { return s1 == s2; }
  };
  template<>
  struct equal_to<State*>
  {
    bool operator()(State* s1, State* s2) const { return s1 == s2; }
  };
  template<>
  struct equal_to<Symbol*>
  {
    bool operator()(Symbol* s1, Symbol* s2) const { return s1 == s2; }
  };
  template<>
  struct equal_to<Var*>
  {
    bool operator()(Var* s1, Var* s2) const { return s1 == s2; }
  };
} // namespace std

#endif //GLOBAL_H
