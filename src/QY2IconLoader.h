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


#ifndef QY2IconLoader_h
#define QY2IconLoader_h

#include <QIcon>

/**
 * Helper class to load icons from various locations
 **/
class QY2IconLoader
{
public:
    /**
     * Load an icon. This tries several locations:
     *
     * - The icon theme from the current desktop
     * - The compiled-in Qt resources
     * - An external file
     *
     * If the icon does not have a filename extension, the icon theme will try
     * to append ".svg" and ".png". For the compiled-in Qt resources, there are
     * aliases specified ("foo" -> "foo.svg"), so it will also work without an
     * extension.
     *
     * For external files, a path and an extension will be necessary.
     *
     * If no icon could be loaded, this will return a null QIcon (check with
     * icon.isNull()), and a warning is logged.
     **/
    static QIcon loadIcon( const QString & iconName_str );

    /**
     * Load an icon from the desktop theme.
     * Those icon names typically do not contain a leading path or a filename extension.
     **/
    static QIcon loadThemeIcon( const QString & iconName );

    /**
     * Load an icon from an absolute path (including filename extension).
     **/
    static QIcon loadIconFromPath( const QString & iconName );

    /**
     * Load a built-in icon, i.e. one from the Qt resource system.
     *
     * Use either an alias from qt_icons.rcc (and no filename extension)
     * or a full filename with path and extension, e.g. "icons/foo.svg".
     * A leading ":/" will be added if not already there.
     **/
    static QIcon loadBuiltInIcon( const QString & iconName );
};

#endif // QY2IconLoader_h
