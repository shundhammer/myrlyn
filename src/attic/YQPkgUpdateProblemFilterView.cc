/*  ------------------------------------------------------
              __   _____  ____  _
              \ \ / / _ \|  _ \| | ____ _
               \ V / | | | |_) | |/ / _` |
                | || |_| |  __/|   < (_| |
                |_| \__\_\_|   |_|\_\__, |
                                    |___/
    ------------------------------------------------------

    Project:  YQPkg Package Selector
    Copyright (c) 2024 SUSE LLC
    License:  GPL V2 - See file LICENSE for details.

    Textdomain "qt-pkg"
 */


#include "Logger.h"

#include "YQi18n.h"
#include "utf8.h"

#include <zypp/ZYppFactory.h>
#include <zypp/Resolver.h>

#include "YQPkgUpdateProblemFilterView.h"


using std::list;


YQPkgUpdateProblemFilterView::YQPkgUpdateProblemFilterView( QWidget * parent )
    : QTextBrowser( parent )
{
    QString html = _( "\
<br>\n\
<h2>Update Problem</h2>\n\
<p>\n\
The packages in this list cannot be updated automatically.\n\
</p>\n\
<p>Possible reasons:</p>\n\
<ul>\n\
<li>They are obsoleted by other packages\n\
<li>There is no newer version to update to on any installation media\n\
<li>They are third-party packages\n\
</ul>\n\
</p>\n\
<p>\n\
Please choose manually what to do with them.\n\
The safest course of action is to delete them.\n\
</p>\
" );

    setHtml( html );
}


YQPkgUpdateProblemFilterView::~YQPkgUpdateProblemFilterView()
{
    // NOP
}


void
YQPkgUpdateProblemFilterView::showFilter( QWidget * newFilter )
{
    if ( newFilter == this )
        filter();
}


void
YQPkgUpdateProblemFilterView::filter()
{
    emit filterStart();

    list<zypp::PoolItem> problemList = zypp::getZYpp()->resolver()->problematicUpdateItems();

    for ( list<zypp::PoolItem>::const_iterator it = problemList.begin();
	  it != problemList.end();
	  ++it )
    {
	ZyppPkg pkg = tryCastToZyppPkg( (*it).resolvable() );

	if ( pkg )
	{
	    ZyppSel sel = _selMapper.findZyppSel( pkg );

	    if ( sel )
	    {
		logInfo() << "Problematic package: "
                          << pkg->name() << "-" << pkg->edition().asString()
                          << Qt::endl;

		emit filterMatch( sel, pkg );
	    }
	}

    }

    emit filterFinished();
}


bool
YQPkgUpdateProblemFilterView::haveProblematicPackages()
{
    return ! zypp::getZYpp()->resolver()->problematicUpdateItems().empty();
}


