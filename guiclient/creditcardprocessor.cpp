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
			      "an error and did not record this correctly:\n%1")

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
  { -10, TR("Credit Card %1 is not active. Make it active or select "
	    "another Credit Card.")					},
  { -11, TR("Credit Card %1 has expired.")				},
  { -12, TR("The Credit Card Processor reported an error:\n%1")		},
  { -13, TR("If Credit Card configuration is inconsistent and the application "
	    "cannot determine whether to run in Test or Live mode.")	},
  { -14, TR("Could not figure out which Credit Card Processor to set up "
	    "(based on %1).") 						},
  { -15, TR("The CCServer %1 is not valid for %2.")			},
  { -16, TR("The CCPort %1 is not valid for %2.")			},
  { -17, TR("Could not find a Credit Card with internal ID %1.")	},
  { -18, TR("Error with message transfer program:\n%1 %2\n\n%3")	},
  { -19, TR("%1 is not implemented.")					},

  // preauthorizing charges
  { -20, TR("The amount to charge must be greater than 0.00.")		},
  { -24, TR("Could not generate a sequence number while preauthorizing.") },

  // processing charges based on a pre-authorization
  { -30, TR("Could not find original Credit Card preauthorization to charge.")	},
  { -32, TR("You must select a preauthorization to charge.") 		},
  { -33, TR("The preauthorization (for %1) is not sufficient to cover "
		 "the desired transaction amount (%2).")		},
  { -34, TR("No Preauthorization found")				},

  // stand-alone charges
  { -40, TR("Inconsistent data passed to charge(): [%1] [%2]")	},
  { -44, TR("Could not generate a sequence number while charging.")	},

  // credits
  { -50, TR("Could not find original Credit Card payment to credit.")	},

  // voids
  //{ -60, TR("")							},

  // other misc errors
  { -99, TR("The CVV value is not valid.")				},
  {-100, TR("No approval code was received:\n%1\n%2\n%3")		},

  // positive values: warnings
  {   1, TR("Database Error")						},
  {   2, TR("Could not generate a unique key for the ccpay table.")	},
  {   3, TR("Stored Procedure Error")					},
  {  10, TR("This Credit Card transaction was denied.\n%1")		},
  {  11, TR("This Credit Card transaction is a duplicate.\n%1")		},
  {  12, TR("This Credit Card transaction was declined.\n%1")		},
  {  13, TR("This Credit Card transaction was denied "
	    "because of possible fraud.\n%1")				},
  {  20, TR("User chose not to process the preauthorization.")		},
  {  30, TR("User chose not to post-authorize process the charge.")	},
  {  40, TR("User chose not to process the charge.")	},
  {  50, TR("User chose not to process the credit.")			},
  {  99, TR("User chose not to proceed without CVV code.")		},

};

CreditCardProcessor::CreditCardProcessor()
{
  if (DEBUG)
    qDebug("CCP:CreditCardProcessor()");
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
  if (DEBUG)
    qDebug("CCP:getProcessor()");
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

int CreditCardProcessor::authorize(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &porder, int &pccpayid, QString preftype, int &prefid)
{
  if (DEBUG)
    qDebug("CCP:authorize(%d, %d, %f, %d, %s, %d)",
	   pccardid, pcvv, pamount, pcurrid, porder.toAscii().data(), pccpayid);
  _errorMsg = "";

  if (pamount <= 0)
  {
    _errorMsg = errorMsg(-20);
    return -20;
  }

  QString ccard_x;
  int returnVal = checkCreditCard(pccardid, pcvv, ccard_x);
  if (returnVal < 0)
    return returnVal;

  if (_metrics->value("CCConfirmTrans") == "A" ||
      _metrics->value("CCConfirmTrans") == "B")
  {
    if (QMessageBox::question(0,
		    tr("Confirm Preauthorization of Credit Card Purchase"),
		    tr("You must confirm that you wish to preauthorize "
		       "credit card %1 in the amount of %2 %3. "
		       "Would you like to preauthorize now?")
		       .arg(ccard_x)
		       .arg((pcurrid == _currencyId) ? _currencySymbol : "")
		       .arg(pamount),
		    QMessageBox::Yes | QMessageBox::Default,
		    QMessageBox::No  | QMessageBox::Escape ) == QMessageBox::No)
    {
      _errorMsg = errorMsg(20);
      return 20;
    }
  }

  returnVal = doAuthorize(pccardid, pcvv, pamount, pcurrid, porder, porder, pccpayid);
  if (returnVal > 0)
    _errorMsg = CCPOSTPROC_WARNING.arg(_errorMsg);
  else if (returnVal == 0)
  {
    XSqlQuery cashq;
    if (preftype == "cashrcpt")
    {
      if (prefid <= 0)
      {
	cashq.exec("SELECT NEXTVAL('cashrcpt_cashrcpt_id_seq') AS cashrcpt_id;");
	if (cashq.first())
	  prefid = cashq.value("cashrcpt_id").toInt();
	else if (q.lastError().type() != QSqlError::None)
	{
	  _errorMsg = cashq.lastError().databaseText();
	  // TODO: log an event?
	  return 1;
	}

	cashq.prepare("INSERT INTO cashrcpt (cashrcpt_id,"
		  "  cashrcpt_cust_id, cashrcpt_amount, cashrcpt_curr_id,"
		  "  cashrcpt_fundstype, cashrcpt_docnumber,"
		  "  cashrcpt_bankaccnt_id, cashrcpt_notes, cashrcpt_distdate) "
		  "SELECT :cashrcptid,"
		  "       ccpay_cust_id, :amount, :curr_id,"
		  "       ccard_type, ccpay_yp_r_ordernum,"
		  "       :bankaccnt_id, :notes, current_date"
		  "  FROM ccpay, ccard "
		  "WHERE (ccpay_ccard_id=ccard_id);");
      }
      else
	cashq.prepare( "UPDATE cashrcpt "
		       "SET cashrcpt_cust_id=ccard_cust_id,"
		       "    cashrcpt_amount=:amount,"
		       "    cashrcpt_fundstype=ccard_type,"
		       "    cashrcpt_docnumber=:docnum,"
		       "    cashrcpt_bankaccnt_id=:bankaccnt_id,"
		       "    cashrcpt_distdate=CURRENT_DATE,"
		       "    cashrcpt_notes=:notes, "
		       "    cashrcpt_curr_id=:curr_id "
		       "FROM ccard "
		       "WHERE ((cashrcpt_id=:cashrcpt_id)"
		       "  AND  (ccard_id=:ccardid));" );

      cashq.bindValue(":cashrcptid",   prefid);
      cashq.bindValue(":ccardid",      pccardid);
      cashq.bindValue(":amount",       pamount);
      cashq.bindValue(":curr_id",      pcurrid);
      cashq.bindValue(":bankaccnt_id", _metrics->value("CCDefaultBank").toInt());
      cashq.bindValue(":notes",        "Credit Card Charge");
      cashq.exec();
      if (cashq.lastError().type() != QSqlError::None)
      {
	_errorMsg = cashq.lastError().databaseText();
	// TODO: log an event?
	returnVal = 1;
      }
    }
    else if (preftype == "cohead")
    {
      cashq.prepare("INSERT INTO payco VALUES"
		    " (:payco_ccpay_id, :payco_cohead_id,"
		    "  :payco_amount, :payco_curr_id);");
      cashq.bindValue(":payco_ccpay_id",  pccpayid);
      cashq.bindValue(":payco_cohead_id", prefid);
      cashq.bindValue(":payco_amount",    pamount);
      cashq.bindValue(":payco_curr_id",   pcurrid);
      cashq.exec();
      if (cashq.lastError().type() != QSqlError::NoError)
      {
	_errorMsg = cashq.lastError().databaseText();
	// TODO: log an event?
	returnVal = 1;
      }
    }
  }

  return returnVal;
}

int CreditCardProcessor::charge(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, QString preftype, int &prefid)
{
  if (DEBUG)
    qDebug("CCP:charge(%d, %d, %f, %d, %s, %s, %d, %s)",
	   pccardid, pcvv, pamount, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid,
	   preftype.toAscii().data());
  _errorMsg = "";

  if (pamount <= 0)
  {
    _errorMsg = errorMsg(-20);
    return -20;
  }

  if (preftype == "cohead" && prefid < 0)
  {
    _errorMsg = errorMsg(-40).arg(preftype).arg(prefid);
    return -40;
  }

  QString ccard_x;
  int returnVal = checkCreditCard(pccardid, pcvv, ccard_x);
  if (returnVal < 0)
    return returnVal;

  if (_metrics->value("CCConfirmTrans") == "A" ||
      _metrics->value("CCConfirmTrans") == "B")
  {
    if (QMessageBox::question(0, tr("Confirm Charge of Credit Card Purchase"),
	      tr("You must confirm that you wish to charge credit card %1 "
		 "in the amount of %2 %3. Would you like to charge now?")
		 .arg(ccard_x)
		 .arg((pcurrid == _currencyId) ? _currencySymbol : "")
		 .arg(pamount),
	      QMessageBox::Yes | QMessageBox::Default,
	      QMessageBox::No  | QMessageBox::Escape ) == QMessageBox::No)
    {
      _errorMsg = errorMsg(40);
      return 40;
    }
  }

  returnVal = doCharge(pccardid, pcvv, pamount, pcurrid, pneworder, preforder, pccpayid);
  if (returnVal > 0)
    _errorMsg = CCPOSTPROC_WARNING.arg(_errorMsg);
  else if (returnVal == 0) // TODO: move this logic to postCCCashReceipt
  {
    XSqlQuery cashq;
    if (preftype == "cashrcpt")
    {
      if (prefid <= 0)
      {
	cashq.exec("SELECT NEXTVAL('cashrcpt_cashrcpt_id_seq') AS cashrcpt_id;");
	if (cashq.first())
	  prefid = cashq.value("cashrcpt_id").toInt();
	else if (q.lastError().type() != QSqlError::None)
	{
	  _errorMsg = cashq.lastError().databaseText();
	  // TODO: log an event?
	  return 1;
	}

	cashq.prepare("INSERT INTO cashrcpt (cashrcpt_id,"
		  "  cashrcpt_cust_id, cashrcpt_amount, cashrcpt_curr_id,"
		  "  cashrcpt_fundstype, cashrcpt_docnumber,"
		  "  cashrcpt_bankaccnt_id, cashrcpt_notes, cashrcpt_distdate) "
		  "SELECT :cashrcptid,"
		  "       ccpay_cust_id, :amount, :curr_id,"
		  "       ccard_type, ccpay_yp_r_ordernum,"
		  "       :bankaccnt_id, :notes, current_date"
		  "  FROM ccpay, ccard "
		  "WHERE (ccpay_ccard_id=ccard_id);");
      }
      else
	cashq.prepare( "UPDATE cashrcpt "
		       "SET cashrcpt_cust_id=ccard_cust_id,"
		       "    cashrcpt_amount=:amount,"
		       "    cashrcpt_fundstype=ccard_type,"
		       "    cashrcpt_docnumber=:docnum,"
		       "    cashrcpt_bankaccnt_id=:bankaccnt_id,"
		       "    cashrcpt_distdate=CURRENT_DATE,"
		       "    cashrcpt_notes=:notes, "
		       "    cashrcpt_curr_id=:curr_id "
		       "FROM ccard "
		       "WHERE ((cashrcpt_id=:cashrcpt_id)"
		       "  AND  (ccard_id=:ccardid));" );

      cashq.bindValue(":cashrcptid",   prefid);
      cashq.bindValue(":ccardid",      pccardid);
      cashq.bindValue(":amount",       pamount);
      cashq.bindValue(":curr_id",      pcurrid);
      cashq.bindValue(":bankaccnt_id", _metrics->value("CCDefaultBank").toInt());
      cashq.bindValue(":notes",        "Credit Card Charge");
      cashq.exec();
      if (cashq.lastError().type() != QSqlError::None)
      {
	_errorMsg = cashq.lastError().databaseText();
	// TODO: log an event?
	returnVal = 1;
      }
    }
    else if (preftype == "cohead")
    {
      cashq.prepare("SELECT postCCcashReceipt(:ccpayid, :bankaccnt) AS cm_id;");
      cashq.bindValue(":ccpayid",   pccpayid); 
      cashq.bindValue(":bankaccnt", _metrics->value("CCDefaultBank").toInt()); 
      cashq.exec();
      if (cashq.first())
      {
	int cm_id = cashq.value("cm_id").toInt();
	if (cm_id < 0)
	{
	  _errorMsg = storedProcErrorLookup("postCCcashReceipt", cm_id);
	  returnVal = 3;
	}

	cashq.prepare("INSERT INTO payaropen VALUES"
		      " (:payco_ccpay_id, :payco_cohead_id,"
		      "  :payco_amount, :payco_curr_id);");
	cashq.bindValue(":payco_ccpay_id",  pccpayid);
	cashq.bindValue(":payco_cohead_id", cm_id);
	cashq.bindValue(":payco_amount",    pamount);
	cashq.bindValue(":payco_curr_id",   pcurrid);
	cashq.exec();
	if (cashq.lastError().type() != QSqlError::NoError)
	{
	  _errorMsg = cashq.lastError().databaseText();
	  // TODO: log an event?
	  returnVal = 1;
	}
	else
	{
	  cashq.prepare("INSERT INTO aropenco VALUES"
			" (:payco_ccpay_id, :payco_cohead_id,"
			"  :payco_amount, :payco_curr_id);");
	  cashq.bindValue(":payco_ccpay_id",  cm_id);
	  cashq.bindValue(":payco_cohead_id", prefid);
	  cashq.bindValue(":payco_amount",    pamount);
	  cashq.bindValue(":payco_curr_id",   pcurrid);
	  cashq.exec();
	  if (cashq.lastError().type() != QSqlError::NoError)
	  {
	    _errorMsg = cashq.lastError().databaseText();
	    // TODO: log an event?
	    returnVal = 1;
	  }
	}
      }
      else if (cashq.lastError().type() != QSqlError::NoError)
      {
	_errorMsg = cashq.lastError().databaseText();
	// TODO: log an event?
	returnVal = 1;
      }
    }
  }
  else if (preftype == "cohead") // record unsuccessful attempt against cohead
  {
    XSqlQuery cashq;

    cashq.prepare("INSERT INTO payco VALUES"
	      " (:payco_ccpay_id, :payco_cohead_id,"
	      "  :payco_amount, :payco_curr_id);");
    cashq.bindValue(":payco_ccpay_id",  pccpayid);
    cashq.bindValue(":payco_cohead_id", prefid);
    cashq.bindValue(":payco_amount",    pamount);
    cashq.bindValue(":payco_curr_id",   pcurrid);
    cashq.exec();
    if (cashq.lastError().type() != QSqlError::NoError)
    {
      _errorMsg = cashq.lastError().databaseText();
      // TODO: log an event?
      returnVal = 1;
    }
  }

  return returnVal;
}

int CreditCardProcessor::chargePreauthorized(const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid)
{
  if (DEBUG)
    qDebug("CCP:chargePreauthorized(%d, %f, %d, %s, %s, %d)",
	   pcvv, pamount, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid);
  _errorMsg = "";

  int ccValidDays = _metrics->value("CCValidDays").toInt();
  if(ccValidDays < 1)
    ccValidDays = 7;

  if (pamount <= 0)
  {
    _errorMsg = errorMsg(-20);
    return -20;
  }

  if (pccpayid < 0)
  {
    _errorMsg = errorMsg(-32);
    return -32;
  }

  q.prepare("SELECT ccpay_amount, ccpay_curr_id, ccpay_order_number,"
	    "       ccpay_ccard_id,"
	    "       currToCurr(ccpay_curr_id, :curr_id, ccpay_amount,"
	    "                  CURRENT_DATE) AS ccpay_amount_converted "
	    "FROM ccpay "
	    "WHERE ((ccpay_status = 'A')"
	    "  AND  (date_part('day', CURRENT_TIMESTAMP - "
	    "                         ccpay_transaction_datetime) < "
	    "        :ccValidDays)"
	    "  AND  (ccpay_id=:id));");
  q.bindValue(":id", pccpayid);
  q.bindValue(":curr_id",     pcurrid);
  q.bindValue(":ccValidDays", ccValidDays);
  q.exec();
  if (q.first())
  {
    if (pamount > q.value("ccpay_amount_converted").toDouble())
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

  int ccardid = q.value("ccpay_ccard_id").toInt();
  preforder = q.value("ccpay_order_number").toString();

  QString ccard_x;
  int returnVal = checkCreditCard(ccardid, pcvv, ccard_x);
  if (returnVal < 0)
    return returnVal;

  if (_metrics->value("CCConfirmTrans") == "A" ||
      _metrics->value("CCConfirmTrans") == "B")
  {
    if (QMessageBox::question(0,
	      tr("Confirm Post-authorization of Credit Card Purchase"),
              tr("You must confirm that you wish to post-authorize process "
                 "a charge to credit card %1 in the amount of %2 %3. "
		 "Would you like to post-authorize now?")
		 .arg(ccard_x)
                 .arg(_currencySymbol)
                 .arg(pamount),
              QMessageBox::Yes | QMessageBox::Default,
              QMessageBox::No  | QMessageBox::Escape ) == QMessageBox::No)
    {
      _errorMsg = errorMsg(30);
      return 30;
    }
  }

  returnVal = doChargePreauthorized(ccardid, pcvv, pamount, pcurrid, pneworder, preforder, pccpayid);
  if (returnVal > 0)
    _errorMsg = CCPOSTPROC_WARNING.arg(_errorMsg);

  else if (returnVal == 0)
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
  if (DEBUG)
    qDebug("CCP:checkConfiguration()");
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


int CreditCardProcessor::credit(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid)
{
  if (DEBUG)
    qDebug("CCP:credit(%d, %d, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid);
  _errorMsg = "";

  QString ccard_x;
  int returnVal = checkCreditCard(pccardid, pcvv,  ccard_x);
  if (returnVal < 0)
    return returnVal;

  if (_metrics->value("CCConfirmTrans") == "R" ||
      _metrics->value("CCConfirmTrans") == "B")
  {
    if (QMessageBox::question(0,
	      tr("Confirm Credit Card Credit"),
              tr("You must confirm that you wish to credit card %1 in the "
                 "amount of %2 %3. Would you like to credit this amount now?")
		 .arg(ccard_x)
                 .arg(currSymbol(pcurrid))
                 .arg(pamount),
              QMessageBox::Yes | QMessageBox::Default,
              QMessageBox::No  | QMessageBox::Escape ) == QMessageBox::No)
      {
	_errorMsg = errorMsg(50);
	return 50;
      }
  }

  if (pccpayid > 0)
  {
    int oldccpayid = pccpayid;

    XSqlQuery ccq;
    ccq.exec("SELECT NEXTVAL('ccpay_ccpay_id_seq') AS ccpay_id;");
    if (ccq.first())
      pccpayid = ccq.value("ccpay_id").toInt();
    else if (ccq.lastError().type() != QSqlError::None)
    {
      _errorMsg = ccq.lastError().databaseText();
      return -1;
    }
    else // no rows found is fatal because we haven't processed the credit yet
    {
      _errorMsg = errorMsg(2);
      return -1;
    }

    int next_seq = -1;
    ccq.prepare("SELECT MAX(COALESCE(ccpay_order_number_seq, -1)) + 1"
		"       AS next_seq "
		"  FROM ccpay "
		" WHERE (ccpay_order_number=:ccpay_order_number);");
    ccq.bindValue(":ccpay_order_number", preforder.toInt());
    ccq.exec();
    if (ccq.first())
      next_seq = ccq.value("next_seq").toInt();
    else if (ccq.lastError().type() != QSqlError::None)
    {
      _errorMsg = ccq.lastError().databaseText();
      return -1;
    }

    ccq.prepare( "INSERT INTO ccpay ("
		 "    ccpay_id, ccpay_ccard_id, ccpay_cust_id,"
		 "    ccpay_auth_charge, ccpay_auth,"
		 "    ccpay_amount,"
		 "    ccpay_curr_id, ccpay_type, ccpay_status,"
		 "    ccpay_order_number, ccpay_order_number_seq"
		 ") SELECT "
		 "    :newccpayid, ccpay_ccard_id, ccpay_cust_id,"
		 "    ccpay_auth_charge, ccpay_auth,"
		 "    currToCurr(:currid, :ccpcurrid, :amount, CURRENT_DATE),"
		 "    :ccpcurrid, 'R', 'X',"
		 "    ccpay_order_number, :nextseq "
		 "FROM ccpay "
		 "WHERE (ccpay_id=:oldccpayid);");
    ccq.bindValue(":newccpayid", pccpayid);
    ccq.bindValue(":currid",     pcurrid);
    ccq.bindValue(":ccpcurrid",  _currencyId);
    ccq.bindValue(":amount",     pamount);
    ccq.bindValue(":nextseq",    next_seq);
    ccq.bindValue(":oldccpayid", oldccpayid);
    ccq.exec();
    if (ccq.lastError().type() != QSqlError::NoError)
    {
      _errorMsg = ccq.lastError().databaseText();
      return -1;
    }
  }

  returnVal = doCredit(pccardid, pcvv, pamount, pcurrid, pneworder, preforder, pccpayid);
  if (returnVal < 0)
    return returnVal;
  else if (returnVal > 0)
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
	_errorMsg = "<p>" +
		    CCPOSTPROC_WARNING.arg(storedProcErrorLookup("postCCCredit",
								 result));
	returnVal = 1;
      }
    }
    else if (cq.lastError().type() != QSqlError::NoError)
    {
      _errorMsg = CCPOSTPROC_WARNING.arg(cq.lastError().databaseText());
      returnVal = 1;
    }
  }

  return returnVal;
}

int CreditCardProcessor::voidPrevious(const int &pccpayid)
{
  if (DEBUG)
    qDebug("CCP:voidPrevious(%d)", pccpayid);
  _errorMsg = "";
  return doVoidPrevious(pccpayid);
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

int CreditCardProcessor::checkCreditCard(int pccid, int pcvv, QString &pccard_x)
{
  if (DEBUG)
    qDebug("CCP:checkCreditCard(%d, %d)", pccid, pcvv);
  _errorMsg = "";

  QString key = omfgThis->_key;

  if(key.isEmpty())
  {
    _errorMsg = errorMsg(-5);
    return -5;
  }

  q.prepare( "SELECT ccard_active, ccard_cust_id, "
             "       formatbytea(decrypt(setbytea(ccard_month_expired),"
	     "               setbytea(:key), 'bf')) AS ccard_month_expired,"
             "       formatbytea(decrypt(setbytea(ccard_year_expired),"
	     "               setbytea(:key), 'bf')) AS ccard_year_expired,"
             "       formatccnumber(decrypt(setbytea(ccard_number),"
	     "               setbytea(:key), 'bf')) AS ccard_number_x,"
             "       ccard_type "
             "FROM ccard "
             "WHERE (ccard_id=:ccardid);");
  q.bindValue(":ccardid", pccid);
  q.bindValue(":key",     key);
  q.exec();
  if (q.first())
  {
    pccard_x = q.value("ccard_number_x").toString();

    if (!q.value("ccard_active").toBool())
    {
      _errorMsg = errorMsg(-10).arg(pccard_x);
      return -10;
    }

    if (QDate(q.value("ccard_year_expired").toInt(),
	      q.value("ccard_month_expired").toInt(),
	      1) < QDate::currentDate())
    {
      XSqlQuery xpq;
      xpq.prepare("SELECT expireCreditCard(:custid, setbytea(:key)) AS result;");
      xpq.bindValue(":custid", q.value("ccard_cust_id"));
      xpq.bindValue(":key", omfgThis->_key);
      xpq.exec();
      // ignore errors from expirecreditcard()
      _errorMsg = errorMsg(-11).arg(pccard_x);
      return -11;
    }
  }

  if (pcvv == -1)
  {
    if (QMessageBox::question(0,
	      tr("Confirm No CVV Code"),
              tr("<p>You must confirm that you wish to proceed "
                 "without a CVV code. Would you like to continue?"),
              QMessageBox::Yes | QMessageBox::Default,
              QMessageBox::No  | QMessageBox::Escape ) == QMessageBox::No)
    {
      _errorMsg = errorMsg(99);
      return 99;
    }
  }
  else if (pcvv == -2)
    ; // the caller knows that this transaction doesn't have cvv available
  else if (pcvv > 9999) // YP docs are not consistent - 000-999 or 3-4 digits?
  {
    _errorMsg = errorMsg(-99);
    return -99;
  }

  return 0;
}

int CreditCardProcessor::doAuthorize(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid)
{
  if (DEBUG)
    qDebug("CCP:doAuthorize(%d, %d, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid);
  _errorMsg = errorMsg(-19).arg("doAuthorize");
  return -19;
}

int CreditCardProcessor::doCharge(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid)
{
  if (DEBUG)
    qDebug("CCP:doCharge(%d, %d, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid);
  _errorMsg = errorMsg(-19).arg("doCharge");
  return -19;
}

int CreditCardProcessor::doChargePreauthorized(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid)
{
  if (DEBUG)
    qDebug("CCP:doChargePreauthorized(%d, %d, %f, %d, %s, %s, %d)",
	    pccardid, pcvv, pamount, pcurrid,
	    pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid);
  _errorMsg = errorMsg(-19).arg("doChargePreauthorized");
  return -19;
}

int CreditCardProcessor::doCheckConfiguration()
{
  if (DEBUG)
    qDebug("CCP:doCheckConfiguration()");
  return 0;	// assume that subclasses will override IFF they need to
}

int CreditCardProcessor::doCredit(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid)
{
  if (DEBUG)
    qDebug("CCP:doCredit(%d, %d, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid);
  _errorMsg = errorMsg(-19).arg("doCredit");
  return -19;
}

int CreditCardProcessor::doVoidPrevious(const int pccpayid)
{
  if (DEBUG)
    qDebug("CCP:doVoidPrevious(%d)", pccpayid);
  _errorMsg = errorMsg(-19).arg("doVoidPrevious");
  return -19;
}

int CreditCardProcessor::processXML(const QDomDocument &prequest,
				     QDomDocument &presponse)
{
  if (DEBUG)
    qDebug("CCP:processXML(input, output) with input:\n%s",
	   prequest.toString().toAscii().data());
  _errorMsg = "";
  QString transactionString = prequest.toString();

  // TODO: find a better place to save this
  if (isTest())
    _metrics->set("CCOrder", transactionString);

  // TODO: why have a hard-coded path to curl?
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
    curl_args.append(_metrics->value("CCProxyServer") + ":" +
		     _metrics->value("CCProxyPort"));
    curl_args.append( "-U" );
    curl_args.append(_metricsenc->value("CCProxyLogin") + ":" +
		     _metricsenc->value("CCPassword"));
  }

  QString curlCmd = curl_path + ((DEBUG) ? (" " + curl_args.join(" ")) : "");

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
  /* TODO: consider changing to the original implementation:
	      start the proc in a separate thread
	      capture the output as it's generated
	      while (proc->isRunning())
		qApp->processEvents();
   */
  proc.start(curl_path, curl_args);
  if ( !proc.waitForStarted() )
  {
    QApplication::restoreOverrideCursor();
    _errorMsg = errorMsg(-18)
		  .arg(curlCmd)
		  .arg("")
		  .arg(QString(proc.readAllStandardError()));
    return -18;
  }

  if (! proc.waitForFinished())
  {
    QApplication::restoreOverrideCursor();
    _errorMsg = errorMsg(-18)
		  .arg(curlCmd)
		  .arg("")
		  .arg(QString(proc.readAllStandardError()));
    return -18;
  }

  QApplication::restoreOverrideCursor();

  if (proc.exitStatus() != QProcess::NormalExit)
  {
    _errorMsg = errorMsg(-18)
		  .arg(curlCmd)
		  .arg("")
		  .arg(QString(proc.readAllStandardError()));
    return -18;
  }

  if (proc.exitCode() != 0)
  {
    _errorMsg = errorMsg(-18)
		  .arg(curlCmd)
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
