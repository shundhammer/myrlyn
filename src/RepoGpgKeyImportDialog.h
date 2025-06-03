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


#ifndef RepoGpgKeyImportDialog_h
#define RepoGpgKeyImportDialog_h


#include <string>
#include <zypp/PublicKey.h>
#include <zypp/RepoInfo.h>

#include <QDialog>

#include "YQZypp.h"


// Generated with 'uic' from a Qt designer .ui form: repo-gpg-key-import-dialog.ui
//
// Check out ../build/src/myrlyn_autogen/include/ui_repo-gpg-key-import-dialog.h
// for the variable names of the widgets.

#include "ui_repo-gpg-key-import-dialog.h"


/**
 * Dialog to import a GPG public key from a repository.
 **/
class RepoGpgKeyImportDialog: public QDialog
{
    Q_OBJECT

public:

    RepoGpgKeyImportDialog( const zypp::PublicKey & key,
                            const ZyppRepoInfo  &   repoInfo,
                            QWidget *               parent = 0 );

    virtual ~RepoGpgKeyImportDialog();

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

    /**
     * Disable the repo and tellt he user about it.
     **/
    void disableRepo();


    // Data members

    Ui::RepoGpgKeyImportDialog * _ui; // See ui_repo-gpg-key-import-dialog.ui
    ZyppRepoInfo                 _repoInfo;
};

#endif // RepoGpgKeyImportDialog_h
