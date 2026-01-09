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

#ifndef ZyppHistoryEvents_h
#define ZyppHistoryEvents_h


#include <QString>
#include <QList>
#include <QtAlgorithms>  // qDeleteAll()


/**
 * Classes and defininitions for the individual zypp history events in
 * /var/log/zypp/history,
 **/
namespace ZyppHistoryEvents
{
    // C++11: To be used as EventType::Unknown, EventType::Comand etc.
    enum class EventType
    {                  // Field #1 in the zypp-history file:
        Unknown = -1,  //
        Command,       //   command
        PkgInstall,    //   install
        PkgRemove,     //   remove
        RepoAdd,       //   radd
        RepoRemove,    //   rremove
        RepoUrl,       //   rurl
        RepoAlias,     //   ralias
        Patch          //   patch
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

        virtual ~Event() {}     // To make it polymorphic for dynamic_cast
    };


    typedef QList<Event *> EventList;


    /**
     * Base class for events that can have child events, like CommandEvent.
     *
     * Child Events are the events triggered in that command or session,
     * i.e. with a single 'zypper' command, or in a single Myrlyn or YaST
     * session; or the PkgEvents (PkgInstallEvent, PkgRemoveEvent)
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
        void addChildEvent( Event * childEvent );

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
        QString command;     // "YaST sw_single", "zypper in qdirstat ..."
        QString rawCommand;  // "/usr/bin/ruby3.3 /usr/lib/YaST2/bin/y2start sw_single qt"

        CommandEvent() { eventType = EventType::Command; }
    };


    /**
     * Package events: PkgInstallEvent, PkgRemoveEvent. Notice that the zypper
     * history does not make a difference between installing and updating a
     * package.
     **/
    struct PkgEvent: public Event
    {
        QString name;
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
    struct PatchEvent: public Event
    {
        QString name;
        QString version;
        QString arch;
        QString repoAlias;
        QString patchState;  // "applied", "needed"
    };

}  // namespace ZyppHistoryEvents


#endif // ZyppHistoryEvents_h
