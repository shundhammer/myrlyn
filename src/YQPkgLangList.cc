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
#include <zypp/sat/LocaleSupport.h>

#include "Logger.h"
#include "QY2ListView.h"
#include "YQi18n.h"
#include "YQPkgLangList.h"

#ifndef VERBOSE_FILTER_VIEWS
#  define VERBOSE_FILTER_VIEWS  0
#endif


YQPkgLangList::YQPkgLangList( QWidget * parent )
    : YQPkgObjList( parent )
{
    // FIXME: The base class works with zypp::Resolvable, but zypp::Locale
    // isn't one any longer!

    logVerbose() << "Creating language list" << Qt::endl;

    // Translators: Table column heading for a language ISO code like "de_DE",
    // "en_US". Please keep this short to avoid stretching the column too wide!
    QString codeHeader = _( "Code" );

    // Full (human readable) language / country name like "German (Austria)"
    QString langheader = _( "Language");

    int numCol = 0;
    QStringList headers;
    headers <<  "";         _statusCol  = numCol++;
    headers << codeHeader;  _nameCol    = numCol++;
    headers << langheader;  _summaryCol = numCol++;

    setHeaderLabels( headers );

    header()->setSectionResizeMode( _nameCol,    QHeaderView::ResizeToContents );
    header()->setSectionResizeMode( _summaryCol, QHeaderView::Stretch );

    setAllColumnsShowFocus( true );
    header()->setSortIndicatorShown( true );
    header()->setSectionsClickable( true );

    setSortingEnabled( true );
    sortByColumn( nameCol(), Qt::AscendingOrder ); // Column "Code"

    connect( this, SIGNAL( currentItemChanged( QTreeWidgetItem *,
                                               QTreeWidgetItem * ) ),
             this, SLOT  ( filter() ) );

    fillList();
    selectSomething();
    resizeColumnToContents(_statusCol);

    logVerbose() << "Creating language list done" << Qt::endl;
}


YQPkgLangList::~YQPkgLangList()
{
    // NOP
}


void
YQPkgLangList::fillList()
{
    clear();
    // logVerbose() << "Filling language list" << Qt::endl;

    zypp::LocaleSet locales = zypp::getZYpp()->pool().getAvailableLocales();

    for ( zypp::LocaleSet::const_iterator it = locales.begin();
          it != locales.end();
          ++it )
    {
        addLangItem( *it );
    }

    // logVerbose() << "Language list filled" << Qt::endl;
}


void
YQPkgLangList::showFilter( QWidget * newFilter )
{
    if ( newFilter == this )
        filter();
}


void
YQPkgLangList::filter()
{
#if VERBOSE_FILTER_VIEWS
    logVerbose() << "Filtering" << Qt::endl;
#endif

    emit filterStart();

    if ( selection() )
    {
        zypp::Locale lang = selection()->zyppLang();

        zypp::sat::LocaleSupport currentLocale( lang );
        for_( it, currentLocale.selectableBegin(), currentLocale.selectableEnd() )
        {
            ZyppPkg zyppPkg = tryCastToZyppPkg( (*it)->theObj() );

            if ( zyppPkg )
            {
                emit filterMatch( *it, zyppPkg );
            }
        }
    }
    emit filterFinished();
}


void
YQPkgLangList::addLangItem( const zypp::Locale & zyppLang )
{
    new YQPkgLangListItem( this, zyppLang );
}


YQPkgLangListItem *
YQPkgLangList::selection() const
{
    QTreeWidgetItem * item = currentItem();

    return item ? dynamic_cast<YQPkgLangListItem *> (item) : 0;
}


void
YQPkgLangList::updateActions( YQPkgObjListItem * item)
{
    actionSetCurrentInstall->setEnabled( true );
    actionSetCurrentDontInstall->setEnabled( true );
    actionSetCurrentTaboo->setEnabled( true );
    actionSetCurrentProtected->setEnabled( false );

    actionSetCurrentKeepInstalled->setEnabled( false );
    actionSetCurrentDelete->setEnabled( false );
    actionSetCurrentUpdate->setEnabled( false );
}




YQPkgLangListItem::YQPkgLangListItem( YQPkgLangList *      langList,
                                      const zypp::Locale & lang )
    : YQPkgObjListItem( langList )
    , _zyppLang( lang )
{
    init();
}


YQPkgLangListItem::~YQPkgLangListItem()
{
    // NOP
}


void
YQPkgLangListItem::applyChanges()
{
    solveResolvableCollections();
}


void
YQPkgLangListItem::init()
{
    _candidateIsNewer = false;
    _installedIsNewer = false;

    if ( nameCol()    >= 0 ) setText( nameCol(),    _zyppLang.code() );
    if ( summaryCol() >= 0 ) setText( summaryCol(), _zyppLang.name() );

    setStatusIcon();
}


ZyppStatus
YQPkgLangListItem::status() const
{
    if ( zypp::getZYpp()->pool().isRequestedLocale( _zyppLang ) )
        return S_Install;
    else
        return S_NoInst;
}


void
YQPkgLangListItem::setStatus( ZyppStatus newStatus, bool sendSignals )
{
    ZyppStatus oldStatus = status();

    switch ( newStatus )
    {
        case S_Install:

            if ( ! zypp::getZYpp()->pool().isRequestedLocale( _zyppLang ) )
            {
                zypp::getZYpp()->pool().addRequestedLocale( _zyppLang );
            }
            break;


        case S_NoInst:

            if ( zypp::getZYpp()->pool().isRequestedLocale( _zyppLang ) )
            {
                zypp::getZYpp()->pool().eraseRequestedLocale( _zyppLang );
            }
            break;

        default:
            return;
    }

    if ( oldStatus != newStatus )
    {
        applyChanges();

        if ( sendSignals )
        {
            _pkgObjList->updateItemStates();
            _pkgObjList->sendUpdatePackages();
        }
    }

    setStatusIcon();
    _pkgObjList->sendStatusChanged();
}


bool
YQPkgLangListItem::bySelection() const
{
    return zypp::getZYpp()->pool().isRequestedLocale( _zyppLang );
}


void
YQPkgLangListItem::cycleStatus()
{
    if ( zypp::getZYpp()->pool().isRequestedLocale( _zyppLang ) )
    {
        zypp::getZYpp()->pool().eraseRequestedLocale( _zyppLang );
    }
    else
    {
        zypp::getZYpp()->pool().addRequestedLocale( _zyppLang );
    }

    setStatusIcon();
    _pkgObjList->sendStatusChanged();
}

bool YQPkgLangListItem::operator<( const QTreeWidgetItem & otherListViewItem ) const
{
    const YQPkgLangListItem * other =
        dynamic_cast<const YQPkgLangListItem *> (&otherListViewItem);

    int col = treeWidget()->sortColumn();

    if ( other )
    {
        if ( col == nameCol() )
        {
            return ( strcoll( this->zyppLang().code().c_str(),
                              other->zyppLang().code().c_str() ) < 0 );
        }
        if ( col == summaryCol() )
        {
            return ( strcoll( this->zyppLang().name().c_str(),
                              other->zyppLang().name().c_str() ) < 0 );
        }
    }

    return QY2ListViewItem::operator<( otherListViewItem );
}

