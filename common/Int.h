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

#ifndef Int
#error Do not include Int directly
#endif

class Int {
public:
  // Constructors
  Int() { m_id = -1; }
  Int(int id) { m_id = id; }

  // Destructor
  ~Int() { }
  
  // Public Methods
  bool equal(const Int& obj) const { assert(m_id != -1); assert(obj.m_id != -1);  return m_id == obj.m_id; }
  void print(ostream& out) const { out << "[" << INT << ": " << m_id << "]"; }
  void clear() { m_id = -1; }
  operator int() const { return m_id; }

private:
  // Private Methods

  // Data Members (m_ prefix)
  int m_id;
};

PRINT_OPERATOR(Int);
EQUAL_OPERATOR(Int);
