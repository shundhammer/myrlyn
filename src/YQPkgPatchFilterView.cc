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


#include <QComboBox>
#include <QElapsedTimer>
#include <QLabel>
#include <QSplitter>
#include <QTabWidget>
#include <QVBoxLayout>

#include <zypp/Patch.h>

#include "Logger.h"
#include "YQPkgDescriptionView.h"
#include "YQPkgPatchList.h"
#include "YQi18n.h"

#include "YQPkgPatchFilterView.h"

#ifndef VERBOSE_FILTER_VIEWS
#  define VERBOSE_FILTER_VIEWS  0
#endif



typedef zypp::Patch::Contents                   ZyppPatchContents;
typedef zypp::Patch::Contents::const_iterator   ZyppPatchContentsIterator;


YQPkgPatchFilterView::YQPkgPatchFilterView( QWidget * parent )
    : QWidget( parent )
{
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins( 0, 0, 0, 0 );
    setLayout( layout );

    _splitter = new QSplitter( Qt::Vertical, this );
    layout->addWidget( _splitter );

    QWidget *     upper_box = new QWidget( _splitter );
    QVBoxLayout * vbox      = new QVBoxLayout( upper_box );
    _patchList = new YQPkgPatchList( upper_box );

    vbox->addWidget( _patchList );

    QHBoxLayout * hbox = new QHBoxLayout();
    vbox->addLayout( hbox );
    vbox->setContentsMargins( 0, 0, 0, 0 );


    QLabel * label = new QLabel( _( "&Show Patch Category:" ), upper_box );
    hbox->addWidget(label);

    _patchFilter = new QComboBox( upper_box );
    hbox->addWidget(_patchFilter);

    _patchFilter->addItem( _( "Needed Patches"   ) );
    _patchFilter->addItem( _( "Unneeded Patches" ) );
    _patchFilter->addItem( _( "All Patches"      ), 2 );
    _patchFilter->setCurrentIndex( 0 );

    label->setBuddy( _patchFilter );

    connect( _patchFilter, SIGNAL( activated( int ) ),
             this,         SLOT  ( fillPatchList()  ) );

    _detailsViews = new QTabWidget( _splitter );

    _descriptionView = new YQPkgDescriptionView( _detailsViews );

    _detailsViews->addTab( _descriptionView, _( "Patch Description" ) );

    connect( _patchList,        SIGNAL( currentItemChanged  ( ZyppSel ) ),
             _descriptionView,  SLOT  ( showDetailsIfVisible( ZyppSel ) ) );
}


YQPkgPatchFilterView::~YQPkgPatchFilterView()
{
    // NOP
}


void
YQPkgPatchFilterView::reset()
{
    fillPatchList();
}


void
YQPkgPatchFilterView::showFilter( QWidget * newFilter )
{
    if ( newFilter == this )
    {
#if VERBOSE_FILTER_VIEWS
        logVerbose() << "Filtering" << Qt::endl;
#endif
        _patchList->filter();
        _patchList->selectSomething();
    }

}


void
YQPkgPatchFilterView::fillPatchList()
{
    switch ( _patchFilter->currentIndex() )
    {
        case 0:  _patchList->setFilterCriteria( YQPkgPatchList::RelevantPatches             );   break;
        case 1:  _patchList->setFilterCriteria( YQPkgPatchList::RelevantAndInstalledPatches );   break;
        case 2:  _patchList->setFilterCriteria( YQPkgPatchList::AllPatches                  );   break;
        default: _patchList->setFilterCriteria( YQPkgPatchList::RelevantPatches             );   break;
    }

    _patchList->fillList();
    _patchList->selectSomething();
}


