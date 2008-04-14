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
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
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


#include "format.h"

#include <qsqlquery.h>

// Default values used if there is any error
// communicating with the database
#define _moneyScale		2	// default 2 decimal places
#define _costExtraScale		2	// => 2 more than _moneyScale
#define	_weightFormat      "%0.2f"
#define _qtyFormat         "%0.2f"
#define _qtyPerFormat      "%0.6f"
#define _salesPriceExtraScale	2	// => 2 more than _moneyScale
#define _purchPriceExtraScale	2	// => 2 more than _moneyScale
#define _uomRatioFormat    "%0.6f"
#define _percentFormat     "%0.2f"

QString formatNumber(double value, int decimals)
{
   return QString().sprintf("%0.*f", decimals, value);
}

/*
  convert a double to a string representing a currency value.
  different currencies have different rounding conventions, so we need
  the currency id to find the right rounding rules.
  we need a few decimal places for some data because some monetary values,
  primarily unit costs (i.e. standard costs), are stored with extra precision.

  ToDo: implement use of curr_id and extraDecimals in the formatMoney stored
	procedure, then change formatMoney here to use them
	and address more ToDo comments below
*/

QString formatMoney(double value, int /* curr_id */, int extraDecimals)
{
  return QString().sprintf("%0.*f", MONEYSCALE + extraDecimals, value);
}

QString formatCost(double value, int curr_id)
{
  return formatMoney(value, curr_id, COSTEXTRASCALE);
}

QString formatWeight(double value)
{
  return QString().sprintf("%0.*f", WEIGHTSCALE, value);
}

QString formatQty(double value)
{
  return QString().sprintf("%0.*f", QTYSCALE, value);
}

QString formatQtyPer(double value)
{
  return QString().sprintf("%0.*f", QTYPERSCALE, value);
}

QString formatSalesPrice(double value, int curr_id)
{
  return formatMoney(value, curr_id, SALESPRICEEXTRASCALE);
}

QString formatPurchPrice(double value, int curr_id)
{
  return formatMoney(value, curr_id, PURCHPRICEEXTRASCALE);
}

QString formatUOMRatio(double value)
{
  return QString().sprintf("%0.*f", UOMRATIOSCALE, value);
}

QString formatPercent(double value)
{
  return QString().sprintf("%0.*f", PERCENTSCALE, (value * 100.0));
}    
