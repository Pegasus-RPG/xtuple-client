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

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a configureCC as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
configureCC::configureCC(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_ccCompany, SIGNAL(activated(int)), _ccWidgetStack, SLOT(setCurrentIndex(int)));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    init();
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


void configureCC::init()
{
  if(_metricsenc == 0)
  {
    QMessageBox::critical( this, tr("Cannot Read Configuration"), tr("Cannot read encrypted information from database."));
  }

  _ccDefaultBank->setType(XComboBox::ARBankAccounts);

  _ccAccept->setChecked(_metrics->boolean("CCAccept"));
  _ccTest->setChecked(_metrics->boolean("CCTest"));
  _ccValidDays->setValue(_metrics->value("CCValidDays").toInt());
  
  QString metric = _metrics->value("CCCompany");
  if("YourPay" == metric)
    _ccCompany->setCurrentItem(1);
  else //if("Verisign" == metric)
    _ccCompany->setCurrentItem(0);
  _ccWidgetStack->setCurrentIndex(_ccCompany->currentItem());

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
  _ccYPLinkShield->setCurrentItem((_metrics->boolean("CCYPLinkShield")?1:0));
  _ccYPLinkShieldMax->setText(_metrics->value("CCYPLinkShieldMax"));

  QString str = _metrics->value("CCConfirmTrans");
  if(str == "B")
    _ccConfirmBoth->setChecked(true);
  else if(str == "A")
    _ccConfirmAuth->setChecked(true);
  else if(str == "C")
    _ccConfirmCharge->setChecked(true);
  else // if(str == "X")
    _ccConfirmNone->setChecked(true);

  str = _metrics->value("CCAvsCheck");
  if(str == "F")
    _ccAvsCheckFull->setChecked(true);
  else if(str == "P")
    _ccAvsCheckPartial->setChecked(true);
  else if(str == "A")
    _ccAvsCheckAny->setChecked(true);
  else // if(str == "X")
    _ccAvsCheckNone->setChecked(true);

  str = _metrics->value("CCSoOptions");
  if(str == "A")
    _ccSoOptionsAuth->setChecked(true);
  else if(str == "C")
    _ccSoOptionsCharge->setChecked(true);
  else // if(str == "B")
    _ccSoOptionsBoth->setChecked(true);

  str = _metrics->value("CCYPTestResult");
  if(str == "D")
    _ccYPTestResultDuplicate->setChecked(true);
  else if(str == "X")
    _ccYPTestResultDecline->setChecked(true);
  else // if(str == "G")
    _ccYPTestResultGood->setChecked(true);

  if(0 != _metricsenc)
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

void configureCC::sSave()
{
  char *company[] = { "Verisign", "YourPay" };

  _metrics->set("CCAccept", _ccAccept->isChecked());
  _metrics->set("CCTest", _ccTest->isChecked());
  _metrics->set("CCValidDays", _ccValidDays->value());
  _metrics->set("CCCompany", QString(company[_ccCompany->currentItem()]));
  _metrics->set("CCServer", _ccServer->text());
  _metrics->set("CCPort", _ccPort->text());
  _metrics->set("CCUseProxyServer", _ccUseProxyServer->isChecked());
  _metrics->set("CCProxyServer", _ccProxyServer->text());
  _metrics->set("CCProxyPort", _ccProxyPort->text());
  _metrics->set("CCDefaultBank", _ccDefaultBank->id());
  _metrics->set("CCEncKeyName", _ccEncKeyName->text());
  _metrics->set("CCWinEncKey", _ccWinEncKey->text());
  _metrics->set("CCLinEncKey", _ccLinEncKey->text());
  _metrics->set("CCMacEncKey", _ccMacEncKey->text());
  _metrics->set("CCYPWinPathPEM", _ccYPWinPathPEM->text());
  _metrics->set("CCYPLinPathPEM", _ccYPLinPathPEM->text());
  _metrics->set("CCYPMacPathPEM", _ccYPMacPathPEM->text());
  _metrics->set("CCYPLinkShield", (_ccYPLinkShield->currentItem()==1));
  _metrics->set("CCYPLinkShieldMax", _ccYPLinkShieldMax->text());

  if(_ccConfirmNone->isChecked())
    _metrics->set("CCConfirmTrans", QString("X"));
  if(_ccConfirmAuth->isChecked())
    _metrics->set("CCConfirmTrans", QString("A"));
  if(_ccConfirmBoth->isChecked())
    _metrics->set("CCConfirmTrans", QString("B"));
  if(_ccConfirmCharge->isChecked())
    _metrics->set("CCConfirmTrans", QString("C"));

  if(_ccAvsCheckNone->isChecked())
    _metrics->set("CCAvsCheck", QString("X"));
  if(_ccAvsCheckAny->isChecked())
    _metrics->set("CCAvsCheck", QString("A"));
  if(_ccAvsCheckFull->isChecked())
    _metrics->set("CCAvsCheck", QString("F"));
  if(_ccAvsCheckPartial->isChecked())
    _metrics->set("CCAvsCheck", QString("P"));

  if(_ccSoOptionsBoth->isChecked())
    _metrics->set("CCSoOptions", QString("B"));
  if(_ccSoOptionsAuth->isChecked())
    _metrics->set("CCSoOptions", QString("A"));
  if(_ccSoOptionsCharge->isChecked())
    _metrics->set("CCSoOptions", QString("C"));

  if(_ccYPTestResultGood->isChecked())
    _metrics->set("CCYPTestResult", QString("G"));
  if(_ccYPTestResultDuplicate->isChecked())
    _metrics->set("CCYPTestResult", QString("D"));
  if(_ccYPTestResultDecline->isChecked())
    _metrics->set("CCYPTestResult", QString("X"));

  _metrics->load();

  if(0 != _metricsenc)
  {
    _metricsenc->set("CCLogin", _ccLogin->text());
    _metricsenc->set("CCPassword", _ccPassword->text());
    _metricsenc->set("CCProxyLogin", _ccProxyLogin->text());
    _metricsenc->set("CCProxyPassword", _ccProxyPassword->text());
    _metricsenc->set("CCVSUser", _ccVSUser->text());
    _metricsenc->set("CCVSVendor", _ccVSVendor->text());
    _metricsenc->set("CCVSPartner", _ccVSPartner->text());
    _metricsenc->set("CCVSPassword", _ccVSPassword->text());
    _metricsenc->set("CCYPStoreNum", _ccYPStoreNum->text());

    _metricsenc->load();
  }

  accept();
}
