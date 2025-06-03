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



#include "Exception.h"
#include "Logger.h"
#include "YQPkgRepoList.h"
#include "YQPkgRepoFilterView.h"


YQPkgRepoFilterView::YQPkgRepoFilterView( QWidget * parent )
    : YQPkgSecondaryFilterView( parent )
{
    _repoList = new YQPkgRepoList( this );
    CHECK_NEW( _repoList );

    init( _repoList) ;
}


YQPkgRepoFilterView::~YQPkgRepoFilterView()
{
    // NOP
}


ZyppRepo
YQPkgRepoFilterView::selectedRepo() const
{
    YQPkgRepoListItem * selection = _repoList->selection();

    if ( selection && selection->zyppRepo() )
        return selection->zyppRepo();

    return zypp::Repository::noRepository;
}


void YQPkgRepoFilterView::primaryFilter()
{
    _repoList->filter();
}
