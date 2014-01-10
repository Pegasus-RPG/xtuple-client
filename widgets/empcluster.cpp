/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "empcluster.h"

EmpCluster::EmpCluster(QWidget* pParent, const char* pName) :
    VirtualCluster(pParent, pName)
{
  addNumberWidget(new EmpClusterLineEdit(this, pName));
}

int EmpClusterLineEdit::idFromList(QWidget *pParent)
{
  return EmpClusterLineEdit(pParent).listFactory()->exec();
}

EmpClusterLineEdit::EmpClusterLineEdit(QWidget* pParent, const char* pName) :
    VirtualClusterLineEdit(pParent, "emp", "emp_id", "emp_code", "emp_number", 0, 0, pName, "emp_active")
{
  setTitles(tr("Employee"), tr("Employees"));
  setUiName("employee");
  setEditPriv("MaintainEmployees");
  setNewPriv("MaintainEmployees");
  setViewPriv("ViewEmployees");
}

VirtualInfo *EmpClusterLineEdit::infoFactory()
{
  return new EmpInfo(this);
}

VirtualList *EmpClusterLineEdit::listFactory()
{
    return new EmpList(this);
}

VirtualSearch *EmpClusterLineEdit::searchFactory()
{
    return new EmpSearch(this);
}

EmpInfo::EmpInfo(QWidget *pParent, Qt::WindowFlags pFlags) : VirtualInfo(pParent, pFlags)
{
  _numberLit->setText(tr("Code:"));
  _nameLit->setText(tr("Number:"));
}

EmpList::EmpList(QWidget *pParent, Qt::WindowFlags pFlags) : VirtualList(pParent, pFlags)
{
  QTreeWidgetItem *hitem = _listTab->headerItem();
  hitem->setText(0, tr("Code"));
  hitem->setText(1, tr("Number"));
}

EmpSearch::EmpSearch(QWidget *pParent, Qt::WindowFlags pFlags) : VirtualSearch(pParent, pFlags)
{
  _searchNumber->setText(tr("Search through Codes"));
  _searchName->setText(tr("Search through Numbers"));
}


// script exposure ////////////////////////////////////////////////////////////

QScriptValue EmpClusterLineEdittoScriptValue(QScriptEngine *engine, EmpClusterLineEdit* const &item)
{
  return engine->newQObject(item);
}

void EmpClusterLineEditfromScriptValue(const QScriptValue &obj, EmpClusterLineEdit* &item)
{
  item = qobject_cast<EmpClusterLineEdit*>(obj.toQObject());
}

QScriptValue constructEmpClusterLineEdit(QScriptContext *context,
                                         QScriptEngine  *engine)
{
  EmpClusterLineEdit *obj = 0;

  if (context->argumentCount() == 1 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)))
    obj = new EmpClusterLineEdit(qscriptvalue_cast<QWidget*>(context->argument(0)));

  else if (context->argumentCount() >= 2 &&
           qscriptvalue_cast<QWidget*>(context->argument(0)))
    obj = new EmpClusterLineEdit(qscriptvalue_cast<QWidget*>(context->argument(0)),
                                 qPrintable(context->argument(1).toString()));

  else
    context->throwError(QScriptContext::UnknownError,
                        "could not find an appropriate EmpClusterLineEdit constructor");

  return engine->toScriptValue(obj);
}

void setupEmpClusterLineEdit(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, EmpClusterLineEdittoScriptValue, EmpClusterLineEditfromScriptValue);

  QScriptValue widget = engine->newFunction(constructEmpClusterLineEdit);

//  widget.setProperty("EmpAll",     QScriptValue(engine, EmpClusterLineEdit::EmpAll),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
//  widget.setProperty("EmpActive",  QScriptValue(engine, EmpClusterLineEdit::EmpActive),  QScriptValue::ReadOnly | QScriptValue::Undeletable);
//  widget.setProperty("EmpInactive",QScriptValue(engine, EmpClusterLineEdit::EmpInactive),QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("EmpClusterLineEdit", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}

QScriptValue EmpClustertoScriptValue(QScriptEngine *engine, EmpCluster* const &item)
{
  return engine->newQObject(item);
}

void EmpClusterfromScriptValue(const QScriptValue &obj, EmpCluster* &item)
{
  item = qobject_cast<EmpCluster*>(obj.toQObject());
}

QScriptValue constructEmpCluster(QScriptContext *context,
                                 QScriptEngine  *engine)
{
  EmpCluster *obj = 0;

  if (context->argumentCount() == 1 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)))
    obj = new EmpCluster(qscriptvalue_cast<QWidget*>(context->argument(0)));

  else if (context->argumentCount() >= 2 &&
           qscriptvalue_cast<QWidget*>(context->argument(0)))
    obj = new EmpCluster(qscriptvalue_cast<QWidget*>(context->argument(0)),
                         qPrintable(context->argument(1).toString()));

  else
    context->throwError(QScriptContext::UnknownError,
                        "could not find an appropriate EmpCluster constructor");

  return engine->toScriptValue(obj);
}

void setupEmpCluster(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, EmpClustertoScriptValue, EmpClusterfromScriptValue);

  QScriptValue widget = engine->newFunction(constructEmpCluster);

  engine->globalObject().setProperty("EmpCluster", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
