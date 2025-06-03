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


#ifndef YQPkgProductList_h
#define YQPkgProductList_h

#include <string>

#include "YQPkgObjList.h"
#include "YQZypp.h"

using std::string;

class YQPkgProductListItem;


/**
 * Display a list of zypp::Product objects.
 **/
class YQPkgProductList : public YQPkgObjList
{
    Q_OBJECT

public:

    /**
     * Constructor
     **/
    YQPkgProductList( QWidget * parent );

    /**
     * Destructor
     **/
    virtual ~YQPkgProductList();


public slots:

    /**
     * Add a product to the list. Connect a filter's filterMatch() signal to
     * this slot. Remember to connect filterStart() to clear() (inherited from
     * QListView).
     **/
    void addProductItem( ZyppSel     selectable,
                         ZyppProduct zyppProduct );

    /**
     * Fill the product list.
     **/
    void fillList();


public:

    /**
     * Returns the currently selected item or 0 if there is none.
     **/
    YQPkgProductListItem * selection() const;

    /**
     * Returns the column for the product's vendor.
     **/
    int vendorCol() const { return _vendorCol; }


protected:


    // Data members

    int _vendorCol;
};



class YQPkgProductListItem: public YQPkgObjListItem
{
public:

    /**
     * Constructor. Creates a YQPkgProductList item that corresponds to
     * zyppProduct.
     **/
    YQPkgProductListItem( YQPkgProductList *    productList,
                          ZyppSel               selectable,
                          ZyppProduct           zyppProduct );

    /**
     * Destructor
     **/
    virtual ~YQPkgProductListItem();

    /**
     * Returns the original zyppProduct object.
     **/
    ZyppProduct zyppProduct() const { return _zyppProduct; }


    // Columns

    int statusCol()     const   { return _productList->statusCol();     }
    int summaryCol()    const   { return _productList->summaryCol();    }
    int versionCol()    const   { return _productList->versionCol();    }
    int vendorCol()     const   { return _productList->vendorCol();     }


protected:

    /**
     * Propagate status changes in this list to other lists:
     * Have the solver transact all products, patterns, etc.
     *
     * Reimplemented from YQPkgObjListItem.
     **/
    virtual void applyChanges() override;


    // Data members

    YQPkgProductList *  _productList;
    ZyppProduct         _zyppProduct;
};


#endif // ifndef YQPkgProductList_h
