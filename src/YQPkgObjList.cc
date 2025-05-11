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


#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMenu>
#include <QPixmap>

#include <zypp/ZYppFactory.h>

#include "LicenseCache.h"
#include "Logger.h"
#include "QY2CursorHelper.h"
#include "YQIconPool.h"
#include "YQPkgTextDialog.h"
#include "YQi18n.h"
#include "utf8.h"
#include "YQPkgObjList.h"

#define VERBOSE_EXCLUDE_RULES    0

using std::list;
using std::string;



YQPkgObjList::YQPkgObjList( QWidget * parent )
    : QY2ListView( parent )
    , _editable( true )
    , _installedContextMenu(0)
    , _notInstalledContextMenu(0)
    , actionSetCurrentInstall(0)
    , actionSetCurrentDontInstall(0)
    , actionSetCurrentKeepInstalled(0)
    , actionSetCurrentDelete(0)
    , actionSetCurrentUpdate(0)
    , actionSetCurrentUpdateForce(0)
    , actionSetCurrentTaboo(0)
    , actionSetCurrentProtected(0)
    , actionSetListInstall(0)
    , actionSetListDontInstall(0)
    , actionSetListKeepInstalled(0)
    , actionSetListDelete(0)
    , actionSetListUpdate(0)
    , actionSetListUpdateForce(0)
    , actionSetListTaboo(0)
    , actionSetListProtected(0)
{
    // This class does not add any columns. This is the main reason why this is
    // an abstract base class: It doesn't know which columns are desired and in
    // what order.

    _iconCol            = -42;
    _statusCol          = -42;
    _nameCol            = -42;
    _versionCol         = -42;
    _instVersionCol     = -42;
    _summaryCol         = -42;
    _sizeCol            = -42;

    _excludedItemsCount = 0;

    initColors();
    createActions();

    connect( this,      SIGNAL( columnClicked             ( int, QTreeWidgetItem *, int, const QPoint & ) ),
             this,      SLOT  ( pkgObjClicked             ( int, QTreeWidgetItem *, int, const QPoint & ) ) );

    connect( this,      SIGNAL( columnDoubleClicked       ( int, QTreeWidgetItem *, int, const QPoint & ) ),
             this,      SLOT  ( pkgObjClicked             ( int, QTreeWidgetItem *, int, const QPoint & ) ) );

    connect( this,      SIGNAL( currentItemChanged        ( QTreeWidgetItem *, QTreeWidgetItem * ) ),
             this,      SLOT  ( currentItemChangedInternal( QTreeWidgetItem * ) ) );

    connect( this,      SIGNAL(customContextMenuRequested ( const QPoint & ) ),
             this,      SLOT  (slotCustomContextMenu      ( const QPoint & ) ) );

    setContextMenuPolicy( Qt::CustomContextMenu );
}


YQPkgObjList::~YQPkgObjList()
{
}


void YQPkgObjList::initColors()
{
    _normalTextColor = palette().color( QPalette::Active, QPalette::Text );
    QColor backgroundColor = palette().color( QPalette::Active, QPalette::Base );

    if ( backgroundColor.lightness() >= 128 )  // light Theme: 0 (black) .. 255 (white)
    {
        _blueTextColor = Qt::blue;
        _redTextColor  = Qt::red;
    }
    else  // dark Theme
    {
        _blueTextColor = Qt::cyan;
        _redTextColor  = Qt::red;
    }

}


void
YQPkgObjList::addPkgObjItem( ZyppSel selectable, ZyppObj zyppObj )
{
    if ( ! selectable )
    {
        logError() << "Null zypp::ui::Selectable!" << endl;
        return;
    }

    YQPkgObjListItem * item = new YQPkgObjListItem( this, selectable, zyppObj );
    applyExcludeRules( item );
}


void
YQPkgObjList::addPassiveItem( const QString & name,
                              const QString & summary,
                              FSize           size )
{
    QY2ListViewItem * item = new QY2ListViewItem( this, QString() );

    if ( item )
    {
        if ( nameCol()    >= 0 && ! name.isEmpty()    ) item->setText( nameCol(),       name    );
        if ( summaryCol() >= 0 && ! summary.isEmpty() ) item->setText( summaryCol(),    summary );
        if ( sizeCol()    >= 0 && size > 0L           )
        {
            QString sizeStr = size.form().c_str();
            item->setText( sizeCol(), sizeStr );
        }
    }
}


void
YQPkgObjList::pkgObjClicked( int               button,
                             QTreeWidgetItem * listViewItem,
                             int               col,
                             const QPoint &    pos )
{
    YQPkgObjListItem * item = dynamic_cast<YQPkgObjListItem *> (listViewItem);

    if ( item )
    {
        // logDebug() << "CLICKED: %s", item->zyppObj()->name().c_str()) << endl;

        if ( button == Qt::LeftButton )
        {
            if ( col == statusCol() )
                // || col == nameCol() )
            {
                if ( editable() && item->editable() )
                    item->cycleStatus();
            }
        }
        // context menus are handled in slotCustomContextMenu()
    }
}


void
YQPkgObjList::currentItemChangedInternal( QTreeWidgetItem * listViewItem )
{
    YQPkgObjListItem * item = dynamic_cast<YQPkgObjListItem *>( listViewItem );

    emit currentItemChanged( item ? item->selectable() : ZyppSel() );
}


void
YQPkgObjList::clear()
{
    emit currentItemChanged( ZyppSel() );
    _excludedItemsCount = 0;

    QY2ListView::clear();
}


void
YQPkgObjList::resetContent()
{
    logDebug() << endl;
    clear();
}


QPixmap
YQPkgObjList::statusIcon( ZyppStatus status, bool enabled, bool bySelection )
{
    QPixmap icon = YQIconPool::pkgNoInst();

    if ( enabled )
    {
        switch ( status )
        {
            case S_Del:            icon = YQIconPool::pkgDel();            break;
            case S_Install:        icon = YQIconPool::pkgInstall();        break;
            case S_KeepInstalled:  icon = YQIconPool::pkgKeepInstalled();  break;
            case S_NoInst:         icon = YQIconPool::pkgNoInst();         break;
            case S_Protected:      icon = YQIconPool::pkgProtected();      break;
            case S_Taboo:          icon = YQIconPool::pkgTaboo();          break;
            case S_Update:         icon = YQIconPool::pkgUpdate();         break;

            case S_AutoDel:        icon = YQIconPool::pkgAutoDel();        break;
            case S_AutoInstall:    icon = YQIconPool::pkgAutoInstall();    break;
            case S_AutoUpdate:     icon = YQIconPool::pkgAutoUpdate();     break;

                // Intentionally omitting 'default' branch so the compiler can
                // catch unhandled enum states
        }
    }
    else
    {
        switch ( status )
        {
            case S_Del:            icon = YQIconPool::disabledPkgDel();            break;
            case S_Install:        icon = YQIconPool::disabledPkgInstall();        break;
            case S_KeepInstalled:  icon = YQIconPool::disabledPkgKeepInstalled();  break;
            case S_NoInst:         icon = YQIconPool::disabledPkgNoInst();         break;
            case S_Protected:      icon = YQIconPool::disabledPkgProtected();      break;
            case S_Taboo:          icon = YQIconPool::disabledPkgTaboo();          break;
            case S_Update:         icon = YQIconPool::disabledPkgUpdate();         break;

            case S_AutoDel:        icon = YQIconPool::disabledPkgAutoDel();        break;
            case S_AutoInstall:    icon = YQIconPool::disabledPkgAutoInstall();    break;
            case S_AutoUpdate:     icon = YQIconPool::disabledPkgAutoUpdate();     break;

                // Intentionally omitting 'default' branch so the compiler can
                // catch unhandled enum states
        }
    }

    return icon;
}


QString
YQPkgObjList::statusText( ZyppStatus status ) const
{
    switch ( status )
    {
        case S_AutoDel:        return _( "Autodelete"                 );
        case S_AutoInstall:    return _( "Autoinstall"                );
        case S_AutoUpdate:     return _( "Autoupdate"                 );
        case S_Del:            return _( "Delete"                     );
        case S_Install:        return _( "Install"                    );
        case S_KeepInstalled:  return _( "Keep"                       );
        case S_NoInst:         return _( "Do Not Install"             );
        case S_Protected:      return _( "Protected -- Do Not Modify" );
        case S_Taboo:          return _( "Taboo -- Never Install"     );
        case S_Update:         return _( "Update"                     );
    }

    return QString();
}


void
YQPkgObjList::setCurrentStatus( ZyppStatus newStatus, bool doSelectNextItem, bool ifNewerOnly )
{
    QTreeWidgetItem * listViewItem = currentItem();

    if ( ! listViewItem )
        return;

    YQPkgObjListItem * item = dynamic_cast<YQPkgObjListItem *> (listViewItem);

    if ( item && item->editable() && _editable && ( item->candidateIsNewer() || !ifNewerOnly ) )
    {
        if ( newStatus != item->status() )
        {
            item->setStatus( newStatus );

            if ( item->showLicenseAgreement() )
            {
                item->showNotifyTexts( newStatus );
            }
            else // License not confirmed?
            {
                // Status is now S_Taboo or S_Del - update status icon
                item->setStatusIcon();
            }

            emit statusChanged();
        }
    }

    if ( doSelectNextItem )
        selectNextItem();
}


void
YQPkgObjList::setAllItemStatus( ZyppStatus newStatus, bool force )
{
    if ( ! _editable )
        return;

    busyCursor();
    QTreeWidgetItemIterator it( this );

    while ( *it )
    {
        YQPkgObjListItem * item = dynamic_cast<YQPkgObjListItem *> (*it);

        if ( item && item->editable() && newStatus != item->status() )
        {
            if ( newStatus == S_Update )
            {
                if ( force )
                {
                    item->setStatus( newStatus,
                                     false );   // sendSignals
                }
                else
                {
                    if ( item->selectable()->installedObj()      &&
                         item->status() != S_Protected           &&
                         item->selectable()->updateCandidateObj()   )
                    {
                        item->selectable()->setOnSystem( item->selectable()->updateCandidateObj() );
                    }
                }
            }
            else
            {
                item->setStatus( newStatus,
                                 false );       // sendSignals
            }
        }

        ++it;
    }

    emit updateItemStates();
    emit updatePackages();

    normalCursor();
    emit statusChanged();
}


void
YQPkgObjList::selectNextItem()
{
    QTreeWidgetItemIterator it( this );
    QTreeWidgetItem * item;

    while ( ( item = *it ) != NULL )
    {
        ++it;
        scrollToItem( *it );    // Scroll if necessary
        setCurrentItem( *it );  // Emits signals

    }
}


void
YQPkgObjList::createActions()
{
    actionSetCurrentInstall       = createAction( S_Install,       "[+]"      );
    actionSetCurrentDontInstall   = createAction( S_NoInst,        "[-]"      );
    actionSetCurrentKeepInstalled = createAction( S_KeepInstalled, "[<], [-]" );
    actionSetCurrentDelete        = createAction( S_Del,           "[-]"      );
    actionSetCurrentUpdate        = createAction( S_Update,        "[>], [+]" );

    actionSetCurrentUpdateForce   = createAction( _( "Update unconditionally" ),
                                                  statusIcon( S_Update, true ),
                                                  statusIcon( S_Update, false ) ,
                                                  "",
                                                  true ) ;


    actionSetCurrentTaboo         = createAction( S_Taboo,         "[!]"    );
    actionSetCurrentProtected     = createAction( S_Protected,     "[*]"    );

    actionSetListInstall          = createAction( S_Install,       "", true );
    actionSetListDontInstall      = createAction( S_NoInst,        "", true );
    actionSetListKeepInstalled    = createAction( S_KeepInstalled, "", true );
    actionSetListDelete           = createAction( S_Del,           "", true );
    actionSetListProtected        = createAction( S_Protected,     "", true );

    actionSetListUpdate           = createAction( _( "Update if newer version available" ),
                                                  statusIcon( S_Update, true ),
                                                  statusIcon( S_Update, false ),
                                                  "",
                                                  true );

    actionSetListUpdateForce      = createAction( _( "Update unconditionally" ),
                                                  statusIcon( S_Update, true ),
                                                  statusIcon( S_Update, false ),
                                                  "",
                                                  true );

    actionSetListTaboo            = createAction( S_Taboo,                "", true );

    connect( actionSetCurrentInstall,        SIGNAL( triggered() ), this, SLOT( setCurrentInstall()       ) );
    connect( actionSetCurrentDontInstall,    SIGNAL( triggered() ), this, SLOT( setCurrentDontInstall()   ) );
    connect( actionSetCurrentKeepInstalled,  SIGNAL( triggered() ), this, SLOT( setCurrentKeepInstalled() ) );
    connect( actionSetCurrentDelete,         SIGNAL( triggered() ), this, SLOT( setCurrentDelete()        ) );
    connect( actionSetCurrentUpdate,         SIGNAL( triggered() ), this, SLOT( setCurrentUpdate()        ) );
    connect( actionSetCurrentUpdateForce,    SIGNAL( triggered() ), this, SLOT( setCurrentUpdateForce()   ) );
    connect( actionSetCurrentTaboo,          SIGNAL( triggered() ), this, SLOT( setCurrentTaboo()         ) );
    connect( actionSetCurrentProtected,      SIGNAL( triggered() ), this, SLOT( setCurrentProtected()     ) );
    connect( actionSetListInstall,           SIGNAL( triggered() ), this, SLOT( setListInstall()          ) );
    connect( actionSetListDontInstall,       SIGNAL( triggered() ), this, SLOT( setListDontInstall()      ) );
    connect( actionSetListKeepInstalled,     SIGNAL( triggered() ), this, SLOT( setListKeepInstalled()    ) );
    connect( actionSetListDelete,            SIGNAL( triggered() ), this, SLOT( setListDelete()           ) );
    connect( actionSetListUpdate,            SIGNAL( triggered() ), this, SLOT( setListUpdate()           ) );
    connect( actionSetListUpdateForce,       SIGNAL( triggered() ), this, SLOT( setListUpdateForce()      ) );
    connect( actionSetListTaboo,             SIGNAL( triggered() ), this, SLOT( setListTaboo()            ) );
    connect( actionSetListProtected,         SIGNAL( triggered() ), this, SLOT( setListProtected()        ) );
}



QAction *
YQPkgObjList::createAction( ZyppStatus      status,
                            const QString & key,
                            bool            enabled )
{
    return createAction( statusText( status ),
                         statusIcon( status, true ),
                         statusIcon( status, false ),
                         key,
                         enabled );
}


QAction *
YQPkgObjList::createAction( const QString & text,
                            const QPixmap & icon,
                            const QPixmap & insensitiveIcon,
                            const QString & key,
                            bool            enabled )
{
    QString label = text;

    if ( ! key.isEmpty() )
        label += "\t" + key;

    QIcon iconSet ( icon );

    if ( ! insensitiveIcon.isNull() )
    {
        iconSet.addPixmap( insensitiveIcon, QIcon::Disabled );
    }

    QAction * action = new QAction( label,      // text
                                    this );     // parent
    Q_CHECK_PTR( action );
    action->setEnabled( enabled );
    action->setIcon( iconSet );

    return action;
}


void
YQPkgObjList::createNotInstalledContextMenu()
{
    _notInstalledContextMenu = new QMenu( this );
    Q_CHECK_PTR( _notInstalledContextMenu );

    _notInstalledContextMenu->addAction( actionSetCurrentInstall     );
    _notInstalledContextMenu->addAction( actionSetCurrentDontInstall );
    _notInstalledContextMenu->addAction( actionSetCurrentTaboo       );

    addAllInListSubMenu( _notInstalledContextMenu );
}


void
YQPkgObjList::createInstalledContextMenu()
{
    _installedContextMenu = new QMenu( this );
    Q_CHECK_PTR( _installedContextMenu );

    _installedContextMenu->addAction( actionSetCurrentKeepInstalled );
    _installedContextMenu->addAction( actionSetCurrentDelete        );
    _installedContextMenu->addAction( actionSetCurrentUpdate        );
    _installedContextMenu->addAction( actionSetCurrentUpdateForce   );

    addAllInListSubMenu( _installedContextMenu );
}


QMenu *
YQPkgObjList::addAllInListSubMenu( QMenu * menu )
{
    QMenu * submenu = new QMenu( menu );
    Q_CHECK_PTR( submenu );

    submenu->addAction( actionSetListInstall       );
    submenu->addAction( actionSetListDontInstall   );
    submenu->addAction( actionSetListKeepInstalled );
    submenu->addAction( actionSetListDelete        );
    submenu->addAction( actionSetListUpdate        );
    submenu->addAction( actionSetListUpdateForce   );
    submenu->addAction( actionSetListTaboo         );

    QAction *action = menu->addMenu( submenu );
    action->setText( _( "&All in This List" ) );

    return submenu;
}


QMenu *
YQPkgObjList::notInstalledContextMenu()
{
    if ( ! _notInstalledContextMenu )
        createNotInstalledContextMenu();

    return _notInstalledContextMenu;
}


QMenu *
YQPkgObjList::installedContextMenu()
{
    if ( ! _installedContextMenu )
        createInstalledContextMenu();

    return _installedContextMenu;
}


void
YQPkgObjList::updateActions( YQPkgObjListItem * item )
{
    if ( !item )
        item = dynamic_cast<YQPkgObjListItem *> ( currentItem() );

    if ( item )
    {
        ZyppSel selectable = item->selectable();

        if ( selectable->hasInstalledObj() )
        {
            actionSetCurrentInstall->setEnabled( false );
            actionSetCurrentDontInstall->setEnabled( false );
            actionSetCurrentTaboo->setEnabled( false );
            actionSetCurrentProtected->setEnabled( true );

            actionSetCurrentKeepInstalled->setEnabled( true );
            actionSetCurrentDelete->setEnabled( true );
            actionSetCurrentUpdate->setEnabled( selectable->hasCandidateObj() );
            actionSetCurrentUpdateForce->setEnabled( selectable->hasCandidateObj() );
        }
        else
        {
            actionSetCurrentInstall->setEnabled( selectable->hasCandidateObj() );
            actionSetCurrentDontInstall->setEnabled( true );
            actionSetCurrentTaboo->setEnabled( true );
            actionSetCurrentProtected->setEnabled( false );

            actionSetCurrentKeepInstalled->setEnabled( false );
            actionSetCurrentDelete->setEnabled( false );
            actionSetCurrentUpdate->setEnabled( false );
            actionSetCurrentUpdateForce->setEnabled( false );
        }
    }
    else  // ! item
    {
        actionSetCurrentInstall->setEnabled( false );
        actionSetCurrentDontInstall->setEnabled( false );
        actionSetCurrentTaboo->setEnabled( false );

        actionSetCurrentKeepInstalled->setEnabled( false );
        actionSetCurrentDelete->setEnabled( false );
        actionSetCurrentUpdate->setEnabled( false );
        actionSetCurrentUpdateForce->setEnabled( false );
        actionSetCurrentProtected->setEnabled( false );

    }
}


void
YQPkgObjList::keyPressEvent( QKeyEvent * event )
{
    if ( event )
    {
        QTreeWidgetItem * selectedListViewItem = currentItem();

        if ( selectedListViewItem )
        {
            YQPkgObjListItem * item = dynamic_cast<YQPkgObjListItem *> (selectedListViewItem);

            if ( item )
            {
                bool installed;

                if ( item->selectable() )
                    installed = item->selectable()->hasInstalledObj();
                else
                    installed = false;

                ZyppStatus status = item->status();

                switch( event->key() )
                {
                    case Qt::Key_Space:  // Cycle
                        item->cycleStatus();
                        event->accept();
                        return;

                    case Qt::Key_Plus:  // Grab everything - install or update

                        if ( installed )
                        {
                            ZyppStatus newStatus = S_KeepInstalled;

                            if ( item->candidateIsNewer() )
                                newStatus = S_Update;

                            setCurrentStatus( newStatus );
                        }
                        else
                            setCurrentStatus( S_Install );

                        event->accept();
                        return;

                    case Qt::Key_Minus: // Get rid of everything - don't install or delete
                        setCurrentStatus( installed ? S_Del : S_NoInst );
                        event->accept();
                        return;

                    case Qt::Key_Exclam:  // Taboo

                        if ( ! installed )
                            setCurrentStatus( S_Taboo );

                        event->accept();
                        return;

                    case Qt::Key_Asterisk:  // Protected

                        if ( installed )
                            setCurrentStatus( S_Protected );

                        event->accept();
                        return;

                    case Qt::Key_Greater:  // Update what can be updated

                        if ( installed && item->candidateIsNewer() )
                            setCurrentStatus( S_Update );

                        event->accept();
                        return;

                    case Qt::Key_Less:  // Revert update

                        if ( status == S_Update ||
                             status == S_AutoUpdate )
                        {
                            setCurrentStatus( S_KeepInstalled );
                        }

                        event->accept();
                        return;
                }
            }
        }
    }

    QY2ListView::keyPressEvent( event );
}


void
YQPkgObjList::message( const QString & text )
{
    QY2ListViewItem * item = new QY2ListViewItem( this );
    Q_CHECK_PTR( item );

    item->setText( nameCol() >= 0 ? nameCol() : 0, text );
}


void
YQPkgObjList::addExcludeRule( YQPkgObjList::ExcludeRule * rule )
{
    _excludeRules.push_back( rule );
}


void
YQPkgObjList::applyExcludeRules()
{
    _excludedItemsCount = 0;
    // logDebug() << "Applying exclude rules" << endl;
    QTreeWidgetItemIterator listView_it( this );

    while ( *listView_it )
    {
        QTreeWidgetItem * current_item = *listView_it;

        // Advance iterator now so it remains valid even if there are changes
        // to the QListView, e.g., if the current item is excluded and thus
        // removed from the QListView

        ++listView_it;

        applyExcludeRules( current_item );
    }
}


void
YQPkgObjList::logExcludeStatistics()
{
    if ( _excludedItemsCount > 0 )
    {
        logVerbose() << _excludedItemsCount << " packages excluded" << endl;

        for ( ExcludeRuleList::iterator rule_it = _excludeRules.begin();
              rule_it != _excludeRules.end();
              ++rule_it )
        {
            ExcludeRule * rule = *rule_it;

            if ( rule->isEnabled() )
            {
                logDebug() << "Active exclude rule: \""
                           << rule->regexp().pattern() << "\""
                           << endl;
            }
        }
    }
}


void
YQPkgObjList::applyExcludeRules( QTreeWidgetItem * listViewItem )
{
    YQPkgObjListItem * item = dynamic_cast<YQPkgObjListItem *>( listViewItem );

    if ( item )
    {
        bool exclude = false;

        for ( ExcludeRuleList::iterator rule_it = _excludeRules.begin();
              rule_it != _excludeRules.end() && ! exclude;
              ++rule_it )
        {
            ExcludeRule * rule = *rule_it;

            if ( rule->match( item ) )
            {
                exclude = true;
            }
        }

        if ( exclude != item->isExcluded() )    // change exclude status?
        {
            this->exclude( item, exclude );

            if ( exclude )
                _excludedItemsCount++;
        }
    }
}


void
YQPkgObjList::exclude( YQPkgObjListItem * item, bool exclude )
{
    if ( exclude == item->isExcluded() )
        return;

    item->setExcluded( exclude );

    QTreeWidgetItem * parentItem = item->parent();

    if ( parentItem )
        parentItem->setHidden( exclude );
    else
        item->setHidden( exclude );
}


void
YQPkgObjList::maybeSetFocus()
{
    if ( ! shouldKeepFocus( QApplication::focusWidget() ) )
        setFocus();
}


bool
YQPkgObjList::shouldKeepFocus( QWidget * widget ) const
{
    if ( ! widget )
        return false;

    // Do not take away the keyboard focus from this kind of widget because it
    // also has internal navigation with the cursor keys; for example a
    // connected filter view like the patterns view, the repositories view, the
    // patches view.
    // bsc#1204429

    if ( dynamic_cast<QAbstractItemView*>( widget ) ) // All kinds of lists and trees
        return true;
    else
        return false;
}




YQPkgObjListItem::YQPkgObjListItem( YQPkgObjList * pkgObjList,
                                    ZyppSel        selectable,
                                    ZyppObj        zyppObj )
    : QY2ListViewItem( pkgObjList )
    , _pkgObjList( pkgObjList )
    , _selectable( selectable )
    , _zyppObj( zyppObj )
    , _editable( true )
    , _excluded( false )
{
    init();
}


YQPkgObjListItem::YQPkgObjListItem( YQPkgObjList *    pkgObjList,
                                    QY2ListViewItem * parent,
                                    ZyppSel           selectable,
                                    ZyppObj           zyppObj )
    : QY2ListViewItem( parent )
    , _pkgObjList( pkgObjList )
    , _selectable( selectable )
    , _zyppObj( zyppObj )
    , _editable( true )
    , _excluded( false )
{
    init();
}


YQPkgObjListItem::YQPkgObjListItem( YQPkgObjList * pkgObjList )
    : QY2ListViewItem( pkgObjList )
    , _pkgObjList( pkgObjList )
    , _selectable( 0 )
    , _zyppObj( 0 )
    , _editable( true )
    , _excluded( false )
{
}


YQPkgObjListItem::~YQPkgObjListItem()
{
    // NOP
}


void
YQPkgObjListItem::init()
{
    if ( _zyppObj == 0 && _selectable )
        _zyppObj = _selectable->theObj();

    _candidateIsNewer   = false;
    _installedIsNewer   = false;

    const ZyppObj candidate = selectable()->candidateObj();
    const ZyppObj installed = selectable()->installedObj();

    if ( candidate && installed )
    {
        if ( candidate->edition() < installed->edition() )
            _installedIsNewer = true;
        else if ( installed->edition() < candidate->edition() )
            _candidateIsNewer = true;
    }

    if ( installed && ! candidate )
        _installedIsNewer = true;

    if ( nameCol()    >= 0 )  setText( nameCol(),     zyppObj()->name()    );
    if ( summaryCol() >= 0 )  setText( summaryCol(),  zyppObj()->summary() );

    if ( sizeCol()    >= 0 )
    {
        zypp::ByteCount size = zyppObj()->installSize();

        if ( size > 0L )
            setText( sizeCol(), size.asString() );
    }

    if ( versionCol() == instVersionCol() ) // Display both versions in the same column: 1.2.3 (1.2.4)
    {
        if ( versionCol() >= 0 )
        {
            if ( installed )
            {
                if ( zyppObj() != installed  &&
                     zyppObj() != candidate )
                {
                    setText( versionCol(), zyppObj()->edition());
                }
                else
                {
                    if ( candidate && installed->edition() != candidate->edition() )
                    {
                        setText( versionCol(),
                                 QString( "%1 (%2)" )
                                 .arg( installed->edition().c_str() )
                                 .arg( candidate->edition().c_str() ) );
                    }
                    else // no candidate or both versions are the same anyway
                    {
                        setText( versionCol(), installed->edition() );
                    }
                }
            }
            else
            {
                if ( candidate )
                    setText( versionCol(), QString( "(%1)" ).arg( candidate->edition().c_str() ) );
                else
                    setText( versionCol(), zyppObj()->edition() );
            }

            if ( _installedIsNewer )
                setForeground( versionCol(), redTextColor()  );
            else if ( _candidateIsNewer )
                setForeground( versionCol(), blueTextColor() );
            else
                setForeground( versionCol(), normalTextColor() );
        }
    }
    else // separate columns for installed and available versions
    {
        if ( instVersionCol() >= 0 )
        {
            if ( installed )
            {
                setText( instVersionCol(), installed->edition() );

                if ( _installedIsNewer )
                    setForeground( instVersionCol(), redTextColor()  );
                else if ( _candidateIsNewer )
                    setForeground( instVersionCol(), blueTextColor() );
                else
                    setForeground( versionCol(), normalTextColor() );
            }
        }

        if ( versionCol() >= 0 )
        {
            if ( zyppObj() != installed &&
                 zyppObj() != candidate )
            {
                setText( versionCol(), zyppObj()->edition() );
            }
            else if ( candidate )
            {
                setText( versionCol(), candidate->edition() );

                if ( _installedIsNewer )
                    setForeground( versionCol(), redTextColor()  );
                else if ( _candidateIsNewer )
                    setForeground( versionCol(), blueTextColor() );
                else
                    setForeground( versionCol(), normalTextColor() );
            }
        }
    }

    setStatusIcon();
}


void
YQPkgObjListItem::updateData()
{
    init();
}


void
YQPkgObjListItem::setText( int column, const string & text )
{
    QTreeWidgetItem::setText( column, fromUTF8( text.c_str() ) );
}


void
YQPkgObjListItem::setText( int column, const zypp::Edition & edition )
{
    setText( column, edition.asString() );
}


ZyppStatus
YQPkgObjListItem::status() const
{
    if ( ! selectable() )
    {
        logError() << "No selectable" << endl;
        return S_NoInst;
    }

    return selectable()->status();
}


bool
YQPkgObjListItem::bySelection() const
{
    zypp::ResStatus::TransactByValue modifiedBy = selectable()->modifiedBy();

    return ( modifiedBy == zypp::ResStatus::APPL_LOW ||
             modifiedBy == zypp::ResStatus::APPL_HIGH  );
}


void
YQPkgObjListItem::setStatus( ZyppStatus newStatus, bool sendSignals )
{
    ZyppStatus oldStatus = selectable()->status();
    selectable()->setStatus( newStatus );

    if ( oldStatus != selectable()->status() )
    {
        applyChanges();

        if ( sendSignals )
        {
            _pkgObjList->updateItemStates();
            _pkgObjList->sendUpdatePackages();
        }
    }

    setStatusIcon();
}


void
YQPkgObjListItem::solveResolvableCollections()
{
    zypp::getZYpp()->resolver()->resolvePool();
}


void
YQPkgObjListItem::updateStatus()
{
    setStatusIcon();
}


void
YQPkgObjListItem::setStatusIcon()
{
    if ( statusCol() >= 0 )
    {
        bool enabled = editable() && _pkgObjList->editable();
        setIcon( statusCol(), _pkgObjList->statusIcon( status(), enabled, bySelection() ) );
    }
}


void
YQPkgObjListItem::cycleStatus()
{
    if ( ! _editable || ! _pkgObjList->editable() )
        return;

    ZyppStatus oldStatus = status();
    ZyppStatus newStatus = oldStatus;

    switch ( oldStatus )
    {
        case S_Install:
            newStatus = S_NoInst;
            break;

        case S_Protected:
            newStatus = selectable()->hasCandidateObj() ?
                S_KeepInstalled: S_NoInst;
            break;

        case S_Taboo:
            newStatus = selectable()->hasInstalledObj() ?
                S_KeepInstalled : S_NoInst;
            break;

        case S_KeepInstalled:
            newStatus = selectable()->hasCandidateObj() ?
                S_Update : S_Del;
            break;

        case S_Update:
            newStatus = S_Del;
            break;

        case S_AutoUpdate:
            newStatus = S_KeepInstalled;
            break;

        case S_Del:
        case S_AutoDel:
            newStatus = S_KeepInstalled;
            break;

        case S_NoInst:
            if ( selectable()->hasCandidateObj() )
            {
                newStatus = S_Install;
            }
            else
            {
                logWarning() << "No candidate for " << selectable()->theObj()->name() << endl;
                newStatus = S_NoInst;
            }
            break;

        case S_AutoInstall:
            // this used to be taboo before, but now ZYpp supports
            // saving weak locks (unselected packages)
            newStatus =  S_NoInst;
            break;
    }

    if ( oldStatus != newStatus )
    {
        setStatus( newStatus );

        if ( showLicenseAgreement() )
        {
            showNotifyTexts( newStatus );
        }
        else // License not confirmed?
        {
            // Status is now S_Taboo or S_Del - update status icon
            setStatusIcon();
        }

        _pkgObjList->sendStatusChanged();
    }
}


void
YQPkgObjListItem::showNotifyTexts( ZyppStatus status )
{
    if ( ! selectable() )
        return;

    string text;

    switch ( status )
    {
        case S_Install:
            if ( selectable()->hasCandidateObj() )
                text = selectable()->candidateObj()->insnotify();
            break;

        case S_NoInst:
        case S_Del:
        case S_Taboo:
            if ( selectable()->hasCandidateObj() )
                text = selectable()->candidateObj()->delnotify();
            break;

        default: break;
    }

    if ( ! text.empty() )
    {
        logDebug() << "Showing notify text" << endl;
        YQPkgTextDialog::showText( _pkgObjList, selectable(), text );
    }
}


bool
YQPkgObjListItem::showLicenseAgreement()
{
    return showLicenseAgreement( selectable() );
}


bool
YQPkgObjListItem::showLicenseAgreement( ZyppSel sel )
{
    // if we have a subclass with no selectable
    // confirming the license should be just always
    // true
    if ( ! sel )
        return true;

    string licenseText;

    switch ( sel->status() )
    {
        case S_Install:
        case S_AutoInstall:
        case S_Update:
        case S_AutoUpdate:

            if ( sel->hasLicenceConfirmed() )
                return true;

            if ( sel->candidateObj() )
                licenseText = sel->candidateObj()->licenseToConfirm();
            break;

        default: return true;
    }

    if ( licenseText.empty() )
        return true;

    if ( LicenseCache::confirmed()->contains( licenseText ) )
    {
        logInfo() << "License verbatim confirmed before: " << sel->name() << endl;
        return true;
    }

    logDebug() << "Showing license agreement for " << sel->name() << endl;

    bool confirmed = YQPkgTextDialog::confirmText( 0, // parent
                                                   sel, licenseText );

    if ( confirmed )
    {
        logInfo() << "User confirmed license agreement for " << sel->name() << endl;
        sel->setLicenceConfirmed( true );
        LicenseCache::confirmed()->add( licenseText );
    }
    else
    {
        // The user rejected the license agreement -
        // make sure the package gets unselected.

        switch ( sel->status() )
        {
            case S_Install:
            case S_AutoInstall:

                logWarning() << "User rejected license agreement for " << sel->name()
                             << " - setting to TABOO"
                             << endl;

                sel->setStatus( S_Taboo );
                break;


            case S_Update:
            case S_AutoUpdate:

                logWarning() << "User rejected license agreement for " << sel->name()
                             << "  - setting to PROTECTED"
                             << endl;

                sel->setStatus( S_Protected );
                // S_Keep wouldn't be good enough: The next solver run might
                // set it to S_AutoUpdate again
                break;

            default: break;
        }
    }

    return confirmed;
}


QString
YQPkgObjListItem::toolTip( int col )
{
    if ( col == statusCol() )
    {
        QString tip = _pkgObjList->statusText( status() );

        switch ( status() )
        {
            case S_AutoDel:
            case S_AutoInstall:
            case S_AutoUpdate:

                if ( bySelection() )
                    // Translators: Additional hint what caused an auto-status
                    tip += "\n" + _( "(by a software selection)" );
                else
                    tip += "\n" + _( "(by dependencies)" );

                break;

            default:
                break;
        }

        return tip;
    }

    return QString();
}



bool YQPkgObjListItem::operator<( const QTreeWidgetItem & otherListViewItem ) const
{
    const YQPkgObjListItem * other = dynamic_cast<const YQPkgObjListItem *> (&otherListViewItem);
    int col = treeWidget()->sortColumn();

    if ( other )
    {
        if ( col == nameCol() )
        {
            return ( strcasecmp( this->zyppObj()->name().c_str(), other->zyppObj()->name().c_str() ) < 0 );
        }
        if ( col == summaryCol() )
        {
            // locale aware sort
            return ( strcoll( this->zyppObj()->summary().c_str(), other->zyppObj()->summary().c_str() ) < 0 );
        }
        if ( col == sizeCol() )
        {
            // Numeric sort by size

            return ( this->zyppObj()->installSize() < other->zyppObj()->installSize() );
        }
        else if ( col == statusCol() )
        {
            // Sorting by status depends on the numeric value of the ZyppStatus
            // enum, thus it is important to insert new package states there
            // where they make most sense. We want to show dangerous or
            // noteworthy states first - e.g., "taboo" which should seldeom
            // occur, but when it does, it is important.

            bool result = ( this->status() < other->status() );
            if ( ! result && this->status() == other->status() )
                result = this->zyppObj()->name() < other->zyppObj()->name();
            return result;
        }
        else if ( col == instVersionCol() ||
                  col == versionCol() )
        {
            // Sorting by version numbers doesn't make too much sense, so let's
            // sort by package relation:
            // - Installed newer than candidate (red)
            // - Candidate newer than installed (blue) - worthwhile updating
            // - Installed
            // - Not installed, but candidate available
            //
            // Within these categories, sort versions by ASCII - OK, it's
            // pretty random, but predictable.

            int thisPoints  = this->versionPoints();
            int otherPoints = other->versionPoints();

            if (thisPoints == otherPoints )
                return ( QString (this->zyppObj()->edition().c_str() ) <
                         QString (other->zyppObj()->edition().c_str() ) );
            else
                return ( thisPoints < otherPoints );
        }
    }

    // Fallback: Use parent class method
    return QY2ListViewItem::operator<( otherListViewItem );
}


int
YQPkgObjListItem::versionPoints() const
{
    int points = 0;

    if ( installedIsNewer() )                   points += 1000;
    if ( candidateIsNewer() )                   points += 100;
    if ( selectable()->hasInstalledObj() )      points += 10;
    if ( selectable()->hasCandidateObj() )      points += 1;

    return points;
}


void
YQPkgObjListItem::setExcluded( bool excl )
{
    _excluded = excl;
}

YQPkgObjList::ExcludeRule::ExcludeRule( YQPkgObjList *             parent,
                                        const QRegularExpression & regexp,
                                        int                        column )
    : _parent( parent )
    , _regexp( regexp )
    , _column( column )
    , _enabled( true )
{
    _parent->addExcludeRule( this );
}


void
YQPkgObjList::ExcludeRule::enable( bool enable )
{
    _enabled = enable;

#if VERBOSE_EXCLUDE_RULES
    logDebug() << ( enable ? "Enabling" : "Disabling" )
               << " exclude rule " << _regexp.pattern()
               << endl;
#endif
}


void
YQPkgObjList::ExcludeRule::setRegexp( const QRegularExpression & regexp )
{
    _regexp = regexp;
}


void
YQPkgObjList::ExcludeRule::setColumn( int column )
{
    _column = column;
}


bool
YQPkgObjList::ExcludeRule::match( QTreeWidgetItem * item )
{
    if ( ! _enabled )
        return false;

    QString text = item->text( _column );

    if ( text.isEmpty() )
        return false;

    return _regexp.match( text,
                          0,  // offset
                          QRegularExpression::NormalMatch,
                          QRegularExpression::AnchorAtOffsetMatchOption ).hasMatch();
}


void YQPkgObjList::slotCustomContextMenu( const QPoint& pos )
{
    YQPkgObjListItem * item =
        dynamic_cast<YQPkgObjListItem *> ( currentItem() );

    if ( item && editable() && item->editable() )
    {
        updateActions( item );

        if ( ! item->selectable() )
            return;

        QMenu * contextMenu =
            ! item->selectable()->installedEmpty() ?
            installedContextMenu() : notInstalledContextMenu();

        if ( contextMenu )
            contextMenu->popup( viewport()->mapToGlobal( pos ) );
    }
}


