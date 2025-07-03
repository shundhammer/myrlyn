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


#ifndef YQPkgVersionsView_h
#define YQPkgVersionsView_h


#include <QScrollArea>
#include <QRadioButton>
#include <QCheckBox>

#include <zypp/ResTraits.h>
#include <zypp/ui/Selectable.h>

#include "YQZypp.h"


class QTabWidget;
class QVBoxLayout;
class QButtonGroup;
class YQPkgMultiVersion;


/**
 * Package version selector: Display a list of available versions from
 * all the different installation sources and let the user change the candidate
 * version for installation / update.
 **/
class YQPkgVersionsView: public QScrollArea
{
    Q_OBJECT

public:

    /**
     * Constructor.
     **/
    YQPkgVersionsView( QWidget * parent );

    /**
     * Destructor
     **/
    virtual ~YQPkgVersionsView();

    /**
     * Returns the minimum size required for this widget.
     * Inherited from QWidget.
     **/
    virtual QSize minimumSizeHint() const;

    /**
     * Return 'true' if 'selectable' has mixed multiversion flags,
     * 'false' if all its pool items are of the same kind
     * (all multiversion or all non-multiversion).
     **/
    static bool isMixedMultiVersion( ZyppSel selectable );

    /**
     * Return the cached value for the current selectable.
     **/
    bool isMixedMultiVersion() const { return _isMixedMultiVersion; }

    /**
     * Negotiate between multiversion and non-multiversion packages if there
     * are both kinds in that selectable. 'newSelected' is the item the user
     * chose to install.
     *
     * This returns 'true' if status setting etc. is already handled inside
     * this function, 'false' otherwise.
     **/
    bool handleMixedMultiVersion( YQPkgMultiVersion * newSelected );

    /**
     * Return the selectable of this details view.
     **/
    ZyppSel selectable() const { return _selectable; }

    /**
     * Set the colors of a widget for a retracted zyppObj.
     **/
    static void setRetractedColor( QWidget * widget );

    /**
     * Return 'true' if 'installed' is retraced, i.e. if there is an available
     * ZyppObj with the same edition, architeture and vendor that has the
     * 'retracted' flag set.
     **/
    static bool installedIsRetracted( ZyppSel selectable, ZyppObj installed );


public slots:

    /**
     * Show details for the specified package.
     * Delayed (optimized) display if this is embedded into a QTabWidget
     * parent: In this case, wait until this page becomes visible.
     **/
    void showDetailsIfVisible( ZyppSel selectable );

    /**
     * Show data for the current package.
     **/
    void reload( int newCurrent );


signals:

    /**
     * Emitted when the user changes the candidate.
     **/
    void candidateChanged( ZyppObj newCandidate );

    /**
     * Emitted when the status of any package changed.
     **/
    void statusChanged();


protected slots:

    /**
     * Check for changed candidates
     **/
    void checkForChangedCandidate();


protected:

    /**
     * Show details for the specified package.
     **/
    void showDetails( ZyppSel selectable );

    /**
     * Ask user if he really wants to install incompatible package versions.
     * Return 'true' if he hits [Continue], 'false' if [Cancel].
     **/
    bool mixedMultiVersionPopup( bool multiversion ) const;

    /**
     * Check if any package version is marked for installation where its
     * 'multiversion' flag is set to 'multiversion'.
     **/
    bool anyMultiVersionToInstall( bool multiversion ) const;

    /**
     * Unselect all multiversion package versions.
     **/
    void unselectAllMultiVersion();

    // Data members

    QTabWidget  *  _parentTab;
    ZyppSel        _selectable;
    bool           _isMixedMultiVersion;
    QButtonGroup * _buttonGroup;
    QVBoxLayout *  _layout;
};


class YQPkgVersion: public QRadioButton
{
public:

    /**
     * Constructor. Creates a YQPkgVersion item that corresponds to the package
     * manager object that 'pkg' refers to.
     **/
    YQPkgVersion( QWidget * parent,
                  ZyppSel   selectable,
                  ZyppObj   zyppObj );

    /**
     * Destructor
     **/
    virtual ~YQPkgVersion();

    /**
     * Returns the original ZYPP object
     **/
    ZyppObj zyppObj() const { return _zyppObj; }

    /**
     * Returns the original ZYPP selectable
     **/
    ZyppSel selectable() const { return _selectable; }


protected:

    // Data members

    ZyppSel _selectable;
    ZyppObj _zyppObj;
};



class YQPkgMultiVersion: public QCheckBox
{
    Q_OBJECT

public:

    /**
     * Constructor.
     **/
    YQPkgMultiVersion( YQPkgVersionsView * parent,
                       ZyppSel             selectable,
                       ZyppPoolItem        zyppPoolItem );

    /**
     * Destructor
     **/
    virtual ~YQPkgMultiVersion();

    /**
     * Returns the original ZYPP object
     **/
    ZyppPoolItem zyppPoolItem() const { return _zyppPoolItem; }

    /**
     * Returns the original ZYPP selectable
     **/
    ZyppSel selectable() const { return _selectable; }

    /**
     * Paints checkboxes with status icons instead of a checkmark
     **/
    void paintEvent(QPaintEvent *);

signals:

    /**
     * Emitted when the status of this package version is changed.
     **/
    void statusChanged();


protected slots:

    void slotIconClicked();


protected:

    /**
     * Cycle the package status to the next valid value.
     **/
    void cycleStatus();

    void setStatus( ZyppStatus newStatus );

    QPixmap statusIcon( ZyppStatus status );

    //
    // Data members
    //

    YQPkgVersionsView * _parent;
    ZyppSel             _selectable;
    ZyppPoolItem        _zyppPoolItem;
};


#endif // ifndef YQPkgVersionsView_h
