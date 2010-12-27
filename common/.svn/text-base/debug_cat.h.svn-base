#ifndef __SCSIM_DEBUG_H
#define __SCSIM_DEBUG_H

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

#include "std-includes.h"
#include "terminal-colors.h"

using namespace std;

// debug categories
typedef enum { 
  DEBUG_NONE = 0,
#include "debug_cat_enum.inc"
  DEBUG_ALL = 0xFFFFFFFF } DebugCategory;

/** a bitfield representing the category(ies) of debugging currently
    enabled */
extern unsigned long long DEBUG_CATEGORY;

/** a string buffer for holding a string representation of the
    currently enabled debugging category(ies) */
extern string DEBUG_CATEGORY_STRING;

/** an array of characters, each element of which is the shortcut
    character for the corresponding bit position in DEBUG_CATEGORY */
extern const char DEBUG_CATEGORY_SHORTCUTS[];

extern inline void NO_DEBUG(void) { DEBUG_CATEGORY = DEBUG_NONE; }
extern inline void ALL_DEBUG(void) { DEBUG_CATEGORY = DEBUG_ALL; }
/** add debugging for a specified category */
extern inline void DEBUG_CAT(DebugCategory category) { 
  DEBUG_CATEGORY = (DEBUG_CATEGORY | category); 
}
/** remove debugging for a specified category */
extern inline void UNDEBUG_CAT(DebugCategory category) { 
  DEBUG_CATEGORY = (DEBUG_CATEGORY & ~category); 
}
/** check if any category in categories is being debugged */
extern inline bool IS_DEBUGGED(DebugCategory categories) { 
  return ((DEBUG_CATEGORY & categories) != 0); 
}

/** 
 * print out the specified message if debugging is enabled for any of
 * the specified categories
 * @param categories a bitfield representing the categories associated
 * with this message
 * @param message the message to be printed
 */
extern inline void DEBUG_CAT_MSG(string MESSAGE, DebugCategory CATEGORIES) {
  if ( IS_DEBUGGED(CATEGORIES) ) {
    cout << "<cycle " << "FIXME: UNKNOWN CYCLE" << ">";
#include "debug_cat_colors.inc"
    PRINT_CATEGORY_COLOR(CATEGORIES, cout);
    cout << MESSAGE << CLEAR << endl << flush;
  }
}

#endif
