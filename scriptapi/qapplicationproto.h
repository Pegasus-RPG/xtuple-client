/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QAPPLICATIONPROTO_H__
#define __QAPPLICATIONPROTO_H__

#include <QApplication>
#include <QDecoration>
#include <QObject>
#include <QSessionManager>
#include <QWSEvent>
#include <QtScript>

class QInputContext;
class QString;
class QSymbianEvent;

Q_DECLARE_METATYPE(QApplication*)

void setupQApplicationProto(QScriptEngine *engine);
QScriptValue constructQApplication(QScriptContext *context, QScriptEngine *engine);

class QApplicationProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QApplicationProto(QObject *parent);

    Q_INVOKABLE void            commitData(QSessionManager &manager);
    Q_INVOKABLE QInputContext  *inputContext()                   const;
    Q_INVOKABLE bool            isSessionRestored()              const;
#ifdef Q_WS_MAC
    Q_INVOKABLE bool            macEventFilter(EventHandlerCallRef caller, EventRef event);
#endif
    Q_INVOKABLE bool            notify(QObject *receiver, QEvent *e);
#ifdef Q_WS_QWS
    Q_INVOKABLE bool            qwsEventFilter(QWSEvent *event);
    Q_INVOKABLE void            qwsSetCustomColors(QRgb *colorTable, int start, int numColors);
#endif
    Q_INVOKABLE void            saveState(QSessionManager &manager);
    Q_INVOKABLE QString         sessionId()         const;
    Q_INVOKABLE QString         sessionKey()        const;
    Q_INVOKABLE void            setInputContext(QInputContext* inputContext);
#ifdef Q_OS_SYMBIAN
    Q_INVOKABLE bool            symbianEventFilter(const QSymbianEvent *event);
    Q_INVOKABLE int             symbianProcessEvent(const QSymbianEvent *event);
#endif
#ifdef Q_WS_X11
    Q_INVOKABLE bool            x11EventFilter(XEvent *event);
    Q_INVOKABLE int             x11ProcessEvent(XEvent *event);
#endif

    Q_INVOKABLE QWidget        *activeModalWidget();
    Q_INVOKABLE QWidget        *activePopupWidget();
    Q_INVOKABLE QWidget        *activeWindow();
    Q_INVOKABLE void            alert(QWidget * widget, int msec = 0);
    Q_INVOKABLE QWidgetList     allWidgets();
    Q_INVOKABLE void            beep();
    Q_INVOKABLE void            changeOverrideCursor(const QCursor &cursor);
    Q_INVOKABLE QClipboard     *clipboard();
    Q_INVOKABLE int             colorSpec();
    Q_INVOKABLE QDesktopWidget *desktop();
    Q_INVOKABLE bool            desktopSettingsAware();
    Q_INVOKABLE int             exec();
    Q_INVOKABLE QWidget        *focusWidget();
    Q_INVOKABLE QFont           font();
    Q_INVOKABLE QFont           font(const QWidget * widget);
    Q_INVOKABLE QFont           font(const char * className);
    Q_INVOKABLE QFontMetrics    fontMetrics();
    Q_INVOKABLE bool            isEffectEnabled(Qt::UIEffect effect);
    Q_INVOKABLE bool            isLeftToRight();
    Q_INVOKABLE bool            isRightToLeft();
    Q_INVOKABLE Qt::LayoutDirection     keyboardInputDirection();
    Q_INVOKABLE QLocale         keyboardInputLocale();
    Q_INVOKABLE Qt::KeyboardModifiers   keyboardModifiers();
    Q_INVOKABLE Qt::MouseButtons        mouseButtons();
    Q_INVOKABLE QCursor        *overrideCursor();
    Q_INVOKABLE QPalette        palette();
    Q_INVOKABLE QPalette        palette(const QWidget *widget);
    Q_INVOKABLE QPalette        palette(const char *className);
    Q_INVOKABLE Qt::KeyboardModifiers   queryKeyboardModifiers();
#if defined(QT_WS_QWS) && !defined(Q_NO_QWS_MANAGER)
    Q_INVOKABLE QDecoration    &qwsDecoration();
    Q_INVOKABLE void            qwsSetDecoration(QDecoration *decoration);
    Q_INVOKABLE QDecoration    *qwsSetDecoration(const QString &decoration);
#endif
    Q_INVOKABLE void            restoreOverrideCursor();
    Q_INVOKABLE void            setActiveWindow(QWidget *active);
    Q_INVOKABLE void            setColorSpec(int spec);
    Q_INVOKABLE void            setDesktopSettingsAware(bool on);
    Q_INVOKABLE void            setDoubleClickInterval(int ms);
    Q_INVOKABLE void            setEffectEnabled(Qt::UIEffect effect, bool enable = true);
    Q_INVOKABLE void            setFont(const QFont &font, const char *className = 0);
    Q_INVOKABLE void            setGraphicsSystem(const QString &system);
    Q_INVOKABLE void            setOverrideCursor(const QCursor &cursor);
    Q_INVOKABLE void            setPalette(const QPalette &palette, const char *className = 0);
    Q_INVOKABLE void            setQuitOnLastWindowClosed(bool quit);
    Q_INVOKABLE void            setStyle(QStyle *style);
    Q_INVOKABLE QStyle         *setStyle(const QString &style);
    Q_INVOKABLE QStyle         *style();
    Q_INVOKABLE void            syncX();
    Q_INVOKABLE QWidget        *topLevelAt(const QPoint &point);
    Q_INVOKABLE QWidget        *topLevelAt(int x, int y);
    Q_INVOKABLE QWidgetList     topLevelWidgets();
    Q_INVOKABLE QApplication::Type      type();
    Q_INVOKABLE QWidget        *widgetAt(const QPoint &point);
    Q_INVOKABLE QWidget        *widgetAt(int x, int y);

    Q_INVOKABLE QString        toString()          const;
};

#endif
