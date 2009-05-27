/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qactionproto.h"

#include <QActionGroup>
#include <QGraphicsWidget>
#include <QIcon>
#include <QString>

#define DEBUG true

void setupQActionProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QActionProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QAction*>(), proto);
  // engine->setDefaultPrototype(qMetaTypeId<QAction>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQAction,
                                                 proto);
  engine->globalObject().setProperty("QAction", constructor);
}

QScriptValue constructQAction(QScriptContext *context,
                              QScriptEngine  *engine)
{
  QAction *obj = 0;

  if (context->argumentCount() == 1 && context->argument(0).isQObject())
  {
    if (DEBUG) qDebug("constructQAction(): calling QAction(QObject)");
    obj = new QAction(context->argument(0).toQObject());
  }
  else if (context->argumentCount() == 2 && context->argument(1).isQObject())
  {
    if (DEBUG) qDebug("constructQAction(): calling QAction(%s, QObject)",
                      qPrintable(context->argument(0).toString()));
    obj = new QAction(context->argument(0).toString(),
                      context->argument(1).toQObject());
  }
  else if (context->argumentCount() == 3 &&
           context->argument(2).isQObject())
  {
    qWarning("QActionProto's constructQAction(): can't convert arg 0 from "
             "QScriptValue to QIcon; using QAction(%s, QObject).",
             qPrintable(context->argument(1).toString()));
    /*
    QIcon icon = dynamic_cast<QIcon>(context->argument(0).toObject());
    obj = new QAction(icon,
                      context->argument(1).toString(),
                      context->argument(2).toQObject());
    */
    obj = new QAction(context->argument(1).toString(),
                      context->argument(2).toQObject());
  }
  else if (DEBUG)
    qDebug("constructQAction(): no appropriate constructor found "
           "[ %d args ]", context->argumentCount());

  if (DEBUG) qDebug("constructQAction() returning %p", obj);
  return engine->toScriptValue(obj);
}

QActionProto::QActionProto(QObject *parent)
    : QObject(parent)
{
}

QActionGroup *QActionProto::actionGroup() const
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    return item->actionGroup();
  return 0;
}

void QActionProto::activate(int event)
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    item->activate((QAction::ActionEvent)event);
}

QList<QGraphicsWidget*> QActionProto::associatedGraphicsWidgets() const
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    return item->associatedGraphicsWidgets();
  return QList<QGraphicsWidget*>();
}

QList<QWidget*> QActionProto::associatedWidgets() const
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    return item->associatedWidgets();
  return QList<QWidget*>();
}

bool QActionProto::autoRepeat() const
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    return item->autoRepeat();
  return false;
}

QVariant QActionProto::data() const
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    return item->data();
  return QVariant();
}

QFont QActionProto::font() const
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    return item->font();
  return QFont();
}

QIcon QActionProto::icon() const
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    return item->icon();
  return QIcon();
}

QString QActionProto::iconText() const
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    return item->iconText();
  return QString();
}

bool QActionProto::isCheckable() const
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    return item->isCheckable();
  return false;
}

bool QActionProto::isChecked() const
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    return item->isChecked();
  return false;
}

bool QActionProto::isEnabled() const
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    return item->isEnabled();
  return false;
}

bool QActionProto::isIconVisibleInMenu() const
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    return item->isIconVisibleInMenu();
  return false;
}

bool QActionProto::isSeparator() const
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    return item->isSeparator();
  return false;
}

bool QActionProto::isVisible() const
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    return item->isVisible();
  return false;
}

QMenu *QActionProto::menu() const
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    return item->menu();
  return 0;
}

int QActionProto::menuRole() const
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    return item->menuRole();
  return 0;
}

QWidget *QActionProto::parentWidget() const
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    return item->parentWidget();
  return 0;
}

void QActionProto::setActionGroup(QActionGroup *group)
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    item->setActionGroup(group);
}

void QActionProto::setAutoRepeat(bool b)
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    item->setAutoRepeat(b);
}

void QActionProto::setCheckable(bool b)
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    item->setCheckable(b);
}

void QActionProto::setData(const QVariant &userData)
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    item->setData(userData);
}

void QActionProto::setFont(const QFont &font)
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    item->setFont(font);
}

void QActionProto::setIcon(const QIcon &icon)
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    item->setIcon(icon);
}

void QActionProto::setIconText(const QString &text)
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    item->setIconText(text);
}

void QActionProto::setIconVisibleInMenu(bool visible)
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    item->setIconVisibleInMenu(visible);
}

void QActionProto::setMenu(QMenu *menu)
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    item->setMenu(menu);
}

void QActionProto::setMenuRole(int menuRole)
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    item->setMenuRole((QAction::MenuRole)menuRole);
}

void QActionProto::setSeparator(bool b)
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    item->setSeparator(b);
}

void QActionProto::setShortcut(const QKeySequence &shortcut)
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    item->setShortcut(shortcut);
}

void QActionProto::setShortcutContext(int context)
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    item->setShortcutContext((Qt::ShortcutContext)context);
}

void QActionProto::setShortcuts(const QList<QKeySequence> &shortcuts)
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    item->setShortcuts(shortcuts);
}

void QActionProto::setShortcuts(int key)
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    item->setShortcuts((QKeySequence::StandardKey)key);
}

void QActionProto::setStatusTip(const QString &statusTip)
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    item->setStatusTip(statusTip);
}

void QActionProto::setText(const QString &text)
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    item->setText(text);
}

void QActionProto::setToolTip(const QString &tip)
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    item->setToolTip(tip);
}

void QActionProto::setWhatsThis(const QString &what)
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    item->setWhatsThis(what);
}

QKeySequence QActionProto::shortcut() const
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    return item->shortcut();
  return QKeySequence();
}

int QActionProto::shortcutContext() const
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    return item->shortcutContext();
  return 0;
}

QList<QKeySequence> QActionProto::shortcuts() const
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    return item->shortcuts();
  return QList<QKeySequence>();
}

bool QActionProto::showStatusText(QWidget *widget)
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    return item->showStatusText(widget);
  return false;
}

QString QActionProto::statusTip() const
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    return item->statusTip();
  return QString();
}

QString QActionProto::text() const
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    return item->text();
  return QString();
}

QString QActionProto::toolTip() const
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    return item->toolTip();
  return QString();
}

QString QActionProto::whatsThis() const
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    return item->whatsThis();
  return QString();
}

/*
QString QActionProto::toString() const
{
  QAction *item = qscriptvalue_cast<QAction*>(thisObject());
  if (item)
    return item->toString();
  return QString();
}
*/
