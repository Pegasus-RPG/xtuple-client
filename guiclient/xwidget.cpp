/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xwidget.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QDebug>
#include <QShowEvent>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QDebug>
#include <QStyleOptionTitleBar>

#include "xcheckbox.h"
#include "xtsettings.h"
#include "guiclient.h"
#include "shortcuts.h"

#define DEBUG false

// Obsolete now?
class XWidgetPrivate
{
  friend class XWidget;

  public:
    XWidgetPrivate(XWidget *parent);
    virtual ~XWidgetPrivate();
};

XWidgetPrivate::XWidgetPrivate(XWidget *parent)
{
  Q_UNUSED(parent);
}

XWidgetPrivate::~XWidgetPrivate()
{
}

XWidget::XWidget(QWidget *parent, Qt::WindowFlags flags)
  : QWidget(parent,
            ((flags & (Qt::Dialog | Qt::Window)) && parent && parent->isModal()) ? (flags | Qt::Dialog)
                  : flags ),
    ScriptablePrivate(parent, this),
    _private(0)
{
  if(parent && parent->isModal() && (flags & (Qt::Dialog | Qt::Window)))
  {
    setWindowModality(Qt::ApplicationModal);
  }
}

XWidget::XWidget(QWidget * parent, const char * name, Qt::WindowFlags flags)
  : QWidget(parent,
            ((flags & (Qt::Dialog | Qt::Window)) && parent && parent->isModal()) ? (flags | Qt::Dialog)
                  : flags ),
    ScriptablePrivate(parent, this),
    _private(0)
{
  if(parent && parent->isModal() && (flags & (Qt::Dialog | Qt::Window)))
  {
    setWindowModality(Qt::ApplicationModal);
  }

  if(name)
    setObjectName(name);
}

XWidget::~XWidget()
{
  if (_private)
    delete _private;
}

void XWidget::closeEvent(QCloseEvent *event)
{
  event->accept(); // we have no reason not to accept and let the script change it if needed
  callCloseEvent(event);

  if(event->isAccepted())
  {
    QString objName = objectName();
    if (omfgThis->showTopLevel() || isModal()) {
      xtsettingsSetValue(objName + "/geometry/size", size());
      xtsettingsSetValue(objName + "/geometry/pos",  pos());
    } else {
      xtsettingsSetValue(objName + "/geometry/size", parentWidget()->size());
      xtsettingsSetValue(objName + "/geometry/pos",  parentWidget()->pos());
    }
  }
}

void XWidget::showEvent(QShowEvent *event)
{
  if (! _shown)
  {
    _shown = true;
    if (windowFlags() & (Qt::Window | Qt::Dialog))
    {
      QRect availableGeometry = QApplication::desktop()->availableGeometry();
      if(!omfgThis->showTopLevel() && !isModal())
        availableGeometry = QRect(QPoint(0, 0), omfgThis->workspace()->size());

      QString objName = objectName();
      QPoint pos = xtsettingsValue(objName + "/geometry/pos").toPoint();
      QSize lsize = xtsettingsValue(objName + "/geometry/size").toSize();
      QSize currsize = size();

      setAttribute(Qt::WA_DeleteOnClose);
      if(omfgThis->showTopLevel() || isModal())
      {
        if(lsize.isValid() && xtsettingsValue(objName + "/geometry/rememberSize", true).toBool())
          resize(lsize);
        omfgThis->_windowList.append(this);
        QRect r(pos, size());
        if(!pos.isNull() && availableGeometry.contains(r) && xtsettingsValue(objName + "/geometry/rememberPos", true).toBool())
          move(pos);
        else if(currsize!=size())
          move(QPoint(1, 1));
      }
      else
      {
        QWidget * fw = focusWidget();

	// this verboseness works around what appear to be qt bugs
        QMdiSubWindow *subwin = new QMdiSubWindow();
        subwin->setParent(omfgThis->workspace());
        omfgThis->workspace()->addSubWindow(subwin);
        subwin->setWidget(this);

        omfgThis->workspace()->setActiveSubWindow(subwin);
        connect(this, SIGNAL(destroyed(QObject*)), subwin, SLOT(close()));
        if(lsize.isValid() && xtsettingsValue(objName + "/geometry/rememberSize", true).toBool()) {
	  if (DEBUG) qDebug() << "resize()" << lsize;
          subwin->resize(lsize);
	}
        QRect r(pos, lsize);
        if(!pos.isNull() && availableGeometry.contains(r) && xtsettingsValue(objName + "/geometry/rememberPos", true).toBool() && parentWidget()) {
	  if (DEBUG) qDebug() << "move()" << pos;
          parentWidget()->move(pos);
	}
        if(fw)
          fw->setFocus();
      }
    }

    loadScriptEngine();

    QList<XCheckBox*> allxcb = findChildren<XCheckBox*>();
    for (int i = 0; i < allxcb.size(); ++i)
      allxcb.at(i)->init();

    shortcuts::setStandardKeys(this);
  }

  callShowEvent(event);
  QWidget::showEvent(event);
}

enum SetResponse XWidget::set(const ParameterList & pParams)
{
  _lastSetParams = pParams;
  loadScriptEngine();
  QTimer::singleShot(0, this, SLOT(postSet()));
  return NoError;
}

enum SetResponse XWidget::postSet()
{
  return callSet(_lastSetParams);
}

ParameterList XWidget::get() const
{
  return _lastSetParams;
}
