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


#include "Logger.h"
#include "YQPkgSelMapper.h"


#define VERBOSE_MAPPER  1


int                   YQPkgSelMapper::_refCount = 0;
YQPkgSelMapper::Cache YQPkgSelMapper::_cache;


YQPkgSelMapper::YQPkgSelMapper()
{
    logDebug() << "Creating YQPkgSelMapper; refCount: " << _refCount + 1 << Qt::endl;

    if ( ++_refCount == 1 )
        rebuildCache();
}


YQPkgSelMapper::~YQPkgSelMapper()
{
    if ( --_refCount == 0 )
    {
        logDebug() << "Destroying pkg -> selectable cache"  << Qt::endl;
        _cache.clear();
    }

    logDebug() << "Destroying YQPkgSelMapper done." << Qt::endl;
}


void YQPkgSelMapper::rebuildCache()
{
    _cache.clear();
    logDebug() << "Building pkg -> selectable cache" << Qt::endl;

    for ( ZyppPoolIterator sel_it = zyppPkgBegin();
          sel_it != zyppPkgEnd();
          ++sel_it )
    {
        ZyppSel sel = *sel_it;

        if ( sel->installedObj() )
        {
            // The installed package (if there is any) may or may not be in the list
            // of available packages. Better make sure to insert it.

            ZyppPkg installedPkg = tryCastToZyppPkg( sel->installedObj() );

            if ( installedPkg )
                _cache.insert( CachePair( installedPkg, sel ) );
        }

        zypp::ui::Selectable::available_iterator it = sel->availableBegin();

        while ( it != sel->availableEnd() )
        {
            ZyppPkg pkg = tryCastToZyppPkg( *it );

            if ( pkg )
                _cache.insert( CachePair( pkg, sel ) );

            ++it;
        }
    }

    logDebug() << "Building pkg -> selectable cache done" << Qt::endl;
}


ZyppSel
YQPkgSelMapper::findZyppSel( ZyppPkg pkg )
{
    YQPkgSelMapper mapper; // This will build a cache, if there is none yet
    ZyppSel sel;

    YQPkgSelMapper::CacheIterator it = YQPkgSelMapper::_cache.find( pkg );

    if ( it != YQPkgSelMapper::_cache.end() )
        sel = it->second;
    else
    {
#if VERBOSE_MAPPER
        logInfo() << "No selectable found for package " << pkg->name() << Qt::endl;
#endif
    }

    return sel;
}

