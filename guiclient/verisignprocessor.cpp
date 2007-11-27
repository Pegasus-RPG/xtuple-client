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

#include <QSqlError>

#include <currcluster.h>

#include "OpenMFGGUIClient.h"
#include "verisignprocessor.h"

VerisignProcessor::VerisignProcessor() : CreditCardProcessor()
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

int VerisignProcessor::doCheckConfiguration()
{
  _errorMsg = tr("Verisign is not yet supported as a Credit Card Processor.");
  return false;

  /* even though this isn't reachable code, hold everything we already know
     about Verisign in here so we can remove it from elsewhere in the app.
  */

  if ((_metrics->value("CCServer") != "payflow.verisign.com") &&
      (_metrics->value("CCServer") != "test-payflow.verisign.com"))
  {
    _errorMsg = errorMsg(-15)
		  .arg(_metrics->value("CCServer"))
		  .arg(_metrics->value("CCCompany"));
    return -15;

  }

  if (_metrics->value("CCPort").toInt() != 443)
  {
    _errorMsg = errorMsg(-16)
		  .arg(_metrics->value("CCPort"))
		  .arg(_metrics->value("CCCompany"));
    return -16;
  }

  return 0;
}

bool VerisignProcessor::isLive()
{
  return (!_metrics->boolean("CCTest") &&
	   _metrics->value("CCServer") == "payflow.verisign.com");
}

bool VerisignProcessor::isTest()
{
  return (_metrics->boolean("CCTest") &&
	  _metrics->value("CCServer") == "test-payflow.verisign.com");
}
