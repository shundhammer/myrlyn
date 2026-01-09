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


#ifndef ZyppHistoryBrowser_h
#define ZyppHistoryBrowser_h

#include <QDialog>


// Generated with 'uic' from a Qt designer .ui form: zypp-history-browser.ui
//
// Check out ../build/src/myrlyn_autogen/include/ui_zypp-history-browser.h
// for the variable names of the widgets.

#include "ui_zypp-history-browser.h"


/**
 * Dialog to browser the zypp history.
 **/
class ZyppHistoryBrowser: public QDialog
{
    Q_OBJECT

public:

    /**
     * Constructor
     **/
    ZyppHistoryBrowser( QWidget * parent = 0 );

    /**
     * Destructor
     **/
    virtual ~ZyppHistoryBrowser();

protected slots:

    // TO DO

protected:

    /**
     * Fill the widgets with data
     **/
    void populate();

    // Data members

    Ui::ZyppHistoryBrowser * _ui;  // see ui_zypp-history-browser.h
};


#endif // ZyppHistoryBrowser_h
