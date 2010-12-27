// ----------------------------------------------------------------------
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
// ----------------------------------------------------------------------

#ifndef __EVENT_LOG_H
#define __EVENT_LOG_H

#include "Global.h"
#include "globals.h"
#include "debug_cat.h"
#include "stats.h"
#include "terminal-colors.h"
#include "Vector.h"
#include "Box.h"

class LogEvent {

private:
  bool valid;
  W64 cycle;
  DebugCategory category;
  Box<TransOp> box;
  string msg;

public:
  LogEvent(DebugCategory e, Box<TransOp> b, string m) {
    cycle = g_stats.getTotalCycles();
    category = e;
    box = b; /** @TODO: does this "do the right thing", i.e. make a copy? */
    msg = m; 
    valid = true;
  }

  ~LogEvent() { }

  bool isValid() { return valid; }

  void print(ostream& os) { 
    if ( IS_DEBUGGED(category) ) {
      os << "<cycle " << cycle << "> ";
#include "debug_cat_colors.inc"
      PRINT_CATEGORY_COLOR(category, os);
      os << msg;
      if ( box.isEmpty() ) {
        os << endl;
      } else{
        box.print(os);
      }
      os << CLEAR << endl; // colorize the uop, too
    }
  }

  void printPlain(ostream& os) { os << msg; }

}; // end class Event


/** A circular buffer for holding the last n events. */
class EventLog {
  
private:
  Vector<LogEvent*> ring;
  int ring_size;
  int head_index; // the next free entry
  
public:
  EventLog(int size=32768) {
    ring_size = size;
    ring = Vector<LogEvent*>(ring_size);
    head_index = 0;
  }

  /** Add a LogEvent to the ring. 
   * @param e A pointer to a LogEvent allocated on the heap. This event
   * will be deleted by the EventLog when/if necessary.
   */
  void add(LogEvent* e) {
    int elems = ring.size();
    if ( elems == ring_size ) {
      assert( ring[0]->isValid() );
      delete ring[0];
      ring.removeFromTop( 1 );
    }
    ring.insertAtBottom( e );
    //head_index = (head_index + 1) % ring_size;
  }

  void print(ostream& os) {
    // Start at head and walk backwards. i = x % y, for x < 0,
    // produces a negative number, so we need to add ring_size to keep
    // everything within bounds
    os << "Event Log:" << endl;
    //int i = (head_index + ring_size - 1) % ring_size;
    for ( int i = 0; i < ring.size(); i++ ) {
      assert( ring[i]->isValid() );
      ring[i]->print(os);
    }
  }
  
}; // end class EventLog

extern EventLog g_eventLog;

#undef LOG_EVENT
#define LOG_EVENT(CATEGORIES, UOP, SSTREAM)\
  if ( IS_DEBUGGED(CATEGORIES) ) {\
    stringstream s;\
    s << SSTREAM;\
    g_eventLog.add( new LogEvent(CATEGORIES, UOP, s.str()) );\
  }

#endif
