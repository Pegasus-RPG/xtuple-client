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
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
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

// workcenterCluster.cpp
// Copyright (c) 2002-2007, OpenMFG, LLC

#include <QPushButton>
#include <QLabel>
#include <QValidator>
#include <QHBoxLayout>

#include <parameter.h>
#include <xsqlquery.h>

#include "workcenterList.h"
#include "workcentercluster.h"

WorkCenterLineEdit::WorkCenterLineEdit(QWidget * parent, const char * name)
  : XLineEdit(parent, name)
{
  _id = -1;

  setMaximumWidth(100);

  connect(this, SIGNAL(lostFocus()), SLOT(sParse()));
}

void WorkCenterLineEdit::setId(int pId)
{
  QString sql("SELECT wrkcnt_code"
              "  FROM wrkcnt"
              " WHERE (wrkcnt_id=:id);");

  XSqlQuery query;
  query.prepare(sql);
  query.bindValue(":id", pId);
  query.exec();
  if(query.first())
  {
    _id = pId;
    _valid = true;
    _workcenter = query.value("wrkcnt_code").toString();
  }
  else
  {
    _id = -1;
    _valid = false;
    _workcenter = "";
  }

  setText(_workcenter);
  emit newId(_id);
  emit valid(_valid);

  _parsed = true;
}

void WorkCenterLineEdit::setWorkCenter(const QString & pWorkCenter)
{
  XSqlQuery query;
  query.prepare("SELECT wrkcnt_id"
                "  FROM wrkcnt"
                " WHERE (wrkcnt_code=:workcenter);");
  query.bindValue(":workcenter", pWorkCenter);
  query.exec();
  if(query.first())
    setId(query.value("wrkcnt_id").toInt());
  else
    setId(-1);
}

void WorkCenterLineEdit::clear()
{
  setId(-1);
}

void WorkCenterLineEdit::sParse()
{
  if(!_parsed)
  {
    _parsed = true;
    setWorkCenter(text());
  }
}

WorkCenterCluster::WorkCenterCluster(QWidget * parent, const char * name)
  : QWidget(parent, name)
{
  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setMargin(0);
  layout->setSpacing(0);

  _label = new QLabel(tr("Work Center:"), this);
  _label->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  layout->addWidget(_label);

  _workcenter = new WorkCenterLineEdit(this);
  layout->addWidget(_workcenter);

  _list = new QPushButton(tr("..."), this);
  _list->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
#ifdef Q_WS_MAC
  _list->setMaximumWidth(50);
#else
  _list->setMaximumWidth(25);
#endif
  _list->setFocusPolicy(Qt::NoFocus);
  layout->addWidget(_list);

  QSpacerItem * spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);
  layout->addItem(spacer);

  setLayout(layout);

  connect(_list, SIGNAL(clicked()), this, SLOT(sList()));
  connect(_workcenter, SIGNAL(newId(int)), this, SIGNAL(newId(int)));
  connect(_workcenter, SIGNAL(valid(bool)), this, SIGNAL(valid(bool)));

  setFocusProxy(_workcenter);
}

void WorkCenterCluster::sList()
{
  ParameterList params;
  params.append("id", _workcenter->id());

  workcenterList newdlg(parentWidget(), "", TRUE);
  newdlg.set(params);

  int id;
  if((id = newdlg.exec()) != QDialog::Rejected)
    _workcenter->setId(id);
}

void WorkCenterCluster::setReadOnly(bool pReadOnly)
{
  if(pReadOnly)
    _list->hide();
  else
    _list->show();
  _workcenter->setEnabled(!pReadOnly);
}
