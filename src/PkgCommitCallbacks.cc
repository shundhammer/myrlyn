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


#include "Logger.h"
#include "Exception.h"
#include "PkgCommitCallbacks.h"


PkgCommitCallbacks::PkgCommitCallbacks()
{
    _pkgDownloadCallback.connect();
    _pkgInstallCallback.connect();
    _pkgRemoveCallback.connect();
    _fileConflictsCheckCallback.connect();
}

PkgCommitCallbacks::~PkgCommitCallbacks()
{
    // Each zypp::Callback disconnects automatically in the dtor.
}


//
//----------------------------------------------------------------------
//


PkgCommitSignalForwarder * PkgCommitSignalForwarder::_instance = 0;


PkgCommitSignalForwarder * PkgCommitSignalForwarder::instance()
{
    if ( ! _instance )
    {
        _instance = new PkgCommitSignalForwarder();
        CHECK_NEW( _instance );
    }

    return _instance;
}


void PkgCommitSignalForwarder::reset()
{
    _doAbort = false;
}


void PkgCommitSignalForwarder::connectAll( QObject * receiver )
{
    connect( instance(), SIGNAL( pkgDownloadStart    ( ZyppRes ) ),
             receiver,   SLOT  ( pkgDownloadStart    ( ZyppRes ) ) );

    connect( instance(), SIGNAL( pkgDownloadProgress ( ZyppRes, int ) ),
             receiver,   SLOT  ( pkgDownloadProgress ( ZyppRes, int ) ) );

    connect( instance(), SIGNAL( pkgDownloadEnd      ( ZyppRes ) ),
             receiver,   SLOT  ( pkgDownloadEnd      ( ZyppRes ) ) );

    connect( instance(), SIGNAL( pkgCachedNotify     ( ZyppRes ) ),
             receiver,   SLOT  ( pkgCachedNotify     ( ZyppRes ) ) );

    connect( instance(), SIGNAL( pkgDownloadError    ( ZyppRes, const QString & ) ),
             receiver,   SLOT  ( pkgDownloadError    ( ZyppRes, const QString & ) ) );


    connect( instance(), SIGNAL( pkgInstallStart     ( ZyppRes ) ),
             receiver,   SLOT  ( pkgInstallStart     ( ZyppRes ) ) );

    connect( instance(), SIGNAL( pkgInstallProgress  ( ZyppRes, int ) ),
             receiver,   SLOT  ( pkgInstallProgress  ( ZyppRes, int ) ) );

    connect( instance(), SIGNAL( pkgInstallEnd       ( ZyppRes ) ),
             receiver,   SLOT  ( pkgInstallEnd       ( ZyppRes ) ) );

    connect( instance(), SIGNAL( pkgInstallError     ( ZyppRes, const QString & ) ),
             receiver,   SLOT  ( pkgInstallError     ( ZyppRes, const QString & ) ) );


    connect( instance(), SIGNAL( pkgRemoveStart      ( ZyppRes ) ),
             receiver,   SLOT  ( pkgRemoveStart      ( ZyppRes ) ) );

    connect( instance(), SIGNAL( pkgRemoveProgress   ( ZyppRes, int ) ),
             receiver,   SLOT  ( pkgRemoveProgress   ( ZyppRes, int ) ) );

    connect( instance(), SIGNAL( pkgRemoveEnd        ( ZyppRes ) ),
             receiver,   SLOT  ( pkgRemoveEnd        ( ZyppRes ) ) );

    connect( instance(), SIGNAL( pkgRemoveError      ( ZyppRes, const QString & ) ),
             receiver,   SLOT  ( pkgRemoveError      ( ZyppRes, const QString & ) ) );


    connect( instance(), SIGNAL( fileConflictsCheckStart() ),
             receiver,   SLOT  ( fileConflictsCheckStart() ) );

    connect( instance(), SIGNAL( fileConflictsCheckProgress( int ) ),
             receiver,   SLOT  ( fileConflictsCheckProgress( int ) ) );

    connect( instance(), SIGNAL( fileConflictsCheckResult( QStringList ) ),
             receiver,   SLOT  ( fileConflictsCheckResult( QStringList ) ) );


    connect( receiver,   SIGNAL( abortCommit() ),
             this,       SLOT  ( abortCommit() ) );
}
