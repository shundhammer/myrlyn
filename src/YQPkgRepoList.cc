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


#include <QHeaderView>
#include <QTreeWidget>

#include <zypp/RepoManager.h>
#include <zypp/PoolQuery.h>

#include "Logger.h"
#include "QY2IconLoader.h"
#include "YQPkgFilters.h"
#include "YQi18n.h"
#include "utf8.h"

#include "YQPkgRepoList.h"


using std::string;


// This is the simple version used in YQPkgRepoFilterView, not to confuse with
// the more complex RepoTable in the RepoEditor.

YQPkgRepoList::YQPkgRepoList( QWidget * parent )
    : QY2ListView( parent )
{
    // logVerbose() << "Creating repository list" << endl;

    _nameCol = -1;

    int numCol = 0;

    QStringList headers;
    headers << _( "Name"); _nameCol = numCol++;
    setHeaderLabels( headers );

    header()->setSectionResizeMode( _nameCol, QHeaderView::Stretch );


    // Allow multi-selection with Ctrl-mouse
    setSelectionMode( QAbstractItemView::ExtendedSelection );

    connect( this, SIGNAL( itemSelectionChanged() ),
	     this, SLOT  ( filter()               ) );

    fillList();
    setSortingEnabled( true );
    sortByColumn( nameCol(), Qt::AscendingOrder );

#if 0
    // This is counterproductive because it will always select the first repo
    // in the list which is always '@System', the installed system and all its
    // packages, which is always HUGE (~ 3000 packages on a moderate TW
    // installation), so it will always take a long time; even if the user just
    // switched to the repo view to select a DIFFERENT repo.
    //
    // So, in this case, let's simply NOT select something, but wait for the
    // user instead to click on a repo. If that is still '@System', so be it,
    // so there is waiting time until that list is filled with ~3000 packages;
    // but then the user did it intentionally.
    selectSomething();
#endif

    // logVerbose() << "Creating repository list done" << endl;
}


YQPkgRepoList::~YQPkgRepoList()
{
    // NOP
}


void YQPkgRepoList::fillList()
{
    clear();

    for ( ZyppRepositoryIterator it = ZyppRepositoriesBegin();
	  it != ZyppRepositoriesEnd();
	  ++it )
    {
	addRepo( *it );
    }
}


int YQPkgRepoList::countEnabledRepositories()
{
    return zyppPool().knownRepositoriesSize();
}


void YQPkgRepoList::filter()
{
    emit filterStart();

    if ( ! selection() )
    {
        emit filterFinished();
        return;
    }


    //
    // Collect all packages of this repository
    //

    QList<QTreeWidgetItem *> items = selectedItems();
    QListIterator<QTreeWidgetItem *> it( items );

    while ( it.hasNext() )
    {
        QTreeWidgetItem *   item     = it.next();
        YQPkgRepoListItem * repoItem = dynamic_cast<YQPkgRepoListItem *>( item );

        if ( repoItem )
        {
            ZyppRepo currentRepo = repoItem->zyppRepo();

	    zypp::PoolQuery query;
	    query.addRepo( currentRepo.info().alias() );
	    query.addKind( zypp::ResKind::package );

    	    for( zypp::PoolQuery::Selectable_iterator it = query.selectableBegin();
	         it != query.selectableEnd();
                 ++it )
    	    {
		emit filterMatch( *it, tryCastToZyppPkg( (*it)->theObj() ) );
    	    }
	}
    }

    emit filterFinished();
}


void YQPkgRepoList::addRepo( ZyppRepo repo )
{
    new YQPkgRepoListItem( this, repo );
}


YQPkgRepoListItem *
YQPkgRepoList::selection() const
{
    QTreeWidgetItem * item = currentItem();

    if ( ! item )
	return 0;

    return dynamic_cast<YQPkgRepoListItem *> (item);
}




YQPkgRepoListItem::YQPkgRepoListItem( YQPkgRepoList * repoList,
				      ZyppRepo        repo     )
    : QY2ListViewItem( repoList )
    , _repoList( repoList )
    , _zyppRepo( repo )
{
    const ZyppRepoInfo & repoInfo = repo.info();

    if ( nameCol() >= 0 )
    {
        string name = repoInfo.name();

        if ( ! name.empty() )
            setText( nameCol(), fromUTF8( name ) );
    }

    string infoToolTip;
    infoToolTip += ("<b>" + repoInfo.name() + "</b>");

    ZyppProduct product = singleProduct( _zyppRepo );

    if ( product )
        infoToolTip += ("<p>" + product->summary() + "</p>");

    if ( ! repoInfo.baseUrlsEmpty() )
        infoToolTip += ( "<p>" + repoInfo.url().asString() + "</p>" );

    setToolTip( nameCol(), fromUTF8( infoToolTip ) );
}


YQPkgRepoListItem::~YQPkgRepoListItem()
{
    // NOP
}


ZyppProduct
YQPkgRepoListItem::singleProduct( ZyppRepo zyppRepo )
{
    return YQPkgFilters::singleProductFilter( [&](const zypp::PoolItem & item )
        {
            // filter the products from the requested repository
            return item.resolvable()->repoInfo().alias() == zyppRepo.info().alias();
        } );
}


bool YQPkgRepoListItem::operator<( const QTreeWidgetItem & other ) const
{
    const YQPkgRepoListItem * otherItem = dynamic_cast<const YQPkgRepoListItem *>(&other);

    return zyppRepo().info().name() < otherItem->zyppRepo().info().name();
}

