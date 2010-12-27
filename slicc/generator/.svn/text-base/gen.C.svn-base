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

#include "gen.h"
#include "fileio.h"
#include "html_gen.h"
#include "param_gen.h"
#include "machine_gen.h"
#include "system_gen.h"
#include "SymbolTable.h"
#include "Type.h"
#include "StateMachine.h"
#include "Event.h"
#include "State.h"
#include "Action.h"
#include "Transition.h"

void generate_files(const string& code_path, const string& html_path, const string& ident)
{
  // Generate C/C++ files
  cerr << "Writing C files..." << endl;

  {
    // Generate the name of the protocol
    ostringstream sstr;
    sstr << "// Auto generated C++ code started by "<<__FILE__<<":"<<__LINE__<<endl;
    sstr << endl;
    sstr << "#ifndef PROTOCOL_NAME_H" << endl;
    sstr << "#define PROTOCOL_NAME_H" << endl;
    sstr << endl;
    sstr << "const char CURRENT_PROTOCOL[] = \"";
    sstr << ident << "\";\n";
    sstr << "#endif // PROTOCOL_NAME_H" << endl;
    conditionally_write_file(code_path + "/protocol_name.h", sstr);
  }

  write_C_files(code_path);

  // Generate HTML files
  cerr << "Writing HTML files..." << endl;
  write_HTML_files(html_path);
  cerr << "Done..." << endl;
}

void write_C_files(string path)
{
  const Vector<Symbol*>& sym_vec = g_sym_table.getAllSymbols();
  int size = sym_vec.size();
  {
    // Write the Types.h include file for the types
    ostringstream sstr;
    sstr << "// Auto generated C++ code started by "<<__FILE__<<":"<<__LINE__<< endl;
    sstr << endl;
    sstr << "#include \"slicc_util.h\"" << endl;
    sstr << "#include \"Param.h\"" << endl;
    for(int i=0; i<size; i++) {
      Type* type = dynamic_cast<Type*>(sym_vec[i]);
      if (type != NULL && !type->isPrimitive()) {
        sstr << "#include \"" << type->cIdent() << ".h" << "\"" << endl;
      }
    }
    conditionally_write_file(path + "/Types.h", sstr);
  }
  
  // Write all the symbols
  for(int i=0; i<size; i++) { 
    sym_vec[i]->writeCFiles(path + '/');
  }
  
  write_param_h_file(path);
  write_param_C_file(path);
  write_system_h(path);
  write_system_C(path);
}

void write_C_files_machine(string path, const StateMachine& machine)
{
  string comp = machine.getIdent();
  string filename;
  
  // Output switch statement for transition table
  {
    ostringstream sstr;
    print_C_switch(sstr, comp, machine);
    conditionally_write_file(path + comp + "_Transitions.C", sstr);
  }
  
  // Output the actions for performing the actions
  {
    ostringstream sstr;
    print_controller_C(sstr, comp, machine);
    conditionally_write_file(path + comp + "_Controller.C", sstr);
  }
  
  // Output the method declarations for the class declaration      
  {
    ostringstream sstr;
    print_controller_h(sstr, comp, machine);
    conditionally_write_file(path + comp + "_Controller.h", sstr);
  }
  
  // Output the wakeup loop for the events
  {
    ostringstream sstr;
    print_C_wakeup(sstr, comp, machine);
    conditionally_write_file(path + comp + "_Wakeup.C", sstr);
  }
  
  // Profiling
  {
    ostringstream sstr;
    print_profiler_C(sstr, comp, machine);
    conditionally_write_file(path + comp + "_Profiler.C", sstr);
  }
  {
    ostringstream sstr;
    print_profiler_h(sstr, comp, machine);
    conditionally_write_file(path + comp + "_Profiler.h", sstr);
  }
}

void printDotty(const StateMachine& sm, ostream& out)
{
  out << "digraph " << sm.getIdent() << " {" << endl;
  for(int i=0; i<sm.numTransitions(); i++) {
    const Transition& t = sm.getTransition(i);
    // Don't print ignored transitions
    if ((t.getActionShorthands() != "--") && (t.getActionShorthands() != "z")) {
    //    if (t.getStateShorthand() != t.getNextStateShorthand()) {
      out << "  " << t.getStateShorthand() << " -> ";
      out << t.getNextStateShorthand() << "[label=\"";
      out << t.getEventShorthand() << "/" 
          << t.getActionShorthands() << "\"]" << endl;
    }
  }
  out << "}" << endl;
}

void printTexTable(const StateMachine& sm, ostream& out)
{
  const Transition* trans_ptr;
  int stateIndex, eventIndex;
  string actions;
  string nextState;

  out << "%& latex" << endl;
  out << "\\documentclass[12pt]{article}" << endl;
  out << "\\usepackage{graphics}" << endl;
  out << "\\begin{document}" << endl;
  //  out << "{\\large" << endl;
  out << "\\begin{tabular}{|l||";
  for(eventIndex=0; eventIndex < sm.numEvents(); eventIndex++) {
    out << "l";
  }
  out << "|} \\hline" << endl;

  for(eventIndex=0; eventIndex < sm.numEvents(); eventIndex++) {
    out << " & \\rotatebox{90}{";
    out << sm.getEvent(eventIndex).getShorthand();
    out << "}";
  }
  out << "\\\\ \\hline  \\hline" << endl;

  for(stateIndex=0; stateIndex < sm.numStates(); stateIndex++) {
    out << sm.getState(stateIndex).getShorthand();
    for(eventIndex=0; eventIndex < sm.numEvents(); eventIndex++) {
      out << " & ";
      trans_ptr = sm.getTransPtr(stateIndex, eventIndex);
      if (trans_ptr == NULL) {
      } else {
        actions = trans_ptr->getActionShorthands();
        // FIXME: should compare index, not the string
        if (trans_ptr->getNextStateShorthand() != 
            sm.getState(stateIndex).getShorthand() ) { 
          nextState = trans_ptr->getNextStateShorthand();
        } else {
          nextState = "";
        }

        out << actions;
        if ((nextState.length() != 0) && (actions.length() != 0)) {
          out << "/";
        }
        out << nextState;
      }
    }
    out << "\\\\" << endl;
  }
  out << "\\hline" << endl;
  out << "\\end{tabular}" << endl;
  //  out << "}" << endl;
  out << "\\end{document}" << endl;
}

