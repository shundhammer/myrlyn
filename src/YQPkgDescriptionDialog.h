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


#ifndef YQPkgDescriptionDialog_h
#define YQPkgDescriptionDialog_h

#include <qdialog.h>


class YQPkgList;
class YQPkgDescriptionView;


/**
 * Pkg status and description as a standalone popup dialog.
 **/
class YQPkgDescriptionDialog : public QDialog
{
    Q_OBJECT

protected:

    /**
     * Constructor: Creates a description dialog for all packages that match
     * 'pkgName'.
     *
     * Use the static 'showDescriptionDialog()' method instead.
     **/
    YQPkgDescriptionDialog( QWidget *       parent,
                            const QString & pkgName );

public:

    /**
     * Static convenience method: Post a description dialog for pkg 'pkgName'.
     **/
    static void showDescriptionDialog( const QString & pkgName );

    /**
     * Returns the preferred size.
     *
     * Reimplemented from QWidget to limit the dialog to the screen dimensions.
     **/
    virtual QSize sizeHint() const override;


protected:

    /**
     * Apply the filter criteria: Fill the pkg list with pkgs that match
     * 'pkgName'.
     **/
    void filter( const QString & pkgName );

    /**
     * Returns 'true' if the pkg list is empty.
     * This is only meaningful after calling 'filter()' !
     **/
    bool isEmpty() const;


    // Data members

    YQPkgList *            _pkgList;
    YQPkgDescriptionView * _pkgDescription;
};


#endif // ifndef YQPkgDescriptionDialog_h
