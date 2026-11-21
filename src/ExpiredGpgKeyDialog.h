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


#ifndef ExpiredGpgKeyDialog_h
#define ExpiredGpgKeyDialog_h


#include <string>
#include <zypp/PublicKey.h>

#include <QDialog>

#include "YQZypp.h"


// Generated with 'uic' from a Qt designer .ui form: repo-gpg-key-import-dialog.ui
//
// Check out ../build/src/myrlyn_autogen/include/ui_expired-gpg-key-dialog.h
// for the variable names of the widgets.

#include "ui_expired-gpg-key-dialog.h"


/**
 * Dialog to import a GPG public key from a repository.
 **/
class ExpiredGpgKeyDialog: public QDialog
{
    Q_OBJECT

public:

    ExpiredGpgKeyDialog( const zypp::PublicKey & key,
                         QWidget *               parent = 0 );

    virtual ~ExpiredGpgKeyDialog();

public slots:

    void accept() override;
    void reject() override;


protected:

    /**
     * Log the user's answer for either "Accepting" or "Rejecting".
     **/
    void logResult( const QString & result );

    /**
     * Set a QLabel with a std::string.
     **/
    void setText( QLabel * label, const std::string & text );


    // Data members

    Ui::ExpiredGpgKeyDialog * _ui; // See ui_expired-gpg-key-dialog.ui
};

#endif // ExpiredGpgKeyDialog_h
