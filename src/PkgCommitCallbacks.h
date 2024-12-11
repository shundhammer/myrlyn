/*  ------------------------------------------------------
              __   _____  ____  _
              \ \ / / _ \|  _ \| | ____ _
               \ V / | | | |_) | |/ / _` |
                | || |_| |  __/|   < (_| |
                |_| \__\_\_|   |_|\_\__, |
                                    |___/
    ------------------------------------------------------

    Project:  YQPkg Package Selector
    Copyright (c) 2024 SUSE LLC
    License:  GPL V2 - See file LICENSE for details.

    Textdomain "qt-pkg"
 */


#ifndef PkgCommitCallbacks_h
#define PkgCommitCallbacks_h


#include <QObject>

#include <zypp/Resolvable.h>
#include <zypp/Url.h>
#include <zypp/ZYppCallbacks.h>

#include "YQZypp.h"     // ZyppRes


class PkgCommitSignalForwarder;

using zypp::Pathname;
using zypp::Url;


/**
 * Signal forwarder for the callbacks so they can send Qt signals to a receiver
 * QObject without being subclasses of QObject themselves.
 *
 * This is a singleton class.
 **/
class PkgCommitSignalForwarder: public QObject
{
   Q_OBJECT

protected:

    /**
     * Constructor. Use instance() instead.
     **/
    PkgCommitSignalForwarder()
        : QObject()
        {}

public:

    /**
     * Destructor.
     **/
    virtual ~PkgCommitSignalForwarder() { _instance = 0; }

    /**
     * Return the singleton of this class. Create it if it doesn't exist yet.
     **/
    static PkgCommitSignalForwarder * instance();

    /**
     * Connect all signals to slots with the same name in 'receiver'.
     **/
    void connectAll( QObject * receiver );


signals:

    //
    // The receiver needs to have a slot with the same name for each of these
    // signals:
    //

    void pkgDownloadStart    ( ZyppRes zyppRes );
    void pkgDownloadProgress ( ZyppRes zyppRes, int value );
    void pkgDownloadEnd      ( ZyppRes zyppRes );

    void pkgInstallStart     ( ZyppRes zyppRes );
    void pkgInstallProgress  ( ZyppRes zyppRes, int value );
    void pkgInstallEnd       ( ZyppRes zyppRes );

    void pkgRemoveStart      ( ZyppRes zyppRes );
    void pkgRemoveProgress   ( ZyppRes zyppRes, int value );
    void pkgRemoveEnd        ( ZyppRes zyppRes );


public:

    // Use each one with  PkgCommitSignalForwarder::instance()->sendPkg...()

    void sendPkgDownloadStart    ( ZyppRes zyppRes )             { emit pkgDownloadStart   ( zyppRes);         }
    void sendPkgDownloadProgress ( ZyppRes zyppRes, int value )  { emit pkgDownloadProgress( zyppRes, value ); }
    void sendPkgDownloadEnd      ( ZyppRes zyppRes )             { emit pkgDownloadEnd     ( zyppRes );        }

    void sendPkgInstallStart     ( ZyppRes zyppRes )             { emit pkgInstallStart    ( zyppRes);         }
    void sendPkgInstallProgress  ( ZyppRes zyppRes, int value )  { emit pkgInstallProgress ( zyppRes, value ); }
    void sendPkgInstallEnd       ( ZyppRes zyppRes )             { emit pkgInstallEnd      ( zyppRes );        }

    void sendPkgRemoveStart      ( ZyppRes zyppRes )             { emit pkgRemoveStart     ( zyppRes);         }
    void sendPkgRemoveProgress   ( ZyppRes zyppRes, int value )  { emit pkgRemoveProgress  ( zyppRes, value ); }
    void sendPkgRemoveEnd        ( ZyppRes zyppRes )             { emit pkgRemoveEnd       ( zyppRes );        }


    static PkgCommitSignalForwarder * _instance;
};


//
//----------------------------------------------------------------------
//
// Libzypp callbacks; see /usr/include/zypp/ZYppCallbacks.h


typedef zypp::repo::DownloadResolvableReport::Action  PkgDownloadAction;
typedef zypp::repo::DownloadResolvableReport::Error   PkgDownloadError;


struct PkgDownloadCallback:
    public zypp::callback::ReceiveReport<zypp::repo::DownloadResolvableReport>
{

    virtual void start( ZyppRes zyppRes, const Url & /*url*/ )
        {
            PkgCommitSignalForwarder::instance()->sendPkgDownloadStart( zyppRes );
        }


    virtual bool progress( int value, ZyppRes zyppRes)
        {
            PkgCommitSignalForwarder::instance()->sendPkgDownloadProgress( zyppRes, value );

            return true; // Don't abort
        }


    virtual void finish( ZyppRes zyppRes,
                         PkgDownloadError    error,
                         const std::string & reason )
        {
            PkgCommitSignalForwarder::instance()->sendPkgDownloadEnd( zyppRes );
        }


    virtual PkgDownloadAction problem( ZyppRes zyppRes,
                                       PkgDownloadError  error,
                                       const std::string description )
        {
            return PkgDownloadAction::ABORT;
        }


    /**
     * Hint that package is available in the local cache (no download needed).
     * This will be the only trigger for an already cached package.
     **/
    virtual void infoInCache( ZyppRes zyppRes,
                              const Pathname & /*localfile*/ )
        {}

#if 0
    // FIXME: TO DO later (much later...)

    virtual void pkgGpgCheck( const UserData & userData_r = UserData() )
        {}

    virtual void startDeltaDownload( const Pathname  & /*filename*/,
                                     const ByteCount & /*downloadSize*/ )
        {}

    virtual bool progressDeltaDownload( int /*value*/ )
        {
            return true; // Don't abort
        }

    virtual void problemDeltaDownload( const std::string & /*description*/ )
        {}

    virtual void finishDeltaDownload()
        {}

    virtual void startDeltaApply( const Pathname & /*filename*/ )
        {}

    virtual void progressDeltaApply( int /*value*/ )
        {}

    virtual void problemDeltaApply( const std::string & /*description*/ )
        {}

    virtual void finishDeltaApply()
        {}
#endif

}; // PkgDownloadCallback



typedef zypp::target::rpm::InstallResolvableReport::Action  PkgInstallAction;
typedef zypp::target::rpm::InstallResolvableReport::Error   PkgInstallError;


struct PkgInstallCallback:
    public zypp::callback::ReceiveReport<zypp::target::rpm::InstallResolvableReport>
{
    virtual void start( ZyppRes zyppRes )
        {
            PkgCommitSignalForwarder::instance()->sendPkgInstallStart( zyppRes );
        }


    virtual bool progress( int value, ZyppRes zyppRes )
        {
            PkgCommitSignalForwarder::instance()->sendPkgInstallProgress( zyppRes, value );

            return true; // Don't abort
        }


    virtual void finish( ZyppRes zyppRes,
                         PkgInstallError error,
                         const std::string & /*reason*/,
                         RpmLevel /*level*/ )
        {
            PkgCommitSignalForwarder::instance()->sendPkgInstallEnd( zyppRes );
        }


    virtual PkgInstallAction problem( ZyppRes zyppRes,
                                      PkgInstallError error,
                                      const std::string & /*description*/,
                                      RpmLevel /*level*/ )
        {
            return PkgInstallAction::ABORT;
        }

}; // PkgInstallCallback



typedef zypp::target::rpm::RemoveResolvableReport::Action  PkgRemoveAction;
typedef zypp::target::rpm::RemoveResolvableReport::Error   PkgRemoveError;


struct PkgRemoveCallback:
    public zypp::callback::ReceiveReport<zypp::target::rpm::RemoveResolvableReport>
{
    virtual void start( ZyppRes zyppRes )
        {
            PkgCommitSignalForwarder::instance()->sendPkgRemoveStart( zyppRes );
        }


    virtual bool progress( int value, ZyppRes zyppRes )
        {
            PkgCommitSignalForwarder::instance()->sendPkgRemoveProgress( zyppRes, value );

            return true; // Don't abort
        }


    virtual void finish( ZyppRes zyppRes,
                         PkgRemoveError error,
                         const std::string & /*reason*/ )
        {
            PkgCommitSignalForwarder::instance()->sendPkgRemoveEnd( zyppRes );
        }


    virtual PkgRemoveAction problem( ZyppRes zyppRes,
                                     PkgRemoveError error,
                                     const std::string & /*description*/ )
        {
            return zypp::target::rpm::RemoveResolvableReport::ABORT;
        }

}; // PkgRemoveCallback


//
//----------------------------------------------------------------------
//


/**
 * Class to bundle the zypp callbacks needed during a zypp package commit and
 * to translate each libzypp event ("report" in libzypp lingo) into a Qt signal.
 *
 * The constructor instantiates and connects the callbacks, the destructor
 * disconnects and deletes them; so the instance of this object needs to live
 * until the commit is finished.
 **/
class PkgCommitCallbacks
{
public:

    /**
     * Constructor: Create the needed callbacks and connect them (register them
     * with libzypp).
     *
     * Remember to call PkgCommitSignalForwarder::connectAll( receiver )
     * once to get the signals that the callbacks send via the signal forwarder.
     **/
    PkgCommitCallbacks();

    /**
     * Destructor: Disconnect (unregister them with libzypp) and delete the
     * callbacks.
     **/
    virtual ~PkgCommitCallbacks();


protected:

    PkgDownloadCallback _pkgDownloadCallback;
    PkgInstallCallback  _pkgInstallCallback;
    PkgRemoveCallback   _pkgRemoveCallback;
};


#endif // PkgCommitCallbacks_h