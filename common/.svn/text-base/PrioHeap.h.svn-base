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

#ifndef PRIOHEAP_H
#define PRIOHEAP_H

#include "Vector.h"

typedef unsigned int HeapIndex;

template <class TYPE>
class PrioHeap {
public:
  // Constructors
  PrioHeap() { init(); }

  // Destructor
  //~PrioHeap();
  
  // Public Methods
  void init() { m_current_size = 0; }
  int size() const { return m_current_size; }
  void insert(const TYPE& key);
  const TYPE& peekMin() const;
  const TYPE& peekElement(int index) const;
  TYPE extractMin();
  void print(ostream& out) const;
private:
  // Private Methods
  bool verifyHeap() const;
  bool verifyHeap(HeapIndex index) const;
  void heapify();

  // Private copy constructor and assignment operator
  PrioHeap(const PrioHeap& obj);
  PrioHeap<TYPE>& operator=(const PrioHeap& obj);
  
  // Data Members (m_ prefix)
  Vector<TYPE> m_heap;
  HeapIndex m_current_size;
};

// Output operator declaration
template <class TYPE> 
ostream& operator<<(ostream& out, const PrioHeap<TYPE>& obj);

// ******************* Helper Functions *******************
inline
HeapIndex get_parent(HeapIndex i)
{
  //  return (i/2);
  return (i>>1);
}

inline
HeapIndex get_right(HeapIndex i)
{
  //  return (2*i) + 1;
  return (i<<1) | 1;
}

inline
HeapIndex get_left(HeapIndex i)
{
  //  return (2*i);
  return (i<<1);
}

template <class TYPE> 
void prio_heap_swap(TYPE& n1, TYPE& n2)
{
  TYPE temp = n1;
  n1 = n2;
  n2 = temp;
}

// ******************* Definitions *******************

template <class TYPE> 
void PrioHeap<TYPE>::insert(const TYPE& key)
{
  int i;
  // grow the vector size
  m_current_size++;
  m_heap.setSize(m_current_size+1);

  if(m_current_size == 1){      // HACK: need to initialize index 0 to avoid purify UMCs
    m_heap[0] = key;
  }

  i = m_current_size;
  while ((i > 1) && (node_less_then_eq(key, m_heap[get_parent(i)]))) {
    m_heap[i] = m_heap[get_parent(i)];
    i = get_parent(i);
  }
  m_heap[i] = key;
  //  assert(verifyHeap());
}

template <class TYPE> 
const TYPE& PrioHeap<TYPE>::peekMin() const
{
  assert(size() > 0);
  return m_heap[1]; // 1, not 0, is the first element
}

template <class TYPE> 
const TYPE& PrioHeap<TYPE>::peekElement(int index) const
{
  assert(size() > 0);
  return m_heap[index];
}

template <class TYPE> 
TYPE PrioHeap<TYPE>::extractMin()
{
  //  TYPE temp;
  assert(size() > 0);
  TYPE temp = m_heap[1]; // 1, not 0, is the first element
  m_heap[1] = m_heap[m_current_size];
  m_current_size--;
  heapify();
  return temp;
}

template <class TYPE> 
bool PrioHeap<TYPE>::verifyHeap() const
{
  return verifyHeap(1);
}

template <class TYPE> 
bool PrioHeap<TYPE>::verifyHeap(HeapIndex index) const
{
  // Recursively verify that each node is <= its parent
  if(index > m_current_size) {
    return true;
  } else if (index == 1) {
    return 
      verifyHeap(get_right(index)) &&
      verifyHeap(get_left(index));
  } else if (node_less_then_eq(m_heap[get_parent(index)], m_heap[index])) {
    return 
      verifyHeap(get_right(index)) &&
      verifyHeap(get_left(index));
  } else {
    // Heap property violation
    return false;
  }
}

template <class TYPE> 
void PrioHeap<TYPE>::heapify()
{
  HeapIndex current_node = 1;
  HeapIndex left, right, smallest;
  //  HeapIndex size = m_current_size;

  while(true) {
    left = get_left(current_node);
    right = get_right(current_node);

    // Find the smallest of the current node and children
    if (left <= m_current_size && node_less_then_eq(m_heap[left], m_heap[current_node])) {
      smallest = left;
    } else {
      smallest = current_node;
    }
    
    if (right <= m_current_size && node_less_then_eq(m_heap[right], m_heap[smallest])) {
      smallest = right;
    }
    
    // Check to see if we are done
    if (smallest == current_node) {
      // We are done
      break;
    } else {
      // Not done, heapify on the smallest child
      prio_heap_swap(m_heap[current_node], m_heap[smallest]);
      current_node = smallest;
    }
  }
  //  assert(verifyHeap());
}

template <class TYPE> 
void PrioHeap<TYPE>::print(ostream& out) const
{
  Vector<TYPE> copyHeap(m_heap);

  // sort copyHeap (inefficient, but will not be done often)

  for(HeapIndex i=0;i<m_current_size; i++){
    for(HeapIndex j=0; j< m_current_size; j++){
      if(copyHeap[i].m_time < copyHeap[j].m_time){
        prio_heap_swap(copyHeap[i], copyHeap[j]);
      }
    }
  }

  out << "[PrioHeap: ";

  for(HeapIndex i=1; i<= m_current_size; i++){
    out << copyHeap[i];

    if(i != m_current_size-1){
      out << ",";
    }
    out << " ";
  }
  out << "]";
}

// Output operator definition
template <class TYPE> 
ostream& operator<<(ostream& out, const PrioHeap<TYPE>& obj)
{
  obj.print(out);
  out << flush;
  return out;
}

#endif //PRIOHEAP_H
