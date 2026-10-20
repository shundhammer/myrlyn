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

 */


#include <QButtonGroup>
#include <QGridLayout>
#include <QGuiApplication>
#include <QLabel>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QStyleOptionButton>
#include <QStylePainter>
#include <QTabWidget>
#include <QVBoxLayout>

#include <zypp/Arch.h>
#include <zypp/Edition.h>
#include <zypp/IdStringType.h>
#include <zypp/PoolItem.h>
#include <zypp/RepoInfo.h>
#include <zypp/Repository.h>
#include <zypp/ResObject.h>
#include <zypp/Resolvable.h>
#include <zypp/sat/SolvableType.h>
#include <zypp/ui/Status.h>

#include "Logger.h"
#include "MyrlynApp.h"
#include "YQIconPool.h"
#include "YQZypp.h"
#include "YQi18n.h"
#include "utf8.h"
#include "YQPkgVersionsView.h"


#ifndef VERBOSE_DETAILS_VIEWS
#  define VERBOSE_DETAILS_VIEWS  0
#endif


enum TableColumns
{
    VersionCol = 0,
    ArchCol,
    RepoCol,
    VendorCol
};


YQPkgVersionsView::YQPkgVersionsView( QWidget * parent )
    : QScrollArea( parent )
    , _buttonGroup( 0 )
    , _outerVBox( 0 )
{
    _selectable          = 0;
    _isMixedMultiVersion = false;
    setFrameStyle( QFrame::NoFrame );

    _parentTab = dynamic_cast<QTabWidget *>( parent );

    if ( _parentTab )
    {
        connect( _parentTab, SIGNAL( currentChanged( int ) ),
                 this,       SLOT  ( reload        ( int ) ) );
    }
}


YQPkgVersionsView::~YQPkgVersionsView()
{
    // NOP
}


void
YQPkgVersionsView::reload( int newCurrent )
{
    if ( _parentTab && _parentTab->widget( newCurrent ) == this )
        showDetailsIfVisible( _selectable );
}


void
YQPkgVersionsView::showDetailsIfVisible( ZyppSel selectable )
{
    _selectable = selectable;
    _isMixedMultiVersion = isMixedMultiVersion( selectable );

    if ( _parentTab )   // Is this view embedded into a tab widget?
    {
        if ( _parentTab->currentWidget() == this )  // Is this page the topmost?
        {
#if VERBOSE_DETAILS_VIEWS

            logVerbose() << ": Showing "
                         << ( selectable ? fromUTF8( selectable->name() ) : "NULL" )
                         << endl;
#endif
            showDetails( selectable );
        }
    }
    else                // No tab parent - simply show data unconditionally.
    {
        showDetails( selectable );
    }
}


void
YQPkgVersionsView::showDetails( ZyppSel selectable )
{
    _selectable          = selectable;
    _isMixedMultiVersion = isMixedMultiVersion( selectable );

    rebuildWidgets();
}


void
YQPkgVersionsView::rebuildWidgets()
{
    QWidget * content = widget();

    delete content; // This recursively deletes all child widgets and qobjects

    content      = new QWidget( this );
    _buttonGroup = new QButtonGroup( content );
    _outerVBox   = new QVBoxLayout( content );
    content->setLayout( _outerVBox );

    if ( ! _selectable )
    {
        setWidget( content );   // Prevent mem leak
        return;
    }

    addPkgNameHeading();
    _outerVBox->addSpacing( 13 );

    addVersionsTable();
    _outerVBox->addStretch();

    // This really needs to wait until this point, or the content will never
    // get its correct size and become visible.

    setWidget( content );
    content->show();
}


void
YQPkgVersionsView::addPkgNameHeading()
{
    if ( ! _selectable || ! _selectable->theObj() )
        return;

    QString  pkgName      = fromUTF8( _selectable->theObj()->name().c_str() );
    QLabel * pkgNameLabel = new QLabel( pkgName, this );

    _outerVBox->addWidget( pkgNameLabel );
    pkgNameLabel->setFont( MyrlynApp::headingFont() );
}


void
YQPkgVersionsView::addVersionsTable()
{
    QHBoxLayout * gridHBox = new QHBoxLayout();
    _outerVBox->addLayout( gridHBox );

    _grid = new QGridLayout();
    _grid->setHorizontalSpacing( 20 );
    _grid->setVerticalSpacing  ( 12 );
    gridHBox->addLayout( _grid );

    _gridRow = 0;
    addTableHeader();


    // Table content

    if ( _selectable->multiversionInstall() ) // at least one (!) PoolItem is multiversion?
    {
        addMultiVersionItems(); // Multi-select: A QCheckBox for each item
    }
    else
    {
        addNormalItems();       // Single-select: A QRadioBox for each item
    }


    // No stretching for all table rows and columns

    for ( int col=0; col < _grid->columnCount(); col++ )
        _grid->setColumnStretch( col, 0 );

    for ( int row=0; row < _grid->rowCount(); row++ )
        _grid->setRowStretch( row, 0 );


    // Add a HStretch to the right of the table to take up all excess width

    gridHBox->addStretch( 1 ); // stretchFactor
}


void
YQPkgVersionsView::addTableHeader()
{
    QStringList tableHeaders;
    tableHeaders << QString( "     %1" ).arg( _( "Version"      ) )
                 << _( "Architecture" )
                 << _( "Repository"   )
                 << _( "Vendor"       );

    int col = 0;

    for ( const QString & headerText: tableHeaders )
    {
        QLabel * headerLabel = new QLabel( headerText, this );
        headerLabel->setFont( MyrlynApp::boldFont() );

        _grid->addWidget( headerLabel, _gridRow, col++ );
    }
}


void
YQPkgVersionsView::addNormalItems()
{
    addNormalInstalledItems();
    addNormalAvailableItems();
}


void
YQPkgVersionsView::addNormalInstalledItems()
{
    //
    // Fill installed objects
    //

    for ( zypp::ui::Selectable::installed_iterator it = _selectable->installedBegin();
          it != _selectable->installedEnd();
          ++it )
    {
        QHBoxLayout * instHBox = new QHBoxLayout();
        instHBox->setSpacing( 2 );

        QLabel * iconLabel = new QLabel( this );
        iconLabel->setPixmap( YQIconPool::checkmark().pixmap( QSize( 16, 16 ) ) );
        iconLabel->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
        instHBox->addWidget( iconLabel );

        QString versionText = fromUTF8( (*it)->edition().asString().c_str() );
        bool retracted      = installedIsRetracted( _selectable, *it );

        if ( retracted )
            versionText += QString( " [%1]" ).arg( _( "[RETRACTED]" ) );

        QLabel * versionLabel = new QLabel( versionText, this );
        instHBox->addWidget( versionLabel );

        if ( retracted )
            setRetractedColor( versionLabel );

        _grid->addLayout( instHBox, ++_gridRow, VersionCol );
        addStaticColumns( *it, true );  // isInstalled
    }
}


void
YQPkgVersionsView::addNormalAvailableItems()
{
    //
    // Fill normal available objects (as a QRadioButton each)
    //

    for ( zypp::ui::Selectable::available_iterator it = _selectable->availableBegin();
          it != _selectable->availableEnd();
          ++it)
    {
        YQPkgVersion * radioButton = new YQPkgVersion( this, _selectable, *it );
        _grid->addWidget( radioButton, ++_gridRow, VersionCol );
        _buttonGroup->addButton( radioButton );
        addStaticColumns( *it );

        connect( radioButton, SIGNAL( clicked( bool )            ),
                 this,        SLOT  ( checkForChangedCandidate() ) );


        // Select the candidate version

        if ( ! _buttonGroup->checkedButton() &&
             _selectable->hasCandidateObj()  &&
             _selectable->candidateObj()->edition() == (*it)->edition() &&
             _selectable->candidateObj()->arch()    == (*it)->arch() )
        {
            radioButton->setChecked( true );
        }
    }
}


void
YQPkgVersionsView::addMultiVersionItems()
{
    //
    // Add installed and available objects for multiversion view as QCheckBox widgets.
    // Unlike for the normal items, installed versions don't get any special treatment:
    // They are added as the same kind of QCheckBox each, but checked.
    // The user can uncheck each one individually.
    //

    for ( zypp::ui::Selectable::picklist_iterator it = _selectable->picklistBegin();
          it != _selectable->picklistEnd();
          ++it )
    {
        YQPkgMultiVersion * version = new YQPkgMultiVersion( this, _selectable, *it );
        _grid->addWidget( version, ++_gridRow, VersionCol );
        addStaticColumns( *it );

        connect( version, SIGNAL( statusChanged() ),
                 this,    SIGNAL( statusChanged() ) );

        connect( this,    SIGNAL( statusChanged() ),
                 version, SLOT  ( update()        ) );
    }
}


void
YQPkgVersionsView::addStaticColumns( ZyppPoolItem item, bool isInstalled )
{
    addGridText( ArchCol, item->arch().asString() );

    if ( isInstalled )
    {
        QLabel * label = addGridText( RepoCol, _( "(installed)" ) );

        QFont font = label->font();
        font.setItalic( true );
        label->setFont( font );
    }
    else
    {
        addGridText( RepoCol,   item->repository().info().name() );
    }

    QString vendor = fromUTF8( item->vendor() );

    if ( vendor.startsWith( "VideoLAN Project (", Qt::CaseInsensitive ) )
        vendor = "VideoLAN"; // cut off cruft

    if ( vendor.contains( "Packman", Qt::CaseInsensitive ) )
        vendor = "Packman";

    if ( vendor == "NVIDIA" ||
         ( vendor.startsWith( "obs", Qt::CaseInsensitive ) &&
           vendor.contains( "Proprietary:X11:Drivers", Qt::CaseInsensitive ) ) )
        vendor = "NVidia";

    addGridText( VendorCol, vendor );
}


QLabel *
YQPkgVersionsView::addGridText( int col, const std::string & text )
{
    return addGridText( col, fromUTF8( text ) );
}


QLabel *
YQPkgVersionsView::addGridText( int col, const QString & text )
{
    QLabel * label = new QLabel( text, this );
    _grid->addWidget( label, _gridRow, col );

    return label;
}


void YQPkgVersionsView::setRetractedColor( QWidget * widget )
{
    QPalette pal = widget->palette();
    pal.setColor( QPalette::WindowText, Qt::red );
    widget->setPalette( pal );
}


bool YQPkgVersionsView::installedIsRetracted( ZyppSel selectable, ZyppObj installed )
{
    for ( zypp::ui::Selectable::available_iterator it = selectable->availableBegin();
          it != selectable->availableEnd();
          ++it )
    {
        if ( (*it)->isRetracted() )
        {
            if ( installed->edition() == (*it)->edition() &&
                 installed->arch()    == (*it)->arch()    &&
                 installed->vendor()  == (*it)->vendor()    )
            {
                return true;
            }
        }
    }

    return false;
}


void
YQPkgVersionsView::checkForChangedCandidate()
{
    QListIterator<QAbstractButton*> it( _buttonGroup->buttons() );

    while ( it.hasNext() )
    {
        YQPkgVersion * versionItem = dynamic_cast<YQPkgVersion *> (it.next());

        if ( versionItem && versionItem->isChecked() )
        {
            ZyppObj newCandidate = versionItem->zyppObj();

            if ( _selectable && *newCandidate != _selectable->candidateObj() )
            {
                logInfo() << "Candidate changed" << endl;

                // Change status of selectable

                ZyppStatus status = _selectable->status();

                if ( ! _selectable->installedEmpty() &&
                     _selectable->installedObj()->arch()    == newCandidate->arch() &&
                     _selectable->installedObj()->edition() == newCandidate->edition() )
                {
                    // Switch back to the original instance -
                    // the version that was previously installed
                    status = S_KeepInstalled;
                }
                else
                {
                    switch ( status )
                    {
                        case S_KeepInstalled:
                        case S_Protected:
                        case S_AutoDel:
                        case S_AutoUpdate:
                        case S_Del:
                        case S_Update:

                            status = S_Update;
                            break;

                        case S_NoInst:
                        case S_Taboo:
                        case S_Install:
                        case S_AutoInstall:
                            status = S_Install;
                            break;
                    }
                }

                _selectable->setStatus( status );


                // Set candidate

                _selectable->setCandidate( newCandidate );
                emit candidateChanged( newCandidate );
                return;
            }
        }
    }
}


QSize
YQPkgVersionsView::minimumSizeHint() const
{
    return QSize( 0, 0 );
}


bool
YQPkgVersionsView::handleMixedMultiVersion( YQPkgMultiVersion * newSelected )
{
    ZyppPoolItem poolItem = newSelected->zyppPoolItem();
    Q_CHECK_PTR( poolItem );

    bool multiVersion = poolItem->multiversionInstall();

    logInfo() << "Selected: "
              << ( multiVersion ? "Multiversion " : "Non-Multiversion " )
              << newSelected->text()
              << endl;

    if ( anyMultiVersionToInstall( !multiVersion ) )
    {
        logInfo() << "Multiversion and non-multiversion conflict!" << endl;
        bool forceContinue = mixedMultiVersionPopup( multiVersion );

        if ( forceContinue )
        {
            _selectable->setPickStatus( poolItem, S_Install );
            emit statusChanged(); // update status icons for all versions
        }
        else
        {
            // Nothing to do here: The status of this item was not changed yet;
            // simply leave it like it was.
        }

        return true; // handled here
    }
    else
    {
        return false; // Not handled here
    }
}


bool
YQPkgVersionsView::mixedMultiVersionPopup( bool multiversion ) const
{
    // Translators: Popup dialog text. Try to keep the lines about the same length.
    QString msg = _( "You are trying to install multiversion-capable\n"
                     "and non-multiversion-capable versions of this\n"
                     "package at the same time." );
    msg += "\n\n";

    if ( multiversion )
    {
        msg +=
            _( "This version is multiversion-capable.\n"
               "\n"
               "Press \"Continue\" to install this version\n"
               "and unselect the non-multiversion-capable version,\n"
               "\"Cancel\" to unselect this version and keep the other one." );
    }
    else
    {
        msg +=
            _( "This version is not multiversion-capable.\n"
               "\n"
               "Press \"Continue\" to install only this version\n"
               "and unselect all other versions,\n"
               "\"Cancel\" to unselect this version and keep the other ones." );
    }

    QMessageBox msgBox( window() );
    msgBox.setText( msg );
    msgBox.setWindowTitle( _( "Incompatible Package Versions" ) );
    msgBox.setIcon( QMessageBox::Question );
    msgBox.addButton( _( "C&ontinue" ), QMessageBox::AcceptRole );
    msgBox.addButton( QMessageBox::Cancel );
    msgBox.setDefaultButton( QMessageBox::Cancel );
    msgBox.exec();

    logInfo() << "User hit "
              <<  ( msgBox.result() == QMessageBox::Cancel ?
                    "[Cancel]" : "[Continue]" )
              << endl;

    return msgBox.result() != QMessageBox::Cancel;
}



bool
YQPkgVersionsView::anyMultiVersionToInstall( bool multiversion ) const
{
    if ( ! _selectable )
        return false;

    for ( zypp::ui::Selectable::available_iterator it = _selectable->availableBegin();
          it != _selectable->availableEnd();
          ++it )
    {
        if ( it->multiversionInstall() == multiversion )
        {
            switch ( _selectable->pickStatus( *it ) )
            {
                case S_Install:
                case S_AutoInstall:
                    logInfo() << "Found " << ( multiversion ? "multiversion" : "non-multiversion" )
                              << " to install" << endl;
                    return true;

                default:
                    break;
            }
        }
    }

    logInfo() << "No " << ( multiversion ? "multiversion" : "non-multiversion" )
              << " to install" << endl;
    return false;
}


void
YQPkgVersionsView::unselectAllMultiVersion()
{
    if ( ! _selectable )
        return;

    for ( zypp::ui::Selectable::available_iterator it = _selectable->availableBegin();
          it != _selectable->availableEnd();
          ++it )
    {
        if ( it->multiversionInstall() )
        {
            switch ( _selectable->pickStatus( *it ) )
            {
                case S_Install:
                case S_AutoInstall:
                    _selectable->setPickStatus( *it, S_NoInst );
                    break;

                default:
                    break;
            }
        }
    }
}


bool
YQPkgVersionsView::isMixedMultiVersion( ZyppSel selectable )
{
    if ( ! selectable )
        return false;

    zypp::ui::Selectable::available_iterator it = selectable->availableBegin();

    if ( it == selectable->availableEnd() )
        return false;

    bool multiversion = it->multiversionInstall();

    while ( it != selectable->availableEnd() )
    {
        if ( it->multiversionInstall() != multiversion )
        {
            logInfo() << "Mixed multiversion" << endl;
            return true;
        }

        ++it;
    }

    return false;
}






YQPkgVersion::YQPkgVersion( QWidget * parent,
                            ZyppSel   selectable,
                            ZyppObj   zyppObj )
    : QRadioButton( parent )
    , _selectable( selectable )
    , _zyppObj( zyppObj )
{
    QString versionText = fromUTF8( zyppObj->edition().asString() );

    if ( zyppObj->isRetracted() )
    {
        versionText += QString( " [%1]" ).arg( _( "[RETRACTED]" ) );
        YQPkgVersionsView::setRetractedColor( this );
    }

    setText( versionText );
}


YQPkgVersion::~YQPkgVersion()
{
    // NOP
}




YQPkgMultiVersion::YQPkgMultiVersion( YQPkgVersionsView * parent,
                                      ZyppSel             selectable,
                                      ZyppPoolItem        zyppPoolItem )
    : QCheckBox( parent )
    , _parent( parent )
    , _selectable( selectable )
    , _zyppPoolItem( zyppPoolItem )
{
    setText( fromUTF8( zyppPoolItem->edition().asString() ) );

    connect( this, SIGNAL( toggled( bool)    ),
             this, SLOT  ( slotIconClicked() ) );
}


YQPkgMultiVersion::~YQPkgMultiVersion()
{
    // NOP
}


void YQPkgMultiVersion::slotIconClicked()
{
    {
        QSignalBlocker sigBlocker( this );
        setChecked( false );
    }

    cycleStatus();
}


void YQPkgMultiVersion::cycleStatus()
{
    ZyppStatus oldStatus = _selectable->pickStatus( _zyppPoolItem );
    ZyppStatus newStatus = oldStatus;

    switch ( oldStatus )
    {
        case S_Install:
        case S_AutoInstall:
        case S_Protected:
            newStatus = S_NoInst;
            break;

        case S_KeepInstalled:
        case S_Update:
        case S_AutoUpdate:
            newStatus = S_Del;
            break;


        case S_Del:
        case S_AutoDel:
            newStatus = S_KeepInstalled;
            break;

        case S_NoInst:
        case S_Taboo:
            newStatus = S_Install;
            break;
    }

    bool handled = false;

    if ( _parent->isMixedMultiVersion() &&
         newStatus == S_Install &&
         oldStatus != newStatus )
    {
        handled = _parent->handleMixedMultiVersion( this );
    }

    if ( ! handled )
        setStatus( newStatus );

    logInfo() << "oldStatus: " << oldStatus << endl;
    ZyppStatus actualStatus = _selectable->pickStatus( _zyppPoolItem );

    if ( actualStatus != newStatus )
        logWarning() << "FAILED to set new status: " << newStatus
                     << "  actual Status: " << actualStatus << endl;
    else
        logInfo() << "newStatus:" << newStatus << endl;

    if ( oldStatus != actualStatus )
    {
        update();
        emit statusChanged();
    }
}


void YQPkgMultiVersion::setStatus( ZyppStatus newStatus )
{
    logInfo() << "Setting pick status to " << newStatus << endl;
    _selectable->setPickStatus( _zyppPoolItem, newStatus );
}


void YQPkgMultiVersion::paintEvent( QPaintEvent * )
{
    // Draw the usual checkbox

    QStylePainter painter( this );
    QStyleOptionButton opt;
    initStyleOption( &opt );
    painter.drawControl( QStyle::CE_CheckBox, opt );


    // Calculate the position and draw the status icon

    QRect   elementRect = style()->subElementRect ( QStyle::SE_CheckBoxIndicator, &opt );
    QIcon   icon        = statusIcon( _selectable->pickStatus( _zyppPoolItem ) );
    QPixmap pixmap      = icon.pixmap( QSize( 16, 16 ) );

    QPoint  start = elementRect.center() - pixmap.rect().center();
    QRect   rect  = QRect( start.x(), start.y(), pixmap.width(), pixmap.height() );

    painter.drawItemPixmap( rect, 0, pixmap );
}


QIcon
YQPkgMultiVersion::statusIcon( ZyppStatus status )
{
    QIcon icon = YQIconPool::pkgNoInst();

    switch ( status )
    {
        case S_KeepInstalled: icon = YQIconPool::pkgKeepInstalled();  break;
        case S_Install:       icon = YQIconPool::pkgInstall();        break;
        case S_Update:        icon = YQIconPool::pkgUpdate();         break;
        case S_Del:           icon = YQIconPool::pkgDel();            break;
        case S_NoInst:        icon = QIcon();                         break;
        case S_Protected:     icon = YQIconPool::pkgProtected();      break;
        case S_Taboo:         icon = YQIconPool::pkgTaboo();          break;

        case S_AutoInstall:   icon = YQIconPool::pkgAutoInstall();    break;
        case S_AutoUpdate:    icon = YQIconPool::pkgAutoUpdate();     break;
        case S_AutoDel:       icon = YQIconPool::pkgAutoDel();        break;

            // Intentionally omitting 'default' branch so the compiler can
            // catch unhandled enum states
    }

    return icon;
}

