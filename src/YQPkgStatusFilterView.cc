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


#include <QSettings>

#include "Exception.h"
#include "Logger.h"
#include "YQIconPool.h"
#include "YQPkgStatusFilterView.h"

#ifndef VERBOSE_FILTER_VIEWS
#  define VERBOSE_FILTER_VIEWS  0
#endif


YQPkgStatusFilterView::YQPkgStatusFilterView( QWidget * parent )
    : QWidget( parent )
    , _ui( new Ui::StatusFilterView )  // Use the Qt designer .ui form (XML)
{
    CHECK_NEW( _ui );
    _ui->setupUi( this ); // Actually create the widgets from the .ui form
    fixupIcons();

    // See ui_status-filter-view.h in the build/ tree for the widget names.
    //
    // That header is generated by Qt's uic (user interface compiler)
    // from the XML .ui file created with Qt designer.
    //
    // Take care in Qt designer to give each widget a meaningful name in the
    // widget tree at the top right: They are also the member variable names
    // for the _ui object.

    // Read the settings before connecting the widgets to prevent firing
    // signals when each checkbox is set to its initial value
    readSettings();
    connectWidgets();
}


YQPkgStatusFilterView::~YQPkgStatusFilterView()
{
    writeSettings();
    delete _ui;
}


void YQPkgStatusFilterView::connectWidgets()
{
    // Use Qt introspection to recursively find all the QCheckBox children of
    // this filter view and bulk-connect them all to the same slot
    const QList<QCheckBox *> & children = findChildren<QCheckBox *>();

    for ( QCheckBox * checkBox: children )
    {
        connect( checkBox,  SIGNAL( clicked() ),
                 this,      SLOT  ( filter()  ) );
    }


    // Connect the "Refresh List" QPushButton

    connect( _ui->refreshButton, SIGNAL( clicked() ),
             this,               SLOT  ( filter()  ) );
}


void
YQPkgStatusFilterView::fixupIcons()
{
    // Replace the resource icons from the .ui file with dynamically loaded
    // icons from the desktop theme (issue #63)

    _ui->iconInstall->setPixmap      ( YQIconPool::pkgInstall()       );
    _ui->iconUpdate->setPixmap       ( YQIconPool::pkgUpdate()        );
    _ui->iconDel->setPixmap          ( YQIconPool::pkgDel()           );
    _ui->iconAutoInstall->setPixmap  ( YQIconPool::pkgAutoInstall()   );
    _ui->iconAutoUpdate->setPixmap   ( YQIconPool::pkgAutoUpdate()    );
    _ui->iconAutoDel->setPixmap      ( YQIconPool::pkgAutoDel()       );
    _ui->iconTaboo->setPixmap        ( YQIconPool::pkgTaboo()         );
    _ui->iconProtected->setPixmap    ( YQIconPool::pkgProtected()     );
    _ui->iconKeepInstalled->setPixmap( YQIconPool::pkgKeepInstalled() );
    _ui->iconNoInst->setPixmap       ( YQIconPool::pkgNoInst()        );
}


void
YQPkgStatusFilterView::showFilter( QWidget * newFilter )
{
    if ( newFilter == this )
        filter();
}


void
YQPkgStatusFilterView::filter()
{
#if VERBOSE_FILTER_VIEWS
    logVerbose() << "Filtering" << endl;
#endif

    emit filterStart();

    for ( ZyppPoolIterator it = zyppPkgBegin();
          it != zyppPkgEnd();
          ++it )
    {
        ZyppSel selectable = *it;

        bool match =
            check( selectable, selectable->candidateObj() ) ||
            check( selectable, selectable->installedObj() );

        // If there is neither an installed nor a candidate package, check
        // any other instance.

        if ( ! match                      &&
             ! selectable->candidateObj() &&
             ! selectable->installedObj()   )
            check( selectable,  selectable->theObj() );
    }

    emit filterFinished();
}


bool
YQPkgStatusFilterView::check( ZyppSel selectable,
                              ZyppObj zyppObj )
{
    bool match = false;

    if ( ! zyppObj )
        return false;

    switch ( selectable->status() )
    {
        case S_Install:       match = _ui->showInstall->isChecked();       break;
        case S_Update:        match = _ui->showUpdate->isChecked();        break;
        case S_Del:           match = _ui->showDel->isChecked();           break;
        case S_AutoInstall:   match = _ui->showAutoInstall->isChecked();   break;
        case S_AutoUpdate:    match = _ui->showAutoUpdate->isChecked();    break;
        case S_AutoDel:       match = _ui->showAutoDel->isChecked();       break;
        case S_Protected:     match = _ui->showProtected->isChecked();     break;
        case S_Taboo:         match = _ui->showTaboo->isChecked();         break;
        case S_KeepInstalled: match = _ui->showKeepInstalled->isChecked(); break;
        case S_NoInst:        match = _ui->showNoInst->isChecked();        break;

            // Intentionally omitting 'default' branch so the compiler can
            // catch unhandled enum states
    }

    if ( match )
    {
        ZyppPkg zyppPkg = tryCastToZyppPkg( zyppObj );

        if ( zyppPkg )
            emit filterMatch( selectable, zyppPkg );
    }

    return match;
}



void YQPkgStatusFilterView::clear()
{
    readSettings();
}


bool
YQPkgStatusFilterView::showingAutomaticChanges() const
{
    // Showing all installed packages will drown out the automatic changes, so
    // let's be conservative here: Better show an additional popup that ONLY
    // shows the automatic changes in that case.

    if ( _ui->showKeepInstalled->isChecked() )
        return false;

    return _ui->showAutoInstall->isChecked() &&
        _ui->showAutoUpdate->isChecked()     &&
        _ui->showAutoDel->isChecked();
}


void YQPkgStatusFilterView::showTransactions()
{
    _ui->showInstall->setChecked( true );
    _ui->showUpdate->setChecked( true );
    _ui->showDel->setChecked( true );
    _ui->showAutoInstall->setChecked( true );
    _ui->showAutoUpdate->setChecked( true );
    _ui->showAutoDel->setChecked( true );
}


void YQPkgStatusFilterView::readSettings()
{
    QSettings settings;
    settings.beginGroup( "PkgStatusFilterView" );

    _ui->showInstall->setChecked      ( settings.value( "showInstall",       true  ).toBool() );
    _ui->showUpdate->setChecked       ( settings.value( "showUpdate",        true  ).toBool() );
    _ui->showDel->setChecked          ( settings.value( "showDel",           true  ).toBool() );
    _ui->showAutoInstall->setChecked  ( settings.value( "showAutoInstall",   true  ).toBool() );
    _ui->showAutoUpdate->setChecked   ( settings.value( "showAutoUpdate",    true  ).toBool() );
    _ui->showAutoDel->setChecked      ( settings.value( "showAutoDel",       true  ).toBool() );
    _ui->showTaboo->setChecked        ( settings.value( "showTaboo",         false ).toBool() );
    _ui->showProtected->setChecked    ( settings.value( "showProtected",     false ).toBool() );
    _ui->showKeepInstalled->setChecked( settings.value( "showKeepInstalled", false ).toBool() );
    _ui->showNoInst->setChecked       ( settings.value( "showNoInst",        false ).toBool() );

    settings.endGroup();
}


void YQPkgStatusFilterView::writeSettings()
{
    QSettings settings;
    settings.beginGroup( "PkgStatusFilterView" );

    settings.setValue( "showInstall",       _ui->showInstall->isChecked()       );
    settings.setValue( "showUpdate",        _ui->showUpdate->isChecked()        );
    settings.setValue( "showDel",           _ui->showDel->isChecked()           );
    settings.setValue( "showAutoInstall",   _ui->showAutoInstall->isChecked()   );
    settings.setValue( "showAutoUpdate",    _ui->showAutoUpdate->isChecked()    );
    settings.setValue( "showAutoDel",       _ui->showAutoDel->isChecked()       );
    settings.setValue( "showTaboo",         _ui->showTaboo->isChecked()         );
    settings.setValue( "showProtected",     _ui->showProtected->isChecked()     );
    settings.setValue( "showKeepInstalled", _ui->showKeepInstalled->isChecked() );
    settings.setValue( "showNoInst",        _ui->showNoInst->isChecked()        );

    settings.endGroup();
}
