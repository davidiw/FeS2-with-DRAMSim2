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


#ifndef ADDRESS_H
#define ADDRESS_H

#include <iomanip>
#include "Global.h"
#include "NodeID.h"
#include "Param.h"

const int ADDRESS_WIDTH = 64; // address width in bytes
const int DATA_BLOCK_BITS = 6; // log of address width

class Address {
public:
  // Constructors
  Address() { m_address = 0; }
  explicit Address(physical_address_t address) { m_address = address; }

  Address(const Address& obj);
  Address& operator=(const Address& obj);

  // Destructor
  //  ~Address();

  // Public Methods

  void setAddress(physical_address_t address) { m_address = address; }
  physical_address_t getAddress() const {return m_address;}
  // selects bits inclusive
  physical_address_t bitSelect(int small, int big) const; 
  physical_address_t maskLowOrderBits(int number) const; 
  physical_address_t shiftLowOrderBits(int number) const; 
  bool isBlockAligned() const { return m_address == maskLowOrderBits(DATA_BLOCK_BITS); }
  physical_address_t getLineAddress() const { return maskLowOrderBits(DATA_BLOCK_BITS); }
  physical_address_t getOffset() const { return bitSelect(0, DATA_BLOCK_BITS-1); }
  void makeLineAddress() { m_address = maskLowOrderBits(DATA_BLOCK_BITS); }
    
  void print(ostream& out) const;
  void output(ostream& out) const;
  void input(istream& in);

private:
  // Private Methods

  // Private copy constructor and assignment operator
  //  Address(const Address& obj);
  //  Address& operator=(const Address& obj);
  
  // Data Members (m_ prefix)
  physical_address_t m_address;
};

inline
Address line_address(const Address& addr) { Address temp(addr); temp.makeLineAddress(); return temp; }

// Output operator declaration
ostream& operator<<(ostream& out, const Address& obj);
// comparison operator declaration
bool operator==(const Address& obj1, const Address& obj2);
bool operator!=(const Address& obj1, const Address& obj2);
bool operator<(const Address& obj1, const Address& obj2);

inline
bool operator<(const Address& obj1, const Address& obj2)
{
  return obj1.getAddress() < obj2.getAddress();
}

// ******************* Definitions *******************

// Output operator definition
inline
ostream& operator<<(ostream& out, const Address& obj)
{
  obj.print(out);
  out << flush;
  return out;
}

inline
bool operator==(const Address& obj1, const Address& obj2)
{
  return (obj1.getAddress() == obj2.getAddress());
}

inline
bool operator!=(const Address& obj1, const Address& obj2)
{
  return (obj1.getAddress() != obj2.getAddress());
}

inline
physical_address_t Address::bitSelect(int small, int big) const // rips bits inclusive
{
  physical_address_t mask;
  assert((big + 1) >= small);

  if (big >= ADDRESS_WIDTH - 1) {
    return (m_address >> small);
  } else {
    mask = ~((physical_address_t)~0 << (big + 1));
    // FIXME - this is slow to manipulate a 64-bit number using 32-bits
    physical_address_t partial = (m_address & mask); 
    return (partial >> small);
  }
}

inline
physical_address_t Address::maskLowOrderBits(int number) const 
{
  physical_address_t mask;
  
  if (number >= ADDRESS_WIDTH - 1) {
    mask = ~0;
  } else {
    mask = (physical_address_t)~0 << number;
  }
  return (m_address & mask);
}

inline
physical_address_t Address::shiftLowOrderBits(int number) const 
{
  return (m_address >> number);
}

inline
void Address::print(ostream& out) const
{
  out << "[" << hex << "0x" << m_address << "," << " line 0x" << maskLowOrderBits(DATA_BLOCK_BITS) << dec << "]" << flush;
}

class Address;
namespace __gnu_cxx {
  template<>
  struct hash<Address>
  {
    size_t operator()(const Address &s) const { return (size_t) s.getAddress(); }
  };
}
namespace std {
  template<>
  struct equal_to<Address>
  {
    bool operator()(const Address& s1, const Address& s2) const { return s1 == s2; }
  };
}

#endif //ADDRESS_H
