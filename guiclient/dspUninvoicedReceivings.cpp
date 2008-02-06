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

#include "dspUninvoicedReceivings.h"

#include <QVariant>
#include <QMessageBox>
#include <QStatusBar>
#include <QMenu>
#include <openreports.h>
#include <parameter.h>
#include "enterPoitemReceipt.h"
#include "poLiabilityDistrib.h"
#include "postPoReturnCreditMemo.h"

/*
 *  Constructs a dspUninvoicedReceivings as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspUninvoicedReceivings::dspUninvoicedReceivings(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_selectedPurchasingAgent, SIGNAL(toggled(bool)), _agent, SLOT(setEnabled(bool)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_agent, SIGNAL(newID(int)), this, SLOT(sFillList()));
  connect(_selectedPurchasingAgent, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_porecv, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));

  _agent->setType(XComboBox::Agent);
  _agent->setText(omfgThis->username());

  _porecv->addColumn(tr("Date"),        _dateColumn,  Qt::AlignCenter );
  _porecv->addColumn(tr("By"),          _orderColumn, Qt::AlignCenter );
  _porecv->addColumn(tr("P/O #"),       _orderColumn, Qt::AlignLeft   );
  _porecv->addColumn(tr("#"),           _whsColumn,   Qt::AlignCenter );
  _porecv->addColumn(tr("Vendor"),      -1,           Qt::AlignLeft   );
  _porecv->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft   );
  _porecv->addColumn(tr("Uninvoiced"),  _qtyColumn,   Qt::AlignRight  );
  _porecv->addColumn(tr("Type"),		_itemColumn,  Qt::AlignLeft   ); 
  _porecv->addColumn(tr("Value"),       _moneyColumn, Qt::AlignRight  );

  sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspUninvoicedReceivings::~dspUninvoicedReceivings()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspUninvoicedReceivings::languageChange()
{
  retranslateUi(this);
}

void dspUninvoicedReceivings::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  if(_privleges->check("MaintainUninvoicedReceipts"))
  {
    if(_porecv->altId() < 3)
    {
      menuItem = pMenu->insertItem(tr("Mark as Invoiced..."), this, SLOT(sMarkAsInvoiced()), 0);
      menuItem = pMenu->insertItem(tr("Correct Receiving..."), this, SLOT(sCorrectReceiving()), 0);
      if(_porecv->altId() == 2)
        pMenu->setItemEnabled(menuItem, false);
    }
    if (_porecv->altId() == 3)
      menuItem = pMenu->insertItem(tr("Create Credit Memo..."), this, SLOT(sCreateCreditMemo()), 0);
  }
}

void dspUninvoicedReceivings::sMarkAsInvoiced()
{
  bool update = TRUE;
  
  q.prepare("SELECT * FROM porecv "
            "WHERE ((porecv_value <> 0) "
            "AND (porecv_id=:porecv_id));");
  q.bindValue(":porecv_id",_porecv->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("porecv_id", _porecv->id());  
    poLiabilityDistrib newdlg(this, "", TRUE);
    newdlg.set(params);
    if (newdlg.exec() == QDialog::Rejected)
      update = FALSE;
  }
  if (update)
  {
    q.prepare("UPDATE recv "
	          "SET recv_invoiced=true "
			  "WHERE (recv_id=:porecv_id); ");
	q.bindValue(":porecv_id",_porecv->id());
	q.exec();
    sFillList();
  }
}

void dspUninvoicedReceivings::sCorrectReceiving()
{
  if (enterPoitemReceipt::correctReceipt(_porecv->id(), this) != QDialog::Rejected)
    sFillList();
}

void dspUninvoicedReceivings::sCreateCreditMemo()
{
  ParameterList params;
  params.append("poreject_id", _porecv->id());

  postPoReturnCreditMemo newdlg(this, "", TRUE);
  newdlg.set(params);

  if(newdlg.exec() == QDialog::Accepted)
    sFillList();
}

void dspUninvoicedReceivings::sPrint()
{
  ParameterList params;
  _warehouse->appendValue(params);

  if(_selectedPurchasingAgent->isChecked())
    params.append("agentUsername", _agent->currentText());

  orReport report("UninvoicedReceipts", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspUninvoicedReceivings::sFillList()
{
  _porecv->clear();
  QString sql( "SELECT porecv_id AS id,"
               "       CASE WHEN(poitem_status='C') THEN 2"
               "            ELSE 1"
               "       END AS doctype,"
               "       formatDate(porecv_date) AS f_date, getUsername(porecv_trans_usr_id) AS f_user,"
               "       porecv_ponumber AS ponumber, poitem_linenumber,"
               "       vend_name, COALESCE(item_number, ('Misc. - ' || porecv_vend_item_number)) AS itemnumber,"
               "       formatQty(porecv_qty) AS f_qty, "
               "	   'Receipt' AS type, "
               "       formatMoney(porecv_value) AS f_value, "
               "       porecv_value AS value "
               "FROM porecv, vend, poitem LEFT OUTER JOIN"
               "                   ( itemsite JOIN item"
               "                     ON (itemsite_item_id=item_id)"
               "                   ) ON (poitem_itemsite_id=itemsite_id) "
               "WHERE ( (porecv_poitem_id=poitem_id)"
               " AND (porecv_vend_id=vend_id)"
               " AND (porecv_posted)"
               " AND (NOT porecv_invoiced) ");
  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_selectedPurchasingAgent->isChecked())
    sql += " AND (porecv_agent_username=:username)";

  sql +=	   ") "
			   "UNION "
               "SELECT poreject_id,"
               "       3,"
               "       formatDate(poreject_date), getUsername(poreject_trans_usr_id),"
               "       poreject_ponumber AS ponumber, poitem_linenumber,"
               "       vend_name, COALESCE(item_number, ('Misc. - ' || poreject_vend_item_number)) AS itemnumber,"
               "       formatQty(poreject_qty), "
               "	   'Return', "
               "       formatMoney(poreject_value*-1), "
               "       poreject_value * -1 "
               "FROM poreject, vend, poitem LEFT OUTER JOIN"
               "                   ( itemsite JOIN item"
               "                     ON (itemsite_item_id=item_id)"
               "                   ) ON (poitem_itemsite_id=itemsite_id) "
               "WHERE ( (poreject_poitem_id=poitem_id)"
               " AND (poreject_vend_id=vend_id)"
               " AND (poreject_posted)"
               " AND (NOT poreject_invoiced) ";

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_selectedPurchasingAgent->isChecked())
    sql += " AND (poreject_agent_username=:username)";

  sql += ") "
         "ORDER BY ponumber, poitem_linenumber;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  q.bindValue(":username", _agent->currentText());
  q.exec();
  if (q.first())
  {
    double total= 0.0;

    XTreeWidgetItem * last = 0;
    do
    {
      last = new XTreeWidgetItem( _porecv, last, q.value("id").toInt(),
                                  q.value("doctype").toInt(), q.value("f_date"),
                                  q.value("f_user"), q.value("ponumber"),
                                  q.value("poitem_linenumber"), q.value("vend_name"),
                                  q.value("itemnumber"), q.value("f_qty"), q.value("type"), q.value("f_value") );
 
      total+= q.value("value").toDouble();
    }
    while (q.next());

    new XTreeWidgetItem( _porecv, last, -1, 4,
                         QVariant(tr("Total")), "", "", "", "", "", "", "",
                         formatMoney(total) );
  }
}


