/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef IMPORTXML_H
#define IMPORTXML_H

#include <QDomDocument>
#include "xwidget.h"
#include <QMenu>

#include "ui_importXML.h"

class importXML : public XWidget, public Ui::importXML
{
  Q_OBJECT
  
  public:
    importXML(QWidget* = 0, Qt::WindowFlags = 0);
    ~importXML();

    static bool userHasPriv();

  public slots:
    virtual void setVisible(bool);

  protected slots:
    virtual void languageChange();
    virtual void sAdd();
    virtual void sClearStatus();
    virtual void sDelete();
    virtual void sFillList();
    virtual void sHandleAutoUpdate(const bool);
    virtual void sImportAll();
    virtual void sImportSelected();
    virtual void sPopulateMenu(QMenu*, QTreeWidgetItem*);

  private:
    QString	_defaultXMLDir;
    QString	_defaultXSLTDir;
    QString	_externalCmd;
    QStringList	_filters;
    bool	importOne(const QString &);
    bool	openDomDocument(const QString &, QDomDocument &);
};

#endif
