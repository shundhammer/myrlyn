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


#include <iostream>	// cout

#include <QApplication>
#include <QString>
#include <QByteArray>
#include "Translator.h"
#include "myrlyn-askpass.h"


AskPassWin::AskPassWin( const QString & prompt )
    : QDialog()
    , ui( new Ui::MyrlynAskPassWin )
    , _prompt( prompt )
{
    ui->setupUi( this ); // Actually create the widgets from the .ui form

    if ( _prompt.isEmpty() )
    {
        QString userName = qgetenv( "USER" );

        if ( ! userName.isEmpty() )
            _prompt = tr( "Password for %1:" ).arg( userName );
    }

    if ( ! _prompt.isEmpty() )
        ui->passwordLabel->setText( _prompt );

    ui->passwordLineEdit->setFocus();
}


AskPassWin::~AskPassWin()
{
    delete ui;
}


int main( int argc, char *argv[] )
{
    QApplication qtApp( argc, argv );
    QStringList argList = qtApp.arguments();
    argList.removeFirst(); // Remove the program name
    QString prompt;

    // 'sudo' calls this with one of
    // "[sudo] password for root"
    // "[sudo] password for kilroy"
    //    0       1      2    3      (sections)
    //   -4      -3     -2   -1

    if ( ! argList.isEmpty() )
    {
        prompt = argList.first();

        if ( prompt.contains( "password for", Qt::CaseInsensitive ) )
        {
            QString user = prompt.section( ' ', -1 ); // The rightmost word
            prompt = QObject::tr( "Password for %1" ).arg( user );
        }
    }

    if ( prompt.isEmpty() )
        prompt = QObject::tr( "Password:" );

    Translator translator( "myrlyn" ); // Using the same textdomain as Myrlyn
    qApp->installTranslator( &translator );

    AskPassWin dialog( prompt );
    dialog.exec();

    if ( dialog.result() == QDialog::Accepted )
    {
        QString password = dialog.ui->passwordLineEdit->text();

        if ( ! password.isEmpty() )
        {
            std::cout << password.toUtf8().data() << std::endl;
            return 0;
        }
    }

    return 1;
}
