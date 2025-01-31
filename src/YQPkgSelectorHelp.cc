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

    Textdomain "qt-pkg"
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


void YQPkgSelector::openActionUrl()
{
    // Donated by the QDirStat project:
    //
    // Use a QAction that was set up in Qt Designer to just open an URL in an
    // external web browser.
    //
    // This misappropriates the action's statusTip property to store the URL in
    // a field that is easily accessible in Qt Designer, yet doesn't get in the
    // way: It's not displayed automatically unlike the toolTip property.

    QAction * action = qobject_cast<QAction *>( sender() );

    if ( action )
    {
        QString url = action->statusTip();

        if ( url.isEmpty() )
            logError() << "No URL in statusTip() for action " << action->objectName();
        else
            openInBrowser( url );
    }
}


void YQPkgSelector::openInBrowser( const QString & url )
{
    logDebug() << "Opening URL " << url << Qt::endl;

    QProcess::startDetached( "/usr/bin/xdg-open", QStringList() << url );
}

