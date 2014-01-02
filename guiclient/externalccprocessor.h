/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef EXTERNALCCPROCESSOR_H
#define EXTERNALCCPROCESSOR_H

#include <QObject>
#include <QString>

#include "creditcardprocessor.h"

class ExternalCCProcessor : public CreditCardProcessor
{
  Q_OBJECT

  public:
    ExternalCCProcessor();
    Q_INVOKABLE virtual int testConfiguration();

  protected:
    virtual int  doAuthorize(const int pccardid, const QString &pcvv, double &pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString &pnweorder, QString &preforder, int &pccpayid, ParameterList &pparams);
    virtual int  doCharge(const int pccardid, const QString &pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString &pnweorder, QString &preforder, int &pccpayid, ParameterList &pparams);
    virtual int  doChargePreauthorized(const int pccardid, const QString &pcvv, const double pamount, const int pcurrid, QString &pnweorder, QString &preforder, int &pccpayid, ParameterList &pparams);
    virtual int  doCredit(const int pccardid, const QString &pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString &pnweorder, QString &preforder, int &pccpayid, ParameterList &pparams);
    virtual int  doVoidPrevious(const int pccardid, const QString &pcvv, const double pamount, const int pcurrid, QString &pnweorder, QString &preforder, QString &papproval, int &pccpayid, ParameterList &pparams);
    virtual bool handlesCreditCards();
    virtual int  handleTrans(const int pccardid, const QString &ptype, const QString &pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams);

  private:
};

#endif // EXTERNALCCPROCESSOR_H
