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

    Q_INVOKABLE virtual bool setParams(ParameterList &) = 0;

    Q_INVOKABLE void setReportName(const QString &);
    Q_INVOKABLE QString reportName() const;
    Q_INVOKABLE void setMetaSQLOptions(const QString &, const QString &);
    Q_INVOKABLE void setListLabel(const QString &);
    Q_INVOKABLE void setUseAltId(bool);
    Q_INVOKABLE bool useAltId() const;
    Q_INVOKABLE void setAutoUpdateEnabled(bool);
    Q_INVOKABLE bool autoUpdateEnabled() const;

public slots:
    virtual void sPrint();
    virtual void sPreview();
    virtual void sFillList();
    virtual void sPopulateMenu(QMenu *, QTreeWidgetItem *, int);

protected:
    Q_INVOKABLE QWidget * optionsWidget();
    Q_INVOKABLE XTreeWidget * list();
    Q_INVOKABLE ParameterList getParams();

protected slots:
    virtual void languageChange();
    virtual void sAutoUpdateToggled();

private:
    displayPrivate * _data;
};

#endif // __DISPLAY_H__
