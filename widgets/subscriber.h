/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include "ui_subscriber.h"

class QWidget;
class QDialog;
class ParameterList;

class subscriber : public QDialog, private Ui::subscriber
{
    Q_OBJECT
    public:
        subscriber(QWidget*,const char* = 0,bool = false,Qt::WindowFlags = 0);
        QString whichSelection();
    public slots:
        virtual void set(ParameterList&);
        void addSelected();
        void switchSelection();
    private:
        int _incdtid;
        bool _whichRButton;
};

#endif // SUBSCRIBER_H
