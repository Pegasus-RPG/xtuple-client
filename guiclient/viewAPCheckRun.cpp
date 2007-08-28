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

#include "viewAPCheckRun.h"

#include <qvariant.h>
#include <qstatusbar.h>
#include <openreports.h>
#include "miscAPCheck.h"
#include "postAPCheck.h"
#include "printAPCheck.h"
#include "printAPChecks.h"
/*
 *  Constructs a viewAPCheckRun as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
viewAPCheckRun::viewAPCheckRun(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_replace, SIGNAL(clicked()), this, SLOT(sReplace()));
    connect(_replaceAll, SIGNAL(clicked()), this, SLOT(sReplaceAll()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_bankaccnt, SIGNAL(newID(int)), this, SLOT(sFillList()));
    connect(_apchk, SIGNAL(selectionChanged(Q3ListViewItem*)), this, SLOT(sHandleItemSelection(Q3ListViewItem*)));
    connect(_apchk, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    connect(_void, SIGNAL(clicked()), this, SLOT(sVoid()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_printCheck, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_postCheck, SIGNAL(clicked()), this, SLOT(sPost()));
    connect(_printCheckRun, SIGNAL(clicked()), this, SLOT(sPrintCheckRun()));
    connect(_printEditList, SIGNAL(clicked()), this, SLOT(sPrintEditList()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
viewAPCheckRun::~viewAPCheckRun()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void viewAPCheckRun::languageChange()
{
    retranslateUi(this);
}


void viewAPCheckRun::init()
{
  statusBar()->hide();
  
  _bankaccnt->setType(XComboBox::APBankAccounts);

  _apchk->setRootIsDecorated(TRUE);
  _apchk->addColumn(tr("Void"),           _ynColumn,    Qt::AlignCenter );
  _apchk->addColumn(tr("Misc."),          _ynColumn,    Qt::AlignCenter );
  _apchk->addColumn(tr("Prt'd"),          _ynColumn,    Qt::AlignCenter );
  _apchk->addColumn(tr("Chk./Voucher #"), _itemColumn,  Qt::AlignCenter );
  _apchk->addColumn(tr("Vendor/Invc. #"), -1,           Qt::AlignLeft   );
  _apchk->addColumn(tr("Check Date") ,    _dateColumn,  Qt::AlignCenter );
  _apchk->addColumn(tr("Amount"),         _moneyColumn, Qt::AlignRight  );
  _apchk->addColumn(tr("Currency"),	  _currencyColumn, Qt::AlignLeft );

  if (omfgThis->singleCurrency())
      _apchk->hideColumn(7);

  connect(omfgThis, SIGNAL(apChecksUpdated(int, int, bool)), this, SLOT(sFillList(int)));

  sFillList();
}

void viewAPCheckRun::sVoid()
{
  q.prepare( "SELECT apchk_bankaccnt_id, voidAPCheck(apchk_id) AS result "
             "FROM apchk "
             "WHERE (apchk_id=:apchk_id);" );
  q.bindValue(":apchk_id", _apchk->id());
  q.exec();
  if (q.first())
    omfgThis->sAPChecksUpdated(q.value("apchk_bankaccnt_id").toInt(), _apchk->id(), TRUE);
}

void viewAPCheckRun::sDelete()
{
  q.prepare( "SELECT apchk_bankaccnt_id, deleteAPCheck(apchk_id) AS result "
             "FROM apchk "
             "WHERE (apchk_id=:apchk_id);" );
  q.bindValue(":apchk_id", _apchk->id());
  q.exec();
  if (q.first())
    omfgThis->sAPChecksUpdated(q.value("apchk_bankaccnt_id").toInt(), _apchk->id(), TRUE);
}

void viewAPCheckRun::sEdit()
{
  ParameterList params;
  params.append("edit");
  params.append("apchk_id", _apchk->id());

  miscAPCheck *newdlg = new miscAPCheck();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void viewAPCheckRun::sReplace()
{
  q.prepare( "SELECT apchk_bankaccnt_id, replaceVoidedAPCheck(:apchk_id) AS result "
             "FROM apchk "
             "WHERE (apchk_id=:apchk_id);" );
  q.bindValue(":apchk_id", _apchk->id());
  q.exec();
  if (q.first())
    omfgThis->sAPChecksUpdated( q.value("apchk_bankaccnt_id").toInt(),
                                q.value("result").toInt(), TRUE);
}

void viewAPCheckRun::sReplaceAll()
{
  q.prepare("SELECT replaceAllVoidedAPChecks(:bankaccnt_id) AS result;");
  q.bindValue(":bankaccnt_id", _bankaccnt->id());
  q.exec();
  if (q.first())
    omfgThis->sAPChecksUpdated(_bankaccnt->id(), -1, TRUE);
}

void viewAPCheckRun::sPrint()
{
  ParameterList params;
  params.append("apchk_id", _apchk->id());

  printAPCheck newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void viewAPCheckRun::sPost()
{
  ParameterList params;
  params.append("apchk_id", _apchk->id());

  postAPCheck newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void viewAPCheckRun::sHandleItemSelection(Q3ListViewItem *pSelected)
{
  if (pSelected->text(0) == tr("Yes"))
  {
    _void->setEnabled(FALSE);
    _delete->setEnabled(TRUE);
    _replace->setEnabled(TRUE);
    _printCheck->setEnabled(FALSE);

    _edit->setEnabled(FALSE);
    _postCheck->setEnabled(FALSE);
  }
  else if (pSelected->text(0) == tr("No"))
  {
    _void->setEnabled(TRUE);
    _delete->setEnabled(FALSE);
    _replace->setEnabled(FALSE);
    _printCheck->setEnabled(TRUE);

    _edit->setEnabled((pSelected->text(1) == tr("Yes")) && (pSelected->text(2) == tr("No")));
    _postCheck->setEnabled((pSelected->text(2) == tr("Yes")) && (_privleges->check("PostPayments")));
  }
}

void viewAPCheckRun::sFillList(int pBankaccntid)
{
  if (pBankaccntid == _bankaccnt->id())
    sFillList();
}

void viewAPCheckRun::sFillList()
{
  _apchk->clear();

  QString sql( "SELECT apchk_id AS apchkid, -1 AS apchkitem_id,"
               "       formatBoolYN(apchk_void) AS f_void,"
               "       formatBoolYN(apchk_misc) AS f_misc,"
               "       formatBoolYN(apchk_printed) AS f_printed,"
               "       TEXT(apchk_number) AS number,"
               "       (vend_number || '-' || vend_name) AS description,"
               "       formatDate(apchk_checkdate) AS f_checkdate,"
               "       formatMoney(apchk_amount) AS f_amount,"
               "       CASE WHEN (apchk_misc) THEN 1"
               "            ELSE 0"
               "       END AS misc,"
               "       apchk_number, currConcat(apchk_curr_id) AS curr_concat, "
	       "       1 AS orderby "
               "FROM apchk, vend "
               "WHERE ( (apchk_vend_id=vend_id) "
               " AND (apchk_bankaccnt_id=:bankaccnt_id) "
               " AND (NOT apchk_posted)"
               " AND (NOT apchk_replaced)"
               " AND (NOT apchk_deleted) ) "

               "UNION SELECT apchkitem_apchk_id AS apchkid, apchkitem_id,"
               "             '' AS f_void, '' AS f_misc, '' AS f_printed,"
               "             apchkitem_vouchernumber AS number,"
               "             apchkitem_invcnumber AS description,"
               "             '' AS f_checkdate,"
               "             formatMoney(apchkitem_amount) AS f_amount,"
               "             0 AS misc, apchk_number, "
	       "             currConcat(apchkitem_curr_id) AS curr_concat, "
	       "             2 AS orderby "
               "FROM apchkitem, apchk "
               "WHERE ( (apchkitem_apchk_id=apchk_id)"
               " AND (apchk_bankaccnt_id=:bankaccnt_id) "
               " AND (NOT apchk_posted)"
               " AND (NOT apchk_replaced)"
               " AND (NOT apchk_deleted) ) "

               "ORDER BY apchk_number, apchkid, orderby;" );

  q.prepare(sql);
  q.bindValue(":bankaccnt_id", _bankaccnt->id());
  q.exec();
  if (q.first())
  {
    XListViewItem *header = NULL;
    int           apchkid = -1;

    do
    {
      if (q.value("apchkid").toInt() != apchkid)
      {
        apchkid = q.value("apchkid").toInt();
        header = new XListViewItem( _apchk, _apchk->lastItem(), apchkid, q.value("misc").toInt(),
                                    q.value("f_void"), q.value("f_misc"),
                                    q.value("f_printed"), q.value("number"),
                                    q.value("description"), q.value("f_checkdate"),
                                    q.value("f_amount"), q.value("curr_concat"));
      }
      else if (header)
      {
        XListViewItem *item = new XListViewItem( header, apchkid, 0);
        item->setText(3, q.value("number"));
        item->setText(4, q.value("description"));
        item->setText(6, q.value("f_amount"));
      }
    }
    while (q.next());
  }
}


void viewAPCheckRun::sPrintEditList()
{
  ParameterList params;
  params.append("bankaccnt_id", _bankaccnt->id()); 
    
  orReport report("ViewAPCheckRunEditList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void viewAPCheckRun::sPrintCheckRun()
{
  ParameterList params;
  params.append("bankaccnt_id", _bankaccnt->id()); 

  printAPChecks newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}
