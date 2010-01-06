/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QSqlError>

#include <currcluster.h>

#include "guiclient.h"
#include "paymentechprocessor.h"

#define DEBUG false

PaymentechProcessor::PaymentechProcessor() : CreditCardProcessor()
{
  _defaultLivePort   = 443;
  _defaultLiveServer = "https://netconnectka1.paymentech.net/NetConnect/controller";
  _defaultTestPort   = 443;
  _defaultTestServer = "https://netconnectkavar1.paymentech.net/NetConnect/controller";

  _msgHash.insert(-200, tr("A Login is required."));
  _msgHash.insert(-201, tr("A Password is required."));
  _msgHash.insert(-202, tr("A Division Number must be no more than 10 characters long."));
  _msgHash.insert(-205, tr("The response from the Gateway appears to be "
			   "incorrectly formatted (could not find field %1 "
			   "as there are only %2 fields present)."));
  _msgHash.insert(-206, tr("The response from the Gateway failed the MD5 "
			   "security check."));
  _msgHash.insert( 206, tr("The response from the Gateway has failed the MD5 "
			   "security check but will be processed anyway."));
  _msgHash.insert(-207, tr("The Gateway returned the following error: %1"));
  _msgHash.insert(-209, tr("The selected credit card is not a know type for Paymentech."));
}

int PaymentechProcessor::buildCommon(QString & pordernum, const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &prequest, QString pordertype)
{
  // TODO: if check and not credit card transaction do something else
  XSqlQuery anq;
  anq.prepare(
    "SELECT ccard_active,"
    "  formatbytea(decrypt(setbytea(ccard_number),   setbytea(:key),'bf')) AS ccard_number,"
    "  formatccnumber(decrypt(setbytea(ccard_number),setbytea(:key),'bf')) AS ccard_number_x,"
    "  formatbytea(decrypt(setbytea(ccard_name),     setbytea(:key),'bf')) AS ccard_name,"
    "  formatbytea(decrypt(setbytea(ccard_address1), setbytea(:key),'bf')) AS ccard_address1,"
    "  formatbytea(decrypt(setbytea(ccard_address2), setbytea(:key),'bf')) AS ccard_address2,"
    "  formatbytea(decrypt(setbytea(ccard_city),     setbytea(:key),'bf')) AS ccard_city,"
    "  formatbytea(decrypt(setbytea(ccard_state),    setbytea(:key),'bf')) AS ccard_state,"
    "  formatbytea(decrypt(setbytea(ccard_zip),      setbytea(:key),'bf')) AS ccard_zip,"
    "  formatbytea(decrypt(setbytea(ccard_country),  setbytea(:key),'bf')) AS ccard_country,"
    "  formatbytea(decrypt(setbytea(ccard_month_expired),setbytea(:key),'bf')) AS ccard_month_expired,"
    "  formatbytea(decrypt(setbytea(ccard_year_expired),setbytea(:key), 'bf')) AS ccard_year_expired,"
    "  ccard_type,"
    "  custinfo.* "
    "  FROM ccard, custinfo "
    "WHERE ((ccard_id=:ccardid)"
    "  AND  (ccard_cust_id=cust_id));");
  anq.bindValue(":ccardid", pccardid);
  anq.bindValue(":key",     omfgThis->_key);
  anq.exec();

  if (anq.first())
  {
    if (!anq.value("ccard_active").toBool())
    {
      _errorMsg = errorMsg(-10);
      return -10;
    }
  }
  else if (anq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = anq.lastError().databaseText();
    return -1;
  }
  else
  {
    _errorMsg = errorMsg(-17).arg(pccardid);
    return -17;
  }

  _extraHeaders.clear();
  _extraHeaders.append(qMakePair("Stateless-Transaction", "true"));
  _extraHeaders.append(qMakePair("Auth-MID", _metricsenc->value("CCPTDivisionNumber").rightJustified(10, '0', true)));
  _extraHeaders.append(qMakePair("Auth-User", _metricsenc->value("CCLogin")));
  _extraHeaders.append(qMakePair("Auth-Password", _metricsenc->value("CCPassword")));
  _extraHeaders.append(qMakePair("Content-type", "SALEM05210/SLM"));

  prequest = "P74V";
  prequest += pordernum.leftJustified(22, ' ', true); // TODO: should we check to make sure isn't empty?

  QString ccardType = anq.value("ccard_type").toString();
  if("V" == ccardType) // Visa
    ccardType = "VI";
  else if("M" == ccardType) // Master Card
    ccardType = "MC";
  else if("A" == ccardType) // American Express
    ccardType = "AX";
  else if("D" == ccardType) // Discover
    ccardType = "DI";
  else if("P" == ccardType) // PayPal
    ccardType = "PY";
  else
  {
    _errorMsg = errorMsg(-209);
    _return -209;
  }
  prequest += ccardType;

  prequest += anq.value("ccard_number").toString().leftJustified(19, ' ', true);

  QString work_month;
  work_month.setNum(anq.value("ccard_month_expired").toDouble());
  if (work_month.length() == 1)
    work_month = "0" + work_month;
  prequest += work_month + anq.value("ccard_year_expired").toString().right(2);

  prequest += _metricsenc->value("CCPTDivisionNumber").rightJustified(10, '0', true);

  double shiftedAmt = pamount * 100.0;
  int amount = (int)shiftedAmt;
  prequest += QString::number(amount).rightJustified(12, '0', true);

  // TODO: this needs to be changed to support non-us
  prequest += "840"; // CurrencyCode: U.S. Dollars

  prequest += "1"; // TransactionType: 1 - single trans over mail/phone card holder not present

  prequest += "    "; // EncryptionFlag, PaymentIndicator: both optional not using

  prequest += pordertype; // ActionCode 2 digit

  prequest += " "; // Reserved

  // Bill To Address Information
  prequest += "AB";
  prequest += "               "; // TelephoneType, TelephoneNumber (1,14, Optional)
  prequest += anq.value("ccard_name").toString().leftJustified(30, ' ', true);
  prequest += anq.value("ccard_address1").toString().leftJustified(30, ' ', true);
  prequest += anq.value("ccard_address2").toString().leftJustified(28, ' ', true);

  prequest += "  "; // CountryCode (2, optional) ccard_country // TODO: figure out how to handle this appropriately if we want to at all
  prequest += anq.value("ccard_city").toString().leftJustified(20, ' ', true);
  prequest += "  "; // State (2, optional) ccard_state // TODO: figure out how to handle this appropriately if we want to at all
  prequest += anq.value("ccard_zip").toString().leftJustified(10, ' ', true);

/* TODO: should probably do something about this in relation to above with the CurrencyCode
  anq.prepare("SELECT curr_abbr FROM curr_symbol WHERE (curr_id=:currid);");
  anq.bindValue(":currid", pcurrid);
  anq.exec();
  if (anq.first())
  {
    APPENDFIELD(prequest, "x_currency_code", anq.value("curr_abbr").toString());
  }
  else if (anq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = anq.lastError().databaseText();
    return -1;
  }
  else
  {
    _errorMsg = errorMsg(-17).arg(pccardid);
    return -17;
  }
*/

  if (pcvv > 0)
    APPENDFIELD(prequest, "x_card_code", pcvv);

  if (DEBUG)
    qDebug("Paymentech:buildCommon built %s\n", prequest.toAscii().data());
  return 0;
}

// TODO
int  PaymentechProcessor::doAuthorize(const int pccardid, const int pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString& pneworder, QString& preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("Paymentech:doAuthorize(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount, ptax, ptaxexempt,  pfreight,  pduty, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid);

  int    returnValue = 0;
  double amount  = pamount;
  double tax     = ptax;
  double freight = pfreight;
  double duty    = pduty;
  int    currid  = pcurrid;

  if (_metrics->value("CCANCurrency") != "TRANS")
  {
    currid = _metrics->value("CCANCurrency").toInt();
    amount = currToCurr(pcurrid, currid, pamount,   &returnValue);
    if (returnValue < 0)
      return returnValue;
    tax     = currToCurr(pcurrid, currid, ptax,     &returnValue);
    if (returnValue < 0)
      return returnValue;
    freight = currToCurr(pcurrid, currid, pfreight, &returnValue);
    if (returnValue < 0)
      return returnValue;
    duty    = currToCurr(pcurrid, currid, pduty,    &returnValue);
    if (returnValue < 0)
      return returnValue;
  }

  QString request;
  returnValue = buildCommon(pneworder, pccardid, pcvv, amount, currid, request, "RC");
  if (returnValue != 0)
    return returnValue;

  APPENDFIELD(request, "x_tax",        QString::number(tax));
  APPENDFIELD(request, "x_tax_exempt", ptaxexempt ? "TRUE" : "FALSE");
  APPENDFIELD(request, "x_freight",    QString::number(freight));
  APPENDFIELD(request, "x_duty",       QString::number(duty));

  if (! preforder.isEmpty())
    APPENDFIELD(request, "x_po_num", preforder.left(20));

  QString response;

  returnValue = sendViaHTTP(request, response);
  if (returnValue < 0)
    return returnValue;

  returnValue = handleResponse(response, pccardid, "A", amount, currid,
			       pneworder, preforder, pccpayid, pparams);

  return returnValue;
}

// TODO
int  PaymentechProcessor::doCharge(const int pccardid, const int pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString& pneworder, QString& preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("Paymentech:doCharge(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount,  ptax, ptaxexempt,  pfreight,  pduty, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid);

  int    returnValue = 0;
  double amount  = pamount;
  double tax     = ptax;
  double freight = pfreight;
  double duty    = pduty;
  int    currid  = pcurrid;

  if (_metrics->value("CCANCurrency") != "TRANS")
  {
    currid = _metrics->value("CCANCurrency").toInt();
    amount = currToCurr(pcurrid, currid, pamount,   &returnValue);
    if (returnValue < 0)
      return returnValue;
    tax     = currToCurr(pcurrid, currid, ptax,     &returnValue);
    if (returnValue < 0)
      return returnValue;
    freight = currToCurr(pcurrid, currid, pfreight, &returnValue);
    if (returnValue < 0)
      return returnValue;
    duty    = currToCurr(pcurrid, currid, pduty,    &returnValue);
    if (returnValue < 0)
      return returnValue;
  }

  QString request;

  returnValue = buildCommon(pneworder, pccardid, pcvv, amount, currid, request, "RP");
  if (returnValue !=  0)
    return returnValue;

  APPENDFIELD(request, "x_tax",        QString::number(tax));
  APPENDFIELD(request, "x_tax_exempt", ptaxexempt ? "TRUE" : "FALSE");
  APPENDFIELD(request, "x_freight",    QString::number(freight));
  APPENDFIELD(request, "x_duty",       QString::number(duty));

  if (! preforder.isEmpty())
    APPENDFIELD(request, "x_po_num",   preforder);

  QString response;

  returnValue = sendViaHTTP(request, response);
  if (returnValue < 0)
    return returnValue;

  returnValue = handleResponse(response, pccardid, "C", amount, currid,
			       pneworder, preforder, pccpayid, pparams);

  return returnValue;
}

// TODO
int PaymentechProcessor::doChargePreauthorized(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("Paymentech:doChargePreauthorized(%d, %d, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount,  pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid);

  int    returnValue = 0;
  double amount  = pamount;
  int    currid  = pcurrid;

  if (_metrics->value("CCANCurrency") != "TRANS")
  {
    currid = _metrics->value("CCANCurrency").toInt();
    amount = currToCurr(pcurrid, currid, pamount, &returnValue);
    if (returnValue < 0)
      return returnValue;
  }

  QString request;

  returnValue = buildCommon(pneworder, pccardid, pcvv, amount, currid, request, "AU");
  if (returnValue !=  0)
    return returnValue;

  APPENDFIELD(request, "x_trans_id", preforder);

  QString response;

  returnValue = sendViaHTTP(request, response);
  if (returnValue < 0)
    return returnValue;

  returnValue = handleResponse(response, pccardid, "CP", amount, currid,
			       pneworder, preforder, pccpayid, pparams);

  return returnValue;
}

// TODO
int PaymentechProcessor::doCredit(const int pccardid, const int pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("Paymentech:doCredit(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount, ptax, ptaxexempt,  pfreight,  pduty, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid);

  int    returnValue = 0;
  double amount  = pamount;
  double tax     = ptax;
  double freight = pfreight;
  double duty    = pduty;
  int    currid  = pcurrid;

  if (_metrics->value("CCANCurrency") != "TRANS")
  {
    currid = _metrics->value("CCANCurrency").toInt();
    amount = currToCurr(pcurrid, currid, pamount, &returnValue);
    if (returnValue < 0)
      return returnValue;
    tax = currToCurr(pcurrid, currid, ptax, &returnValue);
    if (returnValue < 0)
      return returnValue;
    freight = currToCurr(pcurrid, currid, pfreight, &returnValue);
    if (returnValue < 0)
      return returnValue;
    duty = currToCurr(pcurrid, currid, pduty, &returnValue);
    if (returnValue < 0)
      return returnValue;
  }

  QString request;

  returnValue = buildCommon(pneworder, pccardid, pcvv, amount, currid, request, "RF");
  if (returnValue !=  0)
    return returnValue;

  APPENDFIELD(request, "x_trans_id",   preforder);
  APPENDFIELD(request, "x_tax",        QString::number(tax));
  APPENDFIELD(request, "x_tax_exempt", ptaxexempt ? "TRUE" : "FALSE");
  APPENDFIELD(request, "x_freight",    QString::number(freight));
  APPENDFIELD(request, "x_duty",       QString::number(duty));

  QString response;
  returnValue = sendViaHTTP(request, response);
  if (returnValue < 0)
    return returnValue;

  returnValue = handleResponse(response, pccardid, "R", amount, currid,
			       pneworder, preforder, pccpayid, pparams);

  return returnValue;
}

// TODO
int PaymentechProcessor::doVoidPrevious(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, QString &papproval, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("Paymentech:doVoidPrevious(%d, %d, %f, %d, %s, %s, %s, %d)",
	   pccardid, pcvv, pamount, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(),
	   papproval.toAscii().data(), pccpayid);

  QString tmpErrorMsg = _errorMsg;

  int    returnValue = 0;
  double amount = pamount;
  int    currid = pcurrid;

  if (_metrics->value("CCANCurrency") != "TRANS")
  {
    currid = _metrics->value("CCANCurrency").toInt();
    amount = currToCurr(pcurrid, currid, pamount, &returnValue);
    if (returnValue < 0)
    {
      _errorMsg = tmpErrorMsg;
      return returnValue;
    }
  }

  QString request;

  returnValue = buildCommon(pneworder, pccardid, pcvv, amount, currid, request, "AR");
  if (returnValue != 0)
    return returnValue;

  APPENDFIELD(request, "x_trans_id", preforder);

  QString response;

  returnValue = sendViaHTTP(request, response);
  _errorMsg = tmpErrorMsg;
  if (returnValue < 0)
    return returnValue;

  returnValue = handleResponse(response, pccardid, "V", amount, currid,
			       pneworder, preforder, pccpayid, pparams);
  if (! tmpErrorMsg.isEmpty())
    _errorMsg = tmpErrorMsg;
  return returnValue;
}

// TODO
int PaymentechProcessor::fieldValue(const QStringList plist, const int pindex, QString &pvalue)
{
  if (plist.size() < pindex)
  {
    _errorMsg = errorMsg(-205).arg(pindex).arg(plist.size());
    return -205;
  }

  if (_metrics->value("CCANEncap").isEmpty())
    pvalue = plist.at(pindex - 1);
  else
  {
    pvalue = plist.at(pindex - 1);
    // now strip of the encapsulating char from front and back
    int firstPos = plist.at(pindex - 1).indexOf(_metrics->value("CCANEncap"));
    int lastPos  = plist.at(pindex - 1).lastIndexOf(_metrics->value("CCANEncap"));
    pvalue = pvalue.mid(firstPos + 1, lastPos - firstPos - 1);
  }
  if (DEBUG)
    qDebug("Paymentech:fieldValue of %d is %s", pindex, pvalue.toAscii().data());
  return 0;
}

// TODO
int PaymentechProcessor::handleResponse(const QString &presponse, const int pccardid, const QString &ptype, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("Paymentech::handleResponse(%s, %d, %s, %f, %d, %s, %d, pparams)",
	   presponse.toAscii().data(), pccardid,
	   ptype.toAscii().data(), pamount, pcurrid,
	   preforder.toAscii().data(), pccpayid);

  // if we got an error msg very early on
  if (presponse.startsWith("<HTML>"))
  {
    _errorMsg = errorMsg(-207).arg(presponse);
    return -207;
  }

  QString delim = _metrics->value("CCANDelim").isEmpty() ? "," :
						  _metrics->value("CCANDelim");
  QString encap = _metrics->value("CCANEncap");

  QString r_approved;
  QString r_avs;
  QString r_code;
  QString r_cvv;
  QString r_error;
  QString r_message;
  QString r_ordernum;
  QString r_reason;     // not stored
  QString r_ref;
  QString r_shipping;
  QString r_tax;

  QString status;

  // TODO: explore using encap here and code from CSV Import to properly split

  /* add an extra field at the beginning. otherwise we'll be off by one
     because the Advanced Integration Method (AIM) Implementation Guide
     numbers fields starting at 1 but QString::split() creates a list
     starting at 0.
   */
  QStringList responseFields = presponse.split(delim);
  
  QString r_response;
  int returnValue = fieldValue(responseFields, 1, r_response);
  if (returnValue < 0)
    return returnValue;

  if (r_response.toInt() == 1)
    r_approved = "APPROVED";
  else if (r_response.toInt() == 2)
    r_approved = "DECLINED";
  else if (r_response.toInt() == 3)
    r_approved = "ERROR";
  else if (r_response.toInt() == 4)
    r_approved = "HELDFORREVIEW";

  // fieldValue(responseFields, 2);				// subcode

  returnValue = fieldValue(responseFields, 3, r_reason);	// reason code
  if (returnValue < 0)
    return returnValue;
  returnValue = fieldValue(responseFields, 4, r_message);	// reason text
  if (returnValue < 0)
    return returnValue;

  returnValue = fieldValue(responseFields, 5, r_code);	 	// approval code
  if (returnValue < 0)
    return returnValue;
  returnValue = fieldValue(responseFields, 6, r_avs);	 	// avs result
  if (returnValue < 0)
    return returnValue;
  returnValue = fieldValue(responseFields, 7, r_ordernum);	// transaction id

  if (returnValue < 0)
    return returnValue;

  // fieldValue(responseFields, 8-10);	// echo invoice_number description amount 
  // fieldValue(responseFields, 11-13);	// echo method type cust_id
  // fieldValue(responseFields, 14-24);	// echo name, company, and address info
  // fieldValue(responseFields, 25-32);	// echo ship_to fields

  returnValue = fieldValue(responseFields, 33, r_tax);		// echo x_tax
  if (returnValue < 0)
    return returnValue;

  // fieldValue(responseFields, 34);				// echo x_duty

  returnValue = fieldValue(responseFields, 35, r_shipping);	// echo x_freight
  if (returnValue < 0)
    return returnValue;

  // fieldValue(responseFields, 36);		// echo x_tax_exempt
  // fieldValue(responseFields, 37);		// echo x_po_num

  returnValue = fieldValue(responseFields, 39, r_cvv); // ccv response code
  if (returnValue < 0)
    return returnValue;

  // fieldValue(responseFields, 40);		// cavv response code
  // fieldValue(responseFields, 41-68);		// reserved for future use
  // fieldValue(responseFields, 69+);		// echo of merchant-defined fields

  /* treat heldforreview as approved because the AIM doc says response
     reason codes 252 and 253 are both approved but being reviewed.
     the intent of the other heldforreview, 193, is ambiguous.
   */
  if (r_approved == "APPROVED" || r_approved == "HELDFORREVIEW")
  {
    _errorMsg = errorMsg(0).arg(r_code);
    if (ptype == "A")
      status = "A";	// Authorized
    else if (ptype == "V")
      status = "V";	// Voided
    else
      status = "C";	// Completed/Charged
  }
  else if (r_approved == "DECLINED")
  {
    _errorMsg = errorMsg(-92).arg(r_message);
    returnValue = -92;
    status = "D";
  }
  else if (r_approved == "ERROR")
  {
    r_error = r_message;
    _errorMsg = errorMsg(-12).arg(r_error);
    returnValue = -12;
    status = "X";
  }

  else if (r_approved.isEmpty() && ! r_message.isEmpty())
  {
    _errorMsg = errorMsg(-95).arg(r_message);
    returnValue = -95;
    status = "X";
  }

  else if (r_approved.isEmpty())
  {
    _errorMsg = errorMsg(-100).arg(r_error).arg(r_message).arg(presponse);
    returnValue = -100;
    status = "X";
  }

  // always use the AVS checking configured on the gateway
  _passedAvs = ((r_reason.toInt() != 27) &&
	        (r_reason.toInt() != 127));

  // always use the CVV checking configured on the gateway
  _passedCvv = (r_reason.toInt() != 78);

  if (DEBUG)
    qDebug("Paymentech:%s _passedAvs %d\t%s _passedCvv %d",
	    r_avs.toAscii().data(), _passedAvs,
	    r_cvv.toAscii().data(), _passedCvv);

  pparams.append("ccard_id",    pccardid);
  pparams.append("currid",      pcurrid);
  pparams.append("auth_charge", ptype);
  pparams.append("type",        ptype);
  pparams.append("reforder",    (preforder.isEmpty()) ? pneworder : preforder);
  pparams.append("status",      status);
  pparams.append("avs",         r_avs);
  pparams.append("ordernum",    pneworder);
  pparams.append("xactionid",   r_ordernum);
  pparams.append("error",       r_error);
  pparams.append("approved",    r_approved);
  pparams.append("code",        r_code);
  pparams.append("shipping",    r_shipping);
  pparams.append("tax",         r_tax);
  pparams.append("ref",         r_ref);
  pparams.append("message",     r_message);

  if (ptype == "A")
    pparams.append("auth", QVariant(true, 0));
  else
    pparams.append("auth", QVariant(false, 1));

  if (DEBUG)
    qDebug("Paymentech:r_error.isEmpty() = %d", r_error.isEmpty());

  if (returnValue == 0)
    pparams.append("amount",   pamount);
  else
    pparams.append("amount",   0);	// no money changed hands this attempt

  // don't bother checking MD5 if we hit a bigger problem
  if (returnValue == 0 && _metrics->boolean("CCANMD5HashSetOnGateway"))
  {
    QString expected_hash;
    QString r_hash;

    returnValue = fieldValue(responseFields, 38, r_hash);	// md5 hash
    XSqlQuery anq;
    anq.prepare("SELECT UPPER(MD5(:inputstr)) AS expected;");
    anq.bindValue(":inputstr", _metricsenc->value("CCANMD5Hash") +
			       _metricsenc->value("CCLogin") +
			       r_ordernum +
			       QString::number(pamount, 'f', 2));
    anq.exec();
    if (anq.first())
      expected_hash = anq.value("expected").toString();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      _errorMsg = errorMsg(-1).arg(anq.lastError().databaseText());
      returnValue = -1;
    }
    if (DEBUG)
      qDebug("Paymentech:handleResponse expected md5 %s and got %s",
	      expected_hash.toAscii().data(), r_hash.toAscii().data());

    if (_metrics->value("CCANMD5HashAction") == "F" && expected_hash != r_hash)
    {
      _errorMsg = errorMsg(-206);
      returnValue = -206;
    }
    else if (_metrics->value("CCANMD5HashAction") == "W" && expected_hash != r_hash)
    {
      _errorMsg = errorMsg(206);
      returnValue = 206;
    }
  }

  if (DEBUG)
    qDebug("Paymentech::handleResponse returning %d %s",
           returnValue, errorMsg().toAscii().data());
  return returnValue;
}

int PaymentechProcessor::doTestConfiguration()
{
  if (DEBUG)
    qDebug("Pt:doTestConfiguration()");

  int returnValue = 0;

  if (_metricsenc->value("CCLogin").isEmpty())
  {
    _errorMsg = errorMsg(-200);
    returnValue = -200;
  }
  else if (_metricsenc->value("CCPassword").isEmpty())
  {
    _errorMsg = errorMsg(-201);
    returnValue = -201;
  }
  else if (_metricsenc->value("CCPTDivisionNumber").size() > 10)
  {
    _errorMsg = errorMsg(-202);
    returnValue = -202;
  }

  return returnValue;
}

bool PaymentechProcessor::handlesCreditCards()
{
  return true;
}
