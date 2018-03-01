/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "scriptapi_internal.h"
#include "qeventproto.h"

#include <QDebug>
#include <QString>

#define DEBUG false

QScriptValue constructQEvent(QScriptContext *context,
                            QScriptEngine  *engine)
{
  QEvent *obj = 0;

  if (DEBUG)
  {
    qDebug("constructQEvent() entered");
    for (int i = 0; i < context->argumentCount(); i++)
      qDebug() << "context->argument(" << i << ") = " << context->argument(i).toVariant();
  }

  if (context->argumentCount() == 1)
    obj = new QEvent(qscriptvalue_cast<QEvent::Type>(context->argument(0)));
  else
    obj = new QEvent(QEvent::None);

  return engine->toScriptValue(obj);
}

void setupQEventProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QEventProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QEvent*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQEvent, proto);
  engine->globalObject().setProperty("QEvent", constructor);

  constructor.setProperty("None",                             QScriptValue(engine, QEvent::None),                             ENUMPROPFLAGS);
  constructor.setProperty("ActionAdded",                      QScriptValue(engine, QEvent::ActionAdded),                      ENUMPROPFLAGS);
  constructor.setProperty("ActionChanged",                    QScriptValue(engine, QEvent::ActionChanged),                    ENUMPROPFLAGS);
  constructor.setProperty("ActivationChange",                 QScriptValue(engine, QEvent::ActivationChange),                 ENUMPROPFLAGS);
  constructor.setProperty("ApplicationActivate",              QScriptValue(engine, QEvent::ApplicationActivate),              ENUMPROPFLAGS);
  constructor.setProperty("ApplicationActivated",             QScriptValue(engine, QEvent::ApplicationActivated),             ENUMPROPFLAGS);
  constructor.setProperty("ApplicationDeactivate",            QScriptValue(engine, QEvent::ApplicationDeactivate),            ENUMPROPFLAGS);
  constructor.setProperty("ApplicationFontChange",            QScriptValue(engine, QEvent::ApplicationFontChange),            ENUMPROPFLAGS);
  constructor.setProperty("ApplicationLayoutDirectionChange", QScriptValue(engine, QEvent::ApplicationLayoutDirectionChange), ENUMPROPFLAGS);
  constructor.setProperty("ApplicationPaletteChange",         QScriptValue(engine, QEvent::ApplicationPaletteChange),         ENUMPROPFLAGS);
  constructor.setProperty("ApplicationStateChange",           QScriptValue(engine, QEvent::ApplicationStateChange),           ENUMPROPFLAGS);
  constructor.setProperty("ApplicationWindowIconChange",      QScriptValue(engine, QEvent::ApplicationWindowIconChange),      ENUMPROPFLAGS);
  constructor.setProperty("ChildAdded",                       QScriptValue(engine, QEvent::ChildAdded),                       ENUMPROPFLAGS);
  constructor.setProperty("ChildPolished",                    QScriptValue(engine, QEvent::ChildPolished),                    ENUMPROPFLAGS);
  constructor.setProperty("ChildRemoved",                     QScriptValue(engine, QEvent::ChildRemoved),                     ENUMPROPFLAGS);
  constructor.setProperty("Clipboard",                        QScriptValue(engine, QEvent::Clipboard),                        ENUMPROPFLAGS);
  constructor.setProperty("Close",                            QScriptValue(engine, QEvent::Close),                            ENUMPROPFLAGS);
  constructor.setProperty("CloseSoftwareInputPanel",          QScriptValue(engine, QEvent::CloseSoftwareInputPanel),          ENUMPROPFLAGS);
  constructor.setProperty("ContentsRectChange",               QScriptValue(engine, QEvent::ContentsRectChange),               ENUMPROPFLAGS);
  constructor.setProperty("ContextMenu",                      QScriptValue(engine, QEvent::ContextMenu),                      ENUMPROPFLAGS);
  constructor.setProperty("CursorChange",                     QScriptValue(engine, QEvent::CursorChange),                     ENUMPROPFLAGS);
  constructor.setProperty("DeferredDelete",                   QScriptValue(engine, QEvent::DeferredDelete),                   ENUMPROPFLAGS);
  constructor.setProperty("DragEnter",                        QScriptValue(engine, QEvent::DragEnter),                        ENUMPROPFLAGS);
  constructor.setProperty("DragLeave",                        QScriptValue(engine, QEvent::DragLeave),                        ENUMPROPFLAGS);
  constructor.setProperty("DragMove",                         QScriptValue(engine, QEvent::DragMove),                         ENUMPROPFLAGS);
  constructor.setProperty("Drop",                             QScriptValue(engine, QEvent::Drop),                             ENUMPROPFLAGS);
  constructor.setProperty("DynamicPropertyChange",            QScriptValue(engine, QEvent::DynamicPropertyChange),            ENUMPROPFLAGS);
  constructor.setProperty("EnabledChange",                    QScriptValue(engine, QEvent::EnabledChange),                    ENUMPROPFLAGS);
  constructor.setProperty("Enter",                            QScriptValue(engine, QEvent::Enter),                            ENUMPROPFLAGS);
#ifdef QT_KEYPAD_NAVIGATION
  constructor.setProperty("EnterEditFocus",                   QScriptValue(engine, QEvent::EnterEditFocus),                   ENUMPROPFLAGS);
#endif
  constructor.setProperty("EnterWhatsThisMode",               QScriptValue(engine, QEvent::EnterWhatsThisMode),               ENUMPROPFLAGS);
  constructor.setProperty("Expose",                           QScriptValue(engine, QEvent::Expose),                           ENUMPROPFLAGS);
  constructor.setProperty("FileOpen",                         QScriptValue(engine, QEvent::FileOpen),                         ENUMPROPFLAGS);
  constructor.setProperty("FocusIn",                          QScriptValue(engine, QEvent::FocusIn),                          ENUMPROPFLAGS);
  constructor.setProperty("FocusOut",                         QScriptValue(engine, QEvent::FocusOut),                         ENUMPROPFLAGS);
  constructor.setProperty("FocusAboutToChange",               QScriptValue(engine, QEvent::FocusAboutToChange),               ENUMPROPFLAGS);
  constructor.setProperty("FontChange",                       QScriptValue(engine, QEvent::FontChange),                       ENUMPROPFLAGS);
  constructor.setProperty("Gesture",                          QScriptValue(engine, QEvent::Gesture),                          ENUMPROPFLAGS);
  constructor.setProperty("GestureOverride",                  QScriptValue(engine, QEvent::GestureOverride),                  ENUMPROPFLAGS);
  constructor.setProperty("GrabKeyboard",                     QScriptValue(engine, QEvent::GrabKeyboard),                     ENUMPROPFLAGS);
  constructor.setProperty("GrabMouse",                        QScriptValue(engine, QEvent::GrabMouse),                        ENUMPROPFLAGS);
  constructor.setProperty("GraphicsSceneContextMenu",         QScriptValue(engine, QEvent::GraphicsSceneContextMenu),         ENUMPROPFLAGS);
  constructor.setProperty("GraphicsSceneDragEnter",           QScriptValue(engine, QEvent::GraphicsSceneDragEnter),           ENUMPROPFLAGS);
  constructor.setProperty("GraphicsSceneDragLeave",           QScriptValue(engine, QEvent::GraphicsSceneDragLeave),           ENUMPROPFLAGS);
  constructor.setProperty("GraphicsSceneDragMove",            QScriptValue(engine, QEvent::GraphicsSceneDragMove),            ENUMPROPFLAGS);
  constructor.setProperty("GraphicsSceneDrop",                QScriptValue(engine, QEvent::GraphicsSceneDrop),                ENUMPROPFLAGS);
  constructor.setProperty("GraphicsSceneHelp",                QScriptValue(engine, QEvent::GraphicsSceneHelp),                ENUMPROPFLAGS);
  constructor.setProperty("GraphicsSceneHoverEnter",          QScriptValue(engine, QEvent::GraphicsSceneHoverEnter),          ENUMPROPFLAGS);
  constructor.setProperty("GraphicsSceneHoverLeave",          QScriptValue(engine, QEvent::GraphicsSceneHoverLeave),          ENUMPROPFLAGS);
  constructor.setProperty("GraphicsSceneHoverMove",           QScriptValue(engine, QEvent::GraphicsSceneHoverMove),           ENUMPROPFLAGS);
  constructor.setProperty("GraphicsSceneMouseDoubleClick",    QScriptValue(engine, QEvent::GraphicsSceneMouseDoubleClick),    ENUMPROPFLAGS);
  constructor.setProperty("GraphicsSceneMouseMove",           QScriptValue(engine, QEvent::GraphicsSceneMouseMove),           ENUMPROPFLAGS);
  constructor.setProperty("GraphicsSceneMousePress",          QScriptValue(engine, QEvent::GraphicsSceneMousePress),          ENUMPROPFLAGS);
  constructor.setProperty("GraphicsSceneMouseRelease",        QScriptValue(engine, QEvent::GraphicsSceneMouseRelease),        ENUMPROPFLAGS);
  constructor.setProperty("GraphicsSceneMove",                QScriptValue(engine, QEvent::GraphicsSceneMove),                ENUMPROPFLAGS);
  constructor.setProperty("GraphicsSceneResize",              QScriptValue(engine, QEvent::GraphicsSceneResize),              ENUMPROPFLAGS);
  constructor.setProperty("GraphicsSceneWheel",               QScriptValue(engine, QEvent::GraphicsSceneWheel),               ENUMPROPFLAGS);
  constructor.setProperty("Hide",                             QScriptValue(engine, QEvent::Hide),                             ENUMPROPFLAGS);
  constructor.setProperty("HideToParent",                     QScriptValue(engine, QEvent::HideToParent),                     ENUMPROPFLAGS);
  constructor.setProperty("HoverEnter",                       QScriptValue(engine, QEvent::HoverEnter),                       ENUMPROPFLAGS);
  constructor.setProperty("HoverLeave",                       QScriptValue(engine, QEvent::HoverLeave),                       ENUMPROPFLAGS);
  constructor.setProperty("HoverMove",                        QScriptValue(engine, QEvent::HoverMove),                        ENUMPROPFLAGS);
  constructor.setProperty("IconDrag",                         QScriptValue(engine, QEvent::IconDrag),                         ENUMPROPFLAGS);
  constructor.setProperty("IconTextChange",                   QScriptValue(engine, QEvent::IconTextChange),                   ENUMPROPFLAGS);
  constructor.setProperty("InputMethod",                      QScriptValue(engine, QEvent::InputMethod),                      ENUMPROPFLAGS);
  constructor.setProperty("InputMethodQuery",                 QScriptValue(engine, QEvent::InputMethodQuery),                 ENUMPROPFLAGS);
  constructor.setProperty("KeyboardLayoutChange",             QScriptValue(engine, QEvent::KeyboardLayoutChange),             ENUMPROPFLAGS);
  constructor.setProperty("KeyPress",                         QScriptValue(engine, QEvent::KeyPress),                         ENUMPROPFLAGS);
  constructor.setProperty("KeyRelease",                       QScriptValue(engine, QEvent::KeyRelease),                       ENUMPROPFLAGS);
  constructor.setProperty("LanguageChange",                   QScriptValue(engine, QEvent::LanguageChange),                   ENUMPROPFLAGS);
  constructor.setProperty("LayoutDirectionChange",            QScriptValue(engine, QEvent::LayoutDirectionChange),            ENUMPROPFLAGS);
  constructor.setProperty("LayoutRequest",                    QScriptValue(engine, QEvent::LayoutRequest),                    ENUMPROPFLAGS);
  constructor.setProperty("Leave",                            QScriptValue(engine, QEvent::Leave),                            ENUMPROPFLAGS);
#ifdef QT_KEYPAD_NAVIGATION
  constructor.setProperty("LeaveEditFocus",                   QScriptValue(engine, QEvent::LeaveEditFocus),                   ENUMPROPFLAGS);
#endif
  constructor.setProperty("LeaveWhatsThisMode",               QScriptValue(engine, QEvent::LeaveWhatsThisMode),               ENUMPROPFLAGS);
  constructor.setProperty("LocaleChange",                     QScriptValue(engine, QEvent::LocaleChange),                     ENUMPROPFLAGS);
  constructor.setProperty("NonClientAreaMouseButtonDblClick", QScriptValue(engine, QEvent::NonClientAreaMouseButtonDblClick), ENUMPROPFLAGS);
  constructor.setProperty("NonClientAreaMouseButtonPress",    QScriptValue(engine, QEvent::NonClientAreaMouseButtonPress),    ENUMPROPFLAGS);
  constructor.setProperty("NonClientAreaMouseButtonRelease",  QScriptValue(engine, QEvent::NonClientAreaMouseButtonRelease),  ENUMPROPFLAGS);
  constructor.setProperty("NonClientAreaMouseMove",           QScriptValue(engine, QEvent::NonClientAreaMouseMove),           ENUMPROPFLAGS);
  constructor.setProperty("MacSizeChange",                    QScriptValue(engine, QEvent::MacSizeChange),                    ENUMPROPFLAGS);
  constructor.setProperty("MetaCall",                         QScriptValue(engine, QEvent::MetaCall),                         ENUMPROPFLAGS);
  constructor.setProperty("ModifiedChange",                   QScriptValue(engine, QEvent::ModifiedChange),                   ENUMPROPFLAGS);
  constructor.setProperty("MouseButtonDblClick",              QScriptValue(engine, QEvent::MouseButtonDblClick),              ENUMPROPFLAGS);
  constructor.setProperty("MouseButtonPress",                 QScriptValue(engine, QEvent::MouseButtonPress),                 ENUMPROPFLAGS);
  constructor.setProperty("MouseButtonRelease",               QScriptValue(engine, QEvent::MouseButtonRelease),               ENUMPROPFLAGS);
  constructor.setProperty("MouseMove",                        QScriptValue(engine, QEvent::MouseMove),                        ENUMPROPFLAGS);
  constructor.setProperty("MouseTrackingChange",              QScriptValue(engine, QEvent::MouseTrackingChange),              ENUMPROPFLAGS);
  constructor.setProperty("Move",                             QScriptValue(engine, QEvent::Move),                             ENUMPROPFLAGS);
  constructor.setProperty("NativeGesture",                    QScriptValue(engine, QEvent::NativeGesture),                    ENUMPROPFLAGS);
  constructor.setProperty("OrientationChange",                QScriptValue(engine, QEvent::OrientationChange),                ENUMPROPFLAGS);
  constructor.setProperty("Paint",                            QScriptValue(engine, QEvent::Paint),                            ENUMPROPFLAGS);
  constructor.setProperty("PaletteChange",                    QScriptValue(engine, QEvent::PaletteChange),                    ENUMPROPFLAGS);
  constructor.setProperty("ParentAboutToChange",              QScriptValue(engine, QEvent::ParentAboutToChange),              ENUMPROPFLAGS);
  constructor.setProperty("ParentChange",                     QScriptValue(engine, QEvent::ParentChange),                     ENUMPROPFLAGS);
  constructor.setProperty("PlatformPanel",                    QScriptValue(engine, QEvent::PlatformPanel),                    ENUMPROPFLAGS);
  constructor.setProperty("PlatformSurface",                  QScriptValue(engine, QEvent::PlatformSurface),                  ENUMPROPFLAGS);
  constructor.setProperty("Polish",                           QScriptValue(engine, QEvent::Polish),                           ENUMPROPFLAGS);
  constructor.setProperty("PolishRequest",                    QScriptValue(engine, QEvent::PolishRequest),                    ENUMPROPFLAGS);
  constructor.setProperty("QueryWhatsThis",                   QScriptValue(engine, QEvent::QueryWhatsThis),                   ENUMPROPFLAGS);
  constructor.setProperty("ReadOnlyChange",                   QScriptValue(engine, QEvent::ReadOnlyChange),                   ENUMPROPFLAGS);
  constructor.setProperty("RequestSoftwareInputPanel",        QScriptValue(engine, QEvent::RequestSoftwareInputPanel),        ENUMPROPFLAGS);
  constructor.setProperty("Resize",                           QScriptValue(engine, QEvent::Resize),                           ENUMPROPFLAGS);
  constructor.setProperty("ScrollPrepare",                    QScriptValue(engine, QEvent::ScrollPrepare),                    ENUMPROPFLAGS);
  constructor.setProperty("Scroll",                           QScriptValue(engine, QEvent::Scroll),                           ENUMPROPFLAGS);
  constructor.setProperty("Shortcut",                         QScriptValue(engine, QEvent::Shortcut),                         ENUMPROPFLAGS);
  constructor.setProperty("ShortcutOverride",                 QScriptValue(engine, QEvent::ShortcutOverride),                 ENUMPROPFLAGS);
  constructor.setProperty("Show",                             QScriptValue(engine, QEvent::Show),                             ENUMPROPFLAGS);
  constructor.setProperty("ShowToParent",                     QScriptValue(engine, QEvent::ShowToParent),                     ENUMPROPFLAGS);
  constructor.setProperty("SockAct",                          QScriptValue(engine, QEvent::SockAct),                          ENUMPROPFLAGS);
  constructor.setProperty("StateMachineSignal",               QScriptValue(engine, QEvent::StateMachineSignal),               ENUMPROPFLAGS);
  constructor.setProperty("StateMachineWrapped",              QScriptValue(engine, QEvent::StateMachineWrapped),              ENUMPROPFLAGS);
  constructor.setProperty("StatusTip",                        QScriptValue(engine, QEvent::StatusTip),                        ENUMPROPFLAGS);
  constructor.setProperty("StyleChange",                      QScriptValue(engine, QEvent::StyleChange),                      ENUMPROPFLAGS);
  constructor.setProperty("TabletMove",                       QScriptValue(engine, QEvent::TabletMove),                       ENUMPROPFLAGS);
  constructor.setProperty("TabletPress",                      QScriptValue(engine, QEvent::TabletPress),                      ENUMPROPFLAGS);
  constructor.setProperty("TabletRelease",                    QScriptValue(engine, QEvent::TabletRelease),                    ENUMPROPFLAGS);
  constructor.setProperty("TabletEnterProximity",             QScriptValue(engine, QEvent::TabletEnterProximity),             ENUMPROPFLAGS);
  constructor.setProperty("TabletLeaveProximity",             QScriptValue(engine, QEvent::TabletLeaveProximity),             ENUMPROPFLAGS);
#if QT_VERSION >= 0x050900
  constructor.setProperty("TabletTrackingChange",             QScriptValue(engine, QEvent::TabletTrackingChange),             ENUMPROPFLAGS);
#endif
  constructor.setProperty("ThreadChange",                     QScriptValue(engine, QEvent::ThreadChange),                     ENUMPROPFLAGS);
  constructor.setProperty("Timer",                            QScriptValue(engine, QEvent::Timer),                            ENUMPROPFLAGS);
  constructor.setProperty("ToolBarChange",                    QScriptValue(engine, QEvent::ToolBarChange),                    ENUMPROPFLAGS);
  constructor.setProperty("ToolTip",                          QScriptValue(engine, QEvent::ToolTip),                          ENUMPROPFLAGS);
  constructor.setProperty("ToolTipChange",                    QScriptValue(engine, QEvent::ToolTipChange),                    ENUMPROPFLAGS);
  constructor.setProperty("TouchBegin",                       QScriptValue(engine, QEvent::TouchBegin),                       ENUMPROPFLAGS);
  constructor.setProperty("TouchCancel",                      QScriptValue(engine, QEvent::TouchCancel),                      ENUMPROPFLAGS);
  constructor.setProperty("TouchEnd",                         QScriptValue(engine, QEvent::TouchEnd),                         ENUMPROPFLAGS);
  constructor.setProperty("TouchUpdate",                      QScriptValue(engine, QEvent::TouchUpdate),                      ENUMPROPFLAGS);
  constructor.setProperty("UngrabKeyboard",                   QScriptValue(engine, QEvent::UngrabKeyboard),                   ENUMPROPFLAGS);
  constructor.setProperty("UngrabMouse",                      QScriptValue(engine, QEvent::UngrabMouse),                      ENUMPROPFLAGS);
  constructor.setProperty("UpdateLater",                      QScriptValue(engine, QEvent::UpdateLater),                      ENUMPROPFLAGS);
  constructor.setProperty("UpdateRequest",                    QScriptValue(engine, QEvent::UpdateRequest),                    ENUMPROPFLAGS);
  constructor.setProperty("WhatsThis",                        QScriptValue(engine, QEvent::WhatsThis),                        ENUMPROPFLAGS);
  constructor.setProperty("WhatsThisClicked",                 QScriptValue(engine, QEvent::WhatsThisClicked),                 ENUMPROPFLAGS);
  constructor.setProperty("Wheel",                            QScriptValue(engine, QEvent::Wheel),                            ENUMPROPFLAGS);
  constructor.setProperty("WinEventAct",                      QScriptValue(engine, QEvent::WinEventAct),                      ENUMPROPFLAGS);
  constructor.setProperty("WindowActivate",                   QScriptValue(engine, QEvent::WindowActivate),                   ENUMPROPFLAGS);
  constructor.setProperty("WindowBlocked",                    QScriptValue(engine, QEvent::WindowBlocked),                    ENUMPROPFLAGS);
  constructor.setProperty("WindowDeactivate",                 QScriptValue(engine, QEvent::WindowDeactivate),                 ENUMPROPFLAGS);
  constructor.setProperty("WindowIconChange",                 QScriptValue(engine, QEvent::WindowIconChange),                 ENUMPROPFLAGS);
  constructor.setProperty("WindowStateChange",                QScriptValue(engine, QEvent::WindowStateChange),                ENUMPROPFLAGS);
  constructor.setProperty("WindowTitleChange",                QScriptValue(engine, QEvent::WindowTitleChange),                ENUMPROPFLAGS);
  constructor.setProperty("WindowUnblocked",                  QScriptValue(engine, QEvent::WindowUnblocked),                  ENUMPROPFLAGS);
  constructor.setProperty("WinIdChange",                      QScriptValue(engine, QEvent::WinIdChange),                      ENUMPROPFLAGS);
  constructor.setProperty("ZOrderChange",                     QScriptValue(engine, QEvent::ZOrderChange),                     ENUMPROPFLAGS);

  constructor.setProperty("User", QScriptValue(engine, QEvent::User), ENUMPROPFLAGS);
  constructor.setProperty("MaxUser", QScriptValue(engine, QEvent::MaxUser), ENUMPROPFLAGS);

}

QEventProto::QEventProto(QObject *parent)
    : QObject(parent)
{
}

void QEventProto::accept()
{
  QEvent *item = qscriptvalue_cast<QEvent*>(thisObject());
  if (item)
    item->accept();
}

void QEventProto::ignore()
{
  QEvent *item = qscriptvalue_cast<QEvent*>(thisObject());
  if (item)
    item->ignore();
}

bool QEventProto::isAccepted() const
{
  QEvent *item = qscriptvalue_cast<QEvent*>(thisObject());
  if (item)
    return item->isAccepted();
  return false;
}

void QEventProto::setAccepted(bool accepted)
{
  QEvent *item = qscriptvalue_cast<QEvent*>(thisObject());
  if (item)
    item->setAccepted(accepted);
}

bool QEventProto::spontaneous() const
{
  QEvent *item = qscriptvalue_cast<QEvent*>(thisObject());
  if (item)
    return item->spontaneous();
  return false;
}

