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


#include <unistd.h>

#include <QBuffer>

#include "Logger.h"
#include "QY2CursorHelper.h"
#include "QY2IconLoader.h"
#include "YQi18n.h"
#include "utf8.h"

#include "YQPkgSelDescriptionView.h"


using std::list;
using std::string;


YQPkgSelDescriptionView::YQPkgSelDescriptionView( QWidget * parent )
    : YQPkgDescriptionView( parent )
{
}


YQPkgSelDescriptionView::~YQPkgSelDescriptionView()
{
    // NOP
}


void
YQPkgSelDescriptionView::showDetails( ZyppSel selectable )
{
    _selectable = selectable;

    if ( ! selectable )
    {
	clear();
	return;
    }

    QString html_text = htmlHeading( selectable );

    QString description = fromUTF8( selectable->theObj()->description() );

    if ( ! description.contains( "<!-- DT:Rich -->" ) )
	description = simpleHtmlParagraphs( description );

    html_text += description;

    setHtml( html_text );
    //FIXME ensureVisible( 0, 0 );	// Otherwise hyperlinks will be centered
}



QString
YQPkgSelDescriptionView::htmlHeading( ZyppSel selectable )
{
    ZyppObj       	zyppObj       	= selectable->theObj();
    ZyppPattern		pattern		= tryCastToZyppPattern  ( zyppObj );

    if ( ! pattern )
	return YQPkgGenericDetailsView::htmlHeading( selectable );

    QString summary = fromUTF8( zyppObj->summary() );
    bool useBigFont = ( summary.length() <= 40 );

    if ( summary.isEmpty() )			// No summary?
	summary = fromUTF8( zyppObj->name() );	// Use name instead (internal only normally)

    QString iconName = pattern ? pattern->icon().asString().c_str() : "";

    if ( iconName.isEmpty() )
    {
	iconName = zyppObj->name().c_str();
	iconName.replace( ' ', '_' );
    }

    if ( ! iconName.isEmpty() )
    {
	if ( iconName.startsWith( "./" ) )
            iconName.remove( 0, sizeof( "./" ) );

	if ( pattern && iconName.isEmpty() )
	    logWarning() << "No icon for pattern " << zyppObj->name() << Qt::endl;
    }


    QString html = "<table width='100%'><tr><td>"
	+ ( useBigFont ? QString( "<h2>" ) : QString( "<b>" ) )
	+ summary
	+ ( useBigFont ? QString( "</h2>" ) : QString( "</b>" ) )
	+ "</td></tr>"
	+ "</table>";

    if ( ! iconName.isEmpty() )
    {
	QIcon icon = QY2IconLoader::loadIcon( iconName );
        QPixmap pixmap = icon.pixmap( useBigFont ? 32 : 16 );
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        pixmap.save(&buffer, "PNG");
        // The white-space: nowrap is to work around https://bugreports.qt.io/browse/QTBUG-86671
	html = QString( "<table width='100%'><tr>" )
	    + "<td style='white-space: nowrap;' valign='middle'><img src=\"data:image/png;base64," + byteArray.toBase64() + "\"/></td>"
	    + "<td valign='middle' width='100%'>" + html + "</td>"
	    + "</tr></table>";
    }

    return html;
}
