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

#include <QFileInfo>
#include <QSqlError>

#include <currcluster.h>

#include "OpenMFGGUIClient.h"
#include "yourpayprocessor.h"

#define DEBUG false

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

  _msgHash.insert(-100, tr("No Approval Code\n%1\n%2\n%3"));
  _msgHash.insert(-103, tr("The YourPay Store Number is not set."));
  _msgHash.insert(-104, tr("The digital certificate (.pem file) is not set."));
  _msgHash.insert(-105, tr("Could not open digital certificate (.pem file) %1."));
}

int YourPayProcessor::doCredit(const int pccardid, const double pamount, const int pcurrid, const QString &pneworder, const QString &porigorder, int &pccpayid)
{
  XSqlQuery ypq;
  ypq.prepare( "SELECT ROUND(currToCurr(:curr_id, :yp_curr_id,"
	       "              :amount, CURRENT_DATE), 2) AS ccpay_amount_yp,"
	       "       ccard_active, "
	       "       formatbytea(decrypt(setbytea(ccard_number),"
	       "               setbytea(:key), 'bf')) AS ccard_number,"
	       "       formatbytea(decrypt(setbytea(ccard_month_expired),"
	       "               setbytea(:key), 'bf')) AS ccard_month_expired,"
	       "       formatbytea(decrypt(setbytea(ccard_year_expired),"
	       "               setbytea(:key), 'bf')) AS ccard_year_expired,"
	       "       ccard_type "
	       "FROM ccard "
	       "WHERE (ccard_id=:ccardid);");
  ypq.bindValue(":curr_id",    pcurrid);
  ypq.bindValue(":yp_curr_id", _currencyId);
  ypq.bindValue(":amount",     pamount);
  ypq.bindValue(":key",        omfgThis->_key);
  ypq.bindValue(":ccardid",    pccardid);
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
    _errorMsg = errorMsg(-50);
    return -50;
  }

  QDomDocument request;
  QDomElement order = request.createElement("order");
  request.appendChild(order);

  QDomElement	elem;
  QDomElement	child;

  elem = request.createElement("merchantinfo");
  order.appendChild(elem);
  child = request.createElement("configfile");
  child.appendChild(request.createTextNode(_storenum.toLatin1()));
  elem.appendChild(child);

  elem = request.createElement("orderoptions");
  order.appendChild(elem);
  child = request.createElement("ordertype");
  child.appendChild(request.createTextNode("CREDIT"));
  elem.appendChild(child);
  if (isTest())
  {
    child = request.createElement("result");
    int randomnum = qrand();
    switch (randomnum % 10)
    {
      case 0: child.appendChild(request.createTextNode("DECLINE"));
	      break;
      case 1: child.appendChild(request.createTextNode("DUPLICATE"));
	      break;
      default:
	      child.appendChild(request.createTextNode("GOOD"));
	      break;
    }
    elem.appendChild(child);
  }

  elem = request.createElement("payment");
  order.appendChild(elem);
  child = request.createElement("chargetotal");
  child.appendChild(request.createTextNode(ypq.value("ccpay_amount_yp").toString()));
  elem.appendChild(child);

  elem = request.createElement("creditcard");
  order.appendChild(elem);
  child = request.createElement("cardnumber");
  child.appendChild(request.createTextNode(ypq.value("ccard_number").toString()));
  elem.appendChild(child);

  child = request.createElement("cardexpmonth");
  child.appendChild(request.createTextNode(
	  QString("%1").arg(ypq.value("ccard_month_expired").toDouble(), 2, 'f', 0))
  );
  elem.appendChild(child);

  child = request.createElement("cardexpyear");
  child.appendChild(request.createTextNode(
	    ypq.value("ccard_year_expired").toString().right(2))
  );
  elem.appendChild(child);

  elem = request.createElement("transactiondetails");
  order.appendChild(elem);
  child = request.createElement("oid");
  child.appendChild(request.createTextNode(porigorder));
  elem.appendChild(child);
  child = request.createElement("reference_number");
  child.appendChild(request.createTextNode(pneworder));
  elem.appendChild(child);

  /* TODO: billing info not needed for credits?
  <billing>
    <addrnum>numeric portion of street address</addrnum>
    <zip>billing zip</zip>
  */

  if (DEBUG) qDebug(request.toString());

  // TODO: there's got to be a better place to put this
  if (isTest())
    _metrics->set("CCOrder", request.toString());

  QDomDocument response;
  int returnValue = processXML(request, response);
  if (returnValue < 0)
    return returnValue;

  returnValue = handleResponse(response, pccardid, "R", pamount, pcurrid,
			       porigorder, pccpayid);
  if (returnValue < 0)
    return returnValue;

  return returnValue;
}

int YourPayProcessor::handleResponse(const QDomDocument &response, const int pccardid, const QString &ptype, const double pamount, const int pcurrid, const QString &porigorder, int &pccpayid)
{
  if (DEBUG) qDebug("YP::handleResponse(%s, %d, %s, %f, %d, %s, %d)",
	  response.toString().toAscii().data(), pccardid,
	  ptype.toAscii().data(), pamount, pcurrid,
	  porigorder.toAscii().data(), pccpayid);
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
  // TODO: simplify the XML handling?
  // TODO: do we need to check all of these fields?
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

  int returnValue = 0;
  if (r_approved == "APPROVED")
  {
    _errorMsg = errorMsg(0).arg(r_ref);
    status = "C";
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

  /* From here on, treat errors as warnings:
     do as much as possible,
     set _errorMsg if there are problems, and
     return a warning unless YourPay already gave an error.
  */

  XSqlQuery ypq;
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

  ypq.prepare("INSERT INTO ccpay ("
	     "    ccpay_id, ccpay_ccard_id, ccpay_cust_id, ccpay_auth_charge,"
	     "    ccpay_amount,"
	     "    ccpay_curr_id, ccpay_type, ccpay_order_number, ccpay_status,"
	     "    ccpay_yp_r_avs, ccpay_yp_r_ordernum, ccpay_yp_r_error,"
	     "    ccpay_yp_r_approved, ccpay_yp_r_code, ccpay_yp_r_score,"
	     "    ccpay_yp_r_shipping, ccpay_yp_r_tax, ccpay_yp_r_tdate,"
	     "    ccpay_yp_r_ref, ccpay_yp_r_message, ccpay_yp_r_time"
	     ") SELECT :ccpay_id, ccard_id, cust_id, 'R',"
	     "    ROUND(currToCurr(:curr_id,:yp_curr_id,:amount,CURRENT_DATE), 2),"
	     "    :yp_curr_id, :type, :reference, :status,"
	     "    :avs, :ordernum, :error,"
	     "    :approved, :code, :score,"
	     "    :shipping, :tax, :tdate,"
	     "    :ref, :message, :time"
	     "  FROM ccard, custinfo"
	     "  WHERE ((ccard_cust_id=cust_id)"
	     "    AND  (ccard_id=:ccard_id));");

  ypq.bindValue(":ccpay_id",   pccpayid);
  ypq.bindValue(":curr_id",    pcurrid);
  ypq.bindValue(":yp_curr_id", _currencyId);
  ypq.bindValue(":amount",     pamount);
  ypq.bindValue(":type",       ptype);
  ypq.bindValue(":reference",  porigorder);
  ypq.bindValue(":status",     status);
  ypq.bindValue(":avs",        r_avs);
  ypq.bindValue(":ordernum",   r_ordernum);
  ypq.bindValue(":error",      r_error);
  ypq.bindValue(":approved",   r_approved);
  ypq.bindValue(":code",       r_code);
  if (! r_score.isEmpty())
    ypq.bindValue(":score",    r_score);
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

  if (DEBUG) qDebug("r_error.isEmpty() = %d", r_error.isEmpty());

  if (r_error.isEmpty())
    return 0;
  else
  {
    _errorMsg = errorMsg(-12).arg(r_error);
    return -12;
  }
}

int YourPayProcessor::doCheckConfiguration()
{
  if ((_metrics->value("CCServer") != "staging.linkpt.net") &&
      (_metrics->value("CCServer") != "secure.linkpt.net"))
  {
    _errorMsg = errorMsg(-15)
		.arg(_metrics->value("CCServer"))
		.arg(_metrics->value("CCCompany"));
    return -15;

  }

  if (_metrics->value("CCPort").toInt() != 1129)
  {
    _errorMsg = errorMsg(-16)
		  .arg(_metrics->value("CCPort"))
		  .arg(_metrics->value("CCCompany"));
    return -16;
  }

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

bool YourPayProcessor::isLive()
{
  return (!_metrics->boolean("CCTest") &&
	   _metrics->value("CCServer") == "secure.linkpt.net");
}

bool YourPayProcessor::isTest()
{
  return (_metrics->boolean("CCTest") &&
	  _metrics->value("CCServer") == "staging.linkpt.net");
}
