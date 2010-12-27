/* --------------------------------------------------------------------

   Copyright (C) 1999-2005 by Mark D. Hill and David A. Wood for the
   Wisconsin Multifacet Project.  Contact: gems@cs.wisc.edu
   http://www.cs.wisc.edu/gems/

   This file is based upon a pre-release version of Multifacet GEMS
   from 2004 and may also contain additional modifications and code
   Copyright (C) 2004-2007 by Milo Martin for the Penn Architecture
   and Compilers Group.  Contact: acg@lists.seas.upenn.edu
   http://www.cis.upenn.edu/acg/

   --------------------------------------------------------------------
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

#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "Vector.h"

template <class TYPE>
class Allocator {
public:
  // Constructors
  Allocator() { m_counter = 0; }

  // Destructor
  ~Allocator() { for(int i=0; i<m_pool_vec.size(); i++) { delete m_pool_vec[i]; }}
  
  // Public Methods
  TYPE* allocate(const TYPE& obj);
  void deallocate(TYPE* obj_ptr);
private:
  // Private copy constructor and assignment operator
  Allocator(const Allocator& obj);
  Allocator& operator=(const Allocator& obj);

  // Private Methods

  // Data Members (m_ prefix)
  Vector<TYPE*> m_pool_vec;
  int m_counter;
};

template <class TYPE> 
inline 
TYPE* Allocator<TYPE>::allocate(const TYPE& obj)
{ 
  m_counter++;
  DEBUG_EXPR(ALLOCATOR_COMP, LowPrio, m_counter);
  TYPE* new_obj_ptr;
  
  // See if we need to allocate any new objects
  if (m_pool_vec.size() == 0) {
    // Allocate a new item
    m_pool_vec.insertAtBottom(new TYPE);
  }

  // Pop the pointer from the stack/pool
  int size = m_pool_vec.size();
  new_obj_ptr = m_pool_vec[size-1];
  m_pool_vec.setSize(size-1);

  // Copy the object
  *new_obj_ptr = obj;
  return new_obj_ptr;
}

template <class TYPE> 
inline 
void Allocator<TYPE>::deallocate(TYPE* obj)
{ 
  m_pool_vec.insertAtBottom(obj);
}

#endif //ALLOCATOR_H
