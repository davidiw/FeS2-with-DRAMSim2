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

#ifndef __TERMINAL_COLORS_H
#define __TERMINAL_COLORS_H

// color codes and their aliases
#define CLEAR "\033[0m"

#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLUE "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN "\033[1;36m"
#define WHITE "\033[1;37m"

#define DEBUG GREEN

// red background, yellow text
#define ERROR "\033[1;41;33m"
// magenta background, yellow text
#define WARNING "\033[1;45;33m"
// no background, magenta text
#define ADVISORY MAGENTA
// blue background, white text
#define INFO "\033[1;44;37m"

// error/warning macros
#define __SCSIM_ERROR ERROR "ERROR" CLEAR
#define __SCSIM_WARNING WARNING "WARNING" CLEAR

#endif
