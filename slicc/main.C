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

#include "main.h"
#include "gen.h"
#include "DeclListAST.h"

// -- Main conversion functions

DeclListAST* g_decl_list_ptr;
DeclListAST* parse(string filename);

int main(int argc, char *argv[])
{
  cerr << "SLICC v0.3" << endl;

  if (argc < 4) {
    cerr << "  Usage: generator.exec <code path> <html path> <ident> files ... " << endl;
    exit(1);
  }

  // The path we should place the generated code
  string code_path(argv[1]);
  code_path += "/";

  // The path we should place the generated html
  string html_path(argv[2]);
  html_path += "/";

  string ident(argv[3]);

  Vector<DeclListAST*> decl_list_vec;

  // Parse
  cerr << "Parsing..." << endl;
  for(int i=4; i<argc; i++) {
    cerr << "  " << argv[i] << endl;
    DeclListAST* decl_list_ptr = parse(argv[i]);
    decl_list_vec.insertAtBottom(decl_list_ptr);
  }

  // Find machines
  cerr << "Generator pass 1..." << endl;
  int size = decl_list_vec.size();
  for(int i=0; i<size; i++) {
    DeclListAST* decl_list_ptr = decl_list_vec[i];
    decl_list_ptr->findMachines();    
  }

  // Generate Code
  cerr << "Generator pass 2..." << endl;
  for(int i=0; i<size; i++) {
    DeclListAST* decl_list_ptr = decl_list_vec[i];
    decl_list_ptr->generate();    
    delete decl_list_ptr;
  }
  
  generate_files(code_path, html_path, ident);
}
