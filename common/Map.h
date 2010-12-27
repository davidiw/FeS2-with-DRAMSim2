// Based on a file from Multifacet GEMS (General Execution-driven
// Multiprocessor Simulator), which was released under the GPL

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

#ifndef MAP_H
#define MAP_H

#include "Vector.h"

namespace __gnu_cxx {
  template<>
  struct hash<string>
  {
    size_t operator()(const string& s) const { return hash<char*>()(s.c_str()); }
  };
  template<>
  struct hash<unsigned long long>
  {
    size_t operator()(const unsigned long long& __x) const { return __x; }
  };
}

template <class KEY_TYPE, class VALUE_TYPE> 
class Map
{
public:
  Map() { /* empty */ }
  ~Map() { /* empty */ }
  
  void add(const KEY_TYPE& key, const VALUE_TYPE& value);
  bool exist(const KEY_TYPE& key) const;
  int size() const { return m_map.size(); }
  void erase(const KEY_TYPE& key) { assert(exist(key)); m_map.erase(key); }
  Vector<KEY_TYPE> keys() const;
  Vector<VALUE_TYPE> values() const;
  void deleteKeys();
  void deleteValues();
  VALUE_TYPE& lookup(const KEY_TYPE& key) const; 
  void clear() { m_map.clear(); }
  void print(ostream& out) const;

  // Synonyms
  void remove(const KEY_TYPE& key) { erase(key); } 
  void deallocate(const KEY_TYPE& key) { erase(key); } 
  void allocate(const KEY_TYPE& key) { add(key, VALUE_TYPE()); }
  void insert(const KEY_TYPE& key, const VALUE_TYPE& value) { add(key, value); }

  // Use default copy constructor and assignment operator
private:
  // Data members

  // m_map is declared mutable because some methods from the STL "map"
  // class that should be const are not.  Thus we define this as
  // mutable so we can still have conceptually const accessors.
  mutable hash_map<KEY_TYPE, VALUE_TYPE> m_map;
};

template <class KEY_TYPE, class VALUE_TYPE>
extern inline ostream& operator<<(ostream& out, const Map<KEY_TYPE, VALUE_TYPE>& obj) 
{
  obj.print(out);
  out << flush;
  return out;
}

template <class KEY_TYPE, class VALUE_TYPE> 
extern inline string str(const Map<KEY_TYPE, VALUE_TYPE>& obj)
{
  ostringstream sstr;
  sstr << obj;
  string str = sstr.str();
  return str;
}

// *********************

template <class KEY_TYPE, class VALUE_TYPE> 
void Map<KEY_TYPE, VALUE_TYPE>::add(const KEY_TYPE& key, const VALUE_TYPE& value)
{ 
  // Update or add a new key/value pair
  m_map[key] = value;
}

template <class KEY_TYPE, class VALUE_TYPE> 
bool Map<KEY_TYPE, VALUE_TYPE>::exist(const KEY_TYPE& key) const
{
  return (m_map.count(key) != 0);
}

template <class KEY_TYPE, class VALUE_TYPE> 
VALUE_TYPE& Map<KEY_TYPE, VALUE_TYPE>::lookup(const KEY_TYPE& key) const
{
//  assert_msg(exist(key), key);
  assert(exist(key));
  return m_map[key];
}

template <class KEY_TYPE, class VALUE_TYPE> 
Vector<KEY_TYPE> Map<KEY_TYPE, VALUE_TYPE>::keys() const
{
  Vector<KEY_TYPE> keys;
  typename hash_map<KEY_TYPE, VALUE_TYPE>::const_iterator iter;
  for (iter = m_map.begin(); iter != m_map.end(); iter++) {
    keys.insertAtBottom((*iter).first);
  }
  return keys;
}

template <class KEY_TYPE, class VALUE_TYPE> 
Vector<VALUE_TYPE> Map<KEY_TYPE, VALUE_TYPE>::values() const
{
  Vector<VALUE_TYPE> values;
  typename hash_map<KEY_TYPE, VALUE_TYPE>::const_iterator iter;
  pair<KEY_TYPE, VALUE_TYPE> p;
  
  for (iter = m_map.begin(); iter != m_map.end(); iter++) {
    p = *iter;
    keys.insertAtBottom(p.second);
  }
  return values;
}

template <class KEY_TYPE, class VALUE_TYPE> 
void Map<KEY_TYPE, VALUE_TYPE>::deleteKeys()
{
  typename hash_map<KEY_TYPE, VALUE_TYPE>::const_iterator iter;
  pair<KEY_TYPE, VALUE_TYPE> p;
  
  for (iter = m_map.begin(); iter != m_map.end(); iter++) {
    p = *iter;
    delete p.first;
  }
}

template <class KEY_TYPE, class VALUE_TYPE> 
void Map<KEY_TYPE, VALUE_TYPE>::deleteValues()
{
  typename hash_map<KEY_TYPE, VALUE_TYPE>::const_iterator iter;
  pair<KEY_TYPE, VALUE_TYPE> p;
  
  for (iter = m_map.begin(); iter != m_map.end(); iter++) {
    p = *iter;
    delete p.second;
  }
}

template <class KEY_TYPE, class VALUE_TYPE> 
void Map<KEY_TYPE, VALUE_TYPE>::print(ostream& out) const
{
  typename hash_map<KEY_TYPE, VALUE_TYPE>::const_iterator iter;
  pair<KEY_TYPE, VALUE_TYPE> p;
  
  out << "[";
  for (iter = m_map.begin(); iter != m_map.end(); iter++) {
    // unparse each basic block
    p = *iter;
    out << " " << p.first << "=" << p.second;
  }
  out << " ]";
}

#endif //MAP_H
