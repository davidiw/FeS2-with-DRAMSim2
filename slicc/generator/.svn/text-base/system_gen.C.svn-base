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

#include "system_gen.h"
#include "fileio.h"
#include "SymbolTable.h"
#include "Var.h"
#include "Type.h"
#include "StateMachine.h"

void write_system_h(string path)
{
  string code;

  const Vector<Symbol*>& sym_vec = g_sym_table.getAllSymbols();
  int size = sym_vec.size();

  code += "// Auto generated C++ code started by " + string(__FILE__) + ":" + int_to_string(__LINE__) + "\n\n";
  code += "#ifndef SYSTEM_H\n";
  code += "#define SYSTEM_H\n";
  code += "\n";
  
  // Includes
  code += "#include \"Global.h\"\n";
  code += "#include \"Vector.h\"\n";
  code += "#include \"NodeID.h\"\n";
  code += "\n";

  // Class forward declarations
  code += "class Sequencer;\n";
  code += "class CacheRecorder;\n";
  for (int i=0; i<size; i++) {
    Var* var = dynamic_cast<Var*>(sym_vec[i]);
    if (var != NULL) {
      if (var->getMachine() == NULL) {
        Type* type = var->getType();
        assert(type != NULL);
        if ((!var->existPair("parameter")) && (!type->isPrimitive())) {
          code += "class " + type->cIdent() + "; // " + var->cIdent() + " \n";
        }
      }
    }
  }
  code += "\n";

  for (int i=0; i<size; i++) {
    StateMachine* machine_ptr = dynamic_cast<StateMachine*>(sym_vec[i]);
    if (machine_ptr != NULL) {
      code += "class " + machine_ptr->cIdent() + "_Controller;\n";
    }
  }
  code += "\n";

  code += "class System {\n";
  code += "public:\n";
  code += "  System();\n";
  code += "  ~System();\n";
  code += "  \n";
  code += "  // Public Methods\n";
  code += "  Sequencer* getSequencer(NodeID node) { assert(this != NULL); assert(m_sequencers_vec[node] != NULL); return m_sequencers_vec[node]; }\n";
  for (int i=0; i<size; i++) {
    Var* var = dynamic_cast<Var*>(sym_vec[i]);
    if (var != NULL) {
      if (var->getMachine() == NULL) {
        Type* type = var->getType();
        assert(type != NULL);
        if (!var->existPair("parameter")) {
          string var_name = "m_" + var->cIdent() + "_ptr";
          string method_name = "get" + var->cIdent();
          method_name[3] = toupper(method_name[3]);
          code += "  " + type->cIdent() + "* " + method_name + "() { assert(this != NULL); assert(" + var_name + " != NULL); return " + var_name + "; }\n";
        }
      }
    }
  }

  code += "  \n";
  code += "  void printConfig(ostream& out) const;\n";
  code += "  void printStats(ostream& out) const;\n";
  code += "  void clearStats() const;\n";
  code += "  void recordCacheContents(CacheRecorder& tr) const;\n";
  code += "  \n";
  code += "private:\n";

  code += "  // Private copy constructor and assignment operator\n";
  code += "  System(const System& obj);\n";
  code += "  System& operator=(const System& obj);\n";
  code += "  \n";

  code += "  // Data Members (m_ prefix)\n";
  for (int i=0; i<size; i++) {
    Var* var = dynamic_cast<Var*>(sym_vec[i]);
    if (var != NULL) {
      if (var->getMachine() == NULL) {
        Type* type = var->getType();
        assert(type != NULL);
        if (!var->existPair("parameter")) {
          string var_name = "m_" + var->cIdent() + "_ptr";
          code += "  " + type->cIdent() + "* " + var_name + ";\n";
        }
      }
    }
  }

  code += "  Vector<Sequencer*> m_sequencers_vec;\n";

  for (int i=0; i<size; i++) {
    StateMachine* machine_ptr = dynamic_cast<StateMachine*>(sym_vec[i]);
    if (machine_ptr != NULL) {
      code += "  Vector<" + machine_ptr->cIdent() + "_Controller*> m_" + machine_ptr->cIdent() + "_Controller_vec;\n";
    }
  }
  code += "};\n";
  code += "\n";

  code += "#endif // SYSTEM_H\n";

  // Write file
  conditionally_write_file(path + "/System.h", code);
}

void write_system_C(string path)
{
  string code;

  const Vector<Symbol*>& sym_vec = g_sym_table.getAllSymbols();
  int size = sym_vec.size();

  code += "// Auto generated C++ code started by " + string(__FILE__) + ":" + int_to_string(__LINE__) + "\n\n";

  // Includes
  code += "#include \"System.h\"\n";
  code += "#include \"Param.h\"\n";
  code += "#include \"Sequencer.h\"\n";
  code += "#include \"CacheRecorder.h\"\n";
  code += "#include \"protocol_name.h\"\n";

  for (int i=0; i<size; i++) {
    Var* var = dynamic_cast<Var*>(sym_vec[i]);
    if (var != NULL) {
      if (var->getMachine() == NULL) {
        Type* type = var->getType();
        assert(type != NULL);
        if ((!var->existPair("parameter")) && (!type->isPrimitive())) {
          code += "#include \"" + type->cIdent() + ".h\"\n";
        }
      }
    }
  }
  code += "\n";

  for (int i=0; i<size; i++) {
    StateMachine* machine_ptr = dynamic_cast<StateMachine*>(sym_vec[i]);
    if (machine_ptr != NULL) {
      code += "#include \"" + machine_ptr->cIdent() + "_Controller.h\"\n";
    }
  }
  code += "\n";

  code += "System::System()\n";
  code += "{\n";
  for (int i=0; i<size; i++) {
    Var* var = dynamic_cast<Var*>(sym_vec[i]);
    if (var != NULL) {
      if (var->getMachine() == NULL) {
        Type* type = var->getType();
        assert(type != NULL);
        if (!var->existPair("parameter")) {
          code += "  m_" + var->cIdent() + "_ptr = " + type->cIdent() + "::create();\n";
        }
      }
    }
  }
  code += "\n";

  for (int i=0; i<size; i++) {
    StateMachine* machine_ptr = dynamic_cast<StateMachine*>(sym_vec[i]);
    if (machine_ptr != NULL) {
      code += "  m_" + machine_ptr->cIdent() + "_Controller_vec.setSize(g_param_ptr->NUM_NODES());\n";
      code += "  for (int i=0; i<m_" + machine_ptr->cIdent() + "_Controller_vec.size(); i++) {\n";
      code += "    m_" + machine_ptr->cIdent() + "_Controller_vec[i] = new " + machine_ptr->cIdent() + "_Controller(i, m_network_ptr);\n";
      code += "  }\n";
      code += "\n";
    }
  }

  code += "  m_sequencers_vec.setSize(g_param_ptr->NUM_NODES());\n";
  code += "  for(int i=0; i<m_sequencers_vec.size(); i++) {\n";
  code += "    m_sequencers_vec[i] = new Sequencer(i,\n";
  code += "                                        m_L1Cache_Controller_vec[i]->m_L1Cache_requestQueue_ptr,\n";
  code += "                                        m_L1Cache_Controller_vec[i]->m_L1Cache_L1DcacheMemory_ptr,\n";
  code += "                                        m_L1Cache_Controller_vec[i]->m_L1Cache_L1IcacheMemory_ptr);\n";
  code += "  }\n";
  code += "}\n";
  code += "\n";

  // Destructor
  code += "System::~System()\n";
  code += "{\n";
  code += "  m_sequencers_vec.deletePointers();\n";

  for (int i=0; i<size; i++) {
    StateMachine* machine_ptr = dynamic_cast<StateMachine*>(sym_vec[i]);
    if (machine_ptr != NULL) {
      code += "  m_" + machine_ptr->cIdent() + "_Controller_vec.deletePointers();\n";
    }
  }

  for (int i=0; i<size; i++) {
    Var* var = dynamic_cast<Var*>(sym_vec[i]);
    if (var != NULL) {
      if (var->getMachine() == NULL) {
        if (!var->existPair("parameter")) {
          code += "  delete m_" + var->cIdent() + "_ptr;\n";
        }
      }
    }
  }

  code += "}\n\n";

  // printConfig
  code += "void System::printConfig(ostream& out) const\n";
  code += "{\n";

  code += "  out << \"protocol: \" << CURRENT_PROTOCOL << endl;\n";
//  code += "  SIMICS_print_version(out);\n"
  code += "  out << \"compiled_at: \" << __TIME__ << \", \" << __DATE__ << endl;\n";
  code += "  out << \"RUBY_DEBUG: \" << boolalpha << RUBY_DEBUG << endl;\n";
  code += "  char buffer[100];\n";
  code += "  gethostname(buffer, 50);\n";
  code += "  out << \"hostname: \" << buffer << endl;\n";
  code += "\n";
  code += "  g_param_ptr->printConfig(out);\n";

  for (int i=0; i<size; i++) {
    Var* var = dynamic_cast<Var*>(sym_vec[i]);
    if (var != NULL) {
      if (var->getMachine() == NULL) {
        if (!var->existPair("parameter")) {
          code += "  m_" + var->cIdent() + "_ptr->printConfig(out);\n";
        }
      }
    }
  }

  code += "  m_sequencers_vec[0]->printConfig(out);\n";
  
  for (int i=0; i<size; i++) {
    StateMachine* machine_ptr = dynamic_cast<StateMachine*>(sym_vec[i]);
    if (machine_ptr != NULL) {
      code += "  m_" + machine_ptr->cIdent() + "_Controller_vec[0]->printConfig(out);\n";
    }
  }

  code += "}\n\n";

  // printStats
  code += "void System::printStats(ostream& out) const\n";
  code += "{\n";

  for (int i=0; i<size; i++) {
    Var* var = dynamic_cast<Var*>(sym_vec[i]);
    if (var != NULL) {
      if (var->getMachine() == NULL) {
        if (!var->existPair("parameter")) {
          code += "  m_" + var->cIdent() + "_ptr->printStats(out);\n";
        }
      }
    }
  }

  for (int i=0; i<size; i++) {
    StateMachine* machine_ptr = dynamic_cast<StateMachine*>(sym_vec[i]);
    if (machine_ptr != NULL) {
      code += "  " + machine_ptr->cIdent() + "_Controller::dumpStats(out);\n";
    }
  }

  code += "}\n\n";

  // clearStats
  code += "void System::clearStats() const\n";
  code += "{\n";

  for (int i=0; i<size; i++) {
    Var* var = dynamic_cast<Var*>(sym_vec[i]);
    if (var != NULL) {
      if (var->getMachine() == NULL) {
        if (!var->existPair("parameter")) {
          code += "  m_" + var->cIdent() + "_ptr->clearStats();\n";
        }
      }
    }
  }

  for (int i=0; i<size; i++) {
    StateMachine* machine_ptr = dynamic_cast<StateMachine*>(sym_vec[i]);
    if (machine_ptr != NULL) {
      code += "  " + machine_ptr->cIdent() + "_Controller::clearStats();\n";
    }
  }

  code += "}\n";
  code += "\n";
  code += "void System::recordCacheContents(CacheRecorder& tr) const\n";
  code += "{\n";
  code += "  for (int i=0; i < m_L1Cache_Controller_vec.size(); i++) {\n";
  code += "    m_L1Cache_Controller_vec[i]->m_L1Cache_L1IcacheMemory_ptr->recordCacheContents(tr, true);\n";
  code += "    m_L1Cache_Controller_vec[i]->m_L1Cache_L1DcacheMemory_ptr->recordCacheContents(tr, false);\n";
  code += "    m_L1Cache_Controller_vec[i]->m_L1Cache_L2cacheMemory_ptr->recordCacheContents(tr, false); // Remove me\n";
  code += "  }\n";
  code += "}\n";

  // Write file
  conditionally_write_file(path + "/System.C", code);
}

