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

/*
 * Description: The Network class is the base class for classes that
 * implement the interconnection network between components
 * (processor/cache components and memory/directory components).  The
 * interconnection network as described here is not a physical
 * network, but a programming concept used to implement all
 * communication between components.  Thus parts of this 'network'
 * will model the on-chip connections between cache controllers and
 * directory controllers as well as the links between chip and network
 * switches.
 *
 * */

#ifndef NETWORK_H
#define NETWORK_H

#include "Global.h"
#include "NodeID.h"
#include "MessageSizeType.h"
#include "MachineType.h"
#include "Vector.h"

class MessageBuffer;
class Throttle;

class Network {
public:
  // Constructors
  Network() {}

  // Destructor
  virtual ~Network() {}

  // Public Methods

  // returns the queue requested for the given component
  virtual MessageBuffer* getToNetQueue(MachineType machine, NodeID id, bool ordered, int netNumber) = 0;
  virtual MessageBuffer* getFromNetQueue(MachineType machine, NodeID id, bool ordered, int netNumber) = 0;
  virtual const Vector<Throttle*>* getThrottles(NodeID id) const { return NULL; }

  virtual void printStats(ostream& out) const = 0;
  virtual void clearStats() = 0;
  virtual void printConfig(ostream& out) const = 0;
  virtual void print(ostream& out) const = 0;
  static Network* create();

private:

  // Private Methods
  // Private copy constructor and assignment operator
  Network(const Network& obj);
  Network& operator=(const Network& obj);

  // Data Members (m_ prefix)
};

// Output operator declaration
ostream& operator<<(ostream& out, const Network& obj);

// ******************* Definitions *******************

// Output operator definition
extern inline 
ostream& operator<<(ostream& out, const Network& obj)
{
  obj.print(out);
  out << flush;
  return out;
}

// Code to map network message size types to an integer number of bytes
const int CONTROL_MESSAGE_SIZE = 8;
const int DATA_MESSAGE_SIZE = (64+8);

extern inline
int MessageSizeType_to_int(MessageSizeType size_type)
{
  switch(size_type) {
  case MessageSizeType_Undefined:
    ERROR_MSG("Can't convert Undefined MessageSizeType to integer");
    break;
  case MessageSizeType_Control:
  case MessageSizeType_Request_Control:
  case MessageSizeType_Reissue_Control:
  case MessageSizeType_Response_Control:
  case MessageSizeType_Writeback_Control:
  case MessageSizeType_Forwarded_Control:
  case MessageSizeType_Unblock_Control:
  case MessageSizeType_Persistent_Control:
  case MessageSizeType_Completion_Control:
    return CONTROL_MESSAGE_SIZE;
    break;
  case MessageSizeType_Data:
  case MessageSizeType_Response_Data:
  case MessageSizeType_Writeback_Data:
    return DATA_MESSAGE_SIZE;
    break;
  default:
    ERROR_MSG("Invalid range for type MessageSizeType");
    break;
  }
  return 0;
}

#endif //NETWORK_H
