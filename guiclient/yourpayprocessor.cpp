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

bool YourPayProcessor::doCredit(int pccpayid)
{
  XSqlQuery ypq;
  ypq.prepare( "SELECT ccpay.*, "
	       "       currToCurr(ccpay_id, :yp_curr_id,"
	       "              ccpay_amount, CURRENT_DATE) AS ccpay_amount_yp"
	       "       ccard_active, "
	       "       formatbytea(decrypt(setbytea(ccard_number),"
	       "               setbytea(:key), 'bf')) AS ccard_number,"
	       "       formatbytea(decrypt(setbytea(ccard_month_expired),"
	       "               setbytea(:key), 'bf')) AS ccard_month_expired,"
	       "       formatbytea(decrypt(setbytea(ccard_year_expired),"
	       "               setbytea(:key), 'bf')) AS ccard_year_expired,"
	       "       ccard_type "
	       "FROM ccpay, ccard "
	       "WHERE ((ccpay_ccard_id=ccard_id) "
	       " AND (ccpay_id=:ccpay_id));");
  ypq.bindValue(":ccpay_id",       pccpayid);
  ypq.bindValue(":yp_curr_id_id", _currencyId);
  ypq.bindValue(":key",            omfgThis->_key);
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

  QDomDocument transaction;
  QDomElement order = transaction.createElement("order");
  transaction.appendChild(order);

  QDomElement merchantinfo = transaction.createElement("merchantinfo");
  order.appendChild(merchantinfo);
  if (isLive())
    merchantinfo.setNodeValue(_storenum.toLatin1());
  else if (isTest())
    merchantinfo.setNodeValue((_storenum + " testing").toLatin1());

  QDomElement orderoptions = transaction.createElement("orderoptions");
  order.appendChild(orderoptions);
  QDomElement ordertype = transaction.createElement("ordertype");
  orderoptions.appendChild(ordertype);
  orderoptions.setNodeValue("CREDIT");

  QDomElement payment = transaction.createElement("payment");
  order.appendChild(payment);
  QDomElement chargetotal = transaction.createElement("chargetotal");
  chargetotal.setNodeValue(
	    QString::number(ypq.value("ccpay_amount_yp").toDouble(), 'f', 2));

  QDomElement creditcard = transaction.createElement("creditcard");
  order.appendChild(creditcard);
  QDomElement cardnumber = transaction.createElement("cardnumber");
  cardnumber.setNodeValue(ypq.value("ccard_number").toString());

  QDomElement cardexpmonth = transaction.createElement("cardexpmonth");
  cardexpmonth.setNodeValue(
    QString().arg(ypq.value("ccard_month_expired").toDouble(), 2, 'f', 0));
  QDomElement cardexpyear = transaction.createElement("cardexpyear");
  cardexpyear.setNodeValue(ypq.value("ccard_year_expired").toString().right(2));

  QDomElement transactiondetails = transaction.createElement("transactiondetails");
  order.appendChild(transactiondetails);
  QDomElement oid = transaction.createElement("oid");
  oid.setNodeValue(ypq.value("ccpay_yp_r_ordernum").toString());

  /* TODO: billing info not needed for credits?
  <billing>
    <addrnum>numeric portion of street address</addrnum>
    <zip>billing zip</zip>
  */

  qDebug(transaction.toString());

  // TODO: there's got to be a better place to put this
  if (isTest())
    _metrics->set("CCOrder", transaction.toString());

/////////////////////////////////
return false;
/////////////////////////////////

  QDomDocument response;
  if (! processXML(transaction, response))
    return false;

  if (! handleResponse(pccpayid, response))
    return true;	// TODO: return false? the YP xaction is done!

  return true;
}

bool YourPayProcessor::handleResponse(int pccpayid, const QDomDocument &response)
{
  QDomNode node;
  QDomElement root = response.documentElement();
  
  QString r_avs;
  QString r_ordernum;
  QString r_error;
  QString r_approved;
  QString r_ref;
  QString r_message;

  QString status;
  
  node = root.firstChild();
  // TODO: simplify the XML handling?
  // TODO: do we need to check all of these fields?
  while ( !node.isNull() )
  {
    if ( node.isElement() && node.nodeName() == "r_avs" )
      r_avs = node.nodeValue();

    else if ( node.isElement() && node.nodeName() == "r_ordernum" )
      r_ordernum = node.nodeValue();

    else if ( node.isElement() && node.nodeName() == "r_error" )
      r_error = node.nodeValue();

    else if ( node.isElement() && node.nodeName() == "r_approved" )
      r_approved = node.nodeValue();

    else if ( node.isElement() && node.nodeName() == "r_ref" )
      r_ref = node.nodeValue();

    else if ( node.isElement() && node.nodeName() == "r_message" )
      r_message = node.nodeValue();

    node = node.nextSibling();
  }

  if (r_approved == "APPROVED")
  {
    _errorMsg = tr("This transaction was approved\n" + r_ref);
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
    _errorMsg = tr("This transaction is a declined\n") + r_error;
    status = "D";
  }
  
  else if (r_approved == "FRAUD")
  {
    _errorMsg = tr("This transaction is denied because of possible fraud\n") +
		r_error;
    status = "D";
  }
  
  else if (r_approved.isEmpty())
  {
    _errorMsg = tr("<p>No Approval Code<br>%1<br>%2<br>%3")
		     .arg(r_error).arg(r_message).arg(response.toString());
    status = "X";
  }


  XSqlQuery ypq;
  ypq.prepare("UPDATE ccpay SET ccpay_status=:status,"
	      "                 ccpay_yp_r_avs=:avs,"
	      "                 ccpay_yp_r_ordernum=:ordernum,"
	      "                 ccpay_yp_r_error=:error "
	      "WHERE (ccpay_id=:ccpay_id);");
  ypq.bindValue(":status",   status);
  ypq.bindValue(":avs",      r_avs);
  ypq.bindValue(":ordernum", r_ordernum);
  ypq.bindValue(":error",    r_error);
  ypq.bindValue(":ccpay_id", pccpayid);
  ypq.exec();
  if (ypq.lastError().type() != QSqlError::None)
  {
    _errorMsg = ypq.lastError().databaseText();
    return false;
  }

  return true;
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
