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


#include <QBitmap>
#include <QKeyEvent>
#include <QMouseEvent>

#include "Logger.h"
#include "Exception.h"
#include "MainWindow.h"
#include "PopupLogo.h"


PopupLogo::PopupLogo( const QString & smallLogoName,
                      const QString & largeLogoName,
                      QWidget *       parent      )
    : QLabel( parent ? parent : MainWindow::instance() )
    , _smallLogoName( smallLogoName )
    , _largeLogoName( largeLogoName )
    , _clicksEnabled( true )
    , _popup( 0 )
{
    setPixmap( QPixmap( smallLogoName ) );
    setAlignment( Qt::AlignVCenter | Qt::AlignHCenter );
}


PopupLogo::~PopupLogo()
{
    if ( _popup )
        delete _popup;
}


void PopupLogo::mousePressEvent( QMouseEvent * event )
{
    if ( ! _clicksEnabled )
        return;

    if ( event->buttons() == Qt::LeftButton )
    {
        if ( _popup && _popup->isVisible() )
            _popup->hide();
        else
            openPopup();
    }
}


void PopupLogo::openPopup()
{
    if ( ! _popup )
        _popup = new LogoPopup( _largeLogoName );

    CHECK_NEW( _popup );
    _popup->show();
}




LogoPopup::LogoPopup( const QString & logoName,
                      QWidget *       parent    )
    : QLabel( parent ? parent : MainWindow::instance(),
              Qt::Tool | Qt::FramelessWindowHint )
{
    QPixmap logo( logoName );
    setPixmap( logo );
    setMask( logo.mask() );
}


LogoPopup::~LogoPopup()
{
    // NOP
}


void LogoPopup::mousePressEvent( QMouseEvent * event )
{
    if ( event->buttons() == Qt::LeftButton )
    {
        hide();
    }
}


void LogoPopup::keyPressEvent( QKeyEvent * event )
{
    if ( event->modifiers() == Qt::NoModifier &&
         event->key() == Qt::Key_Escape )
    {
        hide();
    }
}
