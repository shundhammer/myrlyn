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


#ifndef YQPkgTextDialog_h
#define YQPkgTextDialog_h


#include "YQZypp.h"
#include <QDialog>


class QPushButton;
class QTextBrowser;
class QEvent;

using std::string;


/**
 * Dialog that shows a scrolled (HTML) text.
 **/
class YQPkgTextDialog: public QDialog
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * Creates a dialog with a text browser and two buttons.
     * This does not pop up a dialog yet.
     * Use 'exec()' (inherited from QDialog).
     * Or, better yet, use the static confirmText() method.
     *
     **/
    YQPkgTextDialog( const QString & text,
                     QWidget *       parent,
                     const QString & acceptButtonLabel,
                     const QString & rejectButtonLabel );
    /**
     * Constructor.
     *
     * Creates a dialog with a text browser and an "OK" button.
     * This does not pop up a dialog yet.
     * Use 'exec()' (inherited from QDialog).
     * Or, better yet, use the static showText() method.
     **/
    YQPkgTextDialog( const QString & text, QWidget * parent );

    /**
     * Destructor.
     **/
    virtual ~YQPkgTextDialog();

    /**
     * Reimplemented from QWidget:
     * Reserve a reasonable amount of space.
     **/
    virtual QSize sizeHint() const override;

    /**
     * Show a text and wait until the user confirmed with 'OK'.
     **/
    static void showText( QWidget * parent, const QString & text );

    /**
     * Show a text with a headline identifying a selectable (name+summary).
     **/
    static void showText( QWidget *      parent,
                          ZyppSel        selectable,
                          const string & text );

    /**
     * Let the user confirm a text. Returns "true" if the user clicked the
     * accept button, false if he clicked the reject button.
     **/
    static bool confirmText( QWidget *       parent,
                             const QString & text,
                             const QString & acceptButtonLabel,
                             const QString & rejectButtonLabel );

    /**
     * Let the use confirm a text with buttons "Accept" and "Cancel".
     * Returns "true" if the user clicked "Accept", "false" on "Cancel".
     **/
    static bool confirmText( QWidget * parent, const QString & text );
    static bool confirmText( QWidget * parent, const char * text );

    /**
     * Let the use confirm a text with a headline identifying a selectable
     * (name+summary) with buttons "Accept" and "Cancel".
     * Returns "true" if the user clicked "Accept", "false" on "Cancel".
     **/
    static bool confirmText( QWidget *      parent,
                             ZyppSel        selectable,
                             const string & text );

    /**
     * Simple HTML formatting: Wrap paragraphs in <p>...</p>
     * Paragraphs are delimited by empty lines.
     * Return unformatted text if it contains "<!-- DT:Rich -->".
     **/
    static QString htmlParagraphs( const string & rawText );

    /**
     * Returns a uniform heading in HTML format.
     **/
    static QString htmlHeading( const QString & text );

    /**
     * Returns a uniform heading in HTML format for the specified selectable:
     * name and summary
     **/
    static QString htmlHeading( ZyppSel selectable );

    /**
     * Escapes characters special to HTML in a (plain text) string, such as:
     *   '<'   ->   '&lt;'
     *   '>'   ->   '&gt;'
     *   '&'   ->   '&amp;'
     *
     * Returns the escaped string.
     **/
    static QString htmlEscape( const QString & plainText );


public slots:

    /**
     * Set the text contents.
     **/
    void setText( const QString & text );
    void setText( const std::string & text );

    /**
     * Show a text with a headline identifying a selectable (name+summary)
     **/
    void setText( ZyppSel             selectable,
                  const std::string & text );

protected:

    /**
     * Create the dialog. Called from all constructors.
     **/
    void buildDialog( const QString & text,
                      QWidget *       parent,
                      const QString & acceptButtonLabel,
                      const QString & rejectButtonLabel = "" );
    /**
     * Grab [Return] press events and close dialog.
     * Inherited from QObject.
     **/
    bool eventFilter( QObject * obj, QEvent * ev ) override;


    // Data members

    QPushButton *  _acceptButton;
    QPushButton *  _rejectButton;
    QTextBrowser * _textBrowser;
};


#endif // ifndef YQPkgTextDialog_h
