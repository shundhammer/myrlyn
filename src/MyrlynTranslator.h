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


#ifndef MyrlynTranslator_h
#define MyrlynTranslator_h

#include <QTranslator>

/**
 * Class to manage translations for widget texts and other user-visible
 * messages.
 *
 * This uses Qt mechanisms like Qt::tr(), but it maps them to GNU gettext calls
 * like dcgettext(). Since it uses common Qt mechanisms, it also works for Qt
 * .ui files generated with Qt Designer.
 **/
class MyrlynTranslator: public QTranslator
{
    Q_OBJECT

public:

    MyrlynTranslator( QObject * parent );
    virtual ~MyrlynTranslator();

    /**
     * Reimplemented from QTranslator:
     *
     * Use GNU gettext functions like dgettext() to get the message
     * translations unless a Qt-internal context is detected.
     *
     * Notice that this is also used for Qt's own messages that need to be
     * translated, such as predefined dialogs (file dialog) and context menus
     * (e.g. when right-clicking input fields, lists, scroll bars), so we
     * cannot simply use GNU gettext everywhere.
     **/
    virtual QString translate( const char * context,
                               const char * sourceText,
                               const char * disambiguation = 0,
                               int          nPlural        = -1 ) const override;

    /**
     * Return a fake translation "xixoxixoxixo" with the same length as
     * 'sourceText'. This can be useful to see where translation markers are
     * missing in the code.
     *
     * Use the '--fake-translations' command line option to enable this.
     *
     **/
    QString fakeTranslation( const char * sourceText ) const;

protected:

    QString _fakeTemplate;

};      // class MyrlynTranslator


#endif  // MyrlynTranslator_h
