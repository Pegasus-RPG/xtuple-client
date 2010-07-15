/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <parameter.h>
#include <xsqlquery.h>

#include "shipToList.h"
#include "shiptocluster.h"

//   ShiptoLineEdit - A Ship To # validating XLineEdit
ShiptoEdit::ShiptoEdit(QWidget *pParent, const char *name) :
  XLineEdit(pParent, name)
{
  _custid = -1;

  connect(this, SIGNAL(lostFocus()), this, SLOT(sParse()));
}

void ShiptoEdit::sParse()
{
  if (!_parsed)
  {
    _parsed = TRUE;

    if (text().length() == 0)
      setId(-1);

    XSqlQuery shipto;
    shipto.prepare( "SELECT shipto_id "
                    "  FROM shipto "
                    " WHERE ( (shipto_cust_id=:cust_id)"
                    "   AND   (UPPER(shipto_num)=UPPER(:shipto_num)) );" );
    shipto.bindValue(":cust_id", _custid);
    shipto.bindValue(":shipto_num", text());
    shipto.exec();
    if (shipto.first())
      setId(shipto.value("shipto_id").toInt());
    else
      clear();
  }
}

void ShiptoEdit::setId(int pId)
{
  if (pId != -1)
  {
    XSqlQuery shipto( QString( "SELECT shipto_num, shipto_name, shipto_address1, shipto_cust_id "
                               "FROM shipto "
                               "WHERE (shipto_id=%1);" )
                      .arg(pId) );
    if (shipto.first())
    {
      _id = pId;
      _valid = TRUE;

      setText(shipto.value("shipto_num").toString());

      if (shipto.value("shipto_cust_id").toInt() != _custid)
      {
        _custid = shipto.value("shipto_cust_id").toInt();
        emit newCustid(_id);
      }

      emit newId(_id);
      emit valid(_valid);
      emit nameChanged(shipto.value("shipto_name").toString());
      emit address1Changed(shipto.value("shipto_address1").toString());
    }
  }
  else
  {
    clear();

    _custid = -1;
    emit newCustid(_custid);
  }
}

void ShiptoEdit::setCustid(int pCustid)
{
  if (pCustid != _custid)
  {
    clear();

    _custid = pCustid;
    emit newCustid(pCustid);

    if (_custid == -1)
    {
      setEnabled(FALSE);
      emit disableList(TRUE);
    }
    else
    {
      setEnabled(TRUE);
      emit disableList(FALSE);
    }
  }
}

void ShiptoEdit::clear()
{
  _id = -1;
  _valid = FALSE;

  setText("");

  emit nameChanged("");
  emit address1Changed("");
  emit newId(_id);
  emit valid(_valid);
}


ShiptoCluster::ShiptoCluster(QWidget *pParent, const char *name) :
  QWidget(pParent)
{
  setObjectName(name);

  QHBoxLayout *_layoutMain         = new QHBoxLayout(this);
  QVBoxLayout *_layoutLit          = new QVBoxLayout(0);
  QVBoxLayout *_layoutData         = new QVBoxLayout(0);
  QHBoxLayout *_layoutShipto       = new QHBoxLayout(0);
  QHBoxLayout *_layoutShiptoNumber = new QHBoxLayout(0);

  _layoutMain->setContentsMargins(0, 0, 0, 0);
  _layoutLit->setContentsMargins(0, 0, 0, 0);
  _layoutData->setContentsMargins(0, 0, 0, 0);
  _layoutShipto->setContentsMargins(0, 0, 0, 0);
  _layoutShiptoNumber->setContentsMargins(0, 0, 0, 0);

  _layoutMain->setSpacing(5);
  _layoutLit->setSpacing(0);
  _layoutData->setSpacing(0);
  _layoutShipto->setSpacing(0);
  _layoutShiptoNumber->setSpacing(7);

  QLabel *_shiptoNumberLit = new QLabel(tr("Ship-To #:"), this);
  _shiptoNumberLit->setObjectName("_shiptoNumberLit");
  _shiptoNumberLit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _shiptoNumberLit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  QSize size = _shiptoNumberLit->minimumSize();
  size.setHeight(28);
  _shiptoNumberLit->setMinimumSize(size);
  _layoutLit->addWidget( _shiptoNumberLit );

  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  _layoutLit->addItem( spacer );
  _layoutMain->addLayout( _layoutLit );

  _shiptoNumber = new ShiptoEdit( this, "_shiptoNumber" );
  _shiptoNumber->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _layoutShiptoNumber->addWidget( _shiptoNumber );

  _list = new QPushButton(tr( "..." ), this);
  _list->setObjectName("_list");
  _list->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
#ifndef Q_WS_MAC
  _list->setMaximumWidth(25);
#endif
  _list->setFocusPolicy(Qt::NoFocus);
  _layoutShiptoNumber->addWidget( _list );
  _layoutShipto->addLayout( _layoutShiptoNumber );

  QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  _layoutShipto->addItem( spacer_2 );
  _layoutData->addLayout( _layoutShipto );

  _shiptoName = new QLabel(this);
  _shiptoName->setObjectName("_shiptoName");
  _shiptoName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _shiptoName->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
  _layoutData->addWidget( _shiptoName );

  _shiptoAddress1 = new QLabel( this);
  _shiptoAddress1->setObjectName("_shiptoAddress1" );
  _shiptoAddress1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _shiptoAddress1->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
  _layoutData->addWidget( _shiptoAddress1 );
  _layoutMain->addLayout( _layoutData );

//  Make some internal connections
  connect(_list,  SIGNAL(clicked()),  this, SLOT(sList()));
  connect(_shiptoNumber, SIGNAL(requestList()), this, SLOT(sList()));
  connect(_shiptoNumber, SIGNAL(nameChanged(const QString &)), _shiptoName, SLOT(setText(const QString &)));
  connect(_shiptoNumber, SIGNAL(address1Changed(const QString &)), _shiptoAddress1, SLOT(setText(const QString &)));
  connect(_shiptoNumber, SIGNAL(disableList(bool)), _list, SLOT(setDisabled(bool)));

  connect(_shiptoNumber, SIGNAL(newId(int)), this, SIGNAL(newId(int)));
  connect(_shiptoNumber, SIGNAL(valid(bool)), this, SIGNAL(valid(bool)));
  connect(_shiptoNumber, SIGNAL(newCustid(int)), this, SIGNAL(newCustid(int)));

  setFocusProxy(_shiptoNumber);

  setCustid(-1);
}

void ShiptoCluster::setId(int pId)
{
  _shiptoNumber->setId(pId);
}

void ShiptoCluster::setCustid(int pId)
{
  _shiptoNumber->setCustid(pId);
}

void ShiptoCluster::setReadOnly(bool pReadOnly)
{
  if (pReadOnly)
  {
    _shiptoNumber->setEnabled(FALSE);
    _list->hide();
  }
  else
  {
    _shiptoNumber->setEnabled(TRUE);
    _list->show();
  }
}

void ShiptoCluster::sList()
{
  ParameterList params;
  params.append("cust_id", _shiptoNumber->_custid);
  params.append("shipto_id", _shiptoNumber->_id);

  shipToList newdlg(parentWidget(), "", TRUE);
  newdlg.set(params);

  int id = newdlg.exec();
  _shiptoNumber->setId(id);

  if (id != -1)
  {
    _shiptoNumber->setFocus();
    focusNextPrevChild(TRUE);
  }
}

