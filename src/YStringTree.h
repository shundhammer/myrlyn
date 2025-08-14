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


#ifndef YStringTree_h
#define YStringTree_h

#include <string>
#include "YTransText.h"
#include "TreeItem.h"


typedef SortedTreeItem<YTransText>  YStringTreeItem;


/**
 * Abstract base class for filter views with hierarchical filter
 * criteria - e.g. RPM groups.
 **/
class YStringTree
{
public:

    /**
     * Constructor: Create a a tree with the specified textdomain.
     **/

    YStringTree( const std::string & textdomain );

    /**
     * Destructor.
     **/
    virtual ~YStringTree();

    /**
     * Return the textdomain used for translating new items.
     **/
    std::string textdomain() const { return _textdomain; }

    /**
     * Add a unique new branch with text content 'content' to the tree,
     * beginning at 'parent' (root if parent == 0).  This content can be a path
     * specification delimited with character 'delimiter' (if not 0), i.e. this
     * method will split 'content' up into path components and insert tree
     * items for each level as appropriate. Leading delimiters will be ignored.
     * If 'delimiter' is 0, 'content' is not split but used 'as is'.  Items are
     * automatically sorted alphabetically. Pathname components are
     * automatically translated.
     *
     * Returns the tree node for this branch - either newly created or the
     * existing one.
     *
     *
     * Example:
     *    addBranch( "/usr/local/bin", '/' )
     *    addBranch( "/usr/lib", '/' )
     *
     *  "usr"
     *          "lib"
     *          "local"
     *                  "bin"
     **/
    YStringTreeItem * addBranch( const std::string & content,
                                 char                delimiter = 0,
                                 YStringTreeItem *   parent    = 0 );


    /**
     * Construct a complete original path for the specified tree item.
     * 'startWithDelimiter' specifies whether or not the complete path should
     * start with the delimiter character.
     **/
    std::string origPath( const YStringTreeItem *    item,
                          char                       delimiter,
                          bool                       startWithDelimiter = true )
        { return completePath( item, false, delimiter, startWithDelimiter ); }


    /**
     * Construct a complete original path for the specified tree item.
     * 'startWithDelimiter' specifies whether or not the complete path should
     * start with the delimiter character.
     **/
    std::string translatedPath( const YStringTreeItem * item,
                                char delimiter,
                                bool startWithDelimiter = true )
        { return completePath( item, true, delimiter, startWithDelimiter ); }


    /**
     * Construct a complete path (both original and translated) for the
     * specified tree item. 'startWithDelimiter' specifies whether or not the
     * complete path should start with the delimiter character.
     *
     * Note: origPath() or translatedPath() are much cheaper if only one
     * version (original or translated) is required.
     **/
    YTransText path( const YStringTreeItem * item,
                     char delimiter,
                     bool startWithDelimiter = true );


    /**
     * Debugging - dump the tree into the log file.
     **/
    void logTree();


    /**
     * Returns the root of the filter view tree.
     * Note: In most cases, the root item itself will not contain any useful
     * information. Consider it the handle for the entire tree, not an actual
     * data element.
     **/
    YStringTreeItem * root() const { return _root; }


    /**
     * Translate message 'orig'. Returns the translated text or the original if
     * there is no translation.
     **/
    std::string translate( const std::string & orig );


protected:

    /**
     * Construct a complete original or translated path for the specified tree
     * item.  'startWithDelimiter' specifies whether or not the complete path
     * should start with the delimiter character.
     **/
    std::string completePath( const YStringTreeItem * item,
                              bool translated,
                              char delimiter,
                              bool startWithDelimiter );

    /**
     * Debugging - dump one branch of the tree into the log file.
     **/
    void logBranch( YStringTreeItem * branch, std::string indentation );


    // Data members

    std::string       _textdomain;
    YStringTreeItem * _root;
};


#endif // YStringTree_h
