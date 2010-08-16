/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __DISPLAYTIMEPHASED_H__
#define __DISPLAYTIMEPHASED_H__

#include "display.h"

class displayTimePhasedPrivate;

class displayTimePhased : public XWidget
{
    Q_OBJECT

    friend class displayPrivate;

public:
    displayTimePhased(QWidget* = 0, const char* = 0, Qt::WindowFlags = 0);
    ~displayTimePhased();

    virtual bool setParams(ParameterList &) = 0;

protected:
    Q_INVOKABLE QWidget * optionsWidget();

private:
    displayTimePhasedPrivate * _data;
};

#endif // __DISPLAYTIMEPHASED_H__
