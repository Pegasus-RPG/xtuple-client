/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>

#include "guiclient.h"

#include "project.h"
#include "projects.h"
#include "dspOrderActivityByProject.h"

#include "contact.h"
#include "contacts.h"
#include "searchForContact.h"
#include "address.h"
#include "addresses.h"
#include "crmaccount.h"
#include "crmaccounts.h"
#include "searchForCRMAccount.h"
#include "dspIncidentsByCRMAccount.h"
#include "dspTodoByUserAndIncident.h"
#include "incidentWorkbench.h"
#include "incident.h"
#include "todoList.h"
#include "todoListCalendar.h"
#include "todoItem.h"

#include "honorifics.h"
#include "incidentCategories.h"
#include "incidentPriorities.h"
#include "incidentSeverities.h"
#include "incidentResolutions.h"
#include "characteristics.h"

#include "opportunity.h"
#include "opportunityList.h"
#include "opportunitySources.h"
#include "opportunityStages.h"
#include "opportunityTypes.h"

#include "editOwners.h"

#include "menuCRM.h"

menuCRM::menuCRM(GUIClient *Pparent) :
  QObject(Pparent)
{
  setObjectName("crmModule");
  parent = Pparent;
  
  toolBar = new QToolBar(tr("CRM Tools"));
  toolBar->setObjectName("CRM Tools");
  toolBar->setIconSize(QSize(32, 32));
  if (_preferences->boolean("ShowCRMToolbar"))
    parent->addToolBar(toolBar);

  // Menus
  crmMenu           = new QMenu(parent);
  projectsMenu      = new QMenu(parent);
  incidentMenu      = new QMenu(parent);
  todoMenu          = new QMenu(parent);
  reportsMenu       = new QMenu(parent);
  accountsMenu      = new QMenu(parent);
  contactsMenu      = new QMenu(parent);
  addressMenu       = new QMenu(parent);
  utilitiesMenu     = new QMenu(parent);
  masterMenu        = new QMenu(parent);
  masterIncdMenu    = new QMenu(parent);
  opportunityMenu   = new QMenu(parent);
  masterOppMenu     = new QMenu(parent);

  crmMenu->setObjectName("menu.crm");
  projectsMenu->setObjectName("menu.crm.projects");
  incidentMenu->setObjectName("menu.crm.incident");
  todoMenu->setObjectName("menu.crm.todo");
  reportsMenu->setObjectName("menu.crm.reports");
  accountsMenu->setObjectName("menu.crm.accounts");
  contactsMenu->setObjectName("menu.crm.contacts");
  addressMenu->setObjectName("menu.crm.address");
  utilitiesMenu->setObjectName("menu.crm.utilities");
  masterMenu->setObjectName("menu.crm.master");
  masterIncdMenu->setObjectName("menu.crm.masterincd");
  opportunityMenu->setObjectName("menu.crm.opportunity");
  masterOppMenu->setObjectName("menu.crm.masteropp");

  actionProperties acts[] = {
    // CRM | Incident
    { "menu",			tr("&Incident"),	(char*)incidentMenu,		crmMenu,	"true", NULL, NULL, true	, NULL },
    { "crm.incident",		tr("&New..."),		SLOT(sIncident()),		incidentMenu,	"AddIncidents MaintainIncidents", NULL, NULL, true , NULL },
    { "separator",		NULL,				NULL,			incidentMenu,	"true", NULL, NULL, true	, NULL },
    { "crm.incidentWorkbench",	tr("&Workbench..."),	SLOT(sIncidentWorkbench()),	incidentMenu,	"ViewIncidents MaintainIncidents", QPixmap(":/images/incidents.png"), toolBar, true , tr("Incident Workbench") },

    // CRM / To Do
    { "menu",			tr("&To-Do"),	(char*)todoMenu,	crmMenu,	"true", NULL, NULL, true	, NULL },
    { "crm.todoItem",		tr("&New..."),	SLOT(sTodoItem()),	todoMenu,	"MaintainPersonalTodoList", NULL, NULL, true	, NULL },
    { "crm.todoList",		tr("&List..."),		SLOT(sTodoList()),	todoMenu,	"MaintainPersonalTodoList ViewPersonalTodoList", QPixmap(":/images/toDoList.png"), toolBar, true	, tr("To-Do List") },
    { "crm.todoListCalendar",		tr("&Calendar List..."),		SLOT(sTodoListCalendar()),	todoMenu,	"MaintainPersonalTodoList ViewPersonalTodoList", NULL, NULL, true, NULL},

    //  Project
    { "menu", tr("Pro&ject"), (char*)projectsMenu, crmMenu, "true", NULL, NULL, true	, NULL },
    { "pm.newProject", tr("&New..."), SLOT(sNewProject()), projectsMenu, "MaintainProjects", NULL, NULL, true , NULL },
    { "pm.projects", tr("&List..."), SLOT(sProjects()), projectsMenu, "ViewProjects", QPixmap(":/images/projects.png"), toolBar, true , tr("List Projects") },
    
    // Opportunity
    { "menu",		tr("&Opportunity"),	(char*)opportunityMenu,	crmMenu,		"true", NULL, NULL, true	, NULL },
    { "crm.newOpportunity", tr("&New..."), SLOT(sNewOpportunity()), opportunityMenu, "MaintainOpportunities", NULL, NULL, true , NULL },
    { "crm.listOpportunity", tr("&List..."), SLOT(sOpportunities()), opportunityMenu, "MaintainOpportunities ViewOpportunities", NULL, NULL, true , NULL },

    { "separator",		NULL,				NULL,			crmMenu,	"true", NULL, NULL, true	, NULL },

    // Reports
    { "menu",				tr("&Reports"),		(char*)reportsMenu,			crmMenu,	"true", NULL, NULL, true	, NULL },

    { "pm.dspOrderActivityByProject", tr("Order &Activity by Project..."), SLOT(sDspOrderActivityByProject()), reportsMenu, "ViewProjects", NULL, NULL, true , NULL },
    { "separator",		NULL,				NULL,			reportsMenu,	"true", NULL, NULL, true	, NULL },
    { "crm.dspIncidentsByCRMAccount",		tr("&Incidents by CRM Account..."),		SLOT(sDspIncidentsByCRMAccount()),		reportsMenu,	"ViewCRMAccounts+ViewIncidents+ViewOtherTodoLists", NULL, NULL, true	, NULL },
    { "crm.dspTodoByUserAndIncident",		tr("&To-Do List Items by User and Incident..."),		SLOT(sDspTodoByUserAndIncident()),		reportsMenu,	"MaintainOtherTodoLists ViewOtherTodoLists", NULL, NULL, true	, NULL },
    { "separator",		NULL,				NULL,			crmMenu,	"true", NULL, NULL, true	, NULL },
    
    // CRM | Account
    { "menu",		tr("&Account"),		(char*)accountsMenu,	crmMenu,		"true", NULL, NULL, true	, NULL },
    { "crm.crmaccount",		tr("&New..."),	SLOT(sCRMAccount()),	accountsMenu,	"MaintainCRMAccounts", NULL, NULL, true , NULL },
    { "crm.crmaccounts",	tr("&List..."),	SLOT(sCRMAccounts()),	accountsMenu,	"MaintainCRMAccounts ViewCRMAccounts", QPixmap(":/images/accounts.png"), toolBar, true , tr("List Accounts") },
    { "crm.crmaccountsearch",	tr("&Search..."),SLOT(sSearchForCRMAccount()),accountsMenu,	"MaintainCRMAccounts ViewCRMAccounts", NULL, NULL, true , NULL },
      
    // CRM | Contact
    { "menu",		tr("&Contact"),		(char*)contactsMenu,	crmMenu,		"true", NULL, NULL, true	, NULL },
    { "crm.contact",	tr("&New..."),		SLOT(sContact()),	contactsMenu,	"MaintainContacts", NULL, NULL, true	, NULL },
    { "crm.contacts",	tr("&List..."),		SLOT(sContacts()),	contactsMenu,	"MaintainContacts ViewContacts", QPixmap(":/images/contacts.png"), toolBar, true , tr("List Contacts") },
    { "crm.contactsearch",	tr("&Search..."),		SLOT(sSearchForContact()),	contactsMenu,	"MaintainContacts ViewContacts", NULL, NULL, true	, NULL },
    
    // CRM | Address
    { "menu",		tr("A&ddress"),		(char*)addressMenu,	crmMenu,		"true", NULL, NULL, true	, NULL },
    { "crm.address",	tr("&New..."),		SLOT(sAddress()),	addressMenu,	"MaintainAddresses", NULL, NULL, true	, NULL },
    { "crm.addresses",	tr("&List..."),	SLOT(sAddresses()),	addressMenu,	"MaintainAddresses ViewAddresses", NULL, NULL, true , NULL },

    { "separator",		NULL,				NULL,			crmMenu,	"true", NULL, NULL, true	, NULL },

    // Master Information
    { "menu",			tr("&Master Information"),		(char*)masterMenu,		crmMenu,	"true", NULL, NULL, true	, NULL },
    { "crm.honorifics",		tr("&Titles..."),			SLOT(sHonorifics()),		masterMenu,	"MaintainTitles ViewTitles", NULL, NULL, true	, NULL },
   
    { "menu",			tr("&Incident"),		(char*)masterIncdMenu,		masterMenu,	"true", NULL, NULL, true	, NULL },
    { "crm.incidentCategories",	tr("&Categories..."),		SLOT(sIncidentCategories()),	masterIncdMenu,	"MaintainIncidentCategories", NULL, NULL, true , NULL },
    { "crm.incidentSeverities",	tr("&Severities..."),		SLOT(sIncidentSeverities()),	masterIncdMenu,	"MaintainIncidentSeverities", NULL, NULL, true , NULL },
    { "crm.incidentResolutions", tr("&Resolutions..."),		SLOT(sIncidentResolutions()),	masterIncdMenu,	"MaintainIncidentResolutions", NULL, NULL, true , NULL },

    { "menu",			tr("&Opportunity"),		(char*)masterOppMenu,		masterMenu,	"true", NULL, NULL, true	, NULL },
    { "crm.opportunitySources",	tr("&Sources..."),		SLOT(sOpportunitySources()),	masterOppMenu,	"MaintainOpportunitySources", NULL, NULL, true , NULL },
    { "crm.opportunityStages",	tr("St&ages..."),		SLOT(sOpportunityStages()),	masterOppMenu,	"MaintainOpportunityStages", NULL, NULL, true , NULL },
    { "crm.opportunityTypes",	tr("&Types..."),		SLOT(sOpportunityTypes()),	masterOppMenu,	"MaintainOpportunityTypes", NULL, NULL, true , NULL },

    { "crm.incidentPriorities",	tr("&Priorities..."),		SLOT(sIncidentPriorities()),	masterMenu,	"MaintainIncidentPriorities", NULL, NULL, true , NULL },
    { "crm.characteristics",	tr("C&haracteristics..."),		SLOT(sCharacteristics()),	masterMenu,	"MaintainCharacteristics ViewCharacteristics", NULL, NULL, true , NULL },

    //Utilities
    { "menu",			tr("&Utilities"),		(char*)utilitiesMenu,		crmMenu,	"true", NULL, NULL, true	, NULL },
    { "crm.replaceOwner",	tr("Edit O&wners"),		SLOT(sEditOwners()),	utilitiesMenu,	"EditOwner", NULL, NULL, true, NULL }

  };

  addActionsToMenu(acts, sizeof(acts) / sizeof(acts[0]));

  parent->populateCustomMenu(crmMenu, "CRM");
  QAction * m = parent->menuBar()->addMenu(crmMenu);
  if(m)
    m->setText(tr("&CRM"));
}
  
void menuCRM::addActionsToMenu(actionProperties acts[], unsigned int numElems)
{
  QAction * m = 0;
  for (unsigned int i = 0; i < numElems; i++)
  {
    if (! acts[i].visible)
    {
      continue;
    }
    else if (acts[i].actionName == QString("menu"))
    {
      m = acts[i].menu->addMenu((QMenu*)(acts[i].slot));
      if(m)
        m->setText(acts[i].actionTitle);
    }
    else if (acts[i].actionName == QString("separator"))
    {
      acts[i].menu->addSeparator();
    }
    else if ((acts[i].toolBar != NULL) && (!acts[i].toolTip.isEmpty()))
    {
      parent->actions.append( new Action( parent,
					  acts[i].actionName,
					  acts[i].actionTitle,
					  this,
					  acts[i].slot,
					  acts[i].menu,
					  acts[i].priv,
					  (acts[i].pixmap),
					  acts[i].toolBar,
                      acts[i].toolTip) );
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
					  (acts[i].pixmap),
					  acts[i].toolBar,
                      acts[i].actionTitle) );
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

void menuCRM::sNewProject()
{
  ParameterList params;
  params.append("mode", "new");

  project newdlg(omfgThis, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuCRM::sProjects()
{
  omfgThis->handleNewWindow(new projects());
}

void menuCRM::sDspOrderActivityByProject()
{
  omfgThis->handleNewWindow(new dspOrderActivityByProject());
}

void menuCRM::sCRMAccount()
{
  ParameterList params;
  params.append("mode", "new");
  crmaccount* newdlg = new crmaccount();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuCRM::sCRMAccounts()
{
  omfgThis->handleNewWindow(new crmaccounts());
}

void menuCRM::sSearchForCRMAccount()
{
  omfgThis->handleNewWindow(new searchForCRMAccount());
}

void menuCRM::sEditOwners()
{
  omfgThis->handleNewWindow(new editOwners());
}

void menuCRM::sContact()
{
  ParameterList params;
  params.append("mode", "new");
  contact* newdlg = new contact();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuCRM::sContacts()
{
  ParameterList params;
  params.append("run");
  contacts* win = new contacts();
  win->set(params);
  omfgThis->handleNewWindow(win);
}

void menuCRM::sSearchForContact()
{
  omfgThis->handleNewWindow(new searchForContact());
}

void menuCRM::sAddress()
{
  ParameterList params;
  params.append("mode", "new");
  address* newdlg = new address();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuCRM::sAddresses()
{
  omfgThis->handleNewWindow(new addresses());
}

void menuCRM::sIncidentWorkbench()
{
  omfgThis->handleNewWindow(new incidentWorkbench());
}

void menuCRM::sIncident()
{
  ParameterList params;
  params.append("mode", "new");
  incident* newdlg = new incident();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuCRM::sTodoList()
{
  ParameterList params;
  params.append("run");
  todoList* win = new todoList();
  win->set(params);
  omfgThis->handleNewWindow(win);
}

void menuCRM::sTodoListCalendar()
{
  omfgThis->handleNewWindow(new todoListCalendar());
}

void menuCRM::sTodoItem()
{
  ParameterList params;
  params.append("mode", "new");
  todoItem* newdlg = new todoItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuCRM::sHonorifics()
{
  omfgThis->handleNewWindow(new honorifics());
}

void menuCRM::sIncidentCategories()
{
  omfgThis->handleNewWindow(new incidentCategories());
}

void menuCRM::sIncidentPriorities()
{
  omfgThis->handleNewWindow(new incidentPriorities());
}

void menuCRM::sIncidentSeverities()
{
  omfgThis->handleNewWindow(new incidentSeverities());
}

void menuCRM::sIncidentResolutions()
{
  omfgThis->handleNewWindow(new incidentResolutions());
}

void menuCRM::sCharacteristics()
{
  omfgThis->handleNewWindow(new characteristics());
}

void menuCRM::sDspIncidentsByCRMAccount()
{
  omfgThis->handleNewWindow(new dspIncidentsByCRMAccount());
}

void menuCRM::sDspTodoByUserAndIncident()
{
  omfgThis->handleNewWindow(new dspTodoByUserAndIncident());
}

void menuCRM::sOpportunitySources()
{
  omfgThis->handleNewWindow(new opportunitySources());
}

void menuCRM::sOpportunityStages()
{
  omfgThis->handleNewWindow(new opportunityStages());
}

void menuCRM::sOpportunityTypes()
{
  omfgThis->handleNewWindow(new opportunityTypes());
}

void menuCRM::sNewOpportunity()
{
  ParameterList params;
  params.append("mode", "new");

  opportunity newdlg(omfgThis, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void menuCRM::sOpportunities()
{
  ParameterList params;
  params.append("run");
  opportunityList* win = new opportunityList();
  win->set(params);
  omfgThis->handleNewWindow(win);
}
