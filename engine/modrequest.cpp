/*
   FALCON - The Falcon Programming Language.
   FILE: modrequest.cpp

   Structure recording the information about foreign module load.
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Sun, 12 Feb 2012 14:13:14 +0100

   -------------------------------------------------------------------
   (C) Copyright 2012: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#include <falcon/modrequest.h>
#include <falcon/importdef.h>
#include <falcon/module.h>

#include <falcon/datawriter.h>
#include <falcon/datareader.h>

#include <deque>
#include <algorithm>

namespace Falcon {

class ModRequest::ImportDefList: public std::deque<ImportDef*>
{
public:
   ImportDefList() {}
   ~ImportDefList() {}
};

ModRequest::ModRequest():
   m_idl( new ImportDefList ),
   m_isLoad( false ),
   m_bIsURI( false ),
   m_module( 0 ),
   m_id(-1)
{}
   
ModRequest::ModRequest( const String& name, bool isUri, bool isLoad, Module* mod):
   m_idl( new ImportDefList ),
   m_name( name ),
   m_isLoad( isLoad ),
   m_bIsURI( isUri ),
   m_module( mod ),
   m_id(-1)
{}
   
ModRequest::ModRequest( const ModRequest& other ):
   m_idl( new ImportDefList ),
   m_name( other.m_name ),
   m_isLoad( other.m_isLoad ),
   m_bIsURI( other.m_bIsURI ),
   m_module( other.m_module ),
   m_id(-1)
{
   if ( m_module != 0 ) {
      m_module->incref();
   }
}
   
ModRequest::~ModRequest()
{
   delete m_idl;
   if( m_module != 0 ) {
      m_module->decref();
   }
}

void ModRequest::module( Module* mod )
{
   if( m_module != 0  ) {
      m_module->decref();
   }

   if( mod != 0 ) {
      mod->incref();
   }

   m_module = mod;
}

void ModRequest::addImportDef( ImportDef* id )
{
   m_idl->push_back( id );
}

void ModRequest::removeImportDef( ImportDef* id )
{
   ImportDefList::iterator pos = 
      std::find( m_idl->begin(), m_idl->end(), id );
   
   if( pos !=  m_idl->end() )
   {
      m_idl->erase(pos);
   }
}

ImportDef* ModRequest::importDefAt( int n ) const
{
   return (*m_idl)[n];
}

int ModRequest::importDefCount() const
{
   return (int) m_idl->size();
}


void ModRequest::store( DataWriter* wr ) const
{
   wr->write( m_name );
   wr->write( m_isLoad );
   wr->write( m_bIsURI );
}


void ModRequest::restore( DataReader* rd )
{
   rd->read( m_name );
   rd->read( m_isLoad );
   rd->read( m_bIsURI );
}

   
}

/* end of modrequest.cpp */