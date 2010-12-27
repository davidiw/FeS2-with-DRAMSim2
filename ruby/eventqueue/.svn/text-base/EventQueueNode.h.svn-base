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

#ifndef EVENTQUEUENODE_H
#define EVENTQUEUENODE_H

#include "Global.h"
class Consumer;

class EventQueueNode {
public:
  // Constructors
  EventQueueNode() { m_time = 0; m_consumer_ptr = NULL; }

  // Destructor
  //~EventQueueNode();
  
  // Public Methods
  void print(ostream& out) const;

  // Assignment operator and copy constructor since the default
  // constructors confuse purify when long longs are present.
  EventQueueNode& operator=(const EventQueueNode& obj) {
    m_time = obj.m_time; 
    m_consumer_ptr = obj.m_consumer_ptr;
    return *this;
  }
  
  EventQueueNode(const EventQueueNode& obj) {
    m_time = obj.m_time; 
    m_consumer_ptr = obj.m_consumer_ptr;
  }
private:
  // Private Methods

  // Default copy constructor and assignment operator
  // EventQueueNode(const EventQueueNode& obj);
  
  // Data Members (m_ prefix)
public:
  Time m_time;
  Consumer* m_consumer_ptr;  
};

// Output operator declaration
ostream& operator<<(ostream& out, const EventQueueNode& obj);

// ******************* Definitions *******************

inline extern bool node_less_then_eq(const EventQueueNode& n1, const EventQueueNode& n2);

inline extern
bool node_less_then_eq(const EventQueueNode& n1, const EventQueueNode& n2)
{
  return (n1.m_time <= n2.m_time);
}

// Output operator definition
extern inline 
ostream& operator<<(ostream& out, const EventQueueNode& obj)
{
  obj.print(out);
  out << flush;
  return out;
}

#endif //EVENTQUEUENODE_H
