/*
   FALCON - The Falcon Programming Language.
   FILE: exprcall.cpp

   Expression controlling item (function) call
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Sat, 11 Jun 2011 21:19:26 +0200

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#ifndef _FALCON_EXPRCALL_H_
#define _FALCON_EXPRCALL_H_

#include <falcon/expression.h>
#include <falcon/pseudofunc.h>

namespace Falcon {

class FALCON_DYN_CLASS ExprCall: public Expression
{
public:
   ExprCall( Expression* op1 );

   /** Create a call-through-pseudo function.
    Calls through pseudofunctions are performed by pushing the
    pseudofunction PStep instead of using this expression psteps.
    */
   ExprCall( PseudoFunction* func );

   ExprCall( const ExprCall& other );
   virtual ~ExprCall();

   inline virtual ExprCall* clone() const { return new ExprCall( *this ); }
   virtual bool simplify( Item& value ) const;
   virtual void describeTo( String& ) const;

   int paramCount() const;
   Expression* getParam( int n ) const;
   ExprCall& addParam( Expression* );

   inline virtual bool isStandAlone() const { return false; }

   virtual bool isStatic() const { return false; }


   /** Returns the pseudofunction associated with this call.
    \return Pseudofunction associated with this expression, or 0 if none.

    If this expression call is actually calling a pseudofunction,
    this will return a non-zero pointer to a PseudoFunction stored
    in the Engine.
    */
   PseudoFunction* pseudo() const { return m_func; }

protected:
   inline ExprCall();
   PseudoFunction* m_func;
   Expression* m_callExpr;

private:
   class Private;
   Private* _p;

   static void apply_( const PStep*, VMContext* ctx );
};

}

#endif

/* end of exprcall.h */