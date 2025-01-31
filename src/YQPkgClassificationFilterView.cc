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


#include <QApplication>

#include <zypp/Package.h>
#include <zypp/PoolItem.h>
#include <zypp/ResStatus.h>
#include <zypp/ResTraits.h>
#include <zypp/Resolver.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>
#include <zypp/sat/Pool.h>
#include <zypp/ui/Selectable.h>

#include "Logger.h"
#include "YQi18n.h"
#include "YQPkgClassificationFilterView.h"

#ifndef VERBOSE_FILTER_VIEWS
#  define VERBOSE_FILTER_VIEWS  0
#endif

using std::string;


QString
translatedText( YQPkgClass pkgClass )
{
    switch ( pkgClass )
    {
	case YQPkgClassNone:               return _( "No Packages"           	    );
	case YQPkgClassRecommended:	   return _( "Recommended Packages"	    );
	case YQPkgClassSuggested:	   return _( "Suggested Packages"	    );
	case YQPkgClassOrphaned:	   return _( "Orphaned Packages"	    );
	case YQPkgClassUnneeded:	   return _( "Unneeded Packages"	    );
	case YQPkgClassMultiversion:	   return _( "Multiversion Packages"	    );
	case YQPkgClassRetracted:	   return _( "Retracted Packages"	    );
	case YQPkgClassRetractedInstalled: return _( "Retracted Installed Packages" );
	case YQPkgClassAll:		   return _( "All Packages"		    );

	// Intentionally omitting 'default' case so gcc can catch unhandled enums
    }

    // this should never be reached, not marked for translation
    return "Unknown PkgClass";
}


string
pkgClassIcon( YQPkgClass pkgClass )
{
    switch ( pkgClass )
    {
	case YQPkgClassAll:
	    return( "preferences-other" );

	default:
	    return( "preferences-desktop-locale" );
    }

    return( "" );
}


YQPkgClassificationFilterView::YQPkgClassificationFilterView( QWidget * parent )
    : QTreeWidget( parent )
{
    setIconSize( QSize( 32, 32 ) );
    setHeaderLabels( QStringList( _( "Package Classification" ) ) );
    setRootIsDecorated( false );
    setSortingEnabled( true );

    connect( this, SIGNAL( currentItemChanged	( QTreeWidgetItem *, QTreeWidgetItem * ) ),
	     this, SLOT	 ( slotSelectionChanged ( QTreeWidgetItem * ) ) );

    fillPkgClasses();
}


YQPkgClassificationFilterView::~YQPkgClassificationFilterView()
{
}


void
YQPkgClassificationFilterView::fillPkgClasses()
{
    new YQPkgClassItem( this, YQPkgClassRecommended	   );
    new YQPkgClassItem( this, YQPkgClassSuggested	   );
    new YQPkgClassItem( this, YQPkgClassOrphaned	   );
    new YQPkgClassItem( this, YQPkgClassUnneeded	   );
    new YQPkgClassItem( this, YQPkgClassRetracted	   );
    new YQPkgClassItem( this, YQPkgClassRetractedInstalled );

    if ( ! zypp::sat::Pool::instance().multiversion().empty() )
	new YQPkgClassItem( this, YQPkgClassMultiversion );

    new YQPkgClassItem( this, YQPkgClassAll );
}


void
YQPkgClassificationFilterView::showFilter( QWidget * newFilter )
{
    if ( newFilter == this )
        filter();
}


void
YQPkgClassificationFilterView::filter()
{
#if VERBOSE_FILTER_VIEWS
    logVerbose() << "Filtering" << Qt::endl;
#endif

    emit filterStart();

    if ( selectedPkgClass() != YQPkgClassNone )
    {
	for ( ZyppPoolIterator it = zyppPkgBegin();
	      it != zyppPkgEnd();
	      ++it )
	{
	    ZyppSel selectable = *it;
	    bool match = false;

	    // If there is an installed obj, check this first. The bits are set
	    // for the installed obj only and the installed obj is not
	    // contained in the pick list if there in an identical candidate
	    // available from a repo.

	    if ( selectable->installedObj() )
	    {
		match = check( selectable, tryCastToZyppPkg( selectable->installedObj() ) );
	    }
	    if ( selectable->candidateObj() && ! match )
	    {
		match = check( selectable, tryCastToZyppPkg( selectable->candidateObj() ) );
	    }

	    // And then check the pick list which contain all availables and
	    // all objects for multi version packages and the installed obj if
	    // there isn't same version in a repo.

	    if ( ! match )
	    {
		zypp::ui::Selectable::picklist_iterator it = selectable->picklistBegin();

		while ( it != selectable->picklistEnd() && ! match )
		{
		    check( selectable, tryCastToZyppPkg( *it ) );
		    ++it;
		}
	    }
	}
    }

    emit filterFinished();
}


void
YQPkgClassificationFilterView::slotSelectionChanged( QTreeWidgetItem * newSelection )
{
    YQPkgClassItem * sel = dynamic_cast<YQPkgClassItem *>( newSelection );

    if ( sel )
    {
        bool needSolverRun;

        switch ( sel->pkgClass() )
        {
            case YQPkgClassRecommended:
            case YQPkgClassSuggested:
            case YQPkgClassOrphaned:
            case YQPkgClassUnneeded:
                needSolverRun = true;
                break;

            default:
                needSolverRun = false;
                break;
        }

	if ( needSolverRun )
	{
	    QApplication::setOverrideCursor(Qt::WaitCursor);
	    zypp::getZYpp()->resolver()->resolvePool();
	    QApplication::restoreOverrideCursor();
	}
    }

    filter();
}


bool
YQPkgClassificationFilterView::check( ZyppSel selectable, ZyppPkg pkg )
{
    bool match = checkMatch( selectable, pkg );

    if ( match )
	emit filterMatch( selectable, pkg );

    return match;
}


bool
YQPkgClassificationFilterView::checkMatch( ZyppSel selectable, ZyppPkg pkg )
{
    if ( ! pkg )
	return false;

    switch ( selectedPkgClass() )
    {
        case YQPkgClassNone:                    return false;
	case YQPkgClassRecommended:		return zypp::PoolItem( pkg ).status().isRecommended();
	case YQPkgClassSuggested:		return zypp::PoolItem( pkg ).status().isSuggested();
	case YQPkgClassOrphaned:		return zypp::PoolItem( pkg ).status().isOrphaned();
	case YQPkgClassUnneeded:		return zypp::PoolItem( pkg ).status().isUnneeded();
	case YQPkgClassMultiversion:		return selectable->multiversionInstall();
	case YQPkgClassRetracted:		return selectable->hasRetracted();
	case YQPkgClassRetractedInstalled:	return selectable->hasRetractedInstalled();
	case YQPkgClassAll:			return true;

        // No 'default' branch to let the compiler catch unhandled enum values
    }

    return false;
}


YQPkgClass
YQPkgClassificationFilterView::selectedPkgClass() const
{
    QTreeWidgetItem * qItem = currentItem();

    if ( ! qItem )
	return YQPkgClassNone;

    YQPkgClassItem * pkgClassItem = dynamic_cast<YQPkgClassItem *> ( qItem );

    if ( ! pkgClassItem )
        return YQPkgClassNone;
    else
        return pkgClassItem->pkgClass();
}


void
YQPkgClassificationFilterView::showPkgClass( YQPkgClass pkgClass )
{
    QTreeWidgetItemIterator it( this );

    while ( *it )
    {
        YQPkgClassItem * item = dynamic_cast<YQPkgClassItem *>( *it );

        if ( item && item->pkgClass() == pkgClass )
        {
            setCurrentItem( item );
            // This will also send the currentItemChanged() signal which will
            // start filtering, i.e. it will populate the package list.
        }

        ++it;
    }
}




YQPkgClassItem::YQPkgClassItem( QTreeWidget * parentView,
				YQPkgClass    pkgClass )
    : QTreeWidgetItem( parentView )
    , _pkgClass( pkgClass )
{
    setText( 0, translatedText( pkgClass ) );
}


YQPkgClassItem::~YQPkgClassItem()
{
    // NOP
}


bool
YQPkgClassItem::operator< ( const QTreeWidgetItem & otherListViewItem ) const
{
    const YQPkgClassItem * otherCategoryItem =
	dynamic_cast<const YQPkgClassItem *>(&otherListViewItem);

    if ( otherCategoryItem )
	return pkgClass() > otherCategoryItem->pkgClass();
    else
	return true;
}
