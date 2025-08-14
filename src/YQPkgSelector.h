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


#ifndef YQPkgSelector_h
#define YQPkgSelector_h

#include <QWidget>
#include <QColor>

#include "YQPkgSelectorBase.h"
#include "YQPkgObjList.h"

class QLabel;
class QPushButton;
class QTabWidget;
class QMenu;
class QMenuBar;

class YQPkgChangeLogView;
class YQPkgDependenciesView;
class YQPkgDescriptionView;
class YQPkgFileListView;
class YQPkgFilterTab;
class YQPkgLangList;
class YQPkgList;
class YQPkgClassificationFilterView;
class YQPkgPatchFilterView;
class YQPkgPatternList;
class YQPkgRepoFilterView;
class YQPkgRpmGroupsFilterView;
class YQPkgSearchFilterView;
class YQPkgServiceFilterView;
class YQPkgStatusFilterView;
class YQPkgTechnicalDetailsView;
class YQPkgUpdatesFilterView;
class YQPkgVersionsView;

class YQPkgSelector : public YQPkgSelectorBase
{
    Q_OBJECT

public:

    /**
     * Constructor.
     **/
    YQPkgSelector( QWidget * parent );

    /**
     * Destructor.
     **/
    ~YQPkgSelector();

    /**
     * Return the instance of this class or 0 if there is none.
     *
     * This is not a singleton, but in each normal application there is no more
     * than one instance, and during the lifetime of any widgets inside the
     * YQPkgSelector it is safe to assume that the YQPkgSelector also exists.
     **/
    static YQPkgSelector * instance() { return _instance; }

    /**
     * Return the package list of this istance or 0 if it hasn't been created
     * yet.
     **/
    YQPkgList * pkgList() const { return _pkgList; }

    /**
     * Return the exclude rule for "-devel" packages.
     **/
    YQPkgObjList::ExcludeRule * excludeRuleDevelPkgs() const { return _excludeDevelPkgs; }

    /**
     * Return the exclude rule for "-debuginfo / -debugsource" packages.
     **/
    YQPkgObjList::ExcludeRule * excludeRuleDebugInfoPkgs() const { return _excludeDebugInfoPkgs; }

    /**
     * Flag: Use the RPM groups filter view?
     **/
    bool useRpmGroups() const { return _useRpmGroups; }



public slots:

    /**
     * Reset the data in connected views and reset the resolver.
     **/
    void reset();

    /**
     * Start the repo configuration.
     **/
    void configRepos();

    /**
     * Resolve package dependencies manually.
     *
     * Returns QDialog::Accepted or QDialog::Rejected.
     **/
    int manualResolvePackageDependencies();

    /**
     * Automatically resolve package dependencies if desired
     * (if the "auto check" checkbox is on).
     **/
    void autoResolveDependencies();

    /**
     * Install any -devel package for packages that are installed or marked for
     * installation
     **/
    void installDevelPkgs();

    /**
     * Install available -debuginfo packages for packages that are installed or
     * marked for installation
     **/
    void installDebugInfoPkgs();

    /**
     * Install available -debugsource packages for packages that are installed
     * or marked for installation
     **/
    void installDebugSourcePkgs();

    /**
     * Install recommended packages for packages that are installed
     **/
    void installRecommendedPkgs();

    /**
     * Install any subpackage that ends with 'suffix' for packages that are
     * installed or marked for installation
     **/
    void installSubPkgs( const QString & suffix );

    /**
     * Enable or disable the package exclude rules (show or suppress -debuginfo
     * or -devel packages) according to the current menu settings and apply the
     * exclude rules.
     **/
    void pkgExcludeDebugChanged( bool on );
    void pkgExcludeDevelChanged( bool on );


    /**
     * Enable or disable verify system mode of the solver
     **/
    void pkgVerifySytemModeChanged( bool on );

    /**
     * Install recommended packages
     **/
    void pkgInstallRecommendedChanged( bool on );

    /**
     * Enable or disable CleandepsOnRemove of the solver
     * (= Cleanup when deleting packages)
     **/
    void pkgCleanDepsOnRemoveChanged( bool on );

    /**
     * Enable or disable vendor change allowed of the solver
     **/
    void pkgAllowVendorChangeChanged( bool on );

    /**
     * Display online help.
     * This opens the project's home page in an external web browser.
     **/
    void help();

    /**
     * Help about repository configuration
     * (opens in external web browser)
     **/
    void helpRepoConfig();

    /**
     * Help about root authentication
     * (opens in external web browser)
     **/
    void helpRootAuth();

    /**
     * Display an about page.
     **/
    void about();

    /**
     * hides or shows the repository upgrade message
     */
    void updateSwitchRepoLabels();

    /**
     * Read the settings from the config file
     * (before the widgets are created)
     */
    void readSettingsEarly();

    /**
     * Read the settings from the config file
     * (after the widgets are created)
     */
    void readSettings();

    /**
     * Write the settings to the config file
     */
    void writeSettings();


protected slots:

    /**
     * Add the "Patches" filter view upon hotkey (F2).
     **/
    void hotkeyAddPatchFilterView();

    /**
     * Set the status of all installed packages (all in the pool, not only
     * those currently displayed in the package list) to "update", if there is
     * a candidate package that is newer than the installed package.
     **/
    void globalUpdatePkg() { globalUpdatePkg( false ); }

    /**
     * Set the status of all installed packages (all in the pool, not only
     * those currently displayed in the package list) to "update", even if the
     * candidate package is not newer than the installed package.
     **/
    void globalUpdatePkgForce() { globalUpdatePkg( true ); }

    /**
     * Show all products in a popup dialog.
     **/
    void showProducts();

    /**
     * Show dialog for package manager history
     **/
    void showHistory();

    /**
     * a link in the repo upgrade label was clicked
     **/
    void switchToRepo( const QString & url );

    /**
     * Show the busy cursor (clock)
     */
    void busyCursor();

    /**
     * Show the standard cursor (arrow)
     */
    void normalCursor();


public:

    /**
     * Open a URL in the desktop's default browser
     * (using the /usr/bin/xdg-open command).
     **/
    static void openInBrowser( const QString & url );


protected:

    /**
     * Show a number of fallback filter view pages if none were read from the
     * settings.
     **/
    void showFallbackPages();

    /**
     * Regardless of user proferences (saved pages), override the initial page
     * for very important special cases.
     **/
    void overrideInitialPage();

    /**
     * Trigger the first dependency resolver run with a single-shot timer.
     **/
    void firstSolverRun();


    // Layout methods - create and layout widgets

    void        basicLayout();
    void        createFilterViews();

    QWidget *   layoutRightPane    ( QWidget * parent );
    void        layoutPkgList      ( QWidget * parent );
    void        layoutNotifications( QWidget * parent );
    void        layoutDetailsViews ( QWidget * parent );
    void        layoutButtons      ( QWidget * parent );
    void        layoutMenuBar      ( QWidget * parent );


    // Create the various filter views

    void createSearchFilterView();
    void createPatchFilterView( bool force = false );
    void createUpdatesFilterView();
    void createRepoFilterView();
    void createRpmGroupsFilterView();
    void createServiceFilterView();
    void createPatternsFilterView();
    void createPkgClassificationFilterView();
    void createLanguagesFilterView();
    void createStatusFilterView();

    /**
     * Update the tab labels of filter views that can have a numeric value with
     * that number: "Patches (4)", "Updates (17)"
     **/
    void updatePageLabels();

    /**
     * Update the tab label of a filter view page with a number. Leave it empty
     * if the number is < 1:  "Patches (4)", "Updates (17)".
     **/
    void updatePageLabel( const QString & rawLabel,
                          QWidget *       page,
                          int             count );

    /**
     * Establish Qt signal / slot connections.
     *
     * This really needs to be a separate method to make sure all affected
     * wigets are created at this point.
     **/
    void makeConnections();

    /**
     * Add pulldown menus to the menu bar.
     *
     * This really needs to be a separate method to make sure all affected
     * wigets are created at this point.
     **/
    void addMenus();

    /**
     * Connect a filter view that provides the usual signals with a package
     * list. By convention, filter views provide the following signals:
     *    filterStart()
     *    filterMatch()
     *    filterFinished()
     *    updatePackages()  (optional)
     **/
    void connectFilter( QWidget *       filter,
                        QWidget *       pkgList,
                        bool            hasUpdateSignal = true );

    /**
     * Connect the patch filter view. Caution: Possible bootstrap problem!
     **/
    void connectPatchFilterView();

    /**
     * Connect the pattern list / filter view.
     **/
    void connectPatternList();

    /**
     * Set the status of all installed packages (all in the pool, not only
     * those currently displayed in the package list) to "update" and switch to
     * the "Installation Summary" view afterwards.
     *
     * 'force' indicates if this should also be done if the the candidate
     * package is not newer than the installed package.
     **/
    void globalUpdatePkg( bool force );

    /**
     * Return 'true' if the "Automatic Changes" dialog should be shown upon
     * accepting the changes.
     *
     * Reimplemented from YQPkgSelectorBase to avoid showing this dialog if the
     * user already sees the "Installation Summary" view page in its default
     * configuration which already shows the automatic changes.
     **/
    virtual bool showAutoChangesDialog() override;

    /**
     * Return 'true' if any selectable has any retracted package version
     * installed.
     **/
    bool anyRetractedPkgInstalled();

    /**
     * Read settings for the resolver from the config file
     * (~/.config/openSUSE/Myrlyn.conf)
     **/
    void readResolverSettings();

    /**
     * Write resolver settings to the config file
     * (~/.config/openSUSE/Myrlyn.conf)
     **/
    void writeResolverSettings();

    /**
     * Basic HTML formatting: Embed text into <p> ... </p>
     **/
    static QString para( const QString & text );

    /**
     * Basic HTML formatting: Embed text into <li> ... </li>
     **/
    static QString listItem( const QString & text );


    //
    // Data members
    //

    YQPkgList *                         _pkgList;
    YQPkgFilterTab *                    _filters;

    // Filter Views
    YQPkgSearchFilterView *             _searchFilterView;
    YQPkgPatchFilterView *              _patchFilterView;
    YQPkgUpdatesFilterView *            _updatesFilterView;
    YQPkgRepoFilterView *               _repoFilterView;
    YQPkgRpmGroupsFilterView *          _rpmGroupsFilterView;
    YQPkgServiceFilterView *            _serviceFilterView;
    YQPkgClassificationFilterView *     _pkgClassificationFilterView;
    YQPkgPatternList *                  _patternList;
    YQPkgStatusFilterView *             _statusFilterView;
    YQPkgLangList *                     _langList;

    // Other widgets
    YQPkgVersionsView *                 _pkgVersionsView;
    QWidget *                           _notificationsArea;
    QLabel *                            _switchToRepoLabel;
    QLabel *                            _cancelSwitchingToRepoLabel;

    // Menus
    QMenuBar *                          _menuBar;
    QMenu *                             _pkgMenu;
    QMenu *                             _patchMenu;

    // Actions
    QAction *                           _autoDependenciesAction;
    QAction *                           _showDevelAction;
    QAction *                           _showDebugAction;
    QAction *                           _verifySystemModeAction;
    QAction *                           _installRecommendedAction;
    QAction *                           _cleanDepsOnRemoveAction;
    QAction *                           _allowVendorChangeAction;

    YQPkgObjList::ExcludeRule *         _excludeDevelPkgs;
    YQPkgObjList::ExcludeRule *         _excludeDebugInfoPkgs;

    bool                                _useRpmGroups;

    static YQPkgSelector *              _instance;
};



#endif // YQPkgSelector_h
