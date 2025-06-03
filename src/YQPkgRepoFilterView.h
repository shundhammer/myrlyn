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


#ifndef YQPkgRepoFilterView_h
#define YQPkgRepoFilterView_h

#include "YQPkgSecondaryFilterView.h"

class QWidget;
class YQPkgRepoList;

class YQPkgRepoFilterView : public YQPkgSecondaryFilterView
{
    Q_OBJECT

public:

    /**
     * Constructor
     **/
    YQPkgRepoFilterView( QWidget * parent );

    /**
     * Destructor
     **/
    virtual ~YQPkgRepoFilterView();

    /**
     * Current selected repository, or 0 if nothing is selected
     **/
    zypp::Repository selectedRepo() const;


protected:

    /**
     * The actual filter method: Find the packages that belong to this repo.
     **/
    virtual void primaryFilter();


    // Data members

    YQPkgRepoList * _repoList;
};



#endif // ifndef YQPkgRepoFilterView_h
