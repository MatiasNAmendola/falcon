/*
   FALCON - The Falcon Programming Language.
   FILE: serialize.cpp

   Serialization support
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: sab nov 13 2004

   -------------------------------------------------------------------
   (C) Copyright 2004: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

/** \file
   Serialization support
*/

#include <falcon/setup.h>
#include <falcon/module.h>
#include <falcon/item.h>
#include <falcon/vm.h>
#include <falcon/string.h>
#include <falcon/carray.h>
#include <falcon/cdict.h>
#include <falcon/memory.h>

#include <falcon/stream.h>

/*#
   
*/

/*#
   @funset rtl_serialization Serialization functions
   @brief Function used to store items persistently.

   Serialization functions allow to flatten a Falcon item,
   or a sequence of items, on a stream for later retrieval,
   storage or transmission. At the moment, custom serialization is not supported.
   This means that all the basic items, as strings and numbers, plus arrays and
   dictionaries are supported. Objects are partially supported: when they are fully
   derived from Falcon classes, or declared as “object” by the scripts, the
   serialization and de-serialization are successful. However, there is no
   mechanism to support creation of user-specific data, as the “load” that objects
   can carry internally in behalf of embedding applications.

   Nevertheless, if there is the need, objects may be serialized/deserialized with
   the provided functions, and after the de-serialization step, a custom mechanism
   may be used to re-create application specific data.

   However, it is necessary that the deserializing application has access to the same
   classes that were used to create the serialized object.

   Notice that also functions are correctly serialized and deserialized.
   Also, static block is not re-executed in case the function is re-entered
   after a de-serialization.

   @beginset core_serialization
*/

namespace Falcon {
namespace core {

/*#
   @function serialize
   @brief Serializes an item on a stream.
   @param stream An instance of the Stream (or derived) class.
   @param item The item to be serialized.
   @raise IoError on underlying stream error.

   The item is stored on the stream so that a deserialize() call on the same
   position in the stream where serialization took place will create an exact
   copy of the serialized item.

   The application must ensure that the item does not contains circular
   references, or the serialization will enter an endless loop.

   In case the underlying stream write causes an i/o failure, an error
   is raised.

   The BOM method @a BOM.serialize is available for all the Falcon items,
   and is equivalent to call this function. Also, the method
   @a Stream.writeItem is equivalent to this function; the difference in
   BOM.serialize() and Stream.writeItem() is only in the fact that
   Item.serialize() accepts a stream on which to perform the serialization as the
   parameter, while Stream.writeItem() must be provided with the item to be
   serialized.
*/
FALCON_FUNC  serialize ( ::Falcon::VMachine *vm )
{
   Item *fileId = vm->param(0);
   Item *source = vm->param(1);

   if( fileId == 0 || source == 0 || ! fileId->isObject() || ! fileId->asObject()->derivedFrom( "Stream" ) )
   {
      vm->raiseModError( new ParamError( ErrorParam( e_inv_params, __LINE__ ).origin( e_orig_runtime ).
         extra( "X,O:Stream" ) ) );
      return;
   }


   Stream *file = (Stream *) fileId->asObject()->getUserData();
   Item::e_sercode sc = source->serialize( file );
   switch( sc )
   {
      case Item::sc_ok: vm->retval( 1 ); break;
      case Item::sc_ferror: vm->raiseModError( new IoError( ErrorParam( e_modio, __LINE__ ).origin( e_orig_runtime ) ) );
      default:
         vm->retnil(); // VM may already have raised an error.
         //TODO: repeat error.
   }
}

/*#
   @function deserialize
   @brief Deserialize an item from a stream.
   @param stream An instance of the Stream (or derived) class.
   @raise IoError on underlying stream error.
   @raise GenericError If the data is correctly de-serialized, but it refers to
         external symbols non defined by this script.
   @raise ParseError if the format of the input data is invalid.

   The returned item is a new copy of the item that has been previously serialized
   on the given stream. After the read, the stream pointer is left ready for a
   new read, so that items that are serialized in sequence may be deserialized
   in the same order.

   If the underlying stream read causes an i/o failure, an error is raised.

   Also, an error is raised if the function cannot deserialize from the stream
   because the data format is invalid. This call is equivalent to call
   @a Stream.readItem method.
*/

FALCON_FUNC  deserialize ( ::Falcon::VMachine *vm )
{
   Item *fileId = vm->param(0);

   if( fileId == 0 || ! fileId->isObject() || ! fileId->isObject() || ! fileId->asObject()->derivedFrom( "Stream" ) )
   {
      vm->raiseModError( new ParamError( ErrorParam( e_inv_params, __LINE__ ).origin( e_orig_runtime ).
         extra( "O:Stream" ) ) );
      return;
   }

   // deserialize rises it's error if it belives it should.
   Stream *file = (Stream *) fileId->asObject()->getUserData();
   Item::e_sercode sc = vm->regA().deserialize( file, vm );
   switch( sc )
   {
      case Item::sc_ok: return; // ok, we've nothing to do
      case Item::sc_ferror: vm->raiseModError( new IoError( ErrorParam( e_modio, __LINE__ ).origin( e_orig_runtime ) ) );
      case Item::sc_misssym: vm->raiseModError( new GenericError( ErrorParam( e_undef_sym, __LINE__ ).origin( e_orig_runtime ) ) );
      case Item::sc_missclass: vm->raiseModError( new GenericError( ErrorParam( e_undef_sym, __LINE__ ).origin( e_orig_runtime ) ) );
      case Item::sc_invformat: vm->raiseModError( new ParseError( ErrorParam( e_invformat, __LINE__ ).origin( e_orig_runtime ) ) );

      case Item::sc_vmerror:
      default:
         vm->retnil(); // VM may already have raised an error.
         //TODO: repeat error.
   }
}

}}
/* end of serialize.cpp */