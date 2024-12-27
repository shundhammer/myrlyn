/*  ------------------------------------------------------
              __   _____  ____  _
              \ \ / / _ \|  _ \| | ____ _
               \ V / | | | |_) | |/ / _` |
                | || |_| |  __/|   < (_| |
                |_| \__\_\_|   |_|\_\__, |
                                    |___/
    ------------------------------------------------------

    Project:  YQPkg Package Selector
    Copyright (c) 2024 SUSE LLC
    License:  GPL V2 - See file LICENSE for details.

    Textdomain "qt-pkg"
 */


#include <errno.h>

#include <zypp/ZYppFactory.h>


#include <QDateTime>
#include <QMessageBox>
#include <QRadioButton>
#include <QVBoxLayout>

#include "Exception.h"
#include "Logger.h"
#include "QY2CursorHelper.h"
#include "YQIconPool.h"
#include "YQPkgConflictDialog.h"
#include "YQi18n.h"
#include "utf8.h"
#include "YQPkgConflictList.h"


YQPkgConflictList::YQPkgConflictList( QWidget * parent )
    : QScrollArea( parent )
    , _layout( 0 )
{
    setWidget( new QFrame( this ) );
    _layout = new QVBoxLayout;
    widget()->setLayout( _layout );
    clear();

    widget()->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred );
}


YQPkgConflictList::~YQPkgConflictList()
{
    // NOP
}


void
YQPkgConflictList::clear()
{
    YQPkgConflict * conflict;

    foreach( conflict, _conflicts )
    {
        _layout->removeWidget( conflict );
        delete conflict;
    }

    _conflicts.clear();

    // kill the stretch item, too

    delete _layout->takeAt( 0 );
}


void
YQPkgConflictList::fill( ZyppProblemList problemList )
{
    clear();

    ZyppProblemList::iterator it = problemList.begin();

    while ( it != problemList.end() )
    {
        YQPkgConflict *conflict = new YQPkgConflict( widget(), *it );
        Q_CHECK_PTR( conflict );

        connect( conflict, SIGNAL( expanded() ), SLOT( relayout() ) );

        _layout->addWidget( conflict );
        _conflicts.push_back( conflict );
        ++it;
    }

    _layout->addStretch( 1 );
    relayout();
}


void YQPkgConflictList::relayout()
{
    // For some weird reason, the layout's minSize is still 18x18 even after
    // 3000 pixels inserted, so we have to do the math on our own.

    int left = 0, top = 0, right = 0, bottom = 0;
    _layout->getContentsMargins(&left, &top, &right, &bottom);
    QSize minSize = QSize( left + right, top + bottom );

    YQPkgConflict * conflict;

    foreach( conflict, _conflicts )
    {
        minSize = minSize.expandedTo( conflict->minimumSizeHint() );
        minSize.rheight() += conflict->minimumSizeHint().height() + _layout->spacing();
    }

    widget()->resize( minSize );
}


void
YQPkgConflictList::applyResolutions()
{
    ZyppSolutionList userChoices;
    YQPkgConflict *  conflict;

    foreach( conflict, _conflicts )
    {
        ZyppSolution userChoice = conflict->userSelectedResolution();

        if ( userChoice )
            userChoices.push_back( userChoice );
    }

    zypp::getZYpp()->resolver()->applySolutions( userChoices );
    emit updatePackages();
}


#if 0
void
YQPkgConflictList::askSaveToFile() const
{
    QString filename = YQApplication::askForSaveFileName( "conflicts.txt",      // startsWith
                                                          "*.txt",              // filter
                                                          _( "Save Conflicts List" ) );
    if ( ! filename.isEmpty() )
        saveToFile( filename, true );
}
#endif


void
YQPkgConflictList::saveToFile( const QString filename, bool interactive ) const
{
    // Open file
    QFile file(filename);

    if ( ! file.open(QIODevice::WriteOnly) )
    {
        logError() << "Can't open file " << filename << Qt::endl;

        if ( interactive )
        {
            // Post error popup.

            QMessageBox::warning( 0,                                            // parent
                                  _( "Error" ),                                 // caption
                                  _( "Cannot open file %1" ).arg( filename ),
                                  QMessageBox::Ok | QMessageBox::Default,       // button0
                                  QMessageBox::NoButton,                        // button1
                                  QMessageBox::NoButton );                      // button2
        }
        return;
    }


    // Write header

    QString header = "#### YaST2 conflicts list - generated ";
    header += QDateTime::currentDateTime().toString( "yyyy-MM-dd hh:mm:ss" );
    header += " ####\n\n";

    file.write(header.toUtf8());

    YQPkgConflict * conflict;

    foreach( conflict, _conflicts )
    {
        conflict->saveToFile( file );
    }


    // Write footer

    file.write("\n#### YaST2 conflicts list END ###\n" );


    // Clean up

    if ( file.isOpen() )
        file.close();
}






YQPkgConflict::YQPkgConflict( QWidget *   parent,
                              ZyppProblem problem )
    : QFrame( parent )
    , _problem( problem )
    , _resolutionsHeader( 0 )
{
    _layout = new QVBoxLayout( this );
    _layout->setSpacing( 0 );
    _layout->setContentsMargins( 0, 0, 0, 0 );

    formatHeading();

    QLabel * detailsLabel = new QLabel( fromUTF8 ( _problem->details() ), this );
    _layout->addWidget( detailsLabel );

    setProperty( "class", "conflict" );
    addSolutions();
    setMinimumSize( _layout->minimumSize() );
    setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
}


void
YQPkgConflict::formatHeading()
{
    QFrame * frame = new QFrame( this );
    frame->setProperty( "class", "conflict-frame" );

    QHBoxLayout * hbox  = new QHBoxLayout( frame );

    QLabel * pix = new QLabel( this );
    pix->setPixmap( YQIconPool::normalPkgConflict() );
    hbox->addWidget( pix );

    QString  text = fromUTF8( problem()->description() );
    QLabel * heading = new QLabel( text, this );
    heading->setProperty( "class", "conflict-heading" );
    heading->setStyleSheet( "font-size: +2; font: bold;" );
    hbox->addWidget( heading );

    hbox->addStretch( 1 );

    _layout->addWidget( frame );
}


void
YQPkgConflict::addSolutions()
{
    _resolutionsHeader = new QLabel( _( "Conflict Resolution:" ), this );
    _layout->addWidget( _resolutionsHeader );

    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->addSpacing( 20 );

    QVBoxLayout *vbox = new QVBoxLayout();
    hbox->addLayout( vbox );
    _layout->addLayout( hbox );

    ZyppSolutionList solutions = problem()->solutions();
    ZyppSolutionList::iterator it = solutions.begin();

    int count = 0;

    while ( it != solutions.end() )
    {
        ZyppSolution solution = *it;
        QString      shortcut;

        if ( ++count < 10 )
            shortcut = QString( "&%1:" ).arg( count );

        QString text = shortcut + fromUTF8( solution->description() );

        QRadioButton * solutionButton = new QRadioButton( text, this );
        vbox->addWidget( solutionButton );
        _solutions[ solutionButton ] = solution;

        QString details = fromUTF8( solution->details() );

        if ( ! details.isEmpty() )
        {
            QStringList lines = details.split( "\n" );

            if ( lines.count() > 7 )
            {
                details = "<qt>";

                for ( int i = 0; i < 4; i++ )
                    details += lines[i] + "<br>\n";

                details += _( "<a href='/'>%1 more...</a>" ).arg( lines.count() - 4 );
            }

            QLabel * detailsLabel = new QLabel( details, this );

            connect( detailsLabel, SIGNAL( linkActivated( QString ) ),
                     this,         SLOT  ( detailsExpanded()        ) );

            QHBoxLayout * hbox = new QHBoxLayout();
            hbox->addSpacing( 15 );
            hbox->addWidget( detailsLabel );
            vbox->addLayout( hbox );
            _details[ detailsLabel ] = solution;
        }

        ++it;
    }
}


void
YQPkgConflict::detailsExpanded()
{
    QLabel * obj = qobject_cast<QLabel*>( sender() );

    if ( !obj || ! _details.contains( obj ) )
        return;

    QSize _size = size();
    int oldHeight = obj->height();
    obj->setText( fromUTF8( _details[obj]->details() ) );

    resize( _size.width(), _size.height() + ( obj->minimumSizeHint().height() - oldHeight ) );
    emit expanded();
}


ZyppSolution
YQPkgConflict::userSelectedResolution()
{
    QMap<QRadioButton*, ZyppSolution>::iterator it;

    for ( it = _solutions.begin(); it != _solutions.end(); ++it )
    {
        QRadioButton * button = it.key();

        if ( button->isChecked() )
        {
            ZyppSolution solution = it.value();
            logInfo() << "User selected resolution \"" << solution->description() << "\"" << Qt::endl;

            return solution;
        }
    }

    return ZyppSolution(); // Null pointer
}


void
YQPkgConflict::saveToFile( QFile &file ) const
{
    if ( ! file.isOpen() )
        return;

    // Write item

    QMap<QRadioButton*, ZyppSolution>::const_iterator it;

    file.write( problem()->description().c_str() );
    file.write( "\n" );
    file.write( problem()->details().c_str() );
    file.write( "\n" );

    QString buffer;

    for ( it = _solutions.begin(); it != _solutions.end(); ++it )
    {
        QRadioButton * button   = it.key();
        ZyppSolution   solution = it.value();
        buffer = QString( "    [%1] %2\n" )
            .arg( button->isChecked() ? "x" : " " )
            .arg( qPrintable( fromUTF8( solution->description() ) ) );
        buffer += fromUTF8( solution->details() );
        buffer += "\n";
        file.write( buffer.toUtf8() );
    }

    file.write( "\n\n" );
}

