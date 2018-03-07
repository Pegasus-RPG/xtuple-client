/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xmainwindow.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QStatusBar>
#include <QCloseEvent>
#include <QShowEvent>
#include <QDebug>

#include "xcheckbox.h"
#include "xtsettings.h"
#include "scriptablePrivate.h"
#include "shortcuts.h"

#define DEBUG false

class XMainWindowPrivate
{
  friend class XMainWindow;

  public:
    XMainWindowPrivate(XMainWindow*);
    ~XMainWindowPrivate();
};

XMainWindowPrivate::XMainWindowPrivate(XMainWindow *parent)
{
  Q_UNUSED(parent);
}

XMainWindowPrivate::~XMainWindowPrivate()
{
}

XMainWindow::XMainWindow(QWidget * parent, Qt::WindowFlags flags)
  : QMainWindow(parent, flags),
    ScriptablePrivate(parent, this),
    _private(0)
{
  _showMe = new QAction(this);
  _showMe->setShortcutContext(Qt::ApplicationShortcut);
  _showMe->setText(windowTitle());
  _showMe->setCheckable(true);
  connect(_showMe, SIGNAL(triggered(bool)), this, SLOT(showMe(bool)));
  _forceFloat=false;
}

XMainWindow::XMainWindow(QWidget * parent, const char * name, Qt::WindowFlags flags)
  : QMainWindow(parent, flags),
    ScriptablePrivate(parent, this),
    _private(0)
{
  if(name)
    setObjectName(name);

  _showMe = new QAction(this);
  _showMe->setShortcutContext(Qt::ApplicationShortcut);
  _showMe->setText(windowTitle());
  _showMe->setCheckable(true);
  connect(_showMe, SIGNAL(triggered(bool)), this, SLOT(showMe(bool)));
  _forceFloat = false;
}

XMainWindow::~XMainWindow()
{
  if(_private)
    delete _private;
}

enum SetResponse XMainWindow::set(const ParameterList & pParams)
{
  _lastSetParams = pParams;
  loadScriptEngine();
  QTimer::singleShot(0, this, SLOT(postSet()));
  return NoError;
}

enum SetResponse XMainWindow::postSet()
{
  return callSet(_lastSetParams);
}

ParameterList XMainWindow::get() const
{
  return _lastSetParams;
}

void XMainWindow::closeEvent(QCloseEvent *event)
{
  event->accept(); // we have no reason not to accept and let the script change it if needed
  callCloseEvent(event);

  if(event->isAccepted())
  {
    QString objName = objectName();
    if(omfgThis->showTopLevel() || isModal()) {
      if (DEBUG) qDebug() << "saving size" << size() << "position" << pos();
      xtsettingsSetValue(objName + "/geometry/size", size());
      xtsettingsSetValue(objName + "/geometry/pos", pos());
    } else if (parentWidget() != 0) {
      if (DEBUG)
        qDebug() << "saving parent size" << parentWidget()->size() << "position" << parentWidget()->pos();
      xtsettingsSetValue(objName + "/geometry/size", parentWidget()->size());
      xtsettingsSetValue(objName + "/geometry/pos",  parentWidget()->pos());
    }
  }
}

void XMainWindow::showEvent(QShowEvent *event)
{
  if(!_shown)
  {
    _shown = true;

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
      if(lsize.isValid() && xtsettingsValue(objName + "/geometry/rememberSize", true).toBool() && (metaObject()->className() != QString("xTupleDesigner"))) {
	if (DEBUG) qDebug() << "resize" << lsize;
        resize(lsize);
      }
      omfgThis->_windowList.append(this);
      statusBar()->show();
      QRect r(pos, lsize);
      if (DEBUG) qDebug() << availableGeometry << "contains?" << r;
      if(!pos.isNull() && availableGeometry.contains(r) && xtsettingsValue(objName + "/geometry/rememberPos", true).toBool()) {
	if (DEBUG) qDebug() << "move" << pos;
        move(pos);
      }
      else if(currsize!=size())
        move(QPoint(1, 1));
    }
    else
    {
      QWidget * fw = focusWidget();
      QMdiSubWindow *subwin = omfgThis->workspace()->addSubWindow(this);
      omfgThis->workspace()->setActiveSubWindow(subwin);
      connect(this, SIGNAL(destroyed(QObject*)), subwin, SLOT(close()));
      if(lsize.isValid() && xtsettingsValue(objName + "/geometry/rememberSize", true).toBool()) {
	if (DEBUG) qDebug() << "subwin resize" << lsize;
        subwin->resize(lsize);
      }
      QRect r(pos, lsize);
      if (DEBUG) qDebug() << availableGeometry << ">" << r << "?" << availableGeometry.contains(r);
      if(!pos.isNull() && availableGeometry.contains(r) && xtsettingsValue(objName + "/geometry/rememberPos", true).toBool()) {
	if (DEBUG) qDebug() << "subwin move" << pos;
        subwin->move(pos);
      }
      // This originally had to be after the show? Will it work here?
      if(fw)
        fw->setFocus();
    }

    loadScriptEngine();

    QList<XCheckBox*> allxcb = findChildren<XCheckBox*>();
    for (int i = 0; i < allxcb.size(); ++i)
      allxcb.at(i)->init();

    shortcuts::setStandardKeys(this);
  }

  bool blocked = _showMe->blockSignals(true);
  _showMe->setChecked(true);
  _showMe->blockSignals(blocked);

  callShowEvent(event);
  QMainWindow::showEvent(event);
}

void XMainWindow::hideEvent(QHideEvent * event)
{
  bool blocked = _showMe->blockSignals(true);
  _showMe->setChecked(false);
  _showMe->blockSignals(blocked);

  QMainWindow::hideEvent(event);
}

QAction *XMainWindow::action() const
{
  return _showMe;
}

void XMainWindow::changeEvent(QEvent *e)
{
  switch (e->type())
  {
    case QEvent::WindowTitleChange:
        _showMe->setText(windowTitle());
        break;
    case QEvent::WindowIconChange:
        _showMe->setIcon(windowIcon());
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

QScriptEngine *engine(XMainWindow *win)
{
  return win ? win->engine() : 0;
}

