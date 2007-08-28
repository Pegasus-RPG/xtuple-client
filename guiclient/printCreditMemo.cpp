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

#include "printCreditMemo.h"

#include <QApplication>
#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>
#include <QValidator>
#include <QVariant>

#include <openreports.h>
#include "editICMWatermark.h"

printCreditMemo::printCreditMemo(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);

    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_numberOfCopies, SIGNAL(valueChanged(int)), this, SLOT(sHandleCopies(int)));
    connect(_watermarks, SIGNAL(itemSelected(int)), this, SLOT(sEditWatermark()));

    _captive = FALSE;
    _setup   = FALSE;
    _alert   = TRUE;

    _cust->setReadOnly(TRUE);

    _watermarks->addColumn( tr("Copy #"),      _dateColumn, Qt::AlignCenter );
    _watermarks->addColumn( tr("Watermark"),   -1,          Qt::AlignLeft   );
    _watermarks->addColumn( tr("Show Prices"), _dateColumn, Qt::AlignCenter );

    _numberOfCopies->setValue(_metrics->value("CreditMemoCopies").toInt());
    if (_numberOfCopies->value())
    {
      Q3ListViewItem *cursor = _watermarks->firstChild();
      for (int counter = 0; cursor; cursor = cursor->nextSibling(), counter++)
      {
	cursor->setText(1, _metrics->value(QString("CreditMemoWatermark%1").arg(counter)));
	cursor->setText(2, ((_metrics->value(QString("CreditMemoShowPrices%1").arg(counter)) == "t") ? tr("Yes") : tr("No")));
      }
    }

    if(!_privleges->check("PostARDocuments"))
    {
      _post->setChecked(false);
      _post->setEnabled(false);
    }

    _print->setFocus();
}

printCreditMemo::~printCreditMemo()
{
  if (_captive)	// see sPrint()
    orReport::endMultiPrint(&_printer);
}

void printCreditMemo::languageChange()
{
    retranslateUi(this);
}

enum SetResponse printCreditMemo::set(const ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("cmhead_id", &valid);
  if (valid)
  {
    _cmheadid = param.toInt();
    populate();
  }

  if (pParams.inList("print"))
  {
    sPrint();
    return NoError_Print;
  }

  if (pParams.inList("persistentPrint"))
  {
    _alert = FALSE;

    if (_setup)
    {
      sPrint();
      return NoError_Print;
    }
    else
      return Error_NoSetup;
  }

  return NoError;
}

void printCreditMemo::sPrint()
{
  q.prepare( "SELECT cmhead_number, cmhead_printed, cmhead_posted,"
             "       findCustomerForm(cmhead_cust_id, 'C') AS reportname "
             "FROM cmhead "
             "WHERE (cmhead_id=:cmhead_id);" );
  q.bindValue(":cmhead_id", _cmheadid);
  q.exec();
  if (q.first())
  {
    QString reportname = q.value("reportname").toString();
    QString cmheadnumber = q.value("cmhead_number").toString();

    bool dosetup = (!_setup);
    if (dosetup)
    {
      bool userCanceled = false;
      if (orReport::beginMultiPrint(&_printer, userCanceled) == false)
      {
        if(!userCanceled)
	  systemError(this, tr("Could not initialize printing system for multiple reports."));
	return;
      }
    }


    Q3ListViewItem *cursor = _watermarks->firstChild();
    for (int counter = 0; cursor; cursor = cursor->nextSibling(), counter++ )
    {
      ParameterList params;
      params.append("cmhead_id", _cmheadid);
      params.append("showcosts", ((cursor->text(2) == tr("Yes")) ? "TRUE" : "FALSE"));
      params.append("watermark", cursor->text(1));

      orReport report(reportname, params);
      if (!report.isValid())
        QMessageBox::critical( this, tr("Cannot Find Credit Memo Form"),
                               tr( "The Credit Memo Form '%1' cannot be found for Credit Memo #%2.\n"
                                   "This Credit Memo cannot be printed until a Customer Form Assignment is updated to remove any\n"
                                   "references to this Credit Memo Form or this Credit Memo Form is created." )
                               .arg(reportname)
                               .arg(cmheadnumber) );
          
      else
      {
        if (report.print(&_printer, dosetup))
          dosetup = FALSE;
        else
        {
          systemError( this, tr("A Printing Error occurred at printCreditMemo::%1.")
                             .arg(__LINE__) );
	  if (!_captive)
	    orReport::endMultiPrint(&_printer);
          return;
        }

        q.prepare( "UPDATE cmhead "
                   "SET cmhead_printed=TRUE "
                   "WHERE (cmhead_id=:cmhead_id);" );
        q.bindValue(":cmhead_id", _cmheadid);
        q.exec();
	if (q.lastError().type() != QSqlError::None)
	{
	  systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	  return;
	}

        if (_alert)
          omfgThis->sCreditMemosUpdated();

        if (_post->isChecked())
        {
          q.prepare("SELECT postCreditMemo(:cmhead_id, 0) AS result;");
          q.bindValue(":cmhead_id", _cmheadid);
          q.exec();
	  if (q.lastError().type() != QSqlError::None)
	  {
	    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	    return;
	  }
        }
      }
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_captive)
    accept();
  else
    orReport::endMultiPrint(&_printer);
}

void printCreditMemo::sHandleCopies(int pValue)
{
  if (_watermarks->childCount() > pValue)
    _watermarks->takeItem(_watermarks->lastItem());
  else
  {
    for (unsigned int counter = (_watermarks->childCount() + 1); counter <= (unsigned int)pValue; counter++)
      new Q3ListViewItem(_watermarks, _watermarks->lastItem(), tr("Copy #%1").arg(counter), "", tr("Yes"));
  }
}

void printCreditMemo::sEditWatermark()
{
  XListViewItem *cursor = _watermarks->selectedItem();
  ParameterList params;
  params.append("watermark", cursor->text(1));
  params.append("showPrices", (cursor->text(2) == tr("Yes")));

  editICMWatermark newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() == QDialog::Accepted)
  {
    cursor->setText(1, newdlg.watermark());
    cursor->setText(2, ((newdlg.showPrices()) ? tr("Yes") : tr("No")));
  }
}

void printCreditMemo::populate()
{
  q.prepare( "SELECT cmhead_number, cmhead_cust_id "
             "FROM cmhead "
             "WHERE (cmhead_id=:cmhead_id);" );
  q.bindValue(":cmhead_id", _cmheadid);
  q.exec();
  if (q.first())
  {
    _number->setText(q.value("cmhead_number").toString());
    _cust->setId(q.value("cmhead_cust_id").toInt());
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

bool printCreditMemo::isSetup()
{
  return _setup;
}

void printCreditMemo::setSetup(bool pSetup)
{
  _setup = pSetup;
}

