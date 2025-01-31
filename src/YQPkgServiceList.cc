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


#include <algorithm>
#include <set>
#include <string>

#include <QHeaderView>
#include <QString>
#include <QTreeWidget>

#include <zypp/PoolQuery.h>
#include <zypp/RepoManager.h>
#include <zypp/ServiceInfo.h>

#include "Logger.h"
#include "QY2IconLoader.h"
#include "YQPkgFilters.h"
#include "YQi18n.h"
#include "utf8.h"
#include "YQPkgServiceList.h"


using std::string;


YQPkgServiceList::YQPkgServiceList( QWidget * parent )
    : QY2ListView( parent )
{
    logDebug() << "Creating service list" << Qt::endl;

    QStringList headers;
    headers <<  _("Name");
    _nameCol = 0;

    setHeaderLabels( headers );
    header()->setSectionResizeMode( _nameCol, QHeaderView::Stretch );

    // Allow multi-selection with Ctrl-mouse
    setSelectionMode( QAbstractItemView::ExtendedSelection );

    connect( this, SIGNAL( itemSelectionChanged() ),
             this, SLOT  ( filter()               ) );

    setIconSize( QSize( 32, 32) );
    fillList();
    setSortingEnabled( true );
    sortByColumn( nameCol(), Qt::AscendingOrder );
    selectSomething();

    logDebug() << "Creating service list done" << Qt::endl;
}


YQPkgServiceList::~YQPkgServiceList()
{
    // NOP
}


void
YQPkgServiceList::fillList()
{
    clear();
    logDebug() << "Filling service list" << Qt::endl;

    std::set<std::string> added_services;
    zypp::RepoManager repo_manager;

    // Yikes - what an ugly piece of code.
    // a lambda inside a lambda, seriously?

    std::for_each( ZyppRepositoriesBegin(),
                   ZyppRepositoriesEnd(),
                   [&](const zypp::Repository& repo)
                       {
                           const std::string &service_name(repo.info().service());

                           if ( ! service_name.empty() )
                           {
                               bool found = std::any_of( added_services.begin(),
                                                         added_services.end(),
                                                         [&](const std::string& name)
                                                             {
                                                                 return service_name == name;
                                                             });

                               if ( ! found )
                               {
                                   addService( service_name, repo_manager );
                                   added_services.insert( service_name );
                               }
                           }
                       }
                   );

    logDebug() << "Service list filled" << Qt::endl;
}


void
YQPkgServiceList::filter()
{
    emit filterStart();

    // logInfo() << "Collecting packages in selected services..." << Qt::endl;

    //
    // Collect all packages from repositories belonging to this service
    //

    QList<QTreeWidgetItem *> items = selectedItems();
    QListIterator<QTreeWidgetItem *> it(items);

    while ( it.hasNext() )
    {
        QTreeWidgetItem * item = it.next();
        YQPkgServiceListItem * serviceItem = dynamic_cast<YQPkgServiceListItem *> (item);

        if ( serviceItem )
        {
            // logVerbose() << "Selected service: " << serviceItem->zyppService() << Qt::endl;

            zypp::PoolQuery query;

            std::for_each( ZyppRepositoriesBegin(),
                           ZyppRepositoriesEnd(),
                           [&](const zypp::Repository& repo)
                               {
                                   if (serviceItem->zyppService() == repo.info().service())
                                   {
                                       // logVerbose() << "Adding repo filter: " << repo.info().alias() << Qt::endl;
                                       query.addRepo( repo.info().alias() );
                                   }
                               }
                           );

            query.addKind( zypp::ResKind::package );

            std::for_each( query.selectableBegin(),
                           query.selectableEnd(),
                           [&](const zypp::ui::Selectable::Ptr &selectable)
                               {
                                   emit filterMatch( selectable, tryCastToZyppPkg( selectable->theObj() ) );
                               }
                           );
        }
    }

    emit filterFinished();
}


void
YQPkgServiceList::addService( ZyppService               service,
                              const zypp::RepoManager & repoManager)
{
    new YQPkgServiceListItem( this, service, repoManager );
}


YQPkgServiceListItem *
YQPkgServiceList::selection() const
{
    QTreeWidgetItem * item = currentItem();
    return item ? dynamic_cast<YQPkgServiceListItem *> (item) : 0;
}




YQPkgServiceListItem::YQPkgServiceListItem( YQPkgServiceList *        parentList,
                                            ZyppService               service,
                                            const zypp::RepoManager & repoManager )
    : QY2ListViewItem( parentList )
    , _serviceList( parentList )
    , _zyppService( service )
{

    zypp::ServiceInfo serviceInfo = repoManager.getService( service );
    _zyppServiceName = serviceInfo.name();
    QString service_name(fromUTF8(_zyppServiceName));

    if ( nameCol() >= 0 && ! service.empty() )
    {
        setText( nameCol(), service_name );
    }

    QString infoToolTip( "<p><b>" + service_name.toHtmlEscaped() + "</b></p>" );
    infoToolTip += "<p><b>" + _("URL: ") + "</b>"
        + fromUTF8( serviceInfo.url().asString() ).toHtmlEscaped() + "</p>";

    ZyppProduct product = singleProduct( _zyppService );

    if ( product )
    {
        infoToolTip += ( "<p><b>" + _("Product: ") + "</b>"
                        + fromUTF8(product->summary()).toHtmlEscaped() +  "</p>" );
    }

    infoToolTip += "<p><b>" + _( "Repositories:" ) + "</b><ul>";

    std::for_each( ZyppRepositoriesBegin(),
                   ZyppRepositoriesEnd(),
                   [&](const zypp::Repository& repo)
                       {
                           if (service == repo.info().service())
                               infoToolTip += "<li>" + fromUTF8(repo.name()).toHtmlEscaped() + "</li>";
                       }
                   );

    infoToolTip += "</ul></p>";
    setToolTip( nameCol(), infoToolTip);

    setIcon( 0, QY2IconLoader::loadIcon( "yast-update" ) );
}


YQPkgServiceListItem::~YQPkgServiceListItem()
{
    // NOP
}


ZyppProduct
YQPkgServiceListItem::singleProduct( ZyppService zyppService )
{
    return YQPkgFilters::singleProductFilter( [&](const zypp::PoolItem & item)
        {
            // filter the products from the requested service
            return item.resolvable()->repoInfo().service() == zyppService;
        });
}


bool
YQPkgServiceListItem::operator< ( const QTreeWidgetItem & other ) const
{
    const YQPkgServiceListItem * otherItem = dynamic_cast<const YQPkgServiceListItem *>(&other);

    // case insensitive compare
    return QString::compare( fromUTF8(zyppServiceName()),
                             fromUTF8(otherItem->zyppServiceName() ),
                             Qt::CaseInsensitive) < 0;
}

