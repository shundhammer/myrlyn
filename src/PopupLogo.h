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


#ifndef PopupLogo_h
#define PopupLogo_h


#include <QLabel>

class LogoPopup;
class QMouseEvent;
class QKeyEvent;
class QShowEvent;


/**
 * Small logo that pops up a large one when clicked.
 **/
class PopupLogo: public QLabel
{
    Q_OBJECT

public:

    /**
     * Constructor: Creates a small QLabel from the filenames of the small
     * logo. When clicked, it creeates the logo popup from the large logo
     * filename.
     **/
    PopupLogo( const QString & smallLogoName,
               const QString & largeLogoName,
               QWidget *       parent = 0     );

    /**
     * Destructor.
     **/
    virtual ~PopupLogo();

    /**
     * Enable clicks to open a popup with the large logo.
     * Clicks are enabled by default.
     **/
    void enableClicks()  { _clicksEnabled = true; }

    /**
     * Disable clicks to open a popup with the large logo.
     * Clicks are enabled by default.
     **/
    void disableClicks() { _clicksEnabled = false;  }

    /**
     * Return 'true' if clicks are enabled, false otherwise.
     **/
    bool clicksEnabled() { return _clicksEnabled; }

protected:

    /**
     * Mouse press event handler: Open the popup.
     *
     * Reimplemented from QWidget.
     **/
    void mousePressEvent( QMouseEvent * event ) override;

    /**
     * Create the popup if it isn't created yet and open it.
     **/
    void openPopup();


    // Data members

    QString     _smallLogoName;
    QString     _largeLogoName;
    bool        _clicksEnabled;
    LogoPopup * _popup;
};


/**
 * The popup for the large logo.  This is a frame-less window with just the
 * large logo.  A click or the [Esc] key will close it.
 **/
class LogoPopup: public QLabel
{
    Q_OBJECT

public:

    LogoPopup( const QString & logoName,
               QWidget *       parent = 0 );

    virtual ~LogoPopup();

protected:

    /**
     * Mouse press event handler: Close the popup.
     *
     * Reimplemented from QWidget.
     **/
    void mousePressEvent( QMouseEvent * event ) override;

    /**
     * Key event handler: Close the popup.
     *
     * Reimplemented from QWidget.
     **/
    void keyPressEvent( QKeyEvent * event ) override;

    /**
     * Process a show event.
     *
     * Reimplemented from QLabel / QWidget.
     **/
    virtual void showEvent( QShowEvent * event ) override;
};


#endif // PopupLogo_h
