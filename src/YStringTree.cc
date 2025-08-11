/*  ---------------------------------------------------------
               __  __            _
              |  \/  |_   _ _ __| |_   _ _ __
              | |\/| | | | | '__| | | | | '_ \
              | |  | | |_| | |  | | |_| | | | |
              |_|  |_|\__, |_|  |_|\__, |_| |_|
                      |___/        |___/
    ---------------------------------------------------------

    Project:  Myrlyn Package Manager GUI
    Copyright (c) 2024-25 SUSE LLC
    License:  GPL V2 - See file LICENSE for details.

 */


#include <stdio.h>
#include "YStringTree.h"

using std::string;


YStringTree::YStringTree()
    : _root( 0 )
{
    _root = new YStringTreeItem( YTransText( "<root>" ) );
}


YStringTree::~YStringTree()
{
    if ( _root )
        delete _root;
}


YStringTreeItem *
YStringTree::addBranch( const string &    content,
                        char              delimiter,
                        YStringTreeItem * parent )
{
    YStringTreeItem * node = 0;

    if ( ! parent )
        parent = _root;

    if ( delimiter == 0 )
    {
        // Simple case: No delimiter, simply create a new item for 'content'
        // and insert it.

        node = new YStringTreeItem( YTransText( content, translate( content ) ), parent );
    }
    else
    {
        // Split 'content' into substrings and insert each subitem

        string::size_type start = 0;
        string::size_type end   = 0;

        while ( start < content.length() )
        {
            // Skip delimiters

            while ( start < content.length() &&
                    content[ start ] == delimiter )
            {
                start++;
            }


            // Search next delimiter

            end = start;

            while ( end < content.length() &&
                    content[ end ] != delimiter )
            {
                end++;
            }


            // Extract substring, if there is any

            if ( end > start )
            {
                string path_component = content.substr( start, end - start );
                YTransText path_component_trans( path_component, translate( path_component ) );

                // Check if an entry with this text already exists
                node = findDirectChild( parent, path_component_trans);

                if ( ! node )   // No entry with this text yet? Create one.
                    node = new YStringTreeItem( path_component_trans, parent );

                parent = node;
            }

            start = end;
        }
    }

    return node;
}


string
YStringTree::translate( const string & orig )
{
    string trans( dgettext( "myrlyn", orig.c_str() ) );

    return trans;
}


string
YStringTree::completePath( const YStringTreeItem * item,
                           bool translated,
                           char delimiter,
                           bool startWithDelimiter )
{
    string path;

    if ( item )
    {
        path = translated ? item->value().trans() : item->value().orig();

        while ( item->parent() && item->parent() != _root )
        {
            string parentPath = translated ?
                item->parent()->value().translation() :
                item->parent()->value().orig();

            path = parentPath + delimiter + path;
            item = item->parent();
        }

    }

    if ( startWithDelimiter )
        path = delimiter + path;

    return path;
}


YTransText
YStringTree::path( const YStringTreeItem * item,
                   char delimiter,
                   bool startWithDelimiter )
{
    if ( ! item )
        return YTransText( "", "" );

    YTransText path = item->value();

    while ( item->parent() && item->parent() != _root )
    {
        path.setOrig       ( item->parent()->value().orig()  + delimiter + path.orig() );
        path.setTranslation( item->parent()->value().trans() + delimiter + path.trans() );

        item = item->parent();
    }

    if ( startWithDelimiter )
    {
        path.setOrig       ( delimiter + path.orig() );
        path.setTranslation( delimiter + path.translation() );
    }

    return path;
}


void
YStringTree::logTree()
{
    printf( "Tree:\n" );
    logBranch( _root, "" );
    printf( " " );
}


void
YStringTree::logBranch( YStringTreeItem * branch, string indentation )
{
    if ( branch )
    {
        printf( "%s%s (%s)\n", indentation.c_str(),
                branch->value().translation().c_str(),
                branch->value().orig().c_str() );

        YStringTreeItem * child = branch->firstChild();
        indentation += "    ";

        while ( child )
        {
            logBranch( child, indentation );
            child = child->next();
        }
    }
    else
    {
        printf( "%s<NULL>\n", indentation.c_str() );
    }
}
