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


#ifndef YQPkgTechnicalDetailsView_h
#define YQPkgTechnicalDetailsView_h

#include "YQPkgGenericDetailsView.h"


/**
 * Display technical details (very much like 'rpm -qi') for a ZYPP
 * object - the installed instance, the candidate instance or both ( in two
 * columns ) if both exist. All other available instances are ignored.
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
     * Overwritten from YQPkgGenericDetailsView.
     **/
    virtual void showDetails( ZyppSel selectable );

    /**
     * Returns a string containing a HTML table for technical details for one
     * package.
     **/
    QString simpleTable( ZyppSel selectable,
                         ZyppPkg pkg );


    /**
     * Returns a string containing a HTML table for technical details for two
     * package instances: The installed instance and an alternate instance.
     * ( usually the candidate instance ).
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
