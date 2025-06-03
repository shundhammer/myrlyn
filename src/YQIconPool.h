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

#include <qpixmap.h>
#include <QHash>

class YQIconPool
{
public:

    static QPixmap pkgAutoDel();
    static QPixmap pkgAutoInstall();
    static QPixmap pkgAutoUpdate();
    static QPixmap pkgDel();
    static QPixmap pkgInstall();
    static QPixmap pkgKeepInstalled();
    static QPixmap pkgNoInst();
    static QPixmap pkgProtected();
    static QPixmap pkgTaboo();
    static QPixmap pkgUpdate();

    static QPixmap disabledPkgAutoDel();
    static QPixmap disabledPkgAutoInstall();
    static QPixmap disabledPkgAutoUpdate();
    static QPixmap disabledPkgDel();
    static QPixmap disabledPkgInstall();
    static QPixmap disabledPkgKeepInstalled();
    static QPixmap disabledPkgNoInst();
    static QPixmap disabledPkgProtected();
    static QPixmap disabledPkgTaboo();
    static QPixmap disabledPkgUpdate();

    static QPixmap normalPkgConflict();

    static QPixmap treePlus();
    static QPixmap treeMinus();

    static QPixmap warningSign();
    static QPixmap pkgSatisfied();

    static QPixmap tabRemove();
    static QPixmap arrowLeft();
    static QPixmap arrowRight();
    static QPixmap arrowDown();
    static QPixmap checkmark();

protected:

    /**
     * Return the global icon pool.
     **/
    static YQIconPool * instance();

    /**
     * Return the cached icon for 'iconName'. If the icon isn't in the cache
     * yet, load it and store it in the cache.
     *
     * Return a red square as an error icon if there is no icon by that name.
     **/
    QPixmap cachedIcon( const QString & iconName, bool enabled );

    /**
     * Load the icon for 'iconName' from the icon theme or, if that fails,
     * from the compiled-in icons (using the Qt resource system). Return a null
     * pixmap if there is no such icon.
     **/
    QPixmap loadIcon( const QString & iconName, bool enabled );


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


    //
    // Data members
    //

    static YQIconPool *             _instance;
    QHash< const QString, QPixmap > _iconCache;
};


#endif // ifndef YQIconPool_h
