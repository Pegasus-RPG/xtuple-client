/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <Q3DragObject>
#include <QHBoxLayout>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QSqlError>
#include <QVBoxLayout>
#include <QValidator>

#include <metasql.h>
#include <parameter.h>
#include <xsqlquery.h>

#include "crmacctcluster.h"

#include "custcluster.h"

//  Routines for CLineEdit - a customer and prospect validating QLineEdit
CLineEdit::CLineEdit(QWidget *pParent, const char *name) :
  XLineEdit(pParent, name)
{
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  setMaximumWidth(100);

  setAcceptDrops(TRUE);

  _id       = -1;
  _valid    = FALSE;
  _parsed   = TRUE;
  _dragging = FALSE;
  _type     = AllCustomers;
  _autoFocus = true;

  connect(this, SIGNAL(lostFocus()), this, SLOT(sParse()));
  connect(this, SIGNAL(requestSearch()), this, SLOT(sSearch()));
  connect(this, SIGNAL(requestList()), this, SLOT(sList()));
}

void CLineEdit::setAutoFocus(bool yes)
{
  _autoFocus = yes;
}

void CLineEdit::sEllipses()
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

void CLineEdit::sSearch()
{
  CRMAcctSearch * newdlg = new CRMAcctSearch(this);
  newdlg->setId(_id);

  int id = newdlg->exec();
  if (id != QDialog::Rejected)
  {
    setId(id);

    if (_autoFocus && id != -1)
      focusNextPrevChild(TRUE);
  }
}

void CLineEdit::sList()
{
  CRMAcctList * newdlg = new CRMAcctList(this);
  newdlg->setId(_id);

  int id = newdlg->exec();
  setId(id);

  if (_autoFocus && id != -1)
  {
    this->setFocus();
    focusNextPrevChild(TRUE);
  }
}

void CLineEdit::setSilentId(int pId)
{
//  Make sure there's a change
  if (pId == _id)
    return;

  if (pId != -1)
  {
    QString sql( "<? if exists(\"customer\") ?>"
                 "  SELECT cust_number,"
                 "         cust_name,"
                 "         addr_line1, addr_line2, addr_line3,"
                 "         addr_city,  addr_state, addr_postalcode,"
                 "         addr_country, cust_creditstatus, "
                 "         cntct_addr_id, cust_cntct_id "
                 "  FROM custinfo LEFT OUTER JOIN "
                 "       cntct ON (cust_cntct_id=cntct_id) LEFT OUTER JOIN "
                 "       addr  ON (cntct_addr_id=addr_id) "
                 "  WHERE ( (cust_id=<? value(\"id\") ?>) "
                 "  <? if exists(\"active\") ?>"
                 "   AND (cust_active)"
                 "  <? endif ?>"
                 "  <? if exists(\"customer_extraclause\") ?>"
                 "  AND (<? literal(\"customer_extraclause\") ?>)"
                 "  <? endif ?>"
                 "  <? if exists(\"all_extraclause\") ?>"
                 "  AND (<? literal(\"all_extraclause\") ?>)"
                 "  <? endif ?>"
                 "  )"
                 "<? endif ?>"
                 "<? if exists(\"prospect\") ?>"
                 "  <? if exists(\"customer\") ?>"
                 "    UNION"
                 "  <? endif ?>"
                 "  SELECT prospect_number AS cust_number,"
                 "         prospect_name AS cust_name,"
                 "         addr_line1, addr_line2, addr_line3,"
                 "         addr_city,  addr_state, addr_postalcode,"
                 "         addr_country, '' AS cust_creditstatus, "
                 "         cntct_addr_id, prospect_cntct_id AS cust_cntct_id "
                 "  FROM prospect LEFT OUTER JOIN "
                 "       cntct ON (prospect_cntct_id=cntct_id) LEFT OUTER JOIN "
                 "       addr  ON (cntct_addr_id=addr_id) "
                 "  WHERE ( (prospect_id=<? value(\"id\") ?>) "
                 "  <? if exists(\"active\") ?>"
                 "   AND (prospect_active)"
                 "  <? endif ?>"
                 "  <? if exists(\"prospect_extraclause\") ?>"
                 "  AND (<? literal(\"prospect_extraclause\") ?>)"
                 "  <? endif ?>"
                 "  <? if exists(\"all_extraclause\") ?>"
                 "  AND (<? literal(\"all_extraclause\") ?>)"
                 "  <? endif ?>"
                 "  )"
                 "<? endif ?>"
                 ";");

    ParameterList params;
    params.append("id", pId);
    switch (_type)
    {
      case ActiveCustomers:
        params.append("active");
        // fall-through
      case AllCustomers:
        params.append("customer");
        break;

      case ActiveProspects:
        params.append("active");
        // fall-through
      case AllProspects:
        params.append("prospect");
        break;

      case ActiveCustomersAndProspects:
        params.append("active");
        // fall-through
      case AllCustomersAndProspects:
        params.append("customer");
        params.append("prospect");
        break;
    }
    if (! _all_extraclause.isEmpty())
      params.append("all_extraclause", _all_extraclause);
    if (! _customer_extraclause.isEmpty())
      params.append("customer_extraclause", _customer_extraclause);
    if (! _prospect_extraclause.isEmpty())
      params.append("prospect_extraclause", _prospect_extraclause);

    MetaSQLQuery mql(sql);
    XSqlQuery cust = mql.toQuery(params);

    if (cust.first())
    {
      _id = pId;

      setText(cust.value("cust_number").toString());

      emit custNumberChanged(cust.value("cust_number").toString());
      emit custNameChanged(cust.value("cust_name").toString());
      emit custAddr1Changed(cust.value("addr_line1").toString());
      emit custAddr2Changed(cust.value("addr_line2").toString());
      emit custAddr3Changed(cust.value("addr_line3").toString());
      emit custCityChanged(cust.value("addr_city").toString());
      emit custStateChanged(cust.value("addr_state").toString());
      emit custZipCodeChanged(cust.value("addr_postalcode").toString());
      emit custCountryChanged(cust.value("addr_country").toString());
      emit creditStatusChanged(cust.value("cust_creditstatus").toString());
      emit custAddressChanged(cust.value("cntct_addr_id").toInt());
      emit custContactChanged(cust.value("cust_cntct_id").toInt());

      _valid = TRUE;

      return;
    }
    else if (cust.lastError().type() != QSqlError::NoError)
      QMessageBox::critical(this, tr("A System Error occurred at %1::%2.")
                            .arg(__FILE__)
                            .arg(__LINE__),
                            cust.lastError().databaseText());
//qDebug("the query: %s", cust.lastQuery().toAscii().data());
  }

//  A valid cust could not be found, clear the cust information
  _id = -1;
  setText("");

  emit custNumberChanged("");
  emit custNameChanged("");
  emit custAddr1Changed("");
  emit custAddr2Changed("");
  emit custAddr3Changed("");
  emit custCityChanged("");
  emit custStateChanged("");
  emit custZipCodeChanged("");
  emit custCountryChanged("");
  emit creditStatusChanged("");
  emit custAddressChanged(-1);
  emit custContactChanged(-1);

  _valid = FALSE;
}


void CLineEdit::setId(int pId)
{
  if (_id == pId)
    return;
    
  setSilentId(pId);

//  Emit the item information signals
  emit newId(_id);
  emit valid(_valid);
}


void CLineEdit::setNumber(const QString& pNumber)
{
    _parsed = false;
    setText(pNumber);
    sParse();
}


void CLineEdit::setType(CLineEditTypes pType)
{
  _type = pType;
}

void CLineEdit::setExtraClause(CLineEditTypes type, const QString &clause)
{
  switch (type)
  {
    case AllCustomers:
    case ActiveCustomers:
      _all_extraclause = QString::null;
      _customer_extraclause = QString(clause);
      _prospect_extraclause = QString::null;
      break;

    case AllProspects:
    case ActiveProspects:
      _all_extraclause = QString::null;
      _customer_extraclause = QString::null;
      _prospect_extraclause = QString(clause);
      break;

    case AllCustomersAndProspects:
    case ActiveCustomersAndProspects:
      _all_extraclause = QString(clause);
      _customer_extraclause = QString::null;
      _prospect_extraclause = QString::null;
      break;

    default:
      _all_extraclause = QString::null;
      _customer_extraclause = QString::null;
      _prospect_extraclause = QString::null;
      qWarning("%s::setExtraClause(%d, %s) called with bad type",
               qPrintable(objectName()), type, qPrintable(clause));
  }
}

void CLineEdit::sParse()
{
  if (!_parsed)
  {
    _parsed = TRUE;

    QString stripped = text().trimmed().toUpper();
    if (stripped.length() == 0)
      setId(-1);
    QString sql("<? if exists(\"customer\") ?>"
                "  SELECT cust_number, cust_id"
                "  FROM custinfo "
                "  WHERE ((UPPER(cust_number)=UPPER(<? value(\"number\") ?>))"
                "  <? if exists(\"active\") ?>"
                "   AND (cust_active)"
                "  <? endif ?>"
                "  <? if exists(\"customer_extraclause\") ?>"
                "  AND (<? literal(\"customer_extraclause\") ?>)"
                "  <? endif ?>"
                "  <? if exists(\"all_extraclause\") ?>"
                "  AND (<? literal(\"all_extraclause\") ?>)"
                "  <? endif ?>"
                "  )"
                "<? endif ?>"
                "<? if exists(\"prospect\") ?>"
                "  <? if exists(\"customer\") ?>"
                "    UNION"
                "  <? endif ?>"
                "  SELECT prospect_number AS cust_number,"
                "         prospect_id AS cust_id "
                "  FROM prospect "
                "  WHERE ((UPPER(prospect_number)=UPPER(<? value(\"number\") ?>))"
                "  <? if exists(\"active\") ?>"
                "   AND (prospect_active)"
                "  <? endif ?>"
                "  <? if exists(\"prospect_extraclause\") ?>"
                "  AND (<? literal(\"prospect_extraclause\") ?>)"
                "  <? endif ?>"
                "  <? if exists(\"all_extraclause\") ?>"
                "  AND (<? literal(\"all_extraclause\") ?>)"
                "  <? endif ?>"
                "  )"
                "<? endif ?>"
                ";");

    ParameterList params;
    params.append("number", stripped);
    switch (_type)
    {
      case ActiveCustomers:
        params.append("active");
        // fall-through
      case AllCustomers:
        params.append("customer");
        break;

      case ActiveProspects:
        params.append("active");
        // fall-through
      case AllProspects:
        params.append("prospect");
        break;

      case ActiveCustomersAndProspects:
        params.append("active");
        // fall-through
      case AllCustomersAndProspects:
        params.append("customer");
        params.append("prospect");
        break;
    }

    MetaSQLQuery mql(sql);
    XSqlQuery cust = mql.toQuery(params);
    if (cust.first())
    {
      setText(cust.value("cust_number").toString());
      setId(cust.value("cust_id").toInt());
    }
    else
    {
      if (cust.lastError().type() != QSqlError::NoError)
        QMessageBox::critical(this, tr("A System Error occurred at %1::%2.")
                              .arg(__FILE__)
                              .arg(__LINE__),
                              cust.lastError().databaseText());
      setText("");
      setId(-1);
    }
//qDebug("the query: %s", cust.lastQuery().toAscii().data());
  }
}

void CLineEdit::mousePressEvent(QMouseEvent *pEvent)
{
  QLineEdit::mousePressEvent(pEvent);

  if (_valid)
    _dragging = TRUE;
}

void CLineEdit::mouseMoveEvent(QMouseEvent *)
{
  if (_dragging)
  {
    Q3DragObject *drag = new Q3TextDrag(QString("custid=%1").arg(_id), this);
    drag->dragCopy();

    _dragging = FALSE;
  }
}

void CLineEdit::dragEnterEvent(QDragEnterEvent *pEvent)
{
  QString dragData;

  if (Q3TextDrag::decode(pEvent, dragData))
  {
    if (dragData.contains("custid="))
      pEvent->accept(TRUE);
  }

  pEvent->accept(FALSE);
}

void CLineEdit::dropEvent(QDropEvent *pEvent)
{
  QString dropData;

  if (Q3TextDrag::decode(pEvent, dropData))
  {
    if (dropData.contains("custid="))
    {
      QString target = dropData.mid((dropData.find("custid=") + 7), (dropData.length() - 7));

      if (target.contains(","))
        target = target.left(target.find(","));
      setId(target.toInt());
    }
  }
}

void CLineEdit::keyPressEvent(QKeyEvent * pEvent)
{
  if(pEvent->key() == Qt::Key_Tab)
    sParse();
  XLineEdit::keyPressEvent(pEvent);
}

/////////////////////////////////////////////////////////

CustInfoAction* CustInfo::_custInfoAction = 0;
CustInfo::CustInfo(QWidget *pParent, const char *name) :
  QWidget(pParent, name)
{
//  Create and place the component Widgets
  QHBoxLayout *layoutMain = new QHBoxLayout(this, 0, 6, "layoutMain"); 
  QHBoxLayout *layoutNumber = new QHBoxLayout(0, 0, 6, "layoutNumber"); 
  QHBoxLayout *layoutButtons = new QHBoxLayout(0, 0, -1, "layoutButtons"); 

  _customerNumberLit = new QLabel(tr("Customer #:"), this, "_customerNumberLit");
  _customerNumberLit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  layoutNumber->addWidget(_customerNumberLit);

  _customerNumber = new CLineEdit(this, "_customerNumber");
  _customerNumber->setMinimumWidth(100);
  layoutNumber->addWidget(_customerNumber);
  
  _customerNumberEdit = new XLineEdit(this, "_customerNumberEdit");
  _customerNumberEdit->setMinimumWidth(100);
  _customerNumberEdit->setMaximumWidth(100);
  layoutNumber->addWidget(_customerNumberEdit);
  _customerNumberEdit->hide();
  layoutMain->addLayout(layoutNumber);

  _list = new QPushButton(tr("..."), this, "_list");
  _list->setFocusPolicy(Qt::NoFocus);
  if(_x_preferences)
  {
    if(_x_preferences->value("DefaultEllipsesAction") == "search")
      _list->setToolTip(tr("Search"));
    else
      _list->setToolTip(tr("List"));
  }
  layoutButtons->addWidget(_list);

  _info = new QPushButton(tr("?"), this, "_info");
  _info->setFocusPolicy(Qt::NoFocus);
  _info->setToolTip(tr("Open"));
  layoutButtons->addWidget(_info);
  
  _delete = new QPushButton(tr("x"), this, "_delete");
  _delete->setFocusPolicy(Qt::NoFocus);
  _delete->setToolTip(tr("Delete"));
  layoutButtons->addWidget(_delete);
  
  _new = new QPushButton(tr("+"), this, "_new");
  _new->setFocusPolicy(Qt::NoFocus);
  _new->setToolTip(tr("New"));
  layoutButtons->addWidget(_new);

  _edit = new QPushButton(tr("!"), this, "_edit");
  _edit->setFocusPolicy(Qt::NoFocus);
  _edit->setCheckable(true);
  _edit->setToolTip(tr("Edit Mode"));
  layoutButtons->addWidget(_edit);
  
#ifndef Q_WS_MAC
  _list->setMaximumWidth(25);
  _info->setMaximumWidth(25);
  _new->setMaximumWidth(25);
  _edit->setMaximumWidth(25);
  _delete->setMaximumWidth(25);
#else
  _list->setMinimumWidth(60);
  _list->setMinimumHeight(32);
  _info->setMinimumWidth(60);
  _info->setMinimumHeight(32);
  _new->setMinimumWidth(60);
  _new->setMinimumHeight(32);
  _edit->setMinimumWidth(60);
  _edit->setMinimumHeight(32);
  _delete->setMinimumWidth(60);
  _delete->setMinimumHeight(32);
  layoutNumber->setContentsMargins(2,0,0,0);
  layoutButtons->setContentsMargins(0,8,0,0);
  layoutButtons->setSpacing(6);
#endif
  
  layoutMain->addLayout(layoutButtons);

//  Make some internal connections
  connect(_list, SIGNAL(clicked()), _customerNumber, SLOT(sEllipses()));
  connect(_info, SIGNAL(clicked()), this, SLOT(sInfo()));

  connect(_customerNumber, SIGNAL(newId(int)), this, SIGNAL(newId(int)));
  connect(_customerNumber, SIGNAL(custNameChanged(const QString &)), this, SIGNAL(nameChanged(const QString &)));
  connect(_customerNumber, SIGNAL(custAddr1Changed(const QString &)), this, SIGNAL(address1Changed(const QString &)));
  connect(_customerNumber, SIGNAL(custAddr2Changed(const QString &)), this, SIGNAL(address2Changed(const QString &)));
  connect(_customerNumber, SIGNAL(custAddr3Changed(const QString &)), this, SIGNAL(address3Changed(const QString &)));
  connect(_customerNumber, SIGNAL(custCityChanged(const QString &)), this, SIGNAL(cityChanged(const QString &)));
  connect(_customerNumber, SIGNAL(custStateChanged(const QString &)), this, SIGNAL(stateChanged(const QString &)));
  connect(_customerNumber, SIGNAL(custZipCodeChanged(const QString &)), this, SIGNAL(zipCodeChanged(const QString &)));
  connect(_customerNumber, SIGNAL(custCountryChanged(const QString &)), this, SIGNAL(countryChanged(const QString &)));
  connect(_customerNumber, SIGNAL(custAddressChanged(const int)), this, SIGNAL(addressChanged(const int)));
  connect(_customerNumber, SIGNAL(custContactChanged(const int)), this, SIGNAL(contactChanged(const int)));
  connect(_customerNumber, SIGNAL(valid(bool)), this, SIGNAL(valid(bool)));

  connect(_customerNumber, SIGNAL(creditStatusChanged(const QString &)), this, SLOT(sHandleCreditStatus(const QString &)));
  connect(_customerNumber, SIGNAL(textChanged(const QString &)), _customerNumberEdit, SLOT(setText(const QString &)));
  connect(_customerNumberEdit, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(setMode()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNewClicked()));
  connect(_delete, SIGNAL(clicked()), this, SIGNAL(deleteClicked()));
  connect(_customerNumber, SIGNAL(newId(int)), this, SLOT(setMode()));

  if(_x_privileges && (!_x_privileges->check("MaintainCustomerMasters") && !_x_privileges->check("ViewCustomerMasters")))
    _info->setEnabled(false);
  else
  {
    connect(_customerNumber, SIGNAL(valid(bool)), _info, SLOT(setEnabled(bool)));
    connect(_customerNumber, SIGNAL(requestInfo()), this, SLOT(sInfo()));
  }

  setFocusProxy(_customerNumber);

  _mapper = new XDataWidgetMapper(this);
  _labelVisible = true;
  _canEdit=true;
  setCanEdit(false);
}
  
void CustInfo::setAutoFocus(bool yes)
{
  _customerNumber->setAutoFocus(yes);
}

void CustInfo::setDataWidgetMap(XDataWidgetMapper* m)
{
  disconnect(_customerNumber, SIGNAL(newId(int)), this, SLOT(updateMapperData()));
  m->addMapping(this, _fieldName, "number", "defaultNumber");
  _mapper=m;
  connect(_customerNumber, SIGNAL(newId(int)), this, SLOT(updateMapperData()));
}

void CustInfo::setReadOnly(bool pReadOnly)
{
  if (pReadOnly)
  {
    _customerNumber->setEnabled(FALSE);
    _list->hide();
  }
  else
  {
    _customerNumber->setEnabled(TRUE);
    _list->show();
  }
}

void CustInfo::clear()
{
  setId(-1);
  _customerNumberEdit->setText("");
}

void CustInfo::setId(int pId)
{
  _customerNumber->setId(pId);
}

void CustInfo::setSilentId(int pId)
{
  _customerNumber->setSilentId(pId);
}

void CustInfo::setType(CLineEdit::CLineEditTypes pType)
{
  _customerNumber->setType(pType);
}

void CustInfo::setLabelVisible(bool p)
{
  _customerNumberLit->setVisible(p);
  _labelVisible=p;
}

void CustInfo::setCanEdit(bool p)
{
  if (_canEdit == p)
    return;
    
  if (!p)
    _edit->setChecked(false);

  _canEdit=p;
    
  _info->setHidden(p);
  _edit->setVisible(p);
  _new->setVisible(p);
  if (!p)
  {
    _delete->setVisible(false);
    setEditMode(false);
  }
}

void CustInfo::setEditMode(bool p)
{
  _edit->setChecked(p);
  setMode();
}

void CustInfo::setMode()
{
  _delete->setDisabled(id() == -1);
    
  if (_editMode == _edit->isChecked())
    return;
 
  _editMode = _edit->isChecked();

  if (_editMode)
  {
   setCanEdit(true);
   _list->hide();
   _delete->show();
   _customerNumber->hide();
   _customerNumberEdit->show();
  }
  else
  {
   _customerNumber->setText(_customerNumberEdit->text());
   _delete->hide();
   _list->show();
   _customerNumberEdit->hide();
   _customerNumber->show();
   setId(-1);
  }

  emit editable(_editMode);
}

QString CustInfo::number()
{
  if (_editMode)
    return _customerNumberEdit->text();
  else
    return _customerNumber->text();
}

void CustInfo::setNumber(QString number)
{
  if (_editMode)
    _customerNumberEdit->setText(number);
  else
    _customerNumber->setNumber(number);
}

void CustInfo::sNewClicked()
{
  setEditMode(true);
  setId(-1);
}

void CustInfo::sInfo()
{
  if(_custInfoAction)
    _custInfoAction->customerInformation(parentWidget(), _customerNumber->_id);
}

void CustInfo::setExtraClause(CLineEdit::CLineEditTypes type, const QString &clause)
{
  _customerNumber->setExtraClause(type, clause);
}

void CustInfo::sHandleCreditStatus(const QString &pStatus)
{
  if ((pStatus == "G") || (pStatus == ""))
  {
    _info->setText(tr("?"));
    _info->setPaletteForegroundColor(QColor("black"));
  }
  else
  {
    _info->setText(tr("$"));

    if (pStatus == "W")
      _info->setPaletteForegroundColor(QColor("orange"));
    else if (pStatus == "H")
      _info->setPaletteForegroundColor(QColor("red"));
  }
}

void CustInfo::updateMapperData()
{
  if (_mapper->model() &&
      _mapper->model()->data(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this))).toString() != number())
    _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this)), number());
}

//////////////////////////////////////////////////////////////

CustCluster::CustCluster(QWidget *parent, const char *name) :
  QWidget(parent, name)
{
//  Create the component Widgets
  QVBoxLayout *_main = new QVBoxLayout(this);
  _main->setMargin(0);
  _main->setSpacing(0);

  _custInfo = new CustInfo(this, "_custInfo");
  _custInfo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _main->addWidget(_custInfo);

  _name = new QLabel(this, "_name");
  _name->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _name->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _main->addWidget(_name);

  _address1 = new QLabel(this, "_address1");
  _address1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _address1->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _main->addWidget(_address1);

  setLayout(_main);

//  Make some internal connections
  connect(_custInfo, SIGNAL(newId(int)), this, SIGNAL(newId(int)));
  connect(_custInfo, SIGNAL(valid(bool)), this, SIGNAL(valid(bool)));
  connect(_custInfo, SIGNAL(nameChanged(const QString &)), _name, SLOT(setText(const QString &)));
  connect(_custInfo, SIGNAL(address1Changed(const QString &)), _address1, SLOT(setText(const QString &)));

  setFocusProxy(_custInfo);
}

void CustCluster::setAutoFocus(bool yes)
{
  _custInfo->setAutoFocus(yes);
}

void CustCluster::setReadOnly(bool pReadOnly)
{
  _custInfo->setReadOnly(pReadOnly);
}

void CustCluster::setId(int pId)
{
  _custInfo->setId(pId);
}

void CustCluster::setSilentId(int pId)
{
  _custInfo->setSilentId(pId);
}

void CustCluster::setType(CLineEdit::CLineEditTypes pType)
{
  _custInfo->setType(pType);
}

void CustCluster::setExtraClause(CLineEdit::CLineEditTypes type, const QString &clause)
{
  _custInfo->setExtraClause(type, clause);
}
