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

#include "debug_cat.h"

unsigned long long DEBUG_CATEGORY;

// there are only 64 possible debugging categories, since we use an
// integer type as the bit field. Actually, there are only 32 types
// since we just use a regular int at the moment.
string DEBUG_CATEGORY_STRING;

const char DEBUG_CATEGORY_SHORTCUTS[] = {
  '#', // the "shh" character suppresses all debugging
#include "debug_cat_shortcuts.inc"
  ' ' // the space character serves as a sentinel
};
