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

#include "dspAPAging.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <openreports.h>
#include "rptAPAging.h"

/*
 *  Constructs a dspAPAging as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspAPAging::dspAPAging(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_calendar, SIGNAL(newCalendarId(int)), this, SLOT(sCalendarSelected(int)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspAPAging::~dspAPAging()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspAPAging::languageChange()
{
    retranslateUi(this);
}


void dspAPAging::init()
{
  _date->setDate( omfgThis->dbDate() );
  
  _list->addColumn( tr("Period/Vendor"),          -1, Qt::AlignLeft   );
  _list->addColumn( tr("Doc. Type"),      _orderColumn, Qt::AlignLeft   );
  _list->addColumn( tr("Doc. Number"),     _itemColumn, Qt::AlignLeft   );
  _list->addColumn( tr("Due Date"),        _dateColumn, Qt::AlignCenter );
  _list->addColumn( tr("Balance"),     _bigMoneyColumn, Qt::AlignRight  );
}

void dspAPAging::sPrint()
{
  ParameterList params;
  params.append("calhead_id", _calendar->id());
  params.append("date", _date->date());
  params.append("relDate", _date->date());
  params.append("print");

  rptAPAging newdlg(this, "", TRUE);
  newdlg.set(params);
}

void dspAPAging::sCalendarSelected( int id )
{
  q.prepare("SELECT calhead_type"
                "  FROM calhead"
                " WHERE (calhead_id=:calheadid); ");
  q.bindValue(":calheadid", id);
  q.exec();
  if( q.first() && (q.value("calhead_type").toString() == "A") )
    _date->setEnabled(false);
  else
    _date->setEnabled(true);
}

void dspAPAging::sFillList()
{
  _list->clear();

  q.prepare("SELECT calitem_id, (calitem_name || '  (' || calitem_periodstart || '-' || calitem_periodend || ')') AS period,"
            "       vend_id, (vend_number || '-' || vend_name) AS vendor,"
            "       apopen_id, apopen_doctype, apopen_docnumber, formatDate(apopen_duedate) AS f_duedate,"
            "       formatMoney((apopen_amount - apopen_paid) * CASE WHEN(apopen_doctype='C') THEN -1 ELSE 1 END) AS f_balance,"
            "       ((apopen_amount - apopen_paid) * CASE WHEN(apopen_doctype='C') THEN -1 ELSE 1 END) AS balance"
            "  FROM apopen, vend,"
            "       ( SELECT rcalitem_id AS calitem_id,"
            "                rcalitem_name AS calitem_name,"
            "                (findPeriodStart(rcalitem_id) + COALESCE(:offset,0)) AS calitem_periodstart,"
            "                (findPeriodEnd(rcalitem_id) + COALESCE(:offset,0)) AS calitem_periodend"
            "           FROM rcalitem"
            "          WHERE (rcalitem_calhead_id=:calhead_id)"
            "          UNION"
            "         SELECT acalitem_id AS calitem_id,"
            "                acalitem_name AS calitem_name,"
            "                findPeriodStart(acalitem_id) AS calitem_periodstart,"
            "                findPeriodEnd(acalitem_id) AS calitem_periodend"
            "           FROM acalitem"
            "          WHERE (acalitem_calhead_id=:calhead_id) ) AS calitem"
            " WHERE ((apopen_vend_id=vend_id)"
            "   AND  (apopen_duedate BETWEEN calitem_periodstart AND calitem_periodend)"
            "   AND  ((apopen_amount - apopen_paid) > 0))"
            " ORDER BY calitem_periodstart DESC, vend_number, apopen_duedate;");
  q.bindValue(":calhead_id", _calendar->id());
  q.bindValue(":offset", omfgThis->dbDate().daysTo(_date->date()));
  q.exec();

  XListViewItem * period = 0;
  XListViewItem * vend   = 0;
  XListViewItem * last   = 0;
  int periodid = -1;
  double periodtotal = 0.0;
  int vendid   = -1;
  double vendtotal = 0.0;
  while(q.next())
  {
    if(q.value("calitem_id").toInt() != periodid)
    {
      if(period)
      {
        if(vend)
          last = new XListViewItem(vend, last, -1, -1, "", "", tr("Subtotal:"), "", formatMoney(vendtotal));
        last = new XListViewItem(period, vend, -1, -1, tr("Total:"), "", "", "", formatMoney(periodtotal));
      }
      periodid = q.value("calitem_id").toInt();
      last = period = new XListViewItem(_list, period, periodid, 1, q.value("period"));
      periodtotal = 0;
      vend = 0;
      vendid = -1;
      vendtotal = 0;
    }

    if(q.value("vend_id").toInt() != vendid)
    {
      if(vend)
        last = new XListViewItem(vend, last, -1, -1, "", "", tr("Subtotal:"), "", formatMoney(vendtotal));
      vendid = q.value("vend_id").toInt();
      last = vend = new XListViewItem(period, vend, vendid, 2, q.value("vendor"));
      vendtotal = 0;
    }

    last = new XListViewItem(vend, last, q.value("apopen_id").toInt(), 3,
      "", q.value("apopen_doctype"), q.value("apopen_docnumber"),
      q.value("f_duedate"), q.value("f_balance"));

    vendtotal += q.value("balance").toDouble();
    periodtotal += q.value("balance").toDouble();
  }

  if(period)
  {
    if(vend)
      last = new XListViewItem(vend, last, -1, -1, "", "", tr("Subtotal:"), "", formatMoney(vendtotal));
    last = new XListViewItem(period, vend, -1, -1, tr("Total:"), "", "", "", formatMoney(periodtotal));
  }

  _list->openAll();
}

