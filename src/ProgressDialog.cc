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


#include "Exception.h"
#include "Logger.h"
#include "MainWindow.h"
#include "ProgressDialog.h"


ProgressDialog::ProgressDialog( const QString & text, QWidget * parent )
    : QDialog( parent ? parent : MainWindow::instance() )
    , _ui( new Ui::ProgressDialog )
    , _shown( false )
{
    CHECK_NEW( _ui );
    _ui->setupUi( this );
    setText( text );
    reset();
}


ProgressDialog::~ProgressDialog()
{
    // NOP
}


void ProgressDialog::reset()
{
    _ui->progressBar->setValue( 0 );
    _timer.start();
    _shown = false;
}


int ProgressDialog::elapsed() const
{
    return _timer.isValid() ? _timer.elapsed() : -1;
}


bool ProgressDialog::showDelayed( int delayMillisec )
{
    if ( _timer.elapsed() < delayMillisec || _shown || isVisible() )
        return false;

    logDebug() << "Showing progress dialog \""
               << _ui->label->text() << "\""
               << endl;
    show();
    _shown = true;

    return _shown;
}
