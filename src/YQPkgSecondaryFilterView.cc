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


#include <QVBoxLayout>
#include <QSplitter>

#include "Exception.h"
#include "Logger.h"
#include "QY2ComboTabWidget.h"
#include "YQPkgSearchFilterView.h"
#include "YQPkgStatusFilterView.h"
#include "YQi18n.h"
#include "YQPkgSecondaryFilterView.h"

#ifndef VERBOSE_FILTER_VIEWS
#  define VERBOSE_FILTER_VIEWS  0
#endif


YQPkgSecondaryFilterView::YQPkgSecondaryFilterView( QWidget * parent )
    : QWidget( parent )
{
}


void YQPkgSecondaryFilterView::init(QWidget * primaryWidget)
{
    QHBoxLayout *layout = new QHBoxLayout( this );
    CHECK_NEW( layout );
    layout->setContentsMargins( 0, 0, 0, 0);

    QSplitter * splitter = new QSplitter( Qt::Vertical, this );
    CHECK_NEW( splitter );

    layout->addWidget( splitter );
    splitter->addWidget( primaryWidget );

    primaryWidget->setSizePolicy( QSizePolicy( QSizePolicy::Ignored, QSizePolicy::Expanding ) );// hor/vert

    // Directly propagate signals filterStart() and filterFinished()
    // from the primary filter to the outside

    connect( primaryWidget, SIGNAL( filterStart() ),
             this,          SIGNAL( filterStart() ) );

    connect( primaryWidget, SIGNAL( filterFinished() ),
             this,          SIGNAL( filterFinished() ) );

    // Redirect filterMatch() and filterNearMatch() signals to the secondary filter

    connect( primaryWidget, SIGNAL( filterMatch             ( ZyppSel, ZyppPkg ) ),
             this,          SLOT  ( primaryFilterMatch      ( ZyppSel, ZyppPkg ) ) );

    connect( primaryWidget, SIGNAL( filterNearMatch         ( ZyppSel, ZyppPkg ) ),
             this,          SLOT  ( primaryFilterNearMatch  ( ZyppSel, ZyppPkg ) ) );

    layoutSecondaryFilters( splitter, primaryWidget );

    splitter->setStretchFactor( 0, 5 );
    splitter->setStretchFactor( 1, 1 );
    splitter->setStretchFactor( 2, 3 );
}


YQPkgSecondaryFilterView::~YQPkgSecondaryFilterView()
{
    // NOP
}


QWidget *
YQPkgSecondaryFilterView::layoutSecondaryFilters( QWidget * parent, QWidget * primaryWidget )
{
    QWidget *vbox = new QWidget( parent );
    CHECK_NEW( vbox );

    QVBoxLayout *layout = new QVBoxLayout();
    CHECK_NEW( layout );

    vbox->setLayout( layout );
    layout->setContentsMargins( 0, 0, 0, 0 );

    // Translators: This is a combo box where the user can apply a secondary filter
    // in addition to the primary filter by repository.

    _secondaryFilters = new QY2ComboTabWidget( _( "&Secondary Filter:" ));
    CHECK_NEW( _secondaryFilters );
    layout->addWidget( _secondaryFilters );

    //
    // All Packages
    //

    _allPackages = new QWidget( this );
    CHECK_NEW( _allPackages );
    _secondaryFilters->addPage( _( "All Packages" ), _allPackages );



    //
    // Package search view
    //

    _searchFilterView = new YQPkgSearchFilterView( this );
    CHECK_NEW( _searchFilterView );
    _secondaryFilters->addPage( _( "Search" ), _searchFilterView );

    connect( _searchFilterView, SIGNAL( filterStart() ),
             primaryWidget,     SLOT  ( filter()      ) );

    connect( _secondaryFilters, SIGNAL( currentChanged( QWidget * ) ),
             this,              SLOT  ( filter()                    ) );


    //
    // Status filter view
    //

    _statusFilterView = new YQPkgStatusFilterView( parent );
    CHECK_NEW( _statusFilterView );

    _secondaryFilters->addPage( _( "Installation Summary" ), _statusFilterView );

    connect( _statusFilterView, SIGNAL( filterStart() ),
             primaryWidget,     SLOT  ( filter()      ) );


    // Collapse the secondary filters whenever "All Packages" is selected
    _secondaryFilters->setMinimizePage( _allPackages );

    return _secondaryFilters;
}


void YQPkgSecondaryFilterView::showFilter( QWidget * newFilter )
{
    if ( newFilter == this )
        filter();
}


void YQPkgSecondaryFilterView::filter()
{
#if VERBOSE_FILTER_VIEWS
    logVerbose() << metaObject()->className() << ": Filtering" << Qt::endl;
#endif

    primaryFilter();
}


void YQPkgSecondaryFilterView::primaryFilterMatch( ZyppSel selectable,
                                                   ZyppPkg pkg )
{
    if ( secondaryFilterMatch( selectable, pkg ) )
        emit filterMatch( selectable, pkg );
}


void YQPkgSecondaryFilterView::primaryFilterNearMatch( ZyppSel  selectable,
                                                       ZyppPkg  pkg )
{
    if ( secondaryFilterMatch( selectable, pkg ) )
        emit filterNearMatch( selectable, pkg );
}


bool
YQPkgSecondaryFilterView::secondaryFilterMatch( ZyppSel selectable,
                                                ZyppPkg pkg )
{
    if ( _allPackages->isVisible() )
    {
        return true;
    }
    else if ( _searchFilterView->isVisible() )
    {
        return _searchFilterView->check( selectable, pkg );
    }
    else if ( _statusFilterView->isVisible() )
    {
        return _statusFilterView->check( selectable, pkg );
    }

    return true;
}

