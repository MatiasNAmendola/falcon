/*
   FALCON - The Falcon Programming Language.
   FILE: exprmultiunpack.cpp

   Syntactic tree item definitions -- expression elements.
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Thu, 02 Jun 2011 14:08:47 +0200

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#include <falcon/trace.h>
#include <falcon/expression.h>
#include <falcon/item.h>
#include <falcon/vm.h>
#include <falcon/pcode.h>
#include <falcon/exprfactory.h>
#include <falcon/operanderror.h>
#include <falcon/codeerror.h>
#include <falcon/itemarray.h>
#include <falcon/symbol.h>

namespace Falcon {

class ExprMultiUnpack::Private {
public:
   std::vector<Symbol*> m_params;
   std::vector<Expression*> m_assignee;
};

//=========================================================
// Unpack

ExprMultiUnpack::ExprMultiUnpack( bool isTop ):
   Expression(t_multiunpack),
   m_bIsTop( isTop ),
   _p( new Private )
{
   apply = apply_;
}

ExprMultiUnpack::ExprMultiUnpack( const ExprMultiUnpack& other ):
         Expression(other)
{
   _p = new Private;

   _p->m_params.reserve(other._p->m_params.size());
   std::vector<Symbol*>::const_iterator iter = other._p->m_params.begin();
   while( iter != other._p->m_params.end() )
   {
      _p->m_params.push_back( *iter );
      ++iter;
   }

   _p->m_assignee.reserve(other._p->m_assignee.size());
   std::vector<Expression*>::const_iterator itere = other._p->m_assignee.begin();
   while( itere != other._p->m_assignee.end() )
   {
      _p->m_assignee.push_back( (*itere)->clone() );
      ++itere;
   }

}

ExprMultiUnpack::~ExprMultiUnpack()
{
   std::vector<Expression*>::const_iterator iter = _p->m_assignee.begin();
   while( iter != _p->m_assignee.end() )
   {
      delete *iter;
      ++iter;
   }
}

bool ExprMultiUnpack::simplify( Item& ) const
{
   return false;
}

void ExprMultiUnpack::describeTo( String& ret ) const
{
   String params;
   
   for( unsigned int i = 0; i < _p->m_params.size(); ++i )
   {
      if ( i >= 1 )
      {
         params += ", ";
      }
      params += _p->m_params[i]->name();
   }

   ret = params + " = ";

   for( unsigned int i = 0; i < _p->m_assignee.size(); ++i )
   {
      if ( i >= 1 )
      {
         params += ", ";
      }

      params += _p->m_assignee[i]->describe();
   }
}


int ExprMultiUnpack::targetCount() const
{
   return _p->m_params.size();
}

Symbol* ExprMultiUnpack::getAssignand( int i) const
{
   return _p->m_params[i];
}

Expression* ExprMultiUnpack::getAssignee( int i) const
{
   return _p->m_assignee[i];
}


ExprMultiUnpack& ExprMultiUnpack::addAssignment( Symbol* e, Expression* expr)
{
   // save exprs and symbols in a parallel array
   _p->m_params.push_back(e);
   _p->m_assignee.push_back(expr);

   return *this;
}


void ExprMultiUnpack::precompile( PCode* pcode ) const
{
   TRACE3( "Precompiling multi unpack: %p (%s)", pcode, describe().c_ize() );

   // First, compile the assigment expressions.
   std::vector<Expression*>::const_iterator iter = _p->m_assignee.begin();
   while( iter != _p->m_assignee.end() )
   {
      (*iter)->precompile( pcode );
      ++iter;
   }

   // then push ourselves
   pcode->pushStep( this );
}


void ExprMultiUnpack::apply_( const PStep* ps, VMContext* ctx )
{
   TRACE3( "Apply multi unpack: %p (%s)", ps, ps->describe().c_ize() );

   ExprMultiUnpack* self = (ExprMultiUnpack*) ps;
   std::vector<Symbol*> &syms = self->_p->m_params;

   size_t pcount = syms.size();

   size_t i = 0;
   Item* topStack = &ctx->topData() - pcount+1;
   for( ; i < pcount; ++i, ++topStack )
   {
      *syms[i]->value( ctx ) = *topStack;
   }
   
   if ( self->isTop() )
   {
      // no need to create an array if noone is using it
      ctx->popData(pcount-1);
      ctx->topData().setNil();
   }
   else
   {
      ItemArray* retval = new ItemArray(pcount);
      i = 0;
      topStack = &ctx->topData() - pcount+1;
      for( ; i < pcount; ++i, ++topStack )
      {
         (*retval)[i] = *topStack;
      }
      ctx->popData(pcount-1);
      ctx->topData().setArray( retval );
   }
}

}

/* end of exprmultiunpack.cpp */
