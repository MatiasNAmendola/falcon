/*
   FALCON - The Falcon Programming Language.
   FILE: falcon.h

   Falcon compiler and interpreter
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Mon, 23 Mar 2009 18:57:37 +0100

   -------------------------------------------------------------------
   (C) Copyright 2009: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

/** \file
   Main Falcon.
*/

#ifndef FALCON_CLT_H
#define FALCON_CLT_H

#include <falcon/falcon.h>

#include "options.h"
#include "int_mode.h"

namespace Falcon {

class FalconApp: public Falcon::Application
{

public:
   int m_exitValue;
   FalconOptions m_options;
   
   /**
    * The logger for the Falcon application.
    *
    * \TODO Add stream/stderr control in command line.
    */
   class Logger: public Log::Listener
   {
   protected:
      virtual void onMessage( int fac, int lvl, const String& message );
   };

   Logger* m_logger;

   FalconApp();
   ~FalconApp();
   
   void guardAndGo( int argc, char* argv[] );
   void interactive();
   void launch( const String& script );

private:
   // we want a VM for this application.
   VMachine* m_vm;
   // and we'll have a single process to run.
   Process* m_process;
};

} // namespace Falcon

#endif

/* end of falcon.h */