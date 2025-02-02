/*
 *   File name: SearchFilter.h
 *   Summary:   Support classes for yqpkg
 *   License:   GPL V2 - See file LICENSE for details.
 *
 *   Author:    Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 *              Donated by the QDirStat project
 */

#ifndef SearchFilter_h
#define SearchFilter_h


#include <string>

#include <QString>
#include <QRegularExpression>
#include <QTextStream>


/**
 * Base class for search filters like PkgFilter or FileSearchFilter.
 **/
class SearchFilter
{
public:

    enum FilterMode
    {
        Auto,       // Guess from pattern (see below)
        Contains,   // Fixed string
        StartsWith, // Fixed string
        ExactMatch, // Fixed string
        Wildcard,
        RegExp,
        SelectAll   // Pattern is irrelevant
    };

    /**
     * Constructor: Create a search filter with the specified pattern and
     * filter mode.
     *
     * Filter mode "Auto" tries to guess a useful mode from the pattern:
     *
     * - If it's a fixed string without any wildcards, it uses
     *   'defaultFilterMode'.
     * - If it contains "*" wildcard characters, it uses "Wildcard".
     * - If it contains ".*" or "^" or "$", it uses "RegExp".
     * - If it starts with "=", it uses "ExactMatch".
     * - If it's empty, it uses "SelectAll".
     **/
    explicit SearchFilter( const QString & pattern,
                           FilterMode      filterMode        = Auto,
                           FilterMode      defaultFilterMode = StartsWith );

    /**
     * Check if a string matches this filter.
     **/
    bool matches( const QString &     str ) const;
    bool matches( const std::string & str ) const;

    /**
     * Return the pattern.
     **/
    const QString & pattern() const { return _pattern; }

    /**
     * Return the regular expression. This is only meaningful in filter modes RegExp
     * and Wildcard.
     **/
    const QRegularExpression & regexp() const { return _regexp; }

    /**
     * Return the filter mode.
     **/
    FilterMode filterMode() const { return _filterMode; }

    /**
     * Return 'true' if the matching is case sensitive, 'false if not.
     **/
    bool isCaseSensitive() const
        { return ( _regexp.patternOptions() & QRegularExpression::CaseInsensitiveOption) == 0; }

    /**
     * Set the match to case sensitive ('true') or case insensitive
     * ('false'). The default is case insensitive.
     **/
    void setCaseSensitive( bool sensitive = true );

    /**
     * Convert a filter mode to a string.
     **/
    static QString toString( FilterMode filterMode );

    /**
     * Guess the filter mode from 'pattern' if "Auto" was selected.
     *
     * 'pattern' might be modified: E.g. a pattern "=foo" would be detected as
     * "ExactMatch", and the '=' would be removed to result in "foo".
     **/
    static FilterMode guessFilterMode( const QString & pattern );


protected:

    /**
     * Guess the filter mode from the pattern if "Auto" was selected.
     **/
    void guessFilterMode();


    // Data members

    QString            _pattern;
    QRegularExpression _regexp;
    FilterMode         _filterMode;
    FilterMode         _defaultFilterMode;

};  // class SearchFilter


inline QTextStream & operator<< ( QTextStream        & stream,
                                  const SearchFilter & filter )
{
    stream << "<SearchFilter \""
           << filter.pattern()
           << "\" mode \""
           << SearchFilter::toString( filter.filterMode() ) << "\" "
           <<( filter.isCaseSensitive()? " case sensitive" : "" )
           << ">";

    return stream;
}


#endif  // SearchFilter_h
