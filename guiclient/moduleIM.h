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

//  moduleIM.h
//  Created 08/22/2000 JSL
//  Copyright (c) 2000-2007, OpenMFG, LLC

#ifndef moduleIM_h
#define moduleIM_h

#include <QObject>
#include <QMenu>

class QToolBar;
class QMenu;
class OpenMFGGUIClient;

class moduleIM : public QObject
{
  Q_OBJECT

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

  public:
    moduleIM(OpenMFGGUIClient *);

  public slots:
    void sNewItemSite();
    void sItemSites();

    void sAdjustmentTrans();
    void sTransferTrans();
    void sNewTransferOrder();
    void sTransferOrders();
    void sReceiptTrans();
    void sScrapTrans();
    void sExpenseTrans();
    void sTransformTrans();
    void sResetQOHBalances();
    void sRelocateInventory();

    void sLotSerialHistory();
    void sLotSerialComments();
    void sReassignLotSerialNumber();

    void sCreateCountTagsByClassCode();
    void sCreateCountTagsByPlannerCode();
    void sCreateCountTagsByItem();
    void sCreateCycleCountTags();
    void sEnterCountSlip();
    void sEnterCountTags();
    void sEnterMiscCount();
    void sZeroUncountedTagsByWarehouse();
    void sThawItemSitesByClassCode();
    void sPostCountSlipsByWarehouse();
    void sPostCountTags();
    void sPurgePostedCountSlips();
    void sPurgePostedCountTags();

    void sDspItemAvailabilityWorkbench();

    void sDspFrozenItemSites();
    void sDspCountSlipEditList();
    void sDspCountTagEditList();
    void sDspCountSlipsByWarehouse();
    void sDspCountTagsByItem();
    void sDspCountTagsByWarehouse();
    void sDspCountTagsByClassCode();
    void sRptFrozenItemSites();
    void sRptCountSlipEditList();
    void sRptCountTagEditList();
    void sRptCountSlipsByWarehouse();
    void sRptCountTagsByItem();
    void sRptCountTagsByWarehouse();
    void sRptCountTagsByClassCode();

    void sDspItemSitesByItem();
    void sDspItemSitesByClassCode();
    void sDspItemSitesByPlannerCode();
    void sDspItemSitesByCostCategory();
    void sDspValidLocationsByItem();
    void sDspQOHByItem();
    void sDspQOHByClassCode();
    void sDspQOHByItemGroup();
    void sDspQOHByLocation();
    void sDspLocationLotSerialDetail();
    void sDspSlowMovingInventoryByClassCode();
    void sDspExpiredInventoryByClassCode();
    void sDspInventoryAvailabilityByItem();
    void sDspInventoryAvailabilityByItemGroup();
    void sDspInventoryAvailabilityByClassCode();
    void sDspInventoryAvailabilityByPlannerCode();
    void sDspInventoryAvailabilityBySourceVendor();
    void sDspSubstituteAvailabilityByRootItem();
    void sDspInventoryHistoryByItem();
    void sDspInventoryHistoryByItemGroup();
    void sDspInventoryHistoryByOrderNumber();
    void sDspInventoryHistoryByClassCode();
    void sDspInventoryHistoryByPlannerCode();
    void sDspDetailedInventoryHistoryByLotSerial();
    void sDspDetailedInventoryHistoryByLocation();
    void sDspItemUsageStatisticsByItem();
    void sDspItemUsageStatisticsByClassCode();
    void sDspItemUsageStatisticsByItemGroup();
    void sDspItemUsageStatisticsByWarehouse();
    void sDspTimePhasedUsageStatisticsByItem();
    void sDspInventoryBufferStatusByItem();
    void sDspInventoryBufferStatusByItemGroup();
    void sDspInventoryBufferStatusByClassCode();
    void sDspInventoryBufferStatusByPlannerCode();

    void sRptItemSitesByItem();
    void sRptItemSitesByClassCode();
    void sRptItemSitesByPlannerCode();
    void sRptItemSitesByCostCategory();
    void sRptValidLocationsByItem();
    void sRptQOHByItem();
    void sRptQOHByClassCode();
    void sRptQOHByItemGroup();
    void sRptQOHByLocation();
    void sRptLocationLotSerialDetail();
    void sRptSlowMovingInventoryByClassCode();
    void sRptExpiredInventoryByClassCode();
    void sRptInventoryAvailabilityByItem();
    void sRptInventoryAvailabilityByItemGroup();
    void sRptInventoryAvailabilityByClassCode();
    void sRptInventoryAvailabilityByPlannerCode();
    void sRptInventoryAvailabilityBySourceVendor();
    void sRptSubstituteAvailabilityByRootItem();
    void sRptInventoryHistoryByItem();
    void sRptInventoryHistoryByItemGroup();
    void sRptInventoryHistoryByOrderNumber();
    void sRptInventoryHistoryByClassCode();
    void sRptInventoryHistoryByPlannerCode();
    void sRptDetailedInventoryHistoryByLotSerial();
    void sRptDetailedInventoryHistoryByLocation();
    void sRptItemUsageStatisticsByItem();
    void sRptItemUsageStatisticsByClassCode();
    void sRptItemUsageStatisticsByItemGroup();
    void sRptItemUsageStatisticsByWarehouse();
    void sRptTimePhasedUsageStatisticsByItem();
    void sRptInventoryBufferStatusByItem();
    void sRptInventoryBufferStatusByItemGroup();
    void sRptInventoryBufferStatusByClassCode();
    void sRptInventoryBufferStatusByPlannerCode();
    void sBarCodeDispatchList();
    void sPrintItemLabelsByClassCode();

    void sWarehouses();
    void sWarehouseLocations();
    void sCostCategories();
    void sExpenseCategories();

    void sDspUnbalancedQOHByClassCode();
    void sUpdateABCClass();
    void sUpdateCycleCountFreq();
    void sUpdateItemSiteLeadTimes();
    void sUpdateReorderLevelByItem();
    void sUpdateReorderLevelsByPlannerCode();
    void sUpdateReorderLevelsByClassCode();
    void sUpdateOUTLevelByItem();
    void sUpdateOUTLevelsByPlannerCode();
    void sUpdateOUTLevelsByClassCode();
    void sSummarizeInvTransByClassCode();
    void sCreateItemSitesByClassCode();

    void sCatchLocationContents(int);
    void sCatchCountTag(int);
    void sCharacteristics();

  private:
    OpenMFGGUIClient *parent;

    QToolBar   *toolBar;
    QMenu *mainMenu;
    QMenu *itemSitesMenu;
    QMenu *transactionsMenu;
    QMenu *lotSerialControlMenu;
    QMenu *physicalMenu;
    QMenu *physicalDisplaysMenu;
    QMenu *physicalReportsMenu;
    QMenu *displaysMenu;
    QMenu *graphsMenu;
    QMenu *reportsMenu;
    QMenu *masterInfoMenu;
    QMenu *utilitiesMenu;
    QMenu *updateItemInfoMenu;

    void	addActionsToMenu(actionProperties [], unsigned int);
};

#endif
