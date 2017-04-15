/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __SCRIPTABLEPRIVATE_H__
#define __SCRIPTABLEPRIVATE_H__

class QEvent;
class QScriptEngine;
class QWidget;

#include <QString>

#include "guiclient.h"
#include "parameter.h"
#include "scriptablewidget.h"

class ScriptablePrivate : public ScriptableWidget
{
  public:
    ScriptablePrivate(QWidget* parent);
    virtual ~ScriptablePrivate();

    virtual QScriptEngine   *engine();
    virtual enum SetResponse callSet(const ParameterList &);
    virtual void             callShowEvent(QEvent*);
    virtual void             callCloseEvent(QEvent*);

};

#endif
