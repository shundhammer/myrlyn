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


#ifndef RepoConfigDialog_h
#define RepoConfigDialog_h

#include <QDialog>


// Generated with 'uic' from a Qt designer .ui form: repo-config.ui
//
// Check out ../build/src/myrlyn_autogen/include/ui_repo-config.h
// for the variable names of the widgets.

#include "ui_repo-config.h"


/**
 * Dialog to configure repos with a RepoTable at the top, some widgets for the
 * current repo below that, and a row of buttons at the bottom.
 *
 * This dialog uses "instant apply", i.e. any changes are active immediately,
 * and just a "Close" button to dismiss it.
 **/
class RepoConfigDialog: public QDialog
{
    Q_OBJECT

public:

    /**
     * Constructor
     **/
    RepoConfigDialog( QWidget * parent = 0 );

    /**
     * Destructor
     **/
    virtual ~RepoConfigDialog();

    /**
     * Set this whole dialog to read-only mode.
     * This is the default if not running as root.
     **/
    void setReadOnlyMode( bool readOnly );


public slots:

    /**
     * Accept the changes. Currently, this is the only way out of this dialog.
     *
     * Reimplemented from QDialog.
     **/
    void accept() override;


protected slots:

    /**
     * Notification from the RepoTable that the currently selected item changed.
     **/
    void currentChanged();

    /**
     * Notification from the "priority" spinbox, the "enabled" checkbox or the
     * "autorefresh" checkbox that one of the details changed.
     **/
    void currentEdited();

    /**
     * Notification about a double click on a repo table item in column 'col'.
     *
     * Notice that the first click (with button #1, the left mouse button)
     * already makes that item the current item.
     **/
    void itemDoubleClicked( int               button,
                            QTreeWidgetItem * item,
                            int               col );

    /**
     * Add a new repo.
     **/
    void addRepo();

    /**
     * Edit the current repo.
     **/
    void editRepo();

    /**
     * Delete the current repo.
     **/
    void deleteRepo();


protected:

    /**
     * Set up signal / slot connections.
     **/
    void connectWidgets();

    /**
     * Update the data for the current repo in the widgets below the table.
     **/
    void updateCurrentData();

    /**
     * Set the _restartNeeded flag and show or hide the corresponding
     * notification widget accordingly.
     **/
    void restartNeeded( bool needed = true );

    /**
     * Show a message box telling the user that the application
     * needs to be restarted for the changes to take effect.
     **/
    void showRestartNeededPopup();

    /**
     * Post a confirmation pop-up dialog to ask the user if the specified repo
     * should really be deleted. Return 'true' if yes, 'false' if no.
     **/
    bool confirmDeleteRepo( const ZyppRepoInfo & repoInfo );

    /**
     * Event handler for WM_CLOSE (Alt-F4).
     *
     * Reimplemented from QWidget.
     **/
    virtual void closeEvent( QCloseEvent * event ) override;


    // Data members

    Ui::RepoConfig * _ui;  // see ui_repo-config.h
    RepoManager_Ptr  _repoManager;
    bool             _restartNeeded;
};


#endif // RepoConfigDialog_h
