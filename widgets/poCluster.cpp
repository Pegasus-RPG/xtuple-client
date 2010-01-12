/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QPushButton>
#include <QLabel>
#include <QValidator>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>

#include <parameter.h>
#include <xsqlquery.h>
#include <metasql.h>

#include "purchaseOrderList.h"
#include "pocluster.h"


//  Routines for PoLineEdit - an pohead validating XLineEdit
PoLineEdit::PoLineEdit(QWidget *parent, const char *name) :
  XLineEdit(parent, name)
{
  _id     = -1;
  _number = -1;
  _valid  = FALSE;
  _type   = 0;

  _vendid = -1;

  setValidator(new QIntValidator(0, 9999999, this));

  setMaximumWidth(100);

  connect(this, SIGNAL(lostFocus()), SLOT(sParse()));
  connect(this, SIGNAL(textChanged(QString)), SLOT(sMarkDirty()));
  
  _mapper = new XDataWidgetMapper(this);
}

// TODO: this really should be in XLineEdit
void PoLineEdit::sMarkDirty()
{
  _parsed = FALSE;
}

void PoLineEdit::setId(int pId)
{
  if ((_x_preferences) && (pId != -1))
  {
    if (_x_preferences->boolean("selectedSites"))
    {
      QString msql("SELECT poitem_id "
                  "FROM poitem, itemsite "
                  "WHERE ((poitem_pohead_id=<? value(\"pohead_id\") ?>) "
                  "  AND (poitem_itemsite_id=itemsite_id) "
                  "  AND (itemsite_warehous_id NOT IN ("
                  "       SELECT usrsite_warehous_id "
                  "       FROM usrsite "
                  "       WHERE (usrsite_username=current_user)))) "
                  "UNION "
                  "SELECT pohead_warehous_id "
                  "FROM pohead "
                  "WHERE ((pohead_id=<? value(\"pohead_id\") ?>) "
                  "  AND (pohead_warehous_id NOT IN ("
                  "       SELECT usrsite_warehous_id "
                  "       FROM usrsite "
                  "       WHERE (usrsite_username=current_user))));");
      MetaSQLQuery mql(msql);
      ParameterList params;
      params.append("pohead_id", pId);
      XSqlQuery chk = mql.toQuery(params);
      if (chk.first())
      {
              QMessageBox::critical(this, tr("Access Denied"),
                                    tr("You may not view or edit this Purchase Order as it references "
                                       "a warehouse for which you have not been granted privileges.")) ;
              setId(-1);
              return;
      }
    }
  }

  QString sql( "SELECT pohead_number, pohead_vend_id,"
               "       vend_name, vend_address1, vend_address2, vend_address3,"
               "       (vend_city || '  ' || vend_state || '  ' || vend_zip) AS citystatezip "
               "FROM pohead, vend "
               "WHERE ( (pohead_vend_id=vend_id)"
               " AND (pohead_id=:pohead_id)" );

  if (_type & (cPOUnposted | cPOOpen | cPOClosed))
  {
    bool qualifier = FALSE;

    sql += " AND (pohead_status IN (";

    if (_type & cPOUnposted)
    {
      qualifier = TRUE;
      sql += "'U'";
    }
    
    if (_type & cPOOpen)
    {
      if (qualifier)
        sql += ", ";
      else
        qualifier = TRUE;

      sql += "'O'";
    }

    if (_type & cPOClosed)
    {
      if (qualifier)
        sql += ", ";
      else
        qualifier = TRUE;

      sql += "'C'";
    }

    sql += "))";
  }

  sql += " );";
  XSqlQuery pohead;
  pohead.prepare(sql);
  pohead.bindValue(":pohead_id", pId);
  pohead.exec();
  if (pohead.first())
  {
    _id     = pId;
    _number = pohead.value("pohead_number").toInt();
    _vendid = pohead.value("pohead_vend_id").toInt();
    _valid  = TRUE;

    emit numberChanged(pohead.value("pohead_number").toString());
    emit vendNameChanged(pohead.value("vend_name").toString());
    emit vendAddress1Changed(pohead.value("vend_address1").toString());
    emit vendAddress2Changed(pohead.value("vend_address2").toString());
    emit vendAddress3Changed(pohead.value("vend_address3").toString());
    emit vendCityStateZipChanged(pohead.value("citystatezip").toString());
    setText(pohead.value("pohead_number").toString());
  }
  else
  {
    _id     = -1;
    _number = -1;
    _vendid = -1;
    _valid  = FALSE;

    emit numberChanged("");
    emit vendNameChanged("");
    emit vendAddress1Changed("");
    emit vendAddress2Changed("");
    emit vendAddress3Changed("");
    emit vendCityStateZipChanged("");
    setText("");
  }

  emit newId(_id);
  emit vendidChanged(_vendid);
  emit numberChanged(_number);
  emit valid(_valid);

  if (_mapper->model() &&
      _mapper->model()->data(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this))).toString() != text())
    _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this)), text());
              
  _parsed = TRUE;
}

void PoLineEdit::setNumber(int pNumber)
{
  XSqlQuery poheadid;
  poheadid.prepare( "SELECT pohead_id "
                    "FROM pohead "
                    "WHERE (pohead_number=:pohead_number);" );
  poheadid.bindValue(":pohead_number", QString("%1").arg(pNumber));
  poheadid.exec();
  if (poheadid.first())
    setId(poheadid.value("pohead_id").toInt());
  else
    setId(-1);
}

void PoLineEdit::clear()
{
  setId(-1);
}

void PoLineEdit::sParse()
{
  if (!_parsed)
  {
    _parsed = TRUE;

    bool numeric;
    int  purchaseOrderNumber = text().toInt(&numeric);

    if (numeric)
      setNumber(purchaseOrderNumber);
    else
      setId(-1);
  }
}


PoCluster::PoCluster(QWidget *parent, const char *name) :
  QWidget(parent, name)
{
//  Create the component Widgets

  QVBoxLayout *layoutMain = new QVBoxLayout(this);
  layoutMain->setMargin(0);
  layoutMain->setSpacing(0);
  QWidget *firstLine = new QWidget(this);
  QHBoxLayout *layoutFirstLine = new QHBoxLayout(firstLine);
  layoutFirstLine->setMargin(0);
  layoutFirstLine->setSpacing(5);

  QLabel *poNumberLit = new QLabel(tr("P/O #:"), firstLine);
  poNumberLit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  layoutFirstLine->addWidget(poNumberLit);

  _poNumber = new PoLineEdit(firstLine);
  layoutFirstLine->addWidget(_poNumber);

  _poList = new QPushButton(tr("..."), firstLine);
  _poList->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
#ifndef Q_WS_MAC
  _poList->setMaximumWidth(25);
#endif
  _poList->setFocusPolicy(Qt::NoFocus);
  layoutFirstLine->addWidget(_poList);

  QSpacerItem* spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);
  layoutFirstLine->addItem( spacer );
  firstLine->setLayout(layoutFirstLine);
  layoutMain->addWidget(firstLine);

  _vendName = new QLabel(this);
  _vendName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _vendName->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
  layoutMain->addWidget(_vendName);

  setLayout(layoutMain);

  connect(_poList, SIGNAL(clicked()), this, SLOT(sList()));
  connect(_poNumber, SIGNAL(vendNameChanged(const QString &)), _vendName, SLOT(setText(const QString &)));
  connect(_poNumber, SIGNAL(newId(int)), this, SIGNAL(newId(int)));
  connect(_poNumber, SIGNAL(vendidChanged(int)), this, SIGNAL(newVendid(int)));
  connect(_poNumber, SIGNAL(valid(bool)), this, SIGNAL(valid(bool)));

  setFocusProxy(_poNumber);
}

void PoCluster::sList()
{
  ParameterList params;
  params.append("pohead_id", _poNumber->id());
  params.append("poType", _poNumber->type());

  purchaseOrderList newdlg(parentWidget(), "", TRUE);
  newdlg.set(params);

  int id;
  if ((id  = newdlg.exec()) != QDialog::Rejected)
    _poNumber->setId(id);
}

void PoCluster::setDataWidgetMap(XDataWidgetMapper* m)
{
  m->addMapping(this, _fieldName, "number", "defaultNumber");
  _poNumber->_mapper=m;
}

void PoCluster::setNumber(const QString& number)
{
  qDebug("number %s", qPrintable(number));
  qDebug("curr %s", qPrintable(_poNumber->text()));
  if (_poNumber->text() == number)
    return
    
  qDebug("setting po");
  _poNumber->setText(number);
  _poNumber->sParse();
}

void PoCluster::setReadOnly(bool pReadOnly)
{
  if (pReadOnly)
  {
    _poNumber->setEnabled(FALSE);
    _poList->hide();
  }
  else
  {
    _poNumber->setEnabled(TRUE);
    _poList->show();
  }
}

