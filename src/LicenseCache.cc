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


#include "Exception.h"
#include "Logger.h"
#include "LicenseCache.h"


LicenseCache * LicenseCache::_confirmed = 0;


LicenseCache * LicenseCache::confirmed()
{
    if ( ! _confirmed )
        _confirmed = new LicenseCache;

    CHECK_NEW( _confirmed );

    return _confirmed;
}


void LicenseCache::add( const std::string & license )
{
    _cache.insert( license );
}


bool LicenseCache::contains( const std::string & license )
{
    return _cache.find( license ) != _cache.end();
}


void LicenseCache::clear()
{
    _cache.clear();
}
