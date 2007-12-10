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

#include <QAction>
#include <QApplication>
#include <QAssistantClient>
#include <QDir>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QMessageBox>
#include <QPixmap>
#include <QWorkspace>
#include <QSettings>

#include "OpenMFGGUIClient.h"

#include <parameter.h>
#include <openreports.h>
#include <../common/batchManager.h>

#include "version.h"

#include "moduleSys.h"

#include "systemMessage.h"
#include "eventManager.h"
#include "users.h"
#include "submitAction.h"
#include "userPreferences.h"
#include "errorLog.h"

#include "databaseInformation.h"
#include "configureBackup.h"
#include "configureAccountingSystem.h"

#include "images.h"
#include "reports.h"
#include "forms.h"
#include "labelForms.h"
#include "calendars.h"
#include "currencies.h"
#include "currencyConversions.h"
#include "locales.h"
#include "commentTypes.h"
#include "accountNumbers.h"
#include "ediProfiles.h"
#include "departments.h"
#include "shifts.h"
#include "customCommands.h"
#include "countries.h"

#include "fixSerial.h"
#include "importXML.h"

#include "configureIE.h"
#include "configureIM.h"
#include "configurePD.h"
#include "configureMS.h"
#include "configureWO.h"
#include "configureSO.h"
#include "configureSR.h"
#include "configurePO.h"
#include "configureAP.h"
#include "configureAR.h"
#include "configureGL.h"
#include "configureCC.h"
#include "configurePM.h"
#include "configureCRM.h"

extern QString __path;

moduleSys::moduleSys(OpenMFGGUIClient *Pparent) :
 QObject(Pparent, "sysModule")
{
  QString _appname = _metrics->value("Application");
  parent = Pparent;

  errorLogListener::initialize();

  cascade = tile = closeActive = closeAll = _rememberPos = _rememberSize = 0;
  _lastActive = 0;
  geometryMenu = 0;

  systemMenu		= new QMenu();
  configModulesMenu	= new QMenu();
  masterInfoMenu	= new QMenu();
  sysUtilsMenu		= new QMenu();
  windowMenu		= new QMenu();
  helpMenu		= new QMenu();

//  Window
  windowMenu = new QMenu();
  windowMenu->setCheckable(TRUE);

  cascade = new Action( parent, "window.cascade", tr("&Cascade"), parent->workspace(), SLOT(cascade()), windowMenu, true);
  parent->actions.append( cascade );

  tile = new Action( parent, "window.tile", tr("&Tile"), parent->workspace(), SLOT(tile()), windowMenu, true);
  parent->actions.append( tile );

  closeActive = new Action( parent, "window.closeActiveWindow", tr("Close Active Window"), this, SLOT(sCloseActive()), windowMenu, true);
  parent->actions.append( closeActive );

  closeAll = new Action( parent, "window.closeAllWindows", tr("Close All Windows"), this, SLOT(sCloseAll()), windowMenu, true);
  parent->actions.append( closeAll );

  _rememberPos = new Action( parent, "window.rememberPositionToggle", tr("Remember Position"), this, SLOT(sRememberPositionToggle()), windowMenu, true);
 _rememberPos->setCheckable(true);

 _rememberSize = new Action( parent, "window.rememberSizeToggle", tr("Remember Size"), this,SLOT(sRememberSizeToggle()), windowMenu, true);
 _rememberSize->setCheckable(true);


  parent->menuBar()->insertItem(tr("Window"),  windowMenu );
  connect(windowMenu, SIGNAL(aboutToShow()), this, SLOT(sPrepareWindowMenu()));
  connect(windowMenu, SIGNAL(aboutToHide()), this, SLOT(sHideWindowMenu()));

  actionProperties acts[] = {

    { "sys.scheduleSystemMessage",	tr("Schedule System Message..."),	SLOT(sScheduleSystemMessage()),	systemMenu,	_privleges->check("IssueSystemMessages"),	NULL,	NULL,	_metrics->boolean("EnableBatchManager")		},
    { "separator",			NULL,					NULL,				systemMenu,	true,						NULL,	NULL,	_metrics->boolean("EnableBatchManager")		},
    { "sys.eventManager",		tr("Event Manager..."),			SLOT(sEventManager()),		systemMenu,	TRUE,						NULL,	NULL,	true	},
    { "sys.batchManager",		tr("Batch Manager..."),			SLOT(sBatchManager()),		systemMenu,	TRUE,						NULL,	NULL,	_metrics->boolean("EnableBatchManager")	},
    { "sys.viewDatabaseLog",		tr("View Database Log..."),		SLOT(sErrorLog()),		systemMenu,	TRUE,						NULL,	NULL,	true	},
    { "separator",			NULL,					NULL,				systemMenu,	true,						NULL,	NULL,	true	},
    { "sys.preferences",		tr("Preferences..."),			SLOT(sPreferences()),		systemMenu,	(_privleges->check("MaintainPreferencesSelf") || _privleges->check("MaintainPreferencesOthers")),	NULL,	NULL,	true	},
    { "sys.rescanPrivileges",		tr("Rescan Privileges..."),		SLOT(sRescanPrivileges()),	systemMenu,	TRUE,						NULL,	NULL,	true	},
    { "separator",			NULL,					NULL,				systemMenu,	true,						NULL,	NULL,	true	},
    { "sys.maintainUsers",		tr("Maintain Users..."),		SLOT(sMaintainUsers()),		systemMenu,	_privleges->check("MaintainUsers"),		NULL,	NULL,	true	},
    { "separator",			NULL,					NULL,				systemMenu,	true,						NULL,	NULL,	true	},
    { "sys.scheduleServerMaintenance",	tr("Schedule Server Maintenance..."),	SLOT(sScheduleServerMaintenance()),systemMenu,	_privleges->check("MaintainServer"),		NULL,	NULL,	_metrics->boolean("EnableBatchManager")		},
    { "sys.scheduleServerBackup",	tr("Schedule Server Backup..."),	SLOT(sScheduleServerBackup()),	systemMenu,	_privleges->check("BackupServer"),		NULL,	NULL,	_metrics->boolean("EnableBatchManager")		},
    { "separator",			NULL,					NULL,				systemMenu,	true,						NULL,	NULL,	_metrics->boolean("EnableBatchManager")		},

  //  System | Configure Modules
    { "menu",			tr("Configure Modules"),(char*)configModulesMenu,systemMenu,		true,					NULL,	NULL,	true	},
    { "sys.configureIM",	tr("Configure I/M..."),	SLOT(sConfigureIM()),	configModulesMenu,	_privleges->check("ConfigureIM"),	NULL,	NULL,	true	},
    { "sys.configurePD",	tr("Configure P/D..."),	SLOT(sConfigurePD()),	configModulesMenu,	_privleges->check("ConfigurePD"),	NULL,	NULL,	true	},
    { "sys.configureMS",	tr("Configure M/S..."),	SLOT(sConfigureMS()),	configModulesMenu,	_privleges->check("ConfigureMS"),	NULL,	NULL,	 (_metrics->value("Application") == "OpenMFG")	},
    { "sys.configureWO",	tr("Configure W/O..."),	SLOT(sConfigureWO()),	configModulesMenu,	_privleges->check("ConfigureWO"),	NULL,	NULL,	true	},
    { "sys.configurePO",	tr("Configure P/O..."),	SLOT(sConfigurePO()),	configModulesMenu,	_privleges->check("ConfigurePO"),	NULL,	NULL,	true	},
    { "sys.configureSO",	tr("Configure S/O..."),	SLOT(sConfigureSO()),	configModulesMenu,	_privleges->check("ConfigureSO"),	NULL,	NULL,	true	},
    { "sys.configureSR",	tr("Configure S/R..."),	SLOT(sConfigureSR()),	configModulesMenu,	_privleges->check("ConfigureSR"),	NULL,	NULL,	true	},
    { "sys.configurePM",	tr("Configure P/M..."),	SLOT(sConfigurePM()),	configModulesMenu,	_privleges->check("ConfigurePM"),	NULL,	NULL,	true	},
    { "sys.configureAP",	tr("Configure A/P..."),	SLOT(sConfigureAP()),	configModulesMenu,	_privleges->check("ConfigureAP"),	NULL,	NULL,	true	},
    { "sys.configureAR",	tr("Configure A/R..."),	SLOT(sConfigureAR()),	configModulesMenu,	_privleges->check("ConfigureAR"),	NULL,	NULL,	true	},
    { "sys.configureGL",	tr("Configure G/L..."),	SLOT(sConfigureGL()),	configModulesMenu,	_privleges->check("ConfigureGL"),	NULL,	NULL,	true	},
    { "sys.configureCRM",	tr("Configure CRM..."),	SLOT(sConfigureCRM()),	configModulesMenu,	_privleges->check("ConfigureCRM"),	NULL,	NULL,	true	},

  //  Master Information
    { "menu",				tr("Master Information"),	(char*)masterInfoMenu,		systemMenu,	true,						NULL,	NULL,	true	},
    { "sys.databaseInformation",	tr("Database Information..."),	SLOT(sDatabaseInformation()),	masterInfoMenu,	_privleges->check("ConfigDatabaseInfo"),	NULL,	NULL,	true	},
    { "sys.configureBackup",		tr("Configure Backup..."),	SLOT(sConfigureBackup()),	masterInfoMenu,	_privleges->check("ConfigureBackupServer"),	NULL,	NULL,	_metrics->boolean("EnableBatchManager")	},
    { "sys.configureAccountingSystemInterface",
					tr("Configure Accounting System Interface..."),
									SLOT(sConfigureAccountingSystemInterface()),
													masterInfoMenu, _privleges->check("ConfigAccountingInterface"),	NULL,	NULL,
																	 _metrics->boolean("EnableExternalAccountingInterface") },
    { "separator",		NULL,			NULL,			masterInfoMenu,	true,					NULL,	NULL,	true	},
    { "sys.images",		tr("Images..."),	SLOT(sImages()),	masterInfoMenu,	_privleges->check("MaintainImages"),	NULL,	NULL,	true	},
    { "sys.reports",		tr("Reports..."),	SLOT(sReports()),	masterInfoMenu,	_privleges->check("MaintainReports"),	NULL,	NULL,	true	},
    { "sys.forms",		tr("Forms..."),		SLOT(sForms()),		masterInfoMenu,	_privleges->check("MaintainForms"),	NULL,	NULL,	true	},
    { "sys.labelForms",		tr("Label Forms..."),	SLOT(sLabelForms()),	masterInfoMenu,	_privleges->check("MaintainForms"),	NULL,	NULL,	true	},
    { "sys.calendars",		tr("Calendars..."),	SLOT(sCalendars()),	masterInfoMenu,	_privleges->check("MaintainCalendars"),	NULL,	NULL,	true	},
    { "sys.currencies",		tr("Currencies..."),	SLOT(sCurrencies()),	masterInfoMenu,	_privleges->check("CreateNewCurrency"),	NULL,	NULL,	true	},
    { "sys.exchangeRates",	tr("Exchange Rates..."),SLOT(sExchangeRates()),	masterInfoMenu,	_privleges->check("MaintainCurrencyRates") || _privleges->check("ViewCurrencyRates"),
																					NULL,	NULL,	true	},
    { "sys.configureCC",	tr("Credit Cards..."),	SLOT(sConfigureCC()),	masterInfoMenu,	_privleges->check("ConfigureCC"),					NULL,	NULL,	true	},
    { "sys.countries",		tr("Countries..."),	SLOT(sCountries()),	masterInfoMenu,	_privleges->check("MaintainCountries"),					NULL,	NULL,	true	},
    { "sys.locales",		tr("Locales..."),	SLOT(sLocales()),	masterInfoMenu,	_privleges->check("MaintainLocales"),					NULL,	NULL,	true	},
    { "sys.commentTypes",	tr("Comment Types..."),	SLOT(sCommentTypes()),	masterInfoMenu,	_privleges->check("MaintainCommentTypes"),				NULL,	NULL,	true	},
    { "sys.ediProfiles",	tr("EDI Profiles..."),	SLOT(sEDIProfiles()),	masterInfoMenu,	_privleges->check("MaintainEDIProfiles"),				NULL,	NULL,	_metrics->boolean("EnableBatchManager")	},
    { "sys.departments",	tr("Departments..."),	SLOT(sDepartments()),	masterInfoMenu,	_privleges->check("ViewDepartments") || _privleges->check("MaintainDepartments"),
																					NULL,	NULL,	true	},
    { "sys.shifts",		tr("Shifts..."),	SLOT(sShifts()),	masterInfoMenu,	_privleges->check("ViewShifts") || _privleges->check("MaintainShifts"),	NULL,	NULL,	true	},
    { "sys.configureIE",	tr("Configure Data Import and E&xport..."),	SLOT(sConfigureIE()),	 masterInfoMenu,	configureIE::userHasPriv(),		NULL,	NULL,	true },
    { "sys.customCommands",	tr("Custom Commands..."),	SLOT(sCustomCommands()), masterInfoMenu,	_privleges->check("MaintainCustomCommands"),		NULL,	NULL,	true	},

    { "menu",			tr("System Utilities"),	(char*)sysUtilsMenu,	systemMenu,	true, NULL, NULL, true	},
    { "sys.fixSerial",		tr("Serial Columns"),	SLOT(sFixSerial()),	sysUtilsMenu,	_privleges->check("FixSerial"), NULL, NULL, true	},
    { "sys.importXML",		tr("Import XML"),	SLOT(sImportXML()),	sysUtilsMenu,	importXML::userHasPriv(),	NULL, NULL, true	},

    { "separator",		NULL,				NULL,				systemMenu,	true,	NULL,	NULL,	true	},
    { "sys.printAlignmentPage",	tr("Print Alignment Page..."),	SLOT(sPrintAlignment()),	systemMenu,	TRUE,	NULL,	NULL,	true	},
    { "separator",		NULL,				NULL,				systemMenu,	true,	NULL,	NULL,	true	},
    { "sys.exitOpenMFG",	tr("Exit " + _appname + "..."),	SLOT(sExit()),			systemMenu,	TRUE,	NULL,	NULL,	true	}

  };

  addActionsToMenu(acts, sizeof(acts) / sizeof(acts[0]));
  parent->populateCustomMenu(systemMenu, "Sys");
  parent->menuBar()->insertItem(tr("S&ystem"), systemMenu);

  // Community
  communityMenu = new QMenu();
  actionProperties community[] = {
    { "community.home",        tr("xTuple.org Home"),          SLOT(sCommunityHome()),        communityMenu, true, NULL, NULL, true },
    { "community.newAccount",  tr("New xTuple.org Account"),   SLOT(sCommunityNewAccount()),  communityMenu, true, NULL, NULL, true },
    { "community.editAccount", tr("Edit xTuple.org Account"),  SLOT(sCommunityEditAccount()), communityMenu, true, NULL, NULL, true },
    { "community.forums",      tr("Browse Discussion Forums"),  SLOT(sCommunityForums()),      communityMenu, true, NULL, NULL, true },
    { "community.blogs",       tr("Browse Blogs"),              SLOT(sCommunityBlogs()),       communityMenu, true, NULL, NULL, true },
    { "community.issues",      tr("Bugs and Feature Requests"), SLOT(sCommunityIssues()),      communityMenu, true, NULL, NULL, true },
    { "community.downloads",   tr("OpenMFG.org Downloads"),     SLOT(sCommunityDownloads()),   communityMenu, true, NULL, NULL, true },
  };
  addActionsToMenu(community, sizeof(community) / sizeof(community[0]));
  parent->menuBar()->insertItem(tr("Community"), communityMenu);

  //  Help
  helpMenu = new QMenu();
  actionProperties help[] = {
    { "help.about",		tr("About..."),			SLOT(sAbout()),	helpMenu, true,	NULL, NULL, true	},
#ifndef Q_WS_MACX
    { "separator",		NULL,				NULL,		helpMenu, true,	NULL, NULL, true	},
#endif
    { "help.tableOfContents",	tr("Table of Contents..."),	SLOT(sTOC()),	helpMenu, true, NULL, NULL, true	}
  };
  addActionsToMenu(help, sizeof(help) / sizeof(help[0]));

  parent->menuBar()->insertItem(tr("&Help"),   helpMenu   );
}

void moduleSys::addActionsToMenu(actionProperties acts[], unsigned int numElems)
{
  for (unsigned int i = 0; i < numElems; i++)
  {
    if (! acts[i].visible)
    {
      continue;
    }
    else if (acts[i].actionName == QString("menu"))
    {
      acts[i].menu->insertItem(acts[i].actionTitle, (QMenu*)(acts[i].slot));
    }
    else if (acts[i].actionName == QString("separator"))
    {
      acts[i].menu->addSeparator();
    }
    else if (acts[i].toolBar != NULL)
    {
      parent->actions.append( new Action( parent,
					  acts[i].actionName,
					  acts[i].actionTitle,
					  this,
					  acts[i].slot,
					  acts[i].menu,
					  acts[i].priv,
					  *(acts[i].pixmap),
					  acts[i].toolBar) );
    }
    else
    {
      parent->actions.append( new Action( parent,
					  acts[i].actionName,
					  acts[i].actionTitle,
					  this,
					  acts[i].slot,
					  acts[i].menu,
					  acts[i].priv ) );
    }
  }
}

void moduleSys::sPrepareWindowMenu()
{
  windowMenu->clear();

  if(!omfgThis->showTopLevel())
  {
    cascade->addTo(windowMenu);
    tile->addTo(windowMenu);
  }
  closeActive->addTo(windowMenu);
  closeAll->addTo(windowMenu);

  QWidgetList windows = omfgThis->windowList();

  bool b = !windows.isEmpty();
  cascade->setEnabled(b);
  tile->setEnabled(b);
  closeActive->setEnabled(b);
  closeAll->setEnabled(b);

  windowMenu->insertSeparator();

  QWidget * activeWindow = parent->workspace()->activeWindow();
  if(omfgThis->showTopLevel())
    activeWindow = qApp->activeWindow();
  _lastActive = activeWindow;

  if(activeWindow)
  {
    if(!geometryMenu)
      geometryMenu = new QMenu();

    geometryMenu->clear();
    geometryMenu->setTitle(activeWindow->caption());

    QSettings settings(QSettings::UserScope, "OpenMFG.com", "OpenMFG");
    QString objName = activeWindow->objectName();

    _rememberPos->setChecked(settings.value(objName + "/geometry/rememberPos", true).toBool());
    _rememberPos->addTo(geometryMenu);
    _rememberSize->setChecked(settings.value(objName + "/geometry/rememberSize", true).toBool());
    _rememberSize->addTo(geometryMenu);

    windowMenu->addMenu(geometryMenu);
    windowMenu->insertSeparator();
  }

  for (int cursor = 0; cursor < windows.count(); cursor++)
  {
    int menuId = windowMenu->insertItem(windows.at(cursor)->caption(), this, SLOT(sActivateWindow(int)));
    windowMenu->setItemParameter(menuId, cursor);
    windowMenu->setItemChecked(menuId, (activeWindow == windows.at(cursor)));
  }
}

void moduleSys::sHideWindowMenu()
{
  cascade->setEnabled(true);
  tile->setEnabled(true);
  closeActive->setEnabled(true);
  closeAll->setEnabled(true);
}

void moduleSys::sActivateWindow(int intWindowid)
{
  QWidgetList windows = parent->workspace()->windowList();
  if(omfgThis->showTopLevel())
    windows = omfgThis->windowList();
  QWidget *window = windows.at(intWindowid);
  if (window)
    if(omfgThis->showTopLevel())
      window->activateWindow();
    else
      window->setFocus();
}

void moduleSys::sRememberPositionToggle()
{
  if(!_lastActive)
    return;

  QSettings settings(QSettings::UserScope, "OpenMFG.com", "OpenMFG");
  QString objName = _lastActive->objectName();
  settings.setValue(objName + "/geometry/rememberPos", _rememberPos->isChecked());
}

void moduleSys::sRememberSizeToggle()
{
  if(!_lastActive)
    return;

  QSettings settings(QSettings::UserScope, "OpenMFG.com", "OpenMFG");
  QString objName = _lastActive->objectName();
  settings.setValue(objName + "/geometry/rememberSize", _rememberSize->isChecked());
}

void moduleSys::sCloseAll()
{
  if(omfgThis->showTopLevel())
  {
    foreach(QWidget * w, omfgThis->windowList())
    {
      w->close();
    }
  }
  else
    parent->workspace()->closeAllWindows();
}

void moduleSys::sCloseActive()
{
  if(omfgThis->showTopLevel())
  {
    if(omfgThis->windowList().contains(qApp->activeWindow()))
      qApp->activeWindow()->close();
  }
  else
    parent->workspace()->closeActiveWindow();
}

void moduleSys::sScheduleSystemMessage()
{
  ParameterList params;
  params.append("mode", "new");

  systemMessage newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleSys::sEventManager()
{
  omfgThis->handleNewWindow(new eventManager());
}

void moduleSys::sBatchManager()
{
  batchManager *newdlg = new batchManager();
  newdlg->setViewOtherEvents(_privleges->check("ViewOtherEvents"));
  omfgThis->handleNewWindow(newdlg);
}

void moduleSys::sPreferences()
{
  userPreferences(parent, "", TRUE).exec();
}

void moduleSys::sRescanPrivileges()
{
  _privleges->load();
  omfgThis->saveToolbarPositions();
  _preferences->load();
  omfgThis->initMenuBar();
}

void moduleSys::sDatabaseInformation()
{
  databaseInformation(parent, "", TRUE).exec();
}

void moduleSys::sConfigureBackup()
{
  configureBackup(parent, "", TRUE).exec();
}

void moduleSys::sImages()
{
  omfgThis->handleNewWindow(new images());
}

void moduleSys::sReports()
{
  omfgThis->handleNewWindow(new reports());
}

void moduleSys::sForms()
{
  omfgThis->handleNewWindow(new forms());
}

void moduleSys::sLabelForms()
{
  omfgThis->handleNewWindow(new labelForms());
}

void moduleSys::sCalendars()
{
  omfgThis->handleNewWindow(new calendars());
}

void moduleSys::sCurrencies()
{
  omfgThis->handleNewWindow(new currencies());
}

void moduleSys::sExchangeRates()
{
  omfgThis->handleNewWindow(new currencyConversions());
}

void moduleSys::sCountries()
{
  omfgThis->handleNewWindow(new countries());
}

void moduleSys::sLocales()
{
  omfgThis->handleNewWindow(new locales());
}

void moduleSys::sCommentTypes()
{
  omfgThis->handleNewWindow(new commentTypes());
}

void moduleSys::sAccountNumbers()
{
  omfgThis->handleNewWindow(new accountNumbers());
}

void moduleSys::sEDIProfiles()
{
  omfgThis->handleNewWindow(new ediProfiles());
}

void moduleSys::sDepartments()
{
  omfgThis->handleNewWindow(new departments());
}

void moduleSys::sShifts()
{
  omfgThis->handleNewWindow(new shifts());
}

void moduleSys::sCustomCommands()
{
  omfgThis->handleNewWindow(new customCommands());
}

void moduleSys::sConfigureIE()
{
  configureIE(parent, "", TRUE).exec();
}

void moduleSys::sConfigureIM()
{
  configureIM(parent, "", TRUE).exec();
}

void moduleSys::sConfigurePD()
{
  configurePD(parent, "", TRUE).exec();
}

void moduleSys::sConfigureMS()
{
  configureMS(parent, "", TRUE).exec();
}

void moduleSys::sConfigureWO()
{
  configureWO(parent, "", TRUE).exec();
}

void moduleSys::sConfigurePO()
{
  configurePO(parent, "", TRUE).exec();
}

void moduleSys::sConfigureSO()
{
  configureSO(parent, "", TRUE).exec();
}

void moduleSys::sConfigureSR()
{
  configureSR(parent, "", TRUE).exec();
}

void moduleSys::sConfigureAP()
{
  configureAP(parent, "", TRUE).exec();
}

void moduleSys::sConfigureAR()
{
  configureAR(parent, "", TRUE).exec();
}

void moduleSys::sConfigureGL()
{
  configureGL(parent, "", TRUE).exec();
}

void moduleSys::sConfigureCC()
{
  configureCC(parent, "", TRUE).exec();
}

void moduleSys::sConfigurePM()
{
  configurePM(parent, "", TRUE).exec();
}

void moduleSys::sConfigureCRM()
{
  configureCRM(parent, "", TRUE).exec();
}

void moduleSys::sMaintainUsers()
{
  omfgThis->handleNewWindow(new users());
}

void moduleSys::sScheduleServerMaintenance()
{
  ParameterList params;
  params.append("action_name", "ServerMaintenance");

  submitAction newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleSys::sScheduleServerBackup()
{
  ParameterList params;
  params.append("action_name", "ServerBackup");

  submitAction newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleSys::sErrorLog()
{
  omfgThis->handleNewWindow(new errorLog());
}

void moduleSys::sPrintAlignment()
{
  orReport report("Alignment");
  if (report.isValid())
    report.print();
  else
    report.reportError(omfgThis);
}

void moduleSys::sExit()
{
  parent->close();
}

void moduleSys::sAbout()
{
  QMessageBox::about( parent, tr("About..."),
                      tr( "%1\nVersion %2\n%3")
                          .arg(_Name)
                          .arg(_Version)
                          .arg(_Copyright) );
}

void moduleSys::sTOC()
{
  parent->_assClient->openAssistant();
}

// START_RW
void moduleSys::sConfigureAccountingSystemInterface()
{
  configureAccountingSystem(parent, "", TRUE).exec();
}
// END_RW

void moduleSys::sFixSerial()
{
  omfgThis->handleNewWindow(new fixSerial());
}

void moduleSys::sImportXML()
{
  omfgThis->handleNewWindow(new importXML());
}

void moduleSys::sCommunityHome()
{
  omfgThis->launchBrowser(omfgThis, "http://www.xtuple.org/");
}

void moduleSys::sCommunityNewAccount()
{
  omfgThis->launchBrowser(omfgThis, "http://www.xtuple.org/index.php?option=com_registration&task=register");
}

void moduleSys::sCommunityEditAccount()
{
  omfgThis->launchBrowser(omfgThis, "http://www.xtuple.org/index.php?option=com_user&task=UserDetails&Itemid=21");
}

void moduleSys::sCommunityForums()
{
  omfgThis->launchBrowser(omfgThis, "http://www.xtuple.org/phpBB2/");
}

void moduleSys::sCommunityBlogs()
{
  omfgThis->launchBrowser(omfgThis, "http://www.xtuple.org/index.php?option=com_content&task=blogsection&id=0&Itemid=9");
}

void moduleSys::sCommunityIssues()
{
  omfgThis->launchBrowser(omfgThis, "http://www.xtuple.org/index.php?option=com_mantis&Itemid=26");
}

void moduleSys::sCommunityDownloads()
{
  omfgThis->launchBrowser(omfgThis, "http://www.xtuple.org/index.php?option=com_docman&Itemid=50");
}

