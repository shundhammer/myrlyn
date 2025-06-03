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


#ifndef YQPkgDependenciesView_h
#define YQPkgDependenciesView_h

#include "YQZypp.h"
#include "YQPkgGenericDetailsView.h"


typedef zypp::Capabilities ZyppCap;


/**
 * Display technical details ( very much like 'rpm -qi' ) for a zypp::Package
 * object - the installed instance, the candidate instance or both ( in two
 * columns ) if both exist. All other available instances are ignored.
 **/
class YQPkgDependenciesView : public YQPkgGenericDetailsView
{
    Q_OBJECT

public:

    /**
     * Constructor
     **/
    YQPkgDependenciesView( QWidget * parent );


    /**
     * Destructor
     **/
    virtual ~YQPkgDependenciesView();


protected:

    /**
     * Show details for the specified selectable:
     * In this case technical data, very much like "rpm -qi".
     *
     * Reimplemented from YQPkgGenericDetailsView.
     **/
    virtual void showDetails( ZyppSel selectable ) override;

    /**
     * Returns a string containing a HTML table for technical details for one
     * package.
     **/
    QString simpleTable( ZyppObj pkg );


    /**
     * Returns a string containing a HTML table for technical details for two
     * package instances: The installed instance and an alternate instance.
     * (usually the candidate instance).
     **/
    QString complexTable( ZyppObj installed,
			  ZyppObj candidate );

    /**
     * Format a zypp::CapSet (describing zypp::Dep::REQUIRES etc.)
     * with a heading in HTML lines.
     *
     * Returns an empty string if capSet is empty.
     **/
    static QString row( const QString &	heading,
			const ZyppCap & capSet );


    /**
     * Format two ZyppCap (describing zypp::Dep::REQUIRES etc.)
     * with a heading in HTML lines.
     *
     * Returns an empty string both capSets are empty.
     **/
    static QString row( const QString & heading,
			const ZyppCap & capSet1,
			const ZyppCap & capSet2 );

    /**
     * Returns a string containing a HTML table row with 'contents'.
     *
     * Required here to avoid inheritance ambiguities.
     **/
    static QString row( const QString & contents )
	{ return YQPkgGenericDetailsView::row( contents ); }

    /**
     * Format a ZyppCap (describing zypp::Dep::REQUIRES etc.)
     * in HTML lines, separated with <BR>.
     * Returns an empty string if capSet is empty.
     **/
    static QString htmlLines( const ZyppCap & capSet );
};


#endif // ifndef YQPkgDependenciesView_h
