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

#include "rptSalesOrderStatus.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <openreports.h>
#include "inputManager.h"
#include "salesOrderList.h"

/*
 *  Constructs a rptSalesOrderStatus as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
rptSalesOrderStatus::rptSalesOrderStatus(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_soList, SIGNAL(clicked()), this, SLOT(sSalesOrderList()));
    connect(_so, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));
    connect(_so, SIGNAL(newId(int)), this, SLOT(sPopulate(int)));
    connect(_so, SIGNAL(requestList()), this, SLOT(sSalesOrderList()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
rptSalesOrderStatus::~rptSalesOrderStatus()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void rptSalesOrderStatus::languageChange()
{
    retranslateUi(this);
}


void rptSalesOrderStatus::init()
{
#ifdef Q_WS_MAC
  _soList->setMaximumWidth(50);
#else
  _soList->setMaximumWidth(25);
#endif

  omfgThis->inputManager()->notify(cBCSalesOrder, this, _so, SLOT(setId(int)));

  _so->setFocus();
}

enum SetResponse rptSalesOrderStatus::set(ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("sohead_id", &valid);
  if (valid)
    _so->setId(param.toInt());

  if (pParams.inList("print"))
  {
    sPrint();
    return NoError;
  }

  return NoError;
}

void rptSalesOrderStatus::sSalesOrderList()
{
  ParameterList params;
  params.append("sohead_id", _so->id());
  params.append("soType", (cSoOpen | cSoClosed));
  
  salesOrderList newdlg(this, "", TRUE);
  newdlg.set(params);

  _so->setId(newdlg.exec());
}

void rptSalesOrderStatus::sPrint()
{
  ParameterList params;

  params.append("sohead_id", _so->id());

  orReport report("SalesOrderStatus", params);
  if (report.isValid())
    report.print();
  else
  {
    QMessageBox::critical( this, tr("Report Definition Not Found"),
                           tr( "The report defintions for this report, \"SalesOrderStatus\" cannot be found.\n"
                               "Please contact your Systems Administrator and report this issue." ) );
    reject();
  }

  if(_captive)
    accept();
  else
  {
    _so->setId(-1);
    _so->setFocus();
  }
}

void rptSalesOrderStatus::sPopulate(int pSoheadid)
{
  q.prepare( "SELECT cohead_orderdate "
             "FROM cohead "
             "WHERE (cohead_id=:cohead_id);" );
  q.bindValue("cohead_id", pSoheadid);
  q.exec();
  if (q.first())
    _orderDate->setDate(q.value("cohead_orderdate").toDate());
}

       
