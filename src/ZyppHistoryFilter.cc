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


#include "YQi18n.h"
#include "ZyppHistoryEvents.h"
#include "ZyppHistoryFilter.h"

using namespace ZyppHistoryEvents;


ZyppHistoryRepoEventsFilter::ZyppHistoryRepoEventsFilter()
{
    _description = _( "Only repo events" );
}


bool
ZyppHistoryRepoEventsFilter::operator() ( Event * event )
{
    if ( ! event )
        return false;  // reject

    switch ( event->eventType )
    {
        case EventType::RepoAdd:
        case EventType::RepoRemove:
        case EventType::RepoUrl:
        case EventType::RepoAlias:
            return true;   // accept

        default:
            return false;  // reject
    }
}




ZyppHistoryPkgNameFilter::ZyppHistoryPkgNameFilter( const QString &          searchPattern,
                                                    SearchFilter::FilterMode filterMode,
                                                    SearchFilter::FilterMode defaultFilterMode )
    : ZyppHistorySearchFilter( searchPattern, filterMode, defaultFilterMode )
{
    _description = _( "Package name \"%1\"" ).arg( searchPattern );
}


bool
ZyppHistoryPkgNameFilter::operator() ( Event * event )
{
    PkgEvent * pkgEvent = dynamic_cast<PkgEvent *>( event );

    if ( pkgEvent )
        return _searchFilter.matches( pkgEvent->name );
    else
        return false; // reject
}




ZyppHistoryPkgRepoAliasFilter::ZyppHistoryPkgRepoAliasFilter( const QString &          searchPattern,
                                                              SearchFilter::FilterMode filterMode,
                                                              SearchFilter::FilterMode defaultFilterMode )
    : ZyppHistorySearchFilter( searchPattern, filterMode, defaultFilterMode )
{
    _description = _( "Packages from repo \"%1\"" ).arg( searchPattern );
}


bool
ZyppHistoryPkgRepoAliasFilter::operator() ( Event * event )
{
    PkgEvent * pkgEvent = dynamic_cast<PkgEvent *>( event );

    if ( pkgEvent )
        return _searchFilter.matches( pkgEvent->repoAlias );
    else
        return false; // reject
}
