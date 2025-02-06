/*
 *   File name: SearchFilter.h
 *   Summary:	Support classes for yqpkg
 *   License:	GPL V2 - See file LICENSE for details.
 *
 *   Author:	Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 *              Donated by the QDirStat project
 */

#include "Logger.h"
#include "utf8.h"
#include "SearchFilter.h"


SearchFilter::SearchFilter( const QString & pattern,
                            FilterMode      filterMode,
                            FilterMode      defaultFilterMode ):
    _pattern( pattern ),
    _regexp( pattern ),
    _filterMode( filterMode ),
    _defaultFilterMode( defaultFilterMode )
{
    if ( _defaultFilterMode == Auto )
        _defaultFilterMode = StartsWith;

    if ( _filterMode == Auto )
        guessFilterMode();

    if ( _filterMode == Wildcard )
        _regexp.setPattern( QRegularExpression::wildcardToRegularExpression( _regexp.pattern() ) );

    _regexp.setPatternOptions( QRegularExpression::CaseInsensitiveOption );
}


void SearchFilter::guessFilterMode()
{
    _filterMode = guessFilterMode( _pattern );

    if ( _filterMode == Auto )             // Still Auto?
        _filterMode = _defaultFilterMode;  // Use the fallback.

#if 1
    logDebug() << "using filter mode " << toString( _filterMode )
               << " from \"" << _pattern << "\""
               << Qt::endl;
#endif

    if ( _filterMode == ExactMatch && _pattern.startsWith( "=" ) )
    {
        // remove leading equals sign
        _pattern.remove( 0, 1 ); // FIXME: Use _pattern.removeFirst() once Qt >= 6.5 is required
        _regexp.setPattern( _pattern );
    }
}


SearchFilter::FilterMode
SearchFilter::guessFilterMode( const QString & pattern )
{
    SearchFilter::FilterMode filterMode = Auto;

    if ( pattern.isEmpty() )
    {
        filterMode = SelectAll;
    }
    else if ( pattern.startsWith( "=" ) )
    {
        filterMode = ExactMatch;
    }
    else if ( pattern.contains( "*.*" ) )
    {
        filterMode = Wildcard;
    }
    else if ( pattern.contains( ".*" ) ||
              pattern.contains( "^"  ) ||
              pattern.contains( "$"  ) ||
              pattern.contains( "("  ) ||
              pattern.contains( "|"  ) ||
              pattern.contains( "["  )   )
    {
        filterMode = RegExp;
    }
    else if ( pattern.contains( "*" ) ||
              pattern.contains( "?" )   )
    {
        filterMode = Wildcard;
    }

    return filterMode;
}


bool SearchFilter::matches( const QString & str ) const
{
    Qt::CaseSensitivity caseSensitivity = Qt::CaseInsensitive;

    if ( ( _regexp.patternOptions() & QRegularExpression::CaseInsensitiveOption ) == 0 )
      caseSensitivity = Qt::CaseSensitive;

    switch ( _filterMode )
    {
        case Contains:   return str.contains  ( _pattern, caseSensitivity );
        case StartsWith: return str.startsWith( _pattern, caseSensitivity );
        case ExactMatch: return QString::compare( str, _pattern, caseSensitivity ) == 0;
        case Wildcard:   return _regexp.match( str ).hasMatch();
        case RegExp:     return str.contains( _regexp );
        case SelectAll:  return true;
        case Auto:
            logWarning() << "Unexpected filter mode 'Auto' - assuming 'Contains'" << Qt::endl;
            return str.contains( _pattern );
    }

    logError() << "Undefined filter mode " << toString( _filterMode ) << Qt::endl;
    return false;
}


bool SearchFilter::matches( const std::string & str ) const
{
    return matches( fromUTF8( str ) );
}



void SearchFilter::setCaseSensitive( bool sensitive )
{
    if (sensitive)
      _regexp.setPatternOptions( QRegularExpression::NoPatternOption );
    else
      _regexp.setPatternOptions( QRegularExpression::CaseInsensitiveOption );
}


QString SearchFilter::toString( FilterMode filterMode )
{
    switch ( filterMode )
    {
        case Contains:   return "Contains";
        case StartsWith: return "StartsWith";
        case ExactMatch: return "ExactMatch";
        case Wildcard:   return "Wildcard";
        case RegExp:     return "Regexp";
        case SelectAll:  return "SelectAll";
        case Auto:       return "Auto";
    }

    return QString( "<Unknown FilterMode %1" ).arg( (int) filterMode );
}
