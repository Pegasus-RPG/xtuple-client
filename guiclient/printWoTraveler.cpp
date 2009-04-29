/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printWoTraveler.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>
#include "inputManager.h"

/*
 *  Constructs a printWoTraveler as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
printWoTraveler::printWoTraveler(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    // signals and slots connections
    connect(_wo, SIGNAL(valid(bool)), this, SLOT(sHandlePrintButton()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_wo, SIGNAL(newId(int)), this, SLOT(sHandleOptions(int)));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_pickList, SIGNAL(toggled(bool)), this, SLOT(sHandlePrintButton()));
    connect(_routing, SIGNAL(toggled(bool)), this, SLOT(sHandlePrintButton()));
    connect(_packingList, SIGNAL(toggled(bool)), this, SLOT(sHandlePrintButton()));
    connect(_woLabel, SIGNAL(toggled(bool)), this, SLOT(sHandlePrintButton()));

    _captive = FALSE;

    omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

    _wo->setType(cWoExploded | cWoReleased | cWoIssued);

    if (!_privileges->check("ReleaseWorkOrders"))
      _releaseWo->setEnabled(FALSE);
      
    if (!_metrics->boolean("Routings"))
    {
      _routing->setChecked(FALSE);
      _routing->hide();
    }
}

/*
 *  Destroys the object and frees any allocated resources
 */
printWoTraveler::~printWoTraveler()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void printWoTraveler::languageChange()
{
    retranslateUi(this);
}

enum SetResponse printWoTraveler::set(const ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _wo->setId(param.toInt());
    _releaseWo->setFocus();
  }

  if (pParams.inList("print"))
  {
    sPrint();
    return NoError_Print;
  }

  return NoError;
}

void printWoTraveler::sHandleOptions(int pWoid)
{
  if (pWoid != -1)
  {
    XSqlQuery check;

    check.prepare( "SELECT womatl_id "
                   "FROM womatl "
                   "WHERE (womatl_wo_id=:wo_id) "
                   "LIMIT 1;" );
    check.bindValue(":wo_id", pWoid);
    check.exec();
    if (check.lastError().type() != QSqlError::NoError)
    {
      systemError(this, check.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    _pickList->setEnabled(check.first());
    _pickList->setChecked(check.first());
  
    if (_metrics->boolean("Routings"))
    {
      check.prepare( "SELECT wooper_id "
                     "FROM wooper "
                     "WHERE (wooper_wo_id=:wo_id) "
                     "LIMIT 1;" );
      check.bindValue(":wo_id", pWoid);
      check.exec();
      if (check.lastError().type() != QSqlError::NoError)
      {
        systemError(this, check.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
      _routing->setEnabled(check.first());
      _routing->setChecked(check.first());
    }

    check.prepare( "SELECT wo_id "
                   "FROM wo "
                   "WHERE ( (wo_ordtype='S')"
                   " AND (wo_id=:wo_id) );" );
    check.bindValue(":wo_id", pWoid);
    check.exec();
    if (check.lastError().type() != QSqlError::NoError)
    {
      systemError(this, check.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    _packingList->setEnabled(check.first());
    _packingList->setChecked(check.first()); 
    
  }
}

void printWoTraveler::sPrint()
{
  QPrinter  printer(QPrinter::HighResolution);
  bool      setupPrinter = TRUE;
  bool userCanceled = false;
  if (orReport::beginMultiPrint(&printer, userCanceled) == false)
  {
    if(!userCanceled)
      systemError(this, tr("Could not initialize printing system for multiple reports."));
    return;
  }

  if (_pickList->isChecked())
  {
    ParameterList params;
    params.append("wo_id", _wo->id());

    orReport report("PickList", params);

    if (report.isValid() && report.print(&printer, setupPrinter))
      setupPrinter = FALSE;
    else
    {
      report.reportError(this);
      orReport::endMultiPrint(&printer);
      return;
    }
  }

  if (_routing->isChecked())
  {
    ParameterList params;
    params.append("wo_id", _wo->id());

    orReport report("Routing", params);

    if (report.isValid() && report.print(&printer, setupPrinter))
      setupPrinter = FALSE;
    else
    {
      report.reportError(this);
      orReport::endMultiPrint(&printer);
      return;
    }
  }

  if (_woLabel->isChecked())
  {
    XSqlQuery query;
    query.prepare( "SELECT wo_id, CAST(wo_qtyord AS INTEGER) AS wo_qtyord_int "
                   "FROM wo "
                   " WHERE (wo_id=:wo_id);" );
    query.bindValue(":wo_id", _wo->id());
    query.exec();
    if (query.first())
    {
      ParameterList params;
      params.append("wo_id", query.value("wo_id"));
      params.append("labelTo", query.value("wo_qtyord_int"));

      orReport report("WOLabel", params);
      if (report.isValid() && report.print(&printer, setupPrinter))
	setupPrinter = FALSE;
      else
      {
	report.reportError(this);
	orReport::endMultiPrint(&printer);
	return;
      }
    }
    else if (query.lastError().type() != QSqlError::NoError)
    {
      systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  if (_packingList->isChecked())
  {
    XSqlQuery query;
    query.prepare( "SELECT cohead_id, findCustomerForm(cohead_cust_id, 'L') AS reportname "
                   "FROM cohead, coitem, wo "
                   "WHERE ( (coitem_cohead_id=cohead_id)"
                   " AND (wo_ordid=coitem_id)"
                   " AND (wo_ordtype='S')"
                   " AND (wo_id=:wo_id) );" );
    query.bindValue(":wo_id", _wo->id());
    query.exec();
    if (query.first())
    {
      ParameterList params;
      params.append("sohead_id", query.value("cohead_id"));
      params.append("head_id",  query.value("cohead_id"));
      params.append("head_type",  "SO");
      if (_metrics->boolean("MultiWhs"))
	params.append("MultiWhs");

      orReport report(query.value("reportname").toString(), params);
      if (report.isValid() && report.print(&printer, setupPrinter))
	setupPrinter = FALSE;
      else
      {
	report.reportError(this);
	orReport::endMultiPrint(&printer);
	return;
      }
    }
    else if (query.lastError().type() != QSqlError::NoError)
    {
      systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  orReport::endMultiPrint(&printer);

  if (_releaseWo->isChecked())
  {
    XSqlQuery release;
    release.prepare("SELECT releaseWo(:wo_id, FALSE);");
    release.bindValue(":wo_id", _wo->id());
    release.exec();
    if (release.lastError().type() != QSqlError::NoError)
    {
      systemError(this, release.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    omfgThis->sWorkOrdersUpdated(_wo->id(), TRUE);
  }

  if (_captive)
    close();
  else
  {
    _wo->setId(-1);
    _wo->setFocus();
  }
}

void printWoTraveler::sHandlePrintButton()
{
  _print->setEnabled( (_wo->isValid()) &&
                      ( (_pickList->isChecked()) || (_routing->isChecked()) || (_packingList->isChecked())  || (_woLabel->isChecked())) );
}
