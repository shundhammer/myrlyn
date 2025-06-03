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


#include <QAction>
#include <QHeaderView>
#include <QMenu>
#include <QTreeWidgetItem>

#include "Logger.h"
#include "YQIconPool.h"
#include "YQi18n.h"
#include "utf8.h"
#include "YQPkgPatchList.h"

#ifndef VERBOSE_FILTER_VIEWS
#  define VERBOSE_FILTER_VIEWS  0
#endif

#define VERBOSE_PATCHES         0
#define ENABLE_DELETING_PATCHES 1


YQPkgPatchList::YQPkgPatchList( QWidget * parent )
    : YQPkgObjList( parent )
{
    logDebug() << "Creating patch list" << endl;

    _filterCriteria = RelevantPatches;

    int numCol = 0;

    QStringList headers;

    headers << "";             _statusCol  = numCol++;
    headers << _( "Summary" ); _summaryCol = numCol++;

    setHeaderLabels( headers );
    setIndentation( 0 );
    setAllColumnsShowFocus( true );
    setSortingEnabled( true );

    header()->setSectionResizeMode( _statusCol,  QHeaderView::ResizeToContents );
    header()->setSectionResizeMode( _summaryCol, QHeaderView::Stretch          );

    connect( this, SIGNAL( currentItemChanged( QTreeWidgetItem *,
                                               QTreeWidgetItem* ) ),
             this, SLOT  ( filter() ) );

    fillList();

    logDebug() << "Creating patch list done" << endl;
}


YQPkgPatchList::~YQPkgPatchList()
{
    // NOP
}


YQPkgPatchCategoryItem *
YQPkgPatchList::category( YQPkgPatchCategory category )
{
    YQPkgPatchCategoryItem * cat = _categories[ category ];

    if ( ! cat )
    {
#if VERBOSE_PATCHES
        logDebug() << "New patch category \""<< category << "\"" << endl;
#endif

        cat = new YQPkgPatchCategoryItem( category, this );
        Q_CHECK_PTR( cat );
        _categories.insert( category, cat );
    }

    return cat;
}

void
YQPkgPatchList::setFilterCriteria( FilterCriteria filterCriteria )
{
    _filterCriteria = filterCriteria;
}


bool
YQPkgPatchList::haveAnyPatches()
{
    return ! zyppPool().empty<zypp::Patch>();
}


bool
YQPkgPatchList::haveNeededPatches()
{
    for ( ZyppPoolIterator it = zyppPatchesBegin();
          it != zyppPatchesEnd();
          ++it )
    {
        ZyppSel   selectable = *it;
        ZyppPatch zyppPatch  = tryCastToZyppPatch( selectable->theObj() );

        if ( isNeeded( selectable, zyppPatch ) )
            return true;
    }

    return false; // No needed patches found
}


int
YQPkgPatchList::countNeededPatches()
{
    int count = 0;

    for ( ZyppPoolIterator it = zyppPatchesBegin();
          it != zyppPatchesEnd();
          ++it )
    {
        ZyppSel   selectable = *it;
        ZyppPatch zyppPatch  = tryCastToZyppPatch( selectable->theObj() );

        if ( isNeeded( selectable, zyppPatch ) )
            ++count;
    }

    return count;
}


bool YQPkgPatchList::isNeeded( ZyppSel selectable, ZyppPatch zyppPatch )
{
    if ( zyppPatch )
    {
        if ( selectable->hasCandidateObj() &&
             selectable->candidateObj().isRelevant() )
        {
            if ( ! selectable->candidateObj().isSatisfied() ||
                 selectable->candidateObj().status().isToBeInstalled() )
            {
                return true;
            }
        }
    }

    return false;
}


void
YQPkgPatchList::fillList()
{
    // wee need to do a full solve in order
    // to get the satisfied status correctly

    _categories.clear();

    clear();
    // logDebug() << "Filling patch list" << endl;

    for ( ZyppPoolIterator it = zyppPatchesBegin();
          it != zyppPatchesEnd();
          ++it )
    {
        ZyppSel   selectable = *it;
        ZyppPatch zyppPatch  = tryCastToZyppPatch( selectable->theObj() );

        if ( zyppPatch )
        {
            bool displayPatch = false;

            switch ( _filterCriteria )
            {
                case RelevantPatches:   // needed + broken + satisfied (but not installed)
                    displayPatch = isNeeded( selectable, zyppPatch );
                    break;

                case RelevantAndInstalledPatches:       // patches we dont need

                    // only shows patches relevant to the system
                    if ( ( selectable->hasCandidateObj() ) &&
                         ( ! selectable->candidateObj().isRelevant()
                           || ( selectable->candidateObj().isSatisfied() &&
                                ! selectable->candidateObj().status().isToBeInstalled() ) ) )
                    {
                        // now we show satisfied patches too
                        displayPatch = true;
                    }
                    break;

                case AllPatches:
                    displayPatch = true;
                    break;

                default:
                    logDebug() << "unknown patch filter" << endl;
                    break;

            }

            if ( displayPatch )
            {
#if VERBOSE_PATCHES
                logDebug() << "Displaying patch " << zyppPatch->name()
                           << " - " <<  zyppPatch->summary()
                           << endl;
#endif
                addPatchItem( *it, zyppPatch);
            }
        }
        else
        {
            logError() << "Found non-patch selectable" << endl;
        }
    }

#if VERBOSE_PATCHES
    logDebug() << "Patch list filled" << endl;
#endif

    resizeColumnToContents( _statusCol );
}


void
YQPkgPatchList::message( const QString & text )
{
    QY2ListViewItem * item = new QY2ListViewItem( this );
    Q_CHECK_PTR( item );

    item->setText( 1, text );
    item->setBackground( 0, QColor( 0xE0, 0xE0, 0xF8 ) );
}


void
YQPkgPatchList::showFilter( QWidget * newFilter )
{
    if ( newFilter == this )
        filter();
}


void
YQPkgPatchList::filter()
{
#if VERBOSE_FILTER_VIEWS
    logVerbose() << "Filtering" << endl;
#endif

    emit filterStart();

    if ( selection() )
    {
        ZyppPatch patch = selection()->zyppPatch();

        if ( patch )
        {
            zypp::Patch::Contents contents( patch->contents() );

            for ( zypp::Patch::Contents::Selectable_iterator it = contents.selectableBegin();
                  it != contents.selectableEnd();
                  ++it )
            {
                ZyppPkg zyppPkg = tryCastToZyppPkg( (*it)->theObj() );

                if ( zyppPkg )
                {
                    emit filterMatch( *it, zyppPkg );
                }
            }
        }
        else
        {
            logInfo() << "patch is bogus" << endl;
        }

    }

    emit filterFinished();
}


void
YQPkgPatchList::addPatchItem( ZyppSel   selectable,
                              ZyppPatch zyppPatch )
{
    if ( ! selectable || ! zyppPatch )
    {
        logError() << "NULL ZyppSel!" << endl;
        return;
    }

    YQPkgPatchCategory       ncat = YQPkgPatchCategoryItem::patchCategory( zyppPatch->category() );
    YQPkgPatchCategoryItem * cat  = category( ncat );
    YQPkgPatchListItem *     item = 0;

    if ( cat )
        item = new YQPkgPatchListItem( this, cat, selectable, zyppPatch );
    else
        item = new YQPkgPatchListItem( this, selectable, zyppPatch );

    if (item)
        applyExcludeRules( item );

}


YQPkgPatchListItem *
YQPkgPatchList::selection() const
{
    QTreeWidgetItem * item = currentItem();

    return item ? dynamic_cast<YQPkgPatchListItem *> (item) : 0;
}


void
YQPkgPatchList::createNotInstalledContextMenu()
{
    _notInstalledContextMenu = new QMenu( this );
    Q_CHECK_PTR( _notInstalledContextMenu );

    _notInstalledContextMenu->addAction( actionSetCurrentInstall     );
    _notInstalledContextMenu->addAction( actionSetCurrentDontInstall );
    _notInstalledContextMenu->addAction( actionSetCurrentTaboo       );

    addAllInListSubMenu( _notInstalledContextMenu );
}


void
YQPkgPatchList::createInstalledContextMenu()
{
    _installedContextMenu = new QMenu( this );
    Q_CHECK_PTR( _installedContextMenu );

    _installedContextMenu->addAction( actionSetCurrentKeepInstalled );

#if ENABLE_DELETING_PATCHES
    _installedContextMenu->addAction( actionSetCurrentDelete );
#endif

    _installedContextMenu->addAction( actionSetCurrentUpdate      );
    _installedContextMenu->addAction( actionSetCurrentUpdateForce );
    _installedContextMenu->addAction( actionSetCurrentProtected   );

    addAllInListSubMenu( _installedContextMenu );
}


QMenu *
YQPkgPatchList::addAllInListSubMenu( QMenu * menu )
{
    QMenu * submenu = new QMenu( menu );
    Q_CHECK_PTR( submenu );

    submenu->addAction( actionSetListInstall       );
    submenu->addAction( actionSetListDontInstall   );
    submenu->addAction( actionSetListKeepInstalled );

#if ENABLE_DELETING_PATCHES
    submenu->addAction( actionSetListDelete );
#endif

    submenu->addAction( actionSetListUpdate      );
    submenu->addAction( actionSetListUpdateForce );
    submenu->addAction( actionSetListTaboo       );
    submenu->addAction( actionSetListProtected   );

    QAction *action = menu->addMenu( submenu );
    action->setText(_( "&All in This List" ));

    return submenu;
}


void
YQPkgPatchList::keyPressEvent( QKeyEvent * event )
{
    if ( event )
    {
#if ! ENABLE_DELETING_PATCHES
        if ( event->ascii() == '-' )
        {
            QTreeWidgetItem * selectedListViewItem = currentItem();

            if ( selectedListViewItem )
            {
                YQPkgPatchListItem * item = dynamic_cast<YQPkgPatchListItem *> (selectedListViewItem);

                if ( item && item->selectable()->hasInstalledObj() )
                {
                    logWarning() << "Deleting patches is not supported" << endl;
                    return;
                }
            }
        }
#endif
    }

    YQPkgObjList::keyPressEvent( event );
}


void
YQPkgPatchList::selectSomething()
{
    QTreeWidgetItemIterator it( this );

    while ( *it )
    {
        YQPkgPatchListItem * patchItem =
            dynamic_cast<YQPkgPatchListItem *>( *it );

        if ( patchItem ) // Select a real patch, not a category
        {
            setCurrentItem( patchItem ); // Sends a signal
            return;
        }

        ++it;
    }
}




YQPkgPatchListItem::YQPkgPatchListItem( YQPkgPatchList *         patchList,
                                        YQPkgPatchCategoryItem * parentCategory,
                                        ZyppSel                  selectable,
                                        ZyppPatch                zyppPatch )
    : YQPkgObjListItem( patchList,
                        parentCategory,
                        selectable,
                        zyppPatch )
    , _patchList( patchList )
    , _zyppPatch( zyppPatch )
{

    init();
}


YQPkgPatchListItem::YQPkgPatchListItem( YQPkgPatchList * patchList,
                                        ZyppSel          selectable,
                                        ZyppPatch        zyppPatch )
    : YQPkgObjListItem( patchList,
                        selectable,
                        zyppPatch )
    , _patchList( patchList )
    , _zyppPatch( zyppPatch )
{
    init();

}


void YQPkgPatchListItem::init()
{
    setStatusIcon();

    if ( summaryCol() > -1 && _zyppPatch->summary().empty() )
        setText( summaryCol(), _zyppPatch->name() ); // use name as fallback
}


YQPkgPatchListItem::~YQPkgPatchListItem()
{
    // NOP
}

void
YQPkgPatchListItem::cycleStatus()
{
    YQPkgObjListItem::cycleStatus();

    if ( status() == S_Del )    // Can't delete patches
        setStatus( S_KeepInstalled );
}


QString
YQPkgPatchListItem::toolTip( int col )
{
    QString text;

    if ( col == statusCol() )
    {
        text = YQPkgObjListItem::toolTip( col );
    }
    else
    {
        text = fromUTF8( zyppPatch()->category() );

        if ( ! text.isEmpty() )
            text += "\n";

        text += fromUTF8( zyppPatch()->downloadSize().asString().c_str() );
    }

    return text;
}


void
YQPkgPatchListItem::applyChanges()
{
    solveResolvableCollections();
}


bool YQPkgPatchListItem::operator<( const QTreeWidgetItem & otherListViewItem ) const
{
    const YQPkgPatchListItem * other =
        dynamic_cast<const YQPkgPatchListItem *> (&otherListViewItem);

    if ( other )
    {
        return ( text(_patchList->summaryCol() ) < other->text( _patchList->summaryCol() ) );
    }

    return YQPkgObjListItem::operator<( otherListViewItem );
}


YQPkgPatchCategoryItem::YQPkgPatchCategoryItem( YQPkgPatchCategory category,
                                                YQPkgPatchList *   patchList )
    : QY2ListViewItem( patchList )
    , _patchList( patchList )
{
    _category = category;

    if ( _patchList->summaryCol() > -1 )
        setText( _patchList->summaryCol(), YQPkgPatchCategoryItem::asString( _category ) );

    setExpanded( true );
    setTreeIcon();

    QFont categoryFont = font( _patchList->summaryCol() );
    categoryFont.setBold(true);

    QFontMetrics metrics( categoryFont );
    categoryFont.setPixelSize( int ( metrics.height() * 1.05 ) );

    setFont( _patchList->summaryCol(), categoryFont );
}


YQPkgPatchCategory
YQPkgPatchCategoryItem::patchCategory( const std::string & category )
{
    return patchCategory( fromUTF8( category ) );
}


YQPkgPatchCategory
YQPkgPatchCategoryItem::patchCategory( const QString & cat )
{
    QString category = cat.toLower();

    if ( category == "yast"        ) return YQPkgYaSTPatch;
    if ( category == "security"    ) return YQPkgSecurityPatch;
    if ( category == "recommended" ) return YQPkgRecommendedPatch;
    if ( category == "optional"    ) return YQPkgOptionalPatch;
    if ( category == "feature"     ) return YQPkgFeaturePatch;
    if ( category == "document"    ) return YQPkgDocumentPatch;

    logWarning() << "Unknown patch category \"" << category << "\"" << endl;

    return YQPkgOptionalPatch;
}


QString
YQPkgPatchCategoryItem::asString( YQPkgPatchCategory category )
{
    switch ( category )
    {
        // Translators: These are patch categories
        case YQPkgYaSTPatch:        return _( "YaST"        );
        case YQPkgSecurityPatch:    return _( "security"    );
        case YQPkgRecommendedPatch: return _( "recommended" );
        case YQPkgOptionalPatch:    return _( "optional"    );
        case YQPkgFeaturePatch:     return _( "feature"    );
        case YQPkgDocumentPatch:    return _( "document"    );
    }

    return "";
}


YQPkgPatchCategoryItem::~YQPkgPatchCategoryItem()
{
    // NOP
}


void
YQPkgPatchCategoryItem::addPatch( ZyppPatch patch )
{
    if ( ! _firstPatch )
        _firstPatch = patch;
}


void
YQPkgPatchCategoryItem::setTreeIcon()
{
    setIcon( _patchList->iconCol(),
             isExpanded() ?
             YQIconPool::arrowDown() :
             YQIconPool::arrowRight()   );
}


bool YQPkgPatchCategoryItem::operator<( const QTreeWidgetItem & otherListViewItem ) const
{
    const YQPkgPatchCategoryItem * otherCategoryItem =
        dynamic_cast<const YQPkgPatchCategoryItem *>( &otherListViewItem );

    return category() > otherCategoryItem->category();
}



