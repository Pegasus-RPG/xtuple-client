/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xmainwindow.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QWorkspace>
#include <QStatusBar>
#include <QCloseEvent>
#include <QShowEvent>
#include <QtScript>
#include <QDebug>
#include <QScriptEngineDebugger>

#include "xtsettings.h"
#include "guiclient.h"
#include "scripttoolbox.h"

#include "../scriptapi/qeventproto.h"
#include "../scriptapi/parameterlistsetup.h"

//
// XMainWindowPrivate
//
class XMainWindowPrivate
{
  friend class XMainWindow;

  public:
    XMainWindowPrivate();
    ~XMainWindowPrivate();

    bool _shown;
    bool _scriptLoaded;
    QScriptEngine * _engine;
    QScriptEngineDebugger * _debugger;
    QAction *_action;
};

XMainWindowPrivate::XMainWindowPrivate()
{
  _shown = false;
  _scriptLoaded = false;
  _engine = 0;
  _action = 0;
}

XMainWindowPrivate::~XMainWindowPrivate()
{
  if(_engine)
    delete _engine;
  if(_action)
    delete _action;
}

XMainWindow::XMainWindow(QWidget * parent, Qt::WindowFlags flags)
  : QMainWindow(parent, flags)
{
  _private = new XMainWindowPrivate();

  _private->_action = new QAction(this);
  _private->_action->setShortcutContext(Qt::ApplicationShortcut);
  _private->_action->setText(windowTitle());
  _private->_action->setCheckable(true);
  connect(_private->_action, SIGNAL(triggered(bool)), this, SLOT(showMe(bool)));

  ScriptToolbox::setLastWindow(this);
}

XMainWindow::XMainWindow(QWidget * parent, const char * name, Qt::WindowFlags flags)
  : QMainWindow(parent, flags)
{
  if(name)
    setObjectName(name);

  _private = new XMainWindowPrivate();

  _private->_action = new QAction(this);
  _private->_action->setShortcutContext(Qt::ApplicationShortcut);
  _private->_action->setText(windowTitle());
  _private->_action->setCheckable(true);
  connect(_private->_action, SIGNAL(triggered(bool)), this, SLOT(showMe(bool)));
}

XMainWindow::~XMainWindow()
{
  if(_private)
    delete _private;
}

enum SetResponse XMainWindow::set(const ParameterList &pParams)
{
  _lastSetParams = pParams;

  loadScriptEngine();
  QTimer::singleShot(0, this, SLOT(postSet()));

  return NoError;
}

enum SetResponse XMainWindow::postSet()
{
  loadScriptEngine();

  enum SetResponse returnValue = NoError;
  if(_private->_engine && _private->_engine->globalObject().property("set").isFunction())
  {
    QScriptValueList args;
    args << ParameterListtoScriptValue(_private->_engine, _lastSetParams);
    QScriptValue tmp = _private->_engine->globalObject().property("set").call(QScriptValue(), args);
    SetResponsefromScriptValue(tmp, returnValue);
  }

  return returnValue;
}

ParameterList XMainWindow::get() const
{
  return _lastSetParams;
}

void XMainWindow::closeEvent(QCloseEvent *event)
{
  event->accept(); // we have no reason not to accept and let the script change it if needed
  if(_private->_engine && (_private->_engine->globalObject().property("closeEvent").isFunction()))
  {
    QScriptValueList args;
    args << _private->_engine->toScriptValue((QEvent*)event);
    _private->_engine->globalObject().property("closeEvent").call(QScriptValue(), args);
  }

  if(event->isAccepted())
  {
    QString objName = objectName();
    xtsettingsSetValue(objName + "/geometry/size", size());
    if(omfgThis->showTopLevel() || isModal())
      xtsettingsSetValue(objName + "/geometry/pos", pos());
    else
      xtsettingsSetValue(objName + "/geometry/pos", parentWidget()->pos());
  }
}

void XMainWindow::showEvent(QShowEvent *event)
{
  if(!_private->_shown)
  {
    _private->_shown = true;
//qDebug("isModal() %s", isModal()?"true":"false");

    QRect availableGeometry = QApplication::desktop()->availableGeometry();
    if(!omfgThis->showTopLevel() && !isModal())
      availableGeometry = omfgThis->workspace()->geometry();

    QString objName = objectName();
    QPoint pos = xtsettingsValue(objName + "/geometry/pos").toPoint();
    QSize lsize = xtsettingsValue(objName + "/geometry/size").toSize();

    if(lsize.isValid() && xtsettingsValue(objName + "/geometry/rememberSize", true).toBool() && (metaObject()->className() != QString("xTupleDesigner")))
      resize(lsize);

    setAttribute(Qt::WA_DeleteOnClose);
    if(omfgThis->showTopLevel() || isModal())
    {
      omfgThis->_windowList.append(this);
      statusBar()->show();
      QRect r(pos, size());
      if(!pos.isNull() && availableGeometry.contains(r) && xtsettingsValue(objName + "/geometry/rememberPos", true).toBool())
        move(pos);
    }
    else
    {
      QWidget * fw = focusWidget();
      omfgThis->workspace()->addWindow(this);
      QRect r(pos, size());
      if(!pos.isNull() && availableGeometry.contains(r) && xtsettingsValue(objName + "/geometry/rememberPos", true).toBool())
        move(pos);
      // This originally had to be after the show? Will it work here?
      if(fw)
        fw->setFocus();
    }

    loadScriptEngine();

    QList<XCheckBox*> allxcb = findChildren<XCheckBox*>();
    for (int i = 0; i < allxcb.size(); ++i)
      allxcb.at(i)->init();
  }

  bool blocked = _private->_action->blockSignals(true);
  _private->_action->setChecked(true);
  _private->_action->blockSignals(blocked);

  QMainWindow::showEvent(event);
}

void XMainWindow::hideEvent(QHideEvent * event)
{
  bool blocked = _private->_action->blockSignals(true);
  _private->_action->setChecked(false);
  _private->_action->blockSignals(blocked);

  QMainWindow::hideEvent(event);
}

QAction *XMainWindow::action() const
{
  return _private->_action;
}

void XMainWindow::changeEvent(QEvent *e)
{
  switch (e->type())
  {
    case QEvent::WindowTitleChange:
        _private->_action->setText(windowTitle());
        break;
    case QEvent::WindowIconChange:
        _private->_action->setIcon(windowIcon());
        break;
    default:
        break;
  }
  QMainWindow::changeEvent(e);
}

void XMainWindow::showMe(bool v)
{
  QWidget *target = parentWidget() == 0 ? this : parentWidget();

  if (v)
    target->setWindowState(target->windowState() & ~Qt::WindowMinimized);

  target->setVisible(v);
}

void XMainWindow::loadScriptEngine()
{
  if(_private->_scriptLoaded)
    return;
  _private->_scriptLoaded = true;

  QStringList parts = objectName().split(" ");
  QStringList search_parts;
  QString oName;
  while(!parts.isEmpty())
  {
    search_parts.append(parts.takeFirst());
    oName = search_parts.join(" ");
    // load and run an QtScript that applies to this window
    qDebug() << "Looking for a script on window " << oName;
    XSqlQuery scriptq;
    scriptq.prepare("SELECT script_source, script_order"
              "  FROM script"
              " WHERE((script_name=:script_name)"
              "   AND (script_enabled))"
              " ORDER BY script_order;");
    scriptq.bindValue(":script_name", oName);
    scriptq.exec();
    while(scriptq.next())
    {
      if(engine(this))
      {
        QString script = scriptHandleIncludes(scriptq.value("script_source").toString());
        QScriptValue result = _private->_engine->evaluate(script, objectName());
        if (_private->_engine->hasUncaughtException())
        {
          int line = _private->_engine->uncaughtExceptionLineNumber();
          qDebug() << "uncaught exception at line" << line << ":" << result.toString();
        }
      }
    }
  }
}

QScriptEngine *engine(XMainWindow *win)
{
  if(win)
  {
    if(!win->_private->_engine)
    {
      win->_private->_engine = new QScriptEngine(win);
      if (_preferences->boolean("EnableScriptDebug"))
      {
        win->_private->_debugger = new QScriptEngineDebugger(win);
        win->_private->_debugger->attachTo(win->_private->_engine);
      }
      omfgThis->loadScriptGlobals(win->_private->_engine);
      QScriptValue mywindow = win->_private->_engine->newQObject(win);
      win->_private->_engine->globalObject().setProperty("mywindow", mywindow);
    }
    return win->_private->_engine;
  }
  return 0;
}

