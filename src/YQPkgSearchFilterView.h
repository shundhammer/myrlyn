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


#ifndef YQPkgSearchFilterView_h
#define YQPkgSearchFilterView_h

#include "YQZypp.h"
#include <QWidget>
#include <QEvent>
#include <QWidget>

#include "SearchFilter.h"


// Generated with 'uic' from a Qt designer .ui form: search-filter-view.ui
//
// Check out ../build/src/myrlyn_autogen/include/ui_search-filter-view.h
// for the variable names of the widgets.

#include "ui_search-filter-view.h"


class QComboBox;
class QCheckBox;
class QPushButton;
class QRadioButton;


/**
 * Filter view for searching within packages
 **/
class YQPkgSearchFilterView : public QWidget
{
    Q_OBJECT

public:
    enum SearchFields
    {
        AllSearchFields,
        BasicSearchFields  // Only name, summary, description
    };


    /**
     * Constructor
     **/
    YQPkgSearchFilterView( QWidget *    parent,
                           SearchFields searchFields = AllSearchFields );

    /**
     * Destructor
     **/
    virtual ~YQPkgSearchFilterView();

    /**
     * Check one ResObject against the currently selected values.
     * Returns true if the package matches, false if not.
     **/
    bool check( ZyppSel selectable,
                ZyppObj zyppObj );


public slots:

    /**
     * Notification that a new filter is the one to be shown.
     **/
    void showFilter( QWidget * newFilter );

    /**
     * Filter according to the view's rules and current selection.
     * Emits those signals:
     *    filterStart()
     *    filterMatch() for each pkg that matches the filter
     *    filterFinished()
     **/
    void filter();

    /**
     * Check if 'searchFilter' matches a zypp capabilites container 'capSet'
     * such as its 'provides()' or 'requires()'.
     **/
    bool checkCap( zypp::Capabilities   capSet,
                   const SearchFilter & searchFilter );


    /**
     * Set the keyboard focus into this view's input field.
     **/
    void setFocus();


protected slots:

    /**
     * Notification that the search mode changed so the detected search mode
     * can be updated.
     **/
    void searchModeChanged( int index );

    /**
     * Update the detected search filter mode label.
     **/
    void updateDetectedFilterMode( int index = -1 );

    /**
     * Update the detected search filter mode label.
     **/
    void updateDetectedFilterMode( const QString & searchPattern );

    /**
     * Cycle through the valid default filter modes for "Auto" mode.
     **/
    void cycleDefaultFilterModes();


signals:

    /**
     * Emitted when the filtering starts. Use this to clear package lists
     * etc. prior to adding new entries.
     **/
    void filterStart();

    /**
     * Emitted during filtering for each pkg that matches the filter.
     **/
    void filterMatch( ZyppSel selectable,
                      ZyppPkg pkg );

    /**
     * Emitted when filtering is finished.
     **/
    void filterFinished();

    /**
     * Send a short message about unsuccessful searches.
     **/
    void message( const QString & text );


protected:

    /**
     * The filtering without overriding and restoring any exclude rules.
     **/
    void filterInternal();

    /**
     * Build a SearchFilter object from the widgets.
     **/
    SearchFilter buildSearchFilterFromWidgets();

    /**
     * Hide the detected auto mode and its tool button.
     **/
    void hideDetectedAutoMode();

    /**
     * Key press event: Execute search upon 'Return'
     * Reimplemented from QVBox / QWidget.
     **/
    virtual void keyPressEvent( QKeyEvent * event ) override;

    /**
     * Event filter to catch foreign events
     *
     * Reimplemented from QObject.
     **/
    virtual bool eventFilter( QObject * watchedObj, QEvent * event ) override;


    /**
     * Read settings from the config file.
     **/
    void readSettings();

    /**
     * Write settings to the config file.
     **/
    void writeSettings();


    //
    // Data members
    //

    Ui::SearchFilterView *   _ui;
    SearchFields             _searchFields;
    SearchFilter::FilterMode _defaultAutoMode;
};


#endif // ifndef YQPkgSearchFilterView_h
