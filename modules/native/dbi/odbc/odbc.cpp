/* 
   FALCON - The Falcon Programming Language.
   FILE: odbc.cpp
 
   ODBC driver for DBI - main module
 
   This is BOTH a driver for the DBI interface AND a standalone
   ODBC module.
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Tue Sep 30 17:00:00 2008
  
   -------------------------------------------------------------------
   (C) Copyright 2010: the FALCON developers (see list in AUTHORS file)
  
   See LICENSE file for licensing details.
*/

#include "odbc_mod.h"
#include "version.h"
#include "odbc_ext.h"

/*#
   @module dbi.odbc ODBC driver module.
   @brief DBI extension supporting ODBC connections.


   Directly importable as @b dbi.odbc, it is usually loaded through
   the @a dbi module.
*/

// Instantiate the driver service
Falcon::DBIServiceODBC theODBCService;

// the main module
FALCON_MODULE_DECL
{
   // Module declaration
   Falcon::Module *self = new Falcon::Module();
   self->name( "odbc" );
   self->engineVersion( FALCON_VERSION_NUM );
   self->version( VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION );
   
   // first of all, we need to declare our dependency from the DBI module.
   self->addDepend( "dbi", "dbi", true, false );

   // also, we declare a ODBC class, which derives from DBIHandler which
   // is in the DBI module.
   Falcon::Symbol *dbh_class = self->addExternalRef( "dbi.%Handle" ); // it's external
   dbh_class->imported( true );
   Falcon::Symbol *odbc_class = self->addClass( "ODBC", Falcon::Ext::ODBC_init )
      ->addParam("connect")->addParam("options");
   odbc_class->getClassDef()->addInheritance( new Falcon::InheritDef( dbh_class ) );
   odbc_class->setWKS( true );

   // service publication
   self->publishService( &theODBCService );

   // we're done
   return self;
}

/* end of odbc.cpp */
