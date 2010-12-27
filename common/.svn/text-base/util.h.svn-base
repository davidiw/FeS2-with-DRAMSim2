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

#ifndef UTIL_H
#define UTIL_H

//#include "Global.h"
#include "std-includes.h"

string string_split(string& str, char split_character = ' ');
string str(bool value);
extern inline string str(char c) { string a; a+=c; return a; }
//string str(int n, bool disp_hex = false, bool zero_fill = false, int width = 0);
//string str(int64 n, bool disp_hex = false, bool zero_fill = false, int width = 0);
//string str(uint64 n, bool disp_hex = false, bool zero_fill = false, int width = 0);
string heading(const string& str);
string readfile(const string& filename);
bool char_is_whitespace(char c);
bool string_is_whitespace(string s);
string strip_whitespace_from_string(string s);

string bool_to_string(bool value);
string int_to_string(int n, bool zero_fill = false, int width = 0);
int log_int(long long n);
bool is_power_of_2(long long n);

// Min and Max functions (since they are extern inline, they are as fast as macros)

extern inline 
int max(int n1, int n2) 
{
  if (n1 > n2) {
    return n1;
  } else {
    return n2;
  }
}

extern inline 
int min(int n1, int n2) 
{
  if (n1 < n2) {
    return n1;
  } else {
    return n2;
  }
}

#endif //UTIL_H
