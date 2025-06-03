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


#include "BaseProduct.h"
#include "Exception.h"
#include "Logger.h"
#include "utf8.h"
#include "CommunityRepos.h"


CommunityRepos::CommunityRepos( DistroType distroType )
{
    if ( distroType == BestMatch )
        distroType = findBestMatch();

    switch ( distroType )
    {
        case openSUSE_Tumbleweed: openSUSE_Tumbleweed_Repos(); break;
        case openSUSE_Slowroll:   openSUSE_Slowroll_Repos();   break;
        case openSUSE_Leap_15x:   openSUSE_Leap_15x_Repos();   break;
        case SLE_15_SPx:          SLE_15_SPx_Repos();          break;
        case SLES_16:             SLES_16_Repos();            break;

        default:
            logWarning() << "No community repos for this distro." << endl;
            break;
    }
}


CommunityRepos::~CommunityRepos()
{
    // NOP
}


QStringList
CommunityRepos::repoNames()
{
    QStringList names;

    for ( const ZyppRepoInfo & repoInfo: _repos )
        names << fromUTF8( repoInfo.name() );

    return names;
}


ZyppRepoInfo
CommunityRepos::repoInfo( const QString & repoName )
{
    std::string wantedRepoName = toUTF8( repoName );

    for ( const ZyppRepoInfo & repoInfo: _repos )
    {
        if ( repoInfo.name() == wantedRepoName )
            return repoInfo;
    }

    logError() << "No community repo with name " << repoName << endl;
    return ZyppRepoInfo::noRepo;
}


CommunityRepos::DistroType
CommunityRepos::findBestMatch()
{
    // See also
    // https://gist.github.com/shundhammer/718a08eabaf6743e1aad1109079e865f

    static bool resultLogged = false;
    DistroType  distroType   = None;

    if ( BaseProduct::summary().contains( "Slowroll", Qt::CaseInsensitive ) )
        // Check for Slowroll first: Summary "openSUSE Tumbleweed-Slowroll"
    {
        distroType = openSUSE_Slowroll;
    }
    else if ( BaseProduct::summary().contains( "Tumbleweed", Qt::CaseInsensitive ) )
    {
        distroType = openSUSE_Tumbleweed;
    }
    else if ( BaseProduct::name().contains( "Leap" ) &&
              BaseProduct::version().startsWith( "15." ) )
    {
        distroType = openSUSE_Leap_15x;
    }
    else if ( BaseProduct::name().contains( "Leap" ) &&
              BaseProduct::version().startsWith( "16." ) )
    {
        distroType = openSUSE_Leap_16x;
    }
    else if ( BaseProduct::summary().startsWith( "SUSE Linux Enterprise", Qt::CaseInsensitive ) &&
              BaseProduct::version().startsWith( "15." ) )
    {
        distroType = SLE_15_SPx;
    }
    else if ( BaseProduct::summary().startsWith( "SUSE Linux Enterprise", Qt::CaseInsensitive ) &&
              BaseProduct::version().startsWith( "16." ) )
    {
        distroType = SLES_16;
    }

    if ( ! resultLogged )
    {
        resultLogged = true;
        BaseProduct::log();

        if ( distroType == None )
            logWarning() << "No match found." << endl;
        else
            logInfo() << "Matches " << toString( distroType ) << endl;
    }

    return distroType;
}


QString CommunityRepos::toString( DistroType distroType )
{
    switch ( distroType )
    {
        case None:                 return "None";
        case BestMatch:            return "BestMatch";
        case openSUSE_Tumbleweed:  return "openSUSE_Tumbleweed";
        case openSUSE_Slowroll:    return "openSUSE_Slowroll";
        case openSUSE_Leap_15x:    return "openSUSE_Leap_15x";
        case openSUSE_Leap_16x:    return "openSUSE_Leap_16x";
        case SLE_15_SPx:           return "SLE_15_SPx";
        case SLES_16:              return "SLES_16";
    }

    return QString( "<Unknown DistroType #%1" ).arg( (int) distroType );
}


static ZyppRepoInfo stdRepo( const std::string & url = "" )
{
    ZyppRepoInfo repoInfo;

    if ( ! url.empty() )
        repoInfo.setBaseUrl( zypp::Url( url ) );

    repoInfo.setEnabled( true );
    repoInfo.setAutorefresh( true );

    return repoInfo;
}


static ZyppRepoInfo packmanRepo( const std::string & url )
{
    ZyppRepoInfo repoInfo = stdRepo( url );

    repoInfo.setName( "Packman" );
    repoInfo.setAlias( "packman-repo" );
    repoInfo.setPriority( 80 );

    return repoInfo;
}


static ZyppRepoInfo packmanEssentialsRepo( const std::string & url )
{
    ZyppRepoInfo repoInfo = stdRepo( url );

    repoInfo.setName( "Packman Essentials" );
    repoInfo.setAlias( "packman-essentials-repo" );
    repoInfo.setPriority( 81 );

    return repoInfo;
}


static ZyppRepoInfo libdvdcssRepo( const std::string & url )
{
    ZyppRepoInfo repoInfo = stdRepo( url );

    repoInfo.setName( "LibDvdCss" );
    repoInfo.setAlias( "libdvdcss-repo" );
    repoInfo.setPriority( 82 );

    return repoInfo;
}


static ZyppRepoInfo openH264Repo( const std::string & url )
{
    ZyppRepoInfo repoInfo = stdRepo( url );

    repoInfo.setName( "Open H.264 Codec" );
    repoInfo.setAlias( "openh264-repo" );
    repoInfo.setPriority( 83 );

    return repoInfo;
}


static ZyppRepoInfo nvidiaRepo( const std::string & url )
{
    ZyppRepoInfo repoInfo = stdRepo( url );

    repoInfo.setName( "nVidia" );
    repoInfo.setAlias( "nvidia-repo" );
    repoInfo.setPriority( 84 );

    return repoInfo;
}


void CommunityRepos::openSUSE_Tumbleweed_Repos()
{
    _repos << packmanRepo          ( "https://ftp.gwdg.de/pub/linux/misc/packman/suse/openSUSE_Tumbleweed/" )
           << packmanEssentialsRepo( "https://ftp.gwdg.de/pub/linux/misc/packman/suse/openSUSE_Tumbleweed/Essentials" )
           << libdvdcssRepo        ( "https://opensuse-guide.org/repo/openSUSE_Tumbleweed/" )
           << openH264Repo         ( "http://codecs.opensuse.org/openh264/openSUSE_Tumbleweed/" )
           << nvidiaRepo           ( "https://download.nvidia.com/opensuse/tumbleweed" );
}


void CommunityRepos::openSUSE_Slowroll_Repos()
{
    _repos << packmanRepo          ( "https://ftp.gwdg.de/pub/linux/misc/packman/suse/openSUSE_Slowroll/" )
           << packmanEssentialsRepo( "https://ftp.gwdg.de/pub/linux/misc/packman/suse/openSUSE_Slowroll/Essentials/" )
           << libdvdcssRepo        ( "https://opensuse-guide.org/repo/openSUSE_Tumbleweed/" )
           << openH264Repo         ( "http://codecs.opensuse.org/openh264/openSUSE_Tumbleweed/" )
           << nvidiaRepo           ( "https://download.nvidia.com/opensuse/tumbleweed" );
}


void CommunityRepos::openSUSE_Leap_15x_Repos()
{
    _repos << packmanRepo          ( "https://ftp.gwdg.de/pub/linux/misc/packman/suse/openSUSE_Leap_$releasever/" )
           << packmanEssentialsRepo( "https://ftp.gwdg.de/pub/linux/misc/packman/suse/openSUSE_Leap_$releasever/Essentials/" )
           << libdvdcssRepo        ( "https://opensuse-guide.org/repo/openSUSE_Leap_$releasever/" )
           << openH264Repo         ( "http://codecs.opensuse.org/openh264/openSUSE_Leap/" )
           << nvidiaRepo           ( "https://download.nvidia.com/opensuse/leap/$releasever/" );
}


void CommunityRepos::openSUSE_Leap_16x_Repos()
{
#if 0
    _repos << packmanRepo          ( "https://ftp.gwdg.de/pub/linux/misc/packman/suse/openSUSE_Leap_$releasever/" )
           << packmanEssentialsRepo( "https://ftp.gwdg.de/pub/linux/misc/packman/suse/openSUSE_Leap_$releasever/Essentials/" )
           << libdvdcssRepo        ( "https://opensuse-guide.org/repo/openSUSE_Leap_$releasever/" )
           << openH264Repo         ( "http://codecs.opensuse.org/openh264/openSUSE_Leap_16/" )
           << nvidiaRepo           ( "https://download.nvidia.com/opensuse/leap/$releasever/" );
#else
    _repos << libdvdcssRepo        ( "https://opensuse-guide.org/repo/openSUSE_Leap_$releasever/" )
           << openH264Repo         ( "http://codecs.opensuse.org/openh264/openSUSE_Leap_16/" )
           << nvidiaRepo           ( "https://download.nvidia.com/opensuse/leap/$releasever/" );
#endif
}


void CommunityRepos::SLE_15_SPx_Repos()
{
    _repos << packmanRepo          ( "https://ftp.gwdg.de/pub/linux/misc/packman/suse/SLE_15/" )
           << packmanEssentialsRepo( "https://ftp.gwdg.de/pub/linux/misc/packman/suse/SLE_15/Essentials/" )
           << libdvdcssRepo        ( "https://opensuse-guide.org/repo/openSUSE_Leap_$releasever/" )
           << openH264Repo         ( "http://codecs.opensuse.org/openh264/openSUSE_Leap/" );
}


void CommunityRepos::SLES_16_Repos()
{
#if 0
    _repos << packmanRepo          ( "https://ftp.gwdg.de/pub/linux/misc/packman/suse/SLES_16/" )
           << packmanEssentialsRepo( "https://ftp.gwdg.de/pub/linux/misc/packman/suse/SLES_16/Essentials/" )
           << libdvdcssRepo        ( "https://opensuse-guide.org/repo/openSUSE_Leap_$releasever/" )
           << openH264Repo         ( "http://codecs.opensuse.org/openh264/openSUSE_Leap_16/" );
#else
    _repos << libdvdcssRepo        ( "https://opensuse-guide.org/repo/openSUSE_Leap_$releasever/" )
           << openH264Repo         ( "http://codecs.opensuse.org/openh264/openSUSE_Leap_16/" );
#endif
}
