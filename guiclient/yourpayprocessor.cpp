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

#define DEBUG true

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
}

bool YourPayProcessor::doCredit(const int pccardid, const double pamount, const int pcurrid, const QString &pneworder, const QString &porigorder, int &pccpayid)
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
      _errorMsg = tr("The Credit Card you are trying to use is not active.");
      return false;
    }
  }
  else if (ypq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = ypq.lastError().databaseText();
    return false;
  }
  else
  {
    _errorMsg = tr("Could not find the credit transaction.");
    return false;
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
  if (! processXML(request, response))
    return false;

  if (! handleResponse(response, pccardid, "R", pamount, pcurrid, porigorder, pccpayid))
    return false;

  return true;
}

bool YourPayProcessor::handleResponse(const QDomDocument &response, const int pccardid, const QString &ptype, const double pamount, const int pcurrid, const QString &porigorder, int &pccpayid)
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

  if (r_approved == "APPROVED")
  {
    _errorMsg = tr("This transaction was approved\n") + r_ref;
    status = "C";
  }

  else if (r_approved == "DENIED")
  {
    _errorMsg = tr("This transaction was denied\n") + r_message;
    status = "D";
  }
  
  else if (r_approved == "DUPLICATE")
  {
    _errorMsg = tr("This transaction is a duplicate\n") + r_message;
    status = "D";
  }
  
  else if (r_approved == "DECLINED")
  {
    _errorMsg = tr("This transaction was declined\n") + r_error;
    status = "D";
  }
  
  else if (r_approved == "FRAUD")
  {
    _errorMsg = tr("This transaction was denied because of possible fraud\n") +
		r_error;
    status = "D";
  }
  
  else if (r_approved.isEmpty())
  {
    _errorMsg = tr("<p>No Approval Code<br>%1<br>%2<br>%3")
		     .arg(r_error).arg(r_message).arg(response.toString());
    status = "X";
  }

  /* From here on, treat errors as warnings:
     do as much as possible,
     set _errorMsg if there are problems, and
     return true unless YourPay returned an error.
  */

  XSqlQuery ypq;
  ypq.exec("SELECT NEXTVAL('ccpay_ccpay_id_seq') AS ccpay_id;");
  if (ypq.first())
    pccpayid = ypq.value("ccpay_id").toInt();
  else if (ypq.lastError().type() != QSqlError::None && r_error.isEmpty())
  {
    _errorMsg = ypq.lastError().databaseText();
    return true;
  }
  else if (ypq.lastError().type() == QSqlError::None && r_error.isEmpty())
  {
    _errorMsg = tr("Could not generate a unique key for the ccpay table.");
    return true;
  }
  else	// no rows found and YP reported an error
  {
    _errorMsg = r_error;
    return false;
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
    _errorMsg = ypq.lastError().databaseText();
  }

  if (DEBUG) qDebug("r_error.isEmpty() = %d", r_error.isEmpty());

  if (r_error.isEmpty())
    return true;
  else
  {
    _errorMsg = r_error;
    return false;
  }
}

bool YourPayProcessor::doCheckConfiguration()
{
  if ((_metrics->value("CCServer") != "staging.linkpt.net") &&
      (_metrics->value("CCServer") != "secure.linkpt.net"))
  {
    _errorMsg = tr("The CCServer %1 is not valid for YourPay.")
		  .arg(_metrics->value("CCServer"));
    return false;

  }

  if (!isLive() && !isTest())
  {
    _errorMsg = tr("If Credit Card test is selected you must select a test "
		   "server. If Credit Card Test is not selected you must "
		   "select a production server");
    return false;
  }

  if (_metrics->value("CCPort").toInt() != 1129)
  {
    _errorMsg = tr("You have an invalid port identified for the requested "
		   "server");
    return false;
  }

  _storenum = _metricsenc->value("CCYPStoreNum");
  if (_storenum.isEmpty())
  {
    _errorMsg = tr("The YourPay Store Number is missing");
    return false;
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
    _errorMsg = tr("The YourPay PEM file is not set.") ;
    return false;
  }

  QFileInfo fileinfo(_pemfile.toLatin1());
  if (!fileinfo.isFile())
  {
    _errorMsg = tr("Could not open PEM file %1", _pemfile.toLatin1());
   return false;
  }

  return true;
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
