/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef menuSystem_h
#define menuSystem_h

#include <QObject>
#include <QPixmap>

class QMenu;
class QPixmap;
class QToolBar;
class GUIClient;
class Action;
class CSVImpPluginInterface;

class menuSystem : public QObject
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
  };

  public:
    menuSystem(GUIClient *);

  public slots:
    void sPrepareWindowMenu();
    void sHideWindowMenu();
    void sActivateWindow();
    void sCloseAll();
    void sCloseActive();
    void sRememberPositionToggle();
    void sRememberSizeToggle();

    void sConfigureIE();
    void sConfigureIM();
    void sConfigurePD();
    void sConfigureMS();
    void sConfigureWO();
    void sConfigureSO();
    void sConfigurePO();
    void sConfigureGL();
    void sConfigureEncryption();
    void sConfigureCC();
    void sConfigureCRM();

    void sScheduleSystemMessage();
    void sEventManager();
    void sBatchManager();
    void sPreferences();
    void sHotKeys();
    void sRescanPrivileges();
    void sMaintainUsers();
    void sMaintainGroups();
    void sNewEmployee();
    void sListEmployees();
    void sSearchEmployees();
    void sEmployeeGroups();
    void sScheduleServerMaintenance();
    void sErrorLog();

    void sDatabaseInformation();

    void sImages();
    void sReports();
    void sForms();
    void sLabelForms();
    void sCalendars();
    void sCurrencies();
    void sExchangeRates();
    void sCountries();
    void sStates();
    void sLocales();
    void sCommentTypes();
    void sAccountNumbers();
    void sDepartments();
    void sCustomCommands();
    void sScripts();
    void sUIForms();
    void sPackages();
    void sMetasqls();

    void sFixACL();
    void sFixSerial();
    void sExportData();
    void sImportCSV();
    void sImportXML();

    void sPrintAlignment();

    void sExit();

    void sAbout();
    void sTOC();

    void sCommunityHome();
    //void sCommunityNewAccount();
    void sCommunityEditAccount();
    void sCommunityForums();
    void sCommunityBlogs();
    void sCommunityIssues();
    void sCommunityWiki();
    void sCommunityDownloads();
    void sRegister();
    void sCommunitySupport();
    void sCommunityTranslation();
    void sCommunityXchange();

  private:
    GUIClient *parent;

    QToolBar *toolBar;

    QMenu *communityMenu;
    QMenu *configModulesMenu;
    QMenu *designMenu;
    QMenu *employeeMenu;
    QMenu *helpMenu;
    QMenu *masterInfoMenu;
    QMenu *sysUtilsMenu;
    QMenu *systemMenu;
    QMenu *windowMenu;

    QMenu *geometryMenu;

    Action *cascade;
    Action *tile;
    Action *closeActive;
    Action *closeAll;
    Action *_rememberPos;
    Action *_rememberSize;

    QWidget *_lastActive;
    CSVImpPluginInterface *_csvimpInterface;

    void	addActionsToMenu(actionProperties [], unsigned int);
    bool        loadCSVPlugin();

};

#endif
