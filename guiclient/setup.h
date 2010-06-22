/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SETUP_H
#define SETUP_H

#include "guiclient.h"
#include "xdialog.h"
#include "xt.h"
#include <parameter.h>

#include "ui_setup.h"

class setup : public XDialog, public Ui::setup
{
    Q_OBJECT

public:
    setup(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~setup();

    enum SetupTypes
    {
      Configure, AccountMapping, MasterInformation
    };

public slots:
    enum SetResponse set(const ParameterList & pParams );
    void insert(const QString &title,
                const QString &uiName,
                const SetupTypes type,
                int modules,
                bool enabled = true,
                int mode = 0,
                const QString &saveMethod = QString());
    void apply();
    void languageChange();
    int mode(const QString &editPriv, const QString &viewPriv = QString());
    void populate();
    void save(bool close = true);

signals:
    void saving();

private slots:
    void setCurrentIndex(XTreeWidgetItem* item);

private:
 //   int                         _mode;
 //   QString                     _module;
    struct ItemProps {
      QString uiName;
      SetupTypes type;
      int modules;
      bool enabled;
      int mode;
      QString saveMethod;
    };
    QMap<QString, ItemProps>    _itemMap;
    QMap<QString, int>          _idxmap;
    QMap<QString, QString>      _methodMap;        

    XTreeWidgetItem*            _configItem;
    XTreeWidgetItem*            _mapItem;
    XTreeWidgetItem*            _masterItem;
};

#endif // SETUP_H
