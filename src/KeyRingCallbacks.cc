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


#include "KeyRingCallbacks.h"


KeyRingCallbacks::KeyRingCallbacks()
{
    _keyRingReceiveCallback.connect();
}


KeyRingCallbacks::~KeyRingCallbacks()
{
    // Each zypp::Callback disconnects automatically in the dtor.
}
