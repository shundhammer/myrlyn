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


#ifndef YQPkgChangeLogView_h
#define YQPkgChangeLogView_h

#include <zypp/Changelog.h>
#include "YQPkgGenericDetailsView.h"
#include "YQZypp.h"


using std::list;
using std::string;


/**
 * Display a pkg's file list
 **/
class YQPkgChangeLogView : public YQPkgGenericDetailsView
{
    Q_OBJECT

public:

    /**
     * Constructor
     **/
    YQPkgChangeLogView( QWidget * parent );

    /**
     * Destructor
     **/
    virtual ~YQPkgChangeLogView();

    /**
     * Show details for the specified package,
     * in this case the package description.
     *
     * Reimplemented from YQPkgGenericDetailsView.
     **/
    virtual void showDetails( ZyppSel selectable ) override;

protected:

    /**
     * Format a change log list in HTML
     **/
    QString changeLogTable( const zypp::Changelog & changeLog ) const;

    /**
     * Format an info message about not displayed changes
     * and return the ranslated message.
     */
    QString omittedEntries( int                 omittedCount,
                            const std::string & pkgName );
};


#endif // ifndef YQPkgChangeLogView_h
