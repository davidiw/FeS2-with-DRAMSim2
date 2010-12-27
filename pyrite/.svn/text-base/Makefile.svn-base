#  -----------------------------------------------------------------------------
# 
#   This file is part of FeS2.
# 
#   FeS2 is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
# 
#   FeS2 is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
# 
#   You should have received a copy of the GNU General Public License
#   along with FeS2.  If not, see <http://www.gnu.org/licenses/>.
# 
#  -----------------------------------------------------------------------------

ifndef FES2_HOME
$(error You forgot to set the FES2_HOME environmental variable!)
endif

ifdef ENABLE_DEBUG
  OPT_FLAGS=-O0
else
  OPT_FLAGS=-O3
endif

SIMICS_API = 3.0 
MODULE_CLASSES = pyrite

EXTRA_VPATH=$(FES2_HOME)/autogen/include:$(FES2_HOME)/autogen/src

EXTRA_OBJ_FILES += $(shell ls $(FES2_HOME)/obj/*.o)

WARNING_FLAGS=-Wall -Wno-inline -Wwrite-strings -Wno-unused
DEBUG_FLAGS=-ggdb -g3 
MODULE_CFLAGS=$(WARNING_FLAGS) $(OPT_FLAGS) $(DEBUG_FLAGS)
MODULE_LDFLAGS=$(WARNING_FLAGS) $(OPT_FLAGS) $(DEBUG_FLAGS) -lz

include $(MODULE_MAKEFILE)
