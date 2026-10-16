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


#ifndef YQIconPool_h
#define YQIconPool_h

#include <QIcon>
#include <QHash>


class YQIconPool
{
public:

    static QIcon pkgInstall();
    static QIcon pkgUpdate();
    static QIcon pkgDel();
    static QIcon pkgAutoDel();
    static QIcon pkgAutoInstall();
    static QIcon pkgAutoUpdate();
    static QIcon pkgKeepInstalled();
    static QIcon pkgNoInst();
    static QIcon pkgProtected();
    static QIcon pkgTaboo();

    static QIcon arrowLeft();
    static QIcon arrowRight();
    static QIcon arrowDown();
    static QIcon checkmark();
    static QIcon pkgConflict();
    static QIcon tabClose();

    static QIcon download();
    static QIcon downloadDone();
    static QIcon downloadOngoing();
    static QIcon downloadPlaceholder();
    static QIcon emojiSad();
    static QIcon emojiSick();
    static QIcon hamburger();
    static QIcon wrench();


    static bool usingDarkTheme();
    static bool usingLightTheme() { return ! usingDarkTheme(); }

    static void setVerboseIconLoader( bool val = true ) { _verboseIconLoader = val; }


protected:

    /**
     * Return the global icon pool.
     **/
    static YQIconPool * instance();

    /**
     * Detect the widget theme (dark or light).
     * Return 'true' for a dark widget theme, 'false' for a light theme.
     **/
    static bool detectDarkWidgetTheme();

    /**
     * Return the cached icon for 'iconName'. If the icon isn't in the cache
     * yet, load it and store it in the cache.
     *
     * Return a red square as an error icon if there is no icon by that name.
     **/
    QIcon cachedIcon( const QString & iconName );

    /**
     * Load the icon for 'iconName' from the icon theme or, if that fails,
     * from the compiled-in icons (using the Qt resource system).
     *
     * Return a null pixmap if there is no such icon.
     **/
    QIcon loadIcon( const QString & iconName );

    /**
     * Load the icon for 'iconName' from the icon theme.
     *
     * Return a null pixmap if there is no such icon.
     **/
    QIcon loadThemeIcon( const QString & iconName );

    /**
     * Load a built-in icon for 'iconName' from the compiled-in Qt resource
     * file.
     *
     * Return a null pixmap if there is no such icon.
     **/
    QIcon loadBuiltinIcon( const QString & iconName );


private:

    /**
     * (Private!) Constructor
     * Singleton object - use the static methods only!
     **/
    YQIconPool();

    /**
     * Destructor
     **/
    virtual ~YQIconPool();

    void readSettings();
    void writeSettings();

    static QIcon ic( const QString & iconName );


    //
    // Data members
    //

    static YQIconPool *           _instance;
    static bool                   _useThemeIcons;
    static bool                   _verboseIconLoader;
    QHash< const QString, QIcon > _iconCache;
};


#endif // ifndef YQIconPool_h
