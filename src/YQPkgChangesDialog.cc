/*  ---------------------------------------------------------
               __  __            _
              |  \/  |_   _ _ __| |_   _ _ __
              | |\/| | | | | '__| | | | | '_ \
              | |  | | |_| | |  | | |_| | | | |
              |_|  |_|\__, |_|  |_|\__, |_| |_|
                      |___/        |___/
    ---------------------------------------------------------

    Project:  Myrlyn Package Manager GUI
    Copyright (c) 2024-25 SUSE LLC
    License:  GPL V2 - See file LICENSE for details.

    Textdomain "qt-pkg"
 */


#include <QBoxLayout>
#include <QGuiApplication>
#include <QLabel>
#include <QScreen>
#include <QPushButton>

#include <zypp/Package.h>
#include <zypp/PoolItem.h>
#include <zypp/ResStatus.h>
#include <zypp/ui/Selectable.h>
#include <zypp/ui/UserWantedPackages.h>

#include "Logger.h"
#include "MainWindow.h"
#include "QY2CursorHelper.h"
#include "QY2IconLoader.h"
#include "QY2LayoutUtils.h"
#include "YQPkgList.h"
#include "YQZypp.h"
#include "YQi18n.h"
#include "YQPkgChangesDialog.h"


YQPkgChangesDialog::YQPkgChangesDialog( QWidget *       parent,
                                        const QString & message,
                                        const QString & acceptButtonLabel,
                                        const QString & rejectButtonLabel )
    : QDialog( parent ? parent : MainWindow::instance() )
    , _filter(0)
{
    // Dialog title
    setWindowTitle( _( "Changed Packages" ) );

    // Enable dialog resizing even without window manager
    setSizeGripEnabled( true );

    // Limit dialog size to available screen size
    setMaximumSize( QGuiApplication::primaryScreen()->availableSize() );


    // Layout for the dialog

    QVBoxLayout * layout = new QVBoxLayout();
    Q_CHECK_PTR( layout );
    setLayout(layout);

    QHBoxLayout * hbox = new QHBoxLayout();
    Q_CHECK_PTR( hbox );
    layout->addLayout( hbox );


    // Icon

    QLabel * iconLabel = new QLabel( this );
    Q_CHECK_PTR( iconLabel );
    hbox->addWidget(iconLabel);
    iconLabel->setPixmap( QY2IconLoader::loadIcon( "dialog-information" ).pixmap(64) );
    iconLabel->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum ) ); // hor/vert

    // Label for the message
    QLabel * label = new QLabel( message, this );
    Q_CHECK_PTR( label );
    hbox->addWidget(label);
    label->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum ) ); // hor/vert

    _filter = new QComboBox(this);

    // add the items.
    _filter->addItem( _( "All" ),                  QVariant::fromValue( Filters( FilterAll       ) ) );
    _filter->addItem( _( "Selected by the user" ), QVariant::fromValue( Filters( FilterUser      ) ) );
    _filter->addItem( _( "Automatic Changes" ),    QVariant::fromValue( Filters( FilterAutomatic ) ) );

    _filter->setCurrentIndex(0);

    layout->addWidget( _filter );

    connect( _filter,
             SIGNAL( currentIndexChanged( int ) ),
             SLOT  ( slotFilterChanged  ( int ) ) );


    // Pkg list

    _pkgList = new YQPkgList( this );
    Q_CHECK_PTR( _pkgList );

    _pkgList->setEditable( false );
    layout->addWidget( _pkgList );


    // Button box

    hbox = new QHBoxLayout();
    Q_CHECK_PTR( hbox );

    layout->addLayout( hbox );
    hbox->addStretch();


    // Accept button - usually "OK" or "Continue"

    QPushButton * button = new QPushButton( acceptButtonLabel, this );
    Q_CHECK_PTR( button );

    hbox->addWidget( button );
    button->setDefault( true );

    connect( button, SIGNAL( clicked() ),
             this,   SLOT  ( accept()  ) );

    hbox->addStretch();

    if ( ! rejectButtonLabel.isEmpty() )
    {
        // Reject button (if desired) - usually "Cancel"

        button = new QPushButton( rejectButtonLabel, this );
        Q_CHECK_PTR( button );

        hbox->addWidget(button);

        connect( button, SIGNAL( clicked() ),
                 this,   SLOT  ( reject()  ) );

        hbox->addStretch();
    }
}


void
YQPkgChangesDialog::filter( Filters flt )
{
    filter( QRegularExpression( "" ), flt );
}


void
YQPkgChangesDialog::slotFilterChanged( int index )
{
    logInfo() << "filter index changed to: " << index << Qt::endl;
    QVariant var = _filter->itemData( index );

    if ( var.isValid() && var.canConvert<Filters>() )
    {
        Filters flt = var.value<Filters>();
        filter( flt );
    }
    else
    {
        logError() << "Can't find filter for index " << index << Qt::endl;
    }

}


void
YQPkgChangesDialog::setFilter( Filters filters )
{
    setFilter( QRegularExpression( "" ), filters );
}


void
YQPkgChangesDialog::setFilter( const QRegularExpression & regexp, Filters flt )
{
    logInfo() << "filter changed to: " << flt << Qt::endl;

    int index = -1;

    for ( int i = 0; i < _filter->count(); ++i )
    {
        QVariant var = _filter->itemData( i );

        if ( var.isValid() && var.canConvert<Filters>() )
        {

            Filters setf = var.value<Filters>();
            if ( setf == flt )
                index = i;
        }
    }

    if ( index != -1 )
    {
        // so we dont get called again
        _filter->blockSignals( true );

        // try to set the widget
        _filter->setCurrentIndex( flt );
        _filter->blockSignals( false );
        filter( regexp, flt );
    }
    else
    {
        logError() << "Can't find index for filter " << flt << Qt::endl;
    }
}


void
YQPkgChangesDialog::filter( const QRegularExpression & regexp, Filters flt )
{
    busyCursor();
    _pkgList->clear();

    bool byAuto = flt.testFlag( FilterAutomatic );
    bool byUser = flt.testFlag( FilterUser      );
    bool byApp  = flt.testFlag( FilterUser      );

    std::set<std::string> ignoredNames;

    if ( ! byUser || ! byApp )
        ignoredNames = zypp::ui::userWantedPackageNames();

    for ( ZyppPoolIterator it = zyppPkgBegin();
          it != zyppPkgEnd();
          ++it )
    {
        ZyppSel selectable = *it;

        if ( selectable->toModify() )
        {
            zypp::ResStatus::TransactByValue modifiedBy = selectable->modifiedBy();

            if ( ( ( modifiedBy == zypp::ResStatus::SOLVER     ) && byAuto ) ||
                 ( ( modifiedBy == zypp::ResStatus::APPL_LOW ||
                     modifiedBy == zypp::ResStatus::APPL_HIGH  ) && byApp ) ||
                 ( ( modifiedBy == zypp::ResStatus::USER       ) && byUser )  )
            {
                if ( regexp.match( selectable->name().c_str() ).hasMatch() )
                {
                    if ( ! contains( ignoredNames, selectable->name() ) )
                    {
                        ZyppPkg pkg = tryCastToZyppPkg( selectable->theObj() );

                        if ( extraFilter( selectable, pkg ) )
                            _pkgList->addPkgItem( selectable, pkg );
                    }
                }
            }
        }
    }

    normalCursor();
}


bool
YQPkgChangesDialog::extraFilter( ZyppSel sel, ZyppPkg pkg )
{
    return true;
}


bool
YQPkgChangesDialog::isEmpty() const
{
    return _pkgList->topLevelItemCount() == 0;
}


QSize
YQPkgChangesDialog::sizeHint() const
{
    return limitToScreenSize( this, QDialog::sizeHint() );
}


bool
YQPkgChangesDialog::showChangesDialog( QWidget *        parent,
                                       const QString &  message,
                                       const QString &  acceptButtonLabel,
                                       const QString &  rejectButtonLabel,
                                       Filters          flt,
                                       Options          options )
{
    YQPkgChangesDialog dialog( parent,
                               message,
                               acceptButtonLabel,
                               rejectButtonLabel );

    dialog.setFilter( flt );

    if ( dialog.isEmpty() && options.testFlag( OptionAutoAcceptIfEmpty ) )
    {
        logInfo() << "No items to show in changes dialog, accepting it automatically" << Qt::endl;
        return true;
    }


    dialog.exec();

    return dialog.result() == QDialog::Accepted;
}


bool
YQPkgChangesDialog::showChangesDialog( QWidget *                  parent,
                                       const QString &            message,
                                       const QRegularExpression & regexp,
                                       const QString &            acceptButtonLabel,
                                       const QString &            rejectButtonLabel,
                                       Filters                    flt,
                                       Options                    options )
{
    YQPkgChangesDialog dialog( parent,
                               message,
                               acceptButtonLabel,
                               rejectButtonLabel );
    dialog.setFilter( regexp, flt );

    if ( dialog.isEmpty() &&  options.testFlag(OptionAutoAcceptIfEmpty) )
    {
        logInfo() << "No items to show in dialog, accepting it automatically" << Qt::endl;
        return true;
    }

    dialog.exec();

    return dialog.result() == QDialog::Accepted;
}


YQPkgUnsupportedPackagesDialog::YQPkgUnsupportedPackagesDialog( QWidget *       parent,
                                                                const QString & message,
                                                                const QString & acceptButtonLabel,
                                                                const QString & rejectButtonLabel )
    : YQPkgChangesDialog( parent,
                          message,
                          acceptButtonLabel,
                          rejectButtonLabel )
{
}


bool
YQPkgUnsupportedPackagesDialog::extraFilter( ZyppSel sel, ZyppPkg pkg )
{
    if (!pkg || !sel)
        return false;

    return pkg->maybeUnsupported() && sel->toInstall();
}


bool
YQPkgUnsupportedPackagesDialog::showUnsupportedPackagesDialog( QWidget *       parent,
                                                               const QString & message,
                                                               const QString & acceptButtonLabel,
                                                               const QString & rejectButtonLabel,
                                                               Filters         flt,
                                                               Options         options )
{
    YQPkgUnsupportedPackagesDialog dialog( parent,
                                           message,
                                           acceptButtonLabel,
                                           rejectButtonLabel );

    dialog.setFilter( flt );

    if ( dialog.isEmpty() && options.testFlag( OptionAutoAcceptIfEmpty ) )
    {
        logInfo() << "No items to show in unsupported packages dialog, accepting it automatically" << Qt::endl;
        return true;
    }

    dialog.exec();

    return dialog.result() == QDialog::Accepted;
}
