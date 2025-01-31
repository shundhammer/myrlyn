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


#include <zypp/ProblemSolution.h>
#include <zypp/Resolver.h>
#include <zypp/ResolverProblem.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>

#include <QDateTime>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QRadioButton>
#include <QVBoxLayout>

#include "Logger.h"
#include "YQIconPool.h"
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
    for ( YQPkgConflict * conflict: _conflicts )
    {
        _layout->removeWidget( conflict );
        delete conflict;
    }

    _conflicts.clear();

    // kill the stretch item, too

    delete _layout->takeAt( 0 );
}


void
YQPkgConflictList::fill( const ZyppProblemList problemList )
{
    clear();

    for ( const ZyppProblem & problem: problemList )
    {
        YQPkgConflict *conflict = new YQPkgConflict( widget(), problem );
        Q_CHECK_PTR( conflict );

        connect( conflict, SIGNAL( expanded() ), SLOT( relayout() ) );

        _layout->addWidget( conflict );
        _conflicts.push_back( conflict );
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

    for ( const YQPkgConflict * conflict: _conflicts )
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

    for ( YQPkgConflict * conflict: _conflicts )
    {
        ZyppSolution userChoice = conflict->userSelectedResolution();

        if ( userChoice )
            userChoices.push_back( userChoice );
    }

    zypp::getZYpp()->resolver()->applySolutions( userChoices );
    emit updatePackages();
}


void
YQPkgConflictList::askSaveToFile() const
{
    QString filename =
        QFileDialog::getSaveFileName( window(), // parent
                                      _( "Save conflicts list" ),
                                      "conflicts.txt" );
    if ( ! filename.isEmpty() )
        saveToFile( filename, true );
}


void
YQPkgConflictList::saveToFile( const QString filename, bool interactive ) const
{
    // Open file
    QFile file( filename );

    if ( ! file.open( QIODevice::WriteOnly ) )
    {
        logError() << "Can't open file " << filename << Qt::endl;

        if ( interactive )
        {
            // Post error popup.

            QMessageBox::warning( window(),         // parent
                                  _( "Error" ), // caption
                                  _( "Cannot open file %1" ).arg( filename ) );
        }

        return;
    }


    // Write header

    QString header = "#### YaST2 conflicts list - generated ";
    header += QDateTime::currentDateTime().toString( "yyyy-MM-dd hh:mm:ss" );
    header += " ####\n\n";

    file.write(header.toUtf8());

    for ( const YQPkgConflict * conflict: _conflicts )
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

    const ZyppSolutionList solutions = problem()->solutions();

    int count = 0;

    for ( const ZyppSolution & solution: solutions )
    {
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

