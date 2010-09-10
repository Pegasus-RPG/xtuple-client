/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QDomAttr>
#include <QDomDocument>
#include <QDomElement>
#include <QSqlError>

#include <currcluster.h>

#include "guiclient.h"
#include "cybersourceprocessor.h"

#define DEBUG true

#define SOAP_ENV_NS "http://schemas.xmlsoap.org/soap/envelope/"
#define WSSE_NS "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd"
//#define NS1_NS "urn:schemas-cybersource-com:transaction-data-1.31"
#define NS1_NS "https://ics2ws.ic3.com/commerce/1.x/transactionProcessor/CyberSourceTransaction_1.53.xsd"

#define TR(a)	QObject::tr(a)

// convenience function to add <ChildName>Content</ChildName> to the Parent node
/* TODO: put this in a generic XMLCCProcessor, subclass of creditcardprocessor?
         make a generic XMLHelper class and make CyberSourceProcessor and
         YourPayProcessor inherit both CreditCardProcessor and XMLHelper?
 */

static QDomElement createChildTextNode(QDomElement parent, QString childName, QString content)
{
  QDomElement child;
  child = parent.ownerDocument().createElement(childName);
  child.appendChild(parent.ownerDocument().createTextNode(content));
  parent.appendChild(child);
  return child;
}

CyberSourceProcessor::CyberSourceProcessor() : CreditCardProcessor()
{
  _defaultLivePort   = 0;
  _defaultLiveServer = "https://ics2ws.ic3.com/commerce/1.x/transactionProcessor";
  _defaultTestPort   = 0;
  _defaultTestServer = "https://ics2wstest.ic3.com/commerce/1.x/transactionProcessor";

  _msgHash.insert(-300, tr("The Merchant ID is required"));
  _msgHash.insert(-301, tr("The message sent to CyberSource was incomplete: %1"));
  _msgHash.insert(-302, tr("The message sent to CyberSource had invalid fields: %1"));
  _msgHash.insert(-303, tr("CyberSource rejected or returned an error for this request (code %1)"));
  _msgHash.insert(-304, tr("CyberSource reports a general system failure"));
  _msgHash.insert(-305, tr("The Merchant ID %1 is too long"));
  _msgHash.insert(-306, tr("SOAP error (probably an xTuple ERP bug): %1"));
  _msgHash.insert(-310, tr("The amount authorized was 0."));
  _msgHash.insert( 310, tr("Only a portion of the total amount requested was authorized"));

  if (FraudCheckResult *b = avsCodeLookup('B'))
  {
    b->sev  = NoMatch | PostalCode;
    b->text = TR("Street Address matches but Postal Code is not verified");
  }
  _avsCodes.append(new FraudCheckResult('C', NoMatch | Address | PostalCode, TR("Street Address and Postal Code do not match")));
  _avsCodes.append(new FraudCheckResult('D', Match,   TR("Street Address and Postal Code match")));

  if (FraudCheckResult *e = avsCodeLookup('E'))
    e->text = TR("AVS data are invalid or AVS not allowed for this card type");

  _avsCodes.append(new FraudCheckResult('F', NoMatch | Name, TR("Card holder's name does not match but Postal Code matches")));
  _avsCodes.append(new FraudCheckResult('H', NoMatch | Name, TR("Card holder's name does not match but Street Address and Postal Code match")));
  _avsCodes.append(new FraudCheckResult('I', NotProcessed | Address, TR("Address not verified")));
  _avsCodes.append(new FraudCheckResult('K', NoMatch | Address | PostalCode, TR("Card holder's name matches but Billing Street Address and Postal Code do not match")));
  _avsCodes.append(new FraudCheckResult('L', NoMatch | Address, TR("Card holder's name and Billing Postal Code match but Street Address does not match")));
  _avsCodes.append(new FraudCheckResult('M', Match,   TR("Street Address and Postal Code match")));
  _avsCodes.append(new FraudCheckResult('O', NoMatch | PostalCode, TR("Card holder's name and Billing Street Address match but Postal Code does not match")));

  if (FraudCheckResult *p = avsCodeLookup('P'))
  {
    p->sev  = NotProcessed | Address;
    p->text = TR("Postal Code matches but Street Address was not verified");
  }

  _avsCodes.append(new FraudCheckResult('T', NoMatch | Name,     TR("Card holder's name does not match but Street Address matches")));
  _avsCodes.append(new FraudCheckResult('U', NotProcessed | Address, TR("Address information unavailable; either the US bank does not support non-US AVS or the AVS at a US bank is not functioning properly")));
  _avsCodes.append(new FraudCheckResult('V', Match,       TR("Card holder's name, Street Address, and Postal Code match")));
  _avsCodes.append(new FraudCheckResult('1', Unsupported, TR("AVS is not supported for this processor or card type")));
  _avsCodes.append(new FraudCheckResult('2', Invalid,     TR("The processor returned an unrecognized AVS response")));

  _cvvCodes.append(new FraudCheckResult('D', Suspicious,  TR("The bank thinks this transaction is suspicious")));
  _cvvCodes.append(new FraudCheckResult('I', NoMatch,     TR("The CVV failed the processor's data validation check")));
  _cvvCodes.append(new FraudCheckResult('1', Unsupported, TR("CVV is not supported by the card association")));
  _cvvCodes.append(new FraudCheckResult('2', Invalid,     TR("The processor returned an unrecognized CVV response")));
  _cvvCodes.append(new FraudCheckResult('3', NotAvail,    TR("The processor did not return a CVV result")));

  _doc = 0;
}

CyberSourceProcessor::~CyberSourceProcessor()
{
  if (_doc)
  {
    delete _doc;
    _doc = 0;
  }
}

int CyberSourceProcessor::buildCommon(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, const CCTransaction ptranstype)
{
  XSqlQuery csq;
  csq.prepare(
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
    "  ccard_type, cust_name, cntct_email"
    "  FROM ccard"
    "  JOIN custinfo ON (ccard_cust_id=cust_id)"
    "  LEFT OUTER JOIN cntct ON (cust_cntct_id=cntct_id)"
    "WHERE (ccard_id=:ccardid);");

  csq.bindValue(":ccardid", pccardid);
  csq.bindValue(":key",     omfgThis->_key);
  csq.exec();

  if (csq.first())
  {
    if (!csq.value("ccard_active").toBool())
    {
      _errorMsg = errorMsg(-10);
      return -10;
    }
  }
  else if (csq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = csq.lastError().databaseText();
    return -1;
  }
  else
  {
    _errorMsg = errorMsg(-17).arg(pccardid);
    return -17;
  }

  if (_doc)
    delete _doc;
  _doc = new QDomDocument();

  QDomAttr attr;

  QDomNode xmlNode = _doc->createProcessingInstruction("xml",
                                                       "version=\"1.0\" "
                                                       "encoding=\"UTF-8\"");
  _doc->appendChild(xmlNode);
  

  QDomElement envelope = _doc->createElementNS(SOAP_ENV_NS, "soap:Envelope");
  _doc->appendChild(envelope);

  QDomElement header = _doc->createElement("soap:Header");
  header.setAttribute("xmlns:wsse", WSSE_NS);
  envelope.appendChild(header);

  QDomElement security = _doc->createElement("wsse:Security");
  security.setAttribute("soap:mustUnderstand", "1");
  header.appendChild(security);

  QDomElement usernametoken = _doc->createElement("wsse:UsernameToken");
  security.appendChild(usernametoken);

  createChildTextNode(usernametoken, "wsse:Username",
                      _metricsenc->value("CCLogin").toLower());

  QDomElement password = createChildTextNode(usernametoken, "wsse:Password",
                                             _metricsenc->value("CCPassword"));

  password.setAttribute("Type",
                        "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordText");

  QDomElement body = _doc->createElement("soap:Body");
  envelope.appendChild(body);

  _requestMessage = _doc->createElementNS(NS1_NS, "ns1:requestMessage");
  body.appendChild(_requestMessage);

  createChildTextNode(_requestMessage, "ns1:merchantID", 
                      _metricsenc->value("CCLogin"));
  createChildTextNode(_requestMessage, "ns1:merchantReferenceCode",
                      pneworder);

  if (ptranstype == Reverse || ptranstype == Capture ||
      ptranstype == Credit  || ptranstype == Void)
    createChildTextNode(_requestMessage, "ns1:orderRequestToken", preforder);

  if (ptranstype == Authorize || ptranstype == Charge || ptranstype == Credit)
  {
    QString country = csq.value("ccard_country").toString();
    if (country.length() > 2)
    {
      XSqlQuery countryq;
      countryq.prepare("SELECT country_abbr FROM country WHERE country_name=:name;");
      countryq.bindValue(":name", country);
      countryq.exec();
      if (countryq.first())
        country = countryq.value("country_abbr").toString();
      else if (csq.lastError().type() != QSqlError::NoError)
      {
        _errorMsg = csq.lastError().databaseText();
        return -1;
      }
    }

    QDomElement billto = _doc->createElement("ns1:billTo");
    _requestMessage.appendChild(billto);
    QStringList name = csq.value("ccard_name").toString().split(QRegExp("\\s+"));
    createChildTextNode(billto, "ns1:firstName",  name.at(0).left(60));
    createChildTextNode(billto, "ns1:lastName",   name.at(name.size() - 1).left(60));
    createChildTextNode(billto, "ns1:street1",    csq.value("ccard_address1").toString().left(60));
    createChildTextNode(billto, "ns1:city",       csq.value("ccard_city").toString().left(50));
    createChildTextNode(billto, "ns1:state",      csq.value("ccard_state").toString().left(2));
    createChildTextNode(billto, "ns1:postalCode", csq.value("ccard_zip").toString().left(10));
    createChildTextNode(billto, "ns1:country",    country.left(2));
    createChildTextNode(billto, "ns1:email",      csq.value("cntct_email").toString().left(255));
    createChildTextNode(billto, "ns1:company",    csq.value("cust_name").toString().left(40));
  }

  if (ptranstype == Authorize || ptranstype == Reverse ||
      ptranstype == Capture   || ptranstype == Charge  || ptranstype == Credit)
  {
    QDomElement purchasetotals = _doc->createElement("ns1:purchaseTotals");
    _requestMessage.appendChild(purchasetotals);
    createChildTextNode(purchasetotals, "ns1:grandTotalAmount",
                        QString::number(pamount));
    XSqlQuery currq;
    currq.prepare("SELECT curr_abbr FROM curr_symbol WHERE (curr_id=:currid);");
    currq.bindValue(":currid", pcurrid);
    currq.exec();
    if (currq.first())
      createChildTextNode(purchasetotals, "ns1:currency",
                          currq.value("curr_abbr").toString());
    else if (currq.lastError().type() != QSqlError::NoError)
    {
      _errorMsg = currq.lastError().databaseText();
      return -1;
    }
    else
    {
      _errorMsg = errorMsg(-17).arg(pccardid);
      return -17;
    }
  }

  if (ptranstype == Authorize || ptranstype == Charge || ptranstype == Credit)
  {
    QString rawcardtype = csq.value("ccard_type").toString();
    QString cardtype;
    if (rawcardtype == "A")
      cardtype = "003";
    else if (rawcardtype == "D")
      cardtype = "004";
    else if (rawcardtype == "M")
      cardtype = "002";
    else if (rawcardtype == "V")
      cardtype = "001";
    else if (rawcardtype == "Diners Club")
      cardtype = "005";
    else if (rawcardtype == "Carte Blanche")
      cardtype = "006";
    else if (rawcardtype == "JCB")
      cardtype = "007";
    else
      cardtype = rawcardtype;

    QDomElement card = _doc->createElement("ns1:card");
    _requestMessage.appendChild(card);
    QString accountNumber = csq.value("ccard_number").toString();
    accountNumber.remove(QRegExp("[^0-9]"));

    QString month = csq.value("ccard_month_expired").toString();
    if (month.length() == 1)
      month = "0" + month;

    createChildTextNode(card, "ns1:accountNumber",   accountNumber.left(20));
    createChildTextNode(card, "ns1:expirationMonth", month.left(2));
    createChildTextNode(card, "ns1:expirationYear",
                        csq.value("ccard_year_expired").toString().left(4));
    if (pcvv > 0)
      createChildTextNode(card, "ns1:cvNumber",
                          QString::number(pcvv).left(4));
    createChildTextNode(card, "ns1:cardType",        cardtype.left(3));
  }

  if (DEBUG)
    qDebug("CS:buildCommon built %s\n", qPrintable(_doc->toString()));
  return 0;
}

int  CyberSourceProcessor::doAuthorize(const int pccardid, const int pcvv, double &pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString& pneworder, QString& preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("CS:doAuthorize(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d)",
           pccardid, pcvv, pamount, ptax, ptaxexempt,  pfreight,  pduty,
           pcurrid, qPrintable(pneworder), qPrintable(preforder), pccpayid);

  int returnValue = buildCommon(pccardid, pcvv, pamount, pcurrid, pneworder,
                                preforder, Authorize);
  if (returnValue != 0)
    return returnValue;

  QDomElement auth = _doc->createElement("ns1:ccAuthService");
  auth.setAttribute("run", "true");
  _requestMessage.appendChild(auth);

  if (DEBUG) qDebug("CS::doAuthorize sending %s", qPrintable(_doc->toString()));

  QString response;
  returnValue = sendViaHTTP(_doc->toString(), response);
  if (returnValue < 0)
    return returnValue;

  returnValue = handleResponse(response, pccardid, Authorize, pamount, pcurrid,
                               pneworder, preforder, pccpayid, pparams);

  return returnValue;
}

int  CyberSourceProcessor::doCharge(const int pccardid, const int pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString& pneworder, QString& preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("CS:doCharge(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d)",
           pccardid, pcvv, pamount,  ptax, ptaxexempt,  pfreight,  pduty, pcurrid,
           qPrintable(pneworder), qPrintable(preforder), pccpayid);

  int returnValue = buildCommon(pccardid, pcvv, pamount, pcurrid, pneworder, preforder, Charge);
  if (returnValue != 0)
    return returnValue;

  QDomElement auth = _doc->createElement("ns1:CCAuthService");
  auth.setAttribute("run", "true");
  _requestMessage.appendChild(auth);

  QDomElement capture = _doc->createElement("ns1:CCCaptureService");
  capture.setAttribute("run", "true");
  _requestMessage.appendChild(capture);

  if (DEBUG) qDebug("CS::doCharge sending %s", qPrintable(_doc->toString()));
  QString response;
  returnValue = sendViaHTTP(_doc->toString(), response);
  if (returnValue < 0)
    return returnValue;

  double amount = pamount;
  returnValue = handleResponse(response, pccardid, Charge, amount, pcurrid,
                               pneworder, preforder, pccpayid, pparams);

  return returnValue;
}

int CyberSourceProcessor::doChargePreauthorized(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("CS:doChargePreauthorized(%d, %d, %f, %d, %s, %s, %d)",
           pccardid, pcvv, pamount,  pcurrid,
           qPrintable(pneworder), qPrintable(preforder), pccpayid);

  int returnValue = buildCommon(pccardid, pcvv, pamount, pcurrid, pneworder, preforder, Capture);
  if (returnValue != 0)
    return returnValue;

  QDomElement capture = _doc->createElement("ns1:CCCaptureService");
  capture.setAttribute("run", "true");
  _requestMessage.appendChild(capture);

  createChildTextNode(capture, "ns1:authRequestId",     preforder);
  if (pneworder.length() > 0)
    createChildTextNode(capture, "ns1:orderRequestToken", pneworder);

  if (DEBUG) qDebug("CS::doChargePreauthorized sending %s", qPrintable(_doc->toString()));
  QString response;
  returnValue = sendViaHTTP(_doc->toString(), response);
  if (returnValue < 0)
    return returnValue;

  double amount = pamount;
  returnValue = handleResponse(response, pccardid, Capture, amount, pcurrid,
                               pneworder, preforder, pccpayid, pparams);

  return returnValue;
}

int CyberSourceProcessor::doCredit(const int pccardid, const int pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("CS:doCredit(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d)",
           pccardid, pcvv, pamount, ptax, ptaxexempt,  pfreight,  pduty, pcurrid,
           qPrintable(pneworder), qPrintable(preforder), pccpayid);

  int returnValue = buildCommon(pccardid, pcvv, pamount, pcurrid, pneworder, preforder, Credit);
  if (returnValue != 0)
    return returnValue;

  QDomElement capture = _doc->createElement("ns1:CCCreditService");
  capture.setAttribute("run", "true");
  _requestMessage.appendChild(capture);

  createChildTextNode(capture, "ns1:captureRequestID",  preforder);
  if (pneworder.length() > 0)
    createChildTextNode(capture, "ns1:orderRequestToken", pneworder);

  if (DEBUG) qDebug("CS::doCredit sending %s", qPrintable(_doc->toString()));
  QString response;
  returnValue = sendViaHTTP(_doc->toString(), response);
  if (returnValue < 0)
    return returnValue;

  double amount = pamount;
  returnValue = handleResponse(response, pccardid, Credit, amount, pcurrid,
                               pneworder, preforder, pccpayid, pparams);
  return returnValue;
}

int CyberSourceProcessor::doReverseAuthorize(const int pccardid, const double pamount, const int pcurrid, QString& pneworder, QString& preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("CS::doReverseAuthorize(%d, %f, %d, %s, %s, %d, pparams",
           pccardid, pamount, pcurrid, qPrintable(pneworder), qPrintable(preforder), pccpayid);

  QString tmpErrorMsg = _errorMsg;
  int returnValue = buildCommon(pccardid, -2, pamount, pcurrid, pneworder, preforder, Reverse);
  if (returnValue != 0)
    return returnValue;

  QDomElement reverse = _doc->createElement("ns1:CCAuthReversalService");
  reverse.setAttribute("run", "true");
  _requestMessage.appendChild(reverse);

  createChildTextNode(reverse, "ns1:authRequestID",     preforder.left(26));
  if (pneworder.length() > 0)
    createChildTextNode(reverse, "ns1:orderRequestToken", pneworder.left(256));

  if (DEBUG) qDebug("CS::doReverseAuthorize sending %s", qPrintable(_doc->toString()));

  QString response;
  returnValue = sendViaHTTP(_doc->toString(), response);
  _errorMsg = tmpErrorMsg;
  if (returnValue < 0)
    return returnValue;

  double amount = pamount;
  returnValue = handleResponse(response, pccardid, Reverse, amount, pcurrid,
                               pneworder, preforder, pccpayid, pparams);
  if (! tmpErrorMsg.isEmpty())
    _errorMsg = tmpErrorMsg;

  return returnValue;
}

int CyberSourceProcessor::doVoidPrevious(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, QString &papproval, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("CS:doVoidPrevious(%d, %d, %f, %d, %s, %s, %s, %d)",
           pccardid, pcvv, pamount, pcurrid, qPrintable(pneworder), qPrintable(preforder),
           qPrintable(papproval), pccpayid);

  QString tmpErrorMsg = _errorMsg;
  int returnValue = buildCommon(pccardid, pcvv, pamount, pcurrid, pneworder, preforder, Void);
  if (returnValue != 0)
    return returnValue;

  QDomElement voidsvc = _doc->createElement("ns1:VoidService");
  voidsvc.setAttribute("run", "true");
  _requestMessage.appendChild(voidsvc);

  createChildTextNode(voidsvc, "ns1:voidRequestID",     preforder);
  if (pneworder.length() > 0)
    createChildTextNode(voidsvc, "ns1:orderRequestToken", pneworder);

  if (DEBUG) qDebug("CS::doVoidPrevious sending %s", qPrintable(_doc->toString()));

  QString response;
  returnValue = sendViaHTTP(_doc->toString(), response);
  _errorMsg = tmpErrorMsg;
  if (returnValue < 0)
    return returnValue;

  double amount = pamount;
  returnValue = handleResponse(response, pccardid, Void, amount, pcurrid,
                               pneworder, preforder, pccpayid, pparams);
  if (! tmpErrorMsg.isEmpty())
    _errorMsg = tmpErrorMsg;

  return returnValue;
}

int CyberSourceProcessor::handleResponse(const QString &presponse, const int pccardid, const CCTransaction ptype, double &pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("CS::handleResponse(%s, %d, %d, %f, %d, %s, %d, pparams)",
           qPrintable(presponse), pccardid, ptype, pamount, pcurrid,
           qPrintable(preforder), pccpayid);

  QDomDocument response;
  response.setContent(presponse);

  QDomElement root = response.documentElement();

  QString r_approved;
  QString r_avs;
  QString r_code;
  QString r_cvv;
  QString r_error;
  QString r_message;
  QString r_ordernum;
  QString r_ref;
  QString r_shipping;
  QString r_tax;
  QString r_tdate;

  QString status;
  int     reasonCode = -1;
  QStringList missing;
  QStringList invalid;

  int returnValue = -95;

  QString requestedAmtStr;
  QString approvedAmtStr;

  for (QDomNode node = root.firstChild(); ! node.isNull(); node = node.nextSibling())
  {
    if (node.isElement())
    {
      QDomElement elem       = node.toElement();
      QDomElement reasonElem = elem.elementsByTagNameNS(NS1_NS,
                                                        "reasonCode").at(0).toElement();
      if (! reasonElem.isNull())
        reasonCode = reasonElem.text().toInt();

      if (node.nodeName() == "ccAuthReply")
      {
        for (QDomNode child = node.firstChild();
             ! child.isNull(); 
             child = child.nextSibling())
        {
          if (child.isElement())
          {
            QDomElement authchild = child.toElement();

            if (authchild.tagName() == "approvedAmount")
              approvedAmtStr = authchild.text();
            else if (authchild.tagName() == "authorizationCode")
              r_code = authchild.text();
            else if (authchild.tagName() == "authorizedDateTime")
              r_tdate = authchild.text();
            else if (authchild.tagName() == "avsCode")
              r_avs = authchild.text();
            else if (DEBUG && authchild.tagName() == "avsCodeRaw")
              qDebug("CS::handleResponse() got avsCodeRaw %s",
                     qPrintable(authchild.text()));
            else if (authchild.tagName() == "cvCode")
              r_cvv = authchild.text();
            else if (DEBUG && authchild.tagName() == "cvCodeRaw")
              qDebug("CS::handleResponse() got cvCodeRaw %s",
                     qPrintable(authchild.text()));
            else if (authchild.tagName() == "requestAmount")
              requestedAmtStr = authchild.text();
            else if (DEBUG)
              qDebug("CS::handleResponse() not handling %s/%s = %s",
                     qPrintable(node.nodeName()),
                     qPrintable(authchild.tagName()),
                     qPrintable(authchild.text()));
          }
        }
      }

      else if (node.nodeName() == "ccAuthReversalReply")
      {
        for (QDomNode child = node.firstChild();
             ! child.isNull();
             child = child.nextSibling())
        {
          if (child.isElement())
          {
            QDomElement revchild = child.toElement();
            if (revchild.tagName() == "amount")
              approvedAmtStr = revchild.text();
            else if (revchild.tagName() == "authorizationCode")
              r_code = revchild.text();
            else if (DEBUG)
              qDebug("CS::handleResponse() not handling %s/%s = %s",
                     qPrintable(node.nodeName()),
                     qPrintable(revchild.tagName()),
                     qPrintable(revchild.text()));
          }
        }
      }

      else if (node.nodeName() == "ccCaptureReply")
      {
        for (QDomNode child = node.firstChild();
             ! child.isNull();
             child = child.nextSibling())
        {
          if (child.isElement())
          {
            QDomElement capturechild = child.toElement();
            if (capturechild.tagName() == "amount")
              approvedAmtStr = capturechild.text();
            else if (DEBUG)
              qDebug("CS::handleResponse() not handling %s/%s = %s",
                     qPrintable(node.nodeName()),
                     qPrintable(capturechild.tagName()),
                     qPrintable(capturechild.text()));
          }
        }
      }

      else if (node.nodeName() == "ccCreditReply")
      {
        for (QDomNode child = node.firstChild();
             ! child.isNull();
             child = child.nextSibling())
        {
          if (child.isElement())
          {
            QDomElement capturechild = child.toElement();
            if (capturechild.tagName() == "amount")
              approvedAmtStr = capturechild.text();
            else if (DEBUG)
              qDebug("CS::handleResponse() not handling %s/%s = %s",
                     qPrintable(node.nodeName()),
                     qPrintable(capturechild.tagName()),
                     qPrintable(capturechild.text()));
          }
        }
      }

      else if (node.nodeName() == "decision")
      {
        if (node.isElement())
        {
          r_approved = node.toElement().text();
        }
      }

      else if (node.nodeName() == "invalidField")
        invalid += node.toElement().text();

      else if (node.nodeName() == "missingField")
        missing += node.toElement().text();

      else if (node.nodeName() == "reasonCode")
        reasonCode = node.toElement().text().toInt();

      else if (node.nodeName() == "requestToken")
      {
        preforder = node.toElement().text();
        r_ordernum = node.toElement().text();      // transaction id
      }

      else if (node.nodeName() == "voidReply")
      {
        for (QDomNode child = node.firstChild();
             ! child.isNull();
             child = child.nextSibling())
        {
          if (child.isElement())
          {
            QDomElement voidchild = child.toElement();
            if (voidchild.tagName() == "amount")
              approvedAmtStr = voidchild.text();
            else if (DEBUG)
              qDebug("CS::handleResponse() not handling %s/%s = %s",
                     qPrintable(node.nodeName()),
                     qPrintable(voidchild.tagName()),
                     qPrintable(voidchild.text()));
          }
        }
      }

      else if (node.nodeName() == "soap:Body")
      {
        if (node.firstChild().toElement().tagName() == "soap:Fault")
        {
          _errorMsg = errorMsg(-306).arg(node.toElement().text());
          return -306;
        }
      }

      else if (DEBUG)
        qDebug("CS::handleResponse() not handling %s = %s",
               qPrintable(node.nodeName()),
               qPrintable(node.toElement().text()));

    }
  }

  switch (reasonCode)
  {
    case 100: _errorMsg = r_message = errorMsg(0).arg(r_approved);
              returnValue = 0;
              break;
    case 101: _errorMsg = r_message = errorMsg(-301).arg(missing.join(tr(", ")));
              returnValue = -301;
              break;
    case 102: _errorMsg = r_message = errorMsg(-302).arg(invalid.join(tr(", ")));
              returnValue = -302;
              break;
    case 110:   // partial approval
            {
              pamount = approvedAmtStr.toDouble();

              ParameterList reverseParams;
              int reverseResult = doReverseAuthorize(pccardid, pamount,
                                                     pcurrid, pneworder,
                                                     preforder, pccpayid,
                                                     reverseParams);
              if (reverseResult)
                returnValue = reverseResult;
              else if (pamount == 0)
              {
                _errorMsg = r_message = errorMsg(-310).arg(r_approved);
                returnValue = -310;
              }
              else
              {
                _errorMsg = r_message = errorMsg(310).arg(r_approved);
                returnValue = 310;
              }
            }
    case 150: _errorMsg = r_message = errorMsg(-304);
              break;
    default: _errorMsg = r_message = errorMsg(-303).arg(reasonCode);
             returnValue = -303;
             break;
  }

  if (r_approved == "ACCEPT" || r_approved == "REVIEW")
  {
    _errorMsg = errorMsg(0).arg(r_code);
    if (ptype == Authorize)
      status = "A";
    else if (ptype == Void)
      status = "V";
    else
      status = "C";
  }
  else if (r_approved == "REJECT")
  {
    _errorMsg = errorMsg(-90).arg(r_message);
    returnValue = -90;
    status = "D";
  }
  else if (r_approved == "ERROR")
  {
    r_error   = r_message;
    _errorMsg = errorMsg(-12).arg(r_message);
    returnValue = -12;
    status = "X";
  }

  // TODO: move this up to CreditCardProcessor::fraudChecks()
  // TODO: change avs and cvv failure check configuration
  CreditCardProcessor::FraudCheckResult *avsresult = avsCodeLookup(r_avs.at(0));
  if (avsresult)
  {
    bool addrMustMatch   = _metrics->value("CCAvsAddr").contains("N");
    bool addrMustBeAvail = _metrics->value("CCAvsAddr").contains("X");
    bool zipMustMatch    = _metrics->value("CCAvsZip").contains("N");
    bool zipMustBeAvail  = _metrics->value("CCAvsZip").contains("X");

    if (avsresult->sev & Name & NoMatch)
      _passedAvs = false;
    else if ((avsresult->sev & Address & NoMatch) && addrMustMatch)
      _passedAvs = false;
    else if ((avsresult->sev & Address & NotAvail) && addrMustBeAvail)
      _passedAvs = false;
    else if ((avsresult->sev & PostalCode & NoMatch) && zipMustMatch)
      _passedAvs = false;
    else if ((avsresult->sev & PostalCode & NotAvail) && zipMustBeAvail)
      _passedAvs = false;
    else if (avsresult->sev != Match && _metrics->value("CCAvsCheck") == "F")
      _passedAvs = false;
    else if (avsresult->sev != Match && _metrics->value("CCAvsCheck") == "W")
    {
      _errorMsg = avsresult->text;
      returnValue = 97;
    }
    else
      _passedAvs = true;
  }
  else
  {
    _errorMsg = TR("AVS did not return a match and there is no description "
                   "for this failure (%1)").arg(r_avs);
    returnValue = 97;
  }

  CreditCardProcessor::FraudCheckResult *cvvresult = cvvCodeLookup(r_cvv.at(0));
  if (cvvresult)
  {
    bool cvvMustMatch         = _metrics->value("CCCVVErrors").contains("N");
    bool cvvMustBeProcessed   = _metrics->value("CCCVVErrors").contains("P");
    bool cvvMustBeOnCard      = _metrics->value("CCCVVErrors").contains("S");
    bool cvvIssuerMustBeValid = _metrics->value("CCCVVErrors").contains("U");

    if ((cvvresult->sev & NoMatch) && cvvMustMatch)
      _passedCvv = false;
    else if ((cvvresult->sev & NotProcessed) && cvvMustBeProcessed)
      _passedCvv = false;
    else if ((cvvresult->sev & NotAvail) && cvvMustBeOnCard)
      _passedCvv = false;
    else if ((cvvresult->sev & IssuerNotCertified) && cvvIssuerMustBeValid)
      _passedCvv = false;
    else if (cvvresult->sev != Match && _metrics->value("CCCVVCheck") == "F")
      _passedCvv = false;
    else if (cvvresult->sev != Match && _metrics->value("CCCVVCheck") == "W")
    {
      _errorMsg = cvvresult->text;
      returnValue = 96;
    }
    else
      _passedCvv = true;
  }
  else
  {
    _errorMsg = TR("CVV did not return a match and there is no "
                   "description for this failure (%1)").arg(r_cvv);
    returnValue = 96;
  }

  if (DEBUG)
    qDebug("CS:%s _passedAvs %d\t%s _passedCvv %d",
            qPrintable(r_avs), _passedAvs, qPrintable(r_cvv), _passedCvv);
  // end TODO

  if (r_approved.isEmpty() && ! r_message.isEmpty())
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

  pparams.append("ccard_id",    pccardid);
  pparams.append("currid",      pcurrid);
  pparams.append("auth_charge", typeToCode(ptype));
  pparams.append("type",        typeToCode(ptype));
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
  pparams.append("tdate",       r_tdate);

  pparams.append("auth", QVariant(ptype == Authorize));

  if (DEBUG)
    qDebug("CS:r_error.isEmpty() = %d", r_error.isEmpty());

  if (returnValue == 0)
    pparams.append("amount", pamount);
  else
    pparams.append("amount", 0);      // no money changed hands this attempt

  if (DEBUG)
    qDebug("CS::handleResponse returning %d %s", returnValue, qPrintable(errorMsg()));
  return returnValue;
}

int CyberSourceProcessor::doTestConfiguration()
{
  if (DEBUG)
    qDebug("CS:doTestConfiguration() entered");

  int returnValue = 0;

  if (_metricsenc->value("CCLogin").isEmpty())
  {
    _errorMsg = errorMsg(-300);
    returnValue = -300;
  }
  else if (_metricsenc->value("CCLogin").size() > 30)
  {
    _errorMsg = errorMsg(-305).arg(_metricsenc->value("CCLogin"));
    returnValue = -305;
  }

  if (DEBUG)
    qDebug("CS:doTestConfiguration() returning %d", returnValue);
  return returnValue;
}

bool CyberSourceProcessor::handlesCreditCards()
{
  return true;
}
