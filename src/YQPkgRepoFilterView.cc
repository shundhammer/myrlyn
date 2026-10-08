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
    , _primaryFilter( 0 )
    , _outerLayout( 0 )
    , _repoList( 0 )
{
    layoutPrimaryFilter();
    layoutButtons();

    layoutSplitter( _primaryFilter );
    connectFilter( _repoList );
}


YQPkgRepoFilterView::~YQPkgRepoFilterView()
{
    // NOP
}


void
YQPkgRepoFilterView::layoutPrimaryFilter()
{
    _primaryFilter = new QWidget( this );
    CHECK_NEW( _primaryFilter );

    _outerLayout = new QVBoxLayout( _primaryFilter );
    CHECK_NEW( _outerLayout );
    _outerLayout->setContentsMargins( 0, 0, 0, 0 ); // left / top / right / bottom

    // Repo List

    _repoList = new YQPkgRepoList( _primaryFilter );
    CHECK_NEW( _repoList );

    _outerLayout->addWidget( _repoList );
}


void
YQPkgRepoFilterView::layoutButtons()
{
    // Buttons (same width, centered):
    //
    // buttonHBox
    //   hstretch
    //   buttonVBox
    //     _protectAllButton
    //     _unProtectAllButton
    //   hstretch
    //
    //  |    [ Protect All  ]    |
    //  |    [Un-Protect All]    |

    CHECK_PTR( _outerLayout );

    QHBoxLayout * buttonHBox = new QHBoxLayout();
    _outerLayout->addLayout( buttonHBox );
    buttonHBox->setContentsMargins( 0, 7, 0, 7 ); // left / top / right / bottom
    buttonHBox->addStretch( 1 );

    QVBoxLayout * buttonVBox = new QVBoxLayout();
    buttonHBox->addLayout( buttonVBox );
    buttonVBox->setSpacing( 7 );

    _protectAllButton = new QPushButton( _( "Protect All" ), _primaryFilter );
    CHECK_NEW( _protectAllButton );
    buttonVBox->addWidget( _protectAllButton );

    connect( _protectAllButton, SIGNAL( clicked()    ),
             this,              SIGNAL( protectAll() ) );

    _unProtectAllButton = new QPushButton( _( "Un-Protect All" ), _primaryFilter );
    CHECK_NEW( _unProtectAllButton );
    buttonVBox->addWidget( _unProtectAllButton );

    connect( _unProtectAllButton, SIGNAL( clicked()      ),
             this,                SIGNAL( unProtectAll() ) );

    buttonHBox->addStretch( 1 );
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
