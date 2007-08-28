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

#include "dspEarnedCommissions.h"

#include <qvariant.h>
#include <qstatusbar.h>
#include <parameter.h>
#include "rptEarnedCommissions.h"
#include "OpenMFGGUIClient.h"

/*
 *  Constructs a dspEarnedCommissions as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspEarnedCommissions::dspEarnedCommissions(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_selectedSalesrep, SIGNAL(toggled(bool)), _salesrep, SLOT(setEnabled(bool)));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspEarnedCommissions::~dspEarnedCommissions()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspEarnedCommissions::languageChange()
{
    retranslateUi(this);
}


void dspEarnedCommissions::init()
{
  statusBar()->hide();

  _salesrep->setType(XComboBox::SalesReps);
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _commission->addColumn(tr("Sales Rep."),  100,          Qt::AlignLeft   );
  _commission->addColumn(tr("S/O #"),       _orderColumn, Qt::AlignRight  );
  _commission->addColumn(tr("Cust. #"),     _orderColumn, Qt::AlignLeft   );
  _commission->addColumn(tr("Ship-To"),     -1,           Qt::AlignLeft   );
  _commission->addColumn(tr("Invc. Date"),  _dateColumn,  Qt::AlignCenter );
  _commission->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft   );
  _commission->addColumn(tr("Qty."),        _qtyColumn,   Qt::AlignRight  );
  _commission->addColumn(tr("Ext. Price"),  _moneyColumn, Qt::AlignRight  );
  _commission->addColumn(tr("Commission"),  _moneyColumn, Qt::AlignRight  );
  _commission->addColumn(tr("Paid"),        _ynColumn,    Qt::AlignCenter );
}

void dspEarnedCommissions::sPrint()
{
  ParameterList params;
  _dates->appendValue(params);
  params.append("print");
  
  if (_selectedSalesrep->isChecked())
    params.append("salesrep_id", _salesrep->id());
  
  rptEarnedCommissions newdlg(this, "", TRUE);
  newdlg.set(params);
}

void dspEarnedCommissions::sFillList()
{
  _commission->clear();

  if (_dates->allValid())
  {
    QString sql( "SELECT cohist_id, salesrep_name, cohist_ordernumber, cust_number, cohist_shiptoname,"
                 "       formatDate(cohist_invcdate) AS f_invoicedate,"
                 "       item_number,"
                 "       formatQty(cohist_qtyshipped) AS f_shipped,"
                 "       round(cohist_qtyshipped * cohist_unitprice,2) AS sales,"
                 "       formatMoney(round(cohist_qtyshipped * cohist_unitprice,2)) AS f_sales,"
                 "       cohist_commission AS commission,"
                 "       formatMoney(cohist_commission) AS f_commission,"
                 "       formatBoolYN(cohist_commissionpaid) AS f_commissionpaid "
                 "FROM cohist, salesrep, cust, itemsite, item "
                 "WHERE ( (cohist_cust_id=cust_id)"
                 " AND (cohist_salesrep_id=salesrep_id)"
                 " AND (cohist_itemsite_id=itemsite_id)"
                 " AND (itemsite_item_id=item_id)"
                 " AND (cohist_commission <> 0)"
                 " AND (cohist_invcdate BETWEEN :startDate AND :endDate)" );
    
    if (_selectedSalesrep->isChecked())
      sql += " AND (salesrep_id=:salesrep_id)";

    sql +=  ") "
            "ORDER BY salesrep_name, cohist_invcdate";

    q.prepare(sql);
    _dates->bindValue(q);
    q.bindValue(":salesrep_id", _salesrep->id());
    q.exec();
    if (q.first())
    {
      double sales = 0.0;
      double comm  = 0.0;
      do
      {
        new XListViewItem( _commission, _commission->lastItem(), q.value("cohist_id").toInt(),
                           q.value("salesrep_name"), q.value("cohist_ordernumber"),
                           q.value("cust_number"), q.value("cohist_shiptoname"),
                           q.value("f_invoicedate"), q.value("item_number"),
                           q.value("f_shipped"), q.value("f_sales"),
                           q.value("f_commission"), q.value("f_commissionpaid") );

        sales += q.value("sales").toDouble();
        comm+= q.value("commission").toDouble();
      }
      while (q.next());

      new XListViewItem( _commission, _commission->lastItem(), -1, QVariant(tr("Totals")),
                         "", "", "", "", "", "",
                         formatMoney(sales), formatMoney(comm) );
    }
  }
}
