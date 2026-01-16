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

#ifndef ZyppHistoryFilter_h
#define ZyppHistoryFilter_h

#include <QString>

#include "SearchFilter.h"
#include "ZyppHistoryEvents.h"


/**
 * Abstract base class functor for zypp history filters.
 *
 * This defines the API for applications using derived classes: operator() is
 * used to check if an event should be accepted (-> true) or rejected.
 *
 * Usage:
 *
 *   class MyFilter: public ZyppHistoryFilter
 *   {
 *   public:
 *
 *     MyFilter( int userData )
 *       : _userData( userData )
 *     {}
 *
 *     bool operator() ( ... *event ) override
 *     {
 *       return event->foo == userData;
 *     }
 *   };
 *
 *   main()
 *   {
 *     MyFilter filter( 42 );
 *
 *     for ( Event ev: events() )
 *       if ( filter( event ) )
 *         filteredEvents << ev;
 *   }
 **/
class ZyppHistoryFilter
{
protected:
    ZyppHistoryFilter() {};
    virtual ~ZyppHistoryFilter() {};

public:

    /**
     * Overloaded function call operator: Check if the specified event should
     * be accepted (kept) (-> true) or rejected (filtered out) (->false).
     *
     * Derived classes are required to implement this.
     **/
    virtual bool operator() ( ZyppHistoryEvents::Event * event ) = 0;

    /**
     * Return a (translated) concise textual description for the user what this
     * filter does.
     **/
    const QString & description() { return _description; }

protected:

    QString _description;
};


/**
 * Simple zypp history event filter that accepts all events of the specified
 * type.
 *
 * Usage:
 *
 *   ZyppHistoryEventTypeFilter myFilter( ZyppHistoryEvents::EventType PkgRemove );
 *
 *   for ( Event event: events() )
 *     if ( myFilter( event ) )
 *       keepEvent( event );
 **/
class ZyppHistoryEventTypeFilter: public ZyppHistoryFilter
{
public:
    ZyppHistoryEventTypeFilter( ZyppHistoryEvents::EventType eventType,
                                const QString & description = QString() )
        : _eventType( eventType )
        { _description = description; }

    virtual ~ZyppHistoryEventTypeFilter() {}

    virtual bool operator() ( ZyppHistoryEvents::Event * event ) override
        { return event && event->eventType == _eventType; }

protected:
    ZyppHistoryEvents::EventType _eventType;
};


/**
 * Zypp history events filter that accepts all repo events
 * (RepoAdd, RepoRemove, RepoUrl, RepoAlias).
 **/
class ZyppHistoryRepoEventsFilter: public ZyppHistoryFilter
{
public:
    ZyppHistoryRepoEventsFilter();
    virtual ~ZyppHistoryRepoEventsFilter() {}

    virtual bool operator() ( ZyppHistoryEvents::Event * event ) override;
};


/**
 * Abstract base class for filters that use a SearchFilter with a search term
 * and a search mode.
 **/
class ZyppHistorySearchFilter: public ZyppHistoryFilter
{
protected:
    ZyppHistorySearchFilter( const QString &          searchPattern,
                             SearchFilter::FilterMode filterMode        = SearchFilter::Auto,
                             SearchFilter::FilterMode defaultFilterMode = SearchFilter::StartsWith )
        : _searchFilter( searchPattern, filterMode, defaultFilterMode )
        {}

    virtual ~ZyppHistorySearchFilter() {}

    // Notice that derived classes are still required to implement operator()

protected:
    SearchFilter _searchFilter;
};


/**
 * Event filter that accepts all PkgEvents with package name 'searchPattern'.
 **/
class ZyppHistoryPkgNameFilter: public ZyppHistorySearchFilter
{
public:
    ZyppHistoryPkgNameFilter( const QString &          searchPattern,
                              SearchFilter::FilterMode filterMode        = SearchFilter::Auto,
                              SearchFilter::FilterMode defaultFilterMode = SearchFilter::StartsWith );

    virtual ~ZyppHistoryPkgNameFilter() {}

    virtual bool operator() ( ZyppHistoryEvents::Event * event ) override;
};


/**
 * Event filter that accepts all PkgEvents with repo alias 'searchPattern'.
 **/
class ZyppHistoryPkgRepoAliasFilter: public ZyppHistorySearchFilter
{
public:
    ZyppHistoryPkgRepoAliasFilter( const QString &          searchPattern,
                                   SearchFilter::FilterMode filterMode        = SearchFilter::Auto,
                                   SearchFilter::FilterMode defaultFilterMode = SearchFilter::StartsWith );

    virtual ~ZyppHistoryPkgRepoAliasFilter() {}

    virtual bool operator() ( ZyppHistoryEvents::Event * event ) override;
};



#endif  // ZyppHistoryFilter_h
