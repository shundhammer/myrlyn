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


#include "YQPkgRpmGroupsFilterView.h"
#include "Exception.h"
#include "Logger.h"
#include "YQi18n.h"
#include "utf8.h"


YRpmGroupsTree * YQPkgRpmGroupsFilterView::_rpmGroupsTree    = 0;
int              YQPkgRpmGroupsFilterView::_unspecifiedCount = 0;


YQPkgRpmGroupsFilterView::YQPkgRpmGroupsFilterView( QWidget * parent )
    : QTreeWidget( parent )
    , _lazyTreeInitDone( false )
{
    setHeaderLabels( QStringList( _( "RPM Groups" ) ) );
    setRootIsDecorated( true );

    // The QTreeWidget's items are initialized on demand in lazyTreeInit()
    // called from showFilter().

    connect( this, SIGNAL( currentItemChanged   ( QTreeWidgetItem *, QTreeWidgetItem * ) ),
             this, SLOT  ( slotSelectionChanged ( QTreeWidgetItem *                    ) ) );

    selectSomething();
}


YQPkgRpmGroupsFilterView::~YQPkgRpmGroupsFilterView()
{
}


void
YQPkgRpmGroupsFilterView::lazyTreeInit()
{
    if ( _lazyTreeInitDone )
        return;

    cloneTree( rpmGroupsTree()->root(), 0 );
    _lazyTreeInitDone = true;
}


YRpmGroupsTree *
YQPkgRpmGroupsFilterView::rpmGroupsTree()
{
    if ( ! _rpmGroupsTree )
    {
        _rpmGroupsTree = new YRpmGroupsTree();
        CHECK_PTR( _rpmGroupsTree );

        fillRpmGroupsTree();
    }

    return _rpmGroupsTree;
}


void
YQPkgRpmGroupsFilterView::fillRpmGroupsTree()
{
    logDebug() << "Filling RPM groups tree" << endl;

    for ( ZyppPoolIterator it = zyppPkgBegin();
          it != zyppPkgEnd();
          ++it )
    {
        ZyppPkg zyppPkg = tryCastToZyppPkg( (*it)->theObj() );

        if ( zyppPkg )
        {
            std::string group = zyppPkg->group();

            if ( group.empty() || group == "Unspecified" )
            {
                group = "zzz Unspecified";
                _unspecifiedCount++;
            }

            rpmGroupsTree()->addRpmGroup( group );

        }
    }

    logDebug() << "Filling RPM groups tree done" << endl;
}


void
YQPkgRpmGroupsFilterView::cloneTree( YStringTreeItem *   parentRpmGroup,
                                     YQPkgRpmGroupItem * parentClone     )
{
    YStringTreeItem *   child = parentRpmGroup->firstChild();
    YQPkgRpmGroupItem *  clone;

    while ( child )
    {
        if ( parentClone )
            clone = new YQPkgRpmGroupItem( this, parentClone, child );
        else
            clone = new YQPkgRpmGroupItem( this, child );

        CHECK_PTR( clone );

        clone->setExpanded( clone->depth() < 1 );
        cloneTree( child, clone );
        child = child->next();
    }
}


void
YQPkgRpmGroupsFilterView::selectSomething()
{
    logDebug() << endl;
}


void
YQPkgRpmGroupsFilterView::showFilter( QWidget * newFilter )
{
    if ( newFilter == this )
    {
        lazyTreeInit();
        filter();
        selectSomething();
    }
}


void
YQPkgRpmGroupsFilterView::filter()
{
    emit filterStart();
    logDebug() << "Filtering packages for RPM group \"" << selectedRpmGroup() << "\"" << endl;

    if ( selection() )
    {
        for ( ZyppPoolIterator it = zyppPkgBegin();
              it != zyppPkgEnd();
              ++it )
        {
            ZyppSel selectable = *it;

            // Multiple instances of this package may or may not be in the same
            // RPM group, so let's check both the installed version (if there
            // is any) and the candidate version.
            //
            // Make sure we emit only one filterMatch() signal if both exist
            // and both are in the same RPM group. We don't want multiple list
            // entries for the same package!

            bool match =
                check( selectable, tryCastToZyppPkg( selectable->candidateObj() ) ) ||
                check( selectable, tryCastToZyppPkg( selectable->installedObj() ) );

            // If there is neither an installed nor a candidate package, check
            // any other instance.

            if ( ! match                        &&
                 ! selectable->candidateObj()   &&
                 ! selectable->installedObj()     )
                check( selectable, tryCastToZyppPkg( selectable->theObj() ) );
        }
    }

    emit filterFinished();
}


void
YQPkgRpmGroupsFilterView::slotSelectionChanged( QTreeWidgetItem * newSelection )
{
    YQPkgRpmGroupItem * sel = dynamic_cast<YQPkgRpmGroupItem *>( newSelection );

    if ( sel )
    {
        if ( sel->rpmGroup() )
            _selectedRpmGroup = rpmGroupsTree()->rpmGroup( sel->rpmGroup() );
        else
            _selectedRpmGroup = "";
    }

    if ( _selectedRpmGroup == "zzz Unspecified" )
        _selectedRpmGroup = "Unspecified";

    filter();
}


bool
YQPkgRpmGroupsFilterView::check( ZyppSel selectable,
                                 ZyppPkg pkg        )
{
    if ( ! pkg || ! selection() )
        return false;

    if ( selectedRpmGroup().empty() )
        return false;

    if ( pkg->group() == selectedRpmGroup() ||                  // full match?
         pkg->group().find( selectedRpmGroup() + "/" ) == 0 )   // starts with selected?
    {
        emit filterMatch( selectable, pkg );
        return true;
    }

    return false;
}


YQPkgRpmGroupItem *
YQPkgRpmGroupsFilterView::selection() const
{
    QTreeWidgetItem * item = currentItem();

    if ( ! item )
        return 0;

    return dynamic_cast<YQPkgRpmGroupItem *> ( item );
}






YQPkgRpmGroupItem::YQPkgRpmGroupItem( YQPkgRpmGroupsFilterView * parentFilterView,
                                      YStringTreeItem *          rpmGroup         )
    : QTreeWidgetItem( parentFilterView )
    , _filterView( parentFilterView )
    , _rpmGroup( rpmGroup )
    , _depth( 0 )
{
    if ( _rpmGroup->value().orig() == "zzz Unspecified" )
    {
        setText( 0, _( "zzz Unspecified (%1)" )
                 .arg( YQPkgRpmGroupsFilterView::unspecifiedCount() ) );
    }
    else
    {
        setText( 0,  fromUTF8( _rpmGroup->value().translation() ) );
    }
}


YQPkgRpmGroupItem::YQPkgRpmGroupItem( YQPkgRpmGroupsFilterView * parentFilterView,
                                      YQPkgRpmGroupItem *        parentItem,
                                      YStringTreeItem *          rpmGroup        )
    : QTreeWidgetItem( parentItem )
    , _filterView( parentFilterView )
    , _rpmGroup( rpmGroup )
    , _depth( parentItem->depth() + 1 )
{
    setText( 0,  fromUTF8( _rpmGroup->value().translation() )  );
}


YQPkgRpmGroupItem::YQPkgRpmGroupItem( YQPkgRpmGroupsFilterView * parentFilterView,
                                      const QString &            rpmGroupName,
                                      YStringTreeItem *          rpmGroup        )
    : QTreeWidgetItem( parentFilterView )
    , _filterView( parentFilterView )
    , _rpmGroup( rpmGroup )
    , _depth(0)
{
    setText( 0,  rpmGroupName );
}


YQPkgRpmGroupItem::~YQPkgRpmGroupItem()
{
    // NOP
}
