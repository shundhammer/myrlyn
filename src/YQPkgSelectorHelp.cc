/*  ---------------------------------------------------------
               __  __            _
              |  \/  |_   _ _ __| |_   _ _ __
              | |\/| | | | | '__| | | | | '_ \
              | |  | | |_| | |  | | |_| | | | |
              |_|  |_|\__, |_|  |_|\__, |_| |_|
                      |___/        |___/
    ---------------------------------------------------------

    Project:  Myrlyn Package Manager GUI
    Copyright (c) 2024-26 SUSE LLC
    License:  GPL V2 - See file LICENSE for details.

 */


#include <grp.h>     // setgroups()
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


void YQPkgSelector::helpRootAuth()
{
    openInBrowser( "https://github.com/shundhammer/myrlyn/issues/99" );
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
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QProcess process;

    if ( getuid() == 0 ) // Running as root?
    {
        if ( ! dropPrivileges( &process, &env ) )
        {
            logError() << "Can't drop privileges to open URL " << url << endl;
            return;
        }
    }

    process.setProgram( "/usr/bin/xdg-open" );
    process.setArguments( QStringList() << url );
    process.setProcessEnvironment( env );

    logDebug() << "Opening URL " << url << endl;
    process.startDetached();
}


bool YQPkgSelector::dropPrivileges( QProcess *            process,
                                    QProcessEnvironment * env )
{
    bool ok1, ok2;
    int sudoUid = env->value( "SUDO_UID" ).toInt( &ok1 );
    int sudoGid = env->value( "SUDO_GID" ).toInt( &ok2 );
    QString sudoUser = env->value( "SUDO_USER" );
    QString sudoHome = env->value( "SUDO_HOME" );
    QString rootHome = env->value( "HOME", "/root" );

    if ( ! ok1 || ! ok2 || sudoUser.isEmpty() || sudoHome.isEmpty() )
        return false; // error

    env->insert( "USER", sudoUser );
    env->insert( "HOME", sudoHome );

    // Change environment variables starting with "/root" to $SUDO_HOME

    for ( const QString & key: env->keys() )
    {
        QString val = env->value( key );

        if ( val.startsWith( rootHome ) )
        {
            val.replace( rootHome, sudoHome );
            env->insert( key, val );
        }
    }

    logDebug() << "Dropping root privileges down to UID "
               << sudoUid << " and GID " << sudoGid << endl;

    process->setChildProcessModifier( [ sudoUid, sudoGid ]
        {
            setuid( sudoUid );
            setgid( sudoGid );
        } );

    return true; // success
}


bool YQPkgSelector::canOpenInBrowser()
{
    if ( geteuid() != 0 )       // Not running as root?
        return true;

    // if the SUDO_USER environment variable and related are set,
    // we can drop privileges, so we can safely run 'xdg-open'.
    //
    // But only 'sudo' (and 'myrlyn-sudo') does this; others like 'kdesu',
    // 'gnomesu', 'xdg-su' don't.

    return ! qgetenv( "SUDO_USER" ).isEmpty();
}

