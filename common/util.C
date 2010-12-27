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

#include <assert.h>
#include "util.h"

// Split a string into a head and tail strings on the specified
// character.  Return the head and the string passed in is modified by
// removing the head, leaving just the tail.
string string_split(string& str, char split_character)
{
  string head = "";
  string tail = "";

  uint counter = 0;
  while (counter < str.size()) {
    if (str[counter] == split_character) {
      counter++;
      break;
    } else {
      head += str[counter];
    }
    counter++;
  }
  
  while (counter < str.size()) {
    tail += str[counter];
    counter++;
  }
  str = tail;
  return head;
}

string str(bool value)
{
  if (value) {
    return "true";
  } else {
    return "false";
  }
}

//string str(int n, bool disp_hex /* = false */, bool zero_fill /* = false*/, int width /* = 0 */)
//{
//  return str(int64(n), disp_hex, zero_fill, width);
//}

//string str(int64 n, bool disp_hex /* = false */, bool zero_fill /* = false*/, int width /* = 0 */)
//{
//  ostringstream sstr;
//  
//  if (disp_hex) {
//    if (zero_fill) {
//    sstr << setw(width) << setfill('0') << hex << n << dec;
//    } else {
//      sstr << hex << n << dec;
//    }
//  } else {
//    if (zero_fill) {
//      sstr << setw(width) << setfill('0') << n;
//    } else {
//      sstr << n;
//    }
//  }
//  
//  string str = sstr.str();
//  return str;
//}
//
//string str(uint64 n, bool disp_hex /* = false */, bool zero_fill /* = false*/, int width /* = 0 */)
//{
//  ostringstream sstr;
//  
//  if (disp_hex) {
//    if (zero_fill) {
//    sstr << setw(width) << setfill('0') << hex << n << dec;
//    } else {
//      sstr << hex << n << dec;
//    }
//  } else {
//    if (zero_fill) {
//      sstr << setw(width) << setfill('0') << n;
//    } else {
//      sstr << n;
//    }
//  }
//  
//  string str = sstr.str();
//  return str;
//}


string heading(const string& str)
{
  string output;
  output += str;
  int length = str.length();
  output += "\n";
  for (int i=0; i < length; i++) {
    output += "-";
  }
  output += "\n";
  return output;
}

string readfile(const string& filename)
{
  ifstream input_file(filename.c_str());
  string output;
  
  char c = input_file.get();
  while (input_file) {
    output += c;
    c = input_file.get();
  }
  return output;
}

bool char_is_whitespace(char c){
  if ((c == ' ') || (c == '\t') || (c == '\n')){
    return true;
  }
  return false;
}

string strip_whitespace_from_string(string s){
  string ret = "";
  for (unsigned int i = 0; i < s.length(); i++){
    if (!char_is_whitespace(s[i])){
      ret += s[i];
    }
  }
  return ret;
}

bool string_is_whitespace(string s){
  for (unsigned int i = 0; i < s.length(); i++){
    if (!char_is_whitespace(s[i])){
      return false;
    }
  }
  return true;
}

string bool_to_string(bool value)
{
  if (value) {
    return "true";
  } else {
    return "false";
  }
}

string int_to_string(int n, bool zero_fill, int width)
{
  ostringstream sstr;
  if(zero_fill) {
    sstr << setw(width) << setfill('0') << n;
  } else {
    sstr << n;
  }
  string str = sstr.str();
  return str;
}

// Log functions
int log_int(long long n)
{
  assert(n > 0);
  int counter = 0;
  while (n >= 2) {
    counter++;
    n = n>>(long long)(1);
  }
  return counter;
}

bool is_power_of_2(long long n)
{
  return (n == ((long long)(1) << log_int(n)));
}

