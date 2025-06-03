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


#ifndef PkgCommitCallbacks_h
#define PkgCommitCallbacks_h


#include <iostream>  // cerr
#include <QObject>
#include <QStringList>

#include <zypp/Resolvable.h>
#include <zypp/Url.h>
#include <zypp/ZYppCallbacks.h>
#include <zypp/sat/FileConflicts.h>

#include "utf8.h"
#include "YQZypp.h"     // ZyppRes

#define TEST_FILE_CONFLICTS     0


using zypp::Pathname;
using zypp::Url;

enum ErrorReply
{
    AbortReply,
    RetryReply,
    IgnoreReply
};


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
        , _doAbort( false )
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

    /**
     * Return 'true' if 'abortCommit()' has been received since the last
     * 'reset()'
     **/
    bool doAbort() const { return _doAbort; }

    /**
     * Return the latest reply to an error.
     **/
    ErrorReply reply() const { return _reply; }

    /**
     * Reply to an error.
     *
     * Since Qt signals cannot return a value, this kludge is used to achieve a
     * similar effect: Before the connected slot returns, it should call this
     * to set a reply (in this case one of Abort / Retry / Ignore).
     *
     * Of course, this only works with direct signal / slot connections,
     * i.e. in the same thread. See also QObject::connect() and
     * Qt::ConnectionType.
     **/
    void setReply( ErrorReply val ) { _reply = val; }

    /**
     * Reset the internal status, including the _doAbort flag.
     **/
    void reset();


signals:

    //
    // The receiver needs to have a slot with the same name for each of these
    // signals:
    //

    void pkgDownloadStart    ( ZyppRes zyppRes );
    void pkgDownloadProgress ( ZyppRes zyppRes, int value );
    void pkgDownloadEnd      ( ZyppRes zyppRes );

    void pkgCachedNotify     ( ZyppRes zyppRes );
    void pkgDownloadError    ( ZyppRes zyppRes, const QString & msg );


    void pkgInstallStart     ( ZyppRes zyppRes );
    void pkgInstallProgress  ( ZyppRes zyppRes, int value );
    void pkgInstallEnd       ( ZyppRes zyppRes );
    void pkgInstallError     ( ZyppRes zyppRes, const QString & msg );

    void pkgRemoveStart      ( ZyppRes zyppRes );
    void pkgRemoveProgress   ( ZyppRes zyppRes, int value );
    void pkgRemoveEnd        ( ZyppRes zyppRes );
    void pkgRemoveError      ( ZyppRes zyppRes, const QString & msg );

    void fileConflictsCheckStart();
    void fileConflictsCheckProgress( int percent );
    void fileConflictsCheckResult  ( const QStringList & conflictsList );


public slots:

    void abortCommit() { _doAbort = true; }


public:

    // Use each one with  PkgCommitSignalForwarder::instance()->sendPkg...()

    void sendPkgDownloadStart    ( ZyppRes zyppRes )             { emit pkgDownloadStart   ( zyppRes);         }
    void sendPkgDownloadProgress ( ZyppRes zyppRes, int value )  { emit pkgDownloadProgress( zyppRes, value ); }
    void sendPkgDownloadEnd      ( ZyppRes zyppRes )             { emit pkgDownloadEnd     ( zyppRes );        }

    void sendPkgCachedNotify     ( ZyppRes zyppRes )             { emit pkgCachedNotify    ( zyppRes );        }
    void sendPkgDownloadError    ( ZyppRes zyppRes,
                                   const QString & msg )         { emit pkgDownloadError   ( zyppRes, msg );   }


    void sendPkgInstallStart     ( ZyppRes zyppRes )             { emit pkgInstallStart    ( zyppRes);         }
    void sendPkgInstallProgress  ( ZyppRes zyppRes, int value )  { emit pkgInstallProgress ( zyppRes, value ); }
    void sendPkgInstallEnd       ( ZyppRes zyppRes )             { emit pkgInstallEnd      ( zyppRes );        }
    void sendPkgInstallError     ( ZyppRes zyppRes,
                                   const QString & msg )         { emit pkgInstallError    ( zyppRes, msg );   }

    void sendPkgRemoveStart      ( ZyppRes zyppRes )             { emit pkgRemoveStart     ( zyppRes);         }
    void sendPkgRemoveProgress   ( ZyppRes zyppRes, int value )  { emit pkgRemoveProgress  ( zyppRes, value ); }
    void sendPkgRemoveEnd        ( ZyppRes zyppRes )             { emit pkgRemoveEnd       ( zyppRes );        }
    void sendPkgRemoveError      ( ZyppRes zyppRes,
                                   const QString & msg )         { emit pkgRemoveError     ( zyppRes, msg );   }

    void sendFileConflictsCheckStart()                           { emit fileConflictsCheckStart();             }
    void sendFileConflictsCheckProgress( int percent )           { emit fileConflictsCheckProgress( percent ); }
    void sendFileConflictsCheckResult( const QStringList & conflicts ) { emit fileConflictsCheckResult( conflicts ); }


    //
    // Data members
    //

    bool       _doAbort;
    ErrorReply _reply;

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

    virtual void start( ZyppRes zyppRes, const Url & /*url*/ ) override
        {
            PkgCommitSignalForwarder::instance()->sendPkgDownloadStart( zyppRes );
        }


    virtual bool progress( int value, ZyppRes zyppRes) override
        {
            PkgCommitSignalForwarder::instance()->sendPkgDownloadProgress( zyppRes, value );

            return ! PkgCommitSignalForwarder::instance()->doAbort();
        }


    virtual void finish( ZyppRes zyppRes,
                         PkgDownloadError    error,
                         const std::string & reason )  override
        {
            PkgCommitSignalForwarder::instance()->sendPkgDownloadEnd( zyppRes );
        }


    virtual PkgDownloadAction problem( ZyppRes zyppRes,
                                       PkgDownloadError    error,
                                       const std::string & description ) override
        {
            PkgCommitSignalForwarder::instance()->setReply( AbortReply );
            PkgCommitSignalForwarder::instance()->sendPkgDownloadError( zyppRes, fromUTF8( description ) );

            switch ( PkgCommitSignalForwarder::instance()->reply() )
            {
                case IgnoreReply: return PkgDownloadAction::IGNORE;
                case RetryReply:  return PkgDownloadAction::RETRY;
                default:          return PkgDownloadAction::ABORT;
            }
        }


    /**
     * Hint that package is available in the local cache (no download needed).
     * This will be the only trigger for an already cached package.
     **/
    virtual void infoInCache( ZyppRes zyppRes,
                              const Pathname & /*localfile*/ )  override
        {
            PkgCommitSignalForwarder::instance()->sendPkgCachedNotify( zyppRes );
        }


#if 0
    // FIXME: TO DO later (much later...)

    virtual void pkgGpgCheck( const UserData & userData_r = UserData() )  override
        {}

    virtual void startDeltaDownload( const Pathname  & /*filename*/,
                                     const ByteCount & /*downloadSize*/ )  override
        {}

    virtual bool progressDeltaDownload( int /*value*/ )  override
        {
            return ! PkgCommitSignalForwarder::instance()->doAbort();
        }

    virtual void problemDeltaDownload( const std::string & /*description*/ )  override
        {}

    virtual void finishDeltaDownload()  override
        {}

    virtual void startDeltaApply( const Pathname & /*filename*/ ) override
        {}

    virtual void progressDeltaApply( int /*value*/ ) override
        {}

    virtual void problemDeltaApply( const std::string & /*description*/ ) override
        {}

    virtual void finishDeltaApply() override
        {}
#endif

}; // PkgDownloadCallback



typedef zypp::target::rpm::InstallResolvableReport::Action  PkgInstallAction;
typedef zypp::target::rpm::InstallResolvableReport::Error   PkgInstallError;


struct PkgInstallCallback:
    public zypp::callback::ReceiveReport<zypp::target::rpm::InstallResolvableReport>
{
    virtual void start( ZyppRes zyppRes ) override
        {
            PkgCommitSignalForwarder::instance()->sendPkgInstallStart( zyppRes );
        }


    virtual bool progress( int value, ZyppRes zyppRes ) override
        {
            PkgCommitSignalForwarder::instance()->sendPkgInstallProgress( zyppRes, value );

            return ! PkgCommitSignalForwarder::instance()->doAbort();
        }


    virtual void finish( ZyppRes zyppRes,
                         PkgInstallError error,
                         const std::string & /*reason*/,
                         RpmLevel /*level*/ ) override
        {
            PkgCommitSignalForwarder::instance()->sendPkgInstallEnd( zyppRes );
        }


    virtual PkgInstallAction problem( ZyppRes zyppRes,
                                      PkgInstallError error,
                                      const std::string & description,
                                      RpmLevel /*level*/ )  override
        {
            PkgCommitSignalForwarder::instance()->setReply( AbortReply );
            PkgCommitSignalForwarder::instance()->sendPkgInstallError( zyppRes, fromUTF8( description ) );

            switch ( PkgCommitSignalForwarder::instance()->reply() )
            {
                case IgnoreReply: return PkgInstallAction::IGNORE;
                case RetryReply:  return PkgInstallAction::RETRY;
                default:          return PkgInstallAction::ABORT;
            }
        }

}; // PkgInstallCallback



typedef zypp::target::rpm::RemoveResolvableReport::Action  PkgRemoveAction;
typedef zypp::target::rpm::RemoveResolvableReport::Error   PkgRemoveError;


struct PkgRemoveCallback:
    public zypp::callback::ReceiveReport<zypp::target::rpm::RemoveResolvableReport>
{
    virtual void start( ZyppRes zyppRes ) override
        {
            PkgCommitSignalForwarder::instance()->sendPkgRemoveStart( zyppRes );
        }


    virtual bool progress( int value, ZyppRes zyppRes ) override
        {
            PkgCommitSignalForwarder::instance()->sendPkgRemoveProgress( zyppRes, value );

            return ! PkgCommitSignalForwarder::instance()->doAbort();
        }


    virtual void finish( ZyppRes zyppRes,
                         PkgRemoveError error,
                         const std::string & /*reason*/ ) override
        {
            PkgCommitSignalForwarder::instance()->sendPkgRemoveEnd( zyppRes );
        }


    virtual PkgRemoveAction problem( ZyppRes zyppRes,
                                     PkgRemoveError error,
                                     const std::string & description ) override
        {
            PkgCommitSignalForwarder::instance()->setReply( AbortReply );
            PkgCommitSignalForwarder::instance()->sendPkgRemoveError( zyppRes, fromUTF8( description ) );

            switch ( PkgCommitSignalForwarder::instance()->reply() )
            {
                case IgnoreReply: return PkgRemoveAction::IGNORE;
                case RetryReply:  return PkgRemoveAction::RETRY;
                default:          return PkgRemoveAction::ABORT;
            }
        }

}; // PkgRemoveCallback




struct FileConflictsCheckCallback:
    public zypp::callback::ReceiveReport<zypp::target::FindFileConflictstReport>
{
    // See also the zypper sources: src/callbacks/rpm.h

    /**
     * Starting the file conflicts check.
     *
     * Return 'true' to continue, 'false' to abort.
     **/
    virtual bool start( const zypp::ProgressData & progress ) override
        {
            PkgCommitSignalForwarder::instance()->sendFileConflictsCheckStart();

            return ! PkgCommitSignalForwarder::instance()->doAbort();
        }

    /**
     * Reporting progress during the file conflicts check.
     *
     * 'skippedSolvables' is the queue of solvables that cannot be checked,
     * e.g. because they are not yet downloaded (not downloading all in
     * advance).
     *
     * Return 'true' to continue, 'false' to abort.
     **/
    virtual bool progress( const zypp::ProgressData & progress,
                           const zypp::sat::Queue &   skippedSolvables ) override
        {
            Q_UNUSED( skippedSolvables );

            // For zypp::ProgressData, see
            //
            //   /usr/include/zypp-core/ui/progressdata.h

            int percent = progress.reportValue();
            PkgCommitSignalForwarder::instance()->sendFileConflictsCheckProgress( percent );

            return ! PkgCommitSignalForwarder::instance()->doAbort();
        }

    /**
     * The result of the file conflicts check, which implies that the check is
     * now finished.
     *
     * 'skippedSolvables' is the queue of solvables that cannot be checked,
     * e.g. because they are not yet downloaded (not downloading all in
     * advance).
     *
     * 'conflicts' contains the problems found.
     *
     * Return 'true' to continue, 'false' to abort. This callback automatically
     * returns 'false' (abort) if any conflicts were found.
     **/
    virtual bool result( const zypp::ProgressData &       progress,
                         const zypp::sat::Queue &         skippedSolvables,
                         const zypp::sat::FileConflicts & conflicts ) override
        {
            Q_UNUSED( skippedSolvables );

            QStringList conflictsList;

            for ( zypp::sat::FileConflicts::const_iterator it = conflicts.begin();
                  it != conflicts.end();
                  ++it )
            {
                conflictsList << fromUTF8( (*it).asUserString() );
            }

#if TEST_FILE_CONFLICTS
            conflictsList << QString( "File /usr/bin/foo\n   from package\n      foo\n   conflicts with file from package \n      foobar" );
            conflictsList << QString( "File /usr/bin/bar\n   from package\n      bar\n   conflicts with file from package \n      foobar" );
            conflictsList << QString( "File /usr/bin/baz\n   from package\n      baz\n   conflicts with file from package \n      foobar" );
#endif

            PkgCommitSignalForwarder::instance()->sendFileConflictsCheckResult( conflictsList );

            if ( ! conflicts.empty() )
                return false; // abort

            return ! PkgCommitSignalForwarder::instance()->doAbort();
        }

}; // FileConflictsCheckCallback


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

    PkgDownloadCallback        _pkgDownloadCallback;
    PkgInstallCallback         _pkgInstallCallback;
    PkgRemoveCallback          _pkgRemoveCallback;
    FileConflictsCheckCallback _fileConflictsCheckCallback;
};


#endif // PkgCommitCallbacks_h
