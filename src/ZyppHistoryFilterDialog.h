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


#ifndef ZyppHistoryFilterDialog_h
#define ZyppHistoryFilterDialog_h

#include <QDialog>


// Generated with 'uic' from a Qt designer .ui form:
// zypp-history-filter_dialog.ui
//
// Check out ../build/src/myrlyn_autogen/include/ui_zypp-history-filter-dialog.h
// for the variable names of the widgets.

#include "ui_zypp-history-filter-dialog.h"


class ZyppHistoryFilter;


class ZyppHistoryFilterDialog: public QDialog
{
    Q_OBJECT

public:
    /**
     * Constructor
     **/
    ZyppHistoryFilterDialog( QWidget * parent = 0 );

    /**
     * Destructor
     **/
    virtual ~ZyppHistoryFilterDialog();

    /**
     * Create a filter from the fields in this dialog.
     *
     * This should only be called when the dialog was finished with
     * QDialog::Accepted, otherwise the result is undefined. In error cases
     * this may also return 0.
     *
     * Ownership of the created object is transferred to the caller.
     * Delete it when appropriate.
     **/
    ZyppHistoryFilter * filter();


protected slots:

    /**
     * Notification that any of the radio buttons was toggled so the fields
     * under the other radio buttons can be enabled or disabled accordingly.
     **/
    void radioButtonToggled( bool checked );

    /**
     * Enable or disable the [OK] button depending on the other widgets.
     **/
    void enableOkButton();

    
protected:

    void connectWidgets();
    void initRadioButtons();

    void readSettings();
    void writeSettings();


    // Data members

    Ui::ZyppHistoryFilterDialog * _ui;
};

#endif  // ZyppHistoryFilterDialog_h
