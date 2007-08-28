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

//  expensecluster.cpp
//  Copyright (c) 2002-2007, OpenMFG, LLC
// TODO: replace with subclasses of VirtualCluster and related classes
// TODO: add ReadOnly property to VirtualCluster and related classes

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

#include <parameter.h>
#include <xsqlquery.h>

#include "xcombobox.h"
#include "xlineedit.h"

#include "expenseList.h"
#include "expensecluster.h"

#include "../common/format.h"

ExpenseLineEdit::ExpenseLineEdit(QWidget *pParent, const char *name) :
  XLineEdit(pParent, name)
{
  _parsed = TRUE;

  connect(this, SIGNAL(lostFocus()),   this, SLOT(sParse()));
  connect(this, SIGNAL(requestList()), this, SLOT(sExpenseList()));
}

void ExpenseLineEdit::setId(int pId)
{
  if (pId != -1)
  {
    XSqlQuery qry;
    qry.prepare( "SELECT expcat_code AS expnumber"
                "  FROM expcat"
                " WHERE (expcat_id=:expcat_id); ");
    qry.bindValue(":expcat_id", pId);
    qry.exec();
    if (qry.first())
    {
      _id    = pId;
      _valid = TRUE;

      setText(qry.value("expnumber").toString());

      emit newId(_id);
      emit valid(TRUE);

      _parsed = TRUE;

      return;
    }
  }

  _id    = -1;
  _valid = FALSE;

  setText("");

  emit newId(-1);
  emit valid(FALSE);
    
  _parsed = TRUE;
}

void ExpenseLineEdit::sParse()
{
  if (!_parsed)
  {
    if (text().stripWhiteSpace().length() == 0)
      setId(-1);
    else
    {
      XSqlQuery exp;
      exp.prepare("SELECT expcat_id "
                  "  FROM expcat "
                  " WHERE (expcat_code=:expcat_code)");
      exp.bindValue(":expcat_code", text().stripWhiteSpace().upper());
      exp.exec();
      if (exp.first())
        setId(exp.value("expcat_id").toInt());
      else
        setId(-1);
    }
  }
}

void ExpenseLineEdit::sExpenseList()
{
  ParameterList params;
  params.append("id", id());

  expenseList newdlg(parentWidget(), "", TRUE);
  newdlg.set(params);
  
  setId(newdlg.exec());
}

ExpenseCluster::ExpenseCluster(QWidget *pParent, const char *name) :
  QWidget(pParent, name)
{
  constructor();
}

void ExpenseCluster::constructor()
{
//  Create the component Widgets
  QHBoxLayout *_expLayout = new QHBoxLayout(this);
  _expLayout->setMargin(0);
  _expLayout->setSpacing(5);

  _expNumberLit = new QLabel(tr("Expense Category:"), this, "_expNumberLit");
  _expNumberLit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _expNumberLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  _expLayout->addWidget(_expNumberLit);

  _expNumber = new ExpenseLineEdit(this);
  _expNumber->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _expLayout->addWidget(_expNumber);

  _expList = new QPushButton(tr("..."), this, "_expList");
  _expList->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
#ifdef Q_WS_MAC
  _expList->setMaximumWidth(50);
#else
  _expList->setMaximumWidth(25);
#endif
  _expList->setFocusPolicy(Qt::NoFocus);
  _expLayout->addWidget(_expList);

  setLayout(_expLayout);

//  Make some internal connections
  connect(_expNumber, SIGNAL(newId(int)), this, SIGNAL(newId(int)));
  connect(_expNumber, SIGNAL(valid(bool)), this, SIGNAL(valid(bool)));

  connect(_expList, SIGNAL(clicked()), SLOT(sExpenseList()));

  setFocusProxy(_expNumber);
}

void ExpenseCluster::setReadOnly(bool pReadOnly)
{
  if (pReadOnly)
  {
    _expNumber->setEnabled(FALSE);
    _expList->hide();
  }
  else
  {
    _expNumber->setEnabled(TRUE);
    _expList->show();
  }
}

void ExpenseCluster::setLabel(QString label)
{
  _expNumberLit->setText(label);
}

void ExpenseCluster::setId(int pId)
{
  _expNumber->setId(pId);
}

void ExpenseCluster::sExpenseList()
{
  _expNumber->sExpenseList();

  if (_expNumber->_id != -1)
  {
    _expNumber->setFocus();
    focusNextPrevChild(TRUE);
  }
}
