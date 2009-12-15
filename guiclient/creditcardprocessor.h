/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CREDITCARDPROCESSOR_H
#define CREDITCARDPROCESSOR_H

#include <QHash>
#include <QObject>
#include <QString>
#include <QHttp>

#include <parameter.h>

class CreditCardProcessor : public QObject
{
  Q_OBJECT

  public:
    virtual ~CreditCardProcessor();

    // no public constructor for abstract class, just a factory
    Q_INVOKABLE static CreditCardProcessor *getProcessor(const QString = QString());

    // these are the primary transaction handlers and should not be overridden:
    virtual int authorize(const int pccardid, const int pcvv, const double pamount, double ptax, bool ptaxexempt, double pfreight, double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, QString preftype, int &prefid);
    virtual int charge(const int, const int, const double, const double, const bool, const double, const double, const int, QString&, QString&, int&, QString, int&);
    virtual int chargePreauthorized(const int, const double, const int, QString&, QString&, int&);
    virtual int credit(const int, const int, const double, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int, QString&, QString&, int&, QString, int&);
    virtual int voidPrevious(int &);
    
    // methods for script access
    Q_INVOKABLE static ParameterList authorize(const ParameterList &);
    Q_INVOKABLE static ParameterList charge(const ParameterList &);
    Q_INVOKABLE static ParameterList chargePreauthorized(const ParameterList &);
    Q_INVOKABLE static ParameterList credit(const ParameterList &);
    Q_INVOKABLE static ParameterList voidPrevious(const ParameterList &);

    // these are support methods that typically won't be overridden
    Q_INVOKABLE virtual int	testConfiguration();
    Q_INVOKABLE virtual int     defaultPort(bool = false);
    Q_INVOKABLE virtual QString defaultServer();
    Q_INVOKABLE virtual bool    handlesChecks();
    Q_INVOKABLE virtual bool    handlesCreditCards();
    Q_INVOKABLE virtual bool    isLive();
    Q_INVOKABLE virtual bool    isTest();
    Q_INVOKABLE virtual void    reset();

    Q_INVOKABLE static  QString errorMsg();		// most recent error
    Q_INVOKABLE static  QString errorMsg(const int);
    Q_INVOKABLE static  int     printReceipt(const int);

  protected:
    CreditCardProcessor();

    // do* handle the processor-specific processing and are expected to be overridden
    virtual int doAuthorize(const int, const int, const double, const double, const bool, const double, const double, const int, QString&, QString&, int&, ParameterList&);
    virtual int doCharge(const int, const int, const double, const double, const bool, const double, const double, const int, QString&, QString&, int&, ParameterList&);
    virtual int doChargePreauthorized(const int, const int, const double, const int, QString&, QString&, int&, ParameterList&);
    virtual int doCredit(const int, const int, const double, const double, const bool, const double, const double, const int, QString&, QString&, int&, ParameterList&);
    virtual int doVoidPrevious(const int, const int, const double, const int, QString&, QString&, QString&, int&, ParameterList&);
    virtual int doTestConfiguration();

    virtual QString buildURL(const QString, const QString, const bool);
    virtual int     checkCreditCard(const int, const int, QString&);
    virtual int     checkCreditCardProcessor()	{ return false; };
    static  double  currToCurr(const int, const int, const double, int * = 0);
    virtual int     fraudChecks();
    virtual int     sendViaHTTP(const QString&, QString&);
    virtual int     updateCCPay(int &, ParameterList &);

    QString		_defaultLiveServer;
    QString		_defaultTestServer;
    int			_defaultLivePort;
    int			_defaultTestPort;
    static QString	_errorMsg;
    static QHash<int, QString>	_msgHash;
    bool		_passedAvs;
    bool		_passedCvv;
    QString		_plogin;
    QString		_ppassword;
    QString		_pport;
    QString		_pserver;
    QHttp             * _http;

};

#endif // CREDITCARDPROCESSOR_H
