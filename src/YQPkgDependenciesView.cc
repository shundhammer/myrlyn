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

#include <zypp/Capability.h>
#include <zypp/Dep.h>
#include <zypp/Edition.h>
#include <zypp/PoolItem.h>
#include <zypp/ResObject.h>
#include <zypp/ResTraits.h>
#include <zypp/ui/Selectable.h>

#include "YQi18n.h"
#include "YQPkgDependenciesView.h"


YQPkgDependenciesView::YQPkgDependenciesView( QWidget * parent )
    : YQPkgGenericDetailsView( parent )
{
}


YQPkgDependenciesView::~YQPkgDependenciesView()
{
    // NOP
}


void
YQPkgDependenciesView::showDetails( ZyppSel selectable )
{
    _selectable = selectable;

    if ( ! selectable )
    {
	clear();
	return;
    }

    QString html_text = htmlStart();
    html_text += htmlHeading( selectable );

    ZyppObj candidate = selectable->candidateObj();
    ZyppObj installed = selectable->installedObj();

    if ( candidate && installed && candidate != installed )
    {
	html_text += complexTable( installed, candidate );
    }
    else
    {
	if ( candidate )
	    html_text += simpleTable( candidate );

	if ( installed )
	    html_text += simpleTable( installed );
    }

    html_text += htmlEnd();

    setHtml( html_text );
}


QString
YQPkgDependenciesView::simpleTable( ZyppObj pkg )
{
    QString html = "<br>" +
	table(
	      row( hcell( _( "Version:" ) ) + cell( pkg->edition().asString()	) ) +

	      row( _("Provides:"),	pkg->dep( zypp::Dep::PROVIDES		) ) +
	      row( _("Prerequires:"),	pkg->dep( zypp::Dep::PREREQUIRES	) ) +
	      row( _("Requires:"),	pkg->dep( zypp::Dep::REQUIRES		) ) +
	      row( _("Conflicts:"),	pkg->dep( zypp::Dep::CONFLICTS		) ) +
	      row( _("Obsoletes:"),	pkg->dep( zypp::Dep::OBSOLETES		) ) +
	      row( _("Recommends:"),	pkg->dep( zypp::Dep::RECOMMENDS		) ) +
	      row( _("Suggests:"),	pkg->dep( zypp::Dep::SUGGESTS		) ) +
	      row( _("Enhances:"),	pkg->dep( zypp::Dep::ENHANCES		) ) +
	      row( _("Supplements:"),	pkg->dep( zypp::Dep::SUPPLEMENTS	) )
	      );

    return html;
}


QString
YQPkgDependenciesView::complexTable( ZyppObj installed, ZyppObj candidate )
{
    ZyppObj p1 = candidate;
    ZyppObj p2 = installed;

    QString p1_header = _( "<b>Alternate Version</b>" );
    QString p2_header = _( "<b>Installed Version</b>" );

    QString html = "<br>" +
	table(
	      row( hcell( QString( "" ) ) + hcell( "<b>" + p1_header + "</b>"	    ) + hcell( "<b>" + p2_header + "</b>" ) ) +

	      row( hcell( _( "Version:" ) ) + cell( p1->edition().asString()	) + cell( p2->edition().asString()	) ) +

	      row( _("Provides:"),	p1->dep( zypp::Dep::PROVIDES	), p2->dep( zypp::Dep::PROVIDES		) ) +
	      row( _("Prerequires:"),	p1->dep( zypp::Dep::PREREQUIRES	), p2->dep( zypp::Dep::PREREQUIRES	) ) +
	      row( _("Requires:"),	p1->dep( zypp::Dep::REQUIRES	), p2->dep( zypp::Dep::REQUIRES		) ) +
	      row( _("Conflicts:"),	p1->dep( zypp::Dep::CONFLICTS	), p2->dep( zypp::Dep::CONFLICTS	) ) +
	      row( _("Obsoletes:"),	p1->dep( zypp::Dep::OBSOLETES	), p2->dep( zypp::Dep::OBSOLETES	) ) +
	      row( _("Recommends:"),	p1->dep( zypp::Dep::RECOMMENDS	), p2->dep( zypp::Dep::RECOMMENDS	) ) +
	      row( _("Suggests:"),	p1->dep( zypp::Dep::SUGGESTS	), p2->dep( zypp::Dep::SUGGESTS		) ) +
	      row( _("Enhances:"),	p1->dep( zypp::Dep::ENHANCES	), p2->dep( zypp::Dep::ENHANCES		) ) +
	      row( _("Supplements:"),	p1->dep( zypp::Dep::SUPPLEMENTS	), p2->dep( zypp::Dep::SUPPLEMENTS	) )
	      );

    return html;
}


QString
YQPkgDependenciesView::row( const QString & heading,
			    const ZyppCap & capSet )
{
    QString content = htmlLines( capSet );

    if ( content.isEmpty() )
	return "";

    return QString( "<tr>" ) +
	hcell( heading ) +
	"<td>" + content + "</td>"
	+ "</tr>";
}


QString
YQPkgDependenciesView::row( const QString & heading,
			    const ZyppCap & capSet1,
			    const ZyppCap & capSet2 )
{
    QString content1 = htmlLines( capSet1 );
    QString content2 = htmlLines( capSet2 );

    if ( content1.isEmpty() && content2.isEmpty() )
	return "";

    return QString( "<tr>" ) +
	hcell( heading ) +
	"<td>" + content1 + "</td>" +
	"<td>" + content2 + "</td>" +
	"</tr>";
}


QString
YQPkgDependenciesView::htmlLines( const ZyppCap & capSet )
{
    QString html;

    for ( ZyppCap::const_iterator it = capSet.begin();
	  it != capSet.end();
	  ++it )
    {
	if ( ! html.isEmpty() )
	    html += "<br>";

	html += htmlEscape( ( *it).asString().c_str() );
    }

    return html;
}
