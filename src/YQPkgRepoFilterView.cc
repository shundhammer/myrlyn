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


#include <QVBoxLayout>
#include <QPushButton>

#include "Exception.h"
#include "Logger.h"
#include "YQPkgRepoList.h"
#include "YQPkgRepoFilterView.h"
#include "YQi18n.h"


YQPkgRepoFilterView::YQPkgRepoFilterView( QWidget * parent )
    : YQPkgSecondaryFilterView( parent )
{
    logDebug() << endl;

    QWidget * primaryFilter = new QWidget( this );
    CHECK_NEW( primaryFilter );

    QVBoxLayout * layout = new QVBoxLayout( primaryFilter );
    CHECK_NEW( layout );
    layout->setContentsMargins( 0, 0, 0, 0 ); // left / top / right / bottom

    // Repo List

    _repoList = new YQPkgRepoList( primaryFilter );
    CHECK_NEW( _repoList );

    layout->addWidget( _repoList );


    // Buttons

    _protectAllButton = new QPushButton( _( "Protect All" ), primaryFilter );
    CHECK_NEW( _protectAllButton );
    layout->addWidget( _protectAllButton );

    connect( _protectAllButton, SIGNAL( clicked()    ),
             this,              SIGNAL( protectAll() ) );

    _unProtectAllButton = new QPushButton( _( "Un-Protect All" ), primaryFilter );
    CHECK_NEW( _unProtectAllButton );
    layout->addWidget( _unProtectAllButton );

    connect( _unProtectAllButton, SIGNAL( clicked()      ),
             this,                SIGNAL( unProtectAll() ) );


    // Put this all into a splitter

    layoutSplitter( primaryFilter );
    connectFilter( _repoList );
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
