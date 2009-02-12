/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xwidget.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QWorkspace>
#include <QCloseEvent>
#include <QShowEvent>
#include <QDebug>

#include "xtsettings.h"
#include "guiclient.h"
#include "scripttoolbox.h"

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
    QScriptEngine * _engine;
};

XWidgetPrivate::XWidgetPrivate()
{
  _shown = false;
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
  QString objName = objectName();
  xtsettingsSetValue(objName + "/geometry/size", size());
  if(omfgThis->showTopLevel() || isModal())
    xtsettingsSetValue(objName + "/geometry/pos", pos());
  else
    xtsettingsSetValue(objName + "/geometry/pos", parentWidget()->pos());

  QWidget::closeEvent(event);
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

    QStringList parts = objectName().split(" ");
    QStringList search_parts;
    QString oName;
    while(!parts.isEmpty())
    {
      search_parts.append(parts.takeFirst());
      oName = search_parts.join(" ");
      // load and run an QtScript that applies to this window
      qDebug() << "Looking for a script on widget " << oName;
      q.prepare("SELECT script_source, script_order"
                "  FROM script"
                " WHERE((script_name=:script_name)"
                "   AND (script_enabled))"
                " ORDER BY script_order;");
      q.bindValue(":script_name", oName);
      q.exec();
      while(q.next())
      {
        QString script = scriptHandleIncludes(q.value("script_source").toString());
        if(!_private->_engine)
        {
          _private->_engine = new QScriptEngine();
          omfgThis->loadScriptGlobals(_private->_engine);
          QScriptValue mywindow = _private->_engine->newQObject(this);
          _private->_engine->globalObject().setProperty("mywindow", mywindow);
        }
  
        QScriptValue result = _private->_engine->evaluate(script);
        if (_private->_engine->hasUncaughtException())
        {
          int line = _private->_engine->uncaughtExceptionLineNumber();
          qDebug() << "uncaught exception at line" << line << ":" << result.toString();
        }
      }
    }
  }
  QWidget::showEvent(event);
}
