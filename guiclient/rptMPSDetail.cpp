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

#include "rptMPSDetail.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <openreports.h>

/*
 *  Constructs a rptMPSDetail as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
rptMPSDetail::rptMPSDetail(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_calendar, SIGNAL(newCalendarId(int)), _periods, SLOT(populate(int)));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_calendar, SIGNAL(select(ParameterList&)), _periods, SLOT(load(ParameterList&)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
rptMPSDetail::~rptMPSDetail()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void rptMPSDetail::languageChange()
{
    retranslateUi(this);
}


void rptMPSDetail::init()
{
  _captive = FALSE;

  _plannerCode->setType(PlannerCode);

  _itemsite->addColumn("Itemtype",         0,           Qt::AlignCenter );
  _itemsite->addColumn(tr("Item Number"),  _itemColumn, Qt::AlignLeft   );
  _itemsite->addColumn(tr("Description"),  -1,          Qt::AlignLeft   );
  _itemsite->addColumn(tr("Whs."),         _whsColumn,  Qt::AlignCenter );
  _itemsite->addColumn(tr("Safety Stock"), _qtyColumn,  Qt::AlignRight  );

  sFillItemsites();
}

enum SetResponse rptMPSDetail::set(ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("plancode_id", &valid);
  if (valid)
    _plannerCode->setId(param.toInt());

  param = pParams.value("plancode_pattern", &valid);
  if (valid)
    _plannerCode->setPattern(param.toString());

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());
  else
    _warehouse->setAll();

  _calendar->load(pParams);
  _periods->load(pParams);

  sFillItemsites();

  param = pParams.value("itemsite_id", &valid);
  if (valid)
    _itemsite->setId(param.toInt());

  if (pParams.inList("print"))
  {
    sPrint();
    return NoError_Print;
  }

  return NoError;
}

void rptMPSDetail::sPrint()
{
  if ( (_periods->isPeriodSelected()) && (_itemsite->id() != -1))
  {
    XSqlQuery wsq ( QString( "SELECT mpsReport(%1, '%2') as worksetid;")
                    .arg(_itemsite->id())
                    .arg(_periods->periodString()) );
    if (wsq.first())
    {
      ParameterList params;

      _plannerCode->appendValue(params);
      _warehouse->appendValue(params);

      XListViewItem *cursor = _periods->firstChild();
      QList<QVariant> periodList;
      while (cursor)
      {
        if (cursor->isSelected())
          periodList.append(cursor->id());

        cursor = cursor->nextSibling();
      }
      params.append("period_id_list", periodList);
      params.append("itemsite_id", _itemsite->id());
      params.append("workset_id", wsq.value("worksetid").toInt());

      orReport report("MPSDetail", params);
      if (report.isValid())
        report.print();
      else
      {
        report.reportError(this);
        reject();
      }

      XSqlQuery dwsq( QString( "SELECT deleteMPSMRPWorkset(%1) as result")
                      .arg(wsq.value("worksetid").toInt()) );

      if(_captive)
        accept();
    }
    else
      QMessageBox::critical( this, tr("Error executing report"),
                             tr("There was an error executing this report. The Data could not\n"
                                "be generated for an unknown reason.") );
  }
  else
    QMessageBox::critical( this, tr("Incomplete criteria"),
                           tr( "The criteria you specified is not complete. Please make sure all\n"
                               "fields are correctly filled out before running the report." ) );
}

void rptMPSDetail::sFillItemsites()
{
  QString sql( "SELECT itemsite_id, item_type, item_number, (item_descrip1 || ' ' || item_descrip2),"
               " warehous_code, formatQty(itemsite_safetystock) "
               "FROM itemsite, item, warehous "
               "WHERE ( (itemsite_active)"
               " AND (itemsite_item_id=item_id)"
               " AND (itemsite_warehous_id=warehous_id)" );

  if (_plannerCode->isSelected())
    sql += " AND (itemsite_plancode_id=:plancode_id)";
  else if (_plannerCode->isPattern())
    sql += " AND (itemsite_plancode_id IN (SELECT plancode_id FROM plancode WHERE (plancode_code ~ :plancode_pattern)))";

  if (_warehouse->isSelected())
    sql += " AND (warehous_id=:warehous_id)";

  sql += ") "
         "ORDER BY item_number, warehous_code";

  XSqlQuery query;
  query.prepare(sql);
  _warehouse->bindValue(query);
  _plannerCode->bindValue(query);
  query.exec();
  _itemsite->populate(query);
}

