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


#include "YQPkgFileListView.h"
#include "YQi18n.h"
#include "utf8.h"


#define MAX_LINES 500


YQPkgFileListView::YQPkgFileListView( QWidget * parent )
    : YQPkgGenericDetailsView( parent )
{
}


YQPkgFileListView::~YQPkgFileListView()
{
    // NOP
}


void
YQPkgFileListView::showDetails( ZyppSel selectable )
{
    _selectable = selectable;

    if ( ! selectable )
    {
        clear();
        return;
    }

    QString html = htmlHeading( selectable,
                                false ); // showVersion

    ZyppPkg installed = tryCastToZyppPkg( selectable->installedObj() );

    if ( installed )
    {
        zypp::Package::FileList fileList( installed->filelist() );
        std::list<string> stringList( fileList.begin(), fileList.end() );

        html += formatFileList( stringList );
    }
    else
    {
        html += "<p><i>" + _( "Information only available for installed packages." ) + "</i></p>";
    }

    setHtml( html );
}



QString YQPkgFileListView::formatFileList( const std::list<string> & fileList ) const
{
    QString  html;
    unsigned lineCount = 0;

    for ( std::list<string>::const_iterator it = fileList.begin();
          it != fileList.end() && lineCount < MAX_LINES;
          ++it, ++lineCount )
    {
        QString line = htmlEscape( fromUTF8( *it ) );

        if ( line.contains( "/bin/"  ) ||
             line.contains( "/sbin/" )   )
        {
            line = "<b>" + line + "</b>";
        }

        html += line + "<br>";
    }

    if ( fileList.size() > MAX_LINES )
    {
        html += "...<br>";
        html += "...<br>";
    }
    else
    {
        // %1 is the total number of files in a file list
        html += "<br>" + _( "%1 files total" ).arg( (unsigned long) fileList.size() );
    }

    return "<p>" + html + "</p>";
}

