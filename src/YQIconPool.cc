/*  ------------------------------------------------------
              __   _____  ____  _
              \ \ / / _ \|  _ \| | ____ _
               \ V / | | | |_) | |/ / _` |
                | || |_| |  __/|   < (_| |
                |_| \__\_\_|   |_|\_\__, |
                                    |___/
    ------------------------------------------------------

    Project:  Myrlyn Package Manager GUI
    Copyright (c) 2024-25 SUSE LLC
    License:  GPL V2 - See file LICENSE for details.

    Textdomain "qt-pkg"
 */


#include <QPixmap>
#include <QIcon>

#include "Logger.h"
#include "utf8.h"
#include "YQIconPool.h"



YQIconPool * YQIconPool::_iconPool = 0;

QPixmap YQIconPool::pkgTaboo()			{ return iconPool()->cachedIcon( "package-available-locked", true );		}
QPixmap YQIconPool::pkgDel()			{ return iconPool()->cachedIcon( "package-remove", true );			}
QPixmap YQIconPool::pkgUpdate()			{ return iconPool()->cachedIcon( "package-upgrade", true );			}
QPixmap YQIconPool::pkgInstall()		{ return iconPool()->cachedIcon( "package-install", true );			}
QPixmap YQIconPool::pkgAutoInstall()		{ return iconPool()->cachedIcon( "package-install-auto", true );		}
QPixmap YQIconPool::pkgAutoUpdate()		{ return iconPool()->cachedIcon( "package-upgrade-auto", true );		}
QPixmap YQIconPool::pkgAutoDel()		{ return iconPool()->cachedIcon( "package-remove-auto", true );			}
QPixmap YQIconPool::pkgKeepInstalled()		{ return iconPool()->cachedIcon( "package-installed-updated", true );		}
QPixmap YQIconPool::pkgNoInst()			{ return iconPool()->cachedIcon( "package-available", true );			}
QPixmap YQIconPool::pkgProtected()		{ return iconPool()->cachedIcon( "package-installed-locked", true );		}

QPixmap YQIconPool::disabledPkgTaboo()		{ return iconPool()->cachedIcon( "package-available-locked", false );		}
QPixmap YQIconPool::disabledPkgDel()		{ return iconPool()->cachedIcon( "package-remove", false );			}
QPixmap YQIconPool::disabledPkgUpdate()		{ return iconPool()->cachedIcon( "package-upgrade", false );			}
QPixmap YQIconPool::disabledPkgInstall()	{ return iconPool()->cachedIcon( "package-install", false );			}
QPixmap YQIconPool::disabledPkgAutoInstall()	{ return iconPool()->cachedIcon( "package-install-auto", false );		}
QPixmap YQIconPool::disabledPkgAutoUpdate()	{ return iconPool()->cachedIcon( "package-upgrade-auto", false );		}
QPixmap YQIconPool::disabledPkgAutoDel()	{ return iconPool()->cachedIcon( "package-remove-auto", false );		}
QPixmap YQIconPool::disabledPkgKeepInstalled()	{ return iconPool()->cachedIcon( "package-installed-updated", false );		}
QPixmap YQIconPool::disabledPkgNoInst()		{ return iconPool()->cachedIcon( "package-available", false );			}
QPixmap YQIconPool::disabledPkgProtected()	{ return iconPool()->cachedIcon( "package-installed-locked", false );		}

QPixmap YQIconPool::normalPkgConflict()		{ return iconPool()->cachedIcon( "emblem-warning", true );			}

QPixmap YQIconPool::treePlus()			{ return iconPool()->cachedIcon( "list-add", true );				}
QPixmap YQIconPool::treeMinus()			{ return iconPool()->cachedIcon( "list-remove", true );				}

QPixmap YQIconPool::warningSign()		{ return iconPool()->cachedIcon( "emblem-warning", true );			}
QPixmap YQIconPool::pkgSatisfied()		{ return iconPool()->cachedIcon( "package-supported", true );			}

QPixmap YQIconPool::tabRemove()			{ return iconPool()->cachedIcon( "tab-close", true );				}
QPixmap YQIconPool::arrowLeft()			{ return iconPool()->cachedIcon( "arrow-left", true );				}
QPixmap YQIconPool::arrowRight()		{ return iconPool()->cachedIcon( "arrow-right", true );				}
QPixmap YQIconPool::arrowDown()			{ return iconPool()->cachedIcon( "arrow-down", true );				}

YQIconPool * YQIconPool::iconPool()
{
    if ( ! _iconPool )
	_iconPool = new YQIconPool();

    return _iconPool;
}


YQIconPool::YQIconPool()
{
}


YQIconPool::~YQIconPool()
{
    // NOP
}



QPixmap
YQIconPool::cachedIcon( const QString icon_name, const bool enabled )
{
    Q_INIT_RESOURCE( icons );
    QPixmap iconPixmap = _iconCache[ icon_name + (enabled ? 'e' : 'd') ];

    if ( !iconPixmap )
    {
        iconPixmap = loadIcon( icon_name, enabled );

        if ( !iconPixmap )
        {
            // Create an icon for the cache to avoid more than one complaint
            // and to have a clearly visible error icon (a small red square)
            iconPixmap = QPixmap( 8, 8 );
            iconPixmap.fill( Qt::red );
        }
    }

    _iconCache.insert( icon_name + (enabled ? 'e' : 'd'), iconPixmap );

    return iconPixmap;
}


QPixmap
YQIconPool::loadIcon( const QString icon_name, const bool enabled )
{
    QPixmap iconPixmap = _iconCache[ icon_name + (enabled ? 'e' : 'd') ];

    if ( QIcon::hasThemeIcon( icon_name ) )
    {
        // logVerbose() << "Loading theme icon " << icon_name << Qt::endl;

        QIcon icon = QIcon::fromTheme( icon_name, QIcon( ":/" + icon_name ) );
        iconPixmap = icon.pixmap( QSize( 16, 16 ), enabled ? QIcon::Normal : QIcon::Disabled );
    }
    else
    {
        // logVerbose() << "Loading built-in icon " << icon_name << Qt::endl;

        QIcon icon = QIcon( ":/" + icon_name );
        iconPixmap = icon.pixmap( QSize( 16, 16 ), enabled ? QIcon::Normal : QIcon::Disabled );
    }

    if ( !iconPixmap )
        logError() << "Could not load icon " << icon_name << Qt::endl;

    return iconPixmap;
}

