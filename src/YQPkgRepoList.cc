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


#include <QHeaderView>
#include <QTreeWidget>

#include <zypp/RepoManager.h>
#include <zypp/PoolQuery.h>

#include "Logger.h"
#include "QY2CursorHelper.h"
#include "QY2IconLoader.h"
#include "YQPkgFilters.h"
#include "YQi18n.h"
#include "utf8.h"

#include "YQPkgRepoList.h"


using std::string;


YQPkgRepoList::YQPkgRepoList( QWidget * parent )
    : QY2ListView( parent )
{
    // logVerbose() << "Creating repository list" << Qt::endl;

    _nameCol = -1;

    int numCol = 0;

    QStringList headers;
    headers << _( "Name"); _nameCol = numCol++;
    setHeaderLabels( headers );

    header()->setSectionResizeMode( _nameCol, QHeaderView::Stretch );


    // Allow multi-selection with Ctrl-mouse
    setSelectionMode( QAbstractItemView::ExtendedSelection );

    connect( this, SIGNAL( itemSelectionChanged() ),
	     this, SLOT  ( filterIfVisible()      ) );

    setIconSize( QSize( 32, 32 ) );

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

    // logVerbose() << "Creating repository list done" << Qt::endl;
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


void YQPkgRepoList::filterIfVisible()
{
    if ( isVisible() )
	filter();
}


void YQPkgRepoList::filter()
{
    if ( ! selection() )
        return;

    emit filterStart();


    //
    // Collect all packages of this repository
    //

    QTreeWidgetItem * item;

    QList<QTreeWidgetItem *>         items = selectedItems();
    QListIterator<QTreeWidgetItem *> it( items );

    while ( it.hasNext() )
    {
        item = it.next();
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
    if ( nameCol() >= 0 )
    {
        string name = repo.info().name();
        if ( ! name.empty() )
        {
            setText( nameCol(), fromUTF8( name ));
        }
    }

    string infoToolTip;
    infoToolTip += ("<b>" + repo.info().name() + "</b>");

    ZyppProduct product = singleProduct( _zyppRepo );

    if ( product )
        infoToolTip += ("<p>" + product->summary() + "</p>");

    if ( ! repo.info().baseUrlsEmpty() )
    {
        infoToolTip += "<ul>";
        zypp::RepoInfo::urls_const_iterator it;

        for ( it = repo.info().baseUrlsBegin();
              it != repo.info().baseUrlsEnd();
              ++it )
        {
            infoToolTip += ("<li>" + (*it).asString() + "</li>");
        }

        infoToolTip += "</ul>";
    }

    setToolTip( nameCol(), fromUTF8( infoToolTip ) );

    QString iconName = "applications-internet";

    if ( ! repo.info().baseUrlsEmpty() )
    {
        zypp::Url zyppRepoUrl = *repo.info().baseUrlsBegin();
        QString repoUrl       = zyppRepoUrl.asString().c_str();

        if      ( repoUrl.contains( "KDE"    ) )  iconName = "kde";
        else if ( repoUrl.contains( "GNOME"  ) )  iconName = "gnome";
        else if ( repoUrl.contains( "update" ) )  iconName = "applications-utilities";
        else if ( repoUrl.contains( "home:"  ) )  iconName = "preferences-desktop";
    }

    if ( repo.isSystemRepo() )
        iconName = "preferences-system";

    // The icon is always in color 0, no matter if that's nameCol() or whatever.

    setIcon( 0, QY2IconLoader::loadIcon( iconName ) );
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

