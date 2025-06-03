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


#ifndef YQPkgFilters_h
#define YQPkgFilters_h

#include "YQZypp.h"

/**
 * Zypp filtering helpers
 **/
class YQPkgFilters
{

public:

    /**
     * Returns the product if the filter finds a single product
     * or null if there are none, or if there are multiple products.
     *
      * 'filter' is the filtering functio. Its parameter is the product pool
      * item. This function returns true if it matches the expectations.
      */
    static ZyppProduct singleProductFilter( std::function<bool(const zypp::PoolItem & item)> filter );
};


#endif // ifndef YQPkgFilters_h

