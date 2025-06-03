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

 */

#include <zypp/Package.h>
#include <zypp/PoolItem.h>
#include <zypp/ui/Selectable.h>

#include "Logger.h"
#include "YQi18n.h"
#include "utf8.h"
#include "YQPkgChangeLogView.h"


static const int MAX_ENTRIES = 512;


YQPkgChangeLogView::YQPkgChangeLogView( QWidget * parent )
    : YQPkgGenericDetailsView( parent )
{
}


YQPkgChangeLogView::~YQPkgChangeLogView()
{
    // NOP
}


void
YQPkgChangeLogView::showDetails( ZyppSel selectable )
{
    _selectable = selectable;

    if ( ! selectable )
    {
	clear();
	return;
    }

    // logVerbose() << "Generating changelog..." << endl;

    QString html = htmlStart();
    html += htmlHeading( selectable, false );

    ZyppPkg installed = tryCastToZyppPkg( selectable->installedObj() );

    if ( installed )
    {
        html += changeLogTable( installed->changelog() );

        int omittedCount = installed->changelog().size() - MAX_ENTRIES;

        if ( omittedCount > 0)
        {
            html.append( "<p class='note'>"
                         + omittedEntries( omittedCount, installed->name() )
                         + "</p>" );
        }
    }
    else
    {
	html += "<p><i>" + _( "Information only available for installed packages." ) + "</i></p>";
    }

    html += htmlEnd();
    setHtml( html );
}



QString YQPkgChangeLogView::changeLogTable( const zypp::Changelog & changeLog ) const
{
    // logVerbose () << "Changelog size: " << changeLog.size() << " entries" << endl;

    QString html;
    int count = 0;

    for ( zypp::Changelog::const_iterator it = changeLog.begin();
	  it != changeLog.end();
	  ++it )
    {
	QString changes = htmlEscape( fromUTF8( (*it).text() ) );
	changes.replace( "\n", "<br>"  );
	changes.replace( " ", "&nbsp;" );

	html += row( cell( (*it).date()   ) +   // cell() calls htmlEscape()!
                     cell( (*it).author() ) +
                     "<td valign='top'>" + changes + "</td>"
                     );

        if ( ++count == MAX_ENTRIES )
            break;
    }

    return html.isEmpty() ? "" : table( html );
}


QString YQPkgChangeLogView::omittedEntries( int                 omittedCount,
                                            const std::string & pkgName )
{
    QString rpmCmd = QString( "rpm -q --changelog %1" ).arg( fromUTF8( pkgName ) );

    QString msg = _( "(%1 more; run \"%2\" to see the complete change log)" )
        .arg( omittedCount )
        .arg( rpmCmd );

    msg = QString( "<i>" ) + msg + "</i>";

    return msg;
}

