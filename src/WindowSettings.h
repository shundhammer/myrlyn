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


#ifndef WindowSettings_h
#define WindowSettings_h

#include <QString>

class QWidget;


class WindowSettings
{
public:
    /**
     * Read window settings (size and position) from the settings and apply
     * them.
     **/
    static void read( QWidget *       widget,
                      const QString & settingsGroup );

    /**
     * Write window settings (size and position) to the settings.
     **/
    static void write( QWidget *       widget,
                       const QString & settingsGroup );

};	// namespace QDirStat

#endif	// WindowSettings_h
