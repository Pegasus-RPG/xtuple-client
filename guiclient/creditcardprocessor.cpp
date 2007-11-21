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

#include <QApplication>
#include <QMessageBox>
#include <QProcess>
#include <QSqlError>

#include <currcluster.h>

#include "OpenMFGGUIClient.h"
#include "creditcardprocessor.h"
#include "storedProcErrorLookup.h"
#include "verisignprocessor.h"
#include "yourpayprocessor.h"

#define DEBUG true

#define CCPOSTPROC_WARNING tr("The Credit Card transaction completed " \
			      "successfully but the application encountered " \
			      "an error with follow-up processing:\n%1")

// TODO: make this a static method of CurrDisplay
static QString currSymbol(int pcurrid)
{
  XSqlQuery cs;
  cs.prepare("SELECT curr_symbol FROM curr_symbol WHERE (curr_id=:currid);");
  cs.bindValue(":currid", pcurrid);
  cs.exec();
  if (cs.first())
    return cs.value("curr_symbol").toString();

  return "";
}

CreditCardProcessor *CreditCardProcessor::_processor = 0;
QString CreditCardProcessor::_errorMsg = "";

CreditCardProcessor::CreditCardProcessor()
{
  if (DEBUG) qDebug("CreditCardProcessor()");
  _currencyId     = 0;
  _currencyName   = "";
  _currencySymbol = "";
  _errorMsg       = "";
}

CreditCardProcessor::~CreditCardProcessor()
{
}

CreditCardProcessor * CreditCardProcessor::getProcessor()
{
  if (DEBUG) qDebug("getProcessor()");
  _errorMsg = "";
  if (! _processor)
  {
    if ((_metrics->value("CCCompany") == "YourPay"))
      _processor = new YourPayProcessor();

    else if (_metrics->value("CCCompany") == "Verisign")
      _processor = new VerisignProcessor();

    else
    {
      _errorMsg = tr("Could not figure out which Credit Card Processor "
		     "to set up (based on %1).")
		    .arg(_metrics->value("CCServer"));
    }
  }

  if (_processor && ! _processor->checkConfiguration())
  {
    delete _processor;
    _processor = 0;
  }

  return _processor;
}

bool CreditCardProcessor::authorize()
{
  if (DEBUG) qDebug("authorize()");
  _errorMsg = "";
  return doAuthorize();
}

bool CreditCardProcessor::charge()
{
  if (DEBUG) qDebug("charge()");
  _errorMsg = "";
  return doCharge();
}

bool CreditCardProcessor::chargePreauthorized(float pamount, int pcurrid, int pccpayid, int pcustid)
{
  if (DEBUG) qDebug("chargePreauthorized(%f, %d, %d, %d)", pamount, pcurrid, pccpayid, pcustid);
  _errorMsg = "";
  int ccValidDays = _metrics->value("CCValidDays").toInt();
  if(ccValidDays < 1)
    ccValidDays = 7;

  // look for a pre-authorization that covers this
  if (pccpayid > 0)
  {
    q.prepare("SELECT ccpay_amount, ccpay_curr_id "
	      "FROM ccpay "
	      "WHERE ((ccpay_status = 'A')"
	      "  AND  (date_part('day', CURRENT_TIMESTAMP - "
	      "                         ccpay_transaction_datetime) < "
	      "        :ccValidDays)"
	      "  AND  (ccpay_id=:id));");
    q.bindValue(":id", pccpayid);
  }
  else if (pcustid > 0)
  {
    q.prepare("SELECT currToBase(ccpay_curr_id, ccpay_amount, CURRENT_DATE)"
	      "              AS ccpay_amount, basecurrid() AS ccpay_curr_id"
	      "FROM ccpay "
	      " WHERE ((ccpay_status='A')"
	      "   AND  (date_part('day', CURRENT_TIMESTAMP - "
	      "                          ccpay_transaction_datetime) <"
	      "         :ccValidDays)"
	      "   AND  (ccpay_cust_id=:id) );");
    q.bindValue(":id", pcustid);
  }
  else
  {
    _errorMsg = tr("You must supply either a ccpay id or a customer id.");
    return false;
  }
  q.bindValue(":ccValidDays", ccValidDays);
  q.exec();
  if (q.first())
  {
    if (pamount > CurrDisplay::convert(q.value("ccpay_curr_id").toInt(),
				       pcurrid,
				       q.value("ccpay_amount").toDouble(),
				       QDate::currentDate()) )
    {
      _errorMsg = tr("The preauthorization (for %1) is not sufficient to cover "
		     "the desired transaction amount (%2).")
		  .arg(q.value("ccpay_amount").toString())
		  .arg(pamount);
      return false;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    _errorMsg = q.lastError().databaseText();
    return false;
  }
  else
  {
    _errorMsg = tr("No Preauthorization found");
    return false;
  }

  if (_metrics->value("CCConfirmTrans") == "A" ||
      _metrics->value("CCConfirmTrans") == "B")
  {
    if (QMessageBox::question(0,
	      tr("Confirm Post-authorization of Credit Card Purchase"),
              tr("You must confirm that you wish to post authorize process "
                 "this charge %1 %2. Would you like to post authorize now?")
                 .arg(_currencySymbol)
                 .arg(pamount),
              QMessageBox::Yes | QMessageBox::Default,
              QMessageBox::No  | QMessageBox::Escape ) == QMessageBox::No)
    {
      _errorMsg = tr("User chose not to post-authorize process the charge.");
      return false;
    }
  }

  QString approvalStatus;
  if (! doChargePreauthorized(approvalStatus))
    return false;

  q.prepare( "UPDATE ccpay"
             "   SET ccpay_amount = :ccpay_amount, "
             "       ccpay_auth = FALSE, "
             "       ccpay_status = :ccpay_status, "
             "       ccpay_curr_id = :ccpay_curr_id "
             " WHERE ccpay_id = :ccpay_id;" );
  q.bindValue(":ccpay_id",      pccpayid);
  if (approvalStatus == "D" || approvalStatus == "X")
    q.bindValue(":ccpay_amount",  0);
  else
    q.bindValue(":ccpay_amount",  pamount);
  q.bindValue(":ccpay_curr_id", pcurrid);
  q.bindValue(":ccpay_status",  approvalStatus);
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    _errorMsg = q.lastError().databaseText();
    // TODO: return true;? return false;? log an event?
    // one way or another the transaction has gone through to the processor
  }

  if (approvalStatus == "D" || approvalStatus == "X")
  {
    ; // TODO: what do we do if the charge was not successful?
  }
  else
  {
    q.prepare("INSERT INTO cashrcpt ("
              "  cashrcpt_cust_id, cashrcpt_amount, cashrcpt_curr_id,"
              "  cashrcpt_fundstype, cashrcpt_docnumber,"
              "  cashrcpt_bankaccnt_id, cashrcpt_notes, cashrcpt_distdate) "
	      "SELECT ccpay_cust_id, :amount, :curr_id,"
	      "       ccard_type, ccpay_yp_r_ordernum,"
	      "       :bankaccnt_id, :notes, current_date"
	      "  FROM ccpay, ccard "
	      "WHERE (ccpay_ccard_id=ccard_id);");
    q.bindValue(":amount",       pamount);
    q.bindValue(":curr_id",      pcurrid);
    q.bindValue(":bankaccnt_id", _metrics->value("CCDefaultBank").toInt());
    q.bindValue(":notes",        "Converted Pre-auth");
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      _errorMsg = q.lastError().databaseText();
      // TODO: return true;? return false;? log an event?
      // one way or another the transaction has gone through to the processor
    }
  }

  return true;
}

bool CreditCardProcessor::checkConfiguration()
{
  if (DEBUG) qDebug("checkConfiguration()");
  _errorMsg = "";

  if (!_privleges->check("ProcessCreditCards"))
  {
    _errorMsg = tr("You do not have permission to process Credit Card "
		   "transactions.");
    return false;
  }

  if(!_metrics->boolean("CCAccept"))
  {
    _errorMsg = tr("The application is not set up to process credit cards.");
    return false;
  }

  if (_metrics->value("CCDefaultBank").isEmpty())
  {
    _errorMsg = tr("The Bank Account is not set for Credit Card transactions.");
    return false;
  }

  QString key = omfgThis->_key;

  if (key.isEmpty())
  {
    _errorMsg = tr("You do not have an encryption key defined");
    return false;
  }

  if(_metrics->boolean("CCUseProxyServer"))
  {
    _plogin = _metricsenc->value("CCProxyLogin");
    if (_plogin.isEmpty())
    {
      _errorMsg = tr("You have selected proxy server support, yet you have"
		     "not provided a login");
     return false;
    }

    _ppassword = _metricsenc->value("CCPassword");
    if (_ppassword.isEmpty())
    {
      _errorMsg = tr("You have selected proxy server support, yet you have "
		     "not provided a password");
     return false;
    }

    _pserver = _metrics->value("CCProxyServer");
    if (_pserver.isEmpty())
    {
      _errorMsg = tr("You have selected proxy server support, yet you have "
		     "not provided a proxy server to use");
     return false;
    }

    _pport = _metrics->value("CCProxyPort");
    if (_pport.isEmpty())
    {
      _errorMsg = tr("You have selected proxy server support, yet you have "
		     "not provided a proxy port to use");
     return false;
    }
  }

  return doCheckConfiguration();
}

bool CreditCardProcessor::credit(const QString &pneworder, int &pccpayid)
{
  if (DEBUG) qDebug("credit(%s, %d)", pneworder.toAscii().data(), pccpayid);

  _errorMsg = "";
  XSqlQuery ccq;
  ccq.prepare("SELECT * "
	      "FROM ccpay "
	      "WHERE ((ccpay_id=:ccpayid) AND (ccpay_type!= 'R'));");
  ccq.bindValue(":ccpayid", pccpayid);
  ccq.exec();
  if (ccq.first())
  {
    return credit(ccq.value("ccpay_ccard_id").toInt(),
		  ccq.value("ccpay_amount").toDouble(),
		  ccq.value("ccpay_curr_id").toInt(),
		  pneworder.isEmpty() ?
		      ccq.value("ccpay_order_number").toString() : pneworder,
		  ccq.value("ccpay_order_number").toString(),
		  pccpayid);
  }
  else if (ccq.lastError().type() != QSqlError::None)
  {
    _errorMsg = ccq.lastError().databaseText();
    return false;
  }
  else
  {
    _errorMsg = tr("Could not find original Credit Card payment to credit.");
    return false;
  }
}

bool CreditCardProcessor::credit(const int pccardid, const double pamount, const int pcurrid, const QString &pneworder, const QString &porigorder, int &pccpayid)
{
  if (DEBUG)
    qDebug("credit(%d, %f, %d, %s, %s, %d)", pccardid, pamount, pcurrid,
	    pneworder.toAscii().data(), porigorder.toAscii().data(), pccpayid);
  _errorMsg = "";
  if (_metrics->value("CCConfirmTrans") == "R" ||
      _metrics->value("CCConfirmTrans") == "B")
  {
    if (QMessageBox::question(0,
	      tr("Confirm Credit Card Credit"),
              tr("You must confirm that you wish to credit "
                 "%1 %2. Would you like to credit this amount now?")
                 .arg(currSymbol(pcurrid))
                 .arg(pamount),
              QMessageBox::Yes | QMessageBox::Default,
              QMessageBox::No  | QMessageBox::Escape ) == QMessageBox::No)
      return false;
  }

  if (! doCredit(pccardid, pamount, pcurrid, pneworder, porigorder, pccpayid))
    return false;
  else if (! _errorMsg.isEmpty())
    _errorMsg = CCPOSTPROC_WARNING.arg(_errorMsg);

  if (pccpayid > 0)
  {
    XSqlQuery cq;
    cq.prepare("SELECT postCCCredit(:ccpayid) AS result;");
    cq.bindValue(":ccpayid", pccpayid);
    cq.exec();
    if (cq.first())
    {
      int result = cq.value("result").toInt();
      if (result < 0)
	_errorMsg = CCPOSTPROC_WARNING.arg(storedProcErrorLookup("postCCCredit",
								 result));
    }
    else if (cq.lastError().type() != QSqlError::NoError)
      _errorMsg = CCPOSTPROC_WARNING.arg(cq.lastError().databaseText());
  }

  return true;
}

bool CreditCardProcessor::doCredit(const int pccardid, const double pamount, const int pcurrid, const QString &pneworder, const QString &porigorder, int &pccpayid)
{
  if (DEBUG)
    qDebug("doCredit(%d, %f, %d, %s, %s, %d)", pccardid, pamount, pcurrid,
	    pneworder.toAscii().data(), porigorder.toAscii().data(), pccpayid);
  return false;
}

bool CreditCardProcessor::voidPrevious()
{
  if (DEBUG) qDebug("voidPrevious()");
  _errorMsg = "";
  return doVoidPrevious();
}

int CreditCardProcessor::currencyId()
{
  _errorMsg = "";
  return _currencyId;
}

QString CreditCardProcessor::currencyName()
{
  _errorMsg = "";
  return _currencyName;
}

QString CreditCardProcessor::currencySymbol()
{
  _errorMsg = "";
  return _currencySymbol;
}

bool CreditCardProcessor::isLive()
{
  _errorMsg = "";
  return true;	// safest assumption
}

bool CreditCardProcessor::isTest()
{
  _errorMsg = "";
  return false;	// safest assumption
}

bool CreditCardProcessor::checkCreditCard(int pccid)
{
  if (DEBUG) qDebug("checkCreditCard(%d)", pccid);
  _errorMsg = "";

  QString key = omfgThis->_key;

  if(key.isEmpty())
  {
    _errorMsg = tr("You do not have an encryption key defined");
    return false;
  }

  q.prepare( "SELECT ccard_active, "
             "       formatbytea(decrypt(setbytea(ccard_month_expired),"
	     "               setbytea(:key), 'bf')) AS ccard_month_expired,"
             "       formatbytea(decrypt(setbytea(ccard_year_expired),"
	     "               setbytea(:key), 'bf')) AS ccard_year_expired,"
             "       ccard_type "
             "FROM ccard "
             "WHERE (ccard_id=:ccardid);");
  q.bindValue(":ccardid", pccid);
  q.bindValue(":key",     key);
  q.exec();
  if (q.first())
  {
    if (!q.value("ccard_active").toBool())
    {
      _errorMsg = tr("The Credit Card you are attempting to use is not active.");
      return false;
    }

    if (QDate(q.value("ccard_year_expired").toInt(),
	      q.value("ccard_month_expired").toInt(),
	      1) < QDate::currentDate())
    {
      _errorMsg = tr("The Credit Card you are attempting to use has expired.");
      return false;
    }
  }

  return true;
}

bool CreditCardProcessor::doChargePreauthorized(QString &pStatus)
{
  if (DEBUG) qDebug("doChargePreauthorized(&pStatus)");
  _errorMsg = "";
  pStatus = "X"; // No approval code obtained, must be valid ccpay_status
  return false;
}

bool CreditCardProcessor::processXML(const QDomDocument &ptransaction,
				     QDomDocument &presponse)
{
  if (DEBUG) qDebug("processXML(input, output) with input:\n%s", ptransaction.toString().toAscii().data());
  _errorMsg = "";
  QString transactionString = ptransaction.toString();

  // TODO: find a better place to save this
  if (isTest())
    _metrics->set("CCOrder", transactionString);

  QProcess proc(this);
  QString curl_path;
#ifdef Q_WS_WIN
  curl_path = qApp->applicationDirPath() + "\\curl";
#elif defined Q_WS_MACX
  curl_path = "/usr/bin/curl";
#elif defined Q_WS_X11
  curl_path = "/usr/bin/curl";
#endif

  QStringList curl_args;
  curl_args.append( "-k" );
  curl_args.append( "-d" );
  curl_args.append( transactionString );
  curl_args.append( "-E" );

#ifdef Q_WS_WIN
  curl_args.append(_metrics->value("CCYPWinPathPEM"));
#elif defined Q_WS_MACX
  curl_args.append(_metrics->value("CCYPMacPathPEM"));
#elif defined Q_WS_X11
  curl_args.append(_metrics->value("CCYPLinPathPEM"));
#endif

  curl_args.append("https://" + _metrics->value("CCServer") +
		   ":" + _metrics->value("CCPort"));

  if(_metrics->boolean("CCUseProxyServer"))
  {
    curl_args.append( "-x" );
    curl_args.append(_metrics->value("CCProxyServer") + ":" + _metrics->value("CCProxyPort"));
    curl_args.append( "-U" );
    curl_args.append(_metricsenc->value("CCProxyLogin") + ":" + _metricsenc->value("CCPassword"));
  }

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
  proc.start(curl_path, curl_args);
  if ( !proc.waitForStarted() )
  {
    _errorMsg = tr("Error starting message transfer program: %1\n\n%2")
		  .arg(curl_path).arg(QString(proc.readAllStandardError()));
    return false;
  }

  if (! proc.waitForFinished())
  {
    QApplication::restoreOverrideCursor();
    _errorMsg = tr("Error running message transfer program: %1\n\n%2")
		  .arg(curl_path).arg(QString(proc.readAllStandardError()));
    return false;
  }

  QApplication::restoreOverrideCursor();

  if (proc.exitStatus() != QProcess::NormalExit)
  {
    _errorMsg = tr("The message transfer program exit abnormally: %1\n\n%2")
		  .arg(curl_path).arg(QString(proc.readAllStandardError()));
    return false;
  }

  if (proc.exitCode() != 0)
  {
    _errorMsg = tr("The message transfer program returned an error: "
		   "%1 returned %2\n\n%3")
		  .arg(curl_path)
		  .arg(proc.exitCode())
		  .arg(QString(proc.readAllStandardError()));
    return false;
  }

  // YourPay's response isn't valid XML: it has no root element!
  QString responseString = "<yp_wrapper>" +
			   proc.readAllStandardOutput() +
			   "</yp_wrapper>";
  if (isTest())
    _metrics->set("CCTestMe", responseString);

  presponse.setContent(responseString);

  return true;
}
