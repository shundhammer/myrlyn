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


#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QPushButton>
#include <QScreen>
#include <QSplitter>
#include <QTabWidget>

#include "MainWindow.h"
#include "YQPkgDependenciesView.h"
#include "YQPkgProductList.h"
#include "YQi18n.h"
#include "YQPkgProductDialog.h"


#define SPACING    4  // between subwidgets
#define MARGIN    10  // around the widget


YQPkgProductDialog::YQPkgProductDialog( QWidget * parent )
    : QDialog( parent ? parent : MainWindow::instance() )
{
    // Dialog title
    setWindowTitle( _( "Products" ) );

    setSizeGripEnabled( true );
    setMinimumSize( 1020, 500 );

    // Layout for the dialog

    QVBoxLayout * layout = new QVBoxLayout();
    Q_CHECK_PTR( layout );
    setLayout( layout );
    layout->setSpacing( SPACING );
    layout->setContentsMargins( MARGIN, MARGIN, MARGIN, MARGIN );

    // VBox for splitter

    QSplitter * splitter = new QSplitter( Qt::Vertical, this );
    Q_CHECK_PTR( splitter );
    layout->addWidget( splitter );


    // Product list

    _productList = new YQPkgProductList( splitter );
    Q_CHECK_PTR( _productList );
    _productList->resize( _productList->width(), 80 );

    // Prevent status changes for now - this would require solver runs etc.
    _productList->setEditable( false );


    // Tab widget for details views (looks better even just with one)

    _detailsViews = new QTabWidget( splitter );
    Q_CHECK_PTR( _detailsViews );
    _detailsViews->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) ); // hor/vert

    // Dependencies view

    _dependenciesView = new YQPkgDependenciesView( _detailsViews );
    Q_CHECK_PTR( _dependenciesView );
    _detailsViews->addTab( _dependenciesView, _( "Dependencies" ) );
    _dependenciesView->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) ); // hor/vert

    connect( _productList,      SIGNAL( currentItemChanged  ( ZyppSel ) ),
             _dependenciesView, SLOT  ( showDetailsIfVisible( ZyppSel ) ) );


    // Button box (to center the single button)

    QHBoxLayout * hbox = new QHBoxLayout();
    Q_CHECK_PTR( hbox );
    hbox->setSpacing( SPACING );
    hbox->setContentsMargins( MARGIN, MARGIN, MARGIN, MARGIN );
    layout->addLayout( hbox );
    hbox->addStretch();


    // "OK" button

    QPushButton * button = new QPushButton( _( "&OK" ), this );
    Q_CHECK_PTR( button );
    hbox->addWidget(button);
    button->setDefault( true );

    connect( button,    SIGNAL( clicked() ),
             this,      SLOT  ( accept()  ) );

    hbox->addStretch();
}


QSize
YQPkgProductDialog::sizeHint() const
{
    QRect available = screen()->availableGeometry();
    QSize size = QDialog::sizeHint();
    size = size.boundedTo( QSize( available.width(), available.height() ) );

    return size;
}


void
YQPkgProductDialog::showProductDialog( QWidget* parent)
{
    YQPkgProductDialog dialog( parent );
    dialog.exec();
}
