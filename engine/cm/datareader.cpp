/*
   FALCON - The Falcon Programming Language.
   FILE: datareader.cpp

   Falcon core module -- Wrapping for DataReader
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Sun, 11 Dec 2011 17:32:58 +0100

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#define SRC "engine/datareader.cpp"

#include <falcon/cm/datareader.h>

#include <falcon/datareader.h>
#include <falcon/stdsteps.h>
#include <falcon/module.h>
#include <falcon/classes/classstream.h>

#include <falcon/errors/accesserror.h>
#include <falcon/errors/paramerror.h>
#include <falcon/vmcontext.h>

namespace Falcon {
namespace Ext {

ClassDataReader::ClassDataReader( Class* clsStream ):
   ClassUser("DataReader"),
   m_clsStream( clsStream ),
   FALCON_INIT_PROPERTY( endianity ),
   FALCON_INIT_PROPERTY( sysEndianity ),
   
   FALCON_INIT_METHOD( read ),
   FALCON_INIT_METHOD( readBool ),
   FALCON_INIT_METHOD( readChar ),
   FALCON_INIT_METHOD( readByte ),
   FALCON_INIT_METHOD( readI16 ),
   FALCON_INIT_METHOD( readU16 ),
   FALCON_INIT_METHOD( readI32 ),
   FALCON_INIT_METHOD( readU32 ),
   FALCON_INIT_METHOD( readI64 ),
   FALCON_INIT_METHOD( readU64 ),
   FALCON_INIT_METHOD( readF32 ),
   FALCON_INIT_METHOD( readF64 ),
   FALCON_INIT_METHOD( readString ),
   FALCON_INIT_METHOD( readItem ),
   
   FALCON_INIT_METHOD( sync ),
   FALCON_INIT_METHOD( eof ),
   
   m_readItemNext( this )
   
{
}

ClassDataReader::~ClassDataReader()
{
}

   
void ClassDataReader::dispose( void* instance ) const
{
   DataReader* wr = static_cast<DataReader*>(instance);
   delete wr;
}


void* ClassDataReader::clone( void* instance ) const
{
   // nothing to clone, it can be shared.
   DataReader* wr = static_cast<DataReader*>(instance);
   return new DataReader(*wr);
}


void ClassDataReader::gcMarkInstance( void* instance, uint32 mark ) const
{
   DataReader* wr = static_cast<DataReader*>(instance);
   wr->gcMark( mark );
}


bool ClassDataReader::gcCheckInstance( void* instance, uint32 mark ) const
{
   DataReader* wr = static_cast<DataReader*>(instance);
   return wr->gcMark() >= mark;
}


void* ClassDataReader::createInstance() const
{
   return new DataReader;
}


bool ClassDataReader::op_init( VMContext* ctx, void* instance, int pcount ) const
{
   if( pcount >= 1 )
   {
      Class* cls=0;
      void* data=0;
      
      Item* params = ctx->opcodeParams(pcount);
      params[0].asClassInst( cls, data );
      if( cls->isDerivedFrom(m_clsStream) )
      {
         DataReader* wr = static_cast<DataReader*>( instance );
         wr->changeStream(
               static_cast<StreamCarrier*>(data)->m_underlying, false, false );
         // this data is going to be added to gc very soon.
         return wr;
      }      
   }
   
   throw new ParamError( ErrorParam( e_inv_params, __LINE__, SRC )
         .origin(ErrorParam::e_orig_runtime)
         .extra( "Stream" ) );      
}


//=================================================================
// Properties
//

FALCON_DEFINE_PROPERTY_SET_P( ClassDataReader, endianity )
{
   DataReader* sc = static_cast<DataReader*>(instance);
     
   if( !value.isOrdinal() )
   {
      // unknown encoding
      throw new ParamError( ErrorParam( e_inv_params, __LINE__, SRC )
         .origin( ErrorParam::e_orig_runtime )
         .extra( "N" ));
   }
   
   int64 edty = (int64) value.forceInteger();
   if( edty == DataReader::e_BE || edty == DataReader::e_LE
       || edty == DataReader::e_reverseEndian || edty == DataReader::e_sameEndian )
   {
      sc->setEndianity( (DataReader::t_endianity) edty );
   }
   else
   {
      throw new ParamError( ErrorParam( e_param_range, __LINE__, SRC )
         .origin( ErrorParam::e_orig_runtime )
         .extra( "Not a valid endianity setting" ));
   }
}


FALCON_DEFINE_PROPERTY_GET_P( ClassDataReader, endianity )
{
   DataReader* sc = static_cast<DataReader*>(instance);
   value = (int64)sc->endianity();
}


FALCON_DEFINE_PROPERTY_SET( ClassDataReader, sysEndianity )( void*, const Item& )
{
   throw new AccessError( ErrorParam( e_prop_ro, __LINE__, SRC )
      .origin( ErrorParam::e_orig_runtime )
      .extra( "sysEndianity" ));
}


FALCON_DEFINE_PROPERTY_GET_P( ClassDataReader, sysEndianity )
{
   DataReader* sc = static_cast<DataReader*>(instance);
   DataReader::t_endianity edity = sc->endianity();
   if( sc->isSameEndianity() )
   {
      value = (int64)edity; 
   }
   else {
      value = (int64) (edity == DataReader::e_LE ? DataReader::e_BE : DataReader::e_LE );
   }
}

//=================================================================
// Methods
//

FALCON_DEFINE_METHOD_P1( ClassDataReader, read )
{
   Item* i_data = ctx->param(0);
   if( i_data == 0 || !(i_data->isString()||i_data->isMemBuf()) )
   {
      throw paramError();
   }
   
   Item* i_count = ctx->param(1);
   
   if ( (i_count != 0 && !(i_count->isOrdinal() || i_count->isNil())) 
      )
   {
      throw paramError();
   }
   
   uint32 dataSize = i_data->asString()->size();
   uint32 count = dataSize;  
      
   if( i_count != 0 && ! i_count->isNil() )
   {      
      count = (uint32) i_count->forceInteger();
   }
   
   if( count == 0 )
   {
      // nothing to do
      ctx->returnFrame( (int64) 0 );
      return;
   }
   
   if( count > dataSize )
   {
      i_data->asString()->reserve( count );
   }
   
   byte* dataSource = i_data->asString()->getRawStorage();
   DataReader* sc = static_cast<DataReader*>(ctx->self().asInst());
   int64 retval = (int64) sc->read(dataSource, count);   
   ctx->returnFrame( retval );   
}


FALCON_DEFINE_METHOD_P1( ClassDataReader, readBool )
{   
   bool bValue;
   DataReader* dw = static_cast<DataReader*>(ctx->self().asInst());
   dw->read(bValue);
   Item retVal;
   retVal.setBoolean(bValue);
   ctx->returnFrame( retVal );
}


FALCON_DEFINE_METHOD_P1( ClassDataReader, readChar )
{
   char chr;   
   DataReader* dw = static_cast<DataReader*>(ctx->self().asInst());
   dw->read(chr);
   ctx->returnFrame((int64) chr);
}

FALCON_DEFINE_METHOD_P1( ClassDataReader, readByte )
{
   byte value;   
   DataReader* dw = static_cast<DataReader*>(ctx->self().asInst());
   dw->read(value);
   ctx->returnFrame((int64) value);
}

FALCON_DEFINE_METHOD_P1( ClassDataReader, readI16 )
{
   int16 value;   
   DataReader* dw = static_cast<DataReader*>(ctx->self().asInst());
   dw->read(value);
   ctx->returnFrame((int64) value);
}


FALCON_DEFINE_METHOD_P1( ClassDataReader, readU16 )
{
   uint16 value;   
   DataReader* dw = static_cast<DataReader*>(ctx->self().asInst());
   dw->read(value);
   ctx->returnFrame((int64) value);
}


FALCON_DEFINE_METHOD_P1( ClassDataReader, readI32 )
{
   int32 value;   
   DataReader* dw = static_cast<DataReader*>(ctx->self().asInst());
   dw->read(value);
   ctx->returnFrame((int64) value);
}


FALCON_DEFINE_METHOD_P1( ClassDataReader, readU32 )
{
   uint16 value;   
   DataReader* dw = static_cast<DataReader*>(ctx->self().asInst());
   dw->read(value);
   ctx->returnFrame((int64) value);
}


FALCON_DEFINE_METHOD_P1( ClassDataReader, readI64 )
{
   int64 value;   
   DataReader* dw = static_cast<DataReader*>(ctx->self().asInst());
   dw->read(value);
   ctx->returnFrame((int64) value);
}


FALCON_DEFINE_METHOD_P1( ClassDataReader, readU64 )
{
   uint64 value;   
   DataReader* dw = static_cast<DataReader*>(ctx->self().asInst());
   dw->read(value);
   ctx->returnFrame((int64) value);
}


FALCON_DEFINE_METHOD_P1( ClassDataReader, readF32 )
{
   float value;   
   DataReader* dw = static_cast<DataReader*>(ctx->self().asInst());
   dw->read(value);
   ctx->returnFrame((numeric) value);
}


FALCON_DEFINE_METHOD_P1( ClassDataReader, readF64 )
{
   double value;   
   DataReader* dw = static_cast<DataReader*>(ctx->self().asInst());
   dw->read(value);
   ctx->returnFrame((numeric) value);
}


FALCON_DEFINE_METHOD_P1( ClassDataReader, readString )
{
   static Class* stringClass = Engine::instance()->stringClass();
   String* str = new String;
   DataReader* dw = static_cast<DataReader*>(ctx->self().asInst());
   try
   {
      dw->read(*str);
   }
   catch( ... )
   {
      delete str;
      throw;
   }
   
   ctx->returnFrame( FALCON_GC_STORE( stringClass, str ) );
}


FALCON_DEFINE_METHOD_P1( ClassDataReader, readItem )
{
   static StdSteps* steps = Engine::instance()->stdSteps();

   Item* i_data = ctx->param(0);
   if( i_data == 0 || ! i_data->isClass() )
   {
      ctx->raiseError( paramError() );
      return;
   }
   
   DataReader* dw = static_cast<DataReader*>(ctx->self().asInst());
   Class* cls = static_cast<Class*>(i_data->asInst());
   
   // store may go deep, so we use a conditional return construct
   ctx->pushCode( &static_cast<ClassDataReader*>(m_methodOf)->m_readItemNext );
   
   long depth = ctx->codeDepth();
   
   // this will punch in later on...
   ctx->pushCode( &steps->m_returnFrameWithTop );
   cls->restore( ctx, dw );
   
   // ... if the restore process isn't done.
   if( depth == ctx->codeDepth() )
   {
      ctx->returnFrame( ctx->topData() );
   }
}


FALCON_DEFINE_METHOD_P1( ClassDataReader, sync )
{
  DataReader* dw = static_cast<DataReader*>(ctx->self().asInst());
  dw->sync();  
  ctx->returnFrame();
}


FALCON_DEFINE_METHOD_P1( ClassDataReader, eof )
{
  DataReader* dw = static_cast<DataReader*>(ctx->self().asInst());
  Item ret;
  ret.setBoolean(dw->eof());  
  ctx->returnFrame( ret );
}

void ClassDataReader::ReadItemNext::apply_( const PStep*, VMContext* ctx )
{
   // we just got to return our good item
   //ClassDataReader::ReadItemNext* self = static_cast<ClassDataReader::ReadItemNext*>(ps);   
   ctx->returnFrame( *ctx->local(0) );
}

}
}

/* end of datareader.cpp */
