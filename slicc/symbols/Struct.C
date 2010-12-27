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

#include "Struct.h"
#include "fileio.h"
#include "Map.h"

Struct::Struct(string id, const Location& location, 
               const Map<string, string>& pairs, 
               const StateMachine* machine_ptr)
  : Type(id, location, pairs, machine_ptr)
{
}

// Return false on error
bool Struct::dataMemberAdd(string id, Type* type_ptr, Map<string, string>& pairs)
{
  if (dataMemberExist(id)) {
    return false; // Error
  } else {
    m_data_member_map.add(id, type_ptr);
    m_data_member_ident_vec.insertAtBottom(id);
    m_data_member_type_vec.insertAtBottom(type_ptr);
    m_data_member_pairs_vec.insertAtBottom(pairs);
  }

  return true;
}

void Struct::writeCFiles(string path) const
{
  printTypeH(path);
  printTypeC(path);
}

void Struct::printTypeH(string path) const
{
  ostringstream out;
  int size = m_data_member_type_vec.size();
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
  out << "#include \"Allocator.h\"" << endl;
  for (int i=0; i < size; i++) {
    Type* type = m_data_member_type_vec[i];
    if (!type->isPrimitive()) {
      out << "#include \"" << type->cIdent() << ".h" << "\"" << endl;
    }
  }
  string interface = "";
  if(existPair("interface")) {
    interface = lookupPair("interface");
    out << "#include \"" << interface << ".h\"" << endl;
  }

  // Class definition
  out << "class " << type_name;

  if(interface != "") {
    out << " : public " << interface ;
  }

  out << " {" << endl;
  out << "public:" << endl;

  // ******** Default constructor ********

  out << "  " << type_name << "() " << endl;

  // Call superclass constructor
  if (interface != "") {
      out << "    : " << interface << "()" << endl;
  }

  out << "  {" << endl;
  for (int i=0; i < size; i++) {
    Type* type_ptr = m_data_member_type_vec[i];
    string id = m_data_member_ident_vec[i];
    if (m_data_member_pairs_vec[i].exist("default")) {
      // look for default value
      string default_value = m_data_member_pairs_vec[i].lookup("default");
      out << "    m_" << id << " = " << default_value << "; // default for this field" << endl;
    } else if (type_ptr->hasDefault()) {
      // Look for the type default
      string default_value = type_ptr->getDefault();
      out << "    m_" << id << " = " << default_value << "; // default value of " << type_ptr->cIdent() << endl;
    } else {
      out << "    // m_" << id << " has no default" << endl;
    }
  }
  out << "  }" << endl;
  
  // ******** Default destructor ********
  out << "  ";
  out << "~" << type_name << "() { };" << endl;
  
  // ******** Full init constructor ********
  out << "  " << type_name << "(";
  
  for (int i=0; i < size; i++) {
    if (i != 0) {
      out << ", ";
      }
    Type* type = m_data_member_type_vec[i];
    string id = m_data_member_ident_vec[i];
    out << "const " << type->cIdent() << "& local_" << id;
  }
  out << ")" << endl;
  
  // Call superclass constructor
  if (interface != "") {
    out << "    : " << interface << "()" << endl;
  }

  out << "  {" << endl;
  for (int i=0; i < size; i++) {
    Type* type_ptr = m_data_member_type_vec[i];
    string id = m_data_member_ident_vec[i];
    out << "    m_" << id << " = local_" << id << ";" << endl;
  }
  out << "  }" << endl;
  out << endl;

  // Virtual constructors and memory management functions
  out << "  // 'Virtual' constructor and other memory management functions" << endl;
  out << "  " << type_name << "* construct() const { return new " << type_name << "(); }" << endl;
  out << "  void reset() { (*this) = " << type_name << "(); }" << endl;
  out << "  " << type_name << "* clone() const { checkAllocator(); return s_allocator_ptr->allocate(*this); }" << endl;
  out << "  void destroy() { checkAllocator(); s_allocator_ptr->deallocate(this); }" << endl;
  out << "  static Allocator<" << type_name << ">* s_allocator_ptr;" << endl;
  out << "  static void checkAllocator() { if (s_allocator_ptr == NULL) { s_allocator_ptr = new Allocator<" << type_name << ">; }}" << endl;
  out << endl;

  // const Get methods for each field
  out << "  // Const accessors methods for each field" << endl;
  for (int i=0; i < size; i++) {
    Type* type_ptr = m_data_member_type_vec[i];
    string type = type_ptr->cIdent();
    string id = m_data_member_ident_vec[i];
    out << "  const " << type << "& get" << id 
        << "() const { return m_" << id << "; }" << endl;
  }
  out << endl;
  
  // Non-const Get methods for each field
  out << "  // Non const Accessors methods for each field" << endl;
  for (int i=0; i < size; i++) {
    Type* type_ptr = m_data_member_type_vec[i];
    string type = type_ptr->cIdent();
    string id = m_data_member_ident_vec[i];
    out << "  " << type << "& get" << id 
        << "() { return m_" << id << "; }" << endl;
  }
  out << endl;
  
  // Set methods for each field
  out << "  // Mutator methods for each field" << endl;
  for (int i=0; i < size; i++) {
    Type* type_ptr = m_data_member_type_vec[i];
    string type = type_ptr->cIdent();
    string id = m_data_member_ident_vec[i];
    out << "  void set" << id << "(const " << type << "& local_" 
        << id << ") { m_" << id << " = local_" << id << "; }" << endl;
  }
  
  out << endl;
  
  out << "  void print(ostream& out) const;" << endl;
  out << "//private:" << endl;

  // Data members for each field
  for (int i=0; i < size; i++) {
    Type* type = m_data_member_type_vec[i];
    string id = m_data_member_ident_vec[i];
    out << "  " << type->cIdent() << " m_" << id << ";";

    if (m_data_member_pairs_vec[i].exist("desc")) {
      string desc = m_data_member_pairs_vec[i].lookup("desc");
      out << " // " << desc;
    }
    out << endl;
  }

  out << "};" << endl;  // End class 

  out << "// Output operator declaration" << endl;
  out << "ostream& operator<<(ostream& out, const " << type_name << "& obj);" << endl;
  out << endl;
  out << "// Output operator definition" << endl;
  out << "extern inline" << endl;
  out << "ostream& operator<<(ostream& out, const " << type_name << "& obj)" << endl;
  out << "{" << endl;
  out << "  obj.print(out);" << endl;
  out << "  out << flush;" << endl;
  out << "  return out;" << endl;
  out << "}" << endl;
  out << endl;
  out << "#endif // " << type_name << "_H" << endl;
  
  // Write it out
  conditionally_write_file(path + type_name + ".h", out);
}

void Struct::printTypeC(string path) const
{
  ostringstream out;
  int size = m_data_member_type_vec.size();
  string type_name = cIdent();  // Identifier for the type in C

  // Header
  out << "// Auto generated C++ code started by "<<__FILE__<<":"<<__LINE__<<endl;
  out << "// " << type_name << ".C" << endl;
  out << endl;
  out << "#include \"" << type_name << ".h\"" << endl;
  out << endl;
  out << "Allocator<" << type_name << ">* " << type_name << "::s_allocator_ptr = NULL;" << endl;
  out << "void " << type_name << "::print(ostream& out) const" << endl;
  out << "{" << endl;
  out << "  out << \"[" << type_name << ": \";" << endl;

  // For each field
  for (int i=0; i < size; i++) {
    string id = m_data_member_ident_vec[i];
    out << "  out << \"" << id << "=\" << m_" << id << " << \" \";" << endl;
  }

  // Trailer
  out << "  out << \"]\";" << endl;
  out << "}" << endl;

  // Write it out
  conditionally_write_file(path + type_name + ".C", out);
}
