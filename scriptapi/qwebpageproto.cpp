/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qwebpageproto.h"

#include <QAction>
#include <QMenu>
#include <QNetworkAccessManager>
#include <QPalette>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QUndoStack>
#include <QVariant>
#include <QWebFrame>
#include <QWebHistory>
#include <QWebPage>
#include <QWebPluginFactory>
#include <QWebSettings>
#include <QWidget>

QScriptValue QWebPagetoScriptValue(QScriptEngine *engine, QWebPage* const &item)
{
  return engine->newQObject(item);
}

void QWebPagefromScriptValue(const QScriptValue &obj, QWebPage* &item)
{
  item = qobject_cast<QWebPage*>(obj.toQObject());
}

void setupQWebPageProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QWebPagetoScriptValue, QWebPagefromScriptValue);
  QScriptValue::PropertyFlags permanent = QScriptValue::ReadOnly | QScriptValue::Undeletable;

  QScriptValue proto = engine->newQObject(new QWebPageProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QWebPage*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQWebPage,
                                                 proto);
  engine->globalObject().setProperty("QWebPage",  constructor);

  // enum QWebPage::LinkDelegationPolicy
  constructor.setProperty("DontDelegateLinks",      QScriptValue(engine, QWebPage::DontDelegateLinks),      permanent);
  constructor.setProperty("DelegateExternalLinks",  QScriptValue(engine, QWebPage::DelegateExternalLinks),  permanent);
  constructor.setProperty("DelegateAllLinks",       QScriptValue(engine, QWebPage::DelegateAllLinks),       permanent);

  // enum QWebPage::VisibilityState
  constructor.setProperty("VisibilityStateVisible",   QScriptValue(engine, QWebPage::DelegateAllLinks), permanent);
  constructor.setProperty("VisibilityStateHidden",    QScriptValue(engine, QWebPage::DelegateAllLinks), permanent);
  constructor.setProperty("VisibilityStatePrerender", QScriptValue(engine, QWebPage::DelegateAllLinks), permanent);
  constructor.setProperty("VisibilityStateUnloaded",  QScriptValue(engine, QWebPage::DelegateAllLinks), permanent);
}

QScriptValue constructQWebPage(QScriptContext * context,
                                    QScriptEngine  *engine)
{
  QWebPage *obj = 0;
  if (context->argumentCount() == 1)
    obj = new QWebPage(qobject_cast<QWidget*>(context->argument(0).toQObject()));
  else
    obj = new QWebPage();
  return engine->toScriptValue(obj);
}

QWebPageProto::QWebPageProto(QObject *parent)
    : QObject(parent)
{
}
QWebPageProto::~QWebPageProto()
{
}

QAction* QWebPageProto::action(QWebPage::WebAction action) const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->action(action);
  return 0;
}

quint64 QWebPageProto::bytesReceived() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->bytesReceived();
  return 0;
}

QMenu* QWebPageProto::createStandardContextMenu()
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->createStandardContextMenu();
  return 0;
}

QWebFrame* QWebPageProto::currentFrame() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->currentFrame();
  return 0;
}

bool QWebPageProto::extension(QWebPage::Extension extension, const QWebPage::ExtensionOption * option, QWebPage::ExtensionReturn * output)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->extension(extension, option, output);
  return false;
}

bool QWebPageProto::findText(const QString & subString, QWebPage::FindFlags options)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->findText(subString, options);
  return false;
}

bool QWebPageProto::focusNextPrevChild(bool next)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->focusNextPrevChild(next);
  return false;
}

bool QWebPageProto::forwardUnsupportedContent() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->forwardUnsupportedContent();
  return false;
}

QWebFrame* QWebPageProto::frameAt(const QPoint & pos) const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->frameAt(pos);
  return 0;
}

bool QWebPageProto::hasSelection() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->hasSelection();
  return false;
}

QWebHistory* QWebPageProto::history() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->history();
  return 0;
}

QVariant QWebPageProto::inputMethodQuery(Qt::InputMethodQuery property) const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->inputMethodQuery(property);
  return QVariant();
}

bool QWebPageProto::isContentEditable() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->isContentEditable();
  return false;
}

bool QWebPageProto::isModified() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->isModified();
  return false;
}

QWebPage::LinkDelegationPolicy QWebPageProto::linkDelegationPolicy() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->linkDelegationPolicy();
  return QWebPage::LinkDelegationPolicy();
}

QWebFrame* QWebPageProto::mainFrame() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->mainFrame();
  return 0;
}

QNetworkAccessManager* QWebPageProto::networkAccessManager() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->networkAccessManager();
  return 0;
}

QPalette QWebPageProto::palette() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->palette();
  return QPalette();
}

QWebPluginFactory* QWebPageProto::pluginFactory() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->pluginFactory();
  return 0;
}

QSize QWebPageProto::preferredContentsSize() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->preferredContentsSize();
  return QSize();
}

QString QWebPageProto::selectedHtml() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->selectedHtml();
  return QString();
}

QString QWebPageProto::selectedText() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->selectedText();
  return QString();
}

void QWebPageProto::setActualVisibleContentRect(const QRect & rect) const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    item->setActualVisibleContentRect(rect);
}

void QWebPageProto::setContentEditable(bool editable)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    item->setContentEditable(editable);
}

void QWebPageProto::setFeaturePermission(QWebFrame * frame, QWebPage::Feature feature, QWebPage::PermissionPolicy policy)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    item->setFeaturePermission(frame, feature, policy);
}

void QWebPageProto::setForwardUnsupportedContent(bool forward)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    item->setForwardUnsupportedContent(forward);
}

void QWebPageProto::setLinkDelegationPolicy(QWebPage::LinkDelegationPolicy policy)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    item->setLinkDelegationPolicy(policy);
}

void QWebPageProto::setNetworkAccessManager(QNetworkAccessManager * manager)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    item->setNetworkAccessManager(manager);
}

void QWebPageProto::setPalette(const QPalette & palette)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    item->setPalette(palette);
}

void QWebPageProto::setPluginFactory(QWebPluginFactory * factory)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    item->setPluginFactory(factory);
}

void QWebPageProto::setPreferredContentsSize(const QSize & size) const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    item->setPreferredContentsSize(size);
}

void QWebPageProto::setView(QWidget * view)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    item->setView(view);
}

void QWebPageProto::setViewportSize(const QSize & size) const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    item->setViewportSize(size);
}

#if QT_VERSION >= 0x050000
void QWebPageProto::setVisibilityState(QWebPage::VisibilityState state)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    item->setVisibilityState(state);
}
#endif

QWebSettings* QWebPageProto::settings() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->settings();
  return 0;
}

bool QWebPageProto::shouldInterruptJavaScript()
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->shouldInterruptJavaScript();
  return false;
}

QStringList QWebPageProto::supportedContentTypes() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->supportedContentTypes();
  return QStringList();
}

bool QWebPageProto::supportsContentType(const QString & mimeType) const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->supportsContentType(mimeType);
  return false;
}

bool QWebPageProto::supportsExtension(QWebPage::Extension extension) const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->supportsExtension(extension);
  return false;
}

bool QWebPageProto::swallowContextMenuEvent(QContextMenuEvent * event)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->swallowContextMenuEvent(event);
  return false;
}

quint64 QWebPageProto::totalBytes() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->totalBytes();
  return 0;
}

void QWebPageProto::triggerAction(QWebPage::WebAction action, bool checked)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    item->triggerAction(action, checked);
}

QUndoStack* QWebPageProto::undoStack() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->undoStack();
  return 0;
}

void QWebPageProto::updatePositionDependentActions(const QPoint & pos)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    item->updatePositionDependentActions(pos);
}

QWidget* QWebPageProto::view() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->view();
  return 0;
}

QWebPage::ViewportAttributes QWebPageProto::viewportAttributesForSize(const QSize & availableSize) const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->viewportAttributesForSize(availableSize);
  return QWebPage::ViewportAttributes();
}

QSize QWebPageProto::viewportSize() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->viewportSize();
  return QSize();
}

#if QT_VERSION >= 0x050000
QWebPage::VisibilityState QWebPageProto::visibilityState() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->visibilityState();
  return QWebPage::VisibilityStateVisible; // don't know the best default
}
#endif
