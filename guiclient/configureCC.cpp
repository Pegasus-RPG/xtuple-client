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

#include "configureCC.h"

#include <QMessageBox>

#include "creditcardprocessor.h"

configureCC::configureCC(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  if (_metricsenc == 0)
  {
    QMessageBox::critical( this, tr("Cannot Read Configuration"),
		    tr("<p>Cannot read encrypted information from database."));
  }

  _ccDefaultBank->setType(XComboBox::ARBankAccounts);

  _ccAccept->setChecked(_metrics->boolean("CCAccept"));
  _ccTest->setChecked(_metrics->boolean("CCTest"));
  _ccValidDays->setValue(_metrics->value("CCValidDays").toInt());
  
  _ccCompany->setCurrentIndex(_ccCompany->findText(_metrics->value("CCCompany")));
  _ccWidgetStack->setCurrentIndex(_ccCompany->currentIndex());

  _ccServer->setText(_metrics->value("CCServer"));
  _ccPort->setText(_metrics->value("CCPort"));

  _ccUseProxyServer->setChecked(_metrics->boolean("CCUseProxyServer"));
  _ccProxyServer->setText(_metrics->value("CCProxyServer"));
  _ccProxyPort->setText(_metrics->value("CCProxyPort"));

  _ccDefaultBank->setId(_metrics->value("CCDefaultBank").toInt());

  _ccEncKeyName->setText(_metrics->value("CCEncKeyName"));
  _ccWinEncKey->setText(_metrics->value("CCWinEncKey"));
  _ccLinEncKey->setText(_metrics->value("CCLinEncKey"));
  _ccMacEncKey->setText(_metrics->value("CCMacEncKey"));

  _ccYPWinPathPEM->setText(_metrics->value("CCYPWinPathPEM"));
  _ccYPLinPathPEM->setText(_metrics->value("CCYPLinPathPEM"));
  _ccYPMacPathPEM->setText(_metrics->value("CCYPMacPathPEM"));
  _ccYPLinkShield->setChecked(_metrics->boolean("CCYPLinkShield"));
  _ccYPLinkShieldMax->setValue(_metrics->value("CCYPLinkShieldMax").toInt());

  _confirmPreauth->setChecked(_metrics->boolean("CCConfirmPreauth"));
  _confirmCharge->setChecked(_metrics->boolean("CCConfirmCharge"));
  _confirmChargePreauth->setChecked(_metrics->boolean("CCConfirmChargePreauth"));
  _confirmCredit->setChecked(_metrics->boolean("CCConfirmCredit"));

  _enablePreauth->setChecked(_metrics->boolean("CCEnablePreauth"));
  _enableCharge->setChecked(_metrics->boolean("CCEnableCharge"));
  _enableChargePreauth->setChecked(_metrics->boolean("CCEnableChargePreauth"));
  _enableCredit->setChecked(_metrics->boolean("CCEnableCredit"));

  _cvvRequired->setChecked(_metrics->boolean("CCRequireCVV"));
  QString str = _metrics->value("CCCVVCheck");
  if (str == "F")
    _cvvReject->setChecked(true);
  else if (str == "W")
    _cvvWarn->setChecked(true);
  else // if (str == "X")
    _cvvNone->setChecked(true);

  str = _metrics->value("CCAvsCheck");
  if (str == "F")
    _avsReject->setChecked(true);
  else if (str == "W")
    _avsWarn->setChecked(true);
  else // if (str == "X")
    _avsNone->setChecked(true);

  str = _metrics->value("CCTestResult");
  if (str == "F")
    _testsAllFail->setChecked(true);
  else if (str == "S")
    _testsSomeFail->setChecked(true);
  else // if (str == "P")
    _testsAllPass->setChecked(true);

  if (0 != _metricsenc)
  {
    _ccLogin->setText(_metricsenc->value("CCLogin"));
    _ccPassword->setText(_metricsenc->value("CCPassword"));
    _ccProxyLogin->setText(_metricsenc->value("CCProxyLogin"));
    _ccProxyPassword->setText(_metricsenc->value("CCProxyPassword"));
    _ccVSUser->setText(_metricsenc->value("CCVSUser"));
    _ccVSVendor->setText(_metricsenc->value("CCVSVendor"));
    _ccVSPartner->setText(_metricsenc->value("CCVSPartner"));
    _ccVSPassword->setText(_metricsenc->value("CCVSPassword"));
    _ccYPStoreNum->setText(_metricsenc->value("CCYPStoreNum"));
  }
  else
  {
    _ccLogin->setEnabled(false);
    _ccPassword->setEnabled(false);
    _ccProxyLogin->setEnabled(false);
    _ccProxyPassword->setEnabled(false);
    _ccVSUser->setEnabled(false);
    _ccVSVendor->setEnabled(false);
    _ccVSPartner->setEnabled(false);
    _ccVSPassword->setEnabled(false);
    _ccYPStoreNum->setEnabled(false);
  }
}

/*
 *  Destroys the object and frees any allocated resources
 */
configureCC::~configureCC()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void configureCC::languageChange()
{
    retranslateUi(this);
}

void configureCC::sSave()
{
  CreditCardProcessor *cardproc =
		  CreditCardProcessor::getProcessor(_ccCompany->currentText());
  /* qDebug("_ccCompany %s, cardproc %x",
	 _ccCompany->currentText().toAscii().data(), int(cardproc));
  */
  if (! cardproc)
  {
    QMessageBox::warning(this, tr("Error getting Credit Card Processor"),
			 tr("Internal error finding the right Credit Card "
			    "Processor. The application will save what it can "
			    "but you should re-open this window and double-"
			    "check all of the settings before continuing."));
  }
  else if (_ccServer->text() != cardproc->defaultServer(_ccTest->isChecked()) &&
      _ccPort->text().toInt() != cardproc->defaultPort(_ccTest->isChecked()) )
  {
    if (QMessageBox::question(this, tr("Invalid Credit Card Configuration?"),
			      tr("<p>This configuration does not appear "
				 "valid for %1 mode: the Server is %2 and is "
				 "expected to be %3, while the Port is %4 "
				 "and is expected to be %5. Credit Card "
				 "processing transactions may fail.<p>Do you "
				 "want to save this configuration anyway?")
				.arg((_ccTest->isChecked() ? "Test" : "Live"))
				.arg(_ccServer->text())
				.arg(cardproc->defaultServer())
				.arg(_ccPort->text())
				.arg(cardproc->defaultPort()),
			      QMessageBox::Yes,
			      QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      return;
  }
  else if (_ccServer->text() != cardproc->defaultServer(_ccTest->isChecked()))
  {
    if (QMessageBox::question(this, tr("Invalid Credit Card Configuration?"),
			      tr("<p>This configuration does not appear "
				 "valid for %1 mode: the Server is %2 and is "
				 "expected to be %3. Credit Card "
				 "processing transactions may fail.<p>Do you "
				 "want to save this configuration anyway?")
				.arg((_ccTest->isChecked() ? "Test" : "Live"))
				.arg(_ccServer->text())
				.arg(cardproc->defaultServer()),
			      QMessageBox::Yes,
			      QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      return;
  }
  else if (_ccPort->text().toInt() != cardproc->defaultPort(_ccTest->isChecked()))
  {
    if (QMessageBox::question(this, tr("Invalid Credit Card Configuration?"),
			      tr("<p>This configuration does not appear "
				 "valid for %1 mode: the Port is %2 "
				 "and is expected to be %3. Credit Card "
				 "processing transactions may fail.<p>Do you "
				 "want to save this configuration anyway?")
				.arg((_ccTest->isChecked() ? "Test" : "Live"))
				.arg(_ccPort->text())
				.arg(cardproc->defaultPort()),
			      QMessageBox::Yes,
			      QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      return;
  }

  if (_ccYPLinkShield->isChecked() && _ccYPLinkShieldMax->value() <= 0)
  {
    QMessageBox::critical(this, tr("Invalid Credit Card Configuration"),
			  tr("<p>If LinkShield is enabled then you must enter "
			     "a cutoff score between 1 and 100. Higher Numbers "
			     "indicate higher risk."));
    return;
  }

  _metrics->set("CCAccept",          _ccAccept->isChecked());
  _metrics->set("CCTest",            _ccTest->isChecked());
  _metrics->set("CCValidDays",       _ccValidDays->value());
  _metrics->set("CCCompany",         _ccCompany->currentText());
  _metrics->set("CCServer",          _ccServer->text());
  _metrics->set("CCPort",            _ccPort->text());
  _metrics->set("CCUseProxyServer",  _ccUseProxyServer->isChecked());
  _metrics->set("CCProxyServer",     _ccProxyServer->text());
  _metrics->set("CCProxyPort",       _ccProxyPort->text());
  _metrics->set("CCDefaultBank",     _ccDefaultBank->id());
  _metrics->set("CCEncKeyName",      _ccEncKeyName->text());
  _metrics->set("CCWinEncKey",       _ccWinEncKey->text());
  _metrics->set("CCLinEncKey",       _ccLinEncKey->text());
  _metrics->set("CCMacEncKey",       _ccMacEncKey->text());

  _metrics->set("CCYPWinPathPEM",    _ccYPWinPathPEM->text());
  _metrics->set("CCYPLinPathPEM",    _ccYPLinPathPEM->text());
  _metrics->set("CCYPMacPathPEM",    _ccYPMacPathPEM->text());
  _metrics->set("CCYPLinkShield",    _ccYPLinkShield->isChecked());
  _metrics->set("CCYPLinkShieldMax", _ccYPLinkShieldMax->text());

  _metrics->set("CCConfirmPreauth",       _confirmPreauth->isChecked());
  _metrics->set("CCConfirmCharge",        _confirmCharge->isChecked());
  _metrics->set("CCConfirmChargePreauth", _confirmChargePreauth->isChecked());
  _metrics->set("CCConfirmCredit",        _confirmCredit->isChecked());

  _metrics->set("CCEnablePreauth",       _enablePreauth->isChecked());
  _metrics->set("CCEnableCharge",        _enableCharge->isChecked());
  _metrics->set("CCEnableChargePreauth", _enableChargePreauth->isChecked());
  _metrics->set("CCEnableCredit",        _enableCredit->isChecked());

  _metrics->set("CCRequireCVV", _cvvRequired->isChecked());
  if(_cvvNone->isChecked())
    _metrics->set("CCCVVCheck", QString("X"));
  else if(_cvvWarn->isChecked())
    _metrics->set("CCCVVCheck", QString("W"));
  else if(_cvvReject->isChecked())
    _metrics->set("CCCVVCheck", QString("F"));

  if(_avsNone->isChecked())
    _metrics->set("CCAvsCheck", QString("X"));
  else if(_avsWarn->isChecked())
    _metrics->set("CCAvsCheck", QString("W"));
  else if(_avsReject->isChecked())
    _metrics->set("CCAvsCheck", QString("F"));

  if(_testsAllFail->isChecked())
    _metrics->set("CCTestResult", QString("F"));
  else if(_testsSomeFail->isChecked())
    _metrics->set("CCTestResult", QString("S"));
  else if(_testsAllPass->isChecked())
    _metrics->set("CCTestResult", QString("P"));

  _metrics->load();

  if (0 != _metricsenc)
  {
    _metricsenc->set("CCLogin",         _ccLogin->text());
    _metricsenc->set("CCPassword",      _ccPassword->text());
    _metricsenc->set("CCProxyLogin",    _ccProxyLogin->text());
    _metricsenc->set("CCProxyPassword", _ccProxyPassword->text());
    _metricsenc->set("CCVSUser",        _ccVSUser->text());
    _metricsenc->set("CCVSVendor",      _ccVSVendor->text());
    _metricsenc->set("CCVSPartner",     _ccVSPartner->text());
    _metricsenc->set("CCVSPassword",    _ccVSPassword->text());
    _metricsenc->set("CCYPStoreNum",    _ccYPStoreNum->text());

    _metricsenc->load();
  }

  accept();
}
