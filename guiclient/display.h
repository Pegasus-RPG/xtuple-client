/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "xwidget.h"

class QTreeWidgetItem;
class XTreeWidget;
class displayPrivate;

class display : public XWidget
{
    Q_OBJECT

    friend class displayPrivate;

public:
    display(QWidget* = 0, const char* = 0, Qt::WindowFlags = 0);
    ~display();

    virtual bool setParams(ParameterList &) = 0;

    void setReportName(const QString &);
    void setMetaSQLOptions(const QString &, const QString &);
    void setListLabel(const QString &);

public slots:
    virtual void sPrint();
    virtual void sFillList();
    virtual void sPopulateMenu(QMenu *, QTreeWidgetItem *);

protected:
    QWidget * optionsWidget();
    XTreeWidget * list();

protected slots:
    virtual void languageChange();

private:
    displayPrivate * _data;
};

#endif // __DISPLAY_H__
