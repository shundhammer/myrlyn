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


#include <zypp/Target.h>

#include "Exception.h"
#include "Logger.h"
#include "YQZypp.h"
#include "utf8.h"
#include "BaseProduct.h"


BaseProduct *
BaseProduct::instance()
{
    static BaseProduct * inst = 0;

    if ( ! inst )
    {
        inst = new BaseProduct();
        CHECK_NEW( inst );

        zypp::Target_Ptr target = zypp::getZYpp()->target();
        zypp::Product::constPtr product = target->baseProduct();

        inst->_name      = fromUTF8( product->name()              );
        inst->_summary   = fromUTF8( product->summary()           );
        inst->_shortName = fromUTF8( product->shortName()         );
        inst->_version   = fromUTF8( product->edition().version() );
        inst->_vendor    = fromUTF8( product->vendor()            );
    }

    return inst;
}


void BaseProduct::log()
{
    logInfo() << "Base product" << endl;
    logInfo() << "  Name:       " << instance()->_name      << endl;
    logInfo() << "  Version:    " << instance()->_version   << endl;
    logInfo() << "  Summary:    " << instance()->_summary   << endl;
    logInfo() << "  Short name: " << instance()->_shortName << endl;
    logInfo() << "  Vendor:     " << instance()->_vendor    << endl;
}
