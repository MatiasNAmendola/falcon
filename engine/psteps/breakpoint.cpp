/*
   FALCON - The Falcon Programming Language.
   FILE: breakpoint.cpp

   Special statement -- breakpoint
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Sun, 16 Oct 2011 21:51:42 +0200

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#include <falcon/trace.h>
#include <falcon/psteps/breakpoint.h>

namespace Falcon
{

Breakpoint::Breakpoint( int32 line, int32 chr ):
   Statement(e_stmt_breakpoint, line, chr )
{
   apply = apply_;
}

Breakpoint::~Breakpoint()
{
}

void Breakpoint::describeTo( String& tgt ) const
{
   tgt = "(*)";
}

void Breakpoint::apply_( const PStep*, VMContext* ctx )
{
   ctx->breakpoint();
   ctx->popCode();
}

}

/* end of breakpoint.cpp */