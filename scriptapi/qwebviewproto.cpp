/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2018 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "scriptapi_internal.h"
#include "qwebviewproto.h"

#include <QAction>
#include <QIcon>
#include <QPrinter>
#include <QUrl>
#include <QWebHistory>
#include <QWebSettings>
#include <QWebView>

QScriptValue QWebViewtoScriptValue(QScriptEngine *engine, QWebView* const &item)
{
  return engine->newQObject(item);
}

void QWebViewfromScriptValue(const QScriptValue &obj, QWebView* &item)
{
  item = qobject_cast<QWebView*>(obj.toQObject());
}

void setupQWebViewProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QWebViewtoScriptValue, QWebViewfromScriptValue);

  QScriptValue proto = engine->newQObject(new QWebViewProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QWebView*>(), proto);
  // Not allowed. Is private in qwebview.h
  //engine->setDefaultPrototype(qMetaTypeId<QWebView>(), proto);

  QScriptValue constructor = engine->newFunction(constructQWebView,
                                                 proto);
  engine->globalObject().setProperty("QWebView",  constructor);
}

QScriptValue constructQWebView(QScriptContext * context,
                                    QScriptEngine  *engine)
{
  QWebView *obj = 0;
  if (context->argumentCount() == 1)
    obj = new QWebView(qobject_cast<QWidget*>(context->argument(0).toQObject()));
  else
    obj = new QWebView();
  return engine->toScriptValue(obj);
}

QWebViewProto::QWebViewProto(QObject *parent)
    : QObject(parent)
{
}
QWebViewProto::~QWebViewProto()
{
}

bool QWebViewProto::hasSelection() const
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    return item->hasSelection();
  return false;
}

bool QWebViewProto::findText(const QString &subString, QWebPage::FindFlags options)
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    return item->findText(subString, options);
  return false;
}

QWebHistory* QWebViewProto::history() const
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    return item->history();
  return 0;
}

QIcon QWebViewProto::icon() const
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    return item->icon();
  return QIcon();
}

bool QWebViewProto::isModified() const
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    return item->isModified();
  return false;
}

void QWebViewProto::load(const QUrl &url)
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    item->load(url);
}

void QWebViewProto::load(const QNetworkRequest &request, QNetworkAccessManager::Operation operation, const QByteArray &body)
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    item->load(request, operation, body);
}

QWebPage* QWebViewProto::page() const
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    return item->page();
  return 0;
}

QAction* QWebViewProto::pageAction(QWebPage::WebAction action)  const
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    return item->pageAction(action);
  return 0;
}

QPainter::RenderHints QWebViewProto::renderHints() const
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    return item->renderHints();
  return QPainter::RenderHints();
}

QString QWebViewProto::selectedHtml() const
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    return item->selectedHtml();
  return QString();
}

QString QWebViewProto::selectedText() const
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    return item->selectedText();
  return QString();
}

void QWebViewProto::setContent(const QByteArray &data, const QString &mimeType, const QUrl &baseUrl)
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    item->setContent(data, mimeType, baseUrl);
}

void QWebViewProto::setHtml(const QString &html, const QUrl &baseUrl)
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    item->setHtml(html, baseUrl);
}

void QWebViewProto::setPage(QWebPage *page)
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    item->setPage(page);
}

void QWebViewProto::setRenderHint(QPainter::RenderHint hint, bool enabled)
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    item->setRenderHint(hint, enabled);
}

void QWebViewProto::setRenderHints(QPainter::RenderHints hints)
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    item->setRenderHints(hints);
}

void QWebViewProto::setTextSizeMultiplier(qreal factor)
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    item->setTextSizeMultiplier(factor);
}

QWebSettings* QWebViewProto::settings() const
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    return item->settings();
  return 0;
}

void QWebViewProto::setUrl(const QUrl & url)
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    item->setUrl(url);
}

void QWebViewProto::setZoomFactor(qreal factor)
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    item->setZoomFactor(factor);
}

qreal QWebViewProto::textSizeMultiplier() const
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    return item->textSizeMultiplier();
  return qreal();
}

QString QWebViewProto::title() const
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    return item->title();
  return QString();
}

void QWebViewProto::triggerPageAction(QWebPage::WebAction action, bool checked)
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    item->triggerPageAction(action, checked);
}

QUrl QWebViewProto::url() const
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    return item->url();
  return QUrl();
}

qreal QWebViewProto::zoomFactor() const
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    return item->zoomFactor();
  return qreal();
}

// Reimplemented Public Functions.
bool QWebViewProto::event(QEvent * e)
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    return item->event(e);
  return false;
}

QVariant QWebViewProto::inputMethodQuery(Qt::InputMethodQuery property) const
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    return item->inputMethodQuery(property);
  return QVariant();
}

QSize QWebViewProto::sizeHint() const
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    return item->sizeHint();
  return QSize();
}

// Public Slots.
void QWebViewProto::back()
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    return item->back();
}

void QWebViewProto::forward()
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    return item->forward();
}

void QWebViewProto::print(QPrinter * printer) const
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    return item->print(printer);
}

void QWebViewProto::reload()
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    return item->reload();
}

void QWebViewProto::stop()
{
  scriptDeprecated("QWebView will not be available in a future version");
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    return item->stop();
}
