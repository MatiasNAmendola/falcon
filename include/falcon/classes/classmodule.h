/*
   FALCON - The Falcon Programming Language.
   FILE: classmodule.h

   Module object handler.
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Wed, 22 Feb 2012 19:50:45 +0100

   -------------------------------------------------------------------
   (C) Copyright 2012: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#ifndef _FALCON_CLASSMODULE_H_
#define _FALCON_CLASSMODULE_H_

#include <falcon/setup.h>
#include <falcon/class.h>

namespace Falcon
{

/** Class handling a whole module.
 */

class FALCON_DYN_CLASS ClassModule: public Class
{
public:

   ClassModule();
   virtual ~ClassModule();

   virtual void dispose( void* self ) const;
   virtual void* clone( void* source ) const;
   virtual void* createInstance() const;
   
   virtual void store( VMContext* ctx, DataWriter* stream, void* instance ) const;
   virtual void restore( VMContext* ctx, DataReader* stream ) const;
   virtual void flatten( VMContext* ctx, ItemArray& subItems, void* instance ) const;
   virtual void unflatten( VMContext* ctx, ItemArray& subItems, void* instance ) const;

   virtual void describe( void* instance, String& target, int maxDepth = 3, int maxLength = 60 ) const;

   //=============================================================

   virtual bool op_init( VMContext* ctx, void* instance, int32 pcount ) const;
   
private:
   void restoreModule( Module* mod, DataReader* stream ) const;
};

}

#endif

/* end of classmodule.h */