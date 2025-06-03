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


#ifndef YQPkgFileListView_h
#define YQPkgFileListView_h

#include "YQPkgGenericDetailsView.h"


using std::string;


/**
 * Display a package's file list
 **/
class YQPkgFileListView : public YQPkgGenericDetailsView
{
    Q_OBJECT

public:

    /**
     * Constructor
     **/
    YQPkgFileListView( QWidget * parent );

    /**
     * Destructor
     **/
    virtual ~YQPkgFileListView();

    /**
     * Show details for the specified package:
     * In this case the package description.
     *
     * Reimplemented from YQPkgGenericDetailsView.
     **/
    virtual void showDetails( ZyppSel selectable ) override;

protected:

    /**
     * Format a file list in HTML
     **/
    QString formatFileList( const std::list<string> & fileList ) const;
};


#endif // ifndef YQPkgFileListView_h
