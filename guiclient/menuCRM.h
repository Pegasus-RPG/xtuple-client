/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef menuCRM_h
#define menuCRM_h

#include <QObject>
#include <QPixmap>

class QToolBar;
class QMenu;
class GUIClient;

class menuCRM : public QObject
{
  Q_OBJECT

  struct actionProperties {
    const char*		actionName;
    const QString	actionTitle;
    const char*		slot;
    QMenu*		menu;
    QString		priv;
    QPixmap		pixmap;
    QToolBar*		toolBar;
    bool		visible;
    const QString   toolTip;
  };

  public:
    menuCRM(GUIClient *);

  public slots:

    void sNewProject();
    void sProjects();
    void sDspOrderActivityByProject();

    void sAddress();
    void sAddresses();
    void sCharacteristics();
    void sCRMAccount();
    void sCRMAccounts();
    void sSearchForCRMAccount();
    void sContact();
    void sContacts();
    void sCreateRecurringItems();
    void sSearchForContact();
    void sDspIncidentsByCRMAccount();
    void sDspTodoByUserAndIncident();
    void sHonorifics();
    void sIncident();
    void sIncidentCategories();
    void sIncidentPriorities();
    void sIncidentResolutions();
    void sIncidentSeverities();
    void sIncidentWorkbench();
    void sTodoItem();
    void sTodoList();
    void sTodoListCalendar();
    void sNewOpportunity();
    void sOpportunities();
    void sOpportunitySources();
    void sOpportunityStages();
    void sOpportunityTypes();
    void sEditOwners();

  private:
    GUIClient *parent;

    QToolBar   *toolBar;
    QMenu *crmMenu;
    QMenu *projectsMenu;
    QMenu *incidentMenu;
    QMenu *todoMenu;
    QMenu *reportsMenu;
    QMenu *accountsMenu;
    QMenu *contactsMenu;
    QMenu *addressMenu;
    QMenu *utilitiesMenu;
    QMenu *masterMenu;
    QMenu *masterIncdMenu;
    QMenu *opportunityMenu;
    QMenu *masterOppMenu;
    
    void	addActionsToMenu(actionProperties [], unsigned int);
};

#endif

