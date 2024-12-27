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


#include "Exception.h"
#include "Logger.h"
#include "YQPkgServiceList.h"
#include "YQZypp.h"

#include "YQPkgServiceFilterView.h"


YQPkgServiceFilterView::YQPkgServiceFilterView( QWidget * parent )
    : YQPkgSecondaryFilterView( parent )
{
    _serviceList = new YQPkgServiceList( this );
    CHECK_NEW( _serviceList );

    init(_serviceList);
}

YQPkgServiceFilterView::~YQPkgServiceFilterView()
{
    // NOP
}

void YQPkgServiceFilterView::primaryFilter()
{
    _serviceList->filter();
}

void YQPkgServiceFilterView::primaryFilterIfVisible()
{
    _serviceList->filterIfVisible();
}

// check if a libzypp service is present
bool YQPkgServiceFilterView::any_service()
{
    bool ret = std::any_of( ZyppRepositoriesBegin(),
                            ZyppRepositoriesEnd(),
                            [&](const zypp::Repository& repo)
                                { return !repo.info().service().empty(); }
                            );

    // if the repository does not belong to any service then the service name is empty

    // logDebug() << "Found a libzypp service: " << ret << Qt::endl;

    return ret;
}
