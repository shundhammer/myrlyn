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
#include <QPushButton>
#include <QScreen>
#include <QSplitter>
#include <QStyle>
#include <QList>
#include <QBoxLayout>

#include "MainWindow.h"
#include "QY2CursorHelper.h"
#include "YQPkgList.h"
#include "YQi18n.h"
#include "utf8.h"
#include "YQZypp.h"
#include "YQPkgDescriptionView.h"
#include "YQPkgDescriptionDialog.h"


#define SPACING  2      // between subwidgets
#define MARGIN   4      // around the widget


YQPkgDescriptionDialog::YQPkgDescriptionDialog( QWidget *       parent,
                                                const QString & pkgName )
    : QDialog( parent ? parent : MainWindow::instance() )
{
    // Dialog title
    setWindowTitle( _( "Package Description" ) );

    // Enable dialog resizing even without window manager
    setSizeGripEnabled( true );


    // Layout for the dialog

    QVBoxLayout * layout = new QVBoxLayout();
    Q_CHECK_PTR( layout );
    setLayout(layout);
    layout->setContentsMargins( MARGIN, MARGIN, MARGIN, MARGIN );
    layout->setSpacing( SPACING );


    // Splitter

    QSplitter * splitter = new QSplitter( Qt::Vertical, this );
    Q_CHECK_PTR( splitter );
    layout->addWidget( splitter );

    // Pkg list

    _pkgList = new YQPkgList( splitter );
    Q_CHECK_PTR( _pkgList );
    _pkgList->resize( _pkgList->width(), 80 );


    // Description view

    _pkgDescription = new YQPkgDescriptionView( splitter );
    Q_CHECK_PTR( _pkgDescription );
    _pkgDescription->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) ); // hor/vert

    connect( _pkgList,        SIGNAL( currentItemChanged  ( ZyppSel ) ),
             _pkgDescription, SLOT  ( showDetailsIfVisible( ZyppSel ) ) );


    // Button box (to center the single button)

    QHBoxLayout * hbox = new QHBoxLayout();
    Q_CHECK_PTR( hbox );
    hbox->setSpacing( SPACING );
    hbox->setContentsMargins( MARGIN, MARGIN, MARGIN, MARGIN );
    layout->addLayout( hbox );

    // "OK" button

    QPushButton * button = new QPushButton( _( "&OK" ), this );
    Q_CHECK_PTR( button );
    hbox->addWidget(button);
    button->setDefault( true );

    connect( button,    SIGNAL( clicked() ),
             this,      SLOT  ( accept()  ) );

    hbox->addStretch();


    filter( pkgName );
}


void
YQPkgDescriptionDialog::filter( const QString & qPkgName )
{
    std::string pkgName( toUTF8( qPkgName ) );
    busyCursor();
    _pkgList->clear();


    // Search for pkgs with that name

    for ( ZyppPoolIterator it = zyppPkgBegin();
          it != zyppPkgEnd();
          ++it )
    {
        ZyppObj zyppObj = (*it)->theObj();

        if ( zyppObj && zyppObj->name() == pkgName )
            _pkgList->addPkgItem( *it, tryCastToZyppPkg( zyppObj ) );
    }

    normalCursor();
}


bool
YQPkgDescriptionDialog::isEmpty() const
{
    return true;
}


QSize
YQPkgDescriptionDialog::sizeHint() const
{
    QSize size = QDialog::sizeHint();
    size = size.boundedTo( screen()->availableGeometry().size() );

    return size;
}


void
YQPkgDescriptionDialog::showDescriptionDialog( const QString & pkgName )
{
    YQPkgDescriptionDialog dialog( 0, pkgName );
    dialog.exec();
}
