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


#ifndef LicenseCache_h
#define LicenseCache_h

#include <string>
#include <set>

/**
 * A cache for licenses to check which ones have already been confirmed.
 **/
class LicenseCache
{
public:

    LicenseCache() {}
    ~LicenseCache() {}

    void add     ( const std::string & license );
    bool contains( const std::string & license );
    void clear();

    /**
     * Return the cache for confirmed licenses. Create it if it doesn't exist yet.
     **/
    static LicenseCache * confirmed();

private:

    std::set<std::string> _cache;
    static LicenseCache * _confirmed;
};

#endif // LicenseCache_h
