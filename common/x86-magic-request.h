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

#ifndef __X86_MAGIC_REQUEST_H
#define __X86_MAGIC_REQUEST_H

#include <simics/magic-instruction.h> // for Simics' magic insn


/* Passes an argument to the simulator via %eax. */
extern inline void* SIMULATOR_REQUEST(void* a) {
  void* ret;
  // CCured doesn't like this inline asm for some reason
#ifdef USING_CCURED  
  { __NOCUREBLOCK
#endif
      do {
        __asm__ __volatile__ ( "movl %1, %%eax" 
                               : "=m"(ret) /*output*/
                               : "r"(a), "m"(ret) /*input*/
                               : "%eax" ); /*clobber*/
        MAGIC( 0 );
      } while( 0 );
      do {
        __asm__ __volatile__ ( "movl %%eax, %0" 
                               : "=r"(ret) /*output*/
                               : ); /*clobber*/
      } while (0);
#ifdef USING_CCURED  
  }
#endif
  return ret;
}

/* Passes 2 arguments to the simulator, in %eax and %ecx, respectively. Try to
   avoid using this whenever possible to give the register allocator more
   freedom. */
extern inline void* SIMULATOR_REQUEST2(void* a, void* c) {
  void* ret;
#ifdef USING_CCURED  
  { __NOCUREBLOCK
#endif
      do {
        __asm__ __volatile__ ( "movl %0, %%eax;\n\t movl %1, %%ecx"
                               : /*output*/
                               : "r"(a), "r"(c) /*input*/
                               : "%eax", "%ecx" ); /*clobber*/
        MAGIC( 0 );
      } while( 0 );
      do {
        __asm__ __volatile__ ( "movl %%eax, %0" 
                               : "=r"(ret) /*output*/
                               : ); /*clobber*/
      } while (0);
#ifdef USING_CCURED
  }
#endif
  return ret;
}

#endif
