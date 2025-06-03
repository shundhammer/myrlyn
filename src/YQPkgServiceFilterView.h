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


#ifndef YQPkgServiceFilterView_h
#define YQPkgServiceFilterView_h

#include "YQPkgSecondaryFilterView.h"

class QWidget;
class YQPkgServiceList;

/**
 * A filter view widget for libzypp services.
 * It should be used only when a libzypp service is present in the system.
 */
class YQPkgServiceFilterView : public YQPkgSecondaryFilterView
{
    Q_OBJECT

public:

    /**
     * Constructor
     **/
    YQPkgServiceFilterView( QWidget * parent );

    /**
     * Destructor
     **/
    virtual ~YQPkgServiceFilterView();

    /**
     * Is any enabled libzypp service present?
     */
    static bool any_service();

protected:

    virtual void primaryFilter();


    // Data members

    // list of services, owned by the parent widget
    YQPkgServiceList * _serviceList;
};


#endif // ifndef YQPkgServiceFilterView_h
