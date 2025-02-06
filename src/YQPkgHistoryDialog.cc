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
#include <QEventLoop>
#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSplitter>
#include <QTreeWidget>

#include <zypp-core/Date.h>
#include <zypp-core/Pathname.h>
#include <zypp-core/Url.h>
#include <zypp-core/base/Exception.h>
#include <zypp/parser/HistoryLogReader.h>
#include <zypp/Edition.h>

#include "Logger.h"
#include "MainWindow.h"
#include "QY2CursorHelper.h"
#include "YQIconPool.h"
#include "YQi18n.h"
#include "utf8.h"

#include "YQPkgHistoryDialog.h"


#define SPACING         4	// between subwidgets
#define MARGIN		9	// around the widget

#define FILENAME	"/var/log/zypp/history"



YQPkgHistoryDialog::YQPkgHistoryDialog( QWidget * parent )
    : QDialog( parent ? parent : MainWindow::instance() )
{
    // Dialog title
    setWindowTitle( _( "Package History" ) );

    setSizeGripEnabled( true );
    setMinimumSize( 750, 550 );


    // Outer (main) layout

    QVBoxLayout * layout = new QVBoxLayout();
    Q_CHECK_PTR( layout );
    setLayout( layout );
    layout->setContentsMargins( MARGIN, MARGIN, MARGIN, MARGIN );
    layout->setSpacing( SPACING );


    // Heading

    QLabel * label = new QLabel( _( "Package History (/var/log/zypp/history)" ), this );
    label->setFixedHeight( label->sizeHint().height() );
    layout->addWidget( label );


    // Splitter between the trees

    QSplitter * splitter = new QSplitter( Qt::Horizontal, this );
    Q_CHECK_PTR( splitter );
    layout->addWidget( splitter );


    // Flat list for the dates

    _datesTree = new QTreeWidget( splitter);
    _datesTree->setColumnCount( 1 );
    _datesTree->setHeaderLabels( QStringList( _( "Date" ) ) );
    _datesTree->setRootIsDecorated( false );


    // Tree for the actions: Action items below a date item parent

    _actionsTree = new QTreeWidget( splitter );
    _actionsTree->setColumnCount( 2 );
    _actionsTree->setHeaderLabels( QStringList( _( "Action" ) ) << _( "Version/URL" ) );
    _actionsTree->setColumnWidth( 0, 350 );


    // Horizontal stretch factors for each tree in the splitter

    splitter->setStretchFactor( 0, 1 );
    splitter->setStretchFactor( 1, 3 );


    // Button box to center the single button

    QHBoxLayout * hbox = new QHBoxLayout();
    Q_CHECK_PTR( hbox );
    layout->addLayout( hbox );
    hbox->addStretch();

    QPushButton * okButton = new QPushButton( _( "&OK" ), this );
    Q_CHECK_PTR( okButton );
    hbox->addWidget( okButton );
    okButton->setDefault( true );

    hbox->addStretch();


    // Signal / slot connections

    connect( okButton,          SIGNAL( clicked() ),
	     this,              SLOT  ( accept()  ) );

    connect( _datesTree,        SIGNAL( itemSelectionChanged() ),
	    this,               SLOT  ( selectDate()           ) );

    connect( _actionsTree,      SIGNAL( itemSelectionChanged() ),
	    this,               SLOT  ( selectAction()         ) );
}


void
YQPkgHistoryDialog::showHistoryDialog( QWidget* parent)
{
    YQPkgHistoryDialog dialog( parent );

    // Give instant feedback: Show the dialog and process the "show" event
    // immediately so the window opens immediately (albeit empty)

    dialog.show();
    QEventLoop eventLoop;
    eventLoop.processEvents( QEventLoop::ExcludeUserInputEvents,
			     200 ); // millisec

    busyCursor();
    dialog.populate(); // This takes a moment
    normalCursor();

    dialog.exec();
}


void
YQPkgHistoryDialog::populate()
{
    YQPkgHistoryItemCollector itemCollector( _datesTree, _actionsTree );
    zypp::parser::HistoryLogReader reader( FILENAME,
                                           zypp::parser::HistoryLogReader::Options(),
                                           boost::ref( itemCollector ) );
    try
    {
	reader.readAll();
    }
    catch (  const zypp::Exception & exception )
    {
        logWarning() << "CAUGHT zypp exception: " << exception.asUserHistory() << Qt::endl;
        showReadHistoryWarning( fromUTF8( exception.asUserHistory() ) );
    }
}


void
YQPkgHistoryDialog::showReadHistoryWarning( const QString & message )
{
    QMessageBox msgBox;

    // Translators: This is a (short) text indicating that something went
    // wrong while trying to read the history file.

    QString heading = _( "Unable to read history" );

    if (  heading.length() < 25 )    // Avoid very narrow message boxes
    {
        QString blanks;
        blanks.fill( ' ', 50 - heading.length() );
        heading += blanks;
    }

    msgBox.setText( heading );
    msgBox.setIcon( QMessageBox::Warning );
    msgBox.setInformativeText( message );
    msgBox.exec();
}


void
YQPkgHistoryDialog::selectDate()
{
    QString item = _datesTree->selectedItems().first()->text( 0 );
    QList<QTreeWidgetItem *> items = _actionsTree->findItems( item, Qt::MatchExactly, 0);

    if ( items.size() > 0 )
    {
        QSignalBlocker sigBlocker( this );

	_actionsTree->expandItem( items.first() );
	_actionsTree->setCurrentItem( items.first() );
	_actionsTree->scrollToItem( items.first(), QAbstractItemView::PositionAtTop );
    }
}


void
YQPkgHistoryDialog::selectAction()
{
    QTreeWidgetItem * item = _actionsTree->selectedItems().first();

    // if this is not a top-level item, better pick a top-level one

    if ( item->parent() )
	item = item->parent();

    QList<QTreeWidgetItem *> items = _datesTree->findItems( item->text( 0 ),
                                                            Qt::MatchExactly | Qt::MatchRecursive,
                                                            0 );
    if (  items.size() > 0 )
    {
        QSignalBlocker sigBlocker( this );
	_datesTree->setCurrentItem( items.first());
    }
}


//
//----------------------------------------------------------------------
//


YQPkgHistoryItemCollector::YQPkgHistoryItemCollector( QTreeWidget * datesTree,
                                                      QTreeWidget * actionsTree )
    : _datesTree( datesTree )
    , _actionsTree( actionsTree )
{
    // NOP
}


bool
YQPkgHistoryItemCollector::operator() ( const zypp::HistoryLogData::Ptr & item_ptr )
{
    QString actionDate = fromUTF8( item_ptr->date().form( "%e %B %Y" ) );

    if ( actionDate != _lastDate ) // First action for a new date?
    {
        addDatesTreeItem( actionDate );
        addActionsDateItem( actionDate );

        _lastDate = actionDate;
    }

    QStringList columns = actionColumns( item_ptr );

    if ( ! columns.isEmpty() )
    {
        QTreeWidgetItem * actionItem = new QTreeWidgetItem( _actionsDateItem, columns );
        actionItem->setIcon( 0, actionIcon( item_ptr->action() ) );
    }

    return true;
}


void
YQPkgHistoryItemCollector::addDatesTreeItem( const QString & actionDate )
{
    new QTreeWidgetItem( _datesTree, QStringList( actionDate ) );
}


void
YQPkgHistoryItemCollector::addActionsDateItem( const QString & actionDate )
{
    _actionsDateItem = new QTreeWidgetItem( _actionsTree, QStringList( actionDate ) );
    _actionsDateItem->setExpanded( true );
}


QStringList
YQPkgHistoryItemCollector::actionColumns( const zypp::HistoryLogData::Ptr & item_ptr )
{
    QStringList columns;

    switch ( item_ptr->action().toEnum() )
    {
        case zypp::HistoryActionID::INSTALL_e:
            {
                zypp::HistoryLogDataInstall * item =
                    static_cast <zypp::HistoryLogDataInstall *>( item_ptr.get() );

                columns << fromUTF8( item->name() );
                columns << fromUTF8( item->edition().version() );
            }
            break;

        case zypp::HistoryActionID::REMOVE_e:
            {
                zypp::HistoryLogDataRemove * item =
                    static_cast <zypp::HistoryLogDataRemove *>( item_ptr.get() );

                columns << fromUTF8( item->name() );
                columns << fromUTF8( item->edition().version() );
            }
            break;

        case zypp::HistoryActionID::REPO_ADD_e:
            {
                zypp::HistoryLogDataRepoAdd * item =
                    static_cast <zypp::HistoryLogDataRepoAdd *>( item_ptr.get() );

                columns << fromUTF8( item->alias() );
                columns << fromUTF8( item->url().asString() );
            }
            break;

        case zypp::HistoryActionID::REPO_REMOVE_e:
            {
                zypp::HistoryLogDataRepoRemove * item =
                    static_cast <zypp::HistoryLogDataRepoRemove *>( item_ptr.get() );

                columns << fromUTF8( item->alias() );
            }
            break;

        case zypp::HistoryActionID::REPO_CHANGE_ALIAS_e:
            {
                zypp::HistoryLogDataRepoAliasChange * item =
                    static_cast <zypp::HistoryLogDataRepoAliasChange *>( item_ptr.get() );

                columns << fromUTF8( item->oldAlias() ) + " -> " + fromUTF8( item->newAlias() );
            }
            break;

        case zypp::HistoryActionID::REPO_CHANGE_URL_e:
            {
                zypp::HistoryLogDataRepoUrlChange * item =
                    static_cast <zypp::HistoryLogDataRepoUrlChange *>( item_ptr.get() );

                columns << fromUTF8( item->alias() );
                columns << fromUTF8( item->newUrl().asString() );
            }
            break;

        default:
            break;
    }

    return columns;
}


QPixmap
YQPkgHistoryItemCollector::actionIcon( zypp::HistoryActionID id )
{
    switch ( id.toEnum() )
    {
        case zypp::HistoryActionID::INSTALL_e:     return YQIconPool::pkgInstall();
        case zypp::HistoryActionID::REMOVE_e:      return YQIconPool::pkgDel();
        case zypp::HistoryActionID::REPO_REMOVE_e: return YQIconPool::treeMinus();
        case zypp::HistoryActionID::REPO_ADD_e:    return YQIconPool::treePlus();

        default: return QPixmap();
    }
}
