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

//  quoteWidgets.cpp
//  Created 03/04/2002 JSL
//  Copyright (c) 2002-2007, OpenMFG, LLC

#include <QLabel>
#include <QPushButton>
#include <QValidator>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <parameter.h>
#include <xsqlquery.h>

#include "quoteList.h"
#include "quoteWidgets.h"

//  Routines for QuoteLine - an sohead validating XLineEdit
QuoteLine::QuoteLine(QWidget *pParent, const char *name) : XLineEdit(pParent, name)
{
  _id     = -1;
  _number = -1;
  _valid  = FALSE;

  _custid = -1;

  setValidator(new QIntValidator(0, 9999999, this));

  connect(this, SIGNAL(lostFocus()), this, SLOT(sParse()));
}

void QuoteLine::setId(int pId)
{
  if (pId == _id)
    return;

  XSqlQuery quhead;
  quhead.prepare( "SELECT quhead_number, quhead_cust_id, quhead_billtoname "
                  "FROM quhead, cust "
                  "WHERE ( (quhead_cust_id=cust_id)"
                  " AND (quhead_id=:sohead_id) );" );
  quhead.bindValue(":sohead_id", pId);
  quhead.exec();
  if (quhead.first())
  {
    _id     = pId;
    _number = quhead.value("quhead_number").toInt();
    _custid = quhead.value("quhead_cust_id").toInt();
    _valid  = TRUE;

    emit newNumber(quhead.value("quhead_number").toString());
    emit newCustName(quhead.value("quhead_billtoname").toString());

    setText(quhead.value("quhead_number").toString());
  }
  else
  {
    _id     = -1;
    _number = -1;
    _custid = -1;
    _valid  = FALSE;

    emit newNumber("");
    emit newCustName("");
    setText("");
  }

  emit newId(_id);
  emit newCustId(_custid);
  emit newNumber(_number);
  emit valid(_valid);

  _parsed = TRUE;
}

void QuoteLine::setNumber(int pNumber)
{
  XSqlQuery quhead;
    quhead.prepare( "SELECT quhead_id "
                    "FROM quhead "
                    "WHERE (quhead_number=:quhead_number);" );

  quhead.bindValue(":quhead", pNumber);
  quhead.exec();
  if (quhead.first())
    setId(quhead.value("quhead_id").toInt());
  else
    setId(-1);
}

void QuoteLine::clear()
{
  setId(-1);
}

void QuoteLine::sParse()
{
  if (!_parsed)
  {
    bool numeric;
    int  quoteNumber = text().toInt(&numeric);

    if (numeric)
      setNumber(quoteNumber);
  }
}


QuoteControl::QuoteControl(QWidget *pParent, const char *name) : QWidget(pParent, name)
{
  QVBoxLayout * vLayout = new QVBoxLayout(this);
  vLayout->setMargin(0);
  vLayout->setSpacing(6);
  QWidget * hWidget = new QWidget(this);
  QHBoxLayout * hLayout = new QHBoxLayout(hWidget);
  hLayout->setMargin(0);
  hLayout->setSpacing(0);

  _quoteLineLit = new QLabel(tr("Sales Order #:"), hWidget);
  //_quoteLineLit->setGeometry(0, 0, 80, 25);
  _quoteLineLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  hLayout->addWidget(_quoteLineLit);
  hLayout->addSpacing(6);

  _quoteLine = new QuoteLine(hWidget);
  //_quoteLine->setGeometry(85, 0, 100, 25);
  hLayout->addWidget(_quoteLine);

  _quoteList = new QPushButton(tr("..."), hWidget);
  //_quoteList->setGeometry(190, 0, 25, 25);
  _quoteList->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, _quoteList->sizePolicy().hasHeightForWidth() ) );
  _quoteList->setMinimumSize( QSize( 25, 25 ) );
  _quoteList->setMaximumSize( QSize( 25, 25 ) );
  _quoteList->setFocusPolicy(Qt::NoFocus);
  hLayout->addWidget(_quoteList);

  hWidget->setLayout(vLayout);
  vLayout->addWidget(hWidget);

  _custName = new QLabel(this);
  //_custName->setGeometry(85, 30, 210, 25);
  _custName->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  vLayout->addWidget(_custName);

  setLayout(vLayout);

  connect(_quoteList, SIGNAL(clicked()), this, SLOT(sList()));
  connect(_quoteLine, SIGNAL(newCustName(const QString &)), _custName, SLOT(setText(const QString &)));
  connect(_quoteLine, SIGNAL(newId(int)), this, SIGNAL(newId(int)));
  connect(_quoteLine, SIGNAL(newCustId(int)), this, SIGNAL(newCustid(int)));
  connect(_quoteLine, SIGNAL(valid(bool)), this, SIGNAL(valid(bool)));

  setFocusProxy(_quoteLine);
}

void QuoteControl::setId(int pQuheadid)
{
  _quoteLine->setId(pQuheadid);
}

void QuoteControl::sList()
{
  ParameterList params;
  params.append("quhead_id", _quoteLine->_id);

  quoteList newdlg(parentWidget(), "", TRUE);
  newdlg.set(params);

  int id = newdlg.exec();
  if (id != QDialog::Rejected)
    _quoteLine->setId(id);
}


