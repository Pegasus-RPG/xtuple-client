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
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
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

#include "dspSummarizedGLTransactions.h"

#include <QVariant>
//#include <QStatusBar>
#include <QWorkspace>
#include <QMenu>
#include <QMessageBox>
#include <openreports.h>
#include <parameter.h>
#include "glTransactionDetail.h"
#include "voucher.h"
#include "miscVoucher.h"
#include "invoice.h"
#include "purchaseOrder.h"

/*
 *  Constructs a dspSummarizedGLTransactions as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspSummarizedGLTransactions::dspSummarizedGLTransactions(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_selectedSource, SIGNAL(toggled(bool)), _source, SLOT(setEnabled(bool)));
  connect(_gltrans, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));

  _gltrans->setRootIsDecorated(TRUE);
  _gltrans->addColumn(tr("Account #"),         150,               Qt::AlignCenter, true,  "account" );
  _gltrans->addColumn(tr("Date"),              _dateColumn,       Qt::AlignCenter, true,  "gltrans_date" );
  _gltrans->addColumn(tr("Description/Notes"), -1,                Qt::AlignLeft,   true,  "descrip_notes"   );
  _gltrans->addColumn(tr("Src."),              _whsColumn,        Qt::AlignCenter, true,  "gltrans_source" );
  _gltrans->addColumn(tr("Doc. Type"),         _docTypeColumn,    Qt::AlignCenter, true,  "gltrans_doctype" );
  _gltrans->addColumn(tr("Doc. #"),            _orderColumn,      Qt::AlignCenter, true,  "gltrans_docnumber" );
  _gltrans->addColumn(tr("Debit"),             _bigMoneyColumn,   Qt::AlignRight,  true,  "debit"  );
  _gltrans->addColumn(tr("Credit"),            _bigMoneyColumn,   Qt::AlignRight,  true,  "credit"  );
  _gltrans->addColumn( tr("Username"),         _userColumn,       Qt::AlignLeft,   true,  "gltrans_username" );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspSummarizedGLTransactions::~dspSummarizedGLTransactions()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspSummarizedGLTransactions::languageChange()
{
  retranslateUi(this);
}

void dspSummarizedGLTransactions::sPopulateMenu(QMenu * menuThis)
{
  if(_gltrans->altId() == -1)
    return;

  menuThis->insertItem(tr("View..."), this, SLOT(sViewTrans()), 0);

  QTreeWidgetItem * item = _gltrans->currentItem();
  if(0 == item)
    return;

  if(item->text(4) == "VO")
    menuThis->insertItem(tr("View Voucher..."), this, SLOT(sViewDocument()));
  else if(item->text(4) == "IN")
    menuThis->insertItem(tr("View Invoice..."), this, SLOT(sViewDocument()));
  else if(item->text(4) == "PO")
    menuThis->insertItem(tr("View Purchase Order..."), this, SLOT(sViewDocument()));
}


void dspSummarizedGLTransactions::sPrint()
{
  ParameterList params;
  _dates->appendValue(params);

  if (_unpostedTransactions->isChecked())
    params.append("unpostedTransactions");
  else if (_postedTransactions->isChecked())
    params.append("postedTransactions");

  if (_selectedSource->isChecked())
    params.append("source", _source->currentText());

  params.append("showUsernames");

  orReport report("SummarizedGLTransactions", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspSummarizedGLTransactions::sFillList()
{
  _gltrans->clear();

  QString sql( "SELECT accnt_id, gltrans_id,"
               "       account, accnt_descrip,"
               "       gltrans_date, gltrans_source, gltrans_doctype, gltrans_docnumber,"
               "       f_notes,"
               "       debit, credit,"
               "       gltrans_username, gltrans_created,"
               "       CASE WHEN (level=0) THEN accnt_descrip"
               "            ELSE f_notes"
               "       END AS descrip_notes,"
               "       'curr' AS debit_xtnumericrole,"
               "       'curr' AS credit_xtnumericrole,"
               "       0 AS debit_xtnumericrole,"
               "       0 AS credit_xtnumericrole,"
               "       CASE WHEN (debit=0) THEN '' END AS debit_qtdisplayrole,"
               "       CASE WHEN (credit=0) THEN '' END AS credit_qtdisplayrole,"
               "       level AS xtindentrole "
               "FROM ( "
               "SELECT DISTINCT accnt_id, -1 AS gltrans_id, 0 AS level,"
               "       formatGLAccount(accnt_id) AS account, accnt_descrip,"
               "       CAST(NULL AS DATE) AS gltrans_date, '' AS gltrans_source, '' AS gltrans_doctype, '' AS gltrans_docnumber,"
               "       '' AS f_notes,"
               "       0 AS debit,"
               "       0 AS credit,"
               "       '' AS gltrans_username, CAST(NULL AS TIMESTAMP) AS gltrans_created "
               "FROM gltrans, accnt "
               "WHERE ( (gltrans_accnt_id=accnt_id)"
               " AND (gltrans_date BETWEEN :startDate AND :endDate) ");

  if (_selectedSource->isChecked())
    sql += " AND (gltrans_source=:source)";

  if (_unpostedTransactions->isChecked())
    sql += " AND (NOT gltrans_posted)";
  else if (_postedTransactions->isChecked())
    sql += " AND (gltrans_posted)";

  sql += ") UNION "
               "SELECT accnt_id, gltrans_id, 1 AS level,"
               "       '' AS account, '' AS accnt_descrip,"
               "       gltrans_date, gltrans_source, gltrans_doctype, gltrans_docnumber,"
               "       firstLine(gltrans_notes) AS f_notes,"
               "       CASE WHEN (gltrans_amount < 0) THEN (gltrans_amount * -1)"
               "            ELSE 0"
               "       END AS debit,"
               "       CASE WHEN (gltrans_amount > 0) THEN gltrans_amount"
               "            ELSE 0"
               "       END AS credit,"
               "       gltrans_username, gltrans_created "
               "FROM gltrans, accnt "
               "WHERE ( (gltrans_accnt_id=accnt_id)"
               " AND (gltrans_date BETWEEN :startDate AND :endDate) ";

  if (_selectedSource->isChecked())
    sql += " AND (gltrans_source=:source)";

  if (_unpostedTransactions->isChecked())
    sql += " AND (NOT gltrans_posted)";
  else if (_postedTransactions->isChecked())
    sql += " AND (gltrans_posted)";

  sql += ") ) AS data "
         "ORDER BY accnt_id, gltrans_id, level, gltrans_date, gltrans_created;";

  q.prepare(sql);
  _dates->bindValue(q);
  q.bindValue(":source", _source->currentText());
  q.exec();
  if (q.first())
  {
    _gltrans->populate(q, true);
  }
}

void dspSummarizedGLTransactions::sViewTrans()
{
  ParameterList params;

  params.append("gltrans_id", _gltrans->altId());

  glTransactionDetail newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspSummarizedGLTransactions::sViewDocument()
{
  QTreeWidgetItem * item = _gltrans->currentItem();
  if(0 == item)
    return;

  ParameterList params;
  if(item->text(4) == "VO")
  {
    q.prepare("SELECT vohead_id, vohead_misc "
              "  FROM vohead"
              " WHERE (vohead_number=:vohead_number)");
    q.bindValue(":vohead_number", item->text(5));
    q.exec();
    if(!q.first())
      return;

    params.append("vohead_id", q.value("vohead_id").toInt());
    params.append("mode", "view");

    if(q.value("vohead_misc").toBool())
    {
      miscVoucher *newdlg = new miscVoucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else
    {
      voucher *newdlg = new voucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
  }
  else if(item->text(4) == "IN")
  {
    q.prepare("SELECT invchead_id"
              "  FROM invchead"
              " WHERE (invchead_invcnumber=:invchead_invcnumber)");
    q.bindValue(":invchead_invcnumber", item->text(5));
    q.exec();
    if(!q.first())
      return;

    invoice::viewInvoice(q.value("invchead_id").toInt());
  }
  else if(item->text(4) == "PO")
  {
    q.prepare("SELECT pohead_id"
              "  FROM pohead"
              " WHERE (pohead_number=:pohead_number)");
    q.bindValue(":pohead_number", item->text(5));
    q.exec();
    if(!q.first())
      return;

    params.append("pohead_id", q.value("pohead_id").toInt());
    params.append("mode", "view");

    purchaseOrder *newdlg = new purchaseOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

