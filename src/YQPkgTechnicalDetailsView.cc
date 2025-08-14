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


#include "Logger.h"
#include "YQi18n.h"
#include "utf8.h"
#include "YRpmGroupsTree.h"  // RPM_GROUPS_TEXTDOMAIN
#include "YQPkgTechnicalDetailsView.h"


using std::list;
using std::string;


YQPkgTechnicalDetailsView::YQPkgTechnicalDetailsView( QWidget * parent )
    : YQPkgGenericDetailsView( parent )
{
}


YQPkgTechnicalDetailsView::~YQPkgTechnicalDetailsView()
{
    // NOP
}


void
YQPkgTechnicalDetailsView::showDetails( ZyppSel selectable )
{
    _selectable = selectable;

    if ( ! selectable )
    {
        clear();
        return;
    }

    QString html_text = htmlStart();

    html_text += htmlHeading( selectable );

    ZyppPkg candidate = tryCastToZyppPkg( selectable->candidateObj() );
    ZyppPkg installed = tryCastToZyppPkg( selectable->installedObj() );

    if ( candidate && installed && candidate != installed )
    {
        html_text += complexTable( selectable, installed, candidate );
    }
    else
    {
        if ( candidate )
            html_text += simpleTable( selectable, candidate );

        if ( installed )
            html_text += simpleTable( selectable, installed );
    }

    html_text += htmlEnd();

    setHtml( html_text );
}


QString
YQPkgTechnicalDetailsView::authorsListCell( ZyppPkg pkg ) const
{
    const std::list<std::string> authors = pkg->authors();

    QString html = "<td align='top'>";
    QString line;

    for ( const std::string & author: authors )
    {
        line = fromUTF8( author );
        line = htmlEscape( line );
        html += line + "<br>";
    }

    html += "</td>";

    return html;
}


QString
YQPkgTechnicalDetailsView::formatRpmGroup( ZyppPkg pkg ) const
{
    QString rpmGroup = fromUTF8( pkg->group() );

    if ( rpmGroup.toLower() == "unspecified" )
        return "";

    QStringList groups = rpmGroup.split( '/', Qt::KeepEmptyParts );


    // Translate group path components

    QStringList translated;

    for ( QString group: groups )
    {
        translated.append( QString::fromUtf8( dgettext( RPM_GROUPS_TEXTDOMAIN, group.toUtf8() ) ) );
    }

    return translated.join( "/" );
}


QString
YQPkgTechnicalDetailsView::simpleTable( ZyppSel selectable,
                                        ZyppPkg pkg )
{
    QString html;

    html += row( hcell( _( "Version:"           ) ) + cell( pkg->edition().asString()      ) );
    html += row( hcell( _( "Build Time:"        ) ) + cell( pkg->buildtime()               ) );

    html +=
        *pkg == selectable->installedObj() ?
        row( hcell( _( "Install Time:" ) ) + cell( pkg->installtime() ) ) : "";

    QString rpmGroup = formatRpmGroup( pkg );

    // if ( ! rpmGroup.isEmpty() )
    html += row( hcell( _( "RPM Group:"         ) ) + cell( rpmGroup                       ) );

    html += row( hcell( _( "License:"           ) ) + cell( pkg->license()                 ) );
    html += row( hcell( _( "Installed Size:"    ) ) + cell( pkg->installSize().asString()  ) );
    html += row( hcell( _( "Download Size:"     ) ) + cell( pkg->downloadSize().asString() ) );
    html += row( hcell( _( "Distribution:"      ) ) + cell( pkg->distribution()            ) );
    html += row( hcell( _( "Vendor:"            ) ) + cell( pkg->vendor()                  ) );
    html += row( hcell( _( "Packager:"          ) ) + cell( pkg->packager()                ) );
    html += row( hcell( _( "Architecture:"      ) ) + cell( pkg->arch().asString()         ) );
    html += row( hcell( _( "URL:"               ) ) + cell( pkg->url()                     ) );
    html += row( hcell( _( "Source Package:"    ) ) + cell( pkg->sourcePkgName() + "-" + pkg->sourcePkgEdition().asString() ) );

    if ( ! pkg->authors().empty() )
        html += row( hcell( _( "Authors:"       ) ) + authorsListCell( pkg                 ) );

    html = "<br>" + table( html );

    return html;
}


QString
YQPkgTechnicalDetailsView::complexTable( ZyppSel selectable,
                                         ZyppPkg installed,
                                         ZyppPkg candidate )
{
    ZyppPkg p1 = candidate;
    ZyppPkg p2 = installed;

    QString p1_header = _( "<b>Alternate Version</b>" );
    QString p2_header = _( "<b>Installed Version</b>" );

    QString html;

    html += row( hcell( QString( "" ) ) + hcell( "<b>" + p1_header + "</b>" ) + hcell( "<b>" + p2_header + "</b>"   ) );

    html += row( hcell( _( "Version:"           ) ) + cell( p1->edition().asString()      ) + cell( p2->edition().asString()      ) );
    html += row( hcell( _( "Build Time:"        ) ) + cell( p1->buildtime()               ) + cell( p2->buildtime()               ) );
    html += row( hcell( _( "Install Time:"      ) ) + cell( p1->installtime()             ) + cell( p2->installtime()             ) );

    QString rpmGroup1 = formatRpmGroup( p1 );
    QString rpmGroup2 = formatRpmGroup( p2 );

    // if ( ! rpmGroup1.isEmpty() || ! rpmGroup2.isEmpty() )
    html += row( hcell( _( "RPM Group:"         ) ) + cell( rpmGroup1                     ) + cell( rpmGroup2                     ) );

    html += row( hcell( _( "License:"           ) ) + cell( p1->license()                 ) + cell( p2->license()                 ) );
    html += row( hcell( _( "Installed Size:"    ) ) + cell( p1->installSize().asString()  ) + cell( p2->installSize().asString()  ) );
    html += row( hcell( _( "Download Size:"     ) ) + cell( p1->downloadSize().asString() ) );
    html += row( hcell( _( "Distribution:"      ) ) + cell( p1->distribution()            ) + cell( p2->distribution()            ) );
    html += row( hcell( _( "Vendor:"            ) ) + cell( p1->vendor()                  ) + cell( p2->vendor()                  ) );
    html += row( hcell( _( "Packager:"          ) ) + cell( p1->packager()                ) + cell( p2->packager()                ) );
    html += row( hcell( _( "Architecture:"      ) ) + cell( p1->arch().asString()         ) + cell( p2->arch().asString()         ) );
    html += row( hcell( _( "URL:"               ) ) + cell( p1->url()                     ) + cell( p2->url()                     ) );
    html += row( hcell( _( "Source Package:"    ) ) + cell( p1->sourcePkgName() + "-" + p1->sourcePkgEdition().asString() )
                                                    + cell( p2->sourcePkgName() + "-" + p2->sourcePkgEdition().asString()         ) );

    if ( ! ( p1->authors().empty() && p2->authors().empty() ) )
         html += row( hcell( _( "Authors:"      ) ) + authorsListCell( p1                 ) + authorsListCell( p2                 ) );

    html = "<br>" + table( html );

    return html;
}


