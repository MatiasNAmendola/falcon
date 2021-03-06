#ifndef GTK_WINDOWGROUP_HPP
#define GTK_WINDOWGROUP_HPP

#include "modgtk.hpp"


namespace Falcon {
namespace Gtk {

/**
 *  \class Falcon::Gtk::WindowGroup
 */
class WindowGroup
    :
    public Gtk::CoreGObject
{
public:

    WindowGroup( const Falcon::CoreClass*, const GtkWindowGroup* = 0 );

    static Falcon::CoreObject* factory( const Falcon::CoreClass*, void*, bool );

    static void modInit( Falcon::Module* );

    static FALCON_FUNC init( VMARG );

    static FALCON_FUNC add_window( VMARG );

    static FALCON_FUNC remove_window( VMARG );
#if GTK_CHECK_VERSION( 2, 14, 0 )
    static FALCON_FUNC list_windows( VMARG );
#endif

};


} // Gtk
} // Falcon

#endif // !GTK_WINDOWGROUP_HPP

// vi: set ai et sw=4:
// kate: replace-tabs on; shift-width 4;
