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

#include "html_gen.h"
#include "fileio.h"
#include "SymbolTable.h"
#include "Symbol.h"
#include "StateMachine.h"
#include "Action.h"
#include "State.h"
#include "Event.h"
#include "Transition.h"

string formatHTMLShorthand(const string shorthand);


void createHTMLSymbol(const Symbol& sym, string title, ostream& out)
{
  out << "<HTML><BODY><BIG>" << endl;
  out << title << ": " << endl;
  out << formatHTMLShorthand(sym.getShorthand()) << " - ";
  out << sym.getDescription();
  out << "</BIG></BODY></HTML>" << endl;
}

void createHTMLindex(string title, ostream& out)
{
  out << "<html>" << endl;
  out << "<head>" << endl;
  out << "<title>" << title << "</title>" << endl;
  out << "</head>" << endl;
  out << "<frameset rows=\"*,30\">" << endl;
  Vector<StateMachine*> machine_vec = g_sym_table.getStateMachines();
  if (machine_vec.size() > 1) {
    string machine = machine_vec[0]->getIdent();
    out << "  <frame name=\"Table\" src=\"" << machine << "_table.html\">" << endl;
  } else {
    out << "  <frame name=\"Table\" src=\"empty.html\">" << endl;
  }
  
  out << "  <frame name=\"Status\" src=\"empty.html\">" << endl;
  out << "</frameset>" << endl;
  out << "</html>" << endl;
}

string formatHTMLShorthand(const string shorthand)
{
  string munged_shorthand = "";
  bool mode_is_normal = true;

  // -- Walk over the string, processing superscript directives
  for(unsigned int i = 0; i < shorthand.length(); i++) {
    if(shorthand[i] == '!') {
      // -- Reached logical end of shorthand name
      break;
    } else if( shorthand[i] == '_') {
      munged_shorthand += " ";      
    } else if( shorthand[i] == '^') {
      // -- Process super/subscript formatting
      mode_is_normal = !mode_is_normal;
      if(mode_is_normal) {
        // -- Back to normal mode
        munged_shorthand += "</SUP>";
      } else {
        // -- Going to superscript mode
        munged_shorthand += "<SUP>";
      }
    } else if(shorthand[i] == '\\') {
      // -- Process Symbol character set
      if((i + 1) < shorthand.length()) {
        i++;  // -- Proceed to next char. Yes I know that changing the loop var is ugly!
        munged_shorthand += "<B><FONT size=+1>";
        munged_shorthand += shorthand[i];
        munged_shorthand += "</FONT></B>";
      } else {
        // -- FIXME: Add line number info later
        cerr << "Encountered a `\\` without anything following it!" << endl;
        exit( -1 );
      }
    } else {
      // -- Pass on un-munged
      munged_shorthand += shorthand[i];
    }
  } // -- end for all characters in shorthand

  // -- Do any other munging 
  if(!mode_is_normal) {
    // -- Back to normal mode
    munged_shorthand += "</SUP>";
  }

  // -- Return the formatted shorthand name
  return munged_shorthand;
}

void write_HTML_files(string path)
{
  // Create index.html
  {
    ostringstream out;
    createHTMLindex(path, out);
    conditionally_write_file(path + "index.html", out);
  }

  // Create empty.html
  {
    ostringstream out;
    out << "<HTML></HTML>";
    conditionally_write_file(path + "empty.html", out);
  }

  // Write all the symbols
  const Vector<Symbol*>& sym_vec = g_sym_table.getAllSymbols();
  int size = sym_vec.size();
  for(int i=0; i<size; i++) {
    sym_vec[i]->writeHTMLFiles(path);
  }
}

string frameRef(string click_href, string click_target, string over_href, string over_target_num, string text) 
{
  string temp;
  temp += "<A href=\"" + click_href + "\" ";
  temp += "target=\"" + click_target + "\" ";
  string javascript = "if (parent.frames[" + over_target_num + "].location != parent.location + '" + over_href + "') { parent.frames[" + over_target_num + "].location='" + over_href + "' }";
  //  string javascript = "parent." + target + ".location='" + href + "'";
  temp += "onMouseOver=\"" + javascript + "\" ";
  temp += ">" + text + "</A>";
  return temp;
}

string frameRef(string href, string target, string target_num, string text) 
{
  return frameRef(href, target, href, target_num, text);
}

void write_HTML_files_machine(string path, const StateMachine& machine)
{
  string filename;
  string component = machine.getIdent();

  // Create table with no row highlighted
  {
    string out;
    print_HTML_transitions(out, machine.numStates()+1, machine);
    
    // -- Write file
    filename = component + "_table.html";
    conditionally_write_file(path + filename, out);
  }

  // Generate transition tables
  for(int i=0; i<machine.numStates(); i++) {
    string out;
    print_HTML_transitions(out, i, machine);
    
    // -- Write file
    filename = component + "_table_" + machine.getState(i).getIdent() + ".html";
    conditionally_write_file(path + filename, out);
  }

  // Generate action descriptions
  for(int i=0; i<machine.numActions(); i++) {
    ostringstream out;
    createHTMLSymbol(machine.getAction(i), "Action", out);
    
    // -- Write file
    filename = component + "_action_" + machine.getAction(i).getIdent() + ".html";
    conditionally_write_file(path + filename, out);
  }

  // Generate state descriptions
  for(int i=0; i<machine.numStates(); i++) {
    ostringstream out;
    createHTMLSymbol(machine.getState(i), "State", out);
    
    // -- Write file
    filename = component + "_State_" + machine.getState(i).getIdent() + ".html";
    conditionally_write_file(path + filename, out);
  }

  // Generate event descriptions
  for(int i=0; i<machine.numEvents(); i++) {
    ostringstream out;
    createHTMLSymbol(machine.getEvent(i), "Event", out);
    
    // -- Write file
    filename = component + "_Event_" + machine.getEvent(i).getIdent() + ".html";
    conditionally_write_file(path + filename, out);
  }
}

void print_HTML_transitions(string& str, int active_state, const StateMachine& machine)
{
  // -- Prolog
  str += "<HTML><BODY link=\"blue\" vlink=\"blue\">\n";
  
  // -- Header
  str += "<H1 align=\"center\">" + formatHTMLShorthand(machine.getShorthand()) + ": \n";
  Vector<StateMachine*> machine_vec = g_sym_table.getStateMachines();
  for (int i=0; i<machine_vec.size(); i++) {
    StateMachine* type = machine_vec[i];
    if (i != 0) {
      str += " - ";
    }
    if (type == &machine) {
      str += type->getIdent() + "\n";
    } else {
      str += "<A target=\"Table\"href=\"" + type->getIdent() + "_table.html\">" + type->getIdent() + "</A>  \n";
    }
  }
  str += "</H1>\n";

  // -- Table header
  str += "<TABLE border=1>\n";

  // -- Column headers
  str += "<TR>\n";

  // -- First column header
  str += "  <TH> </TH>\n";

  for(int event = 0; event < machine.numEvents(); event++ ) {
    str += "  <TH bgcolor=white>";
    str += frameRef(machine.getIdent() + "_Event_" + machine.getEvent(event).getIdent() 
                    + ".html", "Status", "1", formatHTMLShorthand(machine.getEvent(event).getShorthand()));
    str += "</TH>\n";
  }

  str += "</TR>\n";
  
  // -- Body of table 
  for(int state = 0; state < machine.numStates(); state++ ) {
    str += "<TR>\n";

    // -- Each row
    if (state == active_state) {
      str += "  <TH bgcolor=yellow>";
    } else {
      str += "  <TH bgcolor=white>";
    }

    string click_href = machine.getIdent() + "_table_" + machine.getState(state).getIdent() + ".html";
    string text = formatHTMLShorthand(machine.getState(state).getShorthand());

    str += frameRef(click_href, "Table", 
                    machine.getIdent() + "_State_" + machine.getState(state).getIdent() + ".html", 
                    "1", 
                    formatHTMLShorthand(machine.getState(state).getShorthand()));
    str += "</TH>\n";

    // -- One column for each event
    for(int event = 0; event < machine.numEvents(); event++ ) {
      const Transition* trans_ptr = machine.getTransPtr(state, event);
      
      if(trans_ptr != NULL) {
        bool stall_action = false;
        string nextState;
        string actions_str;
        
        // -- Get the actions
        //        actions = trans_ptr->getActionShorthands();
        const Vector<Action*> actions = trans_ptr->getActions();
        for (int action=0; action < actions.size(); action++) {
          if ((actions[action]->getIdent() == "z_stall") ||
              (actions[action]->getIdent() == "zz_recycleMandatoryQueue")) {
            stall_action = true;
          }
          actions_str += "  ";
          actions_str += frameRef(machine.getIdent() + "_action_" + actions[action]->getIdent() + ".html", 
                                  "Status", 
                                  "1",
                                  formatHTMLShorthand(actions[action]->getShorthand()));
          actions_str += "\n";
        }
        
        // -- Get the next state
        if (trans_ptr->getNextStatePtr()->getIdent() != machine.getState(state).getIdent()) { 
          string click_href = machine.getIdent() + "_table_" + trans_ptr->getNextStatePtr()->getIdent() + ".html";
          nextState = frameRef(click_href, 
                               "Table", 
                               machine.getIdent() + "_State_" + trans_ptr->getNextStatePtr()->getIdent() + ".html", 
                               "1", 
                               formatHTMLShorthand(trans_ptr->getNextStateShorthand()));
        } else {
          nextState = "";
        }
        
        // -- Print out "actions/next-state"
        if (stall_action) {
          if (state == active_state) {
            str += "  <TD bgcolor=#C0C000>";
          } else {
            str += "  <TD bgcolor=lightgrey>";
          }
        } else if (active_state < machine.numStates() && 
                   (trans_ptr->getNextStatePtr()->getIdent() == machine.getState(active_state).getIdent())) {
          str += "  <TD bgcolor=aqua>";
        } else if (state == active_state) {
          str += "  <TD bgcolor=yellow>";
        } else {
          str += "  <TD bgcolor=white>";
        }
        
        str += actions_str;
        if ((nextState.length() != 0) && (actions_str.length() != 0)) {
          str += "/";
        }
        str += nextState;
        str += "</TD>\n";
      } else {
        // This is the no transition case
        if (state == active_state) {
          str += "  <TD bgcolor=#C0C000>&nbsp;</TD>\n";
        } else {
          str += "  <TD bgcolor=lightgrey>&nbsp;</TD>\n";
        }
      }
    }
    // -- Each row
    if (state == active_state) {
      str += "  <TH bgcolor=yellow>";
    } else {
      str += "  <TH bgcolor=white>";
    }

    click_href = machine.getIdent() + "_table_" + machine.getState(state).getIdent() + ".html";
    text = formatHTMLShorthand(machine.getState(state).getShorthand());

    str += frameRef(click_href, "Table", 
                    machine.getIdent() + "_State_" + machine.getState(state).getIdent() + ".html", 
                    "1", formatHTMLShorthand(machine.getState(state).getShorthand()));
    str += "</TH>\n";

    str += "</TR>\n";
  }
  
  // -- Column footer
  str += "<TR>\n";
  str += "  <TH> </TH>\n";
  
  for(int i = 0; i < machine.numEvents(); i++ ) {
    str += "  <TH bgcolor=white>";
    str += frameRef(machine.getIdent() + "_Event_" + machine.getEvent(i).getIdent() + ".html", 
                    "Status", "1", 
                    formatHTMLShorthand(machine.getEvent(i).getShorthand()));
    str += "</TH>\n";
  }
  str += "</TR>\n";
  
  // -- Epilog
  str += "</TABLE>\n";
  str += "</BODY></HTML>\n";
}
