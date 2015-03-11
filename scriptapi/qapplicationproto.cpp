/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qapplicationproto.h"

#include <QInputContext>
#include <QString>
#include <QSymbianEvent>
#include <QStyle>
#include <QRgb>

QScriptValue QApplicationtoScriptValue(QScriptEngine *engine, QApplication* const &item)
{ return engine->newQObject(item); }

void QApplicationfromScriptValue(const QScriptValue &obj, QApplication* &item)
{
  item = qobject_cast<QApplication*>(obj.toQObject());
}

void setupQApplicationProto(QScriptEngine *engine)
{
 qScriptRegisterMetaType(engine, QApplicationtoScriptValue, QApplicationfromScriptValue);

  QScriptValue proto = engine->newQObject(new QApplicationProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QApplication*>(), proto);
  /*
  QScriptValue constructor = engine->newFunction(constructQApplication,
                                                 proto);
  engine->globalObject().setProperty("QApplication", constructor);
  */
  engine->globalObject().setProperty("QApplication", engine->newQObject(qApp));
}

QScriptValue constructQApplication(QScriptContext  *context,
                                    QScriptEngine  *engine)
{
  Q_UNUSED(engine);
  context->throwError(QScriptContext::UnknownError,
                      QString("Cannot construct a QApplication in a script!"));
  return QScriptValue();
}

QApplicationProto::QApplicationProto(QObject *parent)
    : QObject(parent)
{
}

void QApplicationProto::commitData(QSessionManager &manager)
{
  QApplication *item = qscriptvalue_cast<QApplication*>(thisObject());
  if (item)
    item->commitData(manager);
}

QInputContext *QApplicationProto::inputContext() const
{
  QApplication *item = qscriptvalue_cast<QApplication*>(thisObject());
  if (item)
    return item->inputContext();
  return 0;
}

bool QApplicationProto::isSessionRestored() const
{
  QApplication *item = qscriptvalue_cast<QApplication*>(thisObject());
  if (item)
    return item->isSessionRestored();
  return false;
}

#ifdef Q_WS_MAC
bool QApplicationProto::macEventFilter(EventHandlerCallRef caller, EventRef event)
{
  QApplication *item = qscriptvalue_cast<QApplication*>(thisObject());
  if (item)
    return item->macEventFilter(caller, event);
  return false;
}
#endif

bool QApplicationProto::notify(QObject *receiver, QEvent *e)
{
  QApplication *item = qscriptvalue_cast<QApplication*>(thisObject());
  if (item)
    return item->notify(receiver, e);
  return false;
}

#ifdef Q_WS_QWS
bool QApplicationProto::qwsEventFilter(QWSEvent *event)
{
  QApplication *item = qscriptvalue_cast<QApplication*>(thisObject());
  if (item)
    return item->qwsEventFilter(event);
  return false;
}

void QApplicationProto::qwsSetCustomColors(QRgb *colorTable, int start, int numColors)
{
  QApplication *item = qscriptvalue_cast<QApplication*>(thisObject());
  if (item)
    item->qwsSetCustomColors(colorTable, start, numColors);
}
#endif

void QApplicationProto::saveState(QSessionManager &manager)
{
  QApplication *item = qscriptvalue_cast<QApplication*>(thisObject());
  if (item)
    item->saveState(manager);
}

QString QApplicationProto::sessionId() const
{
  QApplication *item = qscriptvalue_cast<QApplication*>(thisObject());
  if (item)
    return item->sessionId();
  return QString();
}

QString QApplicationProto::sessionKey() const
{
  QApplication *item = qscriptvalue_cast<QApplication*>(thisObject());
  if (item)
    return item->sessionKey();
  return QString();
}

void QApplicationProto::setInputContext(QInputContext* inputContext)
{
  QApplication *item = qscriptvalue_cast<QApplication*>(thisObject());
  if (item)
    item->setInputContext(inputContext);
}

#ifdef Q_OS_SYMBIAN
bool QApplicationProto::symbianEventFilter(const QSymbianEvent *event)
{
  QApplication *item = qscriptvalue_cast<QApplication*>(thisObject());
  if (item)
    return item->symbianEventFilter(event);
  return false;
}

int QApplicationProto::symbianProcessEvent(const QSymbianEvent *event)
{
  QApplication *item = qscriptvalue_cast<QApplication*>(thisObject());
  if (item)
    return item->symbianProcessEvent(event);
  return 0;
}
#endif

#ifdef Q_WS_X11
bool QApplicationProto::x11EventFilter(XEvent *event)
{
  QApplication *item = qscriptvalue_cast<QApplication*>(thisObject());
  if (item)
    return item->x11EventFilter(event);
  return false;
}

int QApplicationProto::x11ProcessEvent(XEvent *event)
{
  QApplication *item = qscriptvalue_cast<QApplication*>(thisObject());
  if (item)
    return item->x11ProcessEvent(event);
  return 0;
}
#endif

QWidget *QApplicationProto::activeModalWidget()
{
  return qApp->activeModalWidget();
}

QWidget *QApplicationProto::activePopupWidget()
{
  return qApp->activePopupWidget();
}

QWidget *QApplicationProto::activeWindow()
{
  return qApp->activeWindow();
}

void QApplicationProto::alert(QWidget *widget, int msec)
{
  qApp->alert(widget, msec);
}

QWidgetList QApplicationProto::allWidgets()
{
  return qApp->allWidgets();
}

void QApplicationProto::beep()
{
  qApp->beep();
}

void QApplicationProto::changeOverrideCursor(const QCursor &cursor)
{
  qApp->changeOverrideCursor(cursor);
}

QClipboard *QApplicationProto::clipboard()
{
  return qApp->clipboard();
}

int QApplicationProto::colorSpec()
{
  return qApp->colorSpec();
}

QDesktopWidget *QApplicationProto::desktop()
{
  return qApp->desktop();
}

bool QApplicationProto::desktopSettingsAware()
{
  return qApp->desktopSettingsAware();
}

int QApplicationProto::exec()
{
  return qApp->exec();
}

QWidget *QApplicationProto::focusWidget()
{
  return qApp->focusWidget();
}

QFont QApplicationProto::font()
{
  return qApp->font();
}

QFont QApplicationProto::font(const QWidget *widget)
{
  return qApp->font(widget);
}

QFont QApplicationProto::font(const char *className)
{
  return qApp->font(className);
}

QFontMetrics QApplicationProto::fontMetrics()
{
  return qApp->fontMetrics();
}

bool QApplicationProto::isEffectEnabled(Qt::UIEffect effect)
{
  return qApp->isEffectEnabled(effect);
}

bool QApplicationProto::isLeftToRight()
{
  return qApp->isLeftToRight();
}

bool QApplicationProto::isRightToLeft()
{
  return qApp->isRightToLeft();
}

Qt::LayoutDirection QApplicationProto::keyboardInputDirection()
{
  return qApp->keyboardInputDirection();
}

QLocale QApplicationProto::keyboardInputLocale()
{
  return qApp->keyboardInputLocale();
}

Qt::KeyboardModifiers QApplicationProto::keyboardModifiers()
{
  return qApp->keyboardModifiers();
}

Qt::MouseButtons QApplicationProto::mouseButtons()
{
  return qApp->mouseButtons();
}

QCursor *QApplicationProto::overrideCursor()
{
  return qApp->overrideCursor();
}

QPalette QApplicationProto::palette()
{
  return qApp->palette();
}

QPalette QApplicationProto::palette(const QWidget *widget)
{
  return qApp->palette(widget);
}

QPalette QApplicationProto::palette(const char *className)
{
  return qApp->palette(className);
}

Qt::KeyboardModifiers QApplicationProto::queryKeyboardModifiers()
{
  return qApp->queryKeyboardModifiers();
}

#if defined(QT_WS_QWS) && !defined(Q_NO_QWS_MANAGER)
QDecoration &QApplicationProto::qwsDecoration()
{
  return qApp->qwsDecoration();
}

void QApplicationProto::qwsSetDecoration(QDecoration *decoration)
{
  qApp->qwsSetDecoration(decoration);
}

QDecoration *QApplicationProto::qwsSetDecoration(const QString &decoration)
{
  return qApp->qwsSetDecoration(decoration);
}
#endif

void QApplicationProto::restoreOverrideCursor()
{
  qApp->restoreOverrideCursor();
}

void QApplicationProto::setActiveWindow(QWidget *active)
{
  qApp->setActiveWindow(active);
}

void QApplicationProto::setColorSpec(int spec)
{
  qApp->setColorSpec(spec);
}

void QApplicationProto::setDesktopSettingsAware(bool on)
{
  qApp->setDesktopSettingsAware(on);
}

void QApplicationProto::setDoubleClickInterval(int ms)
{
  qApp->setDoubleClickInterval(ms);
}

void QApplicationProto::setEffectEnabled(Qt::UIEffect effect, bool enable)
{
  qApp->setEffectEnabled(effect, enable);
}

void QApplicationProto::setFont(const QFont &font, const char *className)
{
  qApp->setFont(font, className);
}

void QApplicationProto::setGraphicsSystem(const QString &system)
{
  qApp->setGraphicsSystem(system);
}

void QApplicationProto::setOverrideCursor(const QCursor &cursor)
{
  qApp->setOverrideCursor(cursor);
}

void QApplicationProto::setPalette(const QPalette &palette, const char *className)
{
  qApp->setPalette(palette, className);
}

void QApplicationProto::setQuitOnLastWindowClosed(bool quit)
{
  qApp->setQuitOnLastWindowClosed(quit);
}

void QApplicationProto::setStyle(QStyle *style)
{
  qApp->setStyle(style);
}

QStyle *QApplicationProto::setStyle(const QString &style)
{
  return qApp->setStyle(style);
}

QStyle *QApplicationProto::style()
{
  return qApp->style();
}

void QApplicationProto::syncX()
{
  qApp->syncX();
}

QWidget *QApplicationProto::topLevelAt(const QPoint &point)
{
  return qApp->topLevelAt(point);
}

QWidget *QApplicationProto::topLevelAt(int x, int y)
{
  return qApp->topLevelAt(x, y);
}

QWidgetList QApplicationProto::topLevelWidgets()
{
  return qApp->topLevelWidgets();
  return QWidgetList();
}

QApplication::Type QApplicationProto::type()
{
  return qApp->type();
}

QWidget *QApplicationProto::widgetAt(const QPoint &point)
{
  return qApp->widgetAt(point);
}

QWidget *QApplicationProto::widgetAt(int x, int y)
{
  return qApp->widgetAt(x, y);
}

QString QApplicationProto::toString() const
{
  return qApp->arguments().join(" ");
}

