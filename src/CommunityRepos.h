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

    Textdomain "qt-pkg"
 */


#ifndef CommunityRepos_h
#define CommunityRepos_h


#include <string>

#include <QList>
#include <QStringList>
#include "YQZypp.h"


/**
 * Class to hold information about commonly used "community" repositories for
 * the current base product for restricted multimedia packages (codecs, media
 * players) or non- open source proprietary drivers.
 *
 * Depending on the base product / distribution, this typically includes
 *   - Open H.264 Codec Repo
 *   - Packman Repo
 *   - Packman Essentials Repo
 *   - LibDvdCss Repo
 *   - NVidia Drivers Repo
 **/
class CommunityRepos
{
public:

    enum DistroType
    {
        None = 0,
        BestMatch,
        openSUSE_Tumbleweed,
        openSUSE_Slowroll,
        openSUSE_Leap_15x,
        openSUSE_Leap_16x,
        SLE_15_SPx,
        SLES_16
    };

    /**
     * Constructor: Find the community repos for the specified distro type.
     * 'bestMatch' uses the BaseProduct class to find out the appropriate one.
     **/
    CommunityRepos( DistroType distroType = BestMatch );

    /**
     * Destructor.
     **/
    virtual ~CommunityRepos();

    /**
     * Return a list of the names of all matching community repos for this base
     * product. The list may be empty.
     **/
    QStringList repoNames();

    /**
     * Return the ZyppRepoInfo for a repo with the specified name (one from
     * repoNames()) or ZyppRepoInfo::noRepo if there is no community repo with
     * that name.
     **/
    ZyppRepoInfo repoInfo( const QString & repoName );

    /**
     * Convert the DistroType enum to QString.
     **/
    static QString toString( DistroType distroType );


protected:

    /**
     * Find the best match for currently installed distro type from the current
     * base product. This may return DistroType::None if there is no match or
     * if no base product information is available.
     **/
    DistroType findBestMatch();

    //
    // Fill _repos with suitable content for one of the supported distro types
    //

    void openSUSE_Tumbleweed_Repos();
    void openSUSE_Slowroll_Repos();
    void openSUSE_Leap_15x_Repos();
    void openSUSE_Leap_16x_Repos();
    void SLE_15_SPx_Repos();
    void SLES_16_Repos();


    // Data members

    QList<ZyppRepoInfo> _repos;
};


#endif // CommunityRepos_h
