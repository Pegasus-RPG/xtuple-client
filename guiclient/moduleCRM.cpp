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

//  moduleCRM.cpp
//  Created 09/05/2006 GJM
//  Copyright (c) 2006-2008, OpenMFG, LLC

#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>

#include "guiclient.h"

#include "contact.h"
#include "contacts.h"
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
#include "todoItem.h"
/*
#include "todoLists.h"
#include "warrTerms.h"
#include "returnOrder.h"
#include "returnOrders.h"
#include "purgeAddresses.h"
#include "purgeContacts.h"
#include "purgeAccounts.h"
#include "purgeTodoLists.h"
#include "dspContactsByAddress.h"
#include "dspCRMAccountHierarchy.h"
#include "dspIncidentChangesByDate.h"
#include "dspTodoListItemsByDate.h"
#include "dspExpiringWarrantiesByAccountRelationship.h"
#include "dspReturnRepairHistoryByCustomer.h"
#include "dspReturnRepairHistoryByItem.h"
#include "dspReturnRepairHistoryByParameterGroup.h"
#include "dspLotSerialByWorkOrder.h"
#include "dspLotSerialBySalesOrder.h"
#include "dspWorkOrderByLotSerial.h"
#include "dspSalesOrderByLotSerial.h"
*/
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

#include "moduleCRM.h"

moduleCRM::moduleCRM(GUIClient *Pparent) :
  QObject(Pparent, "crmModule")
{
  parent = Pparent;
  
  toolBar = new QToolBar(tr("CRM Tools"));
  toolBar->setObjectName("CRM Tools");
  toolBar->setIconSize(QSize(32, 32));
  if (_preferences->boolean("ShowCRMToolbar"))
    parent->addToolBar(toolBar);

  // Menus
  crmMenu		= new QMenu();
  addressBookMenu	= new QMenu();
  incidentMenu		= new QMenu();
  todoMenu		= new QMenu();
  displaysMenu		= new QMenu();
  utilitiesMenu		= new QMenu();
  masterMenu		= new QMenu();
  opportunityMenu	= new QMenu();

  struct actionProperties {
    const char*		actionName;
    const QString	actionTitle;
    const char*		slot;
    QMenu*		menu;
    bool		priv;
    QPixmap*		pixmap;
    QToolBar*		toolBar;
    bool		visible;
  };

  actionProperties acts[] = {
    { "menu",		tr("&Address Book"),		(char*)addressBookMenu,	crmMenu,		true, NULL, NULL, true	},
    { "crm.crmaccount",		tr("New CRM Account..."),	SLOT(sCRMAccount()),	addressBookMenu,	_privileges->check("MaintainCRMAccounts"), NULL, NULL, true },
    { "crm.crmaccountsearch",	tr("Search for CRM Account..."),SLOT(sSearchForCRMAccount()),addressBookMenu,	_privileges->check("MaintainCRMAccounts") || _privileges->check("ViewCRMAccounts"), NULL, NULL, true },
    { "crm.crmaccounts",	tr("List CRM Accounts..."),	SLOT(sCRMAccounts()),	addressBookMenu,	_privileges->check("MaintainCRMAccounts") || _privileges->check("ViewCRMAccounts"),new QPixmap(":/images/accounts.png"), toolBar, true },
    { "separator",		NULL,				NULL,			addressBookMenu,	true, NULL, NULL, true	},
    { "crm.contact",	tr("New Contact..."),		SLOT(sContact()),	addressBookMenu,	_privileges->check("MaintainContacts"), NULL, NULL, true	},
    { "crm.contacts",	tr("List Contacts..."),		SLOT(sContacts()),	addressBookMenu,	_privileges->check("MaintainContacts") || _privileges->check("ViewContacts"),new QPixmap(":/images/contacts.png"), toolBar, true },
    { "crm.address",	tr("New Address..."),		SLOT(sAddress()),	addressBookMenu,	_privileges->check("MaintainAddresses"), NULL, NULL, true	},
    { "crm.addresses",	tr("List Addresses..."),	SLOT(sAddresses()),	addressBookMenu,	_privileges->check("MaintainAddresses") || _privileges->check("ViewAddresses"), NULL, NULL, true },

    { "menu",			tr("&Incident Management"),	(char*)incidentMenu,		crmMenu,	true, NULL, NULL, true	},
    { "crm.incidentWorkbench",	tr("Incident Workbench..."),	SLOT(sIncidentWorkbench()),	incidentMenu,	_privileges->check("ViewIncidents") || _privileges->check("MaintainIncidents"), new QPixmap(":/images/incidents.png"), toolBar, true },
    { "crm.incident",		tr("New Incident..."),		SLOT(sIncident()),		incidentMenu,	_privileges->check("AddIncidents") || _privileges->check("MaintainIncidents"), NULL, NULL, true },

    { "menu",			tr("&To-Do List Management"),	(char*)todoMenu,	crmMenu,	true, NULL, NULL, true	},
    { "crm.todoList",		tr("To-Do List..."),		SLOT(sTodoList()),	todoMenu,	_privileges->check("MaintainPersonalTodoList") || _privileges->check("ViewPersonalTodoList"),new QPixmap(":/images/toDoList.png"), toolBar, true	},
    { "crm.todoItem",		tr("New To-Do List Item..."),	SLOT(sTodoItem()),	todoMenu,	_privileges->check("MaintainPersonalTodoList"), NULL, NULL, true	},
    /*
    { "crm.todoLists",		tr("To-Do Lists..."),		SLOT(sTodoLists()),	todoMenu,	_privileges->check("MaintainOtherTodoLists") || _privileges->check("ViewOtherTodoLists"), NULL, NULL, true	},
    */
    /*
    { "crm.rma",		tr("New Return Material Authorization..."),	SLOT(sRma()),			crmMenu,	_privileges->check(""), NULL, NULL, true	},
    { "crm.rmas",		tr("List Return Material Authorizations..."),	SLOT(sRmas()),			crmMenu,	_privileges->check(""), NULL, NULL, true	},
    */

    { "menu",		     tr("&Opportunity"),	(char*)opportunityMenu,  crmMenu,	  true, NULL, NULL, true	},
    { "crm.newOpportunity",  tr("&New Opportunity"),	SLOT(sNewOpportunity()), opportunityMenu, _privileges->check("MaintainOpportunities"), NULL, NULL, true },
    { "crm.listOpportunity", tr("&List Opportunities"), SLOT(sOpportunities()),  opportunityMenu, (_privileges->check("MaintainOpportunities") || _privileges->check("ViewOpportunities")), NULL, NULL, true },

    { "menu",				tr("&Displays"),		(char*)displaysMenu,			crmMenu,	true, NULL, NULL, true	},
    /*
    { "crm.dspAddressBook",		tr("Address Book..."),		SLOT(sDspAddressBook()),		displaysMenu,	_privileges->check(""), NULL, NULL, true	},
    { "crm.dspContactsByAddress",	tr("Contacts By Address..."),	SLOT(sDspContactsByAddress()),		displaysMenu,	_privileges->check(""), NULL, NULL, true	},
    { "crm.dspCRMAccountHierarchy",	tr("CRM Account Hierarchies..."),	SLOT(sDspCRMAccountHierarchy()),		displaysMenu,	_privileges->check("MaintainCRMAccounts") || _privileges->check("ViewCRMAccounts"), NULL, NULL, true	},
    { "separator",			NULL,				NULL,					displaysMenu,	true		, NULL, NULL, true	},
    */
    { "crm.dspIncidentsByCRMAccount",		tr("Incidents by CRM Account..."),		SLOT(sDspIncidentsByCRMAccount()),		displaysMenu,	_privileges->check("ViewCRMAccounts") && _privileges->check("ViewIncidents") && _privileges->check("ViewOtherTodoLists"), NULL, NULL, true	},
    { "crm.dspTodoByUserAndIncident",		tr("To-Do List Items by User and Incident..."),		SLOT(sDspTodoByUserAndIncident()),		displaysMenu,	_privileges->check("MaintainOtherTodoLists") || _privileges->check("ViewOtherTodoLists"), NULL, NULL, true	},
    /*
    { "crm.dspIncidentChangesByDate",	tr("Incident Changes..."),	SLOT(sDspIncidentChangesByDate()),	displaysMenu,	_privileges->check(""), NULL, NULL, true	},
    { "separator",			NULL,				NULL,					displaysMenu,	true		, NULL, NULL, true	},
    { "crm.dspTodoListItemsByDate",	tr("To-Do List Items..."),	SLOT(sDspTodoListItemsByDate()),	displaysMenu,	_privileges->check(""), NULL, NULL, true	},
    { "separator",					NULL,							NULL,							displaysMenu,	true		, NULL, NULL, true	},
    { "crm.dspExpiringWarrantiesByAccountRelationship",	tr("Expiring Warranties By Account Relationship..."),	SLOT(sDspExpiringWarrantiesByAccountRelationship()),	displaysMenu,	_privileges->check(""), NULL, NULL, true	},
    { "crm.dspReturnRepairHistoryByCustomer",		tr("Return and Repair History By Customer..."),		SLOT(sDspReturnRepairHistoryByCustomer()),		displaysMenu,	_privileges->check(""), NULL, NULL, true	},
    { "crm.dspReturnRepairHistoryByItem",		tr("Return and Repair History By Item..."),		SLOT(sDspReturnRepairHistoryByItem()),			displaysMenu,	_privileges->check(""), NULL, NULL, true	},
    { "crm.dspReturnRepairHistoryByParameterGroup",	tr("Return and Repair History By Parameter Group..."),	SLOT(dspReturnRepairHistoryByParameterGroup()),		displaysMenu,	_privileges->check(""), NULL, NULL, true	},
    { "separator",			NULL,					NULL,					displaysMenu,	true		, NULL, NULL, true	},
    { "crm.dspLotSerialByWorkOrder",	tr("Lot/Serial By Work Order..."),	SLOT(sDspLotSerialByWorkOrder()),	displayMenu,	_privileges->check(""), NULL, NULL, true	},
    { "crm.dspLotSerialBySalesOrder",	tr("Lot/Serial By Sales Order..."),	SLOT(sDspLotSerialBySalesOrder()),	displaysMenu,	_privileges->check(""), NULL, NULL, true	},
    { "crm.dspWorkOrderByLotSerial",	tr("Work Order By Lot/Serial..."),	SLOT(sWorkOrderByLotSerial()),		displaysMenu,	_privileges->check(""), NULL, NULL, true	},
    { "crm.dspSalesOrderByLotSerial",	tr("Sales Order By Lot/Serial..."),	SLOT(sSalesOrderByLotSerial()),		displaysMenu,	_privileges->check(""), NULL, NULL, true	},
    */

    { "menu",			tr("&Master Information"),		(char*)masterMenu,		crmMenu,	true, NULL, NULL, true	},
    { "crm.honorifics",		tr("Titles..."),			SLOT(sHonorifics()),		masterMenu,	_privileges->check("MaintainTitles") || _privileges->check("ViewTitles"), NULL, NULL, true	},
    { "separator",		NULL,					NULL,				masterMenu,	true,		 NULL, NULL, true	},
    { "crm.incidentCategories",	tr("Incident Categories..."),		SLOT(sIncidentCategories()),	masterMenu,	_privileges->check("MaintainIncidentCategories"), NULL, NULL, true },
    { "crm.incidentPriorities",	tr("Incident Priorities..."),		SLOT(sIncidentPriorities()),	masterMenu,	_privileges->check("MaintainIncidentPriorities"), NULL, NULL, true },
    { "crm.incidentSeverities",	tr("Incident Severities..."),		SLOT(sIncidentSeverities()),	masterMenu,	_privileges->check("MaintainIncidentSeverities"), NULL, NULL, true },
    { "crm.incidentResolutions", tr("Incident Resolutions..."),		SLOT(sIncidentResolutions()),	masterMenu,	_privileges->check("MaintainIncidentResolutions"), NULL, NULL, true },
    { "separator",		NULL,					NULL,				masterMenu,	true,		 NULL, NULL, true	},
    { "crm.opportunitySources", tr("Opportunity Sources..."), SLOT(sOpportunitySources()), masterMenu,	_privileges->check("MaintainOpportunitySources"), NULL, NULL, true },
    { "crm.opportunityStages",  tr("Opportunity Stages..."),  SLOT(sOpportunityStages()),  masterMenu,	_privileges->check("MaintainOpportunityStages"),  NULL, NULL, true },
    { "crm.opportunityTypes",   tr("Opportunity Types..."),   SLOT(sOpportunityTypes()),   masterMenu,	_privileges->check("MaintainOpportunityTypes"),   NULL, NULL, true },
    { "separator",		NULL,					NULL,				masterMenu,	true,		 NULL, NULL, true	},
    /*
    { "crm.warrTerms",		 tr("Warranty Terms..."),		SLOT(sWarrTerms()),		masterMenu,	_privileges->check(""), NULL, NULL, true	},
    */
    { "crm.characteristics",	tr("Characteristics..."),		SLOT(sCharacteristics()),	masterMenu,	_privileges->check("MaintainCharacteristics") ||
															_privileges->check("ViewCharacteristics"), NULL, NULL, true },

    { "menu",			tr("&Utilities"),		(char*)utilitiesMenu,		crmMenu,	true, NULL, NULL, true	},
    /*
    { "crm.purgeAddresses",	tr("Purge Addresses..."),	SLOT(sPurgeAddresses()),	utilitiesMenu,	_privileges->check(""), NULL, NULL, true	},
    { "crm.purgeContacts",	tr("Purge Contacts..."),	SLOT(sPurgeContacts()),		utilitiesMenu,	_privileges->check(""), NULL, NULL, true	},
    { "crm.purgeCRMAccounts",	tr("Purge CRM Accounts..."),	SLOT(sPurgeCRMAccounts()),	utilitiesMenu,	_privileges->check("MaintainCRMAccounts"), NULL, NULL, true	},
    { "crm.purgeTodoLists",	tr("Purge To-Do Lists..."),	SLOT(sPurgeTodoLists()),	utilitiesMenu,	_privileges->check(""), NULL, NULL, true	},
    */
  };

  for (unsigned int i = 0; i < sizeof(acts) / sizeof(acts[0]); i++)
  {
    if (acts[i].actionName == QString("menu"))
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
  parent->populateCustomMenu(crmMenu, "CRM");
  parent->menuBar()->insertItem(tr("C&RM"), crmMenu);
}

void moduleCRM::sCRMAccount()
{
  ParameterList params;
  params.append("mode", "new");
  crmaccount* newdlg = new crmaccount();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleCRM::sCRMAccounts()
{
  omfgThis->handleNewWindow(new crmaccounts());
}

void moduleCRM::sSearchForCRMAccount()
{
  omfgThis->handleNewWindow(new searchForCRMAccount());
}

void moduleCRM::sContact()
{
  ParameterList params;
  params.append("mode", "new");
  contact* newdlg = new contact();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleCRM::sContacts()
{
  omfgThis->handleNewWindow(new contacts());
}

void moduleCRM::sAddress()
{
  ParameterList params;
  params.append("mode", "new");
  address* newdlg = new address();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleCRM::sAddresses()
{
  omfgThis->handleNewWindow(new addresses());
}

void moduleCRM::sIncidentWorkbench()
{
  omfgThis->handleNewWindow(new incidentWorkbench());
}

void moduleCRM::sIncident()
{
  ParameterList params;
  params.append("mode", "new");
  incident* newdlg = new incident();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleCRM::sTodoList()
{
  omfgThis->handleNewWindow(new todoList());
}

void moduleCRM::sTodoItem()
{
  ParameterList params;
  params.append("mode", "new");
  todoItem* newdlg = new todoItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

/*
void moduleCRM::sTodoLists()
{
  omfgThis->handleNewWindow(new todoLists());
}
*/

void moduleCRM::sHonorifics()
{
  omfgThis->handleNewWindow(new honorifics());
}

void moduleCRM::sIncidentCategories()
{
  omfgThis->handleNewWindow(new incidentCategories());
}

void moduleCRM::sIncidentPriorities()
{
  omfgThis->handleNewWindow(new incidentPriorities());
}

void moduleCRM::sIncidentSeverities()
{
  omfgThis->handleNewWindow(new incidentSeverities());
}

void moduleCRM::sIncidentResolutions()
{
  omfgThis->handleNewWindow(new incidentResolutions());
}

void moduleCRM::sCharacteristics()
{
  omfgThis->handleNewWindow(new characteristics());
}

/*
void moduleCRM::sWarrTerms()
{
  omfgThis->handleNewWindow(new warrTerms());
}

void moduleCRM::sRma()
{
  omfgThis->handleNewWindow(new rma());
}

void moduleCRM::sRmas()
{
  omfgThis->handleNewWindow(new rmas());
}

void moduleCRM::sDspAddressBook()
{
  omfgThis->handleNewWindow(new dspAddressBook());
}

void moduleCRM::sDspContactsByAddress()
{
  omfgThis->handleNewWindow(new dspContactsByAddress());
}

void moduleCRM::sDspCRMAccountHierarchy()
{
  omfgThis->handleNewWindow(new dspCRMAccountHierarchy());
}
*/

void moduleCRM::sDspIncidentsByCRMAccount()
{
  omfgThis->handleNewWindow(new dspIncidentsByCRMAccount());
}

void moduleCRM::sDspTodoByUserAndIncident()
{
  omfgThis->handleNewWindow(new dspTodoByUserAndIncident());
}

/*
void moduleCRM::sDspIncidentChangesByDate()
{
  omfgThis->handleNewWindow(new dspIncidentChangesByDate());
}

void moduleCRM::sDspTodoListItemsByDate()
{
  omfgThis->handleNewWindow(new dspTodoListItemsByDate());
}

void moduleCRM::sDspLotSerialByWorkOrder()
{
  omfgThis->handleNewWindow(new dspLotSerialByWorkOrder());
}

void moduleCRM::sDspLotSerialBySalesOrder()
{
  omfgThis->handleNewWindow(new dspLotSerialBySalesOrder());
}

void moduleCRM::sWorkOrderByLotSerial()
{
  omfgThis->handleNewWindow(new dspWorkOrderByLotSerial());
}

void moduleCRM::sSalesOrderByLotSerial()
{
  omfgThis->handleNewWindow(new dspSalesOrderByLotSerial());
}

void moduleCRM::sPurgeAddresses()
{
  omfgThis->handleNewWindow(new purgeAddresses());
}

void moduleCRM::sPurgeContacts()
{
  omfgThis->handleNewWindow(new purgeContacts());
}

void moduleCRM::sPurgeAccounts()
{
  omfgThis->handleNewWindow(new purgeAccounts());
}

void moduleCRM::sPurgeTodoLists()
{
  omfgThis->handleNewWindow(new purgeTodoLists());
}

*/

void moduleCRM::sOpportunitySources()
{
  omfgThis->handleNewWindow(new opportunitySources());
}

void moduleCRM::sOpportunityStages()
{
  omfgThis->handleNewWindow(new opportunityStages());
}

void moduleCRM::sOpportunityTypes()
{
  omfgThis->handleNewWindow(new opportunityTypes());
}

void moduleCRM::sNewOpportunity()
{
  ParameterList params;
  params.append("mode", "new");

  opportunity newdlg(omfgThis, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void moduleCRM::sOpportunities()
{
  omfgThis->handleNewWindow(new opportunityList());
}
