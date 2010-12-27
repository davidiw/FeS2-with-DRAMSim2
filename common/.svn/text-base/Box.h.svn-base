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

#ifndef __BOX_HEADER
#define __BOX_HEADER

template <typename T>
class Box {
private:
  bool empty;
  T contents;
public:
  Box(T t) { 
    empty = false;
    contents = t; 
  }
  Box() {
    empty = true;
  }

  bool isEmpty() { return empty; }

  T unbox() { return contents; }

  void print(ostream& os) {
    if ( !empty ) {
      os << contents;
    }
  }

}; // end class Box

#endif
