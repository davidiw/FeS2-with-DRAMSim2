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

#include "machine_gen.h"
#include "SymbolTable.h"
#include "Type.h"
#include "Var.h"
#include "Func.h"
#include "StateMachine.h"
#include "Action.h"
#include "State.h"
#include "Event.h"
#include "Enum.h"
#include "Transition.h"

void print_controller_h(ostream& out, string component, const StateMachine& machine)
{
  out << "// Auto generated C++ code started by "<<__FILE__<<":"<<__LINE__<< endl;
  out << "// " << machine.getIdent() << ": " << machine.getShorthand() << endl;
  out << endl;
  out << "#ifndef " << component << "_CONTROLLER_H" << endl;
  out << "#define " << component << "_CONTROLLER_H" << endl;
  out << endl;
  out << "#include \"Global.h\"" << endl;
  out << "#include \"Consumer.h\"" << endl;
  out << "#include \"TransitionResult.h\"" << endl;
  out << "#include \"Types.h\"" << endl;
  out << "#include \"Network.h\"" << endl;
  out << "#include \"" << component << "_Profiler.h\"" << endl;
  out << endl;
  out << "class " << component << "_Controller : public Consumer {" << endl;

  out << "public:" << endl;
  out << "  " << component << "_Controller(NodeID id, Network* network_ptr);" << endl;
  out << "  ~" << component << "_Controller();" << endl;
  out << "  void print(ostream& out) const;" << endl;
  out << "  void wakeup();" << endl;
  out << "  void printConfig(ostream& out);" << endl;
  out << "  static void dumpStats(ostream& out) { s_profiler.dumpStats(out); }" << endl;
  out << "  static void clearStats() { s_profiler.clearStats(); }" << endl;

  // Member variables
  out << "  // Member variables (should probably be private)" << endl;
  const Vector<Symbol*>& symbols = g_sym_table.getAllSymbols();
  for (int i=0; i < symbols.size(); i++) {
    Var* var_ptr = dynamic_cast<Var*>(symbols[i]);
    if (var_ptr == NULL || var_ptr->getMachine() != &machine) {
      continue;
    }
    out << "  " << var_ptr->getType()->cIdent() << "* m_" << var_ptr->cIdent() << "_ptr;" << endl;
  }

  out << "private:" << endl;
  out << "  TransitionResult doTransition(" << component << "_Event event, " << component 
      << "_State state, const Address& addr);  // in " << component << "_Transitions.C" << endl;
  out << "  TransitionResult doTransitionWorker(" << component << "_Event event, " << component 
      << "_State state, " <<  component << "_State& next_state, const Address& addr);  // in " 
      << component << "_Transitions.C" << endl;
  out << "  NodeID m_id;" << endl;
  out << "  static " << component << "_Profiler s_profiler;" << endl;
  
  // internal function protypes
  {
    out << "  // Internal functions" << endl;
    const Vector<Symbol*>& symbols = g_sym_table.getAllSymbols();
    for (int i=0; i < symbols.size(); i++) {
      Func* func_ptr = dynamic_cast<Func*>(symbols[i]);
      if (func_ptr == NULL || func_ptr->getMachine() != &machine) {
        continue;
      }
      string proto;
      func_ptr->funcPrototype(proto);
      if (proto != "") {
        out << "  " << proto;
      }
    }
  }
    
  out << "  // Actions" << endl;
  for(int i=0; i < machine.numActions(); i++) {
    const Action& action = machine.getAction(i);
    out << "  void " << action.getIdent() << "(const Address& addr);" << endl;
  }  
  out << "};" << endl;
  out << "#endif // " << component << "_CONTROLLER_H" << endl;
}

void print_controller_C(ostream& out, string component, const StateMachine& machine)
{
  out << "// Auto generated C++ code started by "<<__FILE__<<":"<<__LINE__<< endl;
  out << "// " << machine.getIdent() << ": " << machine.getShorthand() << endl;
  out << endl;
  out << "#include \"Global.h\"" << endl;
  out << "#include \"slicc_util.h\"" << endl;
  out << "#include \"" << component << "_Controller.h\"" << endl;
  out << "#include \"" << component << "_State.h\"" << endl;
  out << "#include \"" << component << "_Event.h\"" << endl;
  out << "#include \"Types.h\"" << endl;
  out << "#include \"Profiler.h\"" << endl;
  out << "#include \"Network.h\"" << endl;
  out << endl;

  out << "// static profiler defn" << endl;
  out << component << "_Profiler " << component << "_Controller::s_profiler;" << endl;
  out << endl;

  out << "// constructor" << endl;
  out << component << "_Controller::" << component << "_Controller(NodeID id, Network* network_ptr)" << endl;
  out << "{" << endl;
  out << "  m_id = id;" << endl;

  // Initialize member variables
  const Vector<Symbol*>& symbols = g_sym_table.getAllSymbols();
  for (int i=0; i < symbols.size(); i++) {
    Var* var = dynamic_cast<Var*>(symbols[i]);
    if (var == NULL || var->getMachine() != &machine) {
      continue;
    }
    out << "  // " << var->cIdent() << endl;
    if (var->existPair("network")) {
      // Network port object
      string network = var->lookupPair("network");
      string ordered =  var->lookupPair("ordered");
      string vnet =  var->lookupPair("virtual_network");
      
      out << "  m_" << var->cIdent() << "_ptr = network_ptr->get" 
          << network << "NetQueue(MachineType_" << var->getMachine()->getIdent() << ", m_id, " 
          << ordered << ", " << vnet << ");\n";
    } else if (var->getType()->existPair("primitive") || (dynamic_cast<Enum*>(var->getType()) != NULL)) {
      // Normal non-object
      out << "  m_" << var->cIdent() << "_ptr = new " << var->getType()->cIdent() << ";\n";
    } else {
      // Normal Object
      out << "  m_" << var->cIdent() << "_ptr = new " << var->getType()->cIdent();
      if (!var->getType()->existPair("non_obj")) {
        if (var->existPair("constructor_hack")) {
          string constructor_hack = var->lookupPair("constructor_hack");
          out << "(m_id, " << constructor_hack << ")";
        } else {
          out << "(m_id)";
        }
        out << ";\n";
      }
    }
    out << "  assert(m_" << var->cIdent() << "_ptr != NULL);" << endl;

    // Set to the default value
    if (var->existPair("default")) {
      out << "  (*m_" << var->cIdent() << "_ptr) = " << var->lookupPair("default") 
          << "; // Object default" << endl;
    } else if (var->getType()->hasDefault()) {
      out << "  (*m_" << var->cIdent() << "_ptr) = " << var->getType()->getDefault() 
          << "; // Type " << var->getType()->getIdent() << " default" << endl;
    }
    
    // Set ordering
    if (var->existPair("ordered")) {
      // A buffer
      string ordered =  var->lookupPair("ordered");
      out << "  m_" << var->cIdent() << "_ptr->setOrdering(" << ordered << ");\n";
    }
    
    // Set randomization
    if (var->existPair("random")) {
      // A buffer
      string value =  var->lookupPair("random");
      out << "  m_" << var->cIdent() << "_ptr->setRandomization(" << value << ");\n";
    }
    out << endl;
  }

  // Set the queue consumers
  for(int i=0; i < machine.numInPorts(); i++) {
    out << "  " << machine.getInPort(i).getCode() << ".setConsumer(this);" << endl;
  }

  out << endl;
  // Set the queue descriptions
  for(int i=0; i < machine.numInPorts(); i++) {
    out << "  " << machine.getInPort(i).getCode() 
        << ".setDescription(\"[Node \" + int_to_string(m_id) + \", " 
        << component << ", " << machine.getInPort(i).toString() << "]\");" << endl;
  }

  // Initialize the transition profiling
  out << endl;
  for(int i=0; i<machine.numTransitions(); i++) {
    const Transition& t = machine.getTransition(i);
    const Vector<Action*>& action_vec = t.getActions();
    int numActions = action_vec.size();

    // Figure out if we stall
    bool stall = false;
    for (int i=0; i<numActions; i++) {
      if(action_vec[i]->getIdent() == "z_stall") {
        stall = true;
      }
    }

    // Only possible if it is not a 'z' case
    if (!stall) {
      out << "  s_profiler.possibleTransition(" << component << "_State_"
          << t.getStatePtr()->getIdent() << ", " << component << "_Event_"
          << t.getEventPtr()->getIdent() << ");" << endl;
    }
  }

  out << "}" << endl;

  out << endl;

  {
    out << component << "_Controller::~" << component << "_Controller()" << endl;
    out << "{" << endl;
    const Vector<Symbol*>& symbols = g_sym_table.getAllSymbols();
    for (int i=0; i < symbols.size(); i++) {
      Var* var_ptr = dynamic_cast<Var*>(symbols[i]);
      if (var_ptr == NULL || var_ptr->getMachine() != &machine) {
        continue;
      }
      if (!var_ptr->existPair("network")) {
        // Normal Object
        out << "  delete m_" << var_ptr->cIdent() << "_ptr;\n";
      }
    }
    out << "}" << endl;
  }

  {  
    out << endl;
    out << "void " << component << "_Controller::printConfig(ostream& out)\n";
    out << "{\n";
    out << "  out << heading(\"" << component << " Config\");\n";
    
    const Vector<Symbol*>& symbols = g_sym_table.getAllSymbols();
    for (int i=0; i < symbols.size(); i++) {
      Var* var_ptr = dynamic_cast<Var*>(symbols[i]);
      if (var_ptr == NULL || var_ptr->getMachine() != &machine) {
        continue;
      }
      if (!var_ptr->existPair("network") && (!var_ptr->getType()->existPair("primitive"))) {
        // Normal Object
        out << "  m_" << var_ptr->cIdent() << "_ptr->printConfig(out);\n";
      }

    }
    out << "  out << endl;\n";
    out << "}" << endl;
  }
  
  out << endl;
  out << "void " << component << "_Controller::print(ostream& out) const { out << \"[" << component 
      << "_Controller \" << m_id << \"]\"; }" << endl;

  out << endl;
  out << "// Actions" << endl;
  out << endl;

  for(int i=0; i < machine.numActions(); i++) {
    const Action& action = machine.getAction(i);
    if (action.existPair("c_code")) {
      out << "//" << action.getDescription() << endl;
      out << "void " << component << "_Controller::" 
          << action.getIdent() << "(const Address& addr)" << endl;
      out << "{" << endl;
      out << "  DEBUG_MSG(GENERATED_COMP, HighPrio,\"executing\");" << endl;
      out << action.lookupPair("c_code");
      out << "}" << endl;
    }
    out << endl;
  }

  // Function definitions
  {
    const Vector<Symbol*>& symbols = g_sym_table.getAllSymbols();
    for (int i=0; i < symbols.size(); i++) {
      Func* func_ptr = dynamic_cast<Func*>(symbols[i]);
      if (func_ptr == NULL || func_ptr->getMachine() != &machine) {
        continue;
      }
      string code;
      func_ptr->funcDefinition(code);
      out << code;
    }
  }
}

void print_C_wakeup(ostream& out, string component, const StateMachine& machine)
{
  out << "// Auto generated C++ code started by "<<__FILE__<<":"<<__LINE__<< endl;
  out << "// " << machine.getIdent() << ": " << machine.getShorthand() << endl;
  out << endl;
  out << "#include \"Global.h\"" << endl;
  out << "#include \"slicc_util.h\"" << endl;
  out << "#include \"" << component << "_Controller.h\"" << endl;
  out << "#include \"" << component << "_State.h\"" << endl;
  out << "#include \"" << component << "_Event.h\"" << endl;
  out << "#include \"Types.h\"" << endl;
  out << "#include \"System.h\"" << endl;
  out << "#include \"Profiler.h\"" << endl;
  out << endl;
  out << "void " << component << "_Controller::wakeup()" << endl;
  out << "{" << endl;
  //  out << "  DEBUG_EXPR(GENERATED_COMP, MedPrio,*this);" << endl;
  //  out << "  DEBUG_EXPR(GENERATED_COMP, MedPrio,g_eventQueue_ptr->getTime());" << endl;
  out << endl;
  out << "  int counter = 0;" << endl;
  out << "  while (true) {" << endl;
  out << "    // Some cases will put us into an infinite loop without this limit" << endl;
  out << "    assert(counter <= g_param_ptr->TRANSITIONS_PER_RUBY_CYCLE());" << endl;
  out << "    if (counter == g_param_ptr->TRANSITIONS_PER_RUBY_CYCLE()) {" << endl;
  out << "      g_system_ptr->getProfiler()->controllerBusy(m_id); // Count how often we're fully utilized" << endl; 
  out << "      g_eventQueue_ptr->scheduleEvent(this, 1); // Wakeup in another cycle and try again" << endl;
  out << "      break;" << endl;
  out << "    }" << endl;

  // InPorts
  for(int i=0; i < machine.numInPorts(); i++) {
    const Var& port = machine.getInPort(i);
    assert(port.existPair("c_code_in_port"));
    out << "    // " 
        << component << "InPort " << port.toString() 
        << endl;
    out << port.lookupPair("c_code_in_port");
    out << endl;
  }

  out << "    break;  // If we got this far, we have nothing left todo" << endl;
  out << "  }" << endl;
  //  out << "  g_eventQueue_ptr->scheduleEvent(this, 1);" << endl;
  //  out << "  DEBUG_NEWLINE(GENERATED_COMP, MedPrio);" << endl;
  out << "}" << endl;
  out << endl;
}

void print_C_switch(ostream& out, string component, const StateMachine& machine)
{
  out << "// Auto generated C++ code started by "<<__FILE__<<":"<<__LINE__<< endl;
  out << "// " << machine.getIdent() << ": " <<  machine.getShorthand() << endl;
  out << endl;
  out << "#include \"Global.h\"" << endl;
  out << "#include \"" << component << "_Controller.h\"" << endl;
  out << "#include \"" << component << "_State.h\"" << endl;
  out << "#include \"" << component << "_Event.h\"" << endl;
  out << "#include \"Types.h\"" << endl;
  out << "#include \"System.h\"" << endl;
  out << "#include \"Profiler.h\"" << endl;
  out << endl;
  out << "#define HASH_FUN(state, event)  ((int(state)*" << component
      << "_Event_NUM)+int(event))" << endl;
  out << endl;
  out << "TransitionResult " << component << "_Controller::doTransition(" 
      << component << "_Event event, "
      << component << "_State state, "
      << "const Address& addr)" << endl;
           
  out << "{" << endl;
  out << "  " << component << "_State next_state = state;" << endl;
  out << endl;
  out << "  DEBUG_NEWLINE(GENERATED_COMP, MedPrio);" << endl;
  out << "  DEBUG_MSG(GENERATED_COMP, MedPrio,*this);" << endl;
  out << "  DEBUG_EXPR(GENERATED_COMP, MedPrio,g_eventQueue_ptr->getTime());" << endl;
  out << "  DEBUG_EXPR(GENERATED_COMP, MedPrio,state);" << endl;
  out << "  DEBUG_EXPR(GENERATED_COMP, MedPrio,event);" << endl;
  out << "  DEBUG_EXPR(GENERATED_COMP, MedPrio,addr);" << endl;
  out << endl;
  out << "  TransitionResult result = doTransitionWorker(event, state, next_state, addr);" << endl;
  out << endl;
  out << "  if (result == TransitionResult_Valid) {" << endl;
  out << "    DEBUG_EXPR(GENERATED_COMP, MedPrio, next_state);" << endl;
  out << "    DEBUG_NEWLINE(GENERATED_COMP, MedPrio);" << endl;
  out << "    s_profiler.countTransition(state, event);" << endl;
  out << "    if (g_param_ptr->PROTOCOL_DEBUG_TRACE()) {" << endl
      << "      g_system_ptr->getProfiler()->profileTransition(\"" << component << "\", m_id, addr, " << endl
      << "        " << component << "_State_to_string(state), " << endl
      << "        " << component << "_Event_to_string(event), " << endl
      << "        " << component << "_State_to_string(next_state), " << endl
      << "        \"\");" << endl
      << "    }" << endl;
  out << "    " << component << "_setState(addr, next_state);" << endl;
  out << "    " << endl;
  out << "  } else if (result == TransitionResult_ResourceStall) {" << endl;
  out << "    if (g_param_ptr->PROTOCOL_DEBUG_TRACE()) {" << endl
      << "      g_system_ptr->getProfiler()->profileTransition(\"" << component << "\", m_id, addr, " << endl
      << "        " << component << "_State_to_string(state), " << endl
      << "        " << component << "_Event_to_string(event), " << endl
      << "        " << component << "_State_to_string(next_state), " << endl
      << "        \"Resource Stall\");" << endl
      << "    }" << endl;
  out << "  } else if (result == TransitionResult_ProtocolStall) {" << endl;
  out << "    DEBUG_MSG(GENERATED_COMP,HighPrio,\"stalling\");" << endl
      << "    DEBUG_NEWLINE(GENERATED_COMP, MedPrio);" << endl;
  out << "    if (g_param_ptr->PROTOCOL_DEBUG_TRACE()) {" << endl
      << "      g_system_ptr->getProfiler()->profileTransition(\"" << component << "\", m_id, addr, " << endl
      << "        " << component << "_State_to_string(state), " << endl
      << "        " << component << "_Event_to_string(event), " << endl
      << "        " << component << "_State_to_string(next_state), " << endl
      << "        \"Protocol Stall\");" << endl
      << "    }" << endl
      << "  }" << endl;
  out << "  return result;" << endl;
  out << "}" << endl;
  out << endl;
  out << "TransitionResult " << component << "_Controller::doTransitionWorker(" 
      << component << "_Event event, "
      << component << "_State state, "
      << component << "_State& next_state, "
      << "const Address& addr)" << endl;
           
  out << "{" << endl;
  out << "" << endl;

  out << "  switch(HASH_FUN(state, event)) {" << endl;

  Map<string, Vector<string> > code_map; // This map will allow suppress generating duplicate code
  Vector<string> code_vec;

  for(int i=0; i<machine.numTransitions(); i++) {
    const Transition& t = machine.getTransition(i);
    string case_string = component + "_State_" + t.getStatePtr()->getIdent() 
      + ", " + component + "_Event_" + t.getEventPtr()->getIdent();

    string code;

    code += "  {\n";
    // Only set next_state if it changes
    if (t.getStatePtr() != t.getNextStatePtr()) {
      code += "    next_state = " + component + "_State_" + t.getNextStatePtr()->getIdent() + ";\n";
    }

    const Vector<Action*>& action_vec = t.getActions();
    int numActions = action_vec.size();

    // Check for resources
    Vector<string> code_sorter;
    const Map<Var*, int>& res = t.getResources();
    Vector<Var*> res_keys = res.keys();
    for (int i=0; i<res_keys.size(); i++) {
      string temp_code;
      temp_code += "    if (!" + (res_keys[i]->getCode()) + ".areNSlotsAvailable(" 
        + int_to_string(res.lookup(res_keys[i])) + ")) {\n";
      temp_code += "      return TransitionResult_ResourceStall;\n";
      temp_code += "    }\n";
      code_sorter.insertAtBottom(temp_code);
    }

    // Emit the code sequences in a sorted order.  This makes the
    // output deterministic (without this the output order can vary
    // since Map's keys() on a vector of pointers is not deterministic
    code_sorter.sortVector();
    for (int i=0; i<code_sorter.size(); i++) {
      code += code_sorter[i];
    }

    // Figure out if we stall
    bool stall = false;
    for (int i=0; i<numActions; i++) {
      if(action_vec[i]->getIdent() == "z_stall") {
        stall = true;
      }
    }
    
    if (stall) {
      code += "    return TransitionResult_ProtocolStall;\n";
    } else {
      for (int i=0; i<numActions; i++) {
        code += "    " + action_vec[i]->getIdent() + "(addr);\n";
      }
      code += "    return TransitionResult_Valid;\n";
    }
    code += "  }\n";

    
    // Look to see if this transition code is unique.
    if (code_map.exist(code)) {
      code_map.lookup(code).insertAtBottom(case_string);
    } else {
      Vector<string> vec;
      vec.insertAtBottom(case_string);
      code_map.add(code, vec);
      code_vec.insertAtBottom(code);
    }
  }
  
  // Walk through all of the unique code blocks and spit out the
  // corresponding case statement elements
  for (int i=0; i<code_vec.size(); i++) {
    string code = code_vec[i];

    // Iterative over all the multiple transitions that share the same code
    for (int case_num=0; case_num<code_map.lookup(code).size(); case_num++) {
      string case_string = code_map.lookup(code)[case_num];
      out << "  case HASH_FUN(" << case_string << "):" << endl;
    }
    out << code;
  }

  out << "  default:" << endl;
  out << "    WARN_EXPR(m_id);" << endl;              
  out << "    WARN_EXPR(g_eventQueue_ptr->getTime());" << endl;
  out << "    WARN_EXPR(addr);" << endl;
  out << "    WARN_EXPR(event);" << endl;
  out << "    WARN_EXPR(state);" << endl;
  out << "    ERROR_MSG(\"Invalid transition\");" << endl;
  out << "  }" << endl;
  out << "  return TransitionResult_Valid;" << endl;
  out << "}" << endl;
}

void print_profiler_h(ostream& out, string component, const StateMachine& machine)
{
  out << "// Auto generated C++ code started by "<<__FILE__<<":"<<__LINE__<< endl;
  out << "// " << machine.getIdent() << ": " <<  machine.getShorthand() << endl;
  out << endl;
  out << "#ifndef " << component << "_PROFILER_H" << endl;
  out << "#define " << component << "_PROFILER_H" << endl;
  out << endl;
  out << "#include \"Global.h\"" << endl;
  out << "#include \"" << component << "_State.h\"" << endl;
  out << "#include \"" << component << "_Event.h\"" << endl;
  out << endl;
  out << "class " << component << "_Profiler {" << endl;
  out << "public:" << endl;
  out << "  " << component << "_Profiler();" << endl;
  out << "  void countTransition(" << component << "_State state, " << component << "_Event event);" << endl;
  out << "  void possibleTransition(" << component << "_State state, " << component << "_Event event);" << endl;
  out << "  void dumpStats(ostream& out) const;" << endl;
  out << "  void clearStats();" << endl;
  out << "private:" << endl;
  out << "  int m_counters[" << component << "_State_NUM][" << component << "_Event_NUM];" << endl;
  out << "  bool m_possible[" << component << "_State_NUM][" << component << "_Event_NUM];" << endl;
  out << "};" << endl;
  out << "#endif // " << component << "_PROFILER_H" << endl;
}

void print_profiler_C(ostream& out, string component, const StateMachine& machine)
{
  out << "// Auto generated C++ code started by "<<__FILE__<<":"<<__LINE__<< endl;
  out << "// " << machine.getIdent() << ": " <<  machine.getShorthand() << endl;
  out << endl;
  out << "#include \"" << component << "_Profiler.h\"" << endl;
  out << endl;

  // Constructor
  out << component << "_Profiler::" << component << "_Profiler()" << endl;
  out << "{" << endl;
  out << "  for (int state = 0; state < " << component << "_State_NUM; state++) {" << endl;
  out << "    for (int event = 0; event < " << component << "_Event_NUM; event++) {" << endl;
  out << "      m_possible[state][event] = false;" << endl;
  out << "      m_counters[state][event] = 0;" << endl;
  out << "    }" << endl;
  out << "  }" << endl;
  out << "}" << endl;

  // Clearstats
  out << "void " << component << "_Profiler::clearStats()" << endl;
  out << "{" << endl;
  out << "  for (int state = 0; state < " << component << "_State_NUM; state++) {" << endl;
  out << "    for (int event = 0; event < " << component << "_Event_NUM; event++) {" << endl;
  out << "      m_counters[state][event] = 0;" << endl;
  out << "    }" << endl;
  out << "  }" << endl;
  out << "}" << endl;

  // Count Transition
  out << "void " << component << "_Profiler::countTransition(" << component << "_State state, " << component << "_Event event)" << endl;
  out << "{" << endl;
  out << "  assert(m_possible[state][event]);" << endl;
  out << "  m_counters[state][event]++;" << endl;
  out << "}" << endl;

  // Possible Transition
  out << "void " << component << "_Profiler::possibleTransition(" << component << "_State state, " << component << "_Event event)" << endl;
  out << "{" << endl;
  out << "  m_possible[state][event] = true;" << endl;
  out << "}" << endl;

  // dumpStats
  out << "void " << component << "_Profiler::dumpStats(ostream& out) const" << endl;
  out << "{" << endl;
  out << "  out << \" --- " << component << " ---\" << endl;" << endl;
  out << "  for (int state = 0; state < " << component << "_State_NUM; state++) {" << endl;
  out << "    for (int event = 0; event < " << component << "_Event_NUM; event++) {" << endl;
  out << "      if (m_possible[state][event]) {" << endl;
  out << "        int count = m_counters[state][event];" << endl;
  out << "        out << (" << component << "_State) state << \"  \" << (" << component << "_Event) event << \"  \" << count;" << endl;
  out << "        if (count == 0) {" << endl;
  out << "            out << \" <-- \";" << endl;
  out << "        }" << endl;
  out << "        out << endl;" << endl;
  out << "      }" << endl;
  out << "    }" << endl;
  out << "    out << endl;" << endl;
  out << "  }" << endl;
  out << "}" << endl;
}
