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

#include "DataBlock.h"
#include "Param.h"

DataBlock::DataBlock() 
{
  if (g_param_ptr->DATA_BLOCK()) {
    m_data.setSize(1 << g_param_ptr->DATA_BLOCK_BITS());
  }
  clear();
}

DataBlock::~DataBlock()
{

}

void DataBlock::clear()
{
  int size = m_data.size();
  for (int i = 0; i < size; i++) {
    m_data[i] = 0;
  }
}

bool DataBlock::equal(const DataBlock& obj) const
{
  bool value = true;
  int size = m_data.size();
  for (int i = 0; i < size; i++) {
    value = value && (m_data[i] == obj.m_data[i]);
  }
  return value;
}

void DataBlock::print(ostream& out) const
{
  out << "[";
  int size = m_data.size();
  for (int i = 0; i < size; i+=4) {
    out << hex << *((uint32*)(&(m_data[i]))) << " ";
  }
  out << dec << "]" << flush;
}

uint8 DataBlock::getByte(int whichByte) const
{
  if (g_param_ptr->DATA_BLOCK()) {
    return m_data[whichByte];
  } else {
    return 0;
  }
}

void DataBlock::setByte(int whichByte, uint8 data)
{
  if (g_param_ptr->DATA_BLOCK()) {
    m_data[whichByte] = data;
  }
}
