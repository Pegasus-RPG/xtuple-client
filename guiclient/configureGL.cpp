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

#include "configureGL.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>

#include "configureEncryption.h"
#include "guiclient.h"

configureGL::configureGL(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  // AP
  _nextAPMemoNumber->setValidator(omfgThis->orderVal());
  q.exec("SELECT currentAPMemoNumber() AS result;");
  if (q.first())
    _nextAPMemoNumber->setText(q.value("result"));

  _achGroup->setVisible(_metrics->boolean("ACHSupported"));
  if (_metrics->boolean("ACHSupported"))
  {
    _achGroup->setChecked(_metrics->boolean("ACHEnabled"));
    _nextACHBatchNumber->setValidator(omfgThis->orderVal());
    if (! _metrics->value("ACHCompanyId").trimmed().isEmpty())
      _companyId->setText(_metrics->value("ACHCompanyId"));
    if (! _metrics->value("ACHCompanyIdType").trimmed().isEmpty())
    {
      if (_metrics->value("ACHCompanyIdType").trimmed() == "D")
        _companyIdIsDUNS->setChecked(true);
      else if (_metrics->value("ACHCompanyIdType").trimmed() == "E")
        _companyIdIsEIN->setChecked(true);
      else if (_metrics->value("ACHCompanyIdType").trimmed() == "O")
        _companyIdIsOther->setChecked(true);
    }
    if (! _metrics->value("ACHCompanyName").trimmed().isEmpty())
      _companyName->setText(_metrics->value("ACHCompanyName"));
    if (_metrics->value("ACHDefaultSuffix").trimmed().isEmpty())
      _achSuffix->setCurrentIndex(_achSuffix->findText(".dat"));
    else
    {
      int suffixidx = _achSuffix->findText(_metrics->value("ACHDefaultSuffix"));
      if (suffixidx < 0)
      {
        _achSuffix->insertItem(0, _metrics->value("ACHDefaultSuffix"));
        _achSuffix->setCurrentIndex(0);
      }
      else
        _achSuffix->setCurrentIndex(suffixidx);
    }
    q.exec("SELECT currentNumber('ACHBatch') AS result;");
    if (q.first())
      _nextACHBatchNumber->setText(q.value("result"));
  }
    
  // AR
  _nextARMemoNumber->setValidator(omfgThis->orderVal());

  q.exec("SELECT currentARMemoNumber() AS result;");
  if (q.first())
    _nextARMemoNumber->setText(q.value("result"));
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  _hideApplyto->setChecked(_metrics->boolean("HideApplyToBalance"));
  _customerDeposits->setChecked(_metrics->boolean("EnableCustomerDeposits"));
  
  _name->setText(_metrics->value("remitto_name"));
  _address->setLine1(_metrics->value("remitto_address1"));
  _address->setLine2(_metrics->value("remitto_address2"));
  _address->setLine3(_metrics->value("remitto_address3"));
  _address->setCity(_metrics->value("remitto_city"));
  _address->setState(_metrics->value("remitto_state"));
  _address->setPostalCode(_metrics->value("remitto_zipcode"));
  _address->setCountry(_metrics->value("remitto_country"));
  _phone->setText(_metrics->value("remitto_phone"));

  _warnLate->setChecked(_metrics->boolean("AutoCreditWarnLateCustomers"));
  if(!_metrics->value("DefaultAutoCreditWarnGraceDays").isEmpty())
    _graceDays->setValue(_metrics->value("DefaultAutoCreditWarnGraceDays").toInt());
  _incdtCategory->setId(_metrics->value("DefaultARIncidentStatus").toInt());
  _closeARIncdt->setChecked(_metrics->boolean("AutoCloseARIncident"));
  
  // GL
  _mainSize->setValue(_metrics->value("GLMainSize").toInt());

  bool extConsolAllowed = _metrics->value("Application") == "OpenMFG" ||
                          _metrics->value("Application") == "xTupleERP";
  _externalConsolidation->setVisible(extConsolAllowed);
  if (_metrics->value("GLCompanySize").toInt() == 0)
  {
    _useCompanySegment->setChecked(FALSE);
    _externalConsolidation->setChecked(FALSE);
  }
  else
  {
    _useCompanySegment->setChecked(TRUE);
    _companySegmentSize->setValue(_metrics->value("GLCompanySize").toInt());

    _externalConsolidation->setChecked(_metrics->boolean("MultiCompanyFinancialConsolidation") &&
                                       extConsolAllowed);
  }

  if (_metrics->value("GLProfitSize").toInt() == 0)
    _useProfitCenters->setChecked(FALSE);
  else
  {
    _useProfitCenters->setChecked(TRUE);
    _profitCenterSize->setValue(_metrics->value("GLProfitSize").toInt());
    _ffProfitCenters->setChecked(_metrics->boolean("GLFFProfitCenters"));
  }

  if (_metrics->value("GLSubaccountSize").toInt() == 0)
    _useSubaccounts->setChecked(FALSE);
  else
  {
    _useSubaccounts->setChecked(TRUE);
    _subaccountSize->setValue(_metrics->value("GLSubaccountSize").toInt());
    _ffSubaccounts->setChecked(_metrics->boolean("GLFFSubaccounts"));
  }

  _yearend->setId(_metrics->value("YearEndEquityAccount").toInt());

  _gainLoss->setId(_metrics->value("CurrencyGainLossAccount").toInt());
  switch(_metrics->value("CurrencyExchangeSense").toInt())
  {
    case 1:
      _localToBase->setChecked(TRUE);
      break;
    case 0:
    default:
      _baseToLocal->setChecked(TRUE);
  }

  _discrepancy->setId(_metrics->value("GLSeriesDiscrepancyAccount").toInt());

  _mandatoryNotes->setChecked(_metrics->boolean("MandatoryGLEntryNotes"));
  _manualGlaccnt->setChecked(_metrics->boolean("AllowManualGLAccountEntry"));
  _taxauth->setId(_metrics->value("DefaultTaxAuthority").toInt());

  _recurringBuffer->setValue(_metrics->value("RecurringInvoiceBuffer").toInt());
  
  adjustSize();
}

configureGL::~configureGL()
{
  // no need to delete child widgets, Qt does it all for us
}

void configureGL::languageChange()
{
  retranslateUi(this);
}

void configureGL::sSave()
{
  if (_metrics->boolean("ACHSupported"))
  {
    QString tmpCompanyId = _companyId->text();
    struct {
      bool    condition;
      QString msg;
      QWidget *widget;
    } error[] = {
      { _companyId->text().isEmpty(),
        tr("Please enter a default Company Id if you are going to create "
           "ACH files."),
        _companyId },
      { (_companyIdIsEIN->isChecked() || _companyIdIsDUNS->isChecked()) && 
        tmpCompanyId.remove("-").size() != 9,
        tr("EIN, TIN, and DUNS numbers are all 9 digit numbers. Other "
           "characters (except dashes for readability) are not allowed."),
        _companyId },
      { _companyIdIsOther->isChecked() && _companyId->text().size() > 10,
        tr("Company Ids must be 10 characters or shorter (not counting dashes "
           "in EIN's, TIN's, and DUNS numbers)."),
        _companyId },
      { ! (_companyIdIsEIN->isChecked() || _companyIdIsDUNS->isChecked() ||
           _companyIdIsOther->isChecked()),
        tr("Please mark whether the Company Id is an EIN, TIN, DUNS number, "
           "or Other."),
        _companyIdIsEIN }
    };
    for (unsigned int i = 0; i < sizeof(error) / sizeof(error[0]); i++)
      if (error[i].condition)
      {
        QMessageBox::critical(this, tr("Cannot Save Accounting Configuration"),
                              error[i].msg);
        error[i].widget->setFocus();
        return;
      }
  }

  // AP
  q.prepare("SELECT setNextAPMemoNumber(:armemo_number) AS result;");
  q.bindValue(":armemo_number", _nextAPMemoNumber->text().toInt());
  q.exec();

  if (_metrics->boolean("ACHSupported"))
  {
    _metrics->set("ACHEnabled",           _achGroup->isChecked());
    if (_achGroup->isChecked())
    {
      _metrics->set("ACHCompanyId",     _companyId->text().trimmed());
      if (_companyId->text().trimmed().length() > 0)
      {
        if (_companyIdIsDUNS->isChecked())
          _metrics->set("ACHCompanyIdType", QString("D"));
        else if (_companyIdIsEIN->isChecked())
          _metrics->set("ACHCompanyIdType", QString("E"));
        else if (_companyIdIsOther->isChecked())
          _metrics->set("ACHCompanyIdType", QString("O"));
      }
      _metrics->set("ACHCompanyName",   _companyName->text().trimmed());
      _metrics->set("ACHDefaultSuffix", _achSuffix->currentText().trimmed());
      q.prepare("SELECT setNextNumber('ACHBatch', :number) AS result;");
      q.bindValue(":number", _nextACHBatchNumber->text().toInt());
      q.exec();
    }
  }
  
  // AR
  q.prepare("SELECT setNextARMemoNumber(:armemo_number) AS result;");
  q.bindValue(":armemo_number", _nextARMemoNumber->text().toInt());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  _metrics->set("HideApplyToBalance", _hideApplyto->isChecked());
  _metrics->set("EnableCustomerDeposits", _customerDeposits->isChecked());

  _metrics->set("remitto_name", 	_name->text().trimmed());
  _metrics->set("remitto_address1",	_address->line1().trimmed());
  _metrics->set("remitto_address2",	_address->line2().trimmed());
  _metrics->set("remitto_address3",	_address->line3().trimmed());
  _metrics->set("remitto_city",		_address->city().trimmed());
  _metrics->set("remitto_state",	_address->state().trimmed());
  _metrics->set("remitto_zipcode",	_address->postalCode().trimmed());
  _metrics->set("remitto_country",	_address->country().trimmed());
  _metrics->set("remitto_phone",	_phone->text().trimmed());
  
  _metrics->set("AutoCreditWarnLateCustomers", _warnLate->isChecked());
  if(_warnLate->isChecked())
    _metrics->set("DefaultAutoCreditWarnGraceDays", _graceDays->value());

  _metrics->set("RecurringInvoiceBuffer", _recurringBuffer->value());
  _metrics->set("DefaultARIncidentStatus", _incdtCategory->id());
  _metrics->set("AutoCloseARIncident", _closeARIncdt->isChecked());
  
  // GL
  Action *profitcenter = 0;
  Action *subaccounts  = 0;
  Action *companyseg   = 0;
  for (ActionSet::iterator action = omfgThis->actions.begin(); action != omfgThis->actions.end(); action++)
  {
    if((*action)->name() == "gl.companies")
      companyseg = *action;
    if((*action)->name() == "gl.profitCenterNumber")
      profitcenter = *action;
    if((*action)->name() == "gl.subaccountNumbers")
      subaccounts = *action;
  }

  _metrics->set("GLMainSize", _mainSize->value());

  if (_useCompanySegment->isChecked())
  {
    _metrics->set("GLCompanySize", _companySegmentSize->value());
    _metrics->set("MultiCompanyFinancialConsolidation", _externalConsolidation->isChecked());
  }
  else
  {
    _metrics->set("GLCompanySize", 0);
    _metrics->set("MultiCompanyFinancialConsolidation", 0);
  }
  if(companyseg)
    companyseg->setEnabled(_useCompanySegment->isChecked());

  if (_useProfitCenters->isChecked())
  {
    _metrics->set("GLProfitSize", _profitCenterSize->value());
    _metrics->set("GLFFProfitCenters", _ffProfitCenters->isChecked());
    if(profitcenter)
      profitcenter->setEnabled(_privileges->check("MaintainChartOfAccounts"));
  }
  else
  {
    _metrics->set("GLProfitSize", 0);
    _metrics->set("GLFFProfitCenters", FALSE);
    if(profitcenter)
      profitcenter->setEnabled(FALSE);
  }

  if (_useSubaccounts->isChecked())
  {
    _metrics->set("GLSubaccountSize", _subaccountSize->value());
    _metrics->set("GLFFSubaccounts", _ffSubaccounts->isChecked());
    if(subaccounts)
      subaccounts->setEnabled(_privileges->check("MaintainChartOfAccounts"));

  }
  else
  {
    _metrics->set("GLSubaccountSize", 0);
    _metrics->set("GLFFSubaccounts", FALSE);
    if(subaccounts)
      subaccounts->setEnabled(FALSE);
  }

  _metrics->set("YearEndEquityAccount", _yearend->id());

  //if (! omfgThis->singleCurrency())
  //{
      _metrics->set("CurrencyGainLossAccount", _gainLoss->id());
      if(_localToBase->isChecked())
        _metrics->set("CurrencyExchangeSense", 1);
      else // if(_baseToLocal->isChecked())
        _metrics->set("CurrencyExchangeSense", 0);
  //}

  _metrics->set("GLSeriesDiscrepancyAccount", _discrepancy->id());
  _metrics->set("MandatoryGLEntryNotes", _mandatoryNotes->isChecked());
  _metrics->set("AllowManualGLAccountEntry", _manualGlaccnt->isChecked());
  _metrics->set("DefaultTaxAuthority", _taxauth->id());

  _metrics->load();

  omfgThis->sConfigureGLUpdated();

  if (_metrics->boolean("ACHSupported") && _metrics->boolean("ACHEnabled") &&
      omfgThis->_key.isEmpty())
  {
    if (_privileges->check("ConfigureEncryption"))
    {
      if (QMessageBox::question(this, tr("Set Encryption?"),
                                tr("Your encryption key is not set. You will "
                                   "not be able to configure electronic "
                                   "checking information for Vendors until you "
                                   "configure encryption. Would you like to do "
                                   "this now?"),
                                    QMessageBox::Yes | QMessageBox::Default,
                                    QMessageBox::No ) == QMessageBox::Yes)
        configureEncryption(this, "", TRUE).exec();
    }
    else
      QMessageBox::question(this, tr("Set Encryption?"),
                            tr("Your encryption key is not set. You will "
                               "not be able to configure electronic "
                               "checking information for Vendors until the "
                               "system is configured to perform encryption."));
  }

  accept();
}
