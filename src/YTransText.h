/*  ---------------------------------------------------------
               __  __            _
              |  \/  |_   _ _ __| |_   _ _ __
              | |\/| | | | | '__| | | | | '_ \
              | |  | | |_| | |  | | |_| | | | |
              |_|  |_|\__, |_|  |_|\__, |_| |_|
                      |___/        |___/
    ---------------------------------------------------------

    Project:  Myrlyn Package Manager GUI
    Copyright (c) 2004-25 SUSE LLC
    License:  GPL V2 - See file LICENSE for details.

 */


#ifndef YTransText_h
#define YTransText_h

#include <libintl.h>
#include <string>


/**
 * Helper class for translated strings: Stores a message in the original
 * (untranslated) version along with the translation into the current locale.
 **/
class YTransText
{
public:

    /**
     * Constructor with both original and translated message.
     **/
    YTransText( const std::string & orig,
                const std::string & translation ):
        _orig( orig ),
        _translation( translation )
        {}


    /**
     * Copy constructor.
     **/
    YTransText( const YTransText & src )
    {
        _orig           = src.orig();
        _translation    = src.translation();
    }

    /**
     * Assignment operator.
     **/
    YTransText & operator= ( const YTransText & src )
    {
        _orig           = src.orig();
        _translation    = src.translation();

        return *this;
    }

    /**
     * Return the original message.
     **/
    const std::string & orig()          const { return _orig;           }

    /**
     * Return the translation.
     **/
    const std::string & translation()   const { return _translation;    }

    /**
     * Return the translation.
     * ( alias, just as a shortcut )
     **/
    const std::string & trans()         const { return _translation;    }

    /**
     * Set the original message. Does not touch the translation, so make sure
     * you change both if you want to keep them synchronized!
     **/
    void setOrig( const std::string & newOrig ) { _orig = newOrig; }

    /**
     * Set the translation.
     **/
    void setTranslation( const std::string & newTrans ) { _translation = newTrans; }

    /**
     * operator< : Compares translations.
     **/
    bool operator< ( const YTransText & other ) const
        { return _translation < other.translation(); }

    /**
     * operator> : Compares translations.
     **/
    bool operator> ( const YTransText & other ) const
        { return _translation > other.translation(); }

    /**
     * operator== : Compares translations.
     **/
    bool operator== ( const YTransText & other ) const
        { return _translation == other.translation(); }


private:

    std::string _orig;
    std::string _translation;

};



#endif // YTransText_h
