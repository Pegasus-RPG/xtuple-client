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

#ifndef CREDITMEMOITEM_H
#define CREDITMEMOITEM_H

#include "OpenMFGGUIClient.h"
#include <QDialog>
#include <parameter.h>

#include "taxCache.h"
#include "ui_creditMemoItem.h"

class creditMemoItem : public QDialog, public Ui::creditMemoItem
{
    Q_OBJECT

public:
    creditMemoItem(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~creditMemoItem();

public slots:
    virtual SetResponse set(const ParameterList & pParams );
    virtual void sPopulateItemInfo();
    virtual void populate();
    virtual void sCalculateDiscountPrcnt();
    virtual void sCalculateFromDiscount();
    virtual void sListPrices();
    virtual void sLookupTax();
    virtual void sLookupTaxCode();
    virtual void sPriceGroup();
    virtual void sTaxDetail();

protected slots:
    virtual void languageChange();

    virtual void sSave();
    virtual void sCalculateExtendedPrice();

private:
    int _mode;
    int _cmitemid;
    int _cmheadid;
    int _custid;
    int _invoiceNumber;
    int _shiptoid;
    double _priceRatio;
    double _listPriceCache;
    double _salePriceCache;
    double _qtyShippedCache;
    int		_taxauthid;
    taxCache	_taxCache;
};

#endif // CREDITMEMOITEM_H
