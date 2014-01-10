/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef AUTHORIZEDOTNETPROCESSOR_H
#define AUTHORIZEDOTNETPROCESSOR_H

#include <QObject>
#include <QString>

#include "creditcardprocessor.h"

class AuthorizeDotNetProcessor : public CreditCardProcessor
{
  Q_OBJECT

  public:
    AuthorizeDotNetProcessor();

  protected:
    virtual int  buildCommon(const int pccardid, const QString &pcvv, const double pamount, const int pcurrid, QString &prequest, QString pordertype);
    virtual int  doTestConfiguration();
    virtual int  doAuthorize(const int, const QString &pcvv, double&, const double, const bool, const double, const double, const int, QString&, QString&, int&, ParameterList &);
    virtual int  doCharge(const int, const QString &pcvv, const double, const double, const bool, const double, const double, const int, QString&, QString&, int&, ParameterList &);
    virtual int  doChargePreauthorized(const int, const QString &pcvv, const double, const int, QString&, QString&, int&, ParameterList &);
    virtual int  doCredit(const int, const QString &pcvv, const double, const double, const bool, const double, const double, const int, QString&, QString&, int&, ParameterList &);
    virtual int  doVoidPrevious(const int, const QString &pcvv, const double, const int, QString&, QString&, QString&, int&, ParameterList &);
    virtual bool handlesCreditCards();
    virtual int  handleResponse(const QString&, const int, const QString&, const double, const int, QString&, QString&, int&, ParameterList&);

  private:
    int fieldValue(const QStringList, const int, QString &);
};

#endif // AUTHORIZEDOTNETPROCESSOR_H
