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


#ifndef YQPkgTechnicalDetailsView_h
#define YQPkgTechnicalDetailsView_h

#include "YQPkgGenericDetailsView.h"
#include "YQZypp.h"


/**
 * Display technical details (very much like 'rpm -qi') for a ZYPP
 * object - the installed instance, the candidate instance or both (in two
 * columns) if both exist. All other available instances are ignored.
 **/
class YQPkgTechnicalDetailsView : public YQPkgGenericDetailsView
{
    Q_OBJECT

public:

    /**
     * Constructor
     **/
    YQPkgTechnicalDetailsView( QWidget * parent );


    /**
     * Destructor
     **/
    virtual ~YQPkgTechnicalDetailsView();


protected:

    /**
     * Show details for the specified zypp::ResObject:
     * In this case technical data, very much like "rpm -qi".
     *
     * Reimplemented from YQPkgGenericDetailsView.
     **/
    virtual void showDetails( ZyppSel selectable ) override;

    /**
     * Returns a string containing a HTML table for technical details for one
     * package.
     **/
    QString simpleTable( ZyppSel selectable,
                         ZyppPkg pkg );


    /**
     * Returns a string containing a HTML table for technical details for two
     * package instances: The installed instance and an alternate instance.
     * (usually the candidate instance).
     **/
    QString complexTable( ZyppSel selectable,
                          ZyppPkg installedPkg,
                          ZyppPkg candidatePkg );

    /**
     * Returns a string containing HTML code for a package's authors list.
     **/
    QString authorsListCell( ZyppPkg pkg ) const;
};


#endif // ifndef YQPkgTechnicalDetailsView_h
