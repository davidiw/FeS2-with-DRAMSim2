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

#ifndef DOUBLE_WORD_H
#define DOUBLE_WORD_H

#include "decoder/globals.h"
#include "Masks.h"

class DoubleWord {
public:
  DoubleWord() { clear(); };
  DoubleWord(const DoubleWord &dw);
  DoubleWord &operator= (const DoubleWord &dw);

  static void setAddressMask(Waddr mask) { s_address_mask = mask; }

  size_t getSize() const { return m_size; }
  void clear() { m_address = 0; m_bytemask = 0; m_value = 0; m_size = 0; }
  bool isClear() const { return (m_address == 0) && (m_bytemask == 0) && (m_value == 0) && (m_size == 0);}

  Waddr addr() const { return m_address; }
  Waddr alignedAddr() const { return m_address & s_address_mask; }
  W8 bytemask() const { return m_bytemask; }
  W64 mask() const { return MASKS[m_bytemask]; }
  W64 value() const { return m_value; }

  void setValue(W64 value);
  size_t setAddress(Waddr address, size_t m_size); 
  void writeValue(DoubleWord &dw) const;
  void overlay(DoubleWord &target_dw) const;

  bool compareValue(W64 value) const;
  bool overlaps(const DoubleWord &_dw) const;
  bool overlapMatches(const DoubleWord &dw) const;
  bool covers(const DoubleWord &dw) const;

private:
  W8 writeValue(W64 &target, int shiftAmt) const;

  W64 m_value; // should be zeros in bytes that are 0's in bytemask
  Waddr m_address;  // original address provided (not necessarily aligned)
  W8 m_size; // in bytes
  W8 m_bytemask; // 8 bits: one bit per byte
  static Waddr s_address_mask;
};

#endif /* DOUBLE_WORD_H */
