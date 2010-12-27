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

#ifndef SubBlock_H
#define SubBlock_H

#include "Global.h"
#include "Address.h"
#include "DataBlock.h"
#include "Vector.h"

class SubBlock {
public:
  // Constructors
  SubBlock() { } 
  SubBlock(const Address& addr, int size);

  // Destructor
  ~SubBlock() { }
  
  // Public Methods
  const Address& getAddress() const { return m_address; }
  void setAddress(const Address& addr) { m_address = addr; }

  int getSize() const { return m_data.size(); }
  void setSize(int size) {  m_data.setSize(size); }
  uint8 getByte(int offset) const { return m_data[offset]; }
  void setByte(int offset, uint8 data) { m_data[offset] = data; }

  // Shorthands
  uint8 readByte() const { return getByte(0); }
  void writeByte(uint8 data) { setByte(0, data); }

  // Merging to and from DataBlocks - We only need to worry about
  // updates when we are using DataBlocks
  void mergeTo(DataBlock& data) const;
  void mergeFrom(const DataBlock& data);

  void print(ostream& out) const;
private:
  // Private Methods
  //  SubBlock(const SubBlock& obj);
  //  SubBlock& operator=(const SubBlock& obj);
  //  bool bytePresent(const Address& addr) { return ((addr.getAddress() >= m_address.getAddress()) && (addr.getAddress() < (m_address.getAddress()+getSize()))); }
  //  uint8 getByte(const Address& addr) { return m_data[addr.getAddress() - m_address.getAddress()]; }

  // Data Members (m_ prefix)
  Address m_address;
  Vector<uint8> m_data;
};

// Output operator declaration
ostream& operator<<(ostream& out, const SubBlock& obj);

// ******************* Definitions *******************

// Output operator definition
extern inline 
ostream& operator<<(ostream& out, const SubBlock& obj)
{
  obj.print(out);
  out << flush;
  return out;
}

#endif //SubBlock_H
