/*  ---------------------------------------------------------
               __  __            _
              |  \/  |_   _ _ __| |_   _ _ __
              | |\/| | | | | '__| | | | | '_ \
              | |  | | |_| | |  | | |_| | | | |
              |_|  |_|\__, |_|  |_|\__, |_| |_|
                      |___/        |___/
    ---------------------------------------------------------

    Project:  Myrlyn Package Manager GUI
    Copyright (c) Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
    License:  GPL V2 - See file LICENSE for details.

 */



#include <QStringList>
#include <QHeaderView>

#include <zypp/RepoManager.h>

#include "Exception.h"
#include "Logger.h"
#include "MyrlynApp.h"
#include "YQIconPool.h"
#include "YQi18n.h"
#include "utf8.h"
#include "RepoTable.h"


RepoTable::RepoTable( QWidget * parent )
    : QY2ListView( parent )
    ,_repoManager( MyrlynApp::instance()->repoManager()->repoManager() )
{
    // logDebug() << "Creating RepoTable" << endl;

    QStringList headers;
    headers << _( "Name"         )
            << _( "Priority"     )
            << _( "Enabled"      )
            << _( "Auto-Refresh" )
            << _( "Service"      )
            << _( "URL"          );

    setHeaderLabels( headers );
    setIndentation( 0 );
    header()->setSectionsClickable( true );
    header()->setSortIndicatorShown( true );
    setSortingEnabled( true );
    sortByColumn( PrioCol, Qt::AscendingOrder );

    for ( int col=0; col < headers.size(); col++ )
        header()->setSectionResizeMode( col, QHeaderView::ResizeToContents );
}


RepoTable::~RepoTable()
{
    // logDebug() << "Destroying RepoTable" << endl;
}


void RepoTable::setHeaderItem( QTreeWidgetItem * headerItem )
{
    // Qt designer dumps code for this into the generated .ui file,
    // and this will generate columns that we don't want here.
    // Just ignore it and get rid of that thing.
    //
    // Since this method is documented to take ownership of the item,
    // we need to delete it here.

    if ( headerItem )
        delete headerItem;
}


void RepoTable::populate()
{
    for ( zypp::RepoManager::RepoConstIterator it = _repoManager->repoBegin();
          it != _repoManager->repoEnd();
          ++it )
    {
        const ZyppRepoInfo & repoInfo = *it;

        RepoTableItem * item = new RepoTableItem( this, repoInfo );
        CHECK_NEW( item );
    }
}


RepoTableItem *
RepoTable::currentRepoItem()
{
    QTreeWidgetItem * item = currentItem();

    return item ? dynamic_cast<RepoTableItem *>( item ) : 0;
}




RepoTableItem::RepoTableItem( RepoTable *          parentTable,
                              const ZyppRepoInfo & repoInfo    )
    : QY2ListViewItem( parentTable )
    , _parentTable( parentTable )
    , _repoInfo( repoInfo )
{
    setTextAlignment( RepoTable::PrioCol, Qt::AlignRight );

    updateData();
}


RepoTableItem::~RepoTableItem()
{
    // NOP
}


void RepoTableItem::setRepoInfo( const ZyppRepoInfo & newRepoInfo )
{
    _repoInfo = newRepoInfo;

    if ( MyrlynApp::runningAsRealRoot() )
        _parentTable->repoManager()->modifyRepository( _repoInfo );
    else
    {
        logWarning() << "Faking setting the repoInfo for "
                     << _repoInfo.name() << endl;
    }

    updateData();
}


void RepoTableItem::updateData()
{
    setText( RepoTable::NameCol,    _repoInfo.name() );
    setText( RepoTable::PrioCol,    std::to_string( _repoInfo.priority() ) + "       " );
    setIcon( RepoTable::EnabledCol, _repoInfo.enabled()     ? checkmarkIcon() : noIcon() );
    setIcon( RepoTable::AutoRefCol, _repoInfo.autorefresh() ? checkmarkIcon() : noIcon() );
    setText( RepoTable::ServiceCol, _repoInfo.service() );
    setText( RepoTable::UrlCol,     _repoInfo.url().asString() );
}


void RepoTableItem::setText( int col, const std::string & txt )
{
    setText( col, fromUTF8( txt ) );
}


void RepoTableItem::setText( int col, const QString & txt )
{
    // This really shouldn't be necessary, but gcc already cost me a full hour
    // of lifetime and screaming at the screen and half a week of life energy
    // with this bullshit: It just wouldn't let me use the publicly inherited
    // method directly. So let's call the inherited method explicitly. WTF?!

    QY2ListViewItem::setText( col, txt );
}


QPixmap
RepoTableItem::checkmarkIcon()
{
    return YQIconPool::checkmark();
}


QPixmap
RepoTableItem::noIcon()
{
    return QPixmap();
}


bool
RepoTableItem::operator< ( const QTreeWidgetItem & rawOther ) const
{
    if ( _parentTable )
    {
        const RepoTableItem & other = dynamic_cast<const RepoTableItem &>( rawOther );

        switch ( _parentTable->sortColumn() )
        {
            case RepoTable::PrioCol:    return repoInfo().priority()     < other.repoInfo().priority();
            case RepoTable::EnabledCol: return repoInfo().enabled()      < other.repoInfo().enabled();
            case RepoTable::AutoRefCol: return repoInfo().autorefresh()  < other.repoInfo().autorefresh();
        }
    }

    return QY2ListViewItem::operator<( rawOther );
}
