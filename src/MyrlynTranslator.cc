/*  ---------------------------------------------------------
               __  __            _
              |  \/  |_   _ _ __| |_   _ _ __
              | |\/| | | | | '__| | | | | '_ \
              | |  | | |_| | |  | | |_| | | | |
              |_|  |_|\__, |_|  |_|\__, |_| |_|
                      |___/        |___/
    ---------------------------------------------------------

    Project:  Myrlyn Package Manager GUI
    Copyright (c) 2024-25 SUSE LLC
    License:  GPL V2 - See file LICENSE for details.

    Textdomain "qt-pkg"
 */


#include <libintl.h>            // dgettext(), bindtextdomain()
#include "utf8.h"
#include "MyrlynTranslator.h"


MyrlynTranslator::MyrlynTranslator( QObject * parent )
    : QTranslator( parent )
{
    // DEBUG: Use the old YaST "qt-pkg" translations for now

    bindtextdomain( "qt-pkg", "/usr/share/YaST2/locale" );
    bindtextdomain( "Myrlyn", "/usr/share/locale" );
}


MyrlynTranslator::~MyrlynTranslator()
{
}


QString
MyrlynTranslator::translate( const char * context_str,
                             const char * sourceText,
                             const char * disambiguation,
                             int          nPlural         ) const
{
    QString context( fromUTF8( context_str ) );

    if ( context.startsWith( "libQt" ) )
        // FIXME: Find out the real Qt-internal context (textomain)
    {
        // For libQt-internal texts (file dialogs etc.), use the parent class
        // for Qt's built-in translations

        return QTranslator::translate( context_str,
                                       sourceText,
                                       disambiguation,
                                       nPlural );
    }

#if 0
    return dgettext( "Myrlyn", sourceText );
#else
    return dgettext( "qt-pkg", sourceText );
#endif
}

