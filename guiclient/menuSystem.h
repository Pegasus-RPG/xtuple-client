/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
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

class menuSystem : public QObject
{
  Q_OBJECT

  struct actionProperties {
    const char*		actionName;
    const QString	actionTitle;
    const char*		slot;
    QMenu*		menu;
    QString		priv;
    QPixmap*		pixmap;
    QToolBar*		toolBar;
    bool		visible;
  };

  public:
    menuSystem(GUIClient *);

  public slots:
    void sSetup();

    void sEventManager();
    void sPreferences();
    void sHotKeys();
    void sRescanPrivileges();
    void sMaintainUsers();
    void sUserPrivileges();
    void sMaintainGroups();
    void sNewEmployee();
    void sListEmployees();
    void sSearchEmployees();
    void sEmployeeGroups();
    void sErrorLog();

    void sCustomCommands();
    void sScripts();
    void sUIForms();
    void sPackages();
    void sMetasqls();
    void sReports();

    void sFixACL();
    void sFixSerial();
    void sProcessManager();
    void sExportData();
    void sImportData();
    void sCSVAtlases();

    void sPrintAlignment();

    void sExit();

    void sAbout();
    void sReferenceGuide();

    void sCommunityHome();
    void sCommunityxTupleU();
    void sCommunityForums();
    void sCommunityIssues();
    void sCommunityBlogs();
    void sCommunityXchange();


  private:
    GUIClient *parent;

    QToolBar *toolBar;

    QMenu *communityMenu;
    QMenu *designMenu;
    QMenu *employeeMenu;
    QMenu *helpMenu;
    QMenu *masterInfoMenu;
    QMenu *sysUtilsMenu;
    QMenu *systemMenu;

    void	addActionsToMenu(actionProperties [], unsigned int);
    bool        loadCSVPlugin();

};

#endif
