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

//  modulePD.cpp
//  Created 08/22/2000 JSL
//  Copyright (c) 2000-2008, OpenMFG, LLC

#include <QAction>
#include <QMenuBar>
#include <QPixmap>
#include <QMenu>
#include <QToolBar>

#include <parameter.h>

#include "guiclient.h"

#include "item.h"
#include "items.h"
#include "searchForItem.h"
#include "copyItem.h"
#include "itemGroups.h"
#include "itemImages.h"

#include "bom.h"
#include "bomList.h"
#include "copyBOM.h"

#include "boo.h"
#include "booList.h"
#include "copyBOO.h"
#include "massReplaceComponent.h"
#include "massExpireComponent.h"

#include "bbom.h"
#include "bboms.h"

#include "maintainItemCosts.h"
#include "updateActualCostsByItem.h"
#include "updateActualCostsByClassCode.h"
#include "postCostsByItem.h"
#include "postCostsByClassCode.h"

#include "dspCostedSingleLevelBOM.h"
#include "dspCostedIndentedBOM.h"
#include "dspCostedSummarizedBOM.h"
#include "dspItemCostsByClassCode.h"
#include "dspItemCostSummary.h"
#include "dspItemCostHistory.h"

#include "costingElements.h"

#include "dspItemsByClassCode.h"
#include "dspItemsByCharacteristic.h"
#include "dspItemsByProductCategory.h"
#include "dspSingleLevelBOM.h"
#include "dspIndentedBOM.h"
#include "dspSummarizedBOM.h"
#include "dspSequencedBOM.h"
#include "dspSingleLevelWhereUsed.h"
#include "dspIndentedWhereUsed.h"
#include "dspPendingBOMChanges.h"
#include "dspOperationsByWorkCenter.h"
#include "dspStandardOperationsByWorkCenter.h"
#include "dspCapacityUOMsByClassCode.h"
#include "dspCapacityUOMsByProductCategory.h"

#include "itemAvailabilityWorkbench.h"

#include "uoms.h"
#include "classCodes.h"
#include "productCategories.h"
#include "characteristics.h"
#include "laborRates.h"
#include "workCenters.h"
#include "standardOperations.h"

#include "dspUndefinedManufacturedItems.h"
#include "dspUnusedPurchasedItems.h"
#include "dspInvalidBillsOfMaterials.h"
#include "reassignClassCodeByClassCode.h"
#include "reassignProductCategoryByProductCategory.h"

#include "modulePD.h"

modulePD::modulePD(GUIClient *Pparent) :
 QObject(Pparent, "pdModule")
{
  parent = Pparent;

  toolBar = new QToolBar(tr("P/D Tools"));
  toolBar->setObjectName("P/D Tools");
  toolBar->setIconSize(QSize(32, 32));
  if (_preferences->boolean("ShowPDToolbar"))
    parent->addToolBar(toolBar);

//  I/M | Items
  itemsMenu = new QMenu();

  parent->actions.append( new Action( parent, "pd.enterNewItem", tr("Enter New Item..."),
                                      this, SLOT(sNewItem()),
                                      itemsMenu, _privleges->check("MaintainItemMasters") ) );
  //                                    QPixmap(":/images/newItem.xpm"), toolBar ) );

  parent->actions.append( new Action( parent, "pd.listItems", tr("List Items..."),
                                      this, SLOT(sItems()),
                                      itemsMenu, (_privleges->check("MaintainItemMasters") || _privleges->check("ViewItemMasters")),
                                      QPixmap(":/images/items.png"), toolBar ) );

  parent->actions.append( new Action( parent, "pd.searchForItems", tr("Search for Items..."),
                                      this, SLOT(sSearchForItems()),
                                      itemsMenu, (_privleges->check("MaintainItemMasters") || _privleges->check("ViewItemMasters")) ) );

  parent->actions.append( new Action( parent, "pd.copyItem", tr("Copy Item..."),
                                      this, SLOT(sCopyItem()),
                                      itemsMenu, _privleges->check("MaintainItemMasters") ) );

  itemsMenu->insertSeparator();

  parent->actions.append( new Action( parent, "pd.itemAvailabilityWorkbench", tr("Item Availability Workbench..."),
                                      this, SLOT(sDspItemAvailabilityWorkbench()),
                                      itemsMenu, _privleges->check("ViewItemAvailabilityWorkbench") ) );

  itemsMenu->insertSeparator();

  parent->actions.append( new Action( parent, "pd.itemGroups", tr("Item Groups..."),
                                      this, SLOT(sItemGroups()),
                                      itemsMenu, (_privleges->check("MaintainItemGroups") || _privleges->check("ViewItemGroups")) ) );

  parent->actions.append( new Action( parent, "pd.itemImages", tr("Item Images..."),
                                      this, SLOT(sItemImages()),
                                      itemsMenu, (_privleges->check("MaintainItemMasters") || _privleges->check("ViewItemMasters")) ) );


//  P/D | Bills of Materials
  bomMenu = new QMenu();

  parent->actions.append( new Action( parent, "pd.enterNewBOM", tr("Enter New Bill of Materials..."),
                                      this, SLOT(sNewBOM()),
                                      bomMenu, _privleges->check("MaintainBOMs") ) );
//                                               QPixmap(":/images/newBOM.xpm"), toolBar ) );

  parent->actions.append( new Action( parent, "pd.listBOMs", tr("List Bills of Materials..."),
                                      this, SLOT(sBOMs()),
                                      bomMenu, (_privleges->check("MaintainBOMs") || _privleges->check("ViewBOMs")),
                                      QPixmap(":/images/boms.png"), toolBar ) );

  parent->actions.append( new Action( parent, "pd.copyBOM", tr("Copy Bill of Materials..."),
                                      this, SLOT(sCopyBOM()),
                                      bomMenu, _privleges->check("MaintainBOMs") ) );

  bomMenu->insertSeparator();

  parent->actions.append( new Action( parent, "pd.massReplaceComponentItem", tr("Mass Replace Component Item..."),
                                      this, SLOT(sMassReplaceComponent()),
                                      bomMenu, _privleges->check("MaintainBOMs") ) );

  parent->actions.append( new Action( parent, "pd.massExpireComponentItem", tr("Mass Expire Component Item..."),
                                      this, SLOT(sMassExpireComponent()),
                                      bomMenu, _privleges->check("MaintainBOMs") ) );


//  P/D | Bills of Operations...

  booMenu = new QMenu();

  parent->actions.append( new Action( parent, "pd.enterNewBOO", tr("Enter New Bill of Operations..."),
                                      this, SLOT(sNewBOO()),
                                      booMenu, (_privleges->check("MaintainBOOs") && _metrics->boolean("Routings")) ) );

  parent->actions.append( new Action( parent, "pd.listBOOs", tr("List Bills of Operations..."),
                                      this, SLOT(sBOOs()),
                                      booMenu, ((_privleges->check("MaintainBOOs") || _privleges->check("ViewBOOs")) && _metrics->boolean("Routings")),
                                      QPixmap(":/images/boos.png"), toolBar ) );

  parent->actions.append( new Action( parent, "pd.copyBOO", tr("Copy Bill of Operations..."),
                                      this, SLOT(sCopyBOO()),
                                      booMenu, (_privleges->check("MaintainBOOs") && _metrics->boolean("Routings")) ) );


//  P/D | Breeder Bills of Materials

  breederBOMMenu = new QMenu();

  parent->actions.append( new Action( parent, "pd.enterNewBreederBOM", tr("Enter New Breeder Bill of Materials..."),
                                      this, SLOT(sNewBreederBOM()),
                                      breederBOMMenu, (_privleges->check("MaintainBBOMs") && _metrics->boolean("BBOM")) ) );

  parent->actions.append( new Action( parent, "pd.listBreederBOMs", tr("List Breeder Bills of Materials..."),
                                      this, SLOT(sBreederBOMs()),
                                      breederBOMMenu, ((_privleges->check("MaintainBBOMs") || _privleges->check("ViewBBOMs")) && _metrics->boolean("BBOM")) ) );

//  P/D | Costing
  costingDisplaysMenu = new QMenu();

  parent->actions.append( new Action( parent, "pd.dspCostedSingleLevelBOM", tr("Costed Single Level BOM..."),
                                      this, SLOT(sDspCostedSingleLevelBOM()),
                                      costingDisplaysMenu, _privleges->check("ViewCosts") ) );

  parent->actions.append( new Action( parent, "pd.dspCostedIndentedBOM", tr("Costed Indented BOM..."),
                                      this, SLOT(sDspCostedIndentedBOM()),
                                      costingDisplaysMenu, _privleges->check("ViewCosts") ) );

  parent->actions.append( new Action( parent, "pd.dspCostedSummarizedBOM", tr("Costed Summarized BOM..."),
                                      this, SLOT(sDspCostedSummarizedBOM()),
                                      costingDisplaysMenu, _privleges->check("ViewCosts") ) );

  parent->actions.append( new Action( parent, "pd.dspItemCostsByClassCode", tr("Item Costs by Class Code..."),
                                      this, SLOT(sDspItemCostsByClassCode()),
                                      costingDisplaysMenu, _privleges->check("ViewCosts") ) );

  parent->actions.append( new Action( parent, "pd.dspItemCostsSummary", tr("Item Costs Summary..."),
                                      this, SLOT(sDspItemCostSummary()),
                                      costingDisplaysMenu, _privleges->check("ViewCosts") ) );

  parent->actions.append( new Action( parent, "pd.dspItemCostsHistory", tr("Item Costs History..."),
                                      this, SLOT(sDspItemCostHistory()),
                                      costingDisplaysMenu, _privleges->check("ViewCosts") ) );

  costingMenu = new QMenu();

  parent->actions.append( new Action( parent, "pd.maintainItemCosts", tr("Maintain Item Costs..."),
                                      this, SLOT(sMaintainItemCosts()),
                                      costingMenu, _privleges->check("ViewCosts") ) );

  costingMenu->insertSeparator();

  parent->actions.append( new Action( parent, "pd.updateActualCostsByItem", tr("Update Actual Costs by Item..."),
                                      this, SLOT(sUpdateActualCostsByItem()),
                                      costingMenu, _privleges->check("UpdateActualCosts") ) );

  parent->actions.append( new Action( parent, "pd.updateActualCostsByClassCode", tr("Update Actual Costs by Class Code..."),
                                      this, SLOT(sUpdateActualCostsByClassCode()),
                                      costingMenu, _privleges->check("UpdateActualCosts") ) );

  costingMenu->insertSeparator();

  parent->actions.append( new Action( parent, "pd.postActualCostsByItem", tr("Post Actual Costs by Item..."),
                                      this, SLOT(sPostActualCostsByItem()),
                                      costingMenu, _privleges->check("PostActualCosts") ) );

  parent->actions.append( new Action( parent, "pd.postActualCostsByClassCode", tr("Post Actual Costs by Class Code..."),
                                      this, SLOT(sPostActualCostsByClassCode()),
                                      costingMenu, _privleges->check("PostActualCosts") ) );

  costingMenu->insertSeparator();

  parent->actions.append( new Action( parent, "pd.updateStandardCostsByItem", tr("Post Standard Costs by Item..."),
                                      this, SLOT(sUpdateStandardCostsByItem()),
                                      costingMenu, _privleges->check("PostStandardCosts") ) );

  parent->actions.append( new Action( parent, "pd.updateStandardCostsByClassCode", tr("Post Standard Costs by Class Code..."),
                                      this, SLOT(sUpdateStandardCostsByClassCode()),
                                      costingMenu, _privleges->check("PostStandardCosts") ) );


  costingMenu->insertSeparator();

  costingMenu->insertItem(tr("&Displays"), costingDisplaysMenu );

  costingMenu->insertSeparator();

  parent->actions.append( new Action( parent, "pd.userDefinedCostingElements", tr("User-Defined Costing Elements..."),
                                      this, SLOT(sUserCostingElements()),
                                      costingMenu, _privleges->check("MaintainUserCostingElements") ) );


//  P/D | Displays
  displaysMenu = new QMenu();

  parent->actions.append( new Action( parent, "pd.dspItemsByClassCode", tr("Items by Class Code..."),
                                      this, SLOT(sDspItemsByClassCode()),
                                      displaysMenu, (_privleges->check("MaintainItemMasters") || _privleges->check("ViewItemMasters")) ) );

  parent->actions.append( new Action( parent, "pd.dspItemsByProductCategory", tr("Items by Product Category..."),
                                      this, SLOT(sDspItemsByProductCategory()),
                                      displaysMenu, (_privleges->check("MaintainItemMasters") || _privleges->check("ViewItemMasters")) ) );

  parent->actions.append( new Action( parent, "pd.dspItemsByCharacteristic", tr("Items by Characteristic..."),
                                      this, SLOT(sDspItemsByCharacteristic()),
                                      displaysMenu, (_privleges->check("MaintainItemMasters") || _privleges->check("ViewItemMasters")) ) );

  displaysMenu->insertSeparator();

  parent->actions.append( new Action( parent, "pd.dspSingleLevelBOM", tr("Single Level Bill of Materials..."),
                                      this, SLOT(sDspSingleLevelBOM()),
                                      displaysMenu, _privleges->check("ViewBOMs") ) );

  parent->actions.append( new Action( parent, "pd.dspIndentedBOM", tr("Indented Bill of Materials..."),
                                      this, SLOT(sDspIndentedBOM()),
                                      displaysMenu, _privleges->check("ViewBOMs") ) );

  parent->actions.append( new Action( parent, "pd.dspSummarizedBOM", tr("Summarized Bill of Materials..."),
                                      this, SLOT(sDspSummarizedBOM()),
                                      displaysMenu, _privleges->check("ViewBOMs") ) );

if (_metrics->boolean("Routings") )
{
  parent->actions.append( new Action( parent, "pd.dspSequencedBOM", tr("Sequenced Bill of Materials..."),
                                      this, SLOT(sDspSequencedBOM()),
                                      displaysMenu, _privleges->check("ViewBOMs") ) );
}

  displaysMenu->insertSeparator();

  parent->actions.append( new Action( parent, "pd.dspSingleLevelWhereUsed", tr("Single Level Where Used..."),
                                      this, SLOT(sDspSingleLevelWhereUsed()),
                                      displaysMenu, _privleges->check("ViewBOMs") ) );

  parent->actions.append( new Action( parent, "pd.dspIndentedWhereUsed", tr("Indented Where Used..."),
                                      this, SLOT(sDspIndentedWhereUsed()),
                                      displaysMenu, _privleges->check("ViewBOMs") ) );

  parent->actions.append( new Action( parent, "pd.dspPendingBOMChanges", tr("Pending BOM Changes..."),
                                      this, SLOT(sDspPendingBOMChanges()),
                                      displaysMenu, _privleges->check("ViewBOMs") ) );

 if (_metrics->boolean("Routings") )
 {
      displaysMenu->insertSeparator();

      parent->actions.append( new Action( parent, "pd.dspOperationsByWorkCenter", tr("Operations by Work Center..."),
                                          this, SLOT(sDspOperationsByWorkCenter()),
                                          displaysMenu,((_privleges->check("ViewBOOs")) && (_metrics->boolean("Routings"))) ) );

      parent->actions.append( new Action( parent, "pd.dspStandardOperationsByWorkCenter", tr("Standard Operations by Work Center..."),
                                          this, SLOT(sDspStandardOperByWorkCenter()),
                                          displaysMenu, ((_privleges->check("ViewStandardOperations")) && (_metrics->boolean("Routings"))) ) );

      displaysMenu->insertSeparator();

      parent->actions.append( new Action( parent, "pd.dspCapacityUOMsByClassCode", tr("Capacity UOMs by Class Code..."),
                                          this, SLOT(sDspCapacityUOMsByClassCode()),
                                          displaysMenu, (_privleges->check("MaintainItemMasters") || _privleges->check("ViewItemMasters")) ) );

      parent->actions.append( new Action( parent, "pd.dspCapacityUOMsByProductCategory", tr("Capacity UOMs by Product Category..."),
                                          this, SLOT(sDspCapacityUOMsByProductCategory()),
                                          displaysMenu, (_privleges->check("MaintainItemMasters") || _privleges->check("ViewItemMasters")) ) );
  }
  
//  P/D | Master Information
  masterInfoMenu = new QMenu();

  parent->actions.append( new Action( parent, "pd.unitsOfMeasure", tr("Units of Measure..."),
                                      this, SLOT(sUnitsOfMeasure()),
                                      masterInfoMenu, (_privleges->check("MaintainUOMs") || _privleges->check("ViewUOMs")) ) );

  parent->actions.append( new Action( parent, "pd.classCodes", tr("Class Codes..."),
                                      this, SLOT(sClassCodes()),
                                      masterInfoMenu, (_privleges->check("MaintainClassCodes") || _privleges->check("ViewClassCodes")) ) );

  parent->actions.append( new Action( parent, "pd.productCategories", tr("Product Categories..."),
                                      this, SLOT(sProductCategories()),
                                      masterInfoMenu, (_privleges->check("MaintainProductCategories") || _privleges->check("ViewProductCategories")) ) );

  parent->actions.append( new Action( parent, "pd.characteristics", tr("Characteristics..."),
                                      this, SLOT(sCharacteristics()),
                                      masterInfoMenu, (_privleges->check("MaintainCharacteristics") || _privleges->check("ViewCharacteristics")) ) );

 if (_metrics->boolean("Routings") )
 {
      masterInfoMenu->insertSeparator();

      parent->actions.append( new Action( parent, "pd.standardLaborRates", tr("Standard Labor Rates..."),
                                          this, SLOT(sStandardLaborRates()),
                                          masterInfoMenu, ((_privleges->check("MaintainLaborRates") || _privleges->check("ViewLaborRates")) && _metrics->boolean("Routings")) ) );

      parent->actions.append( new Action( parent, "pd.workCenters", tr("Work Centers..."),
                                          this, SLOT(sWorkCenters()),
                                          masterInfoMenu, ((_privleges->check("MaintainWorkCenters") || _privleges->check("ViewWorkCenters")) && _metrics->boolean("Routings")) ) );

      parent->actions.append( new Action( parent, "pd.standardOperations", tr("Standard Operations..."),
                                          this, SLOT(sStandardOperations()),
                                          masterInfoMenu, ((_privleges->check("MaintainStandardOperations") || _privleges->check("ViewStandardOperations")) && _metrics->boolean("Routings")) ) );
  }


//  P/D | Utilies
  utilitiesMenu = new QMenu();

  parent->actions.append( new Action( parent, "pd.dspUnusedPurchasedItems", tr("Unused Purchased Items..."),
                                      this, SLOT(sDspUnusedPurchasedItems()),
                                      utilitiesMenu, _privleges->check("ViewBOMs") ) );

  parent->actions.append( new Action( parent, "pd.dspUndefinedManufacturedItems", tr("Undefined Manufactured Items..."),
                                      this, SLOT(sDspUndefinedManufacturedItems()),
                                      utilitiesMenu, (_privleges->check("ViewBOMs") || _privleges->check("ViewBOOs")) ) );

  parent->actions.append( new Action( parent, "pd.dspBillsOfMaterialsWithoutComponentItemSites", tr("Bills of Materials without Component Item Sites..."),
                                      this, SLOT(sDspInvalidBillsOfMaterials()),
                                      utilitiesMenu, _privleges->check("ViewBOMs") ) );

  utilitiesMenu->insertSeparator();

  parent->actions.append( new Action( parent, "pd.reassignClassCodeByClassCode", tr("Reassign Class Code by Class Code..."),
                                      this, SLOT(sReassignClassCodeByClassCode()),
                                      utilitiesMenu, _privleges->check("MaintainItemMasters") ) );

  parent->actions.append( new Action( parent, "pd.reassignProductCategoryByProductCategory", tr("Reassign Product Category by Product Category..."),
                                      this, SLOT(sReassignProductCategoryByProductCategory()),
                                      utilitiesMenu, _privleges->check("MaintainItemMasters") ) );

  mainMenu = new QMenu();
  mainMenu->insertItem(tr("&Items"),                      itemsMenu      );
  mainMenu->insertItem(tr("&Bills of Materials"),         bomMenu        );
  if (_metrics->boolean("Routings"))
    mainMenu->insertItem(tr("Bills of &Operations"),        booMenu        );
  if (_metrics->boolean("BBOM"))
    mainMenu->insertItem(tr("Br&eeder Bills of Materials"), breederBOMMenu );
  mainMenu->insertItem(tr("&Costing"),                    costingMenu    );
  mainMenu->insertItem(tr("&Displays"),                   displaysMenu   );
  mainMenu->insertItem(tr("&Master Information"),         masterInfoMenu );
  mainMenu->insertItem(tr("&Utilities"),                  utilitiesMenu  );
  parent->populateCustomMenu(mainMenu, "P/D");	
  parent->menuBar()->insertItem(tr("P/&D"), mainMenu);
}

void modulePD::sNewItem()
{
  item::newItem();
}

void modulePD::sItems()
{
  omfgThis->handleNewWindow(new items());
}

void modulePD::sSearchForItems()
{
  omfgThis->handleNewWindow(new searchForItem());
}

void modulePD::sCopyItem()
{
  copyItem(parent, "", TRUE).exec();
}

void modulePD::sItemGroups()
{
  omfgThis->handleNewWindow(new itemGroups());
}

void modulePD::sItemImages()
{
  omfgThis->handleNewWindow(new itemImages());
}


void modulePD::sNewBOM()
{
  ParameterList params;
  params.append("mode", "new");

  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void modulePD::sBOMs()
{
  omfgThis->handleNewWindow(new bomList());
}

void modulePD::sCopyBOM()
{
  copyBOM(parent, "", TRUE).exec();
}

void modulePD::sMassReplaceComponent()
{
  omfgThis->handleNewWindow(new massReplaceComponent());
}

void modulePD::sMassExpireComponent()
{
  omfgThis->handleNewWindow(new massExpireComponent());
}

void modulePD::sNewBOO()
{
  ParameterList params;
  params.append("mode", "new");

  boo *newdlg = new boo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void modulePD::sBOOs()
{
  omfgThis->handleNewWindow(new booList());
}

void modulePD::sCopyBOO()
{
  copyBOO(parent, "", TRUE).exec();
}

void modulePD::sNewBreederBOM()
{
  ParameterList params;
  params.append("mode", "new");

  bbom *newdlg = new bbom();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void modulePD::sBreederBOMs()
{
  omfgThis->handleNewWindow(new bboms());
}

//  Costing
void modulePD::sMaintainItemCosts()
{
  omfgThis->handleNewWindow(new maintainItemCosts());
}

void modulePD::sUpdateActualCostsByItem()
{
  ParameterList params;
  params.append("costtype", "actual");

  updateActualCostsByItem *newdlg = new updateActualCostsByItem(parent, "", TRUE);
  newdlg->set(params);
  newdlg->exec();
}

void modulePD::sUpdateActualCostsByClassCode()
{
  ParameterList params;
  params.append("costtype", "actual");

  updateActualCostsByClassCode *newdlg = new updateActualCostsByClassCode(parent, "", TRUE);
  newdlg->set(params);
  newdlg->exec();
}

void modulePD::sPostActualCostsByItem()
{
  postCostsByItem(parent, "", TRUE).exec();
}

void modulePD::sPostActualCostsByClassCode()
{
  postCostsByClassCode(parent, "", TRUE).exec();
}

void modulePD::sUpdateStandardCostsByItem()
{
  ParameterList params;
  params.append("costtype", "standard");

  updateActualCostsByItem *newdlg = new updateActualCostsByItem(parent, "", TRUE);
  newdlg->set(params);
  newdlg->exec();
}

void modulePD::sUpdateStandardCostsByClassCode()
{
  ParameterList params;
  params.append("costtype", "standard");

  updateActualCostsByClassCode *newdlg = new updateActualCostsByClassCode(parent, "", TRUE);
  newdlg->set(params);
  newdlg->exec();
}

void modulePD::sDspCostedSingleLevelBOM()
{
  omfgThis->handleNewWindow(new dspCostedSingleLevelBOM());
}

void modulePD::sDspCostedIndentedBOM()
{
  omfgThis->handleNewWindow(new dspCostedIndentedBOM());
}

void modulePD::sDspCostedSummarizedBOM()
{
  omfgThis->handleNewWindow(new dspCostedSummarizedBOM());
}

void modulePD::sDspItemCostsByClassCode()
{
  omfgThis->handleNewWindow(new dspItemCostsByClassCode());
}

void modulePD::sDspItemCostSummary()
{
  omfgThis->handleNewWindow(new dspItemCostSummary());
}

void modulePD::sDspItemCostHistory()
{
  omfgThis->handleNewWindow(new dspItemCostHistory());
}

void modulePD::sUserCostingElements()
{
  omfgThis->handleNewWindow(new costingElements());
}

//  Displays
void modulePD::sDspItemsByClassCode()
{
  omfgThis->handleNewWindow(new dspItemsByClassCode());
}

void modulePD::sDspItemsByProductCategory()
{
  omfgThis->handleNewWindow(new dspItemsByProductCategory());
}

void modulePD::sDspItemsByCharacteristic()
{
  omfgThis->handleNewWindow(new dspItemsByCharacteristic());
}

void modulePD::sDspSingleLevelBOM()
{
  omfgThis->handleNewWindow(new dspSingleLevelBOM());
}

void modulePD::sDspIndentedBOM()
{
  omfgThis->handleNewWindow(new dspIndentedBOM());
}

void modulePD::sDspSummarizedBOM()
{
  omfgThis->handleNewWindow(new dspSummarizedBOM());
}

void modulePD::sDspSequencedBOM()
{
  omfgThis->handleNewWindow(new dspSequencedBOM());
}

void modulePD::sDspSingleLevelWhereUsed()
{
  omfgThis->handleNewWindow(new dspSingleLevelWhereUsed());
}

void modulePD::sDspIndentedWhereUsed()
{
  omfgThis->handleNewWindow(new dspIndentedWhereUsed());
}

void modulePD::sDspPendingBOMChanges()
{
  omfgThis->handleNewWindow(new dspPendingBOMChanges());
}

void modulePD::sDspOperationsByWorkCenter()
{
  omfgThis->handleNewWindow(new dspOperationsByWorkCenter());
}

void modulePD::sDspStandardOperByWorkCenter()
{
  omfgThis->handleNewWindow(new dspStandardOperationsByWorkCenter());
}

void modulePD::sDspCapacityUOMsByClassCode()
{
  omfgThis->handleNewWindow(new dspCapacityUOMsByClassCode());
}

void modulePD::sDspCapacityUOMsByProductCategory()
{
  omfgThis->handleNewWindow(new dspCapacityUOMsByProductCategory());
}

void modulePD::sDspItemAvailabilityWorkbench()
{
  omfgThis->handleNewWindow(new itemAvailabilityWorkbench());
}

//  Master Information
void modulePD::sUnitsOfMeasure()
{
  omfgThis->handleNewWindow(new uoms());
}

void modulePD::sClassCodes()
{
  omfgThis->handleNewWindow(new classCodes());
}

void modulePD::sProductCategories()
{
  omfgThis->handleNewWindow(new productCategories());
}

void modulePD::sCharacteristics()
{
  omfgThis->handleNewWindow(new characteristics());
}

void modulePD::sStandardLaborRates()
{
  omfgThis->handleNewWindow(new laborRates());
}

void modulePD::sWorkCenters()
{
  omfgThis->handleNewWindow(new workCenters());
}

void modulePD::sStandardOperations()
{
  omfgThis->handleNewWindow(new standardOperations());
}

//  Utilities
void modulePD::sDspUnusedPurchasedItems()
{
  omfgThis->handleNewWindow(new dspUnusedPurchasedItems());
}

void modulePD::sDspUndefinedManufacturedItems()
{
  omfgThis->handleNewWindow(new dspUndefinedManufacturedItems());
}

void modulePD::sDspInvalidBillsOfMaterials()
{
  omfgThis->handleNewWindow(new dspInvalidBillsOfMaterials());
}

void modulePD::sReassignClassCodeByClassCode()
{
  reassignClassCodeByClassCode(parent, "", TRUE).exec();
}

void modulePD::sReassignProductCategoryByProductCategory()
{
  reassignProductCategoryByProductCategory(parent, "", TRUE).exec();
}
