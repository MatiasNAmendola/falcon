/*
   FALCON - The Falcon Programming Language.
   FILE: intcompiler.cpp

   Interactive compiler - step by step dynamic compiler
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Sun, 17 Apr 2011 21:57:04 +0200

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#undef SRC
#define SRC "engine/intcompiler.cpp"

#include <falcon/intcompiler.h>
#include <falcon/module.h>
#include <falcon/symbol.h>
#include <falcon/textwriter.h>

#include <falcon/stringstream.h>
#include <falcon/textreader.h>
#include <falcon/textwriter.h>
#include <falcon/statement.h>
#include <falcon/psteps/stmtautoexpr.h>

#include <falcon/parser/parser.h>
#include <falcon/sp/sourcelexer.h>
#include <falcon/error.h>
#include <falcon/modspace.h>

#include <falcon/expression.h>
#include <falcon/falconclass.h>
#include <falcon/hyperclass.h>
#include <falcon/modloader.h>
#include <falcon/requirement.h>

#include <falcon/errors/genericerror.h>
#include <falcon/errors/syntaxerror.h>
#include <falcon/errors/codeerror.h>
#include <falcon/errors/ioerror.h>

#include <falcon/vm.h>

#include <falcon/psteps/exprvalue.h>

#include <falcon/datareader.h>
#include <falcon/importdef.h>
#include <falcon/synclasses_id.h>
#include <falcon/process.h>

namespace Falcon {

//=======================================================================
// context callback
//

IntCompiler::Context::Context(IntCompiler* owner):
   ParserContext( &owner->m_sp ),
   m_owner(owner)
{
}

IntCompiler::Context::~Context()
{
   // nothing to do
}

void IntCompiler::Context::onInputOver()
{
   //std::cout<< "CALLBACK: Input over"<<std::endl;
}


void IntCompiler::Context::onNewFunc( Function* function )
{
   SourceParser& sp = m_owner->m_sp;

   bool result;
   if( function->name() == "" ) {
      result = true;
      m_owner->m_mod->addAnonMantra(function);
   }
   else {
      result = m_owner->m_mod->addMantra(function);
   }

   if( ! result ) {
      m_owner->m_sp.addError( e_already_def, sp.currentSource(), sp.currentLine()-1, 0, 0,
               function->name() );
      delete function;
      return;
   }

   m_owner->m_currentMantra = function;
}


void IntCompiler::Context::onNewClass( Class* cls, bool )
{
   SourceParser& sp = m_owner->m_sp;

   bool result;
   if( cls->name() == "" ) {
      result = true;
      m_owner->m_mod->addAnonMantra(cls);
   }
   else {
      result = m_owner->m_mod->addMantra(cls);
   }

   if( ! result ) {
      m_owner->m_sp.addError( e_already_def, sp.currentSource(), sp.currentLine()-1, 0, 0,
               cls->name() );
      delete cls;
      return;
   }

   FalconClass* fcls = static_cast<FalconClass*>(cls);
   if( fcls->missingParents() == 0 )
   {
      try
      {
         if( ! fcls->construct() )
         {
            m_owner->m_currentMantra = fcls->hyperConstruct();
         }
         else
         {
            m_owner->m_currentMantra = fcls;
         }
      }
      catch( Error* e )
      {
         m_owner->m_sp.addError( e );
         e->decref();
      }
   }
   else
   {
      // we have already raised undefined symbol error.
      // get rid of the class and of all its deps.
      delete cls;
   }
}


void IntCompiler::Context::onNewStatement( Statement* )
{
   // suspend parsing if we're at top level.
}


void IntCompiler::Context::onLoad( const String& path, bool isFsPath )
{
   SourceParser& sp = m_owner->m_sp;
   if( ! m_owner->m_bAllowDirective )
   {
      sp.addError( e_directive_not_allowed, sp.currentSource(), sp.currentLine()-1, 0, 0 );
      return;
   }

   // get the module space
   ModSpace* theSpace = m_owner->m_mod->modSpace();

   // do we have a module?
   if( m_owner->m_mod->addLoad( path, isFsPath ) )
   {
      sp.addError( e_load_already, sp.currentSource(), sp.currentLine()-1, 0, 0, path );
      return;
   }

   // just adding it top the module won't be of any help.
   theSpace->loadModule( path, isFsPath, m_owner->m_vmctx );
}


bool IntCompiler::Context::onImportFrom( ImportDef* def )
{
   SourceParser& sp = m_owner->m_sp;
   if( ! m_owner->m_bAllowDirective )
   {
      sp.addError( e_directive_not_allowed, sp.currentSource(), sp.currentLine()-1, 0, 0 );
      return false;
   }

   // have we already a module group for the module?
   // get the module space
   VMContext* cctx = m_owner->m_vmctx;
   Module* mod = m_owner->m_mod;
   ModSpace* ms = cctx->vm()->modSpace();
   mod->addImport( def );
   ms->resolveDeps( cctx, mod );

   return true;
}


void IntCompiler::Context::onExport(const String& symName )
{
   SourceParser& sp = m_owner->m_sp;
   if( ! m_owner->m_bAllowDirective )
   {
      sp.addError( e_directive_not_allowed, sp.currentSource(), sp.currentLine()-1, 0, 0 );
      return;
   }

   Module* mod = m_owner->m_mod;

   // do we have a module?
   Variable* sym = 0;
   bool already;
   sym = mod->addExport( symName, already );

   // already exported?
   if( already )
   {
      sp.addError( e_export_already, m_owner->m_sp.currentSource(),
           sym->declaredAt(), 0, 0 );
      return;
   }

   Error* e = mod->modSpace()->exportSymbol( mod, symName, *sym );
   if( e != 0 )
   {
      sp.addError( e );
      e->decref();
   }
}


void IntCompiler::Context::onDirective(const String&, const String&)
{
   // TODO
   SourceParser& sp = m_owner->m_sp;
   if( ! m_owner->m_bAllowDirective )
   {
      sp.addError( e_directive_not_allowed, sp.currentSource(), sp.currentLine()-1, 0, 0 );
      return;
   }
}


void IntCompiler::Context::onGlobal( const String& )
{
   // TODO
   SourceParser& sp = m_owner->m_sp;
   if( ! m_owner->m_bAllowDirective )
   {
      sp.addError( e_directive_not_allowed, sp.currentSource(), sp.currentLine()-1, 0, 0 );
      return;
   }
}


Variable* IntCompiler::Context::onGlobalDefined( const String& name, bool& bAlreadyDef )
{
   Variable* var = m_owner->m_mod->getGlobal( name );
   if( var == 0 )
   {
      bAlreadyDef = false;
      var = m_owner->m_mod->addGlobal( name, Item(), false );
      var->declaredAt( m_owner->m_sp.currentLine() );
   }
   else {
      bAlreadyDef = true;
   }

   return var;
}


Variable* IntCompiler::Context::onGlobalAccessed( const String& name )
{
   Variable* var = m_owner->m_mod->getGlobal( name );
   if( var == 0 )
   {
      Variable* var = m_owner->m_mod->addImplicitImport( name );
      var->declaredAt( m_owner->m_sp.currentLine() );
   }

   return var;
}


Item* IntCompiler::Context::getVariableValue( Variable* var )
{
   return m_owner->m_mod->getGlobalValue( var->id() );
}


void IntCompiler::Context::onRequirement( Requirement* req )
{
   // the incremental compiler cannot store requirements.
   delete req;
   m_owner->m_sp.addError( e_undef_sym, m_owner->m_sp.currentSource(), 
                req->sourceRef().line(), req->sourceRef().chr(), 0, req->name() );
}


//=======================================================================
// Main class
//

IntCompiler::IntCompiler( bool allowDirective ):
   m_currentTree(0),
   m_bAllowDirective( allowDirective )
{
   // Prepare the compiler and the context.
   m_ctx = new Context( this );
   m_sp.setContext(m_ctx);
   m_sp.interactive(true);
   m_compf = 0;
   m_mod = 0;
   m_vmctx = 0;

   m_lexer = new SourceLexer( "(interactive)", &m_sp );
   m_sp.pushLexer( m_lexer );
}


IntCompiler::~IntCompiler()
{
   delete m_ctx;
   delete m_currentTree;
}


void IntCompiler::setCompilationContext( Function * function, Module* mod, VMContext* vmctx )
{
   m_compf = function;
   m_mod = mod;
   m_vmctx = vmctx;
}


IntCompiler::t_compile_status IntCompiler::compileNext( TextReader* input, SynTree*& code, Mantra*& definition )
{
   m_lexer->setReader(input, false);
   t_compile_status status;

   // create a new syntree if it was emptied
   if( m_currentTree == 0 )
   {
      m_currentTree = new SynTree;
      m_ctx->openMain( m_currentTree );
      m_sp.pushState( "Main", false );
   }

   // if there is a compilation error, throw it
   m_sp.step();

   if( isComplete() )
   {
      if( ! m_currentTree->empty() )
      {
         if ( m_currentTree->at(0)->handler()->userFlags() == FALCON_SYNCLASS_ID_AUTOEXPR )
         {
            // this requires evaluation; but is this a direct call?
            StmtAutoexpr* stmt = static_cast<StmtAutoexpr*>(m_currentTree->at(0));
            Expression* expr = stmt->selector();
            if( expr != 0 && expr->handler()->userFlags() == FALCON_SYNCLASS_ID_CALLFUNC )
            {
               status = e_expr_call;
            }
            else
            {
               status = e_expression;
            }
         }
         else {
            status = e_statement;
         }
         // else ret can stay ok

         m_sp.reset();
         // where to put the tree now?
         // it might be reflected?
         code = m_currentTree;
         m_currentTree = 0;
      }
      else {
         status= e_definition;
         definition = m_currentMantra;
      }
   }
   else {
      status = e_incomplete;
   }

   return status;
}


void IntCompiler::throwCompileErrors() const
{
   class MyEnumerator: public Parsing::Parser::ErrorEnumerator
   {
   public:
      MyEnumerator() {
         myError = new CodeError( e_compile );
      }

      virtual bool operator()( const Parsing::Parser::ErrorDef& def, bool bLast ){

         String sExtra = def.sExtra;
         if( def.nOpenContext != 0 && def.nLine != def.nOpenContext )
         {
            if( sExtra.size() != 0 )
               sExtra += " -- ";
            sExtra += "from line ";
            sExtra.N(def.nOpenContext);
         }

         SyntaxError* err = new SyntaxError( ErrorParam( def.nCode, def.nLine )
               .origin( ErrorParam::e_orig_compiler )
               .chr( def.nChar )
               .module(def.sUri)
               .extra(sExtra));

         myError->appendSubError(err);

         if( bLast )
         {
            throw myError;
         }

         return true;
      }

   private:
      Error* myError;

   } rator;

   m_sp.enumerateErrors( rator );
}

void IntCompiler::resetTree()
{
   m_sp.reset();
   delete m_currentTree;
   delete m_currentMantra;
   m_currentTree = 0;
   m_currentMantra = 0;
}


bool IntCompiler::isComplete() const
{
   return m_sp.isComplete() && m_ctx->isCompleteStatement();
}

}

/* end of intcompiler.h */