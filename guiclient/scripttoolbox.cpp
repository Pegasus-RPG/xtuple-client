/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "scripttoolbox.h"

#include <QWidget>
#include <QLayout>
#include <QGridLayout>
#include <QBoxLayout>
#include <QStackedLayout>
#include <QMessageBox>
#include <QScriptEngine>

#include <parameter.h>
#include <metasql.h>

#include "scriptquery.h"

ScriptToolbox::ScriptToolbox(QScriptEngine * engine)
  : QObject(engine)
{
  _engine = engine;
}

ScriptToolbox::~ScriptToolbox()
{
}

QObject * ScriptToolbox::executeQuery(const QString & query, QVariantMap parameters)
{
  ParameterList params;
  QMapIterator<QString, QVariant> i(parameters);
  while (i.hasNext())
  {
    i.next();
    params.append(i.key(), i.value());
  }

  ScriptQuery * sq = new ScriptQuery(_engine);
  MetaSQLQuery mql(query);
  sq->setQuery(mql.toQuery(params));
  return sq;
}

QLayout * ScriptToolbox::widgetGetLayout(QWidget * w)
{
  if(w)
    return w->layout();
  return NULL;
}

void ScriptToolbox::layoutBoxInsertWidget(QBoxLayout * layout, int index, QWidget * widget, int stretch, Qt::Alignment alignment)
{
  if(layout && widget)
    layout->insertWidget(index, widget, stretch, alignment);
}

void ScriptToolbox::layoutGridAddWidget(QGridLayout * layout, QWidget * widget, int row, int column, Qt::Alignment alignment)
{
  if(layout && widget)
    layout->addWidget(widget, row, column, alignment);
}

void ScriptToolbox::layoutGridAddWidget(QGridLayout * layout, QWidget * widget, int fromRow, int fromColumn, int rowSpan, int columnSpan, Qt::Alignment alignment)
{
  if(layout && widget)
    layout->addWidget(widget, fromRow, fromColumn, rowSpan, columnSpan, alignment);
}

void ScriptToolbox::layoutStackedInsertWidget(QStackedLayout * layout, int index, QWidget * widget)
{
  if(layout && widget)
    layout->insertWidget(index, widget);
}

int ScriptToolbox::messageBox(const QString & type, QWidget * parent, const QString & title, const QString & text, int buttons, int defaultButton)
{
  int btn;
  if(type == "critical")
    btn = QMessageBox::critical(parent, title, text, buttons, defaultButton);
  else if(type == "information")
    btn = QMessageBox::information(parent, title, text, buttons, defaultButton);
  else if(type == "question")
    btn = QMessageBox::question(parent, title, text, buttons, defaultButton);
  else //if(type == "warning")
    btn = QMessageBox::warning(parent, title, text, buttons, defaultButton);
  return btn;
}
