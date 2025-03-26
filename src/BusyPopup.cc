/*
 *   File name: BusyPopup.cc
 *   Summary:   QDirStat generic widget classes
 *   License:   GPL V2 - See file LICENSE for details.
 *
 *   Author:    Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 *              Donated by the QDirStat project
 */


#include <QEventLoop>
#include <QFrame>
#include <QGuiApplication>
#include <QRect>
#include <QScreen>

#include "Logger.h"
#include "BusyPopup.h"


BusyPopup::BusyPopup( const QString & text,
                      QWidget *       parent,
                      bool            autoPost ):
    QLabel( text, parent, Qt::SplashScreen ),
    _posted( false )
{
    setMargin( 15 );
    setWindowTitle( " " );

    if ( autoPost )
        post();
}


BusyPopup::~BusyPopup()
{
    // NOP
}


void BusyPopup::post()
{
    if ( _posted )
        return;

    show();
    processEvents();
    _posted = true;
}


void BusyPopup::processEvents( int millisec )
{
    QEventLoop eventLoop;
    eventLoop.processEvents( QEventLoop::ExcludeUserInputEvents,
                             millisec );
}


void BusyPopup::showEvent( QShowEvent * event )
{
    Q_UNUSED( event );

    if ( parentWidget() )
    {
        int x = ( parentWidget()->width()  - width()  ) / 2;
        int y = ( parentWidget()->height() - height() ) / 2;

        move( parentWidget()->x() + x, parentWidget()->y() + y );
    }
    else
    {
        QRect screenGeo = qApp->primaryScreen()->availableGeometry();

        int x = ( screenGeo.width()  - width()  ) / 2;
        int y = ( screenGeo.height() - height() ) / 2;

        move( screenGeo.x() + x, screenGeo.y() + y );
    }
}
