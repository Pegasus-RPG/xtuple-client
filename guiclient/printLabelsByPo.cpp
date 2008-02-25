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

#include "printLabelsByPo.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

// TODO: rename to printLabelsByOrder
printLabelsByPo::printLabelsByPo(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_labelFrom, SIGNAL(valueChanged(int)), this, SLOT(sSetToMin(int)));
  connect(_po,		    SIGNAL(valid(bool)), this, SLOT(sHandlePo()));
  connect(_print,	      SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_to,		    SIGNAL(valid(bool)), this, SLOT(sHandleTo()));

  _captive = FALSE;

  _report->populate( "SELECT labelform_id, labelform_name "
                     "FROM labelform "
                     "ORDER BY labelform_name;" );
  _to->setVisible(_metrics->boolean("MultiWhs"));
}

printLabelsByPo::~printLabelsByPo()
{
    // no need to delete child widgets, Qt does it all for us
}

void printLabelsByPo::languageChange()
{
    retranslateUi(this);
}

enum SetResponse printLabelsByPo::set(const ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("pohead_id", &valid);
  if (valid)
  {
    _po->setId(param.toInt());
    _print->setFocus();
  }

  param = pParams.value("tohead_id", &valid);
  if (valid)
  {
    _to->setId(param.toInt());
    _print->setFocus();
  }

  return NoError;
}

void printLabelsByPo::sPrint()
{
  q.prepare( "SELECT report_name "
             "FROM labelform, report "
             "WHERE ( (labelform_id=:labelform_id) "
             " AND (report_id=labelform_report_id) );" );
  q.bindValue(":labelform_id", _report->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("pohead_id", _po->id());
    params.append("tohead_id", _to->id());
    params.append("labelFrom", _labelFrom->value());
    params.append("labelTo", _labelTo->value());

    orReport report(q.value("report_name").toString(), params);
    if (report.isValid())
      report.print();
    else
    {
      report.reportError(this);
      reject();
    }

    _po->setId(-1);
    _to->setId(-1);
    _po->setFocus();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    QMessageBox::warning(this, tr("Could not locate report"),
                         tr("<p>Could not locate the report definition for the form \"%1\"")
                         .arg(_report->currentText()) );

  if (_captive)
    accept();
}

void printLabelsByPo::sSetToMin(int pValue)
{
  _labelTo->setMinValue(pValue);
}

void printLabelsByPo::sHandlePo()
{
  _print->setEnabled(_po->isValid() || _to->isValid());
  _to->setEnabled(! _po->isValid());
}

void printLabelsByPo::sHandleTo()
{
  _print->setEnabled(_po->isValid() || _to->isValid());
  _po->setEnabled(! _to->isValid());
}
