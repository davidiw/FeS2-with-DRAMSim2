// ----------------------------------------------------------------------
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
// ----------------------------------------------------------------------

//===-- DoubleWord.h - a 64-bit memory word ---------------------*- C++ -*--=//
//
// This class manages up to a 64-bit memory word that does not cross cache
// alignment boundaries.
// 
// All caches and memory images are little endian (for x86).
//
//===---------------------------------------------------------------------===//

#include "DoubleWord.h"
#include "Masks.h"

Waddr DoubleWord::s_address_mask;

DoubleWord::DoubleWord(const DoubleWord &dw) {
  m_value = dw.m_value;
  m_address = dw.m_address;
  m_size = dw.m_size;
  m_bytemask = dw.m_bytemask;
}

DoubleWord &
DoubleWord::operator =(const DoubleWord &rhs) {
  if (this != &rhs) {
    m_value = rhs.m_value;
    m_address = rhs.m_address;
    m_size = rhs.m_size;
    m_bytemask = rhs.m_bytemask;
  }
}

void 
DoubleWord::setValue(W64 value) {
  assert(m_bytemask == 0);
  m_bytemask = ((1 << m_size) - 1);
  assert(m_bytemask != 0);
  m_value = value & MASKS[m_bytemask];
}

bool
DoubleWord::compareValue(W64 value) const { 
  value &= MASKS[m_bytemask];
  return (m_value == value);
}

W8
DoubleWord::writeValue(W64 &target, int shiftAmt) const {
  W64 shiftedValue = m_value;
  W8 shiftedBytemask = m_bytemask;
  if (shiftAmt < 0) {
    shiftedValue = m_value << (-shiftAmt * 8);
    shiftedBytemask = (m_bytemask << -shiftAmt) & 0xff;
  } else if (shiftAmt > 0) {
    shiftedValue = m_value >> (shiftAmt * 8);
    shiftedBytemask = (m_bytemask >> shiftAmt) & 0xff;
  }
  W64 shiftedMask = MASKS[shiftedBytemask];
  target &= ~shiftedMask;
  target |= shiftedValue & shiftedMask;
  return shiftedBytemask;
}

// used to apply this value as a write to another DoubleWord 
void 
DoubleWord::writeValue(DoubleWord &target_dw) const {
  if (overlaps(target_dw)) {
    writeValue(target_dw.m_value, (int)(target_dw.m_address - m_address));
    target_dw.m_value &= MASKS[target_dw.m_bytemask];
  }
}

// the data held in "this" object is layered over the data in
// "target_dw", potentially increasing its mask.
void 
DoubleWord::overlay(DoubleWord &target_dw) const {
  if (overlaps(target_dw)) {
    int shiftAmt = (int)(target_dw.m_address - m_address);
    W8 shiftedBytemask = writeValue(target_dw.m_value, shiftAmt);
    target_dw.m_bytemask |= shiftedBytemask;
  }
}

// do the words overlap?
bool
DoubleWord::overlaps(const DoubleWord &_dw) const {
  bool overlap;
  if (_dw.m_address <= m_address) {
    overlap = (_dw.m_address + _dw.m_size) > m_address;
  } else {
    overlap = (m_address + m_size) > _dw.m_address;
  }
  return overlap;
}

// True if any bytes overlapped between the double words must have
// the same value.  We find this by xoring the values together,
// then masking out any where there was no overlap.
bool 
DoubleWord::overlapMatches(const DoubleWord &dw) const {
  if (!overlaps(dw)) { return true; } // non-overlapping words implicitly match
  
  W64 shiftedValue;
  int shiftAmt = (int)(dw.m_address - m_address);
  W8 shiftedBytemask = writeValue(shiftedValue, shiftAmt);
  
  W8 overlap = shiftedBytemask & dw.m_bytemask;
  W64 xored_value = shiftedValue ^ dw.m_value;
  W64 masked_xored_value = xored_value & MASKS[overlap];
  return (masked_xored_value == 0);
}

// True if the specified word covers this one
bool
DoubleWord::covers(const DoubleWord &dw) const {
  if (!overlaps(dw)) { return false; }
  else if (m_address < dw.m_address) { return false; }
  else {
    int shiftAmt = (int)(m_address - dw.m_address);
    W8 desired_mask = (1 << m_size) - 1;
    return ((dw.m_bytemask >> shiftAmt) & desired_mask) == desired_mask;
  }
}

// returns number of bytes of overflow if access crosses double_word
// boundary.
size_t
DoubleWord::setAddress(Waddr address, size_t size) {
  m_address = address;
  m_size = size;
  m_value = 0;
  size_t overflow = (m_address + m_size) & ~s_address_mask;
  if (overflow < m_size) {
    m_size -= overflow;
  } else {
    overflow = 0;
  }
  assert((m_size + (m_address & ~s_address_mask)) <= (~s_address_mask + 1));
  return overflow;
}
