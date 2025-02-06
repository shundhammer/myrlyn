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


#include <QTabWidget>

#include <zypp/Edition.h>
#include <zypp/PoolItem.h>
#include <zypp/ResObject.h>
#include <zypp/ui/Selectable.h>

#include "Logger.h"
#include "utf8.h"
#include "YQPkgGenericDetailsView.h"

#ifndef VERBOSE_DETAILS_VIEWS
#  define VERBOSE_DETAILS_VIEWS  0
#endif


YQPkgGenericDetailsView::YQPkgGenericDetailsView( QWidget * parent )
    : QTextBrowser( parent )
{
    _selectable = 0;
    setFrameStyle( QFrame::NoFrame );
    _parentTab  = dynamic_cast<QTabWidget *>( parent );

    if ( _parentTab )
    {
        connect( _parentTab, SIGNAL( currentChanged( int ) ),
                 this,       SLOT  ( reloadTab     ( int ) ) );
    }

    // DO NOT add anything like 'font-size: small' here; that makes the text
    // unreadable for everybody over 40 years of age.

    QString css;
    css = "table.stats"
        "{text-align: center;"
        "font-weight: normal;"
        "width: 100%;"
        ""
        "border: 1px;"
        "border-collapse: collapse;"
        "border-spacing: 4px;}"
        ""
        "table.stats td"
        "{"
        "padding: 4px;"
        "text-align: left;"
        "border: 1px; solid;"
        "}"
        ""
        "table.stats td.hed"
        "{"
        "padding: 4px;"
        "text-align: left;"
        "border-bottom: 2px; solid;"
        "font-weight: bold;} "
        "p.note"
        "{color: grey;"
        "font-style: italic;}";

    document()->addResource( QTextDocument::StyleSheetResource,
                             QUrl( "format.css" ), css );
}


YQPkgGenericDetailsView::~YQPkgGenericDetailsView()
{
    // NOP
}


void
YQPkgGenericDetailsView::reloadTab( int newCurrent )
{
    if ( _parentTab && _parentTab->widget( newCurrent ) == this )
    {
        showDetailsIfVisible( _selectable );
    }
}


void
YQPkgGenericDetailsView::showDetailsIfVisible( ZyppSel selectable )
{
    _selectable = selectable;

    if ( _parentTab )  // Is this view embedded into a tab widget?
    {
        if ( _parentTab->currentWidget() == this )  // Is this page the topmost?
        {
#if VERBOSE_DETAILS_VIEWS

            logVerbose() << metaObject()->className() << ": Showing "
                         << ( selectable ? fromUTF8( selectable->name() ) : "NULL" )
                         << Qt::endl;
#endif
            showDetails( selectable );
        }
    }
    else  // No tab parent - simply show data unconditionally.
    {
        showDetails( selectable );
    }
}


QSize
YQPkgGenericDetailsView::minimumSizeHint() const
{
    return QSize( 0, 0 );
}


QString
YQPkgGenericDetailsView::htmlStart()
{
    return "<html><head>"
        "<link rel='stylesheet' type='text/css' href='format.css'>"
        "</head><body>";
}


QString
YQPkgGenericDetailsView::htmlEnd()
{
    return "</body></html>";
}


QString
YQPkgGenericDetailsView::htmlHeading( ZyppSel selectable, bool showVersion )
{
    if ( ! selectable )
        return "";

    ZyppObj zyppObj = selectable->theObj();

    if ( ! zyppObj )
        return "";

    QString summary = fromUTF8( zyppObj->summary() );

    QString html = "<table";
    html +=  " class=\"stats\"";

    html += "><tr><td><b>"
        + fromUTF8( zyppObj->name() )
        + "</b>";

    if ( showVersion )
        html += QString( "<b>-" ) + zyppObj->edition().asString().c_str() + "</b>";

    if ( ! summary.isEmpty() )
        html += " - " + summary;

    html += "</td></tr></table>";

    return html;
}



QString
YQPkgGenericDetailsView::htmlEscape( const QString & plainText )
{
    QString html = plainText;
    // logDebug() << "Escaping \"" << plainText << "\"" << Qt::endl;

    html.replace( "&", "&amp;" );
    html.replace( "<", "&lt;" );
    html.replace( ">", "&gt;" );

    return html;
}


QString
YQPkgGenericDetailsView::table( const QString & contents )
{
    QString html = "<table";
    html +=  " class=\"stats\"";
    html += ">" + contents + "</table>";

    return html;
}


QString
YQPkgGenericDetailsView::row( const QString & contents )
{
    return "<tr>" + contents + "</tr>";
}


QString
YQPkgGenericDetailsView::cell( const QString & contents )
{
    QString html = htmlEscape( contents );
    return "<td>" + html + "</td>";
}


QString
YQPkgGenericDetailsView::cell( const string & contents )
{
    return cell( fromUTF8( contents ) );
}


QString
YQPkgGenericDetailsView::cell( int contents )
{
    return QString( "<td>%1</td>" ).arg( contents );
}


QString
YQPkgGenericDetailsView::cell( const zypp::Date & date )
{
    return cell( ( (time_t) date == (time_t) 0 ? "" : date.asString() ) );
}


QString
YQPkgGenericDetailsView::hcell( QString contents )
{
    return "<td>" + contents + "</td>";
}


