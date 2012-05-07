/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PAYMENTECGPROCESSOR_H
#define PATNEBTECHPROCESSOR_H

#include <QObject>
#include <QString>

#include "creditcardprocessor.h"

class PaymentechProcessor : public CreditCardProcessor
{
  Q_OBJECT

  public:
    PaymentechProcessor();

  protected:
    virtual int  buildCommon(QString &pordernum, const int pccardid, const QString &pcvv, const double pamount, const int pcurrid, QString &prequest, QString pordertype, const QString &pAuthcode = QString::null, const QString &pRespdate = QString::null);
    virtual int  doTestConfiguration();
    virtual int  doAuthorize(const int, const QString &pcvv, double &, const double, const bool, const double, const double, const int, QString&, QString&, int&, ParameterList &);
    virtual int  doCharge(const int pccardid, const QString &pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams);
    virtual int  doChargePreauthorized(const int pccardid, const QString &pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams);
    virtual int  doCredit(const int pccardid, const QString &pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams);
    virtual int  doVoidPrevious(const int pccardid, const QString &pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, QString &papproval, int &pccpayid, ParameterList &pparams);
    virtual bool handlesCreditCards();
    virtual int  handleResponse(const QString&, const int, const QString&, const double, const int, QString&, QString&, int&, ParameterList&);
};

#endif // PAYMENTECHPROCESSOR_H
