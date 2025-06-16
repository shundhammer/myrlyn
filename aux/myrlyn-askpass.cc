/*  ------------------------------------------------------
              __   _____  ____  _
              \ \ / / _ \|  _ \| | ____ _
               \ V / | | | |_) | |/ / _` |
                | || |_| |  __/|   < (_| |
                |_| \__\_\_|   |_|\_\__, |
                                    |___/
    ------------------------------------------------------

    Project:  Myrlyn Package Manager GUI
    Copyright (c) Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
    License:  GPL V2 - See file LICENSE for details.

 */


#include <iostream>	// cerr

#include <QApplication>
#include <QString>
#include <QByteArray>
#include "myrlyn-askpass.h"


AskPassWin::AskPassWin()
    : QDialog()
    , ui( new Ui::MyrlynAskPassWin )
{
    ui->setupUi( this ); // Actually create the widgets from the .ui form

    QString userName = qgetenv( "USER" );

    if ( ! userName.isEmpty() )
    {
        QString msg = tr( "Password for %1:" ).arg( userName );
        ui->passwordLabel->setText( msg );
    }

    ui->passwordLineEdit->setFocus();
}


AskPassWin::~AskPassWin()
{
    delete ui;
}


int main( int argc, char *argv[] )
{
    QApplication qtApp( argc, argv);

    AskPassWin dialog;
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
