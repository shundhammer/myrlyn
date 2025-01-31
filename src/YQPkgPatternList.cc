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


#include <QHeaderView>

#include <zypp-core/Pathname.h>
#include <zypp-core/base/String.h>
#include <zypp/Package.h>
#include <zypp/PoolItem.h>
#include <zypp/ResTraits.h>
#include <zypp/ui/Selectable.h>
#include <zypp/ui/Status.h>

#include "Logger.h"
#include "QY2IconLoader.h"
#include "YQIconPool.h"
#include "YQi18n.h"
#include "utf8.h"
#include "YQPkgPatternList.h"

#ifndef VERBOSE_FILTER_VIEWS
#  define VERBOSE_FILTER_VIEWS  0
#endif

#define VERBOSE_PATTERN_LIST   0

using std::string;
using std::set;


YQPkgPatternList::YQPkgPatternList( QWidget * parent,
                                    bool      autoFill,
                                    bool      autoFilter )
    : YQPkgObjList( parent )
    , _orderCol( -1 )
{
    logDebug() << "Creating pattern list" << Qt::endl;

    // Translators: "Pattern" refers to so-called "software patterns",
    // i.e., specific task-oriented groups of packages, like "everything that
    // is needed to run a web server".

    QString patternHeader = _( "Pattern" );
    int numCol = 0;

    QStringList headers;
    headers << "";             _statusCol  = numCol++;
    headers << "";             _iconCol    = numCol++;
    headers << patternHeader;  _summaryCol = numCol++;

    // Set this environment variable to get an "Order" column in the patterns list

    if ( getenv( "Y2_SHOW_PATTERNS_ORDER" ) )
    {
        headers << _( "Order" ); _orderCol = numCol++;
    }

    if ( getenv( "Y2_SHOW_INVISIBLE_PATTERNS" ) )
        _showInvisiblePatterns = true;
    else
        _showInvisiblePatterns = false;

    setHeaderLabels( headers );
    setColumnCount( numCol );

    setIndentation( 0 );
    setSortingEnabled( true );
    sortByColumn( summaryCol(), Qt::AscendingOrder );
    setAllColumnsShowFocus( true );
    setVerticalScrollMode( QAbstractItemView::ScrollPerPixel ); // bsc#1189550

    header()->setSectionResizeMode( statusCol(),  QHeaderView::Fixed   );
    header()->setSectionResizeMode( summaryCol(), QHeaderView::Stretch );
    header()->resizeSection( statusCol(), 25 );

    setColumnWidth( statusCol(),   25 );
    setColumnWidth( summaryCol(), 100 );

    if ( autoFilter )
    {
        connect( this, SIGNAL( currentItemChanged( QTreeWidgetItem *,
                                                   QTreeWidgetItem * ) ),
                 this, SLOT  ( filter()                                ) );
    }

    setIconSize(QSize(32,32));
    header()->resizeSection( iconCol(), 34 );

    if ( autoFill )
    {
        fillList();
#if 0
        selectSomething();

        // This is pointless as long as the package list on the right is not
        // yet created and receiving our signal: It only results in the first
        // pattern selected, but no packages shown for it, so the user has to
        // click on another pattern and then back on this one to see its
        // content. So it's better not to select something at all.
        //
        // A single shot timer might help here, but that would have to be
        // triggered from the calling YQPkgSelector when all widgets are
        // created and connected.
#endif
    }

    logDebug() << "Creating pattern list done" << Qt::endl;
}


YQPkgPatternList::~YQPkgPatternList()
{
    // NOP
}


void
YQPkgPatternList::fillList()
{
    _categories.clear();

    clear();
    logDebug() << "Filling pattern list" << Qt::endl;

    for ( ZyppPoolIterator it = zyppPatternsBegin();
          it != zyppPatternsEnd();
          ++it )
    {
        ZyppPattern zyppPattern = tryCastToZyppPattern( (*it)->theObj() );

        if ( zyppPattern )
        {
            if ( zyppPattern->userVisible() || _showInvisiblePatterns )
            {
                addPatternItem( *it, zyppPattern );
            }
#if VERBOSE_PATTERN_LIST
            else
                logDebug() << "Pattern " << zyppPattern->name()
                           << " is not user-visible" << Qt::endl;
#endif
        }
        else
        {
            logError() << "Found non-Pattern selectable" << Qt::endl;
        }
    }


#if VERBOSE_PATTERN_LIST
    logDebug() << "Pattern list filled" << Qt::endl;
#endif

    resizeColumnToContents( _iconCol   );
    resizeColumnToContents( _statusCol );
}


YQPkgPatternCategoryItem *
YQPkgPatternList::category( const QString & categoryName )
{
    if ( categoryName.isEmpty() )
        return 0;

    YQPkgPatternCategoryItem * cat = _categories[ categoryName ];

    if ( ! cat )
    {
#if VERBOSE_PATTERN_LIST
        logDebug() << "New pattern category \""<< categoryName << "\"" << Qt::endl;
#endif

        cat = new YQPkgPatternCategoryItem( this, categoryName );
        Q_CHECK_PTR( cat );
        _categories.insert( categoryName, cat );
    }

    return cat;
}


void
YQPkgPatternList::showFilter( QWidget * newFilter )
{
    if ( newFilter == this )
    {
        filter();
        selectSomething();
    }
}


void
YQPkgPatternList::filter()
{
#if VERBOSE_FILTER_VIEWS
    logVerbose() << "Filtering" << Qt::endl;
#endif

    emit filterStart();

    if ( selection() )  // The seleted QListViewItem
    {
        ZyppPattern zyppPattern = selection()->zyppPattern();

        if ( zyppPattern )
        {
            int total = 0;
            int installed = 0;

            zypp::Pattern::Contents contents( zyppPattern->contents() );

            for ( zypp::Pattern::Contents::Selectable_iterator it = contents.selectableBegin();
                  it != contents.selectableEnd();
                  ++it )
            {
                ZyppPkg zyppPkg = tryCastToZyppPkg( (*it)->theObj() );

                if ( zyppPkg )
                {
                    if ( (*it)->installedSize() > 0 )
                        ++installed;
                    ++total;

                    emit filterMatch( *it, zyppPkg );
                }
            }

            selection()->setInstalledPackages( installed );
            selection()->setTotalPackages( total );
            selection()->resetToolTip();
        }
    }

    emit filterFinished();
    resizeColumnToContents( _statusCol );
}


void
YQPkgPatternList::addPatternItem( ZyppSel     selectable,
                                  ZyppPattern zyppPattern )
{
    if ( ! selectable )
    {
        logError() << "NULL ZyppSelectable!" << Qt::endl;
        return;
    }

    YQPkgPatternCategoryItem * cat  = category( fromUTF8( zyppPattern->category() ) );
    YQPkgPatternListItem *     item = 0;

    if ( cat )
    {
        item = new YQPkgPatternListItem( this, cat, selectable, zyppPattern );
    }
    else
    {
        item = new YQPkgPatternListItem( this, selectable, zyppPattern );
    }

    resizeColumnToContents( _statusCol  );
    resizeColumnToContents( _summaryCol );

    if ( showOrderCol() )
        resizeColumnToContents( _orderCol   );

    addTopLevelItem( item );
    applyExcludeRules( item );
}


YQPkgPatternListItem *
YQPkgPatternList::selection() const
{
    QTreeWidgetItem * item = currentItem();

    if ( ! item )
        return 0;

    return dynamic_cast<YQPkgPatternListItem *> (item);
}


void
YQPkgPatternList::pkgObjClicked( int               button,
                                 QTreeWidgetItem * listViewItem,
                                 int               col,
                                 const QPoint &    pos )
{
    YQPkgPatternCategoryItem * categoryItem
        = dynamic_cast<YQPkgPatternCategoryItem *> (listViewItem);

    if ( categoryItem )
    {
        if ( button == Qt::LeftButton )
        {
            if ( col == 0 )
            {
                categoryItem->setExpanded( ! categoryItem->isExpanded() );
            }
        }
    }
    else
    {
        YQPkgObjList::pkgObjClicked( button, listViewItem, col, pos );
    }
}


void
YQPkgPatternList::selectSomething()
{
    QTreeWidgetItemIterator it( this );

    while ( *it )
    {
        QTreeWidgetItem * item = *it;
        YQPkgPatternCategoryItem * categoryItem =
            dynamic_cast<YQPkgPatternCategoryItem *>( item );

        if ( ! categoryItem ) // Select a real pattern, not a category
        {
            setCurrentItem( item ); // Sends a signal
            return;
        }

        ++it;
    }
}


YQPkgPatternListItem::YQPkgPatternListItem( YQPkgPatternList * patternList,
                                            ZyppSel            selectable,
                                            ZyppPattern        zyppPattern )
    : YQPkgObjListItem( patternList,
                        selectable,
                        zyppPattern )
    , _patternList( patternList )
    , _zyppPattern( zyppPattern )
    , _total(0), _installed(0)
{
    init();
}


YQPkgPatternListItem::YQPkgPatternListItem( YQPkgPatternList *         patternList,
                                            YQPkgPatternCategoryItem * parentCategory,
                                            ZyppSel                    selectable,
                                            ZyppPattern                zyppPattern )
    : YQPkgObjListItem( patternList,
                        parentCategory,
                        selectable,
                        zyppPattern )
    , _patternList( patternList )
    , _zyppPattern( zyppPattern )
    , _total(0), _installed(0)
{
    init();
    parentCategory->addPattern( _zyppPattern );
}


void
YQPkgPatternListItem::init()
{
    if ( ! _zyppPattern )
        _zyppPattern = tryCastToZyppPattern( selectable()->theObj() );

    if (_zyppPattern)
    {
        string iconName = _zyppPattern->icon().basename();
        if ( iconName.empty() )
            iconName = "pattern-generic";

        setIcon( _patternList->iconCol(), QY2IconLoader::loadIcon( fromUTF8( iconName ) ) );

        if ( _patternList->showInvisiblePatterns() && ! _zyppPattern->userVisible() )
        {
            // The YQPkgObjListItem base class takes care of setting the name
            // and summary columns, but here we want to add something to it.
            // Notice that patterns use the summary column.

            QString name = text( _patternList->summaryCol() );
            name += QString( " [%1]" ).arg( _( "Invisible" ) );
            setText( _patternList->summaryCol(), name );
        }

        if ( _patternList->showOrderCol() )
            setText( _patternList->orderCol(), fromUTF8( _zyppPattern->order() ) );
    }

    setStatusIcon();
    resetToolTip();
    setFirstColumnSpanned ( false );
}




YQPkgPatternListItem::~YQPkgPatternListItem()
{
    // NOP
}


void
YQPkgPatternListItem::cycleStatus()
{
    if ( ! _editable || ! _pkgObjList->editable() )
        return;

    ZyppStatus oldStatus = status();
    ZyppStatus newStatus = oldStatus;

    switch ( oldStatus )
    {
        case S_Install:
            newStatus = S_NoInst;
            break;

        case S_NoInst:
            newStatus = S_Install;
            break;

        case S_AutoInstall:
            newStatus =  S_NoInst;
            break;

        default:
            break;
    }

    if ( oldStatus != newStatus )
    {
        setStatus( newStatus );

        if ( showLicenseAgreement() )
        {
            showNotifyTexts( newStatus );
        }
        else // License not confirmed?
        {
            // Status is now S_Taboo or S_Del - update status icon
            setStatusIcon();
        }

        _patternList->sendStatusChanged();
    }
}


void
YQPkgPatternListItem::resetToolTip()
{
    std::string infoToolTip;
    infoToolTip +=  ("<p>" + zyppPattern()->description() + "</p>");

    if ( totalPackages() > 0 )
    {
        infoToolTip += ("<p>" + zypp::str::form("%d / %d", installedPackages(), totalPackages() ) + "</p>");
    }

    setToolTip(_patternList->summaryCol(), fromUTF8( infoToolTip ) );
}


void
YQPkgPatternListItem::applyChanges()
{
    solveResolvableCollections();
}


bool YQPkgPatternListItem::operator<( const QTreeWidgetItem & otherListViewItem ) const
{
    const YQPkgPatternListItem * otherPatternListitem =
        dynamic_cast<const YQPkgPatternListItem *>( &otherListViewItem );

    if ( _zyppPattern && otherPatternListitem && otherPatternListitem->zyppPattern() )
    {
        if ( _zyppPattern->order().empty() )
            return false;

        if ( otherPatternListitem->zyppPattern()->order().empty() )
            return true;

        if ( _zyppPattern->order() != otherPatternListitem->zyppPattern()->order() )
            return _zyppPattern->order() < otherPatternListitem->zyppPattern()->order();
        else
            return _zyppPattern->name() < otherPatternListitem->zyppPattern()->name();
    }

    const YQPkgPatternCategoryItem * otherCategoryItem =
        dynamic_cast<const YQPkgPatternCategoryItem *>( &otherListViewItem );

    if ( otherCategoryItem )    // Patterns without category should always be sorted
        return true;            // before any category

    return QTreeWidgetItem::operator<( otherListViewItem );
}


YQPkgPatternCategoryItem::YQPkgPatternCategoryItem( YQPkgPatternList * patternList,
                                                    const QString &    category    )
    : QY2ListViewItem( patternList )
    , _patternList( patternList )
    , _firstPattern( 0 )
{
    setText( _patternList->summaryCol(), category );

    setExpanded( true );
    setTreeIcon();

    QFont categoryFont = font( _patternList->summaryCol() );
    categoryFont.setBold(true);
    QFontMetrics fontMetrics( categoryFont );
    categoryFont.setPixelSize( (int) ( fontMetrics.height() * 1.1 ) );
    setFont( _patternList->summaryCol(), categoryFont );
}


YQPkgPatternCategoryItem::~YQPkgPatternCategoryItem()
{
    // NOP
}


void
YQPkgPatternCategoryItem::addPattern( ZyppPattern pattern )
{
    if ( ! _firstPattern )
    {
        _firstPattern = pattern;
    }
    else
    {
        if ( _firstPattern->order().empty() )
        {
            _firstPattern = pattern;
        }
        else if ( ! pattern->order().empty() &&
                  pattern->order() < _firstPattern->order() )
        {
            _firstPattern = pattern;
        }
    }

    if ( _firstPattern && _patternList->showOrderCol() )
        setText( _patternList->orderCol(), fromUTF8( _firstPattern->order() ) );
}


void
YQPkgPatternCategoryItem::setExpanded( bool open )
{
    QTreeWidgetItem::setExpanded( open );
    setTreeIcon();
}


void
YQPkgPatternCategoryItem::setTreeIcon()
{
    setIcon( 0,
             isExpanded() ?
             YQIconPool::arrowDown() :
             YQIconPool::arrowRight() );
}


bool
YQPkgPatternCategoryItem::operator<( const QTreeWidgetItem & otherListViewItem ) const
{
    const YQPkgPatternCategoryItem * otherCategoryItem =
        dynamic_cast<const YQPkgPatternCategoryItem *>( &otherListViewItem );

    if ( _firstPattern && otherCategoryItem && otherCategoryItem->firstPattern() )
        return _firstPattern->order() < otherCategoryItem->firstPattern()->order();


    const YQPkgPatternListItem * otherPatternListitem =
        dynamic_cast<const YQPkgPatternListItem *>( &otherListViewItem );

    if ( otherPatternListitem ) // Patterns without category should always be sorted
        return false;           // before any category

    return QTreeWidgetItem::operator<( otherListViewItem );
}
