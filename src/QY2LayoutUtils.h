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


#ifndef QY2LayoutUtils_h
#define QY2LayoutUtils_h

class QWidget;


/**
 * Add vertical stretchable space.
 **/
QWidget * addVStretch( QWidget * parent );

/**
 * Add horizontal stretchable space.
 **/
QWidget * addHStretch( QWidget * parent );

/**
 * Add a fixed height vertical space
 **/
QWidget * addVSpacing( QWidget * parent, int height = 8 );

/**
 * Add a fixed height horizontal space
 **/
QWidget * addHSpacing( QWidget * parent, int width = 8 );

/**
 * Limit a QSize to what is available on the same screen as 'widget'.
 **/
QSize limitToScreenSize( const QWidget * widget, const QSize & desiredSize );
QSize limitToScreenSize( const QWidget * widget, int width, int height );

#endif // ifndef QY2LayoutUtils_h
