/*  ---------------------------------------------------------
               __  __            _
              |  \/  |_   _ _ __| |_   _ _ __
              | |\/| | | | | '__| | | | | '_ \
              | |  | | |_| | |  | | |_| | | | |
              |_|  |_|\__, |_|  |_|\__, |_| |_|
                      |___/        |___/
    ---------------------------------------------------------

    Project:  Myrlyn Package Manager GUI
    Copyright (c) Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
    License:  GPL V2 - See file LICENSE for details.

 */

#ifndef ZyppHistory_h
#define ZyppHistory_h

#include <QString>
#include <QList>
#include <QtAlgorithms>  // qDeleteAll()


/**
 * Singleton class for the content of the zypp history file where libzypp
 * events are recorded, like installing / updating / removing a package, adding
 * / removing / modifying a repo, or applying a patch.
 *
 * The default file to use is /var/log/zypp/history, but this has restricted
 * access for non-root users. For debugging and testing the parser, or for
 * analyzing zypp history files from another machine, use the '--zypp-history
 * /path/to/zypp/history' command line option or the static setFileName()
 * method here.
 *
 * This singleton class caches its data. Use the dropCache() method when the
 * underlying data might have changed, such as after a package transaction
 * (installing / updating / removing packages) was done, and the user returns
 * to the Myrlyn main screen.
 **/
class ZyppHistory
{
public:

    enum EventType
    {                     // Field #1 in the zypp-history file:
        CommandEvent,     //   command
        PkgInstallEvent,  //   install
        PkgRemoveEvent,   //   remove
        RepoAddEvent,     //   radd
        RepoRemoveEvent,  //   rremove
        RepoUrlEvent,     //   rurl
        RepoAliasEvent,   //   ralias
        PatchEvent        //   patch
    };

    /**
     * Base class for all the events
     **/
    struct Event
    {
        QString   timestamp;    // "2025-12-28 14:15:26"
        EventType eventType;

        QString date() const;   // "2025-12-28"
        QString time() const;   // "14:15:26"
    };


    typedef QList<Event *> EventList;


    /**
     * Base class for events that can have child events, like CommandEvent or
     * PatchEvent.
     *
     * Those are the events triggered in that command or session, i.e. with a
     * single 'zypper' command, or in a single Myrlyn or YaST session; or the
     * PkgEvents (PkgInstallEvent, PkgRemoveEvent)
     **/
    struct ParentEvent: public Event
    {

        const EventList & childEvents() { return _childEvents; }

        qsizetype childEventsCount() const { return   _childEvents.size();    }
        bool      hasChildEvents()   const { return ! _childEvents.isEmpty(); }

        /**
         * Add a child event. This class takes over ownership of the child and
         * will delete it in its destructor.
         **/
        void addChild( Event * childEvent );

        /**
         * Release any unused memory in the childEvents list.
         *
         * This is useful because QList optimizes memory allocation by
         * reserving more capacity in advance to avoid repeatedly re-allocating
         * the the space for its items. But when we know for sure that we are
         * not going to add any more items, we can tell it to release all
         * reserved slots that are not yet used.
         *
         * As the zypp history file is parsed, the next "command" or "patch"
         * line marks the end of the previous one, so we can "finalize" that
         * previous command or patch by calling this method.
         **/
        void squeezeChildEvents() { _childEvents.squeeze(); }

        /**
         * Destructor. This deletes all child events.
         **/
        virtual ~ParentEvent() { qDeleteAll( _childEvents ); }

    protected:

        EventList _childEvents;
    };


    /**
     * The command / command session event; the only type of event that can
     * have child events. Those are the events triggered in that command or
     * session, i.e. with a single 'zypper' command, or in a single Myrlyn or
     * YaST session.
     **/
    struct CommandEvent: public ParentEvent
    {
        QString command;     // "yast2 sw_single qt", "zypper in qdirstat ..."
        QString rawCommand;  // "/usr/bin/ruby3.3 /usr/lib/YaST2/bin/y2start sw_single qt"
    };


    /**
     * Package events: PkgInstallEvent, PkgRemoveEvent. Notice that the zypper
     * history does not make a difference between installing and updating a
     * package.
     **/
    struct PkgEvent: public Event
    {
        QString pkgName;
        QString version;
        QString arch;
        QString repoAlias;
    };


    /**
     * Repo events: RepoAddEvent, RepoRemoveEvent, RepoUrlEvent, RepoAliasEvent.
     *
     * Those may or may not be interesting for the user, but they are in the
     * zypp history, so they are preserved here.
     **/
    struct RepoEvent: public Event
    {
        QString repoAlias;
        QString url;

        QString oldRepoAlias;
        QString oldUrl;
    };


    /**
     * Patch events.
     **/
    struct PatchEvent: public ParentEvent
    {
        QString patchName;
        QString version;
        QString arch;
        QString repoAlias;
        QString patchState;  // "applied", "needed"
    };


protected:

    /**
     * Constructor. Use the static instance() method instead.
     **/
    ZyppHistory();

    /**
     * Destructor. Use the clear() or the static dropCache() method instead if
     * needed.
     **/
    virtual ~ZyppHistory();


public:

    /**
     * Return the instance of the singleton of this class.
     * Create it if it doesn't exist yet.
     **/
    static ZyppHistory * instance();

    /**
     * Read and parse the zypp history file if that hasn't been done yet.
     * Return 'true' on success.
     *
     * This may throw exceptions.
     **/
    bool read();

    /**
     * Return the zypp history events. Make sure to call read() first.
     **/
    const EventList & events() const { return _events; }

    /**
     * Clear the content.
     **/
    void clear();

    /**
     * Drop the cached content of the zypp history events.
     **/
    static void dropCache();

    /**
     * Set the filename (with full path) of the zypp history file to use.
     * An empty 'fileName' resets it to the default value
     * (/var/log/zypp/history).
     **/
    static void setFileName( const QString & fileName );

    /**
     * Return the filename of the zypp history file.
     **/
    static QString fileName() { return _fileName; }

protected:

    static ZyppHistory * _instance;
    static QString       _fileName;

    EventList _events;
    bool      _dirty;

};  // class ZyppHistory

#endif // ZyppHistory_h
