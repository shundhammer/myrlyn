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


#ifndef YQPkgProductDialog_h
#define YQPkgProductDialog_h

#include <QDialog>


class QTabWidget;
class YQPkgProductList;
class YQPkgDependenciesView;


/**
 * Products list with dependencies details view
 **/
class YQPkgProductDialog : public QDialog
{
    Q_OBJECT

public:

    /**
     * Static convenience method: Post a dialog with all products
     **/
    static void showProductDialog( QWidget* parent = 0);

    /**
     * Returns the preferred size.
     *
     * Reimplemented from QWidget to limit the dialog to the screen dimensions.
     **/
    virtual QSize sizeHint() const override;


protected:

    /**
     * Constructor.
     **/
    YQPkgProductDialog( QWidget * parent );


    // Data members

    YQPkgProductList *      _productList;
    QTabWidget *            _detailsViews;
    YQPkgDependenciesView * _dependenciesView;
};


#endif // ifndef YQPkgProductDialog_h
