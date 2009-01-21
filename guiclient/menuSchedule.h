/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef menuSchedule_h
#define menuSchedule_h

#include <QObject>
#include <QMenu>

class QToolBar;
class QMenu;
class GUIClient;

class menuSchedule : public QObject
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
    const QString   toolTip;
  };

  public:
    menuSchedule(GUIClient *);

  public slots:
    void sListProductionPlans();
    void sNewProductionPlan();

    void sCreatePlannedReplenOrdersByItem();
    void sCreatePlannedReplenOrdersByPlannerCode();
    void sCreatePlannedOrder();
    void sRunMPSByPlannerCode();
    void sDeletePlannedOrder();
    void sDeletePlannedOrdersByPlannerCode();
    void sFirmPlannedOrdersByPlannerCode();
    void sReleasePlannedOrdersByPlannerCode();

    void sCreateBufferStatusByItem();
    void sCreateBufferStatusByPlannerCode();
    void sDspInventoryBufferStatusByItemGroup();
    void sDspInventoryBufferStatusByClassCode();
    void sDspInventoryBufferStatusByPlannerCode();
    void sDspCapacityBufferStatusByWorkCenter();
    void sDspWoBufferStatusByItemGroup();
    void sDspWoBufferStatusByClassCode();
    void sDspWoBufferStatusByPlannerCode();
    void sDspWoOperationBufrStsByWorkCenter();
    void sDspPoItemsByBufferStatus();

    void sDspTimePhasedCapacityByWorkCenter();
    void sDspTimePhasedLoadByWorkCenter();
    void sDspTimePhasedAvailableCapacityByWorkCenter();
    void sDspTimePhasedDemandByPlannerCode();
    void sDspTimePhasedProductionByItem();
    void sDspTimePhasedProductionByPlannerCode();

    void sDspPlannedOrdersByItem();
    void sDspPlannedOrdersByPlannerCode();
    void sDspMPSDetail();
    void sDspRoughCutByWorkCenter();
    void sDspTimePhasedRoughCutByWorkCenter();
    void sDspPlannedRevenueExpensesByPlannerCode();
    void sDspTimePhasedPlannedREByPlannerCode();
    void sDspTimePhasedAvailability();
    void sDspRunningAvailability();
    void sDspMRPDetail();
    void sDspExpediteExceptionsByPlannerCode();
    void sDspReorderExceptionsByPlannerCode();

    void sPlannerCodes();
    void sWarehouseWeek();
    void sWarehouseCalendarExceptions();
    void sWorkCenters();

  private:
    GUIClient *parent;

    QToolBar   *toolBar;
    QMenu *mainMenu;
    QMenu *planningMenu;
    QMenu *plannedOrdersMenu;
    QMenu *plannedOrdersMrpMenu;
    QMenu *capacityPlanMenu;
    QMenu *capacityPlanTpPrdMenu;
    QMenu *bufferMenu;
    QMenu *bufferRunMenu;
    QMenu *bufferInvMenu;
    QMenu *bufferWoMenu;
    QMenu *reportsMenu;
    QMenu *reportsPlannedMenu;
    QMenu *masterInfoMenu;
    
    void	addActionsToMenu(actionProperties [], unsigned int);
};

#endif
