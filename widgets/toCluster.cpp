/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QLabel>
#include <QPushButton>
#include <QValidator>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <parameter.h>
#include <xsqlquery.h>

#include "transferOrderList.h"
#include "tocluster.h"

ToLineEdit::ToLineEdit(QWidget *pParent, const char *name) : XLineEdit(pParent, name)
{
  _id	  = -1;
  _number = -1;
  _srcwhs = -1;
  _dstwhs = -1;
  _valid  = false;

  setValidator(new QIntValidator(0, 9999999, this));

  connect(this, SIGNAL(lostFocus()), this, SLOT(sParse()));
}

void ToLineEdit::setId(int pId)
{
  if (pId == _id)
    return;

  XSqlQuery tohead;
  tohead.prepare( "SELECT tohead_number, tohead_src_warehous_id,"
		  "       tohead_dest_warehous_id "
                  "FROM tohead "
                  "WHERE (tohead_id=:tohead_id);" );
  tohead.bindValue(":tohead_id", pId);
  tohead.exec();
  if (tohead.first())
  {
    _id     = pId;
    _number = tohead.value("tohead_number").toInt();
    _srcwhs = tohead.value("tohead_src_warehous_id").toInt();
    _dstwhs = tohead.value("tohead_dest_warehous_id").toInt();
    _valid  = TRUE;

    setText(tohead.value("tohead_number").toString());
  }
  else
  {
    _id     = -1;
    _number = -1;
    _srcwhs = -1;
    _dstwhs = -1;
    _valid  = false;

    setText("");
  }

  emit newId(_id);
  emit numberChanged(_number);
  emit newSrcwhs(_srcwhs);
  emit newDstwhs(_dstwhs);
  emit valid(_valid);

  _parsed = TRUE;
}

void ToLineEdit::setNumber(int pNumber)
{
  XSqlQuery tohead;

  if (_type == cToOpen)
    tohead.prepare( "SELECT DISTINCT tohead_id "
                    "FROM tohead, toitem "
                    "WHERE ( (toitem_tohead_id=tohead_id)"
                    " AND (toitem_status NOT IN ('C','X'))"
                    " AND (tohead_number=text(:tohead_number)) );" );
  else if (_type == cToClosed)
    tohead.prepare( "SELECT DISTINCT tohead_id "
                    "FROM tohead, toitem "
                    "WHERE ( (toitem_tohead_id=tohead_id)"
                    " AND (toitem_status='C')"
                    " AND (tohead_number=text(:tohead_number)) );" );
  else if (_type == cToAtShipping)
    tohead.prepare( "SELECT DISTINCT tohead_id "
                    "FROM tohead, toitem, shiphead, shipitem "
                    "WHERE ((toitem_tohead_id=tohead_id)"
                    "  AND  (shiphead_order_id=tohead_id)"
                    "  AND  (shipitem_orderitem_id=toitem_id)"
                    "  AND  (NOT shiphead_shipped)"
		    "  AND  (shiphead_order_type='TO')"
                    "  AND  (tohead_number=text(:tohead_number)) );" );
  else
    tohead.prepare( "SELECT tohead_id "
                    "FROM tohead "
                    "WHERE (tohead_number=text(:tohead_number));" );

  tohead.bindValue(":tohead_number",pNumber);
  tohead.exec();
  if (tohead.first())
    setId(tohead.value("tohead_id").toInt());
  else
    setId(-1);
}

void ToLineEdit::clear()
{
  setId(-1);
}

void ToLineEdit::sParse()
{
  if (!_parsed)
  {
    bool numeric;
    int  orderNumber = text().toInt(&numeric);

    if (numeric)
      setNumber(orderNumber);
    else
      setId(-1);
  }
}


ToCluster::ToCluster(QWidget *pParent, const char *name) : QWidget(pParent, name)
{
  constructor();

  _toNumber->_type = 0;
}

ToCluster::ToCluster(int pType, QWidget *pParent) : QWidget(pParent)
{
  constructor();

  _toNumber->_type = pType;
}

void ToCluster::constructor()
{
  QGridLayout *_layoutMain = new QGridLayout(this);
  _layoutMain->setMargin(0);
  _layoutMain->setSpacing(1);

  QLabel *_toNumberLit	= new QLabel(tr("Transfer Order #:"));
  _toNumber		= new ToLineEdit(this);
  _list			= new QPushButton(tr("..."));
  QLabel *_srcwhsLit	= new QLabel(tr("From:"));
  _srcwhs		= new WComboBox(this, "_srcwhs");
  QLabel *_dstwhsLit	= new QLabel(tr("To:"));
  _dstwhs		= new WComboBox(this, "_dstwhs");

  _layoutMain->addWidget(_toNumberLit, 0, 0);
  _layoutMain->addWidget(_toNumber,    0, 1, 1, 2);
  _layoutMain->addWidget(_list,        0, 3);
  _layoutMain->addWidget(_srcwhsLit,   1, 0);
  _layoutMain->addWidget(_srcwhs,      1, 1);
  _layoutMain->addWidget(_dstwhsLit,   1, 2);
  _layoutMain->addWidget(_dstwhs,      1, 3);
  
  _toNumberLit->setObjectName("_toNumberLit");
  _toNumberLit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
  _toNumberLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

  _toNumber->setObjectName("_toNumber");
  _toNumber->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  _list->setObjectName("_list");
  _list->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
#ifndef Q_WS_MAC
  _list->setMaximumWidth(25);
#endif
  _list->setFocusPolicy(Qt::NoFocus);

  _srcwhsLit->setObjectName("_srcwhsLit");
  _srcwhsLit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
  _srcwhsLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  _srcwhsLit->setBuddy(_srcwhs);
  _srcwhs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _srcwhs->setEnabled(false);

  _dstwhsLit->setObjectName("_dstwhsLit");
  _dstwhsLit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
  _dstwhsLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  _dstwhsLit->setBuddy(_dstwhs);
  _dstwhs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _dstwhs->setEnabled(false);

  setLayout(_layoutMain);

  connect(_list, SIGNAL(clicked()), this, SLOT(sList()));
  connect(_toNumber, SIGNAL(requestList()), SLOT(sList()));
  connect(_toNumber, SIGNAL(newSrcwhs(const int)), _srcwhs, SLOT(setId(const int)));
  connect(_toNumber, SIGNAL(newDstwhs(const int)), _dstwhs, SLOT(setId(const int)));
  connect(_toNumber, SIGNAL(newId(int)), this, SIGNAL(newId(int)));
  connect(_toNumber, SIGNAL(valid(bool)), this, SIGNAL(valid(bool)));

  setFocusProxy(_toNumber);
  setFocusPolicy(Qt::StrongFocus);
}

void ToCluster::setId(int pid)
{
  _toNumber->setId(pid);
}

void ToCluster::sList()
{
  ParameterList params;
  params.append("tohead_id", _toNumber->_id);
  params.append("toType", _toNumber->_type);

  transferOrderList newdlg(parentWidget(), "", TRUE);
  newdlg.set(params);

  int id;
  if ((id = newdlg.exec()) != -1)
    _toNumber->setId(id);
}
