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
              (c) 2026 Stefan.Hundhammer@gmx.de
    License:  GPL V2 - See file LICENSE for details.

 */


#include <QVBoxLayout>
#include <QSplitter>

#include "Exception.h"
#include "Logger.h"
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


void YQPkgSecondaryFilterView::init( QWidget * primaryWidget )
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
}


YQPkgSecondaryFilterView::~YQPkgSecondaryFilterView()
{
    // NOP
}


QWidget *
YQPkgSecondaryFilterView::layoutSecondaryFilters( QWidget * parent, QWidget * primaryWidget )
{
    QWidget * vbox = new QWidget( parent );
    CHECK_NEW( vbox );

    QVBoxLayout * layout = new QVBoxLayout();
    CHECK_NEW( layout );

    vbox->setLayout( layout );
    layout->setContentsMargins( 0, 15, 0, 0 ); // left / top / right / bottom

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
    _secondaryFilters->addEmptyPage( _( "All Packages" ), _allPackages );


    //
    // Installed Packages
    //

    _installedPackages = new QWidget( this );
    CHECK_NEW( _installedPackages );
    _secondaryFilters->addEmptyPage( _( "Installed Packages" ), _installedPackages );


    //
    // Not Installed Packages
    //

    _notInstalledPackages = new QWidget( this );
    CHECK_NEW( _notInstalledPackages );
    _secondaryFilters->addEmptyPage( _( "Not Installed Packages" ), _notInstalledPackages );


    //
    // Package search view
    // (only with the basic search fields to save some screen space)
    //

    _searchFilterView = new YQPkgSearchFilterView( this, YQPkgSearchFilterView::BasicSearchFields );
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

    _secondaryFilters->addPage( _( "Status" ), _statusFilterView );
    // Collapse the secondary filters whenever "All Packages" is selected

    connect( _statusFilterView, SIGNAL( filterStart() ),
             primaryWidget,     SLOT  ( filter()      ) );

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
    logVerbose() << metaObject()->className() << ": Filtering" << endl;
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
    QWidget * page = currentPage();

    if ( page == _allPackages )
    {
        return true;
    }
    else if ( page == _installedPackages )
    {
        return ( isInstalled( selectable ) );
    }
    else if ( page == _notInstalledPackages )
    {
        return ( ! isInstalled( selectable ) );
    }
    else if ( page == _searchFilterView )
    {
        return _searchFilterView->check( selectable, pkg );
    }
    else if ( page == _statusFilterView )
    {
        return _statusFilterView->check( selectable, pkg );
    }

    return true;
}


bool
YQPkgSecondaryFilterView::isInstalled( ZyppSel selectable ) const
{
    switch ( selectable->status() )
    {
        case S_KeepInstalled:
        case S_Protected:

        case S_Install:
        case S_Update:
        case S_Del:

        case S_AutoInstall:
        case S_AutoUpdate:
        case S_AutoDel:
            return true;

        case S_NoInst:
        case S_Taboo:
            return false;

            // No 'default' branch to let the compiler catch
            // unhandled enum values
    }

    /*NOTREACHED*/
    return false;
}
