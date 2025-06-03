/*  ---------------------------------------------------------
               __  __            _
              |  \/  |_   _ _ __| |_   _ _ __
              | |\/| | | | | '__| | | | | '_ \
              | |  | | |_| | |  | | |_| | | | |
              |_|  |_|\__, |_|  |_|\__, |_| |_|
                      |___/        |___/
    ---------------------------------------------------------

    Project:  Myrlyn Package Manager GUI
    Copyright (c) Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
    License:  GPL V2 - See file LICENSE for details.

 */


#ifndef InitReposPage_h
#define InitReposPage_h


#include <QPixmap>
#include <QWidget>

#include "YQZypp.h"


class MyrlynRepoManager;


// Generated with 'uic' from a Qt designer .ui form: init-repos-page.ui
//
// Check out ../build/src/myrlyn_autogen/include/ui_init-repos-page.h
// for the variable names of the widgets.

#include "ui_init-repos-page.h"


/**
 * Workflow page to visualize what the MyrlynRepoManager is doing during the
 * "init repos" phase.
 **/
class InitReposPage: public QWidget
{
    Q_OBJECT

public:

    /**
     * Constructor.
     **/
    InitReposPage( MyrlynRepoManager * repoManager, QWidget * parent = 0 );

    /**
     * Destructor.
     **/
    ~InitReposPage();


public slots:

    /**
     * Reset the internal status and all widgets.
     **/
    void reset();

    /**
     * Notification that an enabled repo was found.
     **/
    void foundRepo( const ZyppRepoInfo & repo );

    /**
     * Notification that refreshing a repo starts.
     **/
    void refreshRepoStart( const ZyppRepoInfo & repo );

    /**
     * Notification that refreshing a repo is done.
     **/
    void refreshRepoDone( const ZyppRepoInfo & repo );

    /**
     * Notification that refreshing a repo failed.
     **/
    void refreshRepoError( const ZyppRepoInfo & repo );


protected:

    /**
     * Set up Qt signal / slot connections.
     **/
    void connectSignals();

    /**
     * Load the icons for this page.
     **/
    void loadIcons();

    /**
     * Set the icon for the list widget item for 'repo' to 'icon' and return
     * the item (or 0 if not found).
     **/
    QListWidgetItem * setItemIcon( const ZyppRepoInfo & repo,
                                   const QPixmap &      icon );

    /**
     * Return the item for the specified repo in the repos list widget
     * or 0 if there is no such item.
     **/
    QListWidgetItem * findRepoItem( const ZyppRepoInfo & repo );


    //
    // Data members
    //

    MyrlynRepoManager * _repoManager;
    Ui::InitReposPage * _ui;       // See ui_init-repos-page.h

    int                 _reposCount;
    int                 _refreshDoneCount;

    QPixmap             _emptyIcon;
    QPixmap             _downloadOngoingIcon;
    QPixmap             _downloadDoneIcon;
    QPixmap             _downloadErrorIcon;
};


#endif // InitReposPage_h
