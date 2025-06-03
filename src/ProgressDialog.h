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


#ifndef ProgressDialog_h
#define ProgressDialog_h


#include <QElapsedTimer>
#include <QDialog>


// Generated with 'uic' from a Qt designer .ui form: progress-dialog.ui
//
// Check out ../build/src/myrlyn_autogen/include/ui_progress-dialog-page.h
// for the variable names of the widgets.

#include "ui_progress-dialog.h"

/**
 * Simple progress bar dialog with a label above and a "Close" button below the
 * progress bar. The "Close" button is already set up to close the dialog.
 *
 * This is very much like the standard QProgressDialog, but with a better visual
 * appearance.
 **/
class ProgressDialog: public QDialog
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * If 'parent' is 0, it will use MainWindow::instance(), so the dialog is
     * centered above the main window.
     **/
    ProgressDialog( const QString & text, QWidget * parent = 0 );

    /**
     * Destructor.
     **/
    virtual ~ProgressDialog();

    /**
     * Reset internal values. In particular, it sets the progress bar value to
     * 0 and starts the timer.
     **/
    void reset();

    /**
     * Return the current progressbar value in percent.
     **/
    int value() const { return progressBar()->value(); }

    /**
     * Set the progressbar value in percent (0..100).
     **/
    void setValue( int val ) { progressBar()->setValue( val ); }

    /**
     * Set the text of the label above the progress bar.
     **/
    void setText( const QString & text ) { label()->setText( text ); }

    /**
     * Show this once if 'delayMillisec' have passed since the last reset() and
     * return 'true' if this was actually shown, so it makes sense to process
     * events in a local event loop to make sure to update the display. Return
     * 'false' otherwise.
     *
     * This is only done once so the use can use the "Close" button and it
     * doesn't keep popping up again with the next update of the progress bar.
     *
     * The idea behind showing the dialog only oncd is to prevent very short
     * pop-ups that are opened and then immediately closed again; this is
     * irritating to the user.
     *
     * Remember that there is also QWidget::show() (to show the dialog
     * unconnditionaly) and QWidget::hide().
     **/
    bool showDelayed( int delayMillisec );

    /**
     * Return the time in millisec since last reset() or -1 if the timer is
     * invalid.
     **/
    int elapsed() const;


    // Access to the widgets

    QLabel *        label()       const { return _ui->label;       }
    QProgressBar *  progressBar() const { return _ui->progressBar; }
    QPushButton *   closeButton() const { return _ui->closeButton; }
    QElapsedTimer * timer()             { return &_timer;          }

    //
    // Data members
    //

    Ui::ProgressDialog * _ui;
    QElapsedTimer        _timer;
    bool                 _shown;
};

#endif // ProgressDialog_h
