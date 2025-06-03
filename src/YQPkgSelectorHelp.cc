/*  ------------------------------------------------------
              __   _____  ____  _
              \ \ / / _ \|  _ \| | ____ _
               \ V / | | | |_) | |/ / _` |
                | || |_| |  __/|   < (_| |
                |_| \__\_\_|   |_|\_\__, |
                                    |___/
    ------------------------------------------------------

    Project:  Myrlyn Package Manager GUI
    Copyright (c) 2024-25 SUSE LLC
    License:  GPL V2 - See file LICENSE for details.

 */


#include <QString>
#include <QProcess>

#include "Logger.h"
#include "ui_about-dialog.h"

#include "YQPkgSelector.h"


void YQPkgSelector::help()
{
    openInBrowser( "https://github.com/shundhammer/myrlyn/blob/master/README.md" );
}


void YQPkgSelector::helpRepoConfig()
{
    openInBrowser( "https://github.com/shundhammer/myrlyn/issues/69" );
}


void YQPkgSelector::about()
{
    QDialog aboutDialog( this );
    Ui::AboutDialog * ui( new Ui::AboutDialog );
    ui->setupUi( &aboutDialog );

    QString text = ui->heading->text();
    text += QString( " %1" ).arg( VERSION );
    ui->heading->setText( text );

    aboutDialog.exec();
    delete ui;
}


void YQPkgSelector::openInBrowser( const QString & url )
{
    logDebug() << "Opening URL " << url << endl;

    QProcess::startDetached( "/usr/bin/xdg-open", QStringList() << url );
}

