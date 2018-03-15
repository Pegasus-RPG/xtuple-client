/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2018 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "registrationKeyDialog.h"
#include "xtupleproductkey.h"

#include <QCloseEvent>
#include <QDesktopServices>
#include <QVariant>

registrationKeyDialog::registrationKeyDialog(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : QDialog(parent, modal ? (fl | Qt::Dialog) : fl)
{
  setObjectName(name);
  setupUi(this);

  connect(_key, SIGNAL(textChanged(const QString&)), this, SLOT(sCheckKey()));
  connect(_close,   SIGNAL(clicked()), this, SLOT(reject()));
  connect(_request, SIGNAL(clicked()), this, SLOT(sRequest()));
  connect(_select,  SIGNAL(clicked()), this, SLOT(sSelect()));
}

registrationKeyDialog::~registrationKeyDialog()
{
  // no need to delete child widgets, Qt does it all for us
}

void registrationKeyDialog::languageChange()
{
  retranslateUi(this);
}

enum SetResponse registrationKeyDialog::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("invalid", &valid);
  if (valid)
    setWindowTitle(tr("Invalid Registration Key"));

  param = pParams.value("message", &valid);
  if (valid)
  {
    _status->setHtml(param.toString());
  }

  _status->setHtml(_status->toHtml() + "<p>" + _metrics->value("NewRegistrationKeyRequested"));

  return NoError;
}

void registrationKeyDialog::sCheckKey()
{
  XTupleProductKey pkey(_key->text());
  if (pkey.valid() && pkey.expiration() > QDate::currentDate())
  {
    _select->setEnabled(true);
    _status->setHtml(tr("Valid key expiring %1").arg(pkey.expiration().toString(Qt::ISODate)));
  }
  else
  {
    _select->setEnabled(false);
    _status->setHtml(pkey.valid() ? tr("Expired key") : tr("Invalid key"));
  }
}

void registrationKeyDialog::sSelect()
{
  XTupleProductKey pk(_key->text().trimmed());
  if (pk.valid())
  {
    _metrics->set("RegistrationKey", _key->text().trimmed());
    _metrics->set("NewRegistrationKeyRequested", QString());
    accept();
  }
}

void registrationKeyDialog::sRequest()
{
  QString urlstr("https://xtuple.com/request-new-license-key");
  QUrl    url(urlstr);

  XTupleProductKey pk(_key->text().trimmed());
  if (! pk.customerId().isEmpty())
    url.setQuery(QString("customerId=%1").arg(pk.customerId()), QUrl::StrictMode);

  _status->setHtml(tr("Opening %1").arg(urlstr));
  QDesktopServices::openUrl(url);
  _metrics->set("NewRegistrationKeyRequested",
                tr("New key requested on %1").arg(QDate::currentDate().toString(Qt::ISODate)));
}

void registrationKeyDialog::closeEvent(QCloseEvent *pEvent)
{
  Q_UNUSED(pEvent);
  reject();
}
