/*
   FALCON - The Falcon Programming Language.
   FILE: parser_index.cpp

   Parser for Falcon source files -- index accessor handler
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Sun, 03 Jul 2011 18:04:35 +0200

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#undef SRC
#define SRC "engine/sp/parser_index.cpp"

#include <falcon/setup.h>
#include <falcon/sp/sourceparser.h>
#include <falcon/sp/parsercontext.h>
#include <falcon/sp/parser_index.h>
#include <falcon/sp/parser_deletor.h>

#include <falcon/parser/rule.h>
#include <falcon/parser/parser.h>

#include <falcon/exprindex.h>

namespace Falcon {

using namespace Parsing;

void apply_expr_index( const Rule&, Parser& p )
{
   // << (r_Expr_index << "Expr_index" << apply_expr_index << Expr << T_OpenSquare << Expr << T_CloseSquare )
   SourceParser& sp = static_cast<SourceParser&>(p);

   TokenInstance* v1 = p.getNextToken();
   p.getNextToken();
   TokenInstance* v2 = p.getNextToken();
   p.getNextToken();

   TokenInstance* ti = new TokenInstance(v1->line(), v1->chr(), sp.Expr);
   ti->setValue( new ExprIndex(
         static_cast<Expression*>(v1->detachValue()),
         static_cast<Expression*>(v2->detachValue())
      ), expr_deletor );

   p.simplify(4,ti);
}


void apply_expr_star_index( const Rule&, Parser& p )
{
   // << (r_Expr_star_index << "Expr_star_index" << apply_expr_star_index << Expr << T_OpenSquare << T_Times << Expr << T_CloseSquare )
   SourceParser& sp = static_cast<SourceParser&>(p);

   TokenInstance* v1 = p.getNextToken();
   p.getNextToken();
   p.getNextToken();
   TokenInstance* v2 = p.getNextToken();
   p.getNextToken();

   // Todo: set lvalues and define symbols in the module
   TokenInstance* ti = new TokenInstance(v1->line(), v1->chr(), sp.Expr);
   ti->setValue( new ExprStarIndex(
         static_cast<Expression*>(v1->detachValue()),
         static_cast<Expression*>(v2->detachValue())
      ), expr_deletor );

   p.simplify(5,ti);
}

}

/* end of parser_index.cpp */