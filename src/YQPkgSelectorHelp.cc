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


#include <QString>

#include "Logger.h"
#include "utf8.h"
#include "ui_about-dialog.h"
#include "SysUtil.h"
#include "YQPkgSelector.h"


#define DEBUG_HELP 0

#if DEBUG_HELP
#  define HELP_BASE "https://github.com/shundhammer/myrlyn/blob/huha-help/help/"
#else
#  define HELP_BASE "https://github.com/shundhammer/myrlyn/blob/master/help/"
#endif

using SysUtil::openInBrowser;


void YQPkgSelector::myrlynHomePage() const
{
    openInBrowser( "https://github.com/shundhammer/myrlyn/" );
}


void YQPkgSelector::helpLatestRelease() const
{
    openInBrowser( "https://github.com/shundhammer/myrlyn/releases" );
}


void YQPkgSelector::helpCenter() const
{
    openInBrowser( HELP_BASE "help-center.md" );
}


void YQPkgSelector::helpFontSize() const
{
    openInBrowser( HELP_BASE "font-size.md" );
}


void YQPkgSelector::helpGuiScaling() const
{
    openInBrowser( HELP_BASE "gui-scaling.md" );
}


void YQPkgSelector::helpDarkTheme() const
{
    openInBrowser( HELP_BASE "dark-theme.md" );
}


void YQPkgSelector::helpRepoConfig() const
{
    openInBrowser( "https://github.com/shundhammer/myrlyn/issues/69" );
}


void YQPkgSelector::helpRootAuth() const
{
    openInBrowser( "https://github.com/shundhammer/myrlyn/issues/99" );
}


void YQPkgSelector::helpDevelopmentStatus() const
{
    openInBrowser( "https://github.com/shundhammer/myrlyn/issues/1" );
}


void YQPkgSelector::about()
{
    QDialog aboutDialog( this );
    Ui::AboutDialog * ui( new Ui::AboutDialog );
    ui->setupUi( &aboutDialog );

    QString text = ui->heading->text();
    text += QString( " %1" ).arg( VERSION );
    ui->heading->setText( text );

    QStringList contributors = QStringList() << "\n"  // The "\n" is needed for the join()
        << fromUTF8( "@gruenich (Christoph Grüninger)" )
        << "@Arthurium-On-Linux"
        << "@byDMA"
        << "@slothsnow"
        << "@Sauerlandlinux (Stephan Hemeier)"
        ;
    ui->contributorsText->setText( contributors.join( "\n- " ) );;

    aboutDialog.exec();
    delete ui;
}

