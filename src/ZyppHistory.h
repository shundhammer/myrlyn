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
#include "ZyppHistoryEvents.h"

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
    const ZyppHistoryEvents::EventList & events() const { return _events; }

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

    ZyppHistoryEvents::EventList _events;
    bool                         _dirty;

};  // class ZyppHistory

#endif // ZyppHistory_h
