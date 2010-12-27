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
/* -----------------------------------------------------------------------------
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
// ---------------------------------------------------------------------------*/

%{

#include <assert.h>
#include "ASTs.h"
#include "parser.h"
#include <string>

extern "C" int yylex();
extern "C" void yyerror();
extern "C" int yywrap()
{
    return 1;
}

%}
%x CMNT
%x IMBEDED
%%

[\t ]+   /* Ignore whitespace */ 
[\n]       { g_line_number++; }  
"//".*[\n] { g_line_number++; }  /* C++ style comments */

"/*" BEGIN CMNT;
<CMNT>. ;
<CMNT>\n { g_line_number++; }  
<CMNT>"*/" { BEGIN INITIAL; }

true       { yylval.str_ptr = new string(yytext); return LIT_BOOL; }
false      { yylval.str_ptr = new string(yytext); return LIT_BOOL; }
machine    { return MACHINE_DECL; }
in_port    { return IN_PORT_DECL; }
out_port   { return OUT_PORT_DECL; }
action     { return ACTION_DECL; }
transition { return TRANSITION_DECL; }
structure  { return STRUCT_DECL; }
external_type { return EXTERN_TYPE_DECL; }
enumeration { return ENUM_DECL; }
peek       { return PEEK; }
enqueue    { return ENQUEUE; }
copy_head  { return COPY_HEAD; }
check_allocate  { return CHECK_ALLOCATE; }
parameter  { return PARAMETER_DECL; }
if         { return IF; }
else       { return ELSE; }
return     { return RETURN; }
void       { yylval.str_ptr = new string(yytext); return VOID; }

==        { yylval.str_ptr = new string(yytext); return EQ; }
!=        { yylval.str_ptr = new string(yytext); return NE; }
[<]       { yylval.str_ptr = new string(yytext); return '<'; }
[>]       { yylval.str_ptr = new string(yytext); return '>'; }
[<][<]    { yylval.str_ptr = new string(yytext); return LEFTSHIFT; }
[>][>]    { yylval.str_ptr = new string(yytext); return RIGHTSHIFT; }
[<][=]    { yylval.str_ptr = new string(yytext); return LE; }
[>][=]    { yylval.str_ptr = new string(yytext); return GE; }
[!]       { yylval.str_ptr = new string(yytext); return NOT; }
[&][&]    { yylval.str_ptr = new string(yytext); return AND; }
[|][|]    { yylval.str_ptr = new string(yytext); return OR; }
[+]       { yylval.str_ptr = new string(yytext); return PLUS; }
[-]       { yylval.str_ptr = new string(yytext); return DASH; }
[*]       { yylval.str_ptr = new string(yytext); return STAR; }
[/]       { yylval.str_ptr = new string(yytext); return SLASH; }
[:]       { return ':'; } 
[;]       { return SEMICOLON; }
[[]       { return '['; }
[]]       { return ']'; }
[{]       { return '{'; }
[}]       { return '}'; }
[(]       { return '('; }
[)]       { return ')'; }
[,]       { return ','; }
[=]       { return '='; }
:=        { return ASSIGN; }
[.]       { return DOT; }

[0-9]*     { yylval.str_ptr = new string(yytext); return NUMBER; }
[0-9]+[L]   { yylval.str_ptr = new string(yytext); return LONG; }
[0-9]*[.][0-9]+    { yylval.str_ptr = new string(yytext); return FLOAT; }
[a-zA-Z_][a-zA-Z_0-9]{0,50}    { yylval.str_ptr = new string(yytext); return IDENT; }
\"[^"\n]*\"  { yytext[strlen(yytext)-1] = '\0';  yylval.str_ptr = new string(yytext+1); return STRING; }
\'[^'\n]*\'  { yytext[strlen(yytext)-1] = '\0';  yylval.str_ptr = new string(yytext+1); return STRING; }

.         { return OTHER; }  /* Need so that we handle all characters */

%% 

