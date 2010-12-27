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

#ifndef REFCNT_H
#define REFCNT_H

template <class TYPE>
class RefCnt {
public:
  // Constructors
  RefCnt();
  RefCnt(const TYPE& data);

  // Destructor
  ~RefCnt();
  
  // Public Methods
  const TYPE* ref() const { return m_data_ptr; }
  TYPE* ref() { return m_data_ptr; }
  void freeRef();
  void print(ostream& out) const;

  // Public copy constructor and assignment operator
  RefCnt(const RefCnt& obj);
  RefCnt& operator=(const RefCnt& obj);

private:
  // Private Methods

  // Data Members (m_ prefix)
  TYPE* m_data_ptr;
};

// Output operator declaration
template <class TYPE> 
inline 
ostream& operator<<(ostream& out, const RefCnt<TYPE>& obj);

// ******************* Definitions *******************

// Constructors
template <class TYPE> 
inline
RefCnt<TYPE>::RefCnt()
{
  m_data_ptr = NULL;
}

template <class TYPE> 
inline
RefCnt<TYPE>::RefCnt(const TYPE& data) 
{
  m_data_ptr = data.clone(); 
  m_data_ptr->setRefCnt(1);
}

template <class TYPE> 
inline
RefCnt<TYPE>::~RefCnt() 
{ 
  freeRef();
}

template <class TYPE> 
inline
void RefCnt<TYPE>::freeRef() 
{ 
  if (m_data_ptr != NULL) {
    m_data_ptr->decRefCnt();
    if (m_data_ptr->getRefCnt() == 0) {
      m_data_ptr->destroy();
    }
    m_data_ptr = NULL; 
  }
}

template <class TYPE> 
inline
void RefCnt<TYPE>::print(ostream& out) const 
{
  if (m_data_ptr == NULL) {
    out << "[RefCnt: Null]";
  } else {
    out << "[RefCnt: ";
    m_data_ptr->print(out);
    out << "]";
  }
}

// Copy constructor
template <class TYPE> 
inline
RefCnt<TYPE>::RefCnt(const RefCnt<TYPE>& obj) 
{ 
  //  m_data_ptr = obj.m_data_ptr->clone();  
  m_data_ptr = obj.m_data_ptr;

  // Increment the reference count
  if (m_data_ptr != NULL) {
    m_data_ptr->incRefCnt();
  }
}

// Assignment operator
template <class TYPE> 
inline
RefCnt<TYPE>& RefCnt<TYPE>::operator=(const RefCnt<TYPE>& obj) 
{ 
  if (this == &obj) {
    // If this is the case, do nothing
    //    assert(false);
  } else {
    freeRef();
    m_data_ptr = obj.m_data_ptr;
    if (m_data_ptr != NULL) {
      m_data_ptr->incRefCnt();
    }
  }
  return *this;
}


// Output operator definition
template <class TYPE> 
inline 
ostream& operator<<(ostream& out, const RefCnt<TYPE>& obj)
{
  obj.print(out);
  out << flush;
  return out;
}



#endif //REFCNT_H
