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

/*
 * RefCountable.h
 * 
 * Description: Virtual base class for things that can be reference counted
 *
 */

#ifndef REFCOUNTABLE_H
#define REFCOUNTABLE_H

#include "RefCnt.h"

class RefCountable {
public:
  // Public Methods

  RefCountable() { m_refcnt = 0; }

  // These are used by the RefCnt class to hold the reference count
  // for the object.  These really should be private and accessed by a
  // friend class, but I can't figure out how to make a template class
  // a friend.
  void incRefCnt() { m_refcnt++; }
  void decRefCnt() { m_refcnt--; }
  int getRefCnt() const { return m_refcnt; }
  void setRefCnt(int cnt) { m_refcnt = cnt; }
private:
  // Private Methods
  
  // Data Members (m_ prefix)
  int m_refcnt;  
};

#endif //REFCOUNTABLE_H
