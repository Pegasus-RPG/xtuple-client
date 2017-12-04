/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TODOLIST_H
#define TODOLIST_H

#include "guiclient.h"
#include "display.h"
#include "xtreewidget.h"

#include "ui_todoList.h"

class todoList : public display, public Ui::todoList
{
  Q_OBJECT

  public:
    todoList(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::Window);

  public slots:
    virtual SetResponse	set(const ParameterList&);
    virtual void sDelete();
    virtual void sEdit();
    virtual void sEditParent();
    virtual void sEditTodo(int);
    virtual void sEditIncident(int);
    virtual void sEditTask(int);
    virtual void sEditProject(int);
    virtual void sEditCustomer();
    virtual void sEditOpportunity(int);
    virtual void sNew();
    virtual void sNewIncdt();
    virtual void sNewProject();
    virtual void sNewOpportunity();
    virtual void sPopulateMenu(QMenu *, QTreeWidgetItem *, int);
    virtual void sView();
    virtual void sViewParent();
    virtual void sViewTodo(int);
    virtual void sViewCustomer();
    virtual void sViewIncident(int);
    virtual void sViewTask(int);
    virtual void sViewProject(int);
    virtual void sViewOpportunity(int);
    virtual void sOpen();
    virtual bool setParams(ParameterList &);
    virtual void sFillList();

  private:
    void showEvent(QShowEvent * event);

    int		    _mode;
    bool getPriv(const int = cView, const int = 0, XTreeWidgetItem* = 0);
    int getParentType(XTreeWidgetItem*);
    int       _shown;
    int       _run;
};

#endif // TODOLIST_H
