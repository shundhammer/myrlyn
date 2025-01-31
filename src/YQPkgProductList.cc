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

    Textdomain "qt-pkg"
 */


#include "Logger.h"
#include "YQi18n.h"
#include "YQPkgProductList.h"


using std::list;
using std::set;


YQPkgProductList::YQPkgProductList( QWidget * parent )
    : YQPkgObjList( parent )
    , _vendorCol( -42 )
{
    logDebug() << "Creating product list" << Qt::endl;

    QStringList headers;
    int numCol = 0;
    headers <<  ( "" );        _statusCol  = numCol++;
    headers << _( "Product" ); _nameCol    = numCol++;
    headers << _( "Summary" ); _summaryCol = numCol++;
    headers << _( "Version" ); _versionCol = numCol++;
    headers << _( "Vendor"  ); _vendorCol  = numCol++;

    setColumnCount( numCol );
    setHeaderLabels( headers );
    setAllColumnsShowFocus( true );

    setSortingEnabled( true );
    sortByColumn( nameCol(), Qt::AscendingOrder );

    fillList();
    selectSomething();

    setColumnWidth( _nameCol,    250 );
    setColumnWidth( _summaryCol, 350 );
    setColumnWidth( _versionCol, 150 );
    setColumnWidth( _vendorCol,  200 );

    logDebug() << "Creating product list done" << Qt::endl;
}


YQPkgProductList::~YQPkgProductList()
{
    // NOP
}


void
YQPkgProductList::fillList()
{
    clear();
    // logVerbose() << "Filling product list" << Qt::endl;

    for ( ZyppPoolIterator it = zyppProductsBegin();
          it != zyppProductsEnd();
          ++it )
    {
        ZyppProduct zyppProduct = tryCastToZyppProduct( (*it)->theObj() );

        if ( zyppProduct )
        {
            addProductItem( *it, zyppProduct );
        }
        else
        {
            logError() << "Found non-product selectable" << Qt::endl;
        }
    }

    // logVerbose() << "product list filled" << Qt::endl;
    resizeColumnToContents( _statusCol );
}


void
YQPkgProductList::addProductItem( ZyppSel     selectable,
                                  ZyppProduct zyppProduct )
{
    if ( ! selectable )
    {
        logError() << "NULL ZyppSel!" << Qt::endl;
        return;
    }

    new YQPkgProductListItem( this, selectable, zyppProduct );
}






YQPkgProductListItem::YQPkgProductListItem( YQPkgProductList *  productList,
                                            ZyppSel             selectable,
                                            ZyppProduct         zyppProduct )
    : YQPkgObjListItem( productList,
                        selectable,
                        zyppProduct )
    , _productList( productList )
    , _zyppProduct( zyppProduct )
{
    if ( ! _zyppProduct )
        _zyppProduct = tryCastToZyppProduct( selectable->theObj() );

    if ( ! _zyppProduct )
        return;

    setStatusIcon();

    if ( vendorCol() > -1 )
        setText( vendorCol(), zyppProduct->vendor() );

}


YQPkgProductListItem::~YQPkgProductListItem()
{
    // NOP
}


void
YQPkgProductListItem::applyChanges()
{
    solveResolvableCollections();
}
