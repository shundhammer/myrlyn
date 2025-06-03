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


#ifndef YQPkgConflictList_h
#define YQPkgConflictList_h


#include <QMap>
#include <QFile>
#include <QScrollArea>

#include <zypp/ProblemTypes.h>

class QVBoxLayout;
class QRadioButton;
class YQPkgConflict;

typedef zypp::ResolverProblem_Ptr  ZyppProblem;
typedef zypp::ResolverProblemList  ZyppProblemList;
typedef zypp::ProblemSolution_Ptr  ZyppSolution;
typedef zypp::ProblemSolutionList  ZyppSolutionList;


/**
 * Display package dependency conflicts in a tree list and let the user
 * choose how to resolve each conflict.
 **/
class YQPkgConflictList : public QScrollArea
{
    Q_OBJECT

public:
    /**
     * Constructor.
     **/
    YQPkgConflictList( QWidget * parent );

    /**
     * Destructor.
     **/
    virtual ~YQPkgConflictList();

    /**
     * Fill the list with the specified problems.
     **/
    void fill( ZyppProblemList problemList );

    /**
     * Check if the conflict list is empty.
     **/
    bool isEmpty() const { return count() == 0; }

    /**
     * Returns the number of conflicts in the list.
     **/
    int count() const { return _conflicts.count(); }


public slots:

    /**
     * Apply the choices the user made.
     **/
    void applyResolutions();

    /**
     * Ask for a file name and save the current conflict list to file.
     **/
    void askSaveToFile() const;

    void clear();

    void relayout();

public:

    /**
     * Save the conflict list in its current state to a file. Retains the
     * current 'expanded' state, i.e. writes only those entries that are
     * currently open (not collapsed) in the tree.
     *
     * Posts error popups if 'interactive' is 'true' (only log entries
     * otherwise).
     **/
    void saveToFile( const QString filename, bool interactive ) const;

protected:

    QList<YQPkgConflict*> _conflicts;
    QVBoxLayout *         _layout;

signals:

    /**
     * Update package states - they may have changed.
     **/
    void updatePackages();

};



/**
 * Root item for each individual conflict
 **/
class YQPkgConflict: public QFrame
{
    Q_OBJECT

public:

    /**
     * Constructor.
     **/
    YQPkgConflict( QWidget *   parent,
                   ZyppProblem problem );

    /**
     * Destructor.
     **/
    virtual ~YQPkgConflict() {}

    /**
     * Returns the corresponding ResolverProblem.
     **/
    ZyppProblem problem() const { return _problem; }

    /**
     * Returns the resolution the user selected
     * or 0 if he didn't select one
     **/
    ZyppSolution userSelectedResolution();

    /**
     * save one item to file.
     **/
    void saveToFile( QFile & file ) const;


protected slots:

    void detailsExpanded();

signals:

    void expanded();

protected:

    /**
     * Format the text heading line for this item.
     **/
    void formatHeading();

    /**
     * Add suggestions how to resolve this conflict.
     **/
    void addSolutions();

    //
    // Data members
    //

    ZyppProblem                        _problem;
    QLabel *                           _resolutionsHeader;
    QList<ZyppSolution>                _resolutions;
    QMap<QRadioButton *, ZyppSolution> _solutions;
    QMap<QLabel *, ZyppSolution>       _details;
    QVBoxLayout *                      _layout;
};

#endif // ifndef YQPkgConflictList_h
