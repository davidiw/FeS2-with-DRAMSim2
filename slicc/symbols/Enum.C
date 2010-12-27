/* This file is part of SLICC (Specification Language for Implementing
   Cache Coherence), a component of the Multifacet GEMS (General
   Execution-driven Multiprocessor Simulator) software toolset
   originally developed at the University of Wisconsin-Madison.

   SLICC was originally developed by Milo Martin with substantial
   contributions from Daniel Sorin.

   Substantial further development of Multifacet GEMS at the
   University of Wisconsin was performed by Alaa Alameldeen, Brad
   Beckmann, Ross Dickson, Pacia Harper, Milo Martin, Michael Marty,
   Carl Mauer, Kevin Moore, Manoj Plakal, Daniel Sorin, Min Xu, and
   Luke Yen.

   Additional development was performed at the University of
   Pennsylvania by Milo Martin.

   --------------------------------------------------------------------

   Copyright (C) 1999-2005 by Mark D. Hill and David A. Wood for the
   Wisconsin Multifacet Project.  Contact: gems@cs.wisc.edu
   http://www.cs.wisc.edu/gems/

   This file is based upon a pre-release version of Multifacet GEMS
   from 2004 and may also contain additional modifications and code
   Copyright (C) 2004-2007 by Milo Martin for the Penn Architecture
   and Compilers Group.  Contact: acg@lists.seas.upenn.edu
   http://www.cis.upenn.edu/acg/

   --------------------------------------------------------------------

   Multifacet GEMS is free software; you can redistribute it and/or
   modify it under the terms of version 2 of the GNU General Public
   License as published by the Free Software Foundation.  This 
   software comes with ABSOLUTELY NO WARRANTY.  For details see the 
   file LICENSE included with the distribution.
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

#include "Enum.h"
#include "fileio.h"

Enum::Enum(string id, const Location& location, 
           const Map<string, string>& pairs, 
           const StateMachine* machine_ptr)
  : Type(id, location, pairs, machine_ptr)
{
}

bool Enum::enumAdd(string id, Map<string, string>& pairs)
{
  if (enumExist(id)) {
    return false;
  } else {
    m_enum_map.add(id, true);
    m_enum_vec.insertAtBottom(id);

    // Add default
    if (!existPair("default")) {
      addPair("default", cIdent()+"_NUM");
    }

    return true;
  }
}

void Enum::writeCFiles(string path) const
{
  printEnumH(path);
  printEnumC(path);
}

void Enum::printEnumH(string path) const
{
  ostringstream out;
  int size = m_enum_vec.size();
  string type_name = cIdent();  // Identifier for the type in C

  // Header
  out << "// Auto generated C++ code started by "<<__FILE__<<":"<<__LINE__<<endl;
  out << "// " << type_name << ".h" << endl;
  out << endl;
  out << "#ifndef " << type_name << "_H" << endl;
  out << "#define " << type_name << "_H" << endl;
  out << endl;
  // Include all of the #includes needed
  out << "#include \"Global.h\"" << endl;
  out << endl;

  // Class definition
  out << "enum " << type_name << " {" << endl;

  // For each field
  for(int i = 0; i < size; i++ ) {
    string id = m_enum_vec[i];
    out << "  " << type_name << "_" << id << "," << endl;
  }
  out << "  " << type_name << "_NUM" << endl;
  out << "};" << endl;

  // Code to convert from a string to the enumeration
  out << type_name << " string_to_" << type_name << "(const string& str);" << endl;

  // Code to convert state to a string
  out << "string " << type_name << "_to_string(const " << type_name << "& obj);" << endl;

  // Trailer
  out << "ostream& operator<<(ostream& out, const " << type_name << "& obj);" << endl;
  out << endl;
  out << "#endif // " << type_name << "_H" << endl;

  // Write the file
  conditionally_write_file(path + type_name + ".h", out);
}

void Enum::printEnumC(string path) const
{
  ostringstream out;
  int size = m_enum_vec.size();
  string type_name = cIdent();  // Identifier for the type in C

  // Header
  out << "// Auto generated C++ code started by "<<__FILE__<<":"<<__LINE__<<endl;
  out << "// " << type_name << ".C" << endl;
  out << endl;
  out << "#include \"" << type_name << ".h\"" << endl;
  out << endl;

  // Code for output operator
  out << "ostream& operator<<(ostream& out, const " << type_name << "& obj)" << endl;
  out << "{" << endl;
  out << "  out << " << type_name << "_to_string(obj);" << endl;
  out << "  out << flush;" << endl;
  out << "  return out;" << endl;
  out << "}" << endl;

  // Code to convert state to a string  
  out << endl;
  out << "string " << type_name << "_to_string(const " << type_name << "& obj)" << endl;
  out << "{" << endl;
  out << "  switch(obj) {" << endl;
  
  // For each field
  for( int i = 0; i<size; i++ ) {
    out << "  case " << type_name << "_" << m_enum_vec[i] << ":" << endl;
    out << "    return \"" << m_enum_vec[i] << "\";" << endl;
  }

  // Trailer
  out << "  default:" << endl;
  out << "    ERROR_MSG(\"Invalid range for type " << type_name << "\");" << endl;
  out << "  }" << endl;
  out << "}" << endl;

  // Code to convert from a string to the enumeration
  out << endl;
  out << type_name << " string_to_" << type_name << "(const string& str)" << endl;
  out << "{" << endl;
  out << "  if (false) {" << endl;

  // For each field
  for( int i = 0; i<size; i++ ) {
    out << "  } else if (str == \"" << m_enum_vec[i] << "\") {" << endl;
    out << "    return " << type_name << "_" << m_enum_vec[i] << ";" << endl;
  }

  out << "  } else {" << endl;
  out << "    WARN_EXPR(str);" << endl;
  out << "    ERROR_MSG(\"Invalid string conversion for type " << type_name << "\");" << endl;
  out << "  }" << endl;
  out << "}" << endl;

  // Write the file
  conditionally_write_file(path + type_name + ".C", out);
}
