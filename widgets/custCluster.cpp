/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

//#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSqlError>
#include <QVBoxLayout>

#include <metasql.h>
#include <parameter.h>
#include <xsqlquery.h>

#include "crmacctcluster.h"

#include "custcluster.h"
#include "format.h"

#define DEBUG false

//  Routines for CLineEdit - a customer and prospect validating QLineEdit
CLineEdit::CLineEdit(QWidget *pParent, const char *pName) :
  VirtualClusterLineEdit(pParent, "cust", "id", "number", "name", "description", 0, pName, "active")
{
  _crmacctId = -1;
  _type     = AllCustomers;

  setTitles(tr("Customer"), tr("Customers"));
  setUiName("customer");
  setEditPriv("MaintainCustomerMasters");
  setViewPriv("ViewCustomers");
  //setNewPriv("MaintainCustomerMasters");

  _query = " SELECT * FROM ( "
           "  SELECT cust_id AS id, "
           "         cust_number AS number, "
           "         cust_name AS name,"
           "         addr_line1 AS description,"
           "         cust_active AS active, "
           "         crmacct_id, true AS iscustomer "
           "  FROM custinfo "
           "    LEFT OUTER JOIN cntct  ON (cust_cntct_id=cntct_id) "
           "    LEFT OUTER JOIN addr   ON (cntct_addr_id=addr_id) "
           "    LEFT OUTER JOIN crmacct ON (crmacct_cust_id=cust_id) "
           "  UNION "
           "  SELECT prospect_id AS id, "
           "         prospect_number AS number,"
           "         prospect_name AS name,"
           "         addr_line1 AS description,"
           "         prospect_active AS active, "
           "         crmacct_id, false AS iscustomer "
           "  FROM prospect "
           "    LEFT OUTER JOIN cntct  ON (prospect_cntct_id=cntct_id) "
           "    LEFT OUTER JOIN addr   ON (cntct_addr_id=addr_id) "
           "    LEFT OUTER JOIN crmacct ON (crmacct_prospect_id=prospect_id) "
           "  ) cust "
           "WHERE (true) ";
}

void CLineEdit::setId(int pId)
{

  VirtualClusterLineEdit::setId(pId);
  if (model())
  {
    _crmacctId = model()->data(model()->index(0,CRMACCT_ID)).toInt();

    emit newCrmacctId(_crmacctId);
  }
}

void CLineEdit::setType(CLineEditTypes pType)
{
  _type = pType;

  QStringList list;
  switch (_type)
  {
    case ActiveCustomers:
      list.append("active");
      // fall-through
    case AllCustomers:
      list.append("iscustomer");
      break;

    case ActiveProspects:
      list.append("active");
      // fall-through
    case AllProspects:
      list.append("NOT iscustomer");
      break;

    case ActiveCustomersAndProspects:
      list.append("active");
      // fall-through
      break;
  case AllCustomersAndProspects:
      break;
  }
  list.removeDuplicates();
  setExtraClause(list.join(" AND "));
}

void CLineEdit::sOpen()
{
  if (canOpen())
  {
    QString uiName = _uiName;
    ParameterList params;
    if (_x_privileges->check(_editPriv))
      params.append("mode", "edit");
    else
      params.append("mode", "view");
    if (model()->data(model()->index(0,ISCUSTOMER)).toBool())
      params.append("cust_id", id());
    else
    {
      uiName="prospect";
      params.append("prospect_id", id());
    }

    QWidget* w = 0;
    if (parentWidget()->window())
    {
      if (parentWidget()->window()->isModal())
        w = _guiClientInterface->openWindow(uiName, params, parentWidget()->window() , Qt::WindowModal, Qt::Dialog);
      else
        w = _guiClientInterface->openWindow(uiName, params, parentWidget()->window() , Qt::NonModal, Qt::Window);
    }

    if (w->inherits("QDialog"))
    {
      QDialog* newdlg = qobject_cast<QDialog*>(w);
      int id = newdlg->exec();
      if (id != QDialog::Rejected)
      {
        silentSetId(id);
        emit valid(_id != -1);
      }
    }
  }
}

VirtualList* CLineEdit::listFactory()
{
  CRMAcctList* list = new CRMAcctList(this);
  enum CRMAcctLineEdit::CRMAcctSubtype subtype = CRMAcctLineEdit::Cust;
  switch (_type)
  {
    list->setShowInactive(true);
    case ActiveCustomers:
      list->setShowInactive(false);
      // fall-through
    case AllCustomers:
      break;

    case ActiveProspects:
      list->setShowInactive(false);
      // fall-through
    case AllProspects:
      subtype = CRMAcctLineEdit::Prospect;
      break;

    case ActiveCustomersAndProspects:
      list->setShowInactive(false);
      // fall-through
    case AllCustomersAndProspects:
      subtype = CRMAcctLineEdit::CustAndProspect;
      break;
  }

  list->setSubtype(subtype);
  return list;
}

VirtualSearch* CLineEdit::searchFactory()
{
  CRMAcctSearch* search = new CRMAcctSearch(this);
  enum CRMAcctLineEdit::CRMAcctSubtype subtype = CRMAcctLineEdit::Cust;
  switch (_type)
  {
    search->setShowInactive(true);
    case ActiveCustomers:
      search->setShowInactive(false);
      // fall-through
    case AllCustomers:
      break;

    case ActiveProspects:
      search->setShowInactive(false);
      // fall-through
    case AllProspects:
      subtype = CRMAcctLineEdit::Prospect;
      break;

    case ActiveCustomersAndProspects:
      search->setShowInactive(false);
      // fall-through
    case AllCustomersAndProspects:
      subtype = CRMAcctLineEdit::CustAndProspect;
      break;
  }

  search->setSubtype(subtype);
  return search;
}


//////////////////////////////////////////////////////////////

CustCluster::CustCluster(QWidget *pParent, const char *pName) :
    VirtualCluster(pParent, pName)
{
  addNumberWidget(new CLineEdit(this, pName));

  CLineEdit* number = static_cast<CLineEdit*>(_number);
  connect(number, SIGNAL(newCrmacctId(int)), this, SIGNAL(newCrmacctId(int)));

  setLabel(tr("Customer #:"));
  setNameVisible(true);
  setDescriptionVisible(true);
}

void CustCluster::setType(CLineEdit::CLineEditTypes pType)
{
  static_cast<CLineEdit*>(_number)->setType(pType);
}

void CustCluster::setEditMode(bool p)
{
  // TO DO
}


