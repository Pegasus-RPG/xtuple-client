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
#include <QValidator>
#include <QLabel>
#include <QLayout>
#include <Q3DragObject>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDropEvent>
#include <QMouseEvent>
#include <QDragEnterEvent>

#include <xsqlquery.h>
#include <parameter.h>

#include "crmacctcluster.h"
#include "vendorcluster.h"

VendorLineEdit::VendorLineEdit(QWidget *pParent, const char *name) :
  XLineEdit(pParent, name)
{
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  setMaximumWidth(100);

  setAcceptDrops(TRUE);
  setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

  _id       = -1;
  _valid    = FALSE;
  _parsed   = TRUE;
  _dragging = FALSE;
  _type     = __allVendors;

  connect(this, SIGNAL(lostFocus()), this, SLOT(sParse()));
  connect(this, SIGNAL(requestSearch()), this, SLOT(sSearch()));
  connect(this, SIGNAL(requestList()), this, SLOT(sList()));
  
  _mapper = new XDataWidgetMapper(this);
}

void VendorLineEdit::setId(int pId)
{
  if (pId != -1)
  {
    QString sql( "SELECT vend_number, vend_name,"
                 "       vend_address1, vend_address2, vend_address3,"
                 "       vend_city, vend_state, vend_zip, vend_country, "
                 "       crmacct_id "
                 "FROM vend "
                 " JOIN crmacct ON (crmacct_vend_id=vend_id) "
                 "WHERE ( (vend_id=:vend_id)" );

    if (_type == __activeVendors)
      sql += " AND (vend_active) ";

    sql += ");";

    XSqlQuery vend;
    vend.prepare(sql);
    vend.bindValue(":vend_id", pId);
    vend.exec();
    if (vend.first())
    {
      _id     = pId;
      _valid  = TRUE;
  
      setText(vend.value("vend_number").toString());

      emit nameChanged(vend.value("vend_name").toString());
      emit address1Changed(vend.value("vend_address1").toString());
      emit address2Changed(vend.value("vend_address2").toString());
      emit address3Changed(vend.value("vend_address3").toString());
      emit cityChanged(vend.value("vend_city").toString());
      emit stateChanged(vend.value("vend_state").toString());
      emit zipCodeChanged(vend.value("vend_zip").toString());
      emit countryChanged(vend.value("vend_country").toString());
      emit newId(_id);
      emit newCrmacctId(vend.value("crmacct_id").toInt());
      emit valid(TRUE);
    }
  }
  else
  {
    _id     = -1;
    _valid  = FALSE;

    setText("");

    emit nameChanged("");
    emit address1Changed("");
    emit address2Changed("");
    emit address3Changed("");
    emit cityChanged("");
    emit stateChanged("");
    emit zipCodeChanged("");
    emit countryChanged("");
    emit newId(-1);
    emit newCrmacctId(-1);
    emit valid(FALSE);
  }
  
  if (_mapper->model() &&
    _mapper->model()->data(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this))).toString() != text())
  _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this)), text());

  _parsed = TRUE;
}

void VendorLineEdit::setNumber(const QString &pNumber)
{
  if (pNumber.isEmpty())
  {
    setId(-1);
    return;
  }
  
  XSqlQuery vend( QString( "SELECT vend_id "
                           "FROM vend "
                           "WHERE (vend_number='%1');" )
                  .arg(pNumber) );
  if (vend.first())
    setId(vend.value("vend_id").toInt());
  else
    setId(-1);
}

void VendorLineEdit::setType(int pType)
{
  _type = pType;
}

void VendorLineEdit::sParse()
{
  if (!_parsed)
    setNumber(text().trimmed().toUpper());
}

void VendorLineEdit::sEllipses()
{
  if(_x_preferences)
  {
    if(_x_preferences->value("DefaultEllipsesAction") == "search")
    {
      sSearch();
      return;
    }
  }

  sList();
}

void VendorLineEdit::sSearch()
{
  CRMAcctSearch * newdlg = new CRMAcctSearch(this);
  newdlg->setId(_id);
  newdlg->setShowInactive(_type != __activeVendors);

  int id = newdlg->exec();

  if (id != QDialog::Rejected)
  {
    setId(id);

    if (id != -1)
      focusNextPrevChild(TRUE);
  }
}

void VendorLineEdit::sList()
{
  CRMAcctList * newdlg = new CRMAcctList(this);
  newdlg->setId(_id);
  newdlg->setShowInactive(_type != __activeVendors);

  int newId = newdlg->exec();
  setId(newId);

  if (newId != -1)
  {
    setFocus();
    focusNextPrevChild(TRUE);
  }
}

void VendorLineEdit::mousePressEvent(QMouseEvent *pEvent)
{
  QLineEdit::mousePressEvent(pEvent);

  if (_valid)
    _dragging = TRUE;
}

void VendorLineEdit::mouseMoveEvent(QMouseEvent *)
{
  if (_dragging)
  {
    Q3DragObject *drag = new Q3TextDrag(QString("vendid=%1").arg(_id), this);
    drag->dragCopy();

    _dragging = FALSE;
  }
}

void VendorLineEdit::dragEnterEvent(QDragEnterEvent *pEvent)
{
  QString dragData;

  if (Q3TextDrag::decode(pEvent, dragData))
  {
    if (dragData.contains("vendid="))
      pEvent->accept(TRUE);
  }

  pEvent->accept(FALSE);
}

void VendorLineEdit::dropEvent(QDropEvent *pEvent)
{
  QString dropData;

  if (Q3TextDrag::decode(pEvent, dropData))
  {
    if (dropData.contains("vendid="))
    {
      QString target = dropData.mid((dropData.find("vendid=") + 7), (dropData.length() - 7));

      if (target.contains(","))
        target = target.left(target.find(","));

      setId(target.toInt());
    }
  }
}


VendorInfo::VendorInfo(QWidget *parent, const char *name) :
  QWidget(parent, name)
{
//  Create and place the component Widgets
  QHBoxLayout *layoutMain   = new QHBoxLayout(this, 0, 2, "layoutMain"); 
  QHBoxLayout *layoutNumber = new QHBoxLayout(0, 0, 6, "layoutNumber"); 
  QHBoxLayout *layoutButtons = new QHBoxLayout(0, 0, 6, "layoutButtons"); 

  QLabel *_vendorNumberLit = new QLabel(tr("Vendor #:"), this, "_vendorNumberLit");
  _vendorNumberLit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  layoutNumber->addWidget(_vendorNumberLit);

  _vendorNumber = new VendorLineEdit(this, "_vendorNumber");
  _vendorNumber->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
  _vendorNumber->setMinimumWidth(100);
  layoutNumber->addWidget(_vendorNumber);
  layoutMain->addLayout(layoutNumber);

  _list = new QPushButton(tr("..."), this, "_list");
#ifndef Q_WS_MAC
  _list->setMaximumWidth(25);
#else
  _list->setMinimumWidth(60);
  _list->setMinimumHeight(32);
#endif
  _list->setFocusPolicy(Qt::NoFocus);
  layoutButtons->addWidget(_list);

/* Can implement this later  
  _info = new QPushButton(tr("?"), this, "_info");
  _info->setFocusPolicy(Qt::NoFocus);
  layoutButtons->addWidget(_info);
*/

  layoutMain->addLayout(layoutButtons);

 // QSpacerItem* spacer = new QSpacerItem(20, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);
 // layoutMain->addItem(spacer);

//  Make some internal connections
  connect(_list, SIGNAL(clicked()), _vendorNumber, SLOT(sEllipses()));

  connect(_vendorNumber, SIGNAL(newId(int)), this, SIGNAL(newId(int)));
  connect(_vendorNumber, SIGNAL(newCrmacctId(int)), this, SIGNAL(newCrmacctId(int)));
  connect(_vendorNumber, SIGNAL(nameChanged(const QString &)), this, SIGNAL(nameChanged(const QString &)));
  connect(_vendorNumber, SIGNAL(address1Changed(const QString &)), this, SIGNAL(address1Changed(const QString &)));
  connect(_vendorNumber, SIGNAL(address2Changed(const QString &)), this, SIGNAL(address2Changed(const QString &)));
  connect(_vendorNumber, SIGNAL(address3Changed(const QString &)), this, SIGNAL(address3Changed(const QString &)));
  connect(_vendorNumber, SIGNAL(cityChanged(const QString &)), this, SIGNAL(cityChanged(const QString &)));
  connect(_vendorNumber, SIGNAL(stateChanged(const QString &)), this, SIGNAL(stateChanged(const QString &)));
  connect(_vendorNumber, SIGNAL(zipCodeChanged(const QString &)), this, SIGNAL(zipCodeChanged(const QString &)));
  connect(_vendorNumber, SIGNAL(countryChanged(const QString &)), this, SIGNAL(countryChanged(const QString &)));
  connect(_vendorNumber, SIGNAL(valid(bool)), this, SIGNAL(valid(bool)));

  setFocusProxy(_vendorNumber);
}

void VendorInfo::setReadOnly(bool pReadOnly)
{
  if (pReadOnly)
  {
    _vendorNumber->setEnabled(FALSE);
    _list->hide();
  }
  else
  {
    _vendorNumber->setEnabled(TRUE);
    _list->show();
  }
}

void VendorInfo::setId(int pId)
{
  _vendorNumber->setId(pId);
}

void VendorInfo::setNumber(const QString& number)
{
  if (_vendorNumber->text() == number)
    return
    
  _vendorNumber->setText(number);
  _vendorNumber->sParse();
}

void VendorInfo::setType(int pType)
{
  _vendorNumber->_type = pType;
}

void VendorInfo::setDataWidgetMap(XDataWidgetMapper* m)
{
  m->addMapping(this, _fieldName, "number", "defaultNumber");
  _vendorNumber->_mapper=m;
}


VendorCluster::VendorCluster(QWidget *pParent, const char *name) :
  QWidget(pParent, name)
{
//  Create the component Widgets
  QVBoxLayout *layoutMain      = new QVBoxLayout(this, 0, 0, "layoutMain");
  QHBoxLayout *layoutFirstLine = new QHBoxLayout(0, 0, 5, "layoutFirstLine");

  QLabel *_vendorNumberLit = new QLabel(tr("Vendor #:"), this);
  _vendorNumberLit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  layoutFirstLine->addWidget(_vendorNumberLit);

  _vendorNumber = new VendorLineEdit(this, "_vendorNumber");
  layoutFirstLine->addWidget(_vendorNumber);

  _list = new QPushButton(tr("..."), this);
  _list->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
#ifndef Q_WS_MAC
  _list->setMaximumWidth(25);
#else
  _list->setMinimumWidth(60);
  _list->setMinimumHeight(32);
#endif
  _list->setFocusPolicy(Qt::NoFocus);
  layoutFirstLine->addWidget(_list);

  QSpacerItem* spacer = new QSpacerItem(20, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);
  layoutFirstLine->addItem(spacer);

  layoutMain->addLayout(layoutFirstLine);

  _vendorName = new QLabel(this);
  _vendorName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _vendorName->setAlignment(int(Qt::AlignVCenter | Qt::AlignLeft));
  layoutMain->addWidget(_vendorName);

  connect(_vendorNumber, SIGNAL(nameChanged(const QString &)), _vendorName, SLOT(setText(const QString &)));
  connect(_vendorNumber, SIGNAL(newId(int)), this, SIGNAL(newId(int)));
  connect(_vendorNumber, SIGNAL(newCrmacctId(int)), this, SIGNAL(newCrmacctId(int)));
  connect(_vendorNumber, SIGNAL(valid(bool)), this, SIGNAL(valid(bool)));

  connect(_list, SIGNAL(clicked()), _vendorNumber, SLOT(sEllipses()));

  setFocusProxy(_vendorNumber);
}

void VendorCluster::setReadOnly(bool pReadOnly)
{
  if (pReadOnly)
  {
    _vendorNumber->setEnabled(FALSE);
    _list->hide();
  }
  else
  {
    _vendorNumber->setEnabled(TRUE);
    _list->show();
  }
}

void VendorCluster::setType(int pType)
{
  _vendorNumber->_type = pType;
}

void VendorCluster::setDataWidgetMap(XDataWidgetMapper* m)
{
  m->addMapping(this, _fieldName, QByteArray("number"));
  _vendorNumber->_mapper=m;
}

void VendorCluster::setNumber(const QString& number)
{
  _vendorNumber->setNumber(number);
}
