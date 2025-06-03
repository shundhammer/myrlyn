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


#ifndef BaseProduct_h
#define BaseProduct_h

#include <QString>


/**
 * Singleton class for information about the zypp base product.
 * Libzypp needs should be initialized before accessing this.
 *
 * See also /etc/products.d/baseproduct  and
 * https://gist.github.com/shundhammer/718a08eabaf6743e1aad1109079e865f
 **/
class BaseProduct
{
public:

    static QString name()      { return instance()->_name;      }
    static QString version()   { return instance()->_version;   }
    static QString summary()   { return instance()->_summary;   }
    static QString shortName() { return instance()->_shortName; }
    static QString vendor()    { return instance()->_vendor;    }

    /**
     * Write the base product to the log.
     **/
    static void log();

protected:

    static BaseProduct * instance();


    // Data members

    QString _name;
    QString _version;
    QString _summary;
    QString _shortName;
    QString _vendor;
};


#endif // BaseProduct_h
