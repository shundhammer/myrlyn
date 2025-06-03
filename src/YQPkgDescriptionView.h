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


#ifndef YQPkgDescriptionView_h
#define YQPkgDescriptionView_h


#include <QUrl>

#include "YQZypp.h"
#include "YQPkgGenericDetailsView.h"

using std::list;
using std::string;


/**
 * Display the description of a ZyppObj derived object along with its
 * name and summary.
 **/
class YQPkgDescriptionView : public YQPkgGenericDetailsView
{
    Q_OBJECT

public:

    /**
     * Constructor
     **/
    YQPkgDescriptionView( QWidget * parent );

    /**
     * Destructor
     **/
    virtual ~YQPkgDescriptionView();

    /**
     * Show details for the specified package:
     * In this case the package description.
     *
     * Reimplemented from YQPkgGenericDetailsView.
     **/
    virtual void showDetails( ZyppSel selectable ) override;

#if 0
    /**
     * Get the document pointed to by a hyperlink.
     *
     * Reimplemented from QTextBrowser to avoid having an empty text each time
     * the user clicks on a hyperlink.
     **/
    virtual void setSource( const QUrl & name ) override;

    /**
     * Show information for a hyperlinked object,
     * e.g., a "pkg:somepkg" link to another package.
     **/
    void showLink( const QUrl & url );
#endif


protected:

    /**
     * Format a multi-line text into paragraphs
     **/
    QString simpleHtmlParagraphs( QString text );

    /**
     * Return html text that contains a list of application icons.
     **/
    QString applicationIconList( const list<string> & fileList ) const;

    /**
     * Find absolute file name (incl. path) for a icon.
     **/
    QString findDesktopIcon ( const QString& iconName ) const;

    /**
     * Extract name, icon and exec attributes from a desktop file.
     **/
    QMap<QString, QString> readDesktopFile( const QString & fileName ) const;

    /**
     * Search for all desktop files in a file list.
     **/
    QStringList findDesktopFiles( const list<string> & fileList ) const;

    /**
     * Initialize the language code (lang).
     **/
    void initLang();

private:

    QString _langWithCountry;
    QString _lang;
};


#endif // ifndef YQPkgDescriptionView_h
