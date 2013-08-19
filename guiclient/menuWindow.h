/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef menuWindow_h
#define menuWindow_h

#include <QObject>

class GUIClient;
class QMenu;
class MenuWindowPrivate;

class menuWindow : public QObject
{
  Q_OBJECT

  public:
    menuWindow(GUIClient *);
    ~menuWindow();

  public slots:
    void sActivateWindow();
    void sCascade();
    void sCloseActive();
    void sCloseAll();
    void sHideWindowMenu();
    void sPrepareWindowMenu();
    void sRememberPositionToggle();
    void sRememberSizeToggle();
    void sTile();

  private:
    GUIClient *_parent;
    QMenu     *_windowMenu;

    MenuWindowPrivate *_data;
};

#endif
