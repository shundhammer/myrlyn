/*  ------------------------------------------------------
              __   _____  ____  _
              \ \ / / _ \|  _ \| | ____ _
               \ V / | | | |_) | |/ / _` |
                | || |_| |  __/|   < (_| |
                |_| \__\_\_|   |_|\_\__, |
                                    |___/
    ------------------------------------------------------

    Project:  Myrlyn Package Manager GUI
    Copyright (c) Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
    License:  GPL V2 - See file LICENSE for details.

 */

#ifndef myrlyn_askpass_h
#define myrlyn_askpass_h

#include <QDialog>

// Generated with 'uic' from a Qt designer .ui form: myrlyn-askpass.ui
//
// Check out ../build/aux/myrlyn-askpass_autogen/include/ui_myrlyn-askpass.h
// for the variable names of the widgets.

#include "ui_myrlyn-askpass.h"


class AskPassWin: public QDialog
{
    Q_OBJECT

public:

    AskPassWin( const QString & prompt = QString() );
    virtual ~AskPassWin();

    Ui::MyrlynAskPassWin * ui;

protected:

    QString _prompt;
};



#endif // myrlyn_askpass_h
