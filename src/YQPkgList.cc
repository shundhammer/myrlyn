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


#include <QAction>
#include <QFontMetrics>
#include <QHeaderView>
#include <QMenu>

#include "Logger.h"
#include "QY2CursorHelper.h"
#include "YQi18n.h"
#include "utf8.h"

#include "YQPkgList.h"


#define SINGLE_VERSION_COL      1

#define STATUS_ICON_SIZE        16
#define STATUS_COL_WIDTH        28
#define MAGIC_MISSING_WIDTH     15


YQPkgList::YQPkgList( QWidget * parent )
    : YQPkgObjList( parent )
{
    resetBestColWidths();

    int numCol = 0;
    QStringList headers;
    QString     versionHeaderText;


    headers <<  "";                _statusCol  = numCol++;
    headers <<  _( "Package"    ); _nameCol    = numCol++;
    headers <<  _( "Summary"    ); _summaryCol = numCol++;

    if ( haveInstalledPkgs() )
    {
#if SINGLE_VERSION_COL
        versionHeaderText = _("Installed (Available)");
        headers << versionHeaderText;   _instVersionCol   = numCol++;
        _versionCol = _instVersionCol;
#else
        versionHeaderText = _( "Avail. Ver." );
        headers << versionHeaderText;     _versionCol     = numCol++;

        QString instVersionHeaderText = _( "Inst. Ver."  );
        headers << instVersionHeaderText; _instVersionCol = numCol++;
#endif
    }
    else
    {
        versionHeaderText = _( "Version" );
        headers << versionHeaderText; _versionCol  = numCol++;
    }

    headers <<  _( "Size" ); _sizeCol = numCol++;

    setHeaderLabels( headers );

    header()->setSortIndicatorShown( true );
    header()->setSectionsClickable( true );

    sortByColumn( statusCol(), Qt::AscendingOrder );
    setAllColumnsShowFocus( true );
    setIconSize( QSize( STATUS_ICON_SIZE, STATUS_ICON_SIZE ) );

    header()->setSectionResizeMode( QHeaderView::Interactive );

    /* NOTE: resizeEvent() is automatically triggered afterwards => sets initial column widths */

    connect ( header(), SIGNAL( sectionClicked (int) ),
              this,     SLOT( sortByColumn (int) ) );

}


YQPkgList::~YQPkgList()
{
    // NOP
}


void YQPkgList::addPkgItem( ZyppSel selectable,
                            ZyppPkg zyppPkg )
{
    addPkgItem( selectable, zyppPkg, false );
}


void YQPkgList::addPkgItemDimmed( ZyppSel selectable,
                                  ZyppPkg zyppPkg )
{
    addPkgItem( selectable, zyppPkg, true );
}


void
YQPkgList::addPkgItem( ZyppSel  selectable,
                       ZyppPkg  zyppPkg,
                       bool     dimmed )
{
    scrollToTop();
    scheduleDelayedItemsLayout();

    if ( ! selectable )
    {
        logError() << "NULL zypp::ui::Selectable!" << Qt::endl;
        return;
    }

    YQPkgListItem * item = new YQPkgListItem( this, selectable, zyppPkg );
    Q_CHECK_PTR( item );

    updateBestColWidths( selectable, zyppPkg );
    optimizeColumnWidths();

    item->setDimmed( dimmed );
    applyExcludeRules( item );
}


bool
YQPkgList::haveInstalledPkgs()
{
    for ( ZyppPoolIterator it = zyppPkgBegin();
          it != zyppPkgEnd();
          ++it )
    {
        if ( (*it)->installedObj() )
            return true;
    }

    return false;
}


QSize
YQPkgList::sizeHint() const
{
    return QSize( 600, 350 );
}


void
YQPkgList::resetBestColWidths()
{
    _bestStatusColWidth      = 0;
    _bestNameColWidth        = 0;
    _bestSummaryColWidth     = 0;
    _bestVersionColWidth     = 0;
    _bestInstVersionColWidth = 0;
    _bestSizeColWidth        = 0;
}


void
YQPkgList::updateBestColWidths( ZyppSel selectable,
                                ZyppPkg zyppPkg )
{
    QFontMetrics  fontMetrics( font() );
    QString       colText;
    int           colWidth  = 0;
    const ZyppObj candidate = selectable->candidateObj();
    const ZyppObj installed = selectable->installedObj();

    // Status icon

    _bestStatusColWidth = STATUS_COL_WIDTH;


    // Name

    colText  = fromUTF8( zyppPkg->name().c_str() );
    colWidth = fontMetrics.boundingRect( colText ).width() + ( STATUS_ICON_SIZE / 2 );

    if ( colWidth > _bestNameColWidth )
        _bestNameColWidth = colWidth;


    // Summary

    colText  = fromUTF8( zyppPkg->summary().c_str() );
    colWidth = fontMetrics.boundingRect( colText ).width() + ( STATUS_ICON_SIZE / 2 );

    if ( colWidth > _bestSummaryColWidth )
        _bestSummaryColWidth = colWidth;


    // Version(s)

    if ( instVersionCol() == versionCol() )     // combined column for both versions
    {
        if (installed)
            colText = fromUTF8( installed->edition().c_str() );
        else
            colText.clear();

        if ( candidate && ( ! installed || ( candidate->edition() != installed->edition() ) ) )
        {
            if (installed)
                colText += " ";
            colText += "(" + fromUTF8( candidate->edition().c_str() ) + ")";
        }

        colWidth = fontMetrics.boundingRect( colText ).width() + ( STATUS_ICON_SIZE / 2 );

        if (colWidth > _bestVersionColWidth)
            _bestVersionColWidth = colWidth;
    }
    else // separate columns for both versions
    {
        if ( candidate )
        {
            colText = fromUTF8( candidate->edition().c_str() );
            colWidth = fontMetrics.boundingRect( colText ).width() + ( STATUS_ICON_SIZE / 2 );

            if (colWidth > _bestVersionColWidth)
                _bestVersionColWidth = colWidth;
        }

        if ( installed )
        {
            colText = fromUTF8( installed->edition().c_str() );
            colWidth = fontMetrics.boundingRect( colText ).width() + ( STATUS_ICON_SIZE / 2 );

            if (colWidth > _bestInstVersionColWidth)
                _bestInstVersionColWidth = colWidth;
        }
    }


    // Size

    colText  = fromUTF8( zyppPkg->installSize().asString().c_str() );
    colWidth = fontMetrics.boundingRect( colText ).width() + ( STATUS_ICON_SIZE / 2 );

    if ( colWidth > _bestSizeColWidth )
        _bestSizeColWidth = colWidth;

    //
    // Regardless of all the above voodoo, set some reasonable min and max widths.
    //

    _bestNameColWidth    = qBound( 120, _bestNameColWidth,    280 );
    _bestSummaryColWidth = qBound( 350, _bestSummaryColWidth, 500 );

    if ( instVersionCol() == versionCol() )     // combined column for both versions
    {
        _bestVersionColWidth = qBound( 120, _bestVersionColWidth, 280 );
    }
    else // two columns
    {
        _bestVersionColWidth     = qBound( 120, _bestVersionColWidth,     200 );
        _bestInstVersionColWidth = qBound( 120, _bestInstVersionColWidth, 200 );
    }

    _bestSizeColWidth = qBound( 100, _bestSizeColWidth, 150 );
}


void
YQPkgList::optimizeColumnWidths()
{
    // FIXME: Refactor this. Same reason as above.

    int visibleSpace       = 0;
    int bestWidthsSum      = 0; 
    int colCount           = 4;  // Number of columns: name, summary, version, size
    int statusIconColWidth = _bestStatusColWidth;

    if ( statusIconColWidth == 0 )
        statusIconColWidth = STATUS_COL_WIDTH;

    bestWidthsSum =
        _bestStatusColWidth
        + _bestNameColWidth
        + _bestSummaryColWidth
        + _bestVersionColWidth
        + _bestSizeColWidth;

    if ( instVersionCol() != versionCol() ) // Separate version columns?
    {
        bestWidthsSum += _bestInstVersionColWidth;
        colCount++;
    }


    // Check if we have less visible space than we need

    visibleSpace = viewport()->width() - MAGIC_MISSING_WIDTH;

    if (visibleSpace < 0)
        return;

    if ( bestWidthsSum >= visibleSpace )
    // There is not enough visible space to show all cloumns with optimal widths
    {
        // Only reduce width of the "summary"-column beyond optimal width
        // if this is not enough, we will get a horizontal scroll bar

        int reducedSummaryWidth = visibleSpace - bestWidthsSum + _bestSummaryColWidth;
        reducedSummaryWidth = qMax( reducedSummaryWidth, 400 );

        // Set new column widths

        setColumnWidth( statusCol(),  statusIconColWidth       );
        setColumnWidth( nameCol(),    _bestNameColWidth    );
        setColumnWidth( summaryCol(), reducedSummaryWidth      );
        setColumnWidth( versionCol(), _bestVersionColWidth );

        if ( instVersionCol() != versionCol() )
            setColumnWidth( instVersionCol(), _bestInstVersionColWidth );

        setColumnWidth( sizeCol(), _bestSizeColWidth );
    }
    else // There is enough visible space
    {
        // Distribute remaining visible space to all columns (except the satusicon-column):
        // Calculate additional column widths:

        int addSpace  = ( visibleSpace - bestWidthsSum ) / colCount;
        int addSpaceR = ( visibleSpace - bestWidthsSum ) % colCount;

        // Set new column widths

        setColumnWidth( statusCol(),  statusIconColWidth              );
        setColumnWidth( nameCol(),    _bestNameColWidth    + addSpace );
        setColumnWidth( summaryCol(), _bestSummaryColWidth + addSpace );
        setColumnWidth( versionCol(), _bestVersionColWidth + addSpace );

        if ( instVersionCol() != versionCol() )
            setColumnWidth( instVersionCol(), _bestInstVersionColWidth + addSpace );

        setColumnWidth( sizeCol(), _bestSizeColWidth + addSpace + addSpaceR );
    }
}


void
YQPkgList::clear()
{
    YQPkgObjList::clear();
    resetBestColWidths();
    optimizeColumnWidths();
}


void
YQPkgList::resort()
{
    int col             = sortColumn();
    Qt::SortOrder order = header()->sortIndicatorOrder();
    sortByColumn( col, order );
}


void
YQPkgList::resizeEvent( QResizeEvent * event )
{
    if (event->size().width() != event->oldSize().width())
        optimizeColumnWidths();

    // NOTE: Avoid column width optimization when the size changes
    // because the horizontal scroll bar appeares/disappears

    event->accept();
}


void
YQPkgList::createNotInstalledContextMenu()
{
    _notInstalledContextMenu = new QMenu( this );
    Q_CHECK_PTR( _notInstalledContextMenu );

    _notInstalledContextMenu->addAction( actionSetCurrentInstall     );
    _notInstalledContextMenu->addAction( actionSetCurrentDontInstall );
    _notInstalledContextMenu->addAction( actionSetCurrentTaboo       );

    addAllInListSubMenu( _notInstalledContextMenu );
}


void
YQPkgList::createInstalledContextMenu()
{
    _installedContextMenu = new QMenu( this );
    Q_CHECK_PTR( _installedContextMenu );

    _installedContextMenu->addAction( actionSetCurrentKeepInstalled );
    _installedContextMenu->addAction( actionSetCurrentDelete        );
    _installedContextMenu->addAction( actionSetCurrentUpdate        );
    _installedContextMenu->addAction( actionSetCurrentUpdateForce   );
    _installedContextMenu->addAction( actionSetCurrentProtected     );

    addAllInListSubMenu( _installedContextMenu );
}


QMenu *
YQPkgList::addAllInListSubMenu( QMenu * menu )
{
    QMenu * submenu = new QMenu( menu );
    Q_CHECK_PTR( submenu );

    submenu->addAction( actionSetListInstall       );
    submenu->addAction( actionSetListDontInstall   );
    submenu->addAction( actionSetListKeepInstalled );
    submenu->addAction( actionSetListDelete        );
    submenu->addAction( actionSetListDelete        );
    submenu->addAction( actionSetListUpdate        );
    submenu->addAction( actionSetListUpdateForce   );
    submenu->addAction( actionSetListTaboo         );
    submenu->addAction( actionSetListProtected     );

    QAction *action = menu->addMenu( submenu );
    action->setText(_( "&All in This List" ));

    return submenu;
}


int
YQPkgList::globalSetPkgStatus( ZyppStatus newStatus, bool force, bool countOnly )
{
    busyCursor();
    int changedCount = 0;

    for ( ZyppPoolIterator it = zyppPkgBegin();
          it != zyppPkgEnd();
          ++it )
    {
        ZyppSel    selectable = *it;
        ZyppStatus oldStatus  = selectable->status();

        if ( newStatus != oldStatus )
        {
            bool doChange = false;

            switch ( newStatus )
            {
                case S_KeepInstalled:
                case S_Del:
                case S_AutoDel:
                case S_Protected:
                    doChange = !selectable->installedEmpty();
                    break;

                case S_Update:
                case S_AutoUpdate:

                    if ( force )
                    {
                        doChange = !selectable->installedEmpty();
                    }
                    else // don't force - update only if useful (if candidate is newer)
                    {
                        const ZyppObj candidate = selectable->candidateObj();
                        const ZyppObj installed = selectable->installedObj();

                        if ( candidate && installed )
                        {
                            doChange = ( installed->edition() < candidate->edition() );
                        }
                    }
                    break;

                case S_Install:
                case S_AutoInstall:
                case S_NoInst:
                case S_Taboo:
                    doChange = selectable->installedEmpty();
                    break;
            }

            if ( doChange )
            {
                if ( ! countOnly && oldStatus != S_Protected )
                    selectable->setStatus( newStatus );

                changedCount++;
                // logInfo() << "Updating " << selectable->name() << Qt::endl;
            }
        }
    }

    if ( changedCount > 0 && ! countOnly )
    {
        emit updateItemStates();
        emit updatePackages();
        emit statusChanged();
    }

    normalCursor();

    return changedCount;
}






YQPkgListItem::YQPkgListItem( YQPkgList * pkgList,
                              ZyppSel     selectable,
                              ZyppPkg     zyppPkg )
    : YQPkgObjListItem( pkgList,
                        selectable,
                        zyppPkg )
    , _pkgList( pkgList )
    , _zyppPkg( zyppPkg )
    , _dimmed( false )
{
    if ( ! _zyppPkg )
        _zyppPkg = tryCastToZyppPkg( selectable->theObj() );

    setTextAlignment( sizeCol(), Qt::AlignRight );
}


YQPkgListItem::~YQPkgListItem()
{
    // NOP
}


void
YQPkgListItem::updateData()
{
    YQPkgObjListItem::updateData();
}


QString
YQPkgListItem::toolTip( int col )
{
    QString text;
    QString name = _zyppObj->name().c_str();

    if ( col == statusCol() )
    {
        text = YQPkgObjListItem::toolTip( col );
    }
    else
    {
        text = name + "\n\n";

        QString installed;
        QString candidate;

        if ( ! selectable()->installedEmpty() )
        {
            installed  = selectable()->installedObj()->edition().asString().c_str();
            installed += "-";
            installed +=  selectable()->installedObj()->arch().asString().c_str();
            installed  = _( "Installed Version: %1" ).arg( installed );
        }

        if (  selectable()->hasCandidateObj() )
        {
            candidate  = selectable()->candidateObj()->edition().asString().c_str();
            candidate += "-";
            candidate +=  selectable()->candidateObj()->arch().asString().c_str();
        }

        if ( !selectable()->installedEmpty() )
        {
            text += installed + "\n";

            if ( selectable()->hasCandidateObj() )
            {
                // Translators: This is the relation between two versions of one package
                // if both versions are the same, e.g., both "1.2.3-42", "1.2.3-42"
                QString relation = _( "same" );

                if ( _candidateIsNewer ) relation = _( "newer" );
                if ( _installedIsNewer ) relation = _( "older" );

                // Translators: %1 is the version, %2 is one of "newer", "older", "same"
                text += _( "Available Version: %1 (%2)" ).arg( candidate ).arg( relation );
            }
            else
            {
                text += _( "Not available for installation" );
            }
        }
        else // not installed
        {
            text += candidate;
        }
    }

    return text;
}
