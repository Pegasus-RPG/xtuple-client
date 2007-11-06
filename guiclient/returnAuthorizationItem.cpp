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

#include "returnAuthorizationItem.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "priceList.h"
#include "taxDetail.h"

returnAuthorizationItem::returnAuthorizationItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
  setupUi(this);

#ifndef Q_WS_MAC
  _listPrices->setMaximumWidth(25);
#endif

  _mode = cNew;
  _raitemid = -1;
  _raheadid = -1;
  _custid = -1;
  _priceRatio = 1.0;
  _qtyAuthCache = 0.0;
  _shiptoid = -1;
  _taxauthid	= -1;
  _taxCache.clear();
  _qtyinvuomratio = 1.0;
  _priceinvuomratio = 1.0;
  _invuomid = -1;

  _qtyAuth->setValidator(omfgThis->qtyVal());
  _discountFromSale->setValidator(new QDoubleValidator(-9999, 100, 2, this));
  _taxType->setEnabled(_privleges->check("OverrideTax"));
  _taxCode->setEnabled(_privleges->check("OverrideTax"));
  
  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  } 

  //Remove lot/serial for now until we get to advanced warranty tracking
  _tab->removePage(_tab->page(4));
}

returnAuthorizationItem::~returnAuthorizationItem()
{
    // no need to delete child widgets, Qt does it all for us
}

void returnAuthorizationItem::languageChange()
{
    retranslateUi(this);
}

enum SetResponse returnAuthorizationItem::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("rahead_id", &valid);
  if (valid)
  {
    _raheadid = param.toInt();
    q.prepare("SELECT *,"
	      "       rahead_curr_id AS taxcurr, "
		  "       COALESCE(rahead_cohead_id,-1) AS cohead_id "
	      "FROM rahead "
	      "WHERE (rahead_id=:rahead_id);");
    q.bindValue(":rahead_id", _raheadid);
    q.exec();
    if (q.first())
    {
	  _authNumber->setText(q.value("rahead_number").toString());
      _taxauthid = q.value("rahead_taxauth_id").toInt();
      _tax->setId(q.value("taxcurr").toInt());
	  _rsnCode->setId(q.value("rahead_rsncode_id").toInt());
      _custid = q.value("rahead_cust_id").toInt();
	  _shiptoid = q.value("rahead_shipto_id").toInt();
	  _netUnitPrice->setId(q.value("rahead_curr_id").toInt());
	  _netUnitPrice->setEffective(q.value("rahead_authdate").toDate());
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  param = pParams.value("raitem_id", &valid);
  if (valid)
  {
    _raitemid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.prepare( "SELECT (COALESCE(MAX(raitem_linenumber), 0) + 1) AS n_linenumber "
                 "FROM raitem "
                 "WHERE (raitem_rahead_id=:rahead_id);" );
      q.bindValue(":rahead_id", _raheadid);
      q.exec();
      if (q.first())
        _lineNumber->setText(q.value("n_linenumber").toString());
      else if (q.lastError().type() == QSqlError::None)
      {
	    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	    return UndefinedError;
      }

      q.prepare( "SELECT rahead_disposition "
		         "FROM rahead "
                 "WHERE (rahead_id=:rahead_id);" );
      q.bindValue(":rahead_id", _raheadid);
      q.exec();
      if (q.first())
	  {

        if (q.value("rahead_disposition").toString() == "C")
	      _disposition->setCurrentItem(0);
	    else if (q.value("rahead_disposition").toString() == "R")
	      _disposition->setCurrentItem(1);
	    else if (q.value("rahead_disposition").toString() == "P")
	      _disposition->setCurrentItem(2);
	    else if (q.value("rahead_disposition").toString() == "S")
	      _disposition->setCurrentItem(3);
	  }
      else if (q.lastError().type() == QSqlError::None)
      {
	    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	    return UndefinedError;
      }

      connect(_discountFromSale, SIGNAL(lostFocus()), this, SLOT(sCalculateFromDiscount()));
      connect(_item, SIGNAL(valid(bool)), _listPrices, SLOT(setEnabled(bool)));

	  _orderNumber->hide();
	  _orderNumberLit->hide();
	  _orderLineNumber->hide();
	  _orderLineNumberLit->hide();
	  _qtySold->hide();
	  _qtySoldLit->hide();
      _discountFromSalePrcntLit->hide();
      _discountFromSale->hide();
	  _salePrice->hide();
	  _salePriceLit->hide();
      _item->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);
      _qtyAuth->setFocus();

      connect(_discountFromSale, SIGNAL(lostFocus()), this, SLOT(sCalculateFromDiscount()));
      connect(_item, SIGNAL(valid(bool)), _listPrices, SLOT(setEnabled(bool)));
 
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);
      _qtyAuth->setEnabled(FALSE);
      _netUnitPrice->setEnabled(FALSE);
      _discountFromSale->setEnabled(FALSE);
      _notes->setReadOnly(TRUE);
      _taxType->setEnabled(FALSE);
      _taxCode->setEnabled(FALSE);
      _rsnCode->setEnabled(FALSE);

      _save->hide();
      _close->setText(tr("&Close"));
      _close->setFocus();
    }
  }
 
  _item->setQuery( QString( "SELECT DISTINCT item_id, item_number, item_descrip1, item_descrip2,"
                              "                uom_name, item_type, item_config "
                              "FROM item, itemsite, uom "
                              "WHERE ( (itemsite_item_id=item_id)"
                              " AND (item_inv_uom_id=uom_id)"
                              " AND (itemsite_active)"
                              " AND (item_active)"
                              " AND (customerCanPurchase(item_id, %1, %2)) ) "
                              "ORDER BY item_number;" )
                     .arg(_custid).arg(_shiptoid) );

  connectAll();

  return NoError; 
}

void returnAuthorizationItem::sSave()
{ 
  char *dispositionTypes[] = { "C", "R", "P", "V", "S" };
  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('raitem_raitem_id_seq') AS _raitem_id");
    if (q.first())
      _raitemid  = q.value("_raitem_id").toInt();
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO raitem "
               "( raitem_id, raitem_rahead_id, raitem_linenumber, raitem_itemsite_id,"
               "  raitem_disposition, raitem_qtyauthorized, "
               "  raitem_qty_uom_id, raitem_qty_invuomratio,"
               "  raitem_price_uom_id, raitem_price_invuomratio,"
               "  raitem_unitprice, raitem_tax_id,"
               "  raitem_notes, raitem_rsncode_id, raitem_cos_accnt_id) "
			   "SELECT :raitem_id, :rahead_id, :raitem_linenumber, itemsite_id,"
			   "       :raitem_disposition, :raitem_qtyauthorized,"
               "       :qty_uom_id, :qty_invuomratio,"
               "       :price_uom_id, :price_invuomratio,"
               "       :raitem_unitprice, :raitem_tax_id,"
			   "       :raitem_notes, :raitem_rsncode_id, :raitem_cos_accnt_id "
               "FROM itemsite "
               "WHERE ( (itemsite_item_id=:item_id)"
               " AND (itemsite_warehous_id=:warehous_id) );" );
  }
  else
    q.prepare( "UPDATE raitem "
	           "SET raitem_disposition=:raitem_disposition, "
			   "    raitem_qtyauthorized=:raitem_qtyauthorized, "
               "    raitem_qty_uom_id=:qty_uom_id,"
               "    raitem_qty_invuomratio=:qty_invuomratio,"
               "    raitem_price_uom_id=:price_uom_id,"
               "    raitem_price_invuomratio=:price_invuomratio,"
               "    raitem_unitprice=:raitem_unitprice,"
    	       "    raitem_tax_id=:raitem_tax_id,"
	           "    raitem_notes=:raitem_notes,"
               "    raitem_rsncode_id=:raitem_rsncode_id, "
			   "    raitem_cos_accnt_id=:raitem_cos_accnt_id "
               "WHERE (raitem_id=:raitem_id);" );

  q.bindValue(":raitem_id", _raitemid);
  q.bindValue(":rahead_id", _raheadid);
  q.bindValue(":raitem_linenumber", _lineNumber->text().toInt());
  q.bindValue(":raitem_qtyauthorized", _qtyAuth->toDouble());
  q.bindValue(":raitem_disposition", QString(dispositionTypes[_disposition->currentItem()]));
  q.bindValue(":qty_uom_id", _qtyUOM->id());
  q.bindValue(":qty_invuomratio", _qtyinvuomratio);
  q.bindValue(":price_uom_id", _pricingUOM->id());
  q.bindValue(":price_invuomratio", _priceinvuomratio);
  q.bindValue(":raitem_unitprice", _netUnitPrice->localValue());
  if (_taxCode->isValid())
    q.bindValue(":raitem_tax_id",	_taxCode->id());
  q.bindValue(":raitem_notes", _notes->text());
  if (_rsnCode->isValid())
    q.bindValue(":raitem_rsncode_id", _rsnCode->id());
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehouse->id());
  if (_altcosAccntid->id() != -1)
    q.bindValue(":raitem_cos_accnt_id", _altcosAccntid->id()); 
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_raitemid); 
}

void returnAuthorizationItem::sPopulateItemInfo()
{
  XSqlQuery uom;
  uom.prepare("SELECT uom_id, uom_name"
              "  FROM item"
              "  JOIN uom ON (item_inv_uom_id=uom_id)"
              " WHERE(item_id=:item_id)"
              " UNION "
              "SELECT uom_id, uom_name"
              "  FROM item"
              "  JOIN itemuomconv ON (itemuomconv_item_id=item_id)"
              "  JOIN uom ON (itemuomconv_to_uom_id=uom_id)"
              " WHERE((itemuomconv_from_uom_id=item_inv_uom_id)"
              "   AND (item_id=:item_id))"
              " UNION "
              "SELECT uom_id, uom_name"
              "  FROM item"
              "  JOIN itemuomconv ON (itemuomconv_item_id=item_id)"
              "  JOIN uom ON (itemuomconv_from_uom_id=uom_id)"
              " WHERE((itemuomconv_to_uom_id=item_inv_uom_id)"
              "   AND (item_id=:item_id))"
              " ORDER BY uom_name;");
  uom.bindValue(":item_id", _item->id());
  uom.exec();
  _qtyUOM->populate(uom);
  _pricingUOM->populate(uom);

  XSqlQuery item;
  item.prepare( "SELECT item_inv_uom_id, item_price_uom_id,"
                "       iteminvpricerat(item_id) AS iteminvpricerat, formatUOMRatio(iteminvpricerat(item_id)) AS f_invpricerat,"
                "       item_listprice, "
                "       stdCost(item_id) AS f_cost,"
		        "       getItemTaxType(item_id, :taxauth) AS taxtype_id "
                "  FROM item"
                " WHERE (item_id=:item_id);" );
  item.bindValue(":item_id", _item->id());
  item.exec();
  if (item.first())
  {
    _priceRatio = item.value("iteminvpricerat").toDouble();
    _qtyUOM->setId(item.value("item_inv_uom_id").toInt());
    _pricingUOM->setId(item.value("item_price_uom_id").toInt());
    _priceinvuomratio = item.value("iteminvpricerat").toDouble();
    _qtyinvuomratio = 1.0;
    //_ratio->setText(item.value("f_invpricerat").toString());
    _invuomid = item.value("item_inv_uom_id").toInt();
    // {_listPrice,_unitCost}->setBaseValue() because they're stored in base
    _listPrice->setBaseValue(item.value("item_listprice").toDouble());
    _unitCost->setBaseValue(item.value("f_cost").toDouble());
    _taxType->setId(item.value("taxtype_id").toInt());
  }
  else if (item.lastError().type() != QSqlError::None)
  {
    systemError(this, item.lastError().databaseText(), __FILE__, __LINE__);
    return;
  } 

  /*
  if (_coitem != -1)
  {
    XSqlQuery raitem;
    raitem.prepare( "SELECT coitem_warehous_id,"
                    "       coietm_qty_uom_id, coitem_qty_invuomratio,"
                    "       coitem_price_uom_id, coitem_price_invuomratio,"
                    "       coitem_shipped * coitem_qty_invuomratio AS f_sold,"
                    "       currToCurr(invchead_curr_id, :curr_id, "
		            "                  coitem_price / coitem_price_invuomratio, invchead_invcdate) AS coitem_price_local "
                    "FROM invchead, coitem "
                    "WHERE ( (coitem_invchead_id=invchead_id)"
                    " AND (invchead_invcnumber=:invoiceNumber)"
                    " AND (coitem_item_id=:item_id) ) "
                    "LIMIT 1;" );
    raitem.bindValue(":coitem", _coitemid);
    raitem.bindValue(":item_id", _item->id());
    raitem.bindValue(":curr_id", _netUnitPrice->id());
    raitem.exec();
    if (raitem.first())
    {
      _qtyUOM->setId(raitem.value("coitem_qty_uom_id").toInt());
	  _qtyUOM->setEnabled(FALSE);
      _pricingUOM->setId(raitem.value("coitem_price_uom_id").toInt());
	  _pricingUOM->setEnabled(FALSE);
      _priceinvuomratio = raitem.value("coitem_price_invuomratio").toDouble();
    //  _ratio->setText(formatUOMRatio(_priceinvuomratio));
      _salePrice->setLocalValue(raitem.value("coitem_price_local").toDouble() * _priceinvuomratio);

      if (_mode == cNew)
        _netUnitPrice->setLocalValue(raitem.value("coitem_price_local").toDouble() * _priceinvuomratio);

      _warehouse->setId(raitem.value("coitem_warehous_id").toInt());
      _qtySoldCache = raitem.value("f_billed").toDouble();
      _qtySold->setText(formatQty(raitem.value("f_sold").toDouble() / _qtyinvuomratio));
    }
    else if (raitem.lastError().type() != QSqlError::None)
    {
      systemError(this, raitem.lastError().databaseText(), __FILE__, __LINE__);
      _salePrice->clear();
      return;
    } 
  }*/
}

void returnAuthorizationItem::populate()
{
  XSqlQuery raitem;
  raitem.prepare("SELECT rahead_number, raitem.*,cohead_number,coitem_linenumber, "
	             "       COALESCE(raitem_coitem_id,-1) AS ra_coitem_id, coitem_price, "
				 "       formatQty(coitem_qtyshipped) AS qtysold,"   
                 "       formatQty(raitem_qtyauthorized) AS qtyauth,"
				 "       formatQty(raitem_qtyreceived) AS qtyrcvd,"
				 "       formatQty(raitem_qtyshipped) AS qtyshipd,"
		         "       rahead_taxauth_id,"
		         "       rahead_curr_id AS taxcurr "
                 "FROM raitem "
				 "  LEFT OUTER JOIN coitem ON (raitem_coitem_id=coitem_id) "
				 "  LEFT OUTER JOIN cohead ON (coitem_cohead_id=cohead_id),"
				 "  rahead "
                 "WHERE ((raitem_rahead_id=rahead_id)"
		 "  AND  (raitem_id=:raitem_id));" );
  raitem.bindValue(":raitem_id", _raitemid);
  raitem.exec();
  if (raitem.first())
  {
    _authNumber->setText(raitem.value("rahead_number").toString());
	if (raitem.value("cohead_number").toInt() > 0)
	{
      _orderNumber->setText(raitem.value("cohead_number").toString());
	  _orderLineNumber->setText(raitem.value("coitem_linenumber").toString());
	}
	else
	{
	  _orderNumberLit->hide();
	  _orderLineNumberLit->hide();
	}
	_salePrice->setLocalValue(raitem.value("coitem_price").toDouble());
    _netUnitPrice->setLocalValue(raitem.value("raitem_unitprice").toDouble());
    // do _item and _taxauth before other tax stuff because of signal cascade
    _taxauthid = raitem.value("rahead_taxauth_id").toInt();
    _tax->setId(raitem.value("taxcurr").toInt());
    _item->setItemsiteid(raitem.value("raitem_itemsite_id").toInt());
    _qtyUOM->setId(raitem.value("raitem_qty_uom_id").toInt());
    _pricingUOM->setId(raitem.value("raitem_price_uom_id").toInt());
    _lineNumber->setText(raitem.value("raitem_linenumber").toString());
    _qtyAuth->setText(raitem.value("qtyauth").toString());
    _qtyReceived->setText(raitem.value("qtyrcvd").toString());
    _qtyShipped->setText(raitem.value("qtyshipd").toString());
    _notes->setText(raitem.value("raitem_notes").toString());
    _taxCode->setId(raitem.value("raitem_tax_id").toInt());
    _taxType->setId(raitem.value("raitem_taxtype_id").toInt());
    _taxCache.setLinePct(raitem.value("raitem_tax_pcta").toDouble(),
			 raitem.value("raitem_tax_pctb").toDouble(),
			 raitem.value("raitem_tax_pctc").toDouble());
    _taxCache.setLine(raitem.value("raitem_tax_ratea").toDouble(),
		      raitem.value("raitem_tax_rateb").toDouble(),
		      raitem.value("raitem_tax_ratec").toDouble());
    _tax->setLocalValue(_taxCache.total());
    _rsnCode->setId(raitem.value("raitem_rsncode_id").toInt());
    if (raitem.value("raitem_disposition").toString() == "C")
	  _disposition->setCurrentItem(0);
	else if (raitem.value("raitem_disposition").toString() == "R")
	  _disposition->setCurrentItem(1);
	else if (raitem.value("raitem_disposition").toString() == "P")
	  _disposition->setCurrentItem(2);
	else if (raitem.value("raitem_disposition").toString() == "V")
	  _disposition->setCurrentItem(3);
    else if (raitem.value("raitem_disposition").toString() == "S")
	  _disposition->setCurrentItem(4);
    _altcosAccntid->setId(raitem.value("raitem_cos_accnt_id").toInt());


    _coitemid = raitem.value("ra_coitem_id").toInt();
	if (_coitemid != -1)
	{
	  _orderNumber->setText(raitem.value("cohead_number").toString());
	  _orderLineNumber->setText(raitem.value("coitem_linenumber").toString());
	  _qtySold->setText(raitem.value("qtysold").toString());
      _qtyUOM->setEnabled(FALSE);
	  _pricingUOM->setEnabled(FALSE);
      _priceinvuomratio = raitem.value("coitem_price_invuomratio").toDouble();
   //   _ratio->setText(formatUOMRatio(_priceinvuomratio));
      _salePrice->setLocalValue(raitem.value("coitem_price").toDouble());
	}
	else
	{
	  _orderNumber->hide();
	  _orderNumberLit->hide();
	  _orderLineNumber->hide();
	  _orderLineNumberLit->hide();
      _discountFromSalePrcntLit->hide();
      _discountFromSale->hide();
	  _qtySold->hide();
	  _qtySoldLit->hide();
	  _salePrice->hide();
	  _salePriceLit->hide();
	}
  }
  else if (raitem.lastError().type() != QSqlError::None)
  {
    systemError(this, raitem.lastError().databaseText(), __FILE__, __LINE__);
    return;
  } 
  sPopulateItemInfo();
  sPriceGroup();
  sQtyUOMChanged();
  sPriceUOMChanged();
  sCalculateDiscountPrcnt();
  sLookupTaxCode();
  sLookupTax();
}

void returnAuthorizationItem::sCalculateExtendedPrice()
{
  _extendedPrice->setLocalValue(((_qtyAuth->toDouble() * _qtyinvuomratio) / _priceinvuomratio) * _netUnitPrice->localValue());
}

void returnAuthorizationItem::sCalculateDiscountPrcnt()
{
  double unitPrice = _netUnitPrice->localValue();

  if (unitPrice == 0.0)
  {
    _discountFromList->setText("N/A");
    _discountFromSale->setText("N/A");
  }
  else
  {
    if (_listPrice->isZero())
      _discountFromList->setText("N/A");
    else
      _discountFromList->setText(formatPercent(1 - (unitPrice / _listPrice->localValue())) );

    if (_salePrice->isZero())
      _discountFromSale->setText("N/A");
    else
      _discountFromSale->setText(formatPercent(1 - (unitPrice / _salePrice->localValue())) );
  }
}

void returnAuthorizationItem::sCalculateFromDiscount()
{
  double discount = _discountFromSale->toDouble() / 100.0;

  if (_salePrice->isZero())
    _discountFromSale->setText(tr("N/A"));
  else
    _netUnitPrice->setLocalValue(_salePrice->localValue() - (_salePrice->localValue() * discount));
}

void returnAuthorizationItem::sPriceGroup()
{
  if (! omfgThis->singleCurrency())
    _priceGroup->setTitle(tr("In %1:").arg(_netUnitPrice->currAbbr())); 
}

void returnAuthorizationItem::sListPrices()
{
  q.prepare( "SELECT formatSalesPrice(currToCurr(ipshead_curr_id, :curr_id, ipsprice_price, :effective)) AS price"
             "       FROM ipsass, ipshead, ipsprice "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (ipsass_cust_id=:cust_id)"
             "        AND (COALESCE(LENGTH(ipsass_shipto_pattern), 0) = 0)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1) ) )"

             "       UNION SELECT formatSalesPrice(ipsprice_price) AS price"
             "       FROM ipsass, ipshead, ipsprice "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (ipsass_shipto_id=:shipto_id)"
             "        AND (ipsass_shipto_id != -1)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1)) )"
             
             "       UNION SELECT formatSalesPrice(ipsprice_price) AS price"
             "       FROM ipsass, ipshead, ipsprice, cust "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (ipsass_custtype_id=cust_custtype_id)"
             "        AND (cust_id=:cust_id)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1)) )"
             
             "       UNION SELECT formatSalesPrice(ipsprice_price) AS price"
             "       FROM ipsass, ipshead, ipsprice, custtype, cust "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (coalesce(length(ipsass_custtype_pattern), 0) > 0)"
             "        AND (custtype_code ~ ipsass_custtype_pattern)"
             "        AND (cust_custtype_id=custtype_id)"
             "        AND (cust_id=:cust_id)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1)))"
             
             "       UNION SELECT formatSalesPrice(ipsprice_price) AS price"
             "       FROM ipsass, ipshead, ipsprice, shipto "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (shipto_id=:shipto_id)"
             "        AND (COALESCE(LENGTH(ipsass_shipto_pattern), 0) > 0)"
             "        AND (shipto_num ~ ipsass_shipto_pattern)"
             "        AND (ipsass_cust_id=:cust_id)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1)) )"

             "       UNION SELECT formatSalesPrice(ipsprice_price) AS price"
             "       FROM sale, ipshead, ipsprice "
             "       WHERE ((sale_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (CURRENT_DATE BETWEEN sale_startdate AND (sale_enddate - 1)) ) "

             "       UNION SELECT formatSalesPrice(item_listprice - (item_listprice * cust_discntprcnt)) AS price "
             "       FROM item, cust "
             "       WHERE ( (item_sold)"
             "        AND (NOT item_exclusive)"
             "        AND (item_id=:item_id)"
             "        AND (cust_id=:cust_id) );");
  q.bindValue(":item_id", _item->id());
  q.bindValue(":cust_id", _custid);
  q.bindValue(":shipto_id", _shiptoid);
  q.bindValue(":curr_id", _netUnitPrice->id());
  q.bindValue(":effective", _netUnitPrice->effective());
  q.exec();
  if (q.size() == 1)
  {
	q.first();
	_netUnitPrice->setLocalValue((q.value("price").toDouble() * _priceRatio) * _priceinvuomratio);
  }
  else
  {
    ParameterList params;
    params.append("cust_id", _custid);
    params.append("shipto_id", _shiptoid);
    params.append("item_id", _item->id());
    // don't params.append("qty", ...) as we don't know how many were purchased
    params.append("curr_id", _netUnitPrice->id());
    params.append("effective", _netUnitPrice->effective());

    priceList newdlg(this, "", TRUE);
    newdlg.set(params);
    if (newdlg.exec() == QDialog::Accepted)
    {
      _netUnitPrice->setLocalValue((newdlg._selectedPrice * _priceRatio) * _priceinvuomratio);
      sCalculateDiscountPrcnt();
    }
  }
}

void returnAuthorizationItem::sLookupTax()
{
  XSqlQuery calcq;
  calcq.prepare("SELECT currToCurr(:extcurr, COALESCE(taxauth_curr_id, :extcurr),"
		"         calculateTax(:tax_id, :ext, 0, 'A'), :date) AS valA,"
		"       currToCurr(:extcurr, COALESCE(taxauth_curr_id, :extcurr),"
		"         calculateTax(:tax_id, :ext, 0, 'B'), :date) AS valB,"
		"       currToCurr(:extcurr, COALESCE(taxauth_curr_id, :extcurr),"
		"         calculateTax(:tax_id, :ext, 0, 'C'), :date) AS valC "
		"FROM taxauth "
		"WHERE (taxauth_id=:auth);");

  calcq.bindValue(":extcurr", _extendedPrice->id());
  calcq.bindValue(":tax_id",  _taxCode->id());
  calcq.bindValue(":ext",     _extendedPrice->localValue());
  calcq.bindValue(":date",    _extendedPrice->effective());
  calcq.bindValue(":auth",    _taxauthid);
  calcq.exec();
  if (calcq.first())
  {
    _taxCache.setLine(calcq.value("valA").toDouble(),
		      calcq.value("valB").toDouble(),
		      calcq.value("valC").toDouble());
    _tax->setLocalValue(_taxCache.total());
  }
  else if (calcq.lastError().type() != QSqlError::None)
  {
    systemError(this, calcq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  } 
}

void returnAuthorizationItem::sTaxDetail()
{
  taxDetail newdlg(this, "", true);
  ParameterList params;
  params.append("tax_id",   _taxCode->id());
  params.append("curr_id",  _tax->id());
  params.append("valueA",   _taxCache.line(0));
  params.append("valueB",   _taxCache.line(1));
  params.append("valueC",   _taxCache.line(2));
  params.append("pctA",	    _taxCache.linePct(0));
  params.append("pctB",	    _taxCache.linePct(1));
  params.append("pctC",	    _taxCache.linePct(2));
  params.append("subtotal", CurrDisplay::convert(_extendedPrice->id(), _tax->id(),
						 _extendedPrice->localValue(),
						 _extendedPrice->effective()));
  if (cView == _mode)
    params.append("readOnly");

  if (newdlg.set(params) == NoError && newdlg.exec())
  {
    _taxCache.setLine(newdlg.amountA(), newdlg.amountB(), newdlg.amountC());
    _taxCache.setLinePct(newdlg.pctA(), newdlg.pctB(),    newdlg.pctC());

    if (_taxCode->id() != newdlg.tax())
      _taxCode->setId(newdlg.tax());

    _tax->setLocalValue(_taxCache.total());
  } 
}

void returnAuthorizationItem::sLookupTaxCode()
{ 
  XSqlQuery taxq;
  taxq.prepare("SELECT tax_ratea, tax_rateb, tax_ratec, tax_id "
	       "FROM tax "
	       "WHERE (tax_id=getTaxSelection(:auth, :type));");
  taxq.bindValue(":auth",    _taxauthid);
  taxq.bindValue(":type",    _taxType->id());
  taxq.exec();
  if (taxq.first())
  {
    _taxCache.setLinePct(taxq.value("tax_ratea").toDouble(),
		         taxq.value("tax_rateb").toDouble(),
		         taxq.value("tax_ratec").toDouble());
    _taxCode->setId(taxq.value("tax_id").toInt());
  }
  else if (taxq.lastError().type() != QSqlError::None)
  {
    systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    _taxCode->setId(-1); 
}

void returnAuthorizationItem::sQtyUOMChanged()
{
  if(_qtyUOM->id() == _invuomid)
    _qtyinvuomratio = 1.0;
  else
  {
    XSqlQuery invuom;
    invuom.prepare("SELECT itemuomtouomratio(item_id, :uom_id, item_inv_uom_id) AS ratio"
                   "  FROM item"
                   " WHERE(item_id=:item_id);");
    invuom.bindValue(":item_id", _item->id());
    invuom.bindValue(":uom_id", _qtyUOM->id());
    invuom.exec();
    if(invuom.first())
      _qtyinvuomratio = invuom.value("ratio").toDouble();
    else
      systemError(this, invuom.lastError().databaseText(), __FILE__, __LINE__);
  }

  if(_qtyUOM->id() != _invuomid)
  {
    _pricingUOM->setId(_qtyUOM->id());
    _pricingUOM->setEnabled(false);
  }
  else
    _pricingUOM->setEnabled(true);
  sCalculateExtendedPrice(); 
}

void returnAuthorizationItem::sPriceUOMChanged()
{
  if(_pricingUOM->id() == -1 || _qtyUOM->id() == -1)
    return;

  if(_pricingUOM->id() == _invuomid)
    _priceinvuomratio = 1.0;
  else
  {
    XSqlQuery invuom;
    invuom.prepare("SELECT itemuomtouomratio(item_id, :uom_id, item_inv_uom_id) AS ratio"
                   "  FROM item"
                   " WHERE(item_id=:item_id);");
    invuom.bindValue(":item_id", _item->id());
    invuom.bindValue(":uom_id", _pricingUOM->id());
    invuom.exec();
    if(invuom.first())
      _priceinvuomratio = invuom.value("ratio").toDouble();
    else
      systemError(this, invuom.lastError().databaseText(), __FILE__, __LINE__);
  }
  //_ratio->setText(formatUOMRatio(_priceinvuomratio));

  updatePriceInfo(); 
}

void returnAuthorizationItem::updatePriceInfo()
{
  XSqlQuery item;
  item.prepare("SELECT item_listprice"
               "  FROM item"
               " WHERE(item_id=:item_id);");
  item.bindValue(":item_id", _item->id());
  item.exec();
  item.first();
  _listPrice->setBaseValue((item.value("item_listprice").toDouble() * _priceRatio)  * _priceinvuomratio);
  sCalculateExtendedPrice();
} 

void returnAuthorizationItem::connectAll()
{
  connect(_discountFromSale, SIGNAL(lostFocus()), this, SLOT(sCalculateFromDiscount()));
  connect(_extendedPrice, SIGNAL(valueChanged()), this, SLOT(sLookupTax()));
  connect(_item,	  SIGNAL(newId(int)),     this, SLOT(sPopulateItemInfo()));
  connect(_listPrices,	  SIGNAL(clicked()),      this, SLOT(sListPrices()));
  connect(_netUnitPrice,  SIGNAL(valueChanged()), this, SLOT(sCalculateDiscountPrcnt()));
  connect(_netUnitPrice,  SIGNAL(valueChanged()), this, SLOT(sCalculateExtendedPrice()));
  connect(_netUnitPrice,  SIGNAL(idChanged(int)), this, SLOT(sPriceGroup()));
  connect(_qtyAuth,	  SIGNAL(textChanged(const QString&)), this, SLOT(sCalculateExtendedPrice()));
  connect(_save,	  SIGNAL(clicked()),      this, SLOT(sSave()));
  connect(_taxCode,	  SIGNAL(newID(int)),	  this, SLOT(sLookupTax()));
  connect(_taxLit, SIGNAL(leftClickedURL(const QString&)), this, SLOT(sTaxDetail()));
  connect(_taxType,	  SIGNAL(newID(int)),	  this, SLOT(sLookupTaxCode()));
  connect(_qtyUOM, SIGNAL(newID(int)), this, SLOT(sQtyUOMChanged()));
  connect(_pricingUOM, SIGNAL(newID(int)), this, SLOT(sPriceUOMChanged()));
}
