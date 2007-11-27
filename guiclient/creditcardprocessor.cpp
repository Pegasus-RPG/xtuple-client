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

#define DEBUG false

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

CreditCardProcessor	*CreditCardProcessor::_processor = 0;
QString			 CreditCardProcessor::_errorMsg = "";
QHash<int, QString>	 CreditCardProcessor::_msgHash;

/*
   > 0 => credit card transaction processing completed but there is a warning
   	  condition: the credit card processing company denied the transaction
	  or the post-processing generated an error

   = 0 => transaction succeeded in its entirety

   < 0 => preprocessing failed or the credit card company returned an error

   codes < -100 and codes > 100 are available for use by subclasses
   and should be loaded into _msgHash in the subclass' constructor.
 */
#define TR(a)	QObject::tr(a)
static struct {
    int		code;
    QString	text;
} messages[] = {

  {   0, TR("This transaction was approved.\n%1")			},

  {  -1, TR("Database Error")						},
  {  -2, TR("You do not have permission to process Credit Card transactions.") },
  {  -3, TR("The application is not set up to process credit cards.")	},
  {  -4, TR("The Bank Account is not set for Credit Card transactions.") },
  {  -5, TR("The encryption key is not defined.")			},
  {  -6, TR("The login for the proxy server is not defined.")		},
  {  -7, TR("The password for the proxy server is not defined.")	},
  {  -8, TR("The proxy server is not defined.")				},
  {  -9, TR("The port to use for the proxy server is not defined.")	},
  { -10, TR("This Credit Card is not active.")				},
  { -11, TR("This Credit Card has expired.")				},
  { -12, TR("The Credit Card Processor reported an error:\n%1")		},
  { -13, TR("If Credit Card configuration is inconsistent and the application "
	    "cannot determine whether to run in Test or Live mode.")	},
  { -14, TR("Could not figure out which Credit Card Processor to set up "
	    "(based on %1).") 						},
  { -15, TR("The CCServer %1 is not valid for %2.")			},
  { -16, TR("The CCPort %1 is not valid for %2.")			},
  { -18, TR("Error with message transfer program:\n%1 %2\n\n%3")	},
  { -19, TR("%1 is not implemented.")					},

  // preauthorizing charges
  //{ -20, TR("")							},

  // processing charges based on a pre-authorization
  { -32, TR("You must supply either a ccpay id or a customer id.") 	},
  { -33, TR("The preauthorization (for %1) is not sufficient to cover "
		 "the desired transaction amount (%2).")		},
  { -34, TR("No Preauthorization found")				},

  // stand-alone charges
  //{ -40, TR("")							},

  // credits
  { -50, TR("Could not find original Credit Card payment to credit.")	},

  // voids
  //{ -60, TR("")							},

  // positive values: warnings
  {   1, TR("Database Error")						},
  {   2, TR("Could not generate a unique key for the ccpay table.")	},
  {  10, TR("This Credit Card transaction was denied.\n%1")		},
  {  11, TR("This Credit Card transaction is a duplicate.\n%1")		},
  {  12, TR("This Credit Card transaction was declined.\n%1")		},
  {  13, TR("This Credit Card transaction was denied "
	    "because of possible fraud.\n%1")				},
  {  30, TR("User chose not to post-authorize process the charge.")	},
  {  50, TR("User chose not to process the credit.")			},

};

CreditCardProcessor::CreditCardProcessor()
{
  if (DEBUG) qDebug("CreditCardProcessor()");
  _currencyId     = 0;
  _currencyName   = "";
  _currencySymbol = "";
  _errorMsg       = "";

  for (unsigned int i = 0; i < sizeof(messages) / sizeof(messages[0]); i++)
    _msgHash.insert(messages[i].code, messages[i].text);
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
      _errorMsg = errorMsg(-14).arg(_metrics->value("CCServer"));
  }

  if (_processor && _processor->checkConfiguration() != 0)
  {
    delete _processor;
    _processor = 0;
  }

  return _processor;
}

int CreditCardProcessor::authorize()
{
  if (DEBUG) qDebug("authorize()");
  _errorMsg = "";
  return doAuthorize();
}

int CreditCardProcessor::charge()
{
  if (DEBUG) qDebug("charge()");
  _errorMsg = "";
  return doCharge();
}

int CreditCardProcessor::chargePreauthorized(float pamount, int pcurrid, int pccpayid, int pcustid)
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
    _errorMsg = errorMsg(-32);
    return -32;
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
      _errorMsg = errorMsg(-33)
		  .arg(q.value("ccpay_amount").toString())
		  .arg(pamount);
      return -33;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    _errorMsg = q.lastError().databaseText();
    return -1;
  }
  else
  {
    _errorMsg = errorMsg(-34);
    return -34;
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
      _errorMsg = errorMsg(30);
      return 30;
    }
  }

  QString approvalStatus;
  int returnVal = doChargePreauthorized(approvalStatus);
  if (returnVal < 0)
    return returnVal;

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
    // TODO: log an event?
    returnVal = 1;
  }

  if (approvalStatus == "D" || approvalStatus == "X")
  {
    ; // returnVal should have been set by doChargePreauthorized
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
      // TODO: log an event?
      returnVal = 1;
    }
  }

  return returnVal;
}

int CreditCardProcessor::checkConfiguration()
{
  if (DEBUG) qDebug("checkConfiguration()");
  _errorMsg = "";

  if (!_privleges->check("ProcessCreditCards"))
  {
    _errorMsg = errorMsg(-2);
    return -2;
  }

  if(!_metrics->boolean("CCAccept"))
  {
    _errorMsg = errorMsg(-3);
    return -3;
  }

  if (_metrics->value("CCDefaultBank").isEmpty())
  {
    _errorMsg = errorMsg(-4);
    return -4;
  }

  QString key = omfgThis->_key;

  if (key.isEmpty())
  {
    _errorMsg = errorMsg(-5);
    return -5;
  }

  if(_metrics->boolean("CCUseProxyServer"))
  {
    _plogin = _metricsenc->value("CCProxyLogin");
    if (_plogin.isEmpty())
    {
      _errorMsg = errorMsg(-6);
      return -6;
    }

    _ppassword = _metricsenc->value("CCPassword");
    if (_ppassword.isEmpty())
    {
      _errorMsg = errorMsg(-7);
      return -7;
    }

    _pserver = _metrics->value("CCProxyServer");
    if (_pserver.isEmpty())
    {
      _errorMsg = errorMsg(-8);
      return -8;
    }

    _pport = _metrics->value("CCProxyPort");
    if (_pport.isEmpty())
    {
      _errorMsg = errorMsg(-9);
      return -9;
    }
  }

  if (isLive() == isTest())	// if both true or both false
  {
    _errorMsg = errorMsg(-13);
    return -13;
  }

  return doCheckConfiguration();
}

int CreditCardProcessor::credit(const QString &pneworder, int &pccpayid)
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
    return -1;
  }
  else
  {
    _errorMsg = errorMsg(-50);
    return -50;
  }
}

int CreditCardProcessor::credit(const int pccardid, const double pamount, const int pcurrid, const QString &pneworder, const QString &porigorder, int &pccpayid)
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
      {
	_errorMsg = errorMsg(50);
	return 50;
      }
  }

  int returnValue = doCredit(pccardid, pamount, pcurrid, pneworder,
			     porigorder, pccpayid);
  if (returnValue < 0)
    return returnValue;
  else if (returnValue > 0)
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
      {
	_errorMsg = CCPOSTPROC_WARNING.arg(storedProcErrorLookup("postCCCredit",
								 result));
	returnValue = 1;
      }
    }
    else if (cq.lastError().type() != QSqlError::NoError)
    {
      _errorMsg = CCPOSTPROC_WARNING.arg(cq.lastError().databaseText());
      returnValue = 1;
    }
  }

  return returnValue;
}

int CreditCardProcessor::doCredit(const int pccardid, const double pamount, const int pcurrid, const QString &pneworder, const QString &porigorder, int &pccpayid)
{
  if (DEBUG)
    qDebug("doCredit(%d, %f, %d, %s, %s, %d)", pccardid, pamount, pcurrid,
	    pneworder.toAscii().data(), porigorder.toAscii().data(), pccpayid);
  return 0;
}

int CreditCardProcessor::voidPrevious()
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

QString CreditCardProcessor::errorMsg()
{
  return _errorMsg;
}

QString CreditCardProcessor::errorMsg(const int pcode)
{
  return _msgHash.value(pcode);
}

int CreditCardProcessor::checkCreditCard(int pccid)
{
  if (DEBUG) qDebug("checkCreditCard(%d)", pccid);
  _errorMsg = "";

  QString key = omfgThis->_key;

  if(key.isEmpty())
  {
    _errorMsg = errorMsg(-5);
    return -5;
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
      _errorMsg = errorMsg(-10);
      return -10;
    }

    if (QDate(q.value("ccard_year_expired").toInt(),
	      q.value("ccard_month_expired").toInt(),
	      1) < QDate::currentDate())
    {
      _errorMsg = errorMsg(-11);
      return -11;
    }
  }

  return 0;
}

int CreditCardProcessor::doChargePreauthorized(QString &pStatus)
{
  if (DEBUG) qDebug("doChargePreauthorized(&pStatus)");
  pStatus = "X"; // No approval code obtained, must be valid ccpay_status
  _errorMsg = errorMsg(-19).arg("doChargePreauthorized");
  return -19;
}

int CreditCardProcessor::processXML(const QDomDocument &ptransaction,
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
    QApplication::restoreOverrideCursor();
    _errorMsg = errorMsg(-18)
		  .arg(curl_path)
		  .arg("")
		  .arg(QString(proc.readAllStandardError()));
    return -18;
  }

  if (! proc.waitForFinished())
  {
    QApplication::restoreOverrideCursor();
    _errorMsg = errorMsg(-18)
		  .arg(curl_path)
		  .arg("")
		  .arg(QString(proc.readAllStandardError()));
    return -18;
  }

  QApplication::restoreOverrideCursor();

  if (proc.exitStatus() != QProcess::NormalExit)
  {
    _errorMsg = errorMsg(-18)
		  .arg(curl_path)
		  .arg("")
		  .arg(QString(proc.readAllStandardError()));
    return -18;
  }

  if (proc.exitCode() != 0)
  {
    _errorMsg = errorMsg(-18)
		  .arg(curl_path)
		  .arg(proc.exitCode())
		  .arg(QString(proc.readAllStandardError()));
    return -18;
  }

  // YourPay's response isn't valid XML: it has no root element!
  QString responseString = "<yp_wrapper>" +
			   proc.readAllStandardOutput() +
			   "</yp_wrapper>";
  if (isTest())
    _metrics->set("CCTestMe", responseString);

  presponse.setContent(responseString);

  return 0;
}
