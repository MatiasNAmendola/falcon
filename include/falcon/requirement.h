/*
   FALCON - The Falcon Programming Language.
   FILE: requirement.h

   ABC holding information about classes needed elsewhere.
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Mon, 15 Aug 2011 13:58:12 +0200

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#ifndef _FALCON_REQUIREMENT_H_
#define _FALCON_REQUIREMENT_H_

#include <falcon/setup.h>
#include <falcon/string.h>
#include <falcon/sourceref.h>

namespace Falcon
{

class Error;
class Module;
class Symbol;
class ItemArray;
class DataWriter;
class DataReader;
class Class;
class Item;
class Variable;

/** Functionoid for delayed resolution of symbols.
 
  Requirements are the Falcon version of a "forward declaration". When a compiler
    that can accept undefined symbols in some context finds a unresolved name
    in an expression (i.e. a branch in a switch, or an inheritance in a class),
    it asks for a requirement, that can be resolved at different times:
    - immediately, if the entity is actually already defined somewhere.
    - later on during the compilation of the same module.
    - at link step when importing symbols from other modules.
    
    The requirement class doesn't just take care of requesting for an temporarily 
    unknown symbol, it's also a callback point that, once the symbol is resolved,
    gets called and performs the operations that was left pending. For instance,
    a class inheriting from a requirement will need to check if the imported
    symbols is really a class itself, and if all the forward declarations get
    resolved, it will need to create the complete class entity in the host module;
    if not, it will need to throw an exception.
 
 Third party native modules can create requirements as an easy way to import
 symbols from the system; for instance, a class or function that is needed by
 the module to perform some operations might be resolved at link step via
 a requirement instead of searching for it at runtime.
 
 In that case, the native module writer may want to create a subclass and
 a static instance of the requirement to access some static data in the 
 module. The requirement objects are usually dynamic and owned by the 
 base class Module, but to prevent the base class to destroy the requirement,
 it can be created with the \b bIsStatic flag set to true.
 
 To add a "generic" requirement, that is, a requirement that can be resolved by
 the engine via any possible mean (e.g. via other modules exported symbols or
 via static entities in the engine), just add the requirement to the module
 via Module::addRequirement.
 
 For more specific requests, for instance, for requirements insisting on symbols
 directly imported from a specific module, use Module::addImportRequest.
 
 Requirements have a Class handler called ClassRequirement. The class handler
 is not meant to be used by scripts (in fact, it's pure abstract at Falcon
 level), but is used to allow the Requirement to be a part of a Syntree, or
 in other words, to participate grammar reflection. This is practically used
 only during storage of entities that need a requirement in the syntactic
 tree; for instance, in class declaration or forward constrained symbol
 declarations (i.e. in switch branches).
 
 As such, they support serialization. The base implementation of store, restore,
 flatten and unflatten do nothing. Requirements used in third party modules
 rarely need to configure themselves, as only non-native modules require
 full serialization; native modules just serialize a mean to re-load the
 module from the source where they was created from (for instance, the URL
 of their DLL), and don't ask a requirement to be serialized.
 */
class FALCON_DYN_CLASS Requirement
{
public:
   /** Creates the requirement.
    \param name The symbol name that is expected to be resolved.
    \param bIsStatic if false (default), the requirement is disposed by the host module.
    
    The symbol name shall include namespaces.
    
    If this requirement is created statically somewhere, the bIsStatic parameter
    should be set to true so that the Module class instance where it will be stored
    won't destroy it.
    */
   Requirement( const String& name, bool bIsStatic = false ) :
      m_bIsStatic( bIsStatic ),
      m_name( name )
   {}
      

   Requirement( const String& name, int line, int chr, bool bIsStatic = false ) :
      m_bIsStatic( bIsStatic ),
      m_sr( line, chr ),
      m_name( name )
   {}
      
   virtual ~Requirement() {}
   
   /** Called back when the requirement is resolved.
    \param sourceModule The module where the requirement is found -- might be 0 if
           generated internally.
    \param sourceName The name of the variable answering the requirement.
    \param targetModule The module that asked for the requirement to be resolved
             (it's usually "this" module, or the module hosting the requirement owner).
    \param value The concrete value that was found in the source module for the required name.
    \param targetVar; a pointer to a variable i the targetModule where the value is stored.

    \throw A CodeError may be thrown by the implementation to indicate some kind of
          rule being broken (e.g. the requirement for inheritances must be classes).

    
    The pointer parameters can be zero in the following cases:
    - The source module can be 0 if the symbol is internally generated.
    - The target module can be 0 if the system resolves the dependency on its own.
    - The target variable might be 0 if the requirement is not to be stored in a local
       target module variable.
    
    The source and target modules can be the same module if a requirement is resolved by
    a forward definition.

    The sourceName is given for reference, to help the code to understand what was resolved.
    The name of the variable in which the resolved requirement is imported can be taken by
    invoking
    \code
       targetModule->getVariableName( targetVar->id() ).
    \endcode

    \note targetVar being not zero implies also targetModule being not zero.
    */
   virtual void onResolved( const Module* sourceModule, const String& sourceName, Module* targetModule, const Item& value, const Variable* targetVar ) = 0;

   /** Returns the symbol name that is associated with this requirement.
      \return A name.
    */
   const String& name() const { return m_name; }
   
   /** Returns whether this requirement is static or not.
    Static requirements are statically allocated and should not be deleted
    by the host module.
      
    They can be stored in third party native modules, and destroyed when the
    module is unloated.
    
    The staticity of a requirement is determined at construction.
    */
   bool isStatic() const { return m_bIsStatic; }
   
   const SourceRef& sourceRef() const { return m_sr; }
   SourceRef& sourceRef() { return m_sr; }

   virtual void store( DataWriter* stream ) const;
   virtual void restore( DataReader* stream );
   virtual void flatten( ItemArray& subItems ) const;
   virtual void unflatten( ItemArray& subItems );

   /** Returns the class handling this kind of requirement. 
    
    Subclasses can set this to something returning 0 if this
    requirement is only used in native modules.
    */
   virtual Class* cls() const = 0;
   
protected:
   bool m_bIsStatic;
   
private:
   SourceRef m_sr;
   String m_name;
};

}

#endif /* _FALCON_REQUIREMENT_H_ */

/* end of requirement.h */