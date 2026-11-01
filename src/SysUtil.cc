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


#include <grp.h>        // setgroups()
#include <unistd.h>     // access(), getuid(), geteduid()
#include <QProcess>

#include "Logger.h"
#include "SysUtil.h"

#define OPEN_CMD "/usr/bin/xdg-open"


void SysUtil::openInBrowser( const QString & url )
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QProcess process;

    if ( runningAsRoot() )
    {
        if ( ! dropPrivileges( &process, &env ) )
        {
            logError() << "Can't drop privileges to open URL " << url << endl;
            return;
        }
    }

    process.setProgram( OPEN_CMD );
    process.setArguments( QStringList() << url );
    process.setProcessEnvironment( env );
    process.setStandardOutputFile( QProcess::nullDevice() );

    logDebug() << "Opening URL " << url << endl;
    process.startDetached();
}


bool SysUtil::canOpenInBrowser()
{
    if ( ! haveXdgOpenCommand() )
        return false;

    if ( ! runningAsRoot() )
        return true;

    return canDropPrivileges();
}


bool SysUtil::canDropPrivileges()
{
    // Only 'sudo' (and implicitly 'myrlyn-sudo') sets environment variables
    // to restore a normal user's session so we can actually run non-privileged
    // commands in a graphical sessions; see dropPrivileges().
    //
    // Others like 'kdesu', 'gnomesu', 'xdg-su' don't.

    return runningWithSudo();
}


bool SysUtil::dropPrivileges( QProcess *            process,
                              QProcessEnvironment * env )
{
    bool ok1, ok2;
    int     sudoUid  = env->value( "SUDO_UID"  ).toInt( &ok1 );
    int     sudoGid  = env->value( "SUDO_GID"  ).toInt( &ok2 );
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
            // Get rid of group 0 (root) first
            setgid( sudoGid );

            const gid_t groups[] = { (gid_t) sudoGid };

            // This needs root permissions, so we need to do it before setuid()
            setgroups( sizeof( groups ) / sizeof( gid_t ), groups );

            // Drop privileges
            setuid( sudoUid );
        } );

    return true; // success
}


bool SysUtil::haveXdgOpenCommand()
{
    static bool reported = false;
    bool result = haveCommand( OPEN_CMD );

    if ( ! result && ! reported )
    {
        logWarning() << "Command not found: " << OPEN_CMD << endl;
        logWarning() << "Install package 'xdg-utils'." << endl;
        reported = true;
    }

    return result;
}


bool SysUtil::haveCommand( const QString & command )
{
    return access( command.toUtf8(), X_OK ) == 0;
}


bool SysUtil::runningAsRoot()
{
    return geteuid() == 0;
}


bool SysUtil::runningWithSudo()
{
    return ! qgetenv( "SUDO_USER" ).isEmpty();
}
