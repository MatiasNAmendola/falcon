#ifndef MODGTK_HPP
#define MODGTK_HPP

#include <falcon/autocstring.h>
#include <falcon/coreslot.h>
#include <falcon/error.h>
#include <falcon/falconobject.h>
#include <falcon/garbagelock.h>
#include <falcon/item.h>
#include <falcon/module.h>
#include <falcon/string.h>
#include <falcon/vm.h>

#include <gtk/gtk.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "modgtk_st.hpp"
#include "modgtk_version.hpp"

/*
 *  some helper defines..
 */

#define VMARG           ::Falcon::VMachine* vm

#define MYSELF \
        Gtk::CoreGObject* self = Falcon::dyncast<Gtk::CoreGObject*>( vm->self().asObjectSafe() )

#define GET_OBJ( self ) \
        GObject* _obj = self->getObject()

#define COREGOBJECT( pItem ) \
        (Falcon::dyncast<Gtk::CoreGObject*>( (pItem)->asObjectSafe() ))

#define GET_SIGNALS( gobj ) \
        ::Falcon::CoreSlot* _signals = (::Falcon::CoreSlot*) \
        g_object_get_data( Gtk::CoreGObject::add_slots( (GObject*) gobj ), "__signals" )

#define IS_DERIVED( it, cls ) \
        ( (it)->isOfClass( #cls ) || (it)->isOfClass( "gtk." #cls ) )

#define CoreObject_IS_DERIVED( obj, cls ) \
        ( (obj)->derivedFrom( #cls ) || (obj)->derivedFrom( "gtk." #cls ) )

#define throw_inv_params( x ) \
        throw new ::Falcon::ParamError( \
        ::Falcon::ErrorParam( ::Falcon::e_inv_params, __LINE__ ).extra( x ) )

#define throw_require_no_args() \
        throw_inv_params( FAL_STR( gtk_e_require_no_args_ ) )

#define throw_gtk_error( n, x ) \
        throw new ::Falcon::Gtk::GtkError( \
        ::Falcon::ErrorParam( ::Falcon::Gtk::n, __LINE__ ).desc( x ) )

#ifdef STRICT_PARAMETER_CHECK
#define NO_ARGS     if ( vm->paramCount() ) throw_require_no_args();
#else
#define NO_ARGS
#endif


namespace Falcon {

/**
 *  \namespace Falcon::Gtk
 */
namespace Gtk {


class VoidObject;
class CoreGObject;
class Signal;


/**
 *  \brief Common init method for all abstract classes.
 */
FALCON_FUNC abstract_init( VMARG );


/**
 *  \class Falcon::Gtk::VoidObject
 *  \brief Embeds a void pointer.
 */
class VoidObject
    :
    public Falcon::CoreObject
{
public:

    virtual ~VoidObject() {}

    virtual VoidObject* clone() const { return new VoidObject( *this ); }

    void gcMark( Falcon::uint32 ) {}

    virtual bool getProperty( const Falcon::String&, Falcon::Item& ) const;

    virtual bool setProperty( const Falcon::String&, const Falcon::Item& );

    void* getObject() const { return (void*) m_obj; }

    virtual void setObject( const void* obj )
    {
        assert( m_obj == 0 );
        assert( obj != 0 );
        m_obj = (void*) obj;
    }

    /**
     *  \brief Create a valid object of given type and throw it in the machine.
     *  \param vm a virtual machine
     *  \param type the GType of obj
     *  \param obj a pointer to an instance of type
     *  \param doFree wether to free the object when it is possible
     *  \note if the object gets freed it will be set to NULL
     */
    static void retval( VMachine* vm,
                        const GType type,
                        void*& obj,
                        const bool doFree = true );

protected:

    VoidObject( const Falcon::CoreClass* cls, const void* obj = 0 )
        :
        CoreObject( cls ),
        m_obj( (void*) obj )
    {}

    VoidObject( const VoidObject& other )
        :
        CoreObject( other ),
        m_obj( other.m_obj )
    {}

    void*   m_obj;

};


/**
 *  \class Falcon::Gtk::CoreGObject
 *  \brief The base class exposing a GObject.
 */
class CoreGObject
    :
    public Gtk::VoidObject
{
public:

    ~CoreGObject();

    virtual CoreGObject* clone() const { return new CoreGObject( *this ); }

    /**
     *  \brief Get a property.
     *  Properties are stored in the table of associations of the GObject,
     *  as garbage-locked items.
     */
    virtual bool getProperty( const Falcon::String&, Falcon::Item& ) const;

    /**
     *  \brief Set a property.
     *  Stores a garbage-lock in the table of associations of the GObject.
     */
    virtual bool setProperty( const Falcon::String&, const Falcon::Item& );

    /**
     *  \brief Get the GObject.
     */
    GObject* getObject() const { return (GObject*) m_obj; }

    /**
     *  \brief Set the GObject.
     */
    virtual void setObject( const void* );

    /**
     *  \brief Create a valid object of given type and throw it in the machine.
     *  \param vm a virtual machine
     *  \param type the GType of obj, derived from G_TYPE_OBJECT
     *  \param obj a pointer to an instance of that type
     *  \param doFree wether to unref the object
     */
    static void retval( VMachine* vm,
                        const GType type,
                        void*& obj,
                        const bool doFree = true );

    /**
     *  \brief Add an anonymous VMSlot in the GObject.
     *  VMSlots are used as a convenience to store the callback functions.
     */
    static GObject* add_slots( GObject* );

    /**
     *  \brief Get a signal.
     *  This is used by derived classes to return a specific signal into the vm.
     *  \param signame signal name (e.g: "some_event")
     *  \param cb pointer to callback function
     *  \param vm virtual machine
     */
    static void get_signal( const char* signame, const void* cb, Falcon::VMachine* vm );

    /**
     *  \brief Trigger internal slot.
     *  This is used by derived classes to activate a specific signal, without
     *  any further checking of values returned by callbacks.
     *  \param obj signal emitter
     *  \param signame signal name (e.g: "some_event")
     *  \param cbname callback name (e.g: "on_some_event")
     *  \param vm virtual machine
     */
    static void trigger_slot( GObject* obj, const char* signame,
                            const char* cbname, Falcon::VMachine* vm );

protected:

    CoreGObject( const Falcon::CoreClass*, const GObject* = 0 );

    CoreGObject( const CoreGObject& );

    /**
     *  \brief Return the hash table of garbage locks of the GObject.
     *  Locks protecting the callback functions are stored in a hash table.
     *  This is used just before connecting signals.
     *  \return the hash table containing the locks
     */
    static GHashTable* get_locks( GObject* );

    /**
     *  \brief Lock an item.
     *  This adds an item to the table of locks.
     *  \param obj The GObject
     *  \param it The callable
     *  \return the new GarbageLock
     */
    static Falcon::GarbageLock* lockItem( GObject* obj, const Falcon::Item& it );

    /**
     *  \brief GDestroyNotify function for GarbageLock objects.
     */
    static void release_lock( gpointer );

private:

    /**
     *  \brief GDestroyNotify function to delete all slots.
     */
    static void release_slots( gpointer );

    /**
     *  \brief GDestroyNotify function for the GarbageLock table.
     */
    static void release_locks( gpointer );

    void incref() const;

    void decref() const;

};


/**
 *  \brief Class managing a signal.
 */
class Signal
    :
    public Gtk::CoreGObject
{
    friend class Gtk::CoreGObject;

public:

    ~Signal() {}

    Signal* clone() const { return new Signal( *this ); }

    bool getProperty( const Falcon::String&, Falcon::Item& ) const;

    bool setProperty( const Falcon::String&, const Falcon::Item& );

    static Falcon::CoreObject* factory( const Falcon::CoreClass*, void*, bool );

    static void modInit( Falcon::Module* );

    /**
     *  \brief Connect a signal to a callback.
     */
    static FALCON_FUNC connect( VMARG );

private:

    /**
     *  \brief Signal ctor
     *  \param gobj a GObject
     *  \param name signal name (e.g: "some_event")
     *  \param cb pointer to callback function
     */
    Signal( const Falcon::CoreClass* cls,
            const GObject* gobj, const char* name, const void* cb );

    Signal( const Signal& );

    char*     m_name;

    void*     m_cb;

};


/**
 *  \class Falcon::Gtk::GtkError
 */
class GtkError
    :
    public Falcon::Error
{
public:

    GtkError()
        :
        Error( "GtkError" )
    {}

    GtkError( const ErrorParam& params  )
        :
        Error( "GtkError", params )
    {}

};


/**
 *  \brief exception type initialization
 */
FALCON_FUNC GtkError_init ( VMARG );


/**
 *  \enum Falcon::Gtk::GtkErrorIds
 */
enum GtkErrorIds
{
    e_abstract_class,       // unable to create instance of abstract type
    e_init_failure,         // failure due to gtk_init* functions
    e_inv_property          // invalid property
};


/**
 *  \brief struct holding class methods information
 */
typedef struct
{
    const char* name;
    void (*cb)( Falcon::VMachine* );
} MethodTab;


/**
 *  \brief struct holding enums information
 */
typedef struct
{
    const char* name;
    Falcon::int64 value;
} ConstIntTab;


/**
 *  \brief class to help with arguments checking
 */
template<int numStrings>
class ArgCheck
{

    Falcon::AutoCString m_strings[ numStrings ];

    Falcon::VMachine*   m_vm;

    const char*     m_spec;

    int     m_p;

public:

    ArgCheck( Falcon::VMachine* vm, const char* spec )
        :
        m_vm( vm ),
        m_spec( spec ),
        m_p( 0 )
    {}

    char* getCString( int index, bool mandatory = true )
    {
        Item* it = m_vm->param( index );
        if ( mandatory )
        {
#ifndef NO_PARAMETER_CHECK
            if ( !it || it->isNil() || !it->isString() )
                throw_inv_params( m_spec );
#endif
        }
        else
        {
            if ( !it || it->isNil() )
                return 0;
#ifndef NO_PARAMETER_CHECK
            if ( !it->isString() )
                throw_inv_params( m_spec );
#endif
        }
        m_strings[ m_p ].set( it->asString() );
        return (char*) m_strings[ m_p++ ].c_str();
    }

    Falcon::int64 getInteger( int index, bool mandatory = true, bool* wasNil = 0 ) const
    {
        Item* it = m_vm->param( index );
        if ( mandatory )
        {
#ifndef NO_PARAMETER_CHECK
            if ( !it || it->isNil() || !it->isInteger() )
                throw_inv_params( m_spec );
#endif
        }
        else
        {
            if ( !it || it->isNil() )
            {
                if ( wasNil )
                    *wasNil = true;
                return 0;
            }
#ifndef NO_PARAMETER_CHECK
            if ( !it->isInteger() )
                throw_inv_params( m_spec );
#endif
            if ( wasNil )
                *wasNil = false;
        }
        return it->asInteger();
    }

    Falcon::numeric getNumeric( int index, bool mandatory = true, bool* wasNil = 0 ) const
    {
        Item* it = m_vm->param( index );
        if ( mandatory )
        {
#ifndef NO_PARAMETER_CHECK
            if ( !it || it->isNil() || !it->isOrdinal() )
                throw_inv_params( m_spec );
#endif
        }
        else
        {
            if ( !it || it->isNil() )
            {
                if ( wasNil )
                    *wasNil = true;
                return 0;
            }
#ifndef NO_PARAMETER_CHECK
            if ( !it->isOrdinal() )
                throw_inv_params( m_spec );
#endif
            if ( wasNil )
                *wasNil = false;
        }
        return it->forceNumeric();
    }

    bool getBoolean( int index, bool mandatory = true, bool* wasNil = 0 ) const
    {
        Item* it = m_vm->param( index );
        if ( mandatory )
        {
#ifndef NO_PARAMETER_CHECK
            if ( !it || it->isNil() || !it->isBoolean() )
                throw_inv_params( m_spec );
#endif
        }
        else
        {
            if ( !it || it->isNil() )
            {
                if ( wasNil )
                    *wasNil = true;
                return false;
            }
#ifndef NO_PARAMETER_CHECK
            if ( !it->isBoolean() )
                throw_inv_params( m_spec );
#endif
            if ( wasNil )
                *wasNil = false;
        }
        return it->asBoolean();
    }

    Falcon::CoreObject* getObject( int index, bool mandatory = true ) const
    {
        Item* it = m_vm->param( index );
        if ( mandatory )
        {
#ifndef NO_PARAMETER_CHECK
            if ( !it || it->isNil() || !it->isObject() )
                throw_inv_params( m_spec );
#endif
        }
        else
        {
            if ( !it || it->isNil() )
                return 0;
#ifndef NO_PARAMETER_CHECK
            if ( !it->isObject() )
                throw_inv_params( m_spec );
#endif
        }
        return it->asObjectSafe();
    }

    Gtk::CoreGObject* getCoreGObject( int index, bool mandatory = true ) const
    {
        Item* it = m_vm->param( index );
        if ( mandatory )
        {
#ifndef NO_PARAMETER_CHECK
            if ( !it || it->isNil() || !it->isObject() )
                throw_inv_params( m_spec );
#endif
        }
        else
        {
            if ( !it || it->isNil() )
                return 0;
#ifndef NO_PARAMETER_CHECK
            if ( !it->isObject() )
                throw_inv_params( m_spec );
#endif
        }
        return Falcon::dyncast<Gtk::CoreGObject*>( it->asObjectSafe() );
    }

    Falcon::CoreArray* getArray( int index, bool mandatory = true ) const
    {
        Item* it = m_vm->param( index );
        if ( mandatory )
        {
#ifndef NO_PARAMETER_CHECK
            if ( !it || it->isNil() || !it->isArray() )
                throw_inv_params( m_spec );
#endif
        }
        else
        {
            if ( !it || it->isNil() )
                return 0;
#ifndef NO_PARAMETER_CHECK
            if ( !it->isArray() )
                throw_inv_params( m_spec );
#endif
        }
        return it->asArray();
    }

    ~ArgCheck() {}

};


/*
 *  typedefs for Gtk::ArgCheck<x>
 */

#ifndef __GNUC__
typedef Falcon::Gtk::ArgCheck<1>    ArgCheck0;
#else
typedef Falcon::Gtk::ArgCheck<0>    ArgCheck0;
#endif
typedef Falcon::Gtk::ArgCheck<1>    ArgCheck1;
typedef Falcon::Gtk::ArgCheck<2>    ArgCheck2;
typedef Falcon::Gtk::ArgCheck<3>    ArgCheck3;
typedef Falcon::Gtk::ArgCheck<4>    ArgCheck4;
typedef Falcon::Gtk::ArgCheck<5>    ArgCheck5;
typedef Falcon::Gtk::ArgCheck<6>    ArgCheck6;
typedef Falcon::Gtk::ArgCheck<7>    ArgCheck7;
typedef Falcon::Gtk::ArgCheck<8>    ArgCheck8;
typedef Falcon::Gtk::ArgCheck<9>    ArgCheck9;


/**
 *  \brief Get an array of gchar* from a CoreArray.
 *  \param arr The core array.
 *  \param strings (out) The resulting C-strings (as a NULL terminated array).
 *  \param temp (out) The intermediate auto-C-strings.
 *  \return The length of the array.
 *
 *  \note If arr was non-empty, both strings and temp must be freed with delete[] after use.
 */
uint32
getGCharArray( const Falcon::CoreArray* arr,
        gchar**& strings,
        Falcon::AutoCString*& temp );


/**
 *  \brief Format a file path given from string.
 *  \param filepath A string representing a filepath.
 *  \return The modified string passed as argument.
 */
Falcon::String*
formatPath( Falcon::String* filepath );


} // Gtk
} // Falcon

#endif // !MODGTK_HPP

// vi: set ai et sw=4:
// kate: replace-tabs on; shift-width 4;
