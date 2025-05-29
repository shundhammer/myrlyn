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


#include <string.h>             // strlen()
#include <libintl.h>            // dgettext(), bindtextdomain()
#include "utf8.h"
#include "MyrlynApp.h"          // MyrlynApp::isOptionSet()
#include "MyrlynTranslator.h"


MyrlynTranslator::MyrlynTranslator( QObject * parent )
    : QTranslator( parent )
{
    // bindtextdomain( "myrlyn", "/usr/share/locale" );
    // Not needed: /usr/share/locale is the default location.

    // Initialize _fakeTemplate with "xixoxixoxixo..."

    for ( int i=1; i < 20; i++ )
        _fakeTemplate += "xixo";
}


MyrlynTranslator::~MyrlynTranslator()
{
    // NOP
}


QString
MyrlynTranslator::translate( const char * context_str,
                             const char * sourceText,
                             const char * disambiguation,
                             int          nPlural         ) const
{
    if ( MyrlynApp::isOptionSet( OptFakeTranslations ) )
         return fakeTranslation( sourceText );

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

    return dgettext( "myrlyn", sourceText );
}


QString
MyrlynTranslator::fakeTranslation( const char * sourceText ) const
{
    return _fakeTemplate.left( strlen( sourceText ) );
}

