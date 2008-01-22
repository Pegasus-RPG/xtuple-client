/*
 * Common Public Attribution License Version 1.0.
 *
 * The contents of this file are subject to the Common Public Attribution
 * License Version 1.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the License
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla
 * Public License Version 1.1 but Sections 14 and 15 have been added to
 * cover use of software over a computer network and provide for limited
 * attribution for the Original Developer. In addition, Exhibit A has
 * been modified to be consistent with Exhibit B.
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite.
 *
 * The Original Developer is not the Initial Developer and is __________.
 * If left blank, the Original Developer is the Initial Developer.
 * The Initial Developer of the Original Code is OpenMFG, LLC,
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved.
 *
 * Contributor(s): ______________________.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the xTuple End-User License Agreeement (the xTuple License), in which
 * case the provisions of the xTuple License are applicable instead of
 * those above.  If you wish to allow use of your version of this file only
 * under the terms of the xTuple License and not to allow others to use
 * your version of this file under the CPAL, indicate your decision by
 * deleting the provisions above and replace them with the notice and other
 * provisions required by the xTuple License. If you do not delete the
 * provisions above, a recipient may use your version of this file under
 * either the CPAL or the xTuple License.
 *
 * EXHIBIT B.  Attribution Information
 *
 * Attribution Copyright Notice:
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 *
 * Attribution Phrase:
 * Powered by PostBooks, an open source solution from xTuple
 *
 * Attribution URL: www.xtuple.org
 * (to be included in the "Community" menu of the application if possible)
 *
 * Graphic Image as provided in the Covered Code, if any.
 * (online at www.xtuple.com/poweredby)
 *
 * Display of Attribution Information is required in Larger Works which
 * are defined in the CPAL as a work which combines Covered Code or
 * portions thereof with code not governed by the terms of the CPAL.
 */

 /* TODO: add more elements:
	  transactiondetails
	    taxexempt		required for LEVEL 2

	  payment
	    tax		required for LEVEL 2; set to calculated tax value or 0
 */


#include <QFileInfo>
#include <QSqlError>

#include <currcluster.h>

#include "OpenMFGGUIClient.h"
#include "yourpayprocessor.h"

#define DEBUG true

// convenience macro to add <ChildName>Content</ChildName> to the Parent node
#define CREATECHILDTEXTNODE(Parent, ChildName, Content) \
	{ \
	  QDomElement Child = Parent.ownerDocument().createElement(ChildName); \
	  Child.appendChild(Parent.ownerDocument().createTextNode(Content)); \
	  Parent.appendChild(Child); \
	}

YourPayProcessor::YourPayProcessor() : CreditCardProcessor()
{
  XSqlQuery currq;
  currq.exec("SELECT * FROM curr_symbol WHERE (curr_abbr='USD');");
  if (currq.first())
  {
    _currencyId     = currq.value("curr_id").toInt();
    _currencyName   = currq.value("curr_name").toString();
    _currencySymbol = currq.value("curr_symbol").toString();
  }
  else if (currq.lastError().type() != QSqlError::None)
    _errorMsg = currq.lastError().databaseText();
  else if (currq.exec("SELECT * "
		      "FROM curr_symbol "
		      "WHERE ((curr_abbr ~* 'US')"
		      "  AND  (curr_symbol ~ '\\$')"
		      "  AND  (curr_name ~* 'dollar'));") &&
	   currq.first())
  {
    _currencyId     = currq.value("curr_id").toInt();
    _currencyName   = currq.value("curr_name").toString();
    _currencySymbol = currq.value("curr_symbol").toString();
  }
  else if (currq.lastError().type() != QSqlError::None)
    _errorMsg = currq.lastError().databaseText();

  if (_currencyId <= 0)
  {
    _errorMsg = tr("Could not find US $ in the curr_symbol table; "
		    "defaulting to base currency.") + "\n\n" + _errorMsg;
    _currencyId     = CurrDisplay::baseId();
    _currencyName   = CurrDisplay::baseCurrAbbr();
    _currencySymbol = CurrDisplay::baseCurrAbbr();
  }

  _defaultLivePort   = 1129;
  _defaultLiveServer = "secure.linkpt.net";
  _defaultTestPort   = 1129;
  _defaultTestServer = "staging.linkpt.net";

  _msgHash.insert(-100, tr("No Approval Code\n%1\n%2\n%3"));
  _msgHash.insert(-103, tr("The YourPay Store Number is not set."));
  _msgHash.insert(-104, tr("The digital certificate (.pem file) is not set."));
  _msgHash.insert(-105, tr("Could not open digital certificate (.pem file) %1."));
  _msgHash.insert(-106, tr("Transaction failed LinkShield check. Either the "
			   "score was greater than the maximum configured "
			   "allowed score or the score was empty (this may "
			   "happen if the application is configured to "
			   "check the LinkShield score but you have not "
			   "subscribed to this service)."));
}

int YourPayProcessor::buildCommonXML(int pccardid, int pcvv, QString pamount, QDomDocument &prequest, QString pordertype)
{
  QDomElement order = prequest.createElement("order");
  prequest.appendChild(order);

  XSqlQuery ypq;
  ypq.prepare(
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
    "  formatbytea(decrypt(setbytea(ccard_year_expired),setbytea(:key), 'bf')) AS ccard_year_expired"
    "  FROM ccard "
    "WHERE (ccard_id=:ccardid);");
  ypq.bindValue(":ccardid", pccardid);
  ypq.bindValue(":key",     omfgThis->_key);
  ypq.exec();

  if (ypq.first())
  {
    if (!ypq.value("ccard_active").toBool())
    {
      _errorMsg = errorMsg(-10);
      return -10;
    }
  }
  else if (ypq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = ypq.lastError().databaseText();
    return -1;
  }
  else
  {
    _errorMsg = errorMsg(-17).arg(pccardid);
    return -17;
  }

  QDomElement elem;

  elem = prequest.createElement("merchantinfo");
  order.appendChild(elem);
  CREATECHILDTEXTNODE(elem, "configfile", _storenum.toLatin1());

  elem = prequest.createElement("orderoptions");
  order.appendChild(elem);
  CREATECHILDTEXTNODE(elem, "ordertype", pordertype);
  QString resultText;
  if (isLive())
    resultText = "LIVE";
  else if (isTest())
  {
    if (_metrics->value("CCTestResult") == "G")
      resultText = "GOOD";
    else if (_metrics->value("CCTestResult") == "F")
      resultText = (qrand() % 2) ? "DECLINE" : "DUPLICATE";
    else if (_metrics->value("CCTestResult") == "S")
      switch (qrand() % 10)
      {
	case 0: resultText = "DECLINE";
		break;
	case 1: resultText = "DUPLICATE";
		break;
	default:
		resultText = "GOOD";
		break;
      }
  }
  else
  {
    _errorMsg = errorMsg(-13).arg(pccardid);
    return -13;
  }
  CREATECHILDTEXTNODE(elem, "result", resultText);

  elem = prequest.createElement("payment");
  order.appendChild(elem);
  CREATECHILDTEXTNODE(elem, "chargetotal", pamount);

  elem = prequest.createElement("creditcard");
  order.appendChild(elem);
  CREATECHILDTEXTNODE(elem, "cardnumber", ypq.value("ccard_number").toString());

  QString work_month;
  work_month.setNum(ypq.value("ccard_month_expired").toDouble());
  if (work_month.length() == 1)
    work_month = "0" + work_month;
  CREATECHILDTEXTNODE(elem,  "cardexpmonth", work_month);
  CREATECHILDTEXTNODE(elem,  "cardexpyear",
		      ypq.value("ccard_year_expired").toString().right(2));
  if (pcvv > 0)
  {
    CREATECHILDTEXTNODE(elem, "cvmvalue", QString::number(pcvv));
    CREATECHILDTEXTNODE(elem, "cvmindicator", "provided");
  }

  elem = prequest.createElement("billing");
  order.appendChild(elem);

  CREATECHILDTEXTNODE(elem, "name", ypq.value("ccard_name").toString());
  // TODO: company
  CREATECHILDTEXTNODE(elem, "address1", ypq.value("ccard_address1").toString());
  CREATECHILDTEXTNODE(elem, "address2", ypq.value("ccard_address2").toString());
  CREATECHILDTEXTNODE(elem, "city",     ypq.value("ccard_city").toString());
  CREATECHILDTEXTNODE(elem, "state",    ypq.value("ccard_state").toString());
  CREATECHILDTEXTNODE(elem, "zip",      ypq.value("ccard_zip").toString());
    // TODO: country // should be 2-char country code abbr
    // TODO: phone, fax, email
  CREATECHILDTEXTNODE(elem, "addrnum",
		    ypq.value("ccard_address1").toString().section(" ", 0, 0));

  if (DEBUG)
    qDebug("YP:buildCommonXML built %s", prequest.toString().toAscii().data());
  return 0;
}

int  YourPayProcessor::doAuthorize(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString& pneworder, QString& preforder, int &pccpayid)
{
  if (DEBUG)
    qDebug("YP:doAuthorize(%d, %d, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid);
  XSqlQuery ypq;
  ypq.prepare("SELECT ROUND(currToCurr(:curr_id, :yp_curr_id,"
	      "                :amount, CURRENT_DATE), 2) AS ccpay_amount_yp;");
  ypq.bindValue(":curr_id",    pcurrid);
  ypq.bindValue(":yp_curr_id", _currencyId);
  ypq.bindValue(":amount",     pamount);
  ypq.exec();

  if (ypq.first())
  {
    ; // we'll use the result later
  }
  else if (ypq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = ypq.lastError().databaseText();
    return -1;
  }
  else
  {
    _errorMsg = errorMsg(-17).arg(pccardid);
    return -17;
  }

  QDomDocument request;

  int returnValue = buildCommonXML(pccardid, pcvv,
				   ypq.value("ccpay_amount_yp").toString(),
				   request, "PREAUTH");
  if (returnValue !=  0)
    return returnValue;

  QDomElement elem;

  elem = request.createElement("transactiondetails");
  request.documentElement().appendChild(elem);

  // in case we're reusing an order number
  QString oidstr = pneworder;
  ypq.prepare("SELECT MAX(COALESCE(ccpay_order_number_seq, -1)) + 1"
	      "       AS next_seq "
	      "  FROM ccpay "
	      " WHERE (ccpay_order_number=:ccpay_order_number);");
  ypq.bindValue(":ccpay_order_number", pneworder.toInt());
  ypq.exec();
  if (ypq.first() && ! ypq.value("next_seq").isNull())
    oidstr = oidstr + "-" + ypq.value("next_seq").toString();
  else if (ypq.lastError().type() != QSqlError::None)
  {
    _errorMsg = errorMsg(-24);
    return -24;
  }
  CREATECHILDTEXTNODE(elem, "oid",          oidstr);
  CREATECHILDTEXTNODE(elem, "terminaltype", "UNSPECIFIED");

  if (! preforder.isEmpty())
    CREATECHILDTEXTNODE(elem, "ponumber", preforder);

  /* TODO: salesOrder.cpp set the shipping elem. YP only seems to use it for
	   CALCSHIPPING transactions; we don't do these yet so don't bother
  elem = request.createElement("shipping");
  request.documentElement().appendChild(elem);
  CREATECHILDTEXTNODE(elem, "address1", _shipto_address1);
  CREATECHILDTEXTNODE(elem, "address2", _shipto_address2);
  CREATECHILDTEXTNODE(elem, "city",     _shipto_city);
  CREATECHILDTEXTNODE(elem, "name",     _shipto_name);
  CREATECHILDTEXTNODE(elem, "state",    _shipto_state);
  CREATECHILDTEXTNODE(elem, "zip",      _shipto_zip);
  */

  QDomDocument response;

  returnValue = processXML(request, response);
  if (returnValue < 0)
    return returnValue;

  returnValue = handleResponse(response, pccardid, "A", pamount, pcurrid,
			       pneworder, preforder, pccpayid);

  return returnValue;
}

int  YourPayProcessor::doCharge(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString& pneworder, QString& preforder, int &pccpayid)
{
  if (DEBUG)
    qDebug("YP:doCharge(%d, %d, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid);
  XSqlQuery ypq;
  ypq.prepare("SELECT ROUND(currToCurr(:curr_id, :yp_curr_id,"
	      "                :amount, CURRENT_DATE), 2) AS ccpay_amount_yp;");
  ypq.bindValue(":curr_id",    pcurrid);
  ypq.bindValue(":yp_curr_id", _currencyId);
  ypq.bindValue(":amount",     pamount);
  ypq.exec();

  if (ypq.first())
  {
    ; // we'll use the result later
  }
  else if (ypq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = ypq.lastError().databaseText();
    return -1;
  }
  else
  {
    _errorMsg = errorMsg(-17).arg(pccardid);
    return -17;
  }

  QDomDocument request;

  int returnValue = buildCommonXML(pccardid, pcvv,
				   ypq.value("ccpay_amount_yp").toString(),
				   request, "SALE");
  if (returnValue !=  0)
    return returnValue;

  QDomElement elem;

  elem = request.createElement("transactiondetails");
  request.documentElement().appendChild(elem);

  // in case we're reusing an order number
  QString oidstr = pneworder;
  ypq.prepare("SELECT MAX(COALESCE(ccpay_order_number_seq, -1)) + 1"
	      "       AS next_seq "
	      "  FROM ccpay "
	      " WHERE (ccpay_order_number=:ccpay_order_number);");
  ypq.bindValue(":ccpay_order_number", pneworder.toInt());
  ypq.exec();
  if (ypq.first() && ! ypq.value("next_seq").isNull())
    oidstr = oidstr + "-" + ypq.value("next_seq").toString();
  else if (ypq.lastError().type() != QSqlError::None)
  {
    _errorMsg = errorMsg(-44);
    return -44;
  }
  CREATECHILDTEXTNODE(elem, "oid",          oidstr);
  CREATECHILDTEXTNODE(elem, "terminaltype", "UNSPECIFIED");

  if (! preforder.isEmpty())
    CREATECHILDTEXTNODE(elem, "ponumber", preforder);

  /* TODO: salesOrder.cpp had the following, but it doesn't appear to be useful
	   except for having YP calculate shipping charges. since we don't
	   use YP to calculate shipping and this is a SALE and not a
	   CALCSHIPPING transaction, we won't bother for now.
  elem = request.createElement("shipping");
  request.documentElement().appendChild(elem);
  CREATECHILDTEXTNODE(elem, "address1", _shipto_address1);
  CREATECHILDTEXTNODE(elem, "address2", _shipto_address2);
  CREATECHILDTEXTNODE(elem, "city",     _shipto_city);
  CREATECHILDTEXTNODE(elem, "name",     _shipto_name);
  CREATECHILDTEXTNODE(elem, "state",    _shipto_state);
  CREATECHILDTEXTNODE(elem, "zip",      _shipto_zip);
  */

  QDomDocument response;
  returnValue = processXML(request, response);
  if (returnValue < 0)
    return returnValue;

  returnValue = handleResponse(response, pccardid, "C", pamount, pcurrid,
			       pneworder, preforder, pccpayid);

  return returnValue;
}

int YourPayProcessor::doChargePreauthorized(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid)
{
  if (DEBUG)
    qDebug("YP:doChargePreauthorized(%d, %d, %f, %d, %s, %s, %d)",
	    pccardid, pcvv, pamount, pcurrid,
	    pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid);
  XSqlQuery ypq;
  ypq.prepare( "SELECT ROUND(currToCurr(:curr_id, :yp_curr_id,"
	       "              :amount, CURRENT_DATE), 2) AS ccpay_amount_yp "
	       "FROM ccard, ccpay "
	       "WHERE ((ccard_id=:ccardid)"
	       "  AND  (ccpay_id=:ccpayid));");
  ypq.bindValue(":curr_id",    pcurrid);
  ypq.bindValue(":yp_curr_id", _currencyId);
  ypq.bindValue(":amount",     pamount);
  ypq.bindValue(":key",        omfgThis->_key);
  ypq.bindValue(":ccardid",    pccardid);
  ypq.bindValue(":ccpayid",    pccpayid);
  ypq.exec();
  if (ypq.first())
  {
    ; // we'll use the result later
  }
  else if (ypq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = ypq.lastError().databaseText();
    return -1;
  }
  else
  {
    _errorMsg = errorMsg(-17).arg(pccardid);
    return -17;
  }

  QDomDocument request;

  int returnValue = buildCommonXML(pccardid, pcvv,
				   ypq.value("ccpay_amount_yp").toString(),
				   request, "POSTAUTH");
  if (returnValue !=  0)
    return returnValue;

  QDomElement elem = request.createElement("transactiondetails");
  request.documentElement().appendChild(elem);
  CREATECHILDTEXTNODE(elem, "oid",          preforder);
  CREATECHILDTEXTNODE(elem, "terminaltype", "UNSPECIFIED");

  QDomDocument response;
  returnValue = processXML(request, response);
  if (returnValue < 0)
    return returnValue;

  returnValue = handleResponse(response, pccardid, "CP", pamount, pcurrid,
			       pneworder, preforder, pccpayid);

  return returnValue;
}

int YourPayProcessor::doCredit(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid)
{
  if (DEBUG)
    qDebug("YP:doCredit(%d, %d, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid);
  XSqlQuery ypq;
  ypq.prepare( "SELECT ROUND(currToCurr(:curr_id, :yp_curr_id,"
	       "              :amount, CURRENT_DATE), 2) AS ccpay_amount_yp;");
  ypq.bindValue(":curr_id",    pcurrid);
  ypq.bindValue(":yp_curr_id", _currencyId);
  ypq.bindValue(":amount",     pamount);
  ypq.exec();
  if (ypq.first())
  {
    ; // we'll use the result later
  }
  else if (ypq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = ypq.lastError().databaseText();
    return -1;
  }
  else
  {
    _errorMsg = errorMsg(-17);
    return -17;
  }

  QDomDocument request;

  int returnValue = buildCommonXML(pccardid, pcvv,
				   ypq.value("ccpay_amount_yp").toString(),
				   request, "CREDIT");
  if (returnValue !=  0)
    return returnValue;

  QDomElement elem = request.createElement("transactiondetails");
  request.documentElement().appendChild(elem);
  CREATECHILDTEXTNODE(elem, "oid",              preforder);
  CREATECHILDTEXTNODE(elem, "reference_number", pneworder);
  CREATECHILDTEXTNODE(elem, "terminaltype",     "UNSPECIFIED");

  QDomDocument response;
  returnValue = processXML(request, response);
  if (returnValue < 0)
    return returnValue;

  returnValue = handleResponse(response, pccardid, "R", pamount, pcurrid,
			       pneworder, preforder, pccpayid);

  return returnValue;
}

int YourPayProcessor::doVoidPrevious(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid)
{
  if (DEBUG)
    qDebug("YP:doVoidPrevious(%d, %d, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(),
	   pccpayid);
  XSqlQuery ypq;
  ypq.prepare( "SELECT ROUND(currToCurr(:curr_id, :yp_curr_id,"
	       "              :amount, CURRENT_DATE), 2) AS ccpay_amount_yp;");
  ypq.bindValue(":curr_id",    pcurrid);
  ypq.bindValue(":yp_curr_id", _currencyId);
  ypq.bindValue(":amount",     pamount);
  ypq.exec();
  if (ypq.first())
  {
    ; // we'll use the result later
  }
  else if (ypq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = ypq.lastError().databaseText();
    return -1;
  }
  else
  {
    _errorMsg = errorMsg(-17);
    return -17;
  }

  QString tmpErrorMsg = _errorMsg;

  QDomDocument request;

  int returnValue = buildCommonXML(pccardid, pcvv,
				   ypq.value("ccpay_amount_yp").toString(),
				   request, "VOID");
  if (returnValue != 0)
    return returnValue;

  QDomElement elem = request.createElement("transactiondetails");
  request.documentElement().appendChild(elem);
  CREATECHILDTEXTNODE(elem, "oid",              pneworder);
  CREATECHILDTEXTNODE(elem, "terminaltype",     "UNSPECIFIED");

  QDomDocument response;
  returnValue = processXML(request, response);
  _errorMsg = tmpErrorMsg;
  if (returnValue < 0)
    return returnValue;

  returnValue = handleResponse(response, pccardid, "V", pamount, pcurrid,
			       pneworder, preforder, pccpayid);
  _errorMsg = tmpErrorMsg;
  return returnValue;
}

int YourPayProcessor::handleResponse(const QDomDocument &response, const int pccardid, const QString &ptype, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid)
{
  if (DEBUG)
    qDebug("YP::handleResponse(%s, %d, %s, %f, %d, %s, %d)",
	   response.toString().toAscii().data(), pccardid,
	   ptype.toAscii().data(), pamount, pcurrid,
	   preforder.toAscii().data(), pccpayid);

  QDomNode node;
  QDomElement root = response.documentElement();

  QString r_approved;
  QString r_avs;
  QString r_code;
  QString r_error;
  QString r_message;
  QString r_ordernum;
  QString r_ref;
  QString r_score;
  QString r_shipping;
  QString r_tax;
  QString r_tdate;
  QString r_time;

  QString status;

  node = root.firstChild();
  while ( !node.isNull() )
  {
    if (node.isElement())
    {
      if (node.nodeName() == "r_approved" )
	r_approved = node.toElement().text();

      else if (node.nodeName() == "r_avs" )
	r_avs = node.toElement().text();

      else if (node.nodeName() == "r_code" )
	r_code = node.toElement().text();

      else if (node.nodeName() == "r_error" )
	r_error = node.toElement().text();

      else if (node.nodeName() == "r_message" )
	r_message = node.toElement().text();

      else if (node.nodeName() == "r_ordernum" )
	r_ordernum = node.toElement().text();

      else if (node.nodeName() == "r_ref" )
	r_ref = node.toElement().text();

      else if (node.nodeName() == "r_score" )
	r_score = node.toElement().text();

      else if (node.nodeName() == "r_shipping" )
	r_shipping = node.toElement().text();

      else if (node.nodeName() == "r_tax" )
	r_tax = node.toElement().text();

      else if (node.nodeName() == "r_tdate" )
	r_tdate = node.toElement().text();

      else if (node.nodeName() == "r_time" )
	r_time = node.toElement().text();
    }

    node = node.nextSibling();
  }

  /* YP in test mode is inconsistent in the responses it sends.
     Sometimes it sends a complete message and sometimes it doesn't,
     so try to make the app behave in TEST mode like it will in LIVE mode
     (but we have to guess how YP will actually behave in LIVE mode!-{ )
    */
  if (isTest() && r_approved == "APPROVED")
  {
    if (r_code.isEmpty())
      r_code = "00000";
  }

  int returnValue = 0;
  if (r_approved == "APPROVED")
  {
    _errorMsg = errorMsg(0).arg(r_ref);
    if (ptype == "A")
      status = "A";	// Authorized
    else if (ptype == "V")
      status = "V";	// Voided
    else
      status = "C";	// Completed/Charged
  }

  else if (r_approved == "DENIED")
  {
    _errorMsg = errorMsg(10).arg(r_message);
    returnValue = 10;
    status = "D";
  }

  else if (r_approved == "DUPLICATE")
  {
    _errorMsg = errorMsg(11).arg(r_message);
    returnValue = 11;
    status = "D";
  }

  else if (r_approved == "DECLINED")
  {
    _errorMsg = errorMsg(12).arg(r_error);
    returnValue = 12;
    status = "D";
  }

  else if (r_approved == "FRAUD")
  {
    _errorMsg = errorMsg(13).arg(r_error);
    returnValue = 13;
    status = "D";
  }

  else if (r_approved.isEmpty())
  {
    _errorMsg = errorMsg(-100)
		     .arg(r_error).arg(r_message).arg(response.toString());
    returnValue = -100;
    status = "X";
  }

  // YP encodes AVS and CVV checking in the r_avs response field
  _passedAvs = r_avs.contains(QRegExp("^[XY][XY]"));
  if (_passedAvs)
  {
    XSqlQuery ypq;
    ypq.prepare("SELECT ccard_type FROM ccard WHERE ccard_id=:ccardid;");
    ypq.bindValue(":ccardid", pccardid);
    ypq.exec();
    if (ypq.first())
    {
      QString cardtype = ypq.value("ccard_type").toString();
      QString cardspecificpart = r_avs.mid(2, 1);
      if (cardtype == "V")
	_passedAvs = cardspecificpart.contains(QRegExp("[YZANURSEG]"));
      else if (cardtype == "M")
	_passedAvs = cardspecificpart.contains(QRegExp("[YZANXWURS]"));
      else if (cardtype == "D")
	_passedAvs = cardspecificpart.contains(QRegExp("[AXYNWU]"));
      else if (cardtype == "A")
	_passedAvs = cardspecificpart.contains(QRegExp("[YZANURS]"));
    }
  }

  _passedCvv = r_avs.contains(QRegExp("[MPSUXZ ]$"));

  _passedLinkShield = (! _metrics->boolean("CCYPLinkShield")) ||
	      (! r_score.isEmpty() &&
	       r_score.toInt() <= _metrics->value("CCYPLinkShieldMax").toInt());

  if (DEBUG)
    qDebug("YP:%s\t_passedAvs %d\t_passedCvv %d\t_passedLinkShield %d",
	    r_avs.toAscii().data(), _passedAvs, _passedCvv, _passedLinkShield);

  /* From here on, treat errors as warnings:
     do as much as possible,
     set _errorMsg if there are problems, and
     return a warning unless YourPay already gave an error.
  */

  /* TODO: try implementing a second cc processor to see if from here on can be
	   moved to CreditCardProcessor
   */
  XSqlQuery ypq;
  int next_seq = 0;

  if (pccpayid > 0)
  {
    ypq.prepare( "UPDATE ccpay"
		 "   SET ccpay_amount = ROUND(currToCurr(:curr_id, :yp_curr_id,"
		 "                                  :amount, CURRENT_DATE), 2),"
		 "       ccpay_auth=:auth,"
		 "       ccpay_status=:status,"
		 "       ccpay_curr_id=:yp_curr_id,"
		 "       ccpay_yp_r_avs=:avs,"
		 "       ccpay_yp_r_ordernum=:ordernum,"
		 "       ccpay_yp_r_error=:error,"
		 "       ccpay_yp_r_approved=:approved,"
		 "       ccpay_yp_r_code=:code,"
		 "       ccpay_yp_r_score=:score,"
		 "       ccpay_yp_r_shipping=:shipping,"
		 "       ccpay_yp_r_tax=:tax,"
		 "       ccpay_yp_r_tdate=:tdate,"
		 "       ccpay_yp_r_ref=:ref,"
		 "       ccpay_yp_r_message=:message,"
		 "       ccpay_yp_r_time=:time"
		 " WHERE (ccpay_id=:ccpay_id);" );
  }
  else
  {
    ypq.exec("SELECT NEXTVAL('ccpay_ccpay_id_seq') AS ccpay_id;");
    if (ypq.first())
      pccpayid = ypq.value("ccpay_id").toInt();
    else if (ypq.lastError().type() != QSqlError::None && r_error.isEmpty())
    {
      _errorMsg = ypq.lastError().databaseText();
      return 1;
    }
    else if (ypq.lastError().type() == QSqlError::None && r_error.isEmpty())
    {
      _errorMsg = errorMsg(2);
      return 2;
    }
    else	// no rows found and YP reported an error
    {
      _errorMsg = errorMsg(-12).arg(r_error);
      return -12;
    }

    ypq.prepare("SELECT MAX(COALESCE(ccpay_order_number_seq, -1)) + 1"
		"       AS next_seq "
	        "  FROM ccpay "
	        " WHERE (ccpay_order_number=:ccpay_order_number);");
    ypq.bindValue(":ccpay_order_number", pneworder.toInt());
    ypq.exec();
    if (ypq.first())
      next_seq = ypq.value("next_seq").toInt();
    // ignore errors because there's only a tiny chance that a future
    // transaction might have trouble if we don't have a sequence number now

    ypq.prepare("INSERT INTO ccpay ("
	       "    ccpay_id, ccpay_ccard_id, ccpay_cust_id,"
	       "    ccpay_auth_charge, ccpay_auth,"
	       "    ccpay_amount,"
	       "    ccpay_curr_id, ccpay_type,"
	       "    ccpay_order_number, ccpay_order_number_seq,"
	       "    ccpay_status,"
	       "    ccpay_yp_r_avs, ccpay_yp_r_ordernum, ccpay_yp_r_error,"
	       "    ccpay_yp_r_approved, ccpay_yp_r_code, ccpay_yp_r_score,"
	       "    ccpay_yp_r_shipping, ccpay_yp_r_tax, ccpay_yp_r_tdate,"
	       "    ccpay_yp_r_ref, ccpay_yp_r_message, ccpay_yp_r_time"
	       ") SELECT :ccpay_id, ccard_id, cust_id,"
	       "    :auth_charge, :auth,"
	       "    ROUND(currToCurr(:curr_id,:yp_curr_id,:amount,CURRENT_DATE), 2),"
	       "    :yp_curr_id, :type,"
	       "    :reference, :next_seq,"
	       "    :status,"
	       "    :avs, :ordernum, :error,"
	       "    :approved, :code, :score,"
	       "    :shipping, :tax, :tdate,"
	       "    :ref, :message, :time"
	       "  FROM ccard, custinfo"
	       "  WHERE ((ccard_cust_id=cust_id)"
	       "    AND  (ccard_id=:ccard_id));");
  }

  ypq.bindValue(":ccpay_id",   pccpayid);
  ypq.bindValue(":curr_id",    pcurrid);
  ypq.bindValue(":yp_curr_id", _currencyId);

  ypq.bindValue(":auth_charge", ptype);

  if (ptype == "A")
    ypq.bindValue(":auth", QVariant(true, 0));
  else
    ypq.bindValue(":auth", QVariant(false, 1));

  if (returnValue == 0)
    ypq.bindValue(":amount",   pamount);
  else
    ypq.bindValue(":amount",   0);	// no money changed hands this attempt

  ypq.bindValue(":type",       ptype);
  ypq.bindValue(":reference",  (preforder.isEmpty()) ? pneworder : preforder);
  ypq.bindValue(":status",     status);
  ypq.bindValue(":avs",        r_avs);
  ypq.bindValue(":ordernum",   r_ordernum);
  ypq.bindValue(":next_seq",   next_seq);
  ypq.bindValue(":error",      r_error);
  ypq.bindValue(":approved",   r_approved);
  ypq.bindValue(":code",       r_code);
  ypq.bindValue(":score",      r_score.toInt());
  ypq.bindValue(":shipping",   r_shipping);
  ypq.bindValue(":tax",        r_tax);
  ypq.bindValue(":tdate",      r_tdate);
  ypq.bindValue(":ref",        r_ref);
  ypq.bindValue(":message",    r_message);
  if (! r_time.isEmpty())
    ypq.bindValue(":time",     r_time);
  ypq.bindValue(":ccard_id",   pccardid);

  ypq.exec();
  if (ypq.lastError().type() != QSqlError::None)
  {
    pccpayid = -1;
    if (returnValue == 0)
    {
      _errorMsg = ypq.lastError().databaseText();
      returnValue = 1;
    }
  }

  if (DEBUG)
    qDebug("YP:r_error.isEmpty() = %d", r_error.isEmpty());

  if (! r_error.isEmpty())
  {
    _errorMsg = errorMsg(-12).arg(r_error);
    return -12;
  }

  return returnValue;
}

int YourPayProcessor::doCheckConfiguration()
{
  if (DEBUG)
    qDebug("YP:doCheckConfiguration()");

  _storenum = _metricsenc->value("CCYPStoreNum");
  if (_storenum.isEmpty())
  {
    _errorMsg = errorMsg(-103);
    return -103;
  }

#ifdef Q_WS_WIN
  _pemfile = _metrics->value("CCYPWinPathPEM");
#elif defined Q_WS_MACX
  _pemfile = _metrics->value("CCYPMacPathPEM");
#elif defined Q_WS_X11
  _pemfile = _metrics->value("CCYPLinPathPEM");
#endif

  if (_pemfile.isEmpty())
  {
    _errorMsg = errorMsg(-104);
    return -104;
  }

  QFileInfo fileinfo(_pemfile.toLatin1());
  if (!fileinfo.isFile())
  {
    _errorMsg = errorMsg(-105).arg(fileinfo.fileName());
   return -105;
  }

  return 0;
}

void YourPayProcessor::reset()
{
  CreditCardProcessor::reset();
  _passedLinkShield = true;
}

int YourPayProcessor::fraudChecks()
{
  if (DEBUG)
    qDebug("YP:fraudChecks()");

  int returnValue = CreditCardProcessor::fraudChecks();
  if (returnValue < 0)
    return returnValue;

  else if (!_passedLinkShield)
  {
    _errorMsg = errorMsg(-106);
    returnValue = -106;
  }

  return returnValue;
}
