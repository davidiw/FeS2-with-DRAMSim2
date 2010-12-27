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

#include "param_gen.h"
#include "fileio.h"
#include "SymbolTable.h"
#include "Var.h"
#include "Enum.h"

void write_param_h_file(string path)
{
  // Create Param.h
  string code;

  const Vector<Symbol*>& sym_vec = g_sym_table.getAllSymbols();
  int size = sym_vec.size();

  code += "// Auto generated C++ code started by " + string(__FILE__) + ":" + int_to_string(__LINE__) + "\n\n";
  code += "#ifndef PARAM_H\n";
  code += "#define PARAM_H\n";
  code += "\n";
  
  code += "#include \"Global.h\"\n";
  for(int i=0; i<size; i++) {
    Var* var = dynamic_cast<Var*>(sym_vec[i]);
    if (var != NULL) {
      if (var->existPair("parameter")) {
        Type* type = var->getType();
        if (type != NULL && !type->isPrimitive()) {
          code += "#include \"" + type->cIdent() + ".h\"\n";
        }
      }
    }
  }
  
  code += "\n";
  
  code += "// global parameter object\n";
  code += "class Param;\n";
  code += "extern Param* g_param_ptr;\n";
  code += "\n";

  code += "class Param {\n";
  code += "public:\n";
  
  code += "  Param();\n";
  code += "  void printConfig(ostream& out) const;\n";
  code += "  void printOptions(ostream& out) const;\n";
  code += "  void setParam(const string& param, const string& value);\n";
  code += "  void freeze() { m_frozen = true; }\n";
  code += "  void checkFreeze() const;\n";
  code += "\n";
  
  code += "  // accessors\n";
  for(int i=0; i<size; i++) { 
    Var* var = dynamic_cast<Var*>(sym_vec[i]);
    if (var != NULL) {
      if (var->existPair("parameter")) {
        code += "  const " + var->getType()->toString() + "& " + var->getIdent() + "() const { assert(this != NULL); return m_" + var->getIdent() + "; }\n";
      }
    }
  }

  code += "\n";
  code += "  // mutators\n";
  for(int i=0; i<size; i++) { 
    Var* var = dynamic_cast<Var*>(sym_vec[i]);
    if (var != NULL) {
      if (var->existPair("parameter")) {
        code += "  void set_" + var->getIdent() + "(const " + var->getType()->toString() + "& value) { assert(this != NULL); checkFreeze(); m_" + var->getIdent() + " = value; }\n";
      }
    }
  }

  code += "private:\n";
  code += "  bool m_frozen;\n";

  for(int i=0; i<size; i++) { 
    Var* var = dynamic_cast<Var*>(sym_vec[i]);
    if (var != NULL) {
      if (var->existPair("parameter")) {
        code += "  " + var->getType()->toString() + " m_" + var->getIdent() + ";\n";
      }
    }
  }
  
  code += "};\n";
  code += "\n";

  code += "#endif // PARAM_H\n";

  // Write file
  conditionally_write_file(path + "/Param.h", code);
}

void write_param_C_file(string path)
{
  // Create Param.C
  string code;

  const Vector<Symbol*>& sym_vec = g_sym_table.getAllSymbols();
  int size = sym_vec.size();

  code += "// Auto generated C++ code started by " + string(__FILE__) + ":" + int_to_string(__LINE__) + "\n\n";
  
  code += "#include \"Param.h\"\n";
  code += "#include \"Types.h\"\n";
  code += "#include \"slicc_util.h\"\n";
  code += "\n";
  code += "Param* g_param_ptr = NULL;\n";
  code += "\n";

  // Init
  code += "Param::Param()\n";
  code += "{\n";
  code += "  m_frozen = false;\n";

  for(int i=0; i<size; i++) { 
    Var* var = dynamic_cast<Var*>(sym_vec[i]);
    if (var != NULL) {
      if (var->existPair("parameter")) {
        code += "  m_" + var->getIdent() + " = " + var->lookupPair("default") + ";\n";
      }
    }
  }
  code += "}\n";
  code += "\n";

  // checkFreeze
  code += "void Param::checkFreeze() const\n";
  code += "{\n";
  code += "  if (m_frozen) {\n";
  code += "    ERROR_MSG(\"Can't change parameter after initialization\");\n";
  code += "  }\n";
  code += "}\n";

  // printConfig
  code += "void Param::printConfig(ostream& out) const\n";
  code += "{\n";
  code += "  assert(this != NULL);\n";
  code += "  out << endl;\n";
  code += "  out << heading(\"Ruby Parameters\");\n";
  for(int i=0; i<size; i++) { 
    Var* var = dynamic_cast<Var*>(sym_vec[i]);
    if (var != NULL) {
      if (var->existPair("parameter")) {
        code += "  out << left << setw(35) << \"" + var->getIdent() + ": \" << setw(20) << boolalpha << m_" + var->getIdent() + ";\n";
        if (var->existPair("desc")) {
          code += "  out << \"  ; " + var->lookupPair("desc") + "\" << endl;\n";
        } else {
          code += "  out << endl;\n";
        }

      }
    }
  }
  code += "}\n";
  code += "\n";

  // printOptions
  code += "void Param::printOptions(ostream& out) const\n";
  code += "{\n";
  code += "  assert(this != NULL);\n";
  code += "  out << \"Runtime parameter options: \" << endl;\n";
  code += "  out << endl;\n";
  code += "  out << \"  \" << left << setw(35) << \"parameter\" << setw(15) << \"type\" << setw(20) << \"default\" << \"description\" << endl;\n";
  code += "  out << endl;\n";
  for(int i=0; i<size; i++) { 
    Var* var = dynamic_cast<Var*>(sym_vec[i]);
    if (var != NULL) {
      if (var->existPair("parameter")) {
        code += "\n";
        if (var->existPair("shorthand")) {
          code += "  out << \"" +  var->lookupPair("shorthand") + " \";\n";
        } else {
          code += "  out << \"  \";\n";
        }

        code += "  out << left << setw(35) << \"" + var->getIdent() + "\";\n";
        code += "  out << setw(15) << \"" + var->getType()->toString() + "\";\n";

        code += "  out << setw(20) << boolalpha << m_" + var->getIdent() + ";\n";
        if (var->existPair("desc")) {
          code += "  out << \"" + var->lookupPair("desc") + "\" << endl;\n";
        } else {
          code += "  out << endl;\n";
        }

        Enum* enum_ptr = dynamic_cast<Enum*>(var->getType()); // Ok
        if (enum_ptr != NULL) {
          code += "  out << \"    " + enum_ptr->toString() + ":\";\n";
          const Vector<string>& enum_vec = enum_ptr->enumList();
          for (int i=0; i<enum_vec.size(); i++) {
            code += "  out << \"  " + enum_vec[i] + "\";\n";
          }
          code += "  out << endl;\n";
        }
      }
    }
  }
  code += "}\n";
  code += "\n";


  // setParam
  code += "void Param::setParam(const string& param, const string& value)\n";
  code += "{\n";
  code += "  assert(this != NULL);\n";
  code += "  checkFreeze();\n";

  code += "  if (false) {\n";
  code += "    // Do nothing\n";

  for(int i=0; i<size; i++) { 
    Var* var = dynamic_cast<Var*>(sym_vec[i]);
    if (var != NULL) {
      if (var->existPair("parameter")) {
        if (var->existPair("shorthand")) {
          code += "  } else if ((param == \"" + var->getIdent() + "\") || (param == \"" + var->lookupPair("shorthand") + "\")) {\n";
        } else {
          code += "  } else if (param == \"" + var->getIdent() + "\") {\n";
        }
        if (var->getType()->toString() == "string") {
          code += "    m_" + var->getIdent() + " = value;\n";
        } else {
          code += "    m_" + var->getIdent() + " = string_to_" + var->getType()->toString() + "(value);\n";
        }
        code += "    cout << \"  Parameter '" + var->getIdent() + "' set to: \" << boolalpha << m_" + var->getIdent() + " << endl;\n";
      }
    }
  }

  code += "  } else {\n";
  code += "    // error case\n";
  code += "    ERROR_MSG(\"Invalid configuration parameter: \" + param);\n";
  code += "  }\n";
  
  code += "}\n";
  code += "\n";

  // Write file
  conditionally_write_file(path + "/Param.C", code);
}
