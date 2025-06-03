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


#ifndef YQPkgPatchFilterView_h
#define YQPkgPatchFilterView_h

#include <QLabel>


class YQPkgPatchList;
class YQPkgDescriptionView;
class QComboBox;
class QSplitter;
class QTabWidget;


/**
 * Display a list of zypp::Patch objects and details about the currently
 * selected patch.
 **/
class YQPkgPatchFilterView : public QWidget
{
    Q_OBJECT

public:

    /**
     * Constructor
     **/
    YQPkgPatchFilterView( QWidget * parent );

    /**
     * Destructor
     **/
    virtual ~YQPkgPatchFilterView();


    /**
     * Returns this view's selections list.
     **/
    YQPkgPatchList * patchList() const { return _patchList; }


public slots:

    /**
     * Notification that a new filter is the one to be shown.
     **/
    void showFilter( QWidget * newFilter );

    /**
     * Reset this when going back after committing zypp changes.
     **/
    void reset();


protected slots:

    /**
     * Fill the patch list with regard to the _patchCategory
     * combo box.
     **/
    void fillPatchList();


protected:


    // Data members

    QSplitter *            _splitter;
    YQPkgPatchList *       _patchList;
    QComboBox *            _patchFilter;
    QTabWidget *           _detailsViews;
    YQPkgDescriptionView * _descriptionView;
};


#endif // ifndef YQPkgPatchFilterView_h
