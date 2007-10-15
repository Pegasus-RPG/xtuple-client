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

#include "userPreferences.h"

#include <QSqlError>
#include <QVariant>

#include <parameter.h>

#include "hotkeys.h"
#include "imageList.h"
#include "timeoutHandler.h"
#include "userEventNotification.h"

userPreferences::userPreferences(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
  setupUi(this);

  if(!_privleges->check("MaintainPreferencesOthers"))
    _selectedUser->setEnabled(false);

  connect(_backgroundList,SIGNAL(clicked()),     this, SLOT(sBackgroundList()));
  connect(_close,         SIGNAL(clicked()),     this, SLOT(sClose()));
  connect(_events,        SIGNAL(clicked()),     this, SLOT(sEvents()));
  connect(_hotkeys,       SIGNAL(clicked()),     this, SLOT(sHotkeys()));
  connect(_neo,           SIGNAL(toggled(bool)), this, SLOT(sStyleChanged()));
  connect(_save,          SIGNAL(clicked()),     this, SLOT(sSave()));
  connect(_selectedUser,  SIGNAL(toggled(bool)), this, SLOT(sPopulate()));
  connect(_user,          SIGNAL(newID(int)),    this, SLOT(sPopulate()));

  _dirty = FALSE;

#ifndef Q_WS_MAC
  _backgroundList->setMaximumWidth(25);
#endif

  _user->setType(XComboBox::Users);

  _userGroup->setEnabled(_privleges->check("MaintainUsers"));

  _ellipsesAction->append(1, tr("List"));
  _ellipsesAction->append(2, tr("Search"));

  sPopulate();
}

userPreferences::~userPreferences()
{
  // no need to delete child widgets, Qt does it all for us
}

void userPreferences::languageChange()
{
  retranslateUi(this);
}

void userPreferences::setBackgroundImage(int pImageid)
{
  q.prepare( "SELECT image_id, (image_name || ' - ' || image_descrip) AS description "
             "FROM image "
             "WHERE (image_id=:image_id);" );
  q.bindValue(":image_id", pImageid);
  q.exec();
  if (q.first())
  {
    _backgroundImageid = q.value("image_id").toInt();
    _background->setText(q.value("description").toString());
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void userPreferences::sBackgroundList()
{
  ParameterList params;
  params.append("image_id", _backgroundImageid);

  imageList newdlg(this, "", TRUE);
  newdlg.set(params);

  int imageid;
  if ((imageid = newdlg.exec()) != 0)
    setBackgroundImage(imageid);
}

void userPreferences::sPopulate()
{
  if (_currentUser->isChecked())
    _pref = Preferences(omfgThis->username());
  else
    _pref = Preferences(_user->currentText());

  if (_pref.value("BackgroundImageid").toInt() > 0)
  {
    _backgroundImage->setChecked(TRUE);
    setBackgroundImage(_pref.value("BackgroundImageid").toInt());
  }
  else
  {
    _noBackgroundImage->setChecked(TRUE);
    _backgroundImageid = -1;
  }

  if (_pref.value("PreferredWarehouse").toInt() == -1)
    _noWarehouse->setChecked(TRUE);
  else
  {
    _selectedWarehouse->setChecked(TRUE);
    _warehouse->setId(_pref.value("PreferredWarehouse").toInt());
  }

  if (_pref.value("InterfaceWindowOption") == "TopLevel")
    _interfaceTopLevel->setChecked(true);
  else
    _interfaceWorkspace->setChecked(true);

  _rememberCheckBoxes->setChecked(! _pref.boolean("XCheckBox/forgetful"));

  _itemCache->setChecked(_pref.boolean("UseItemCache"));
  _customerCache->setChecked(_pref.boolean("UseCustCache"));
  
  _standard->setChecked(_pref.boolean("UseOldMenu"));
  sStyleChanged();
  	
  //New Menus
  _inventoryMenu->setChecked(_pref.boolean("ShowIMMenu"));
  _productsMenu->setChecked(_pref.boolean("ShowPDMenu"));
  _scheduleMenu->setChecked(_pref.boolean("ShowMSMenu"));
  _manufactureMenu->setChecked(_pref.boolean("ShowWOMenu"));
  _crmMenu2->setChecked(_pref.boolean("ShowCRMMenu"));
  _purchaseMenu->setChecked(_pref.boolean("ShowPOMenu"));
  _salesMenu->setChecked(_pref.boolean("ShowSOMenu"));
  _accountingMenu->setChecked(_pref.boolean("ShowGLMenu"));

  _inventoryToolbar->setChecked(_pref.boolean("ShowIMToolbar"));
  _productsToolbar->setChecked(_pref.boolean("ShowPDToolbar"));
  _scheduleToolbar->setChecked(_pref.boolean("ShowMSToolbar"));
  _manufactureToolbar->setChecked(_pref.boolean("ShowWOToolbar"));
  _crmToolbar2->setChecked(_pref.boolean("ShowCRMToolbar"));
  _purchaseToolbar->setChecked(_pref.boolean("ShowPOToolbar"));
  _salesToolbar->setChecked(_pref.boolean("ShowSOToolbar"));
  _accountingToolbar->setChecked(_pref.boolean("ShowGLToolbar"));
  
  //Old Menus  
  _imMenu->setChecked(_pref.boolean("ShowIMMenu"));
  _pdMenu->setChecked(_pref.boolean("ShowPDMenu"));
  _msMenu->setChecked(_pref.boolean("ShowMSMenu"));
  _cpMenu->setChecked(_pref.boolean("ShowCPMenu"));
  _woMenu->setChecked(_pref.boolean("ShowWOMenu"));
  _crmMenu->setChecked(_pref.boolean("ShowCRMMenu"));
  _poMenu->setChecked(_pref.boolean("ShowPOMenu"));
  _soMenu->setChecked(_pref.boolean("ShowSOMenu"));
  _srMenu->setChecked(_pref.boolean("ShowSRMenu"));
  _saMenu->setChecked(_pref.boolean("ShowSAMenu"));
  _pmMenu->setChecked(_pref.boolean("ShowPMMenu"));
  _arMenu->setChecked(_pref.boolean("ShowARMenu"));
  _apMenu->setChecked(_pref.boolean("ShowAPMenu")); 
  _glMenu->setChecked(_pref.boolean("ShowGLMenu"));

  _imToolbar->setChecked(_pref.boolean("ShowIMToolbar"));
  _pdToolbar->setChecked(_pref.boolean("ShowPDToolbar"));
  _msToolbar->setChecked(_pref.boolean("ShowMSToolbar"));
  _cpToolbar->setChecked(_pref.boolean("ShowCPToolbar"));
  _woToolbar->setChecked(_pref.boolean("ShowWOToolbar"));
  _crmToolbar->setChecked(_pref.boolean("ShowCRMToolbar"));
  _poToolbar->setChecked(_pref.boolean("ShowPOToolbar"));
  _soToolbar->setChecked(_pref.boolean("ShowSOToolbar"));
  _srToolbar->setChecked(_pref.boolean("ShowSRToolbar"));
  _saToolbar->setChecked(_pref.boolean("ShowSAToolbar"));
  _pmToolbar->setChecked(_pref.boolean("ShowPMToolbar"));
  _arToolbar->setChecked(_pref.boolean("ShowARToolbar"));
  _apToolbar->setChecked(_pref.boolean("ShowAPToolbar")); 
  _glToolbar->setChecked(_pref.boolean("ShowGLToolbar"));

  _fixedWidthFonts->setChecked(_pref.boolean("UsedFixedWidthFonts"));
  _showRIWorkOrders->setChecked(_pref.boolean("ShowRIWorkOrdersByDefault"));
  _listNumericItemsFirst->setChecked(_pref.boolean("ListNumericItemNumbersFirst"));
  _showSoitemAvailability->setChecked(_pref.boolean("ShowSOItemAvailability"));

  _idleTimeout->setValue(_pref.value("IdleTimeout").toInt());

  if(_pref.value("DefaultEllipsesAction") == "search")
    _ellipsesAction->setId(2);
  else
    _ellipsesAction->setId(1);
    
  //Hide for PostBooks 
  if (_metrics->value("Application") != "OpenMFG")
  {
    _scheduleMenu->hide();
    _scheduleToolbar->hide();
    
    _msMenu->hide();
    _msToolbar->hide();
    _cpMenu->hide();
    _cpToolbar->hide();
  }
  
  if (!_metrics->boolean("MultiWhs"))
    _warehouseGroup->hide();
}

void userPreferences::sSave()
{
  if (_backgroundImage->isChecked())
    _pref.set("BackgroundImageid", _backgroundImageid);
  else
    _pref.set("BackgroundImageid", -1);
  
  _pref.set("UseOldMenu", _standard->isChecked());
  
  if (_neo->isChecked())
  {
    _pref.set("ShowIMMenu", _inventoryMenu->isChecked());
    _pref.set("ShowPDMenu", _productsMenu->isChecked());
    _pref.set("ShowMSMenu", _scheduleMenu->isChecked());
    _pref.set("ShowWOMenu", _manufactureMenu->isChecked());
    _pref.set("ShowCRMMenu", _crmMenu2->isChecked());
    _pref.set("ShowPOMenu", _purchaseMenu->isChecked());
    _pref.set("ShowSOMenu", _salesMenu->isChecked());
    _pref.set("ShowGLMenu", _accountingMenu->isChecked());
  
    _pref.set("ShowIMToolbar", _inventoryToolbar->isChecked());
    _pref.set("ShowPDToolbar", _productsToolbar->isChecked());
    _pref.set("ShowMSToolbar", _scheduleToolbar->isChecked());
    _pref.set("ShowWOToolbar", _manufactureToolbar->isChecked());
    _pref.set("ShowCRMToolbar", _crmToolbar2->isChecked());
    _pref.set("ShowPOToolbar", _purchaseToolbar->isChecked());
    _pref.set("ShowSOToolbar", _salesToolbar->isChecked());
    _pref.set("ShowGLToolbar", _accountingToolbar->isChecked());
  }
  else
  {
    _pref.set("ShowIMMenu", _imMenu->isChecked());
    _pref.set("ShowPDMenu", _pdMenu->isChecked());
    _pref.set("ShowMSMenu", _msMenu->isChecked());
    _pref.set("ShowCPMenu", _cpMenu->isChecked());
    _pref.set("ShowWOMenu", _woMenu->isChecked());
    _pref.set("ShowCRMMenu", _crmMenu->isChecked());
    _pref.set("ShowPOMenu", _poMenu->isChecked());
    _pref.set("ShowSOMenu", _soMenu->isChecked());
    _pref.set("ShowSRMenu", _srMenu->isChecked());
    _pref.set("ShowSAMenu", _saMenu->isChecked());
    _pref.set("ShowPMMenu", _pmMenu->isChecked());
    _pref.set("ShowAPMenu", _apMenu->isChecked());
    _pref.set("ShowARMenu", _arMenu->isChecked());
    _pref.set("ShowGLMenu", _glMenu->isChecked());
  
    _pref.set("ShowIMToolbar", _imToolbar->isChecked());
    _pref.set("ShowPDToolbar", _pdToolbar->isChecked());
    _pref.set("ShowMSToolbar", _msToolbar->isChecked());
    _pref.set("ShowCPToolbar", _cpToolbar->isChecked());
    _pref.set("ShowWOToolbar", _woToolbar->isChecked());
    _pref.set("ShowCRMToolbar", _crmToolbar->isChecked());
    _pref.set("ShowPOToolbar", _poToolbar->isChecked());
    _pref.set("ShowSOToolbar", _soToolbar->isChecked());
    _pref.set("ShowSRToolbar", _srToolbar->isChecked());
    _pref.set("ShowSAToolbar", _saToolbar->isChecked());
    _pref.set("ShowPMToolbar", _pmToolbar->isChecked());
    _pref.set("ShowAPToolbar", _apToolbar->isChecked());
    _pref.set("ShowARToolbar", _arToolbar->isChecked()); 
    _pref.set("ShowGLToolbar", _glToolbar->isChecked());
  }
  
  _pref.set("PreferredWarehouse", ((_noWarehouse->isChecked()) ? -1 : _warehouse->id())  );
  _pref.set("XCheckBox/forgetful", !_rememberCheckBoxes->isChecked());
  _pref.set("UseItemCache", _itemCache->isChecked());
  _pref.set("UseCustCache", _customerCache->isChecked());

  _pref.set("UsedFixedWidthFonts", _fixedWidthFonts->isChecked());
  _pref.set("ShowRIWorkOrdersByDefault", _showRIWorkOrders->isChecked());
  _pref.set("ListNumericItemNumbersFirst", _listNumericItemsFirst->isChecked());
  _pref.set("ShowSOItemAvailability", _showSoitemAvailability->isChecked());

  _pref.set("IdleTimeout", _idleTimeout->value());
  omfgThis->_timeoutHandler->setIdleMinutes(_idleTimeout->value());

  if(_ellipsesAction->id() == 2)
    _pref.set("DefaultEllipsesAction", QString("search"));
  else
    _pref.set("DefaultEllipsesAction", QString("list"));

  if(_interfaceTopLevel->isChecked())
    _pref.set("InterfaceWindowOption", QString("TopLevel"));
  else
    _pref.set("InterfaceWindowOption", QString("Workspace"));

  if (_currentUser->isChecked())
  {
    _preferences->load();
        omfgThis->initMenuBar();
  }

  accept();
}

void userPreferences::sClose()
{
  if (_dirty)
    omfgThis->initMenuBar();

  reject();
}

void userPreferences::sEvents()
{
  ParameterList params;

  if (_currentUser->isChecked())
    params.append("username", omfgThis->username());
  else
    params.append("username", _user->currentText());

  userEventNotification newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void userPreferences::sHotkeys()
{
  ParameterList params;

  if (_currentUser->isChecked())
    params.append("currentUser");
  else
    params.append("username", _user->currentText());

  hotkeys newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() == QDialog::Accepted)
    _dirty = TRUE;
}

void userPreferences::sStyleChanged()
{
  if(!_neo->isChecked())
  {
    _widgetStack->setCurrentIndex(0);
  }
  else
  {
    _widgetStack->setCurrentIndex(1);
  }
}
