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
#include <QValidator>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>

#include <parameter.h>
#include <xsqlquery.h>
#include <metasql.h>

#include "salesOrderList.h"
#include "socluster.h"

//  Routines for SoLineEdit - an sohead validating XLineEdit
SoLineEdit::SoLineEdit(QWidget *pParent, const char *name) : XLineEdit(pParent, name)
{
  _id     = -1;
  _number = -1;
  _custid = -1;
  _sitePrivs = true;

  setValidator(new QIntValidator(0, 9999999, this));
  _mapper = new XDataWidgetMapper(this);
  
  connect(this, SIGNAL(lostFocus()), this, SLOT(sParse()));
}

void SoLineEdit::setId(int pId)
{
  if (pId == _id)
    return;

  if ((_x_preferences) && (pId != -1) && (_sitePrivs))
  {
    if (_x_preferences->boolean("selectedSites"))
    {
      QString sql("SELECT coitem_id "
                  "FROM coitem, itemsite "
                  "WHERE ((coitem_cohead_id=<? value(\"sohead_id\") ?>) "
                  "  AND (coitem_itemsite_id=itemsite_id) "
                  "  AND (itemsite_warehous_id NOT IN ("
                  "       SELECT usrsite_warehous_id "
                  "       FROM usrsite "
                  "       WHERE (usrsite_username=current_user)))) "
                  "UNION "
                  "SELECT cohead_warehous_id "
                  "FROM cohead "
                  "WHERE ((cohead_id=<? value(\"sohead_id\") ?>) "
                  "  AND (cohead_warehous_id NOT IN ("
                  "       SELECT usrsite_warehous_id "
                  "       FROM usrsite "
                  "       WHERE (usrsite_username=current_user))));");
      MetaSQLQuery mql(sql);
      ParameterList params;
      params.append("sohead_id", pId);
      XSqlQuery chk = mql.toQuery(params);
      if (chk.first())
      {
              QMessageBox::critical(this, tr("Access Denied"),
                                    tr("You may not view or edit this Sales Order as it references "
                                       "a warehouse for which you have not been granted privileges.")) ;
              setId(-1);
              return;
      }
    }
  }


  XSqlQuery sohead;
  sohead.prepare( "SELECT cohead_number, cohead_cust_id, cohead_billtoname "
                  "FROM cohead, cust "
                  "WHERE ( (cohead_cust_id=cust_id)"
                  " AND (cohead_id=:sohead_id) );" );
  sohead.bindValue(":sohead_id", pId);
  sohead.exec();
  if (sohead.first())
  {
    _id     = pId;
    _number = sohead.value("cohead_number").toInt();
    _custid = sohead.value("cohead_cust_id").toInt();
    _valid  = TRUE;

    emit numberChanged(sohead.value("cohead_number").toString());
    emit custNameChanged(sohead.value("cohead_billtoname").toString());

    setText(sohead.value("cohead_number").toString());
    
    if (_mapper->model() &&
        _mapper->model()->data(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this))).toString() != text())
      _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this)), text());
  }
  else
  {
    _id     = -1;
    _number = -1;
    _custid = -1;
    _valid  = FALSE;

    emit numberChanged("");
    emit custNameChanged("");
    setText("");
  }

  emit newId(_id);
  emit custidChanged(_custid);
  emit numberChanged(_number);
  emit valid(_valid);

  _parsed = TRUE;
}

void SoLineEdit::setCustId(int pId)
{
  if (pId == _custid)
    return;
    
  _custid=pId;
  setNumber(_number);
  
  emit custidChanged(_custid);
}

void SoLineEdit::setNumber(QString pNumber)
{
  XSqlQuery sohead;

  if (_type == cSoOpen)
    sohead.prepare( "SELECT DISTINCT cohead_id "
                    "FROM cohead, coitem "
                    "WHERE ( (coitem_cohead_id=cohead_id)"
                    " AND (coitem_status NOT IN ('C','X'))"
                    " AND (cohead_number=:sohead_number) );" );
  else if (_type == cSoClosed)
    sohead.prepare( "SELECT DISTINCT cohead_id "
                    "FROM cohead, coitem "
                    "WHERE ( (coitem_cohead_id=cohead_id)"
                    " AND (coitem_status='C')"
                    " AND (cohead_number=:sohead_number) );" );
  else if (_type == cSoAtShipping)
    sohead.prepare( "SELECT DISTINCT cohead_id "
                    "FROM cohead, coitem, cosmisc, coship "
                    "WHERE ( (coitem_cohead_id=cohead_id)"
                    " AND (cosmisc_cohead_id=cohead_id)"
                    " AND (coship_coitem_id=coitem_id)"
                    " AND (NOT cosmisc_shipped)"
                    " AND (cohead_number=:sohead_number) );" );
  else if (_type == cSoCustomer)
  {
    sohead.prepare( "SELECT cohead_id "
                    "FROM cohead "
                    "WHERE ( (cohead_number=:sohead_number) "
                    "AND (cohead_cust_id=:cust_id) );" );
    sohead.bindValue(":cust_id", _custid);
  }
  else
    sohead.prepare( "SELECT cohead_id "
                    "FROM cohead "
                    "WHERE (cohead_number=:sohead_number);" );

  sohead.bindValue(":sohead_number", pNumber);
  sohead.exec();
  if (sohead.first())
    setId(sohead.value("cohead_id").toInt());
  else
    setId(-1);
}

void SoLineEdit::clear()
{
  setId(-1);
}

void SoLineEdit::sParse()
{
  if (!_parsed)
    setNumber(text());
}


SoCluster::SoCluster(QWidget *pParent, const char *name) : QWidget(pParent)
{
  setObjectName(name);
  constructor();

  _soNumber->_type = 0;
}

SoCluster::SoCluster(int pType, QWidget *pParent) : QWidget(pParent)
{
  constructor();

  _soNumber->_type = pType;
}

void SoCluster::constructor()
{
  _readOnly = false;
  QVBoxLayout *_layoutMain = new QVBoxLayout(this);
  _layoutMain->setMargin(0);
  _layoutMain->setSpacing(5);
  QWidget * _layoutOrderNumberWidget = new QWidget(this);
  QHBoxLayout *_layoutOrderNumber = new QHBoxLayout(_layoutOrderNumberWidget);
  _layoutOrderNumber->setMargin(0);
  _layoutOrderNumber->setSpacing(5);

  _soNumberLit = new QLabel(tr("Sales Order #:"), _layoutOrderNumberWidget);
  _soNumberLit->setObjectName("_soNumberLit");
  _soNumberLit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
  _soNumberLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  _layoutOrderNumber->addWidget(_soNumberLit);

  _soNumber = new SoLineEdit(_layoutOrderNumberWidget);
  _soNumber->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _layoutOrderNumber->addWidget(_soNumber);

  _list = new QPushButton(tr("..."), _layoutOrderNumberWidget);
  _list->setObjectName("_list");
  _list->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
#ifndef Q_WS_MAC
  _list->setMaximumWidth(25);
#endif
  _list->setFocusPolicy(Qt::NoFocus);
  _layoutOrderNumber->addWidget(_list);
  _layoutOrderNumberWidget->setLayout(_layoutOrderNumber);
  _layoutMain->addWidget(_layoutOrderNumberWidget);

  _custName = new QLabel(this);
  _custName->setObjectName("_custName");
  _custName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _custName->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  _layoutMain->addWidget(_custName);

  setLayout(_layoutMain);

  connect(_list, SIGNAL(clicked()), this, SLOT(sList()));
  connect(_soNumber, SIGNAL(requestList()), SLOT(sList()));
  connect(_soNumber, SIGNAL(custNameChanged(const QString &)), _custName, SLOT(setText(const QString &)));
  connect(_soNumber, SIGNAL(newId(int)), this, SIGNAL(newId(int)));
  connect(_soNumber, SIGNAL(custidChanged(int)), this, SIGNAL(newCustid(int)));
  connect(_soNumber, SIGNAL(valid(bool)), this, SIGNAL(valid(bool)));

  setFocusProxy(_soNumber);
}

void SoCluster::setId(int pSoid)
{
  _soNumber->setId(pSoid);
}

void SoCluster::setCustId(int pCustid)
{
  _soNumber->setCustId(pCustid);
}

void SoCluster::setDataWidgetMap(XDataWidgetMapper* m)
{
  m->addMapping(this, _fieldName, "number", "defaultNumber");
  _soNumber->_mapper=m;
}

void SoCluster::setLabel(const QString p)
{
  _label=p;
  _soNumberLit->setText(p);
}

void SoCluster::setNumber(QString pNumber)
{
  _soNumber->setNumber(pNumber);
}

void SoCluster::sList()
{
  ParameterList params;
  params.append("sohead_id", _soNumber->_id);
  params.append("soType", _soNumber->_type);
  params.append("cust_id", _soNumber->_custid);

  salesOrderList newdlg(parentWidget(), "", TRUE);
  newdlg.set(params);

  int id;
  if ((id = newdlg.exec()) != QDialog::Rejected)
    _soNumber->setId(id);
}

void SoCluster::setReadOnly(bool ro)
{
  _soNumber->setEnabled(!ro);
  _list->setVisible(!ro);
}
