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

//  modulePM.cpp
//  Created 10/31/2003 JSL
//  Copyright (c) 2003-2007, OpenMFG, LLC

#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>

#include <parameter.h>

#include "OpenMFGGUIClient.h"

#include "project.h"
#include "projects.h"

//#include "postLaborToProject.h"

#include "dspOrderActivityByProject.h"

#include "rptOrderActivityByProject.h"

#include "modulePM.h"

modulePM::modulePM(OpenMFGGUIClient *Pparent) :
  QObject(Pparent, "pmModule")
{
  parent = Pparent;
  
  toolBar = new QToolBar(tr("P/M Tools"));
  toolBar->setObjectName("P/M Tools");
  toolBar->setIconSize(QSize(32, 32));
  if (_preferences->boolean("ShowPMToolbar"))
    parent->addToolBar(toolBar);

//  Projects
  projectsMenu = new QMenu();

  parent->actions.append( new Action( parent, "pm.newProject", tr("New Project..."),
                                      this, SLOT(sNewProject()),
                                      projectsMenu, _privleges->check("MaintainProjects") ) );

  parent->actions.append( new Action( parent, "pm.projects", tr("Projects..."),
                                      this, SLOT(sProjects()),
                                      projectsMenu, _privleges->check("ViewProjects"),
									  QPixmap(":/images/projects.png"), toolBar ) );

  //projectsMenu->insertSeparator();
  //
  //parent->actions.append( new Action( parent, "pm.postLaborToProject", tr("Post Labor to Project..."),
  //                                    this, SLOT(sPostLaborToProject()),
  //                                    projectsMenu, TRUE ) );

//  Displays
  displaysMenu = new QMenu();

  parent->actions.append( new Action( parent, "pm.dspOrderActivityByProject", tr("Order Activity by Project..."),
                                      this, SLOT(sDspOrderActivityByProject()),
                                      displaysMenu, _privleges->check("ViewProjects") ) );

//  Reports
  reportsMenu = new QMenu();

  parent->actions.append( new Action( parent, "pm.rptOrderActivityByProject", tr("Order Activity by Project..."),
                                      this, SLOT(sRptOrderActivityByProject()),
                                      reportsMenu, _privleges->check("ViewProjects") ) );

  mainMenu = new QMenu();
  mainMenu->insertItem(tr("Projects"), projectsMenu);
  mainMenu->insertItem(tr("Displays"), displaysMenu);
  mainMenu->insertItem(tr("Reports"), reportsMenu);
  parent->populateCustomMenu(mainMenu, "P/M");
  parent->menuBar()->insertItem(tr("P/M"), mainMenu);
}

void modulePM::sNewProject()
{
  ParameterList params;
  params.append("mode", "new");

  project newdlg(omfgThis, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void modulePM::sProjects()
{
  omfgThis->handleNewWindow(new projects());
}

void modulePM::sPostLaborToProject()
{
  //postLaborToProject newdlg(omfgThis, "", TRUE);
  //newdlg.exec();
}

void modulePM::sDspOrderActivityByProject()
{
  omfgThis->handleNewWindow(new dspOrderActivityByProject());
}

void modulePM::sRptOrderActivityByProject()
{
  rptOrderActivityByProject(parent, "", true).exec();
}

