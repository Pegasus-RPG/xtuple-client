/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef YOURPAYPROCESSOR_H
#define YOURPAYPROCESSOR_H

#include <QDomDocument>
#include <QObject>
#include <QString>

#include "creditcardprocessor.h"

class YourPayProcessor : public CreditCardProcessor
{
  Q_OBJECT

  public:
    YourPayProcessor();

  protected:
    virtual int  buildCommon(const int pccardid, const QString &pcvv, const double pamount, QDomDocument &prequest, QString pordertype);
    virtual int  doTestConfiguration();
    virtual int  doAuthorize(const int pccardid, const QString &pcvv, double &pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pParams);
    virtual int  doCharge(const int pccardid, const QString &pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString& pneworder, QString& preforder, int &pccpayid, ParameterList &pparams);
    virtual int  doChargePreauthorized(const int pccardid, const QString &pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams);
    virtual int  doCredit(const int pccardid, const QString &pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams);
    virtual int  doVoidPrevious(const int pccardid, const QString &pcvv, double pamount, const int pcurrid, QString &pneworder, QString &preforder, QString &papproval, int &pccpayid, ParameterList &pparams);
    virtual int  fraudChecks();
    virtual int  handleResponse(const QString&, const int, const QString&, const double, const int, QString&, QString&, int&, ParameterList&);
    virtual bool handlesCreditCards();
    virtual void reset();

  private:
    bool	_passedLinkShield;
    int		_ypcurrid;
};

#endif // YOURPAYPROCESSOR_H
