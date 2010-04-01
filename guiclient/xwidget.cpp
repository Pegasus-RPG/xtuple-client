/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xwidget.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QDesktopWidget>
#include <QScriptEngineDebugger>
#include <QShowEvent>
#include <QWorkspace>
#include <QtScript>

#include "xtsettings.h"
#include "guiclient.h"
#include "scripttoolbox.h"

#include "../scriptapi/qeventproto.h"
#include "../scriptapi/parameterlistsetup.h"

//
// XWidgetPrivate
//
class XWidgetPrivate
{
  friend class XWidget;

  public:
    XWidgetPrivate();
    ~XWidgetPrivate();

    bool _shown;
    bool _scriptLoaded;
    QScriptEngine * _engine;
    QScriptEngineDebugger * _debugger;
};

XWidgetPrivate::XWidgetPrivate()
{
  _shown = false;
  _scriptLoaded = false;
  _engine = 0;
}

XWidgetPrivate::~XWidgetPrivate()
{
  if(_engine)
    delete _engine;
}

XWidget::XWidget(QWidget * parent, Qt::WindowFlags flags)
  : QWidget(parent,
            ((flags & (Qt::Dialog | Qt::Window)) && parent && parent->isModal()) ? (flags | Qt::Dialog)
                  : flags )
{
  if(parent && parent->isModal() && (flags & (Qt::Dialog | Qt::Window)))
  {
    setWindowModality(Qt::ApplicationModal);
  }

  _private = new XWidgetPrivate();
  ScriptToolbox::setLastWindow(this);
}

XWidget::XWidget(QWidget * parent, const char * name, Qt::WindowFlags flags)
  : QWidget(parent,
            ((flags & (Qt::Dialog | Qt::Window)) && parent && parent->isModal()) ? (flags | Qt::Dialog)
                  : flags )
{
  if(parent && parent->isModal() && (flags & (Qt::Dialog | Qt::Window)))
  {
    setWindowModality(Qt::ApplicationModal);
    //setWindowFlags(windowFlags() | Qt::Dialog);
  }

  if(name)
    setObjectName(name);

  _private = new XWidgetPrivate();
}

XWidget::~XWidget()
{
  if(_private)
    delete _private;
}

void XWidget::closeEvent(QCloseEvent *event)
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

void XWidget::showEvent(QShowEvent *event)
{
  if(!_private->_shown)
  {
    _private->_shown = true;
    if (windowFlags() & (Qt::Window | Qt::Dialog))
    {
      QRect availableGeometry = QApplication::desktop()->availableGeometry();
      if(!omfgThis->showTopLevel() && !isModal())
        availableGeometry = QRect(QPoint(0, 0), omfgThis->workspace()->size());

      QString objName = objectName();
      QPoint pos = xtsettingsValue(objName + "/geometry/pos").toPoint();
      QSize lsize = xtsettingsValue(objName + "/geometry/size").toSize();

      if(lsize.isValid() && xtsettingsValue(objName + "/geometry/rememberSize", true).toBool())
        resize(lsize);

      setAttribute(Qt::WA_DeleteOnClose);
      if(omfgThis->showTopLevel() || isModal())
      {
        omfgThis->_windowList.append(this);
        QRect r(pos, size());
        if(!pos.isNull() && availableGeometry.contains(r) && xtsettingsValue(objName + "/geometry/rememberPos", true).toBool())
          move(pos);
      }
      else
      {
        QWidget * fw = focusWidget();
        omfgThis->workspace()->addWindow(this);
        QRect r(pos, size());
        if(!pos.isNull() && availableGeometry.contains(r) && xtsettingsValue(objName + "/geometry/rememberPos", true).toBool() && parentWidget())
          parentWidget()->move(pos);
        // This originally had to be after the show? Will it work here?
        if(fw)
          fw->setFocus();
      }
    }

    loadScriptEngine();

    QList<XCheckBox*> allxcb = findChildren<XCheckBox*>();
    for (int i = 0; i < allxcb.size(); ++i)
      allxcb.at(i)->init();
  }
  QWidget::showEvent(event);
}

enum SetResponse XWidget::set( const ParameterList & pParams )
{
  _lastSetParams = pParams;

  loadScriptEngine();

  QTimer::singleShot(0, this, SLOT(postSet()));

  return NoError;
}

enum SetResponse XWidget::postSet()
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

ParameterList XWidget::get() const
{
  return _lastSetParams;
}

void XWidget::loadScriptEngine()
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
    qDebug() << "Looking for a script on widget " << oName;
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
      QString script = scriptHandleIncludes(scriptq.value("script_source").toString());
      if(!_private->_engine)
      {
        _private->_engine = new QScriptEngine();
        if (_preferences->boolean("EnableScriptDebug"))
        {
          _private->_debugger = new QScriptEngineDebugger(this);
          _private->_debugger->attachTo(_private->_engine);
        }
        omfgThis->loadScriptGlobals(_private->_engine);
        QScriptValue mywindow = _private->_engine->newQObject(this);
        _private->_engine->globalObject().setProperty("mywindow", mywindow);
      }

      QScriptValue result = _private->_engine->evaluate(script, objectName());
      if (_private->_engine->hasUncaughtException())
      {
        int line = _private->_engine->uncaughtExceptionLineNumber();
        qDebug() << "uncaught exception at line" << line << ":" << result.toString();
      }
    }
  }
}
