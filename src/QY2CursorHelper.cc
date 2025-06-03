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


#include <QCursor>
#include <QGuiApplication>

#include "QY2CursorHelper.h"


void busyCursor()
{
    // qApp is Qt's global QApplication instance
    qApp->setOverrideCursor( Qt::BusyCursor );
}


void normalCursor()
{
    // qApp is Qt's global QApplication instance

    while ( qApp->overrideCursor() )
	qApp->restoreOverrideCursor();
}
