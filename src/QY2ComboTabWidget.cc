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


#include <QComboBox>
#include <QLabel>
#include <QStackedWidget>
#include <QHBoxLayout>

#include <QFrame>

#include "QY2ComboTabWidget.h"

using std::string;



QY2ComboTabWidget::QY2ComboTabWidget( const QString & label,
                                      QWidget *       parent )
    : QWidget( parent )
    , _minimizePage( 0 )
{
    QVBoxLayout *vbox = new QVBoxLayout( this );
    vbox->setContentsMargins( 0, 0, 0, 0 );

    this->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred ) ); // hor/vert

    _comboLabel = new QLabel( label );
    Q_CHECK_PTR( _comboLabel );
    _comboLabel->setAlignment( Qt::AlignLeft | Qt::AlignBottom );
    vbox->addWidget( _comboLabel );

    _comboBox = new QComboBox( this );
    Q_CHECK_PTR( _comboBox );
    vbox->addWidget(_comboBox);

    _comboLabel->setBuddy( _comboBox );
    _comboBox->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) ); // hor/vert

    connect( _comboBox, SIGNAL( activated    ( int ) ),
             this,      SLOT  ( showPageIndex( int ) ) );

    _widgetStack = new QStackedWidget( this );
    Q_CHECK_PTR( _widgetStack );

    vbox->addWidget(_widgetStack);
}



QY2ComboTabWidget::~QY2ComboTabWidget()
{

}


void
QY2ComboTabWidget::addPage( const QString & page_label, QWidget * new_page )
{
    _pages.insert( _comboBox->count(), new_page );
    _comboBox->addItem( page_label );
    _widgetStack->addWidget( new_page );

    if ( ! _widgetStack->currentWidget() )
        _widgetStack->setCurrentWidget( new_page );
}


void
QY2ComboTabWidget::setMinimizePage( QWidget * page )
{
    _minimizePage = page;
    _widgetStack->setVisible( page != _widgetStack->currentWidget() );
}


void
QY2ComboTabWidget::showPageIndex( int index )
{
    if ( _pages.contains(index) )
    {
        QWidget * page = _pages[ index ];
        _widgetStack->setCurrentWidget( page );
        _widgetStack->setVisible( page != _minimizePage );

        // yuiDebug() << "Changing current page" << Qt::endl;
        emit currentChanged( page );
    }
    else
    {
        qWarning( "QY2ComboTabWidget: Page #%d not found", index );
        return;
    }
}


void
QY2ComboTabWidget::showPage( QWidget * page )
{
    _widgetStack->setCurrentWidget( page );

    if ( page == _pages[ _comboBox->currentIndex() ] )
    {
        // Shortcut: If the requested page is the one that belongs to the item
        // currently selected in the combo box, don't bother searching the
        // correct combo box item.
        return;
    }

    // Search the hash for this page

    QHashIterator<int, QWidget *> it( _pages );

    while ( it.hasNext() )
    {
        it.next();
        if ( page == it.value() )
        {
            _comboBox->setCurrentIndex( it.key() );
            return;
        }
    }

    // If we come this far, that page isn't present in the dict.

    qWarning( "QY2ComboTabWidget: Page not found" );
}
