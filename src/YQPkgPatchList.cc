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


#include "YQi18n.h"
#include "utf8.h"

#include <zypp/base/Logger.h>
#include <set>

#include <QPainter>
#include <QMenu>
#include <QAction>
#include <QEvent>
#include <QHeaderView>

#include "YQPkgPatchList.h"
#include "YQPkgTextDialog.h"
#include "YQIconPool.h"

#include "Logger.h"
#include "Exception.h"

#define ENABLE_DELETING_PATCHES 1
#define VERBOSE_PATCHES         0


using std::list;
using std::set;


YQPkgPatchList::YQPkgPatchList( QWidget * parent )
    : YQPkgObjList( parent )
{
    logDebug() << "Creating patch list" << Qt::endl;

    _filterCriteria = RelevantPatches;

    int numCol = 0;

    QStringList headers;

    headers << "";             _statusCol  = numCol++;
    headers << _( "Summary" ); _summaryCol = numCol++;

    setHeaderLabels(headers);
    setIndentation( 0 );
    setAllColumnsShowFocus( true );
    setSortingEnabled( true );

    header()->setSectionResizeMode( _statusCol,  QHeaderView::ResizeToContents );
    header()->setSectionResizeMode( _summaryCol, QHeaderView::Stretch          );


    connect( this, SIGNAL( currentItemChanged( QTreeWidgetItem *,
                                               QTreeWidgetItem* ) ),
             this, SLOT  ( filter() ) );


    fillList();

    logDebug() << "Creating patch list done" << Qt::endl;
}


YQPkgPatchList::~YQPkgPatchList()
{
    // NOP
}


void
YQPkgPatchList::polish()
{
    // Delayed initialization after widget is fully created etc.

    // Only now send currentItemChanged() signal so attached details views also
    // display something if their showDetailsIfVisible() slot is connected to
    // currentItemChanged() signals.

    selectSomething();
}


YQPkgPatchCategoryItem *
YQPkgPatchList::category( YQPkgPatchCategory category )
{
    YQPkgPatchCategoryItem * cat = _categories[ category ];

    if ( ! cat )
    {
#if VERBOSE_PATCHES
        logDebug() << "New patch category \""<< category << "\"" << Qt::endl;
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


void
YQPkgPatchList::fillList()
{
    // wee need to do a full solve in order
    // to get the satisfied status correctly

    _categories.clear();

    clear();
    // logDebug() << "Filling patch list" << Qt::endl;

    for ( ZyppPoolIterator it = zyppPatchesBegin();
          it != zyppPatchesEnd();
          ++it )
    {
        ZyppSel   selectable = *it;
        ZyppPatch zyppPatch = tryCastToZyppPatch( selectable->theObj() );

        if ( zyppPatch )
        {
            bool displayPatch = false;

            switch ( _filterCriteria )
            {
                case RelevantPatches:   // needed + broken + satisfied (but not installed)

                    // only shows patches relevant to the system
                    if ( selectable->hasCandidateObj() &&
                         selectable->candidateObj().isRelevant() )
                    {
                        // and only those that are needed
                        if ( ! selectable->candidateObj().isSatisfied() ||
                             // maybe it's satisfied because it's preselected
                             selectable->candidateObj().status().isToBeInstalled() )
                        {
                            displayPatch = true;
                        }
                    }
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
                    logDebug() << "unknown patch filter" << Qt::endl;
                    break;

            }

            if ( displayPatch )
            {
#if VERBOSE_PATCHES
                logDebug() << "Displaying patch " << zyppPatch->name()
                           << " - " <<  zyppPatch->summary()
                           << Qt::endl;
#endif
                addPatchItem( *it, zyppPatch);
            }
        }
        else
        {
            logError() << "Found non-patch selectable" << Qt::endl;
        }
    }

#if VERBOSE_PATCHES
    logDebug() << "Patch list filled" << Qt::endl;
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
YQPkgPatchList::filterIfVisible()
{
    if ( isVisible() )
        filter();
}


void
YQPkgPatchList::filter()
{
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
            logInfo() << "patch is bogus" << Qt::endl;
        }

    }
    else
        logWarning() << "selection empty" << Qt::endl;

    emit filterFinished();
}


void
YQPkgPatchList::addPatchItem( ZyppSel   selectable,
                              ZyppPatch zyppPatch )
{
    if ( ! selectable || ! zyppPatch )
    {
        logError() << "NULL ZyppSel!" << Qt::endl;
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
                    logWarning() << "Deleting patches is not supported" << Qt::endl;
                    return;
                }
            }
        }
#endif
    }

    YQPkgObjList::keyPressEvent( event );
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
YQPkgPatchCategoryItem::patchCategory( const string & category )
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
    if ( category == "document"    ) return YQPkgDocumentPatch;

    logWarning() << "Unknown patch category \"" << category << "\"" << Qt::endl;

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
YQPkgPatchCategoryItem::setExpanded( bool open )
{
    QTreeWidgetItem::setExpanded( open );
    setTreeIcon();
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



