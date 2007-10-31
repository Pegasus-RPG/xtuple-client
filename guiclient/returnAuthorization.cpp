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

#include "returnAuthorization.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>
#include <QValidator>
#include <QVariant>

#include "returnAuthorizationItem.h"
#include "shipToList.h"
#include "storedProcErrorLookup.h"
#include "taxBreakdown.h"

returnAuthorization::returnAuthorization(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_authNumber, SIGNAL(lostFocus()), this, SLOT(sCheckAuthorizationNumber()));
  connect(_copyToShipto, SIGNAL(clicked()), this, SLOT(sCopyToShipto()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_shipToNumber, SIGNAL(lostFocus()), this, SLOT(sParseShipToNumber()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSaveClick()));
  connect(_shipToList, SIGNAL(clicked()), this, SLOT(sShipToList()));
  connect(_taxLit, SIGNAL(leftClickedURL(const QString&)), this, SLOT(sTaxDetail()));
  connect(_subtotal, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_tax, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_miscCharge, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_freight,	SIGNAL(valueChanged()),	this, SLOT(sCalculateTotal()));
  connect(_freight,	SIGNAL(valueChanged()),	this, SLOT(sFreightChanged()));
  connect(_taxauth,	SIGNAL(newID(int)),	this, SLOT(sTaxAuthChanged()));
  connect(_so, SIGNAL(newId(int)), this, SLOT(sSalesOrder()));
  connect(_shipToAddr, SIGNAL(changed()), this, SLOT(sClearShiptoNumber()));
  connect(_shipToName, SIGNAL(textChanged(const QString&)), this, SLOT(sNewTest()));
/*  connect(_upCC, SIGNAL(clicked()), this, SLOT(sMoveUp()));
  connect(_viewCC, SIGNAL(clicked()), this, SLOT(sViewCreditCard()));
  connect(_authCC, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal())); */

#ifndef Q_WS_MAC
  _shipToList->setMaximumWidth(25);
#endif

  _taxcurrid		= -1;
  _custtaxauthid	= -1;
  _taxauthidCache	= -1;
  _taxCache.clear();
  _shiptoid		= -1;
  _ignoreShiptoSignals = false;
  _ignoreSoSignals = false;
  _ffBillto = TRUE;
  _ffShipto = TRUE;

  _so->setType((cSoReleased));
  _authNumber->setValidator(omfgThis->orderVal());
  _comments->setType(Comments::ReturnAuth);

  _currency->setLabel(_currencyLit);

  _raitem->addColumn(tr("#"),           _seqColumn,   Qt::AlignCenter );
  _raitem->addColumn(tr("Item"),        _itemColumn,  Qt::AlignLeft   );
  _raitem->addColumn(tr("Description"), -1,           Qt::AlignLeft   );
  _raitem->addColumn(tr("Whs."),        _whsColumn,   Qt::AlignCenter );
  _raitem->addColumn(tr("Status"),      _statusColumn,Qt::AlignCenter );
  _raitem->addColumn(tr("Disposition"), _itemColumn,  Qt::AlignLeft   );
  _raitem->addColumn(tr("Warranty"),    _statusColumn,Qt::AlignCenter );
  _raitem->addColumn(tr("Sold"),        _qtyColumn,   Qt::AlignRight  );
  _raitem->addColumn(tr("Authorized"),  _qtyColumn,   Qt::AlignRight  );
  _raitem->addColumn(tr("Received"),    _qtyColumn,   Qt::AlignRight  );
  _raitem->addColumn(tr("Shipped"),     _qtyColumn,   Qt::AlignRight  );
  _raitem->addColumn(tr("Price"),       _priceColumn, Qt::AlignRight  );
  _raitem->addColumn(tr("Extended"),    _moneyColumn, Qt::AlignRight  );
  _raitem->addColumn(tr("Credited"),    _moneyColumn, Qt::AlignRight  );
  _raitem->addColumn(tr("Refunded"),    _moneyColumn, Qt::AlignRight  );

}

returnAuthorization::~returnAuthorization()
{
    // no need to delete child widgets, Qt does it all for us
}

void returnAuthorization::languageChange()
{
    retranslateUi(this);
}

enum SetResponse returnAuthorization::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("rahead_id", &valid);
  if (valid)
  {
    _raheadid = param.toInt();
    _comments->setId(_raheadid);
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    _mode = cNew;

    if (param.toString() == "new")
    {
      q.prepare("SELECT NEXTVAL('rahead_rahead_id_seq') AS rahead_id;");
      q.exec();
      if (q.first())
	  {
        _raheadid = q.value("rahead_id").toInt();
        _comments->setId(_raheadid);
	  }
      else if (q.lastError().type() != QSqlError::None)
      {
	    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	    return UndefinedError;
      }

      setNumber();
      _authDate->setDate(omfgThis->dbDate(), true);

      q.prepare("INSERT INTO rahead ("
		        "    rahead_id, rahead_number, rahead_authdate"
		        ") VALUES ("
		        "    :rahead_id, :rahead_number, :rahead_authdate"
		        ");");
      q.bindValue(":rahead_id",		_raheadid);
      q.bindValue(":rahead_number",	_authNumber->text().toInt());
      q.exec();
      if (q.lastError().type() != QSqlError::None)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	    return UndefinedError;
      }

      connect(_cust, SIGNAL(newId(int)), this, SLOT(sPopulateCustomerInfo()));
      connect(_cust, SIGNAL(valid(bool)), _new, SLOT(setEnabled(bool)));
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _authNumber->setEnabled(FALSE);
      _cust->setReadOnly(TRUE);

      _new->setEnabled(TRUE);

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _authNumber->setEnabled(FALSE);
      _authDate->setEnabled(FALSE);
      _expireDate->setReadOnly(TRUE);
      _salesRep->setEnabled(FALSE);
      _commission->setEnabled(FALSE);
      _taxauth->setEnabled(FALSE);
      _rsnCode->setEnabled(FALSE);
      _disposition->setEnabled(FALSE);
      _immediately->setEnabled(FALSE);
      _uponReceipt->setEnabled(FALSE);
      _creditBy->setEnabled(FALSE);

      _so->setEnabled(FALSE);
      _incident->setEnabled(FALSE);
      _project->setEnabled(FALSE);

      _cust->setReadOnly(TRUE);
      _billToName->setEnabled(FALSE);
      _billToAddr->setEnabled(FALSE);

      _customerPO->setEnabled(FALSE);

      _miscCharge->setEnabled(FALSE);
      _miscChargeDescription->setEnabled(FALSE);
      _miscChargeAccount->setReadOnly(TRUE);
      _freight->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
	  _comments->setEnabled(FALSE);
      _shipToNumber->setEnabled(FALSE);
      _shipToName->setEnabled(FALSE);
      _shipToAddr->setEnabled(FALSE);
      _currency->setEnabled(FALSE);
      _shipToList->hide();
      _save->hide();
      _new->hide();
      _delete->hide();
      _edit->setText(tr("&View"));

      _close->setFocus();
    }
  }

  param = pParams.value("cust_id", &valid);
  if(cNew == _mode && valid)
    _cust->setId(param.toInt());

  return NoError;
}

void returnAuthorization::setNumber()
{
  if ( (_metrics->value("RANumberGeneration") == "A") || 
       (_metrics->value("RANumberGeneration") == "O")   )
  {
    q.prepare("SELECT fetchRaNumber() AS ranumber;");
    q.exec();
    if (q.first())
    {
      _authNumber->setText(q.value("ranumber").toString());

      if (_metrics->value("RANumberGeneration") == "A")
        _authNumber->setEnabled(FALSE);
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
    _authNumber->setFocus();
}

void returnAuthorization::sSave()
{
  char *dispositionTypes[] = { "C", "R", "P", "V", "M" };
  char *creditMethods[] = { "N", "M", "K", "C" };
  //  Make sure that all of the required field have been populated
  /* if (!_cust->isValid())  User specifically wants to be able to save w/o this
  {
    QMessageBox::information(this, tr("Select a Customer"),
                             tr("Please select a Customer before continuing.") );
    _cust->setFocus();
    return;
  } */

  if ( ! _miscCharge->isZero() && (!_miscChargeAccount->isValid()) )
  {
    QMessageBox::warning( this, tr("No Misc. Charge Account Number"),
                         tr("<p>You may not enter a Misc. Charge without "
			    "indicating the G/L Sales Account number for the "
			    "charge. Please set the Misc. Charge amount to 0 "
			    "or select a Misc. Charge Sales Account." ) );
    _returnAuthInformation->setCurrentPage(1);
    _miscChargeAccount->setFocus();
    return;
  }
  
  if (_total->localValue() < 0 )
  {
    QMessageBox::information(this, tr("Total Less than Zero"),
                             tr("<p>The Total must be a positive value.") );
    _cust->setFocus();
    return;
  }

  // save address info in case someone wants to use 'em again later
  // but don't make any global changes to the data and ignore errors
  _billToAddr->save(AddressCluster::CHANGEONE);
  _shipToAddr->save(AddressCluster::CHANGEONE);

  q.prepare( "UPDATE rahead "
	     "SET rahead_cust_id=:rahead_cust_id,rahead_number=:rahead_number,"
		 "    rahead_authdate=:rahead_authdate,rahead_expiredate=:rahead_expiredate,"
		 "    rahead_status=:rahead_status,"
 	     "    rahead_salesrep_id=:rahead_salesrep_id, rahead_commission=:rahead_commission,"
 	     "    rahead_taxauth_id=:rahead_taxauth_id,rahead_rsncode_id=:rahead_rsncode_id,"
		 "    rahead_disposition=:rahead_disposition,rahead_timing=:rahead_timing,"
		 "    rahead_creditmethod=:rahead_creditmethod,rahead_cohead_id=:rahead_cohead_id,"
		 "    rahead_incdt_id=:rahead_incdt_id,rahead_prj_id=:rahead_prj_id,"
	     "    rahead_billtoname=:rahead_billtoname, rahead_billtoaddress1=:rahead_billtoaddress1,"
	     "    rahead_billtoaddress2=:rahead_billtoaddress2, rahead_billtoaddress3=:rahead_billtoaddress3,"
	     "    rahead_billtocity=:rahead_billtocity, rahead_billtostate=:rahead_billtostate,"
	     "    rahead_billtozip=:rahead_billtozip,rahead_billtocountry=:rahead_billtocountry,"
	     "    rahead_shipto_id=:rahead_shipto_id,"
	     "    rahead_shipto_name=:rahead_shipto_name, rahead_shipto_address1=:rahead_shipto_address1,"
	     "    rahead_shipto_address2=:rahead_shipto_address2, rahead_shipto_address3=:rahead_shipto_address3,"
	     "    rahead_shipto_city=:rahead_shipto_city, rahead_shipto_state=:rahead_shipto_state,"
	     "    rahead_shipto_zipcode=:rahead_shipto_zipcode,rahead_shipto_country=:rahead_shipto_country,"
		 "    rahead_custponumber=:rahead_custponumber,rahead_notes=:rahead_notes, "
	     "    rahead_misc_accnt_id=:rahead_misc_accnt_id,rahead_misc=:rahead_misc, "
	     "    rahead_misc_descrip=:rahead_misc_descrip,"
 	     "    rahead_curr_id=:rahead_curr_id, "
	     "    rahead_freight=:rahead_freight,"
	     "    rahead_adjtax_id=:rahead_adjtax_id,"
	     "    rahead_adjtaxtype_id=:rahead_adjtaxtype_id,"
 	     "    rahead_tax_curr_id=:rahead_tax_curr_id,"
 	     "    rahead_adjtax_ratea=:rahead_adjtax_ratea,"
	     "    rahead_adjtax_rateb=:rahead_adjtax_rateb,"
	     "    rahead_adjtax_ratec=:rahead_adjtax_ratec,"
	     "    rahead_adjtax_pcta=:rahead_adjtax_pcta,"
	     "    rahead_adjtax_pctb=:rahead_adjtax_pctb,"
	     "    rahead_adjtax_pctc=:rahead_adjtax_pctc,"
	     "    rahead_freighttax_id=:rahead_freighttax_id,"
	     "    rahead_freighttaxtype_id=:rahead_freighttaxtype_id,"
	     "    rahead_freighttax_pcta=:rahead_freighttax_pcta,"
	     "    rahead_freighttax_pctb=:rahead_freighttax_pctb,"
	     "    rahead_freighttax_pctc=:rahead_freighttax_pctc,"
	     "    rahead_freighttax_ratea=:rahead_freighttax_ratea,"
	     "    rahead_freighttax_rateb=:rahead_freighttax_rateb,"
	     "    rahead_freighttax_ratec=:rahead_freighttax_ratec,"
		 "    rahead_printed=:rahead_printed "
	     "WHERE (rahead_id=:rahead_id);" );

  q.bindValue(":rahead_id", _raheadid);
  q.bindValue(":rahead_number", _authNumber->text().toInt());
  q.bindValue(":rahead_authdate", _authDate->date());
  q.bindValue(":rahead_expiredate", _expireDate->date());
  q.bindValue(":rahead_salesrep_id", _salesRep->id());
  q.bindValue(":rahead_commission", (_commission->toDouble() / 100));
  if (_taxauth->isValid())
    q.bindValue(":rahead_taxauth_id",	_taxauth->id());
  q.bindValue(":rahead_rsncode_id", _rsnCode->id());
  q.bindValue(":rahead_disposition", QString(dispositionTypes[_disposition->currentItem()]));
  if (_immediately->isChecked())
    q.bindValue(":rahead_timing", "I");
  else
	  q.bindValue(":rahead_timing", "R");
  q.bindValue(":rahead_creditmethod", QString(creditMethods[_creditBy->currentItem()]));
  if (_so->id() != -1)
    q.bindValue(":rahead_cohead_id", _so->id());
  if (_incident->id() != -1)
    q.bindValue(":rahead_incdt_id", _incident->id());
  if (_project->id() != -1)
    q.bindValue(":rahead_prj_id", _project->id());
  q.bindValue(":rahead_cust_id", _cust->id());
  q.bindValue(":rahead_billtoname", _billToName->text().stripWhiteSpace());
  q.bindValue(":rahead_billtoaddress1",	_billToAddr->line1());
  q.bindValue(":rahead_billtoaddress2",	_billToAddr->line2());
  q.bindValue(":rahead_billtoaddress3",	_billToAddr->line3());
  q.bindValue(":rahead_billtocity",	_billToAddr->city());
  q.bindValue(":rahead_billtostate",	_billToAddr->state());
  q.bindValue(":rahead_billtozip",	_billToAddr->postalCode());
  q.bindValue(":rahead_billtocountry",	_billToAddr->country());
  if (_shiptoid > 0)
    q.bindValue(":rahead_shipto_id",	_shiptoid);
  q.bindValue(":rahead_shipto_name", _shipToName->text().stripWhiteSpace());
  q.bindValue(":rahead_shipto_address1", _shipToAddr->line1());
  q.bindValue(":rahead_shipto_address2", _shipToAddr->line2());
  q.bindValue(":rahead_shipto_address3", _shipToAddr->line3());
  q.bindValue(":rahead_shipto_city",	 _shipToAddr->city());
  q.bindValue(":rahead_shipto_state",	 _shipToAddr->state());
  q.bindValue(":rahead_shipto_zipcode",	 _shipToAddr->postalCode());
  q.bindValue(":rahead_shipto_country",	 _shipToAddr->country());
  q.bindValue(":rahead_custponumber", _customerPO->text().stripWhiteSpace());
  q.bindValue(":rahead_notess", _notes->text());
  q.bindValue(":rahead_misc", _miscCharge->localValue());
  if (_miscChargeAccount->id() != -1)
    q.bindValue(":rahead_misc_accnt_id", _miscChargeAccount->id());
  q.bindValue(":rahead_misc_descrip", _miscChargeDescription->text());
  q.bindValue(":rahead_curr_id", _currency->id());
  q.bindValue(":rahead_freight", _freight->localValue());
  if (_taxCache.adjId() > 0)
    q.bindValue(":rahead_adjtax_id",	_taxCache.adjId());
  if (_taxCache.adjType() > 0)
    q.bindValue(":rahead_adjtaxtype_id",_taxCache.adjType());
  if (_taxcurrid > 0)
    q.bindValue(":rahead_tax_curr_id",	_taxcurrid);
  q.bindValue(":rahead_adjtax_ratea",	_taxCache.adj(0));
  q.bindValue(":rahead_adjtax_rateb",	_taxCache.adj(1));
  q.bindValue(":rahead_adjtax_ratec",	_taxCache.adj(2));
  q.bindValue(":rahead_adjtax_pcta",	_taxCache.adjPct(0));
  q.bindValue(":rahead_adjtax_pctb",	_taxCache.adjPct(1));
  q.bindValue(":rahead_adjtax_pctc",	_taxCache.adjPct(2));
  if (_taxCache.freightId() > 0)
    q.bindValue(":rahead_freighttax_id",	_taxCache.freightId());
  if (_taxCache.freightType() > 0)
    q.bindValue(":rahead_freighttaxtype_id",	_taxCache.freightType());
  q.bindValue(":rahead_freighttax_pcta",	_taxCache.freightPct(0));
  q.bindValue(":rahead_freighttax_pctb",	_taxCache.freightPct(1));
  q.bindValue(":rahead_freighttax_pctc",	_taxCache.freightPct(2));
  q.bindValue(":rahead_freighttax_ratea",	_taxCache.freight(0));
  q.bindValue(":rahead_freighttax_rateb",	_taxCache.freight(1));
  q.bindValue(":rahead_freighttax_ratec",	_taxCache.freight(2));

  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sReturnAuthorizationsUpdated();
  omfgThis->sProjectsUpdated(_project->id());

  _comments->setId(_raheadid);
}

void returnAuthorization::sSaveClick()
{
  sSave();
  _raheadid=-1;
  close();
}

void returnAuthorization::sShipToList()
{
  ParameterList params;
  params.append("cust_id", _cust->id());
  params.append("shipto_id", _shiptoid);

  shipToList newdlg(this, "", TRUE);
  newdlg.set(params);

  int shiptoid = newdlg.exec();

  if (shiptoid != -1)
    populateShipto(shiptoid);
}

void returnAuthorization::sSalesOrder()
{
  if (!_ignoreSoSignals && (_so->id() != 0))
  {
    XSqlQuery sohead;
    sohead.prepare( "SELECT cohead.*,custinfo.*, custtype_code, "
                    "       formatScrap(cohead_commission) AS f_commission, "
					"       shipto_num "
                    "FROM cohead "
					"  LEFT OUTER JOIN shiptoinfo ON (cohead_shipto_id=shipto_id), "
					"  custinfo, custtype "
					"WHERE ((cohead_id=:cohead_id) "
					"AND (cohead_cust_id=cust_id) "
					"AND (cust_custtype_id=custtype_id) ) "
                    "LIMIT 1;" );
    sohead.bindValue(":cohead_id", _so->id());
    sohead.exec();
    if (sohead.first())
    {
      _salesRep->setId(sohead.value("cohead_salesrep_id").toInt());
      _commission->setText(sohead.value("f_commission"));

      _taxauth->setId(sohead.value("cohead_taxauth_id").toInt());
      _customerPO->setText(sohead.value("cohead_ponumber"));

      _cust->setEnabled(FALSE);

      _cust->setId(sohead.value("cohead_cust_id").toInt());
  	  _custType->setText(sohead.value("custtype_code").toString());
      _billToName->setText(sohead.value("cohead_billtoname"));
      _billToAddr->setLine1(sohead.value("cohead_billtoaddress1").toString());
      _billToAddr->setLine2(sohead.value("cohead_billtoaddress2").toString());
      _billToAddr->setLine3(sohead.value("cohead_billtoaddress3").toString());
      _billToAddr->setCity(sohead.value("cohead_billtocity").toString());
      _billToAddr->setState(sohead.value("cohead_billtostate").toString());
      _billToAddr->setPostalCode(sohead.value("cohead_billtozipcode").toString());
      _billToAddr->setCountry(sohead.value("cohead_billtocountry").toString());
      if ( (_mode == cNew) || (_mode == cEdit) )
        _ffBillto = sohead.value("cust_ffbillto").toBool();
      else
        _ffBillto = FALSE;
      _billToName->setEnabled(_ffBillto);
      _billToAddr->setEnabled(_ffBillto);

      _ignoreShiptoSignals = TRUE;
	  _shiptoid = sohead.value("cohead_shipto_id").toInt();
	  _shipToNumber->setText(sohead.value("shipto_num").toString());
      _shipToName->setText(sohead.value("cohead_shiptoname"));
      _shipToAddr->setLine1(sohead.value("cohead_shiptoaddress1").toString());
      _shipToAddr->setLine2(sohead.value("cohead_shiptoaddress2").toString());
      _shipToAddr->setLine3(sohead.value("cohead_shiptoaddress3").toString());
      _shipToAddr->setCity(sohead.value("cohead_shiptocity").toString());
      _shipToAddr->setState(sohead.value("cohead_shiptostate").toString());
      _shipToAddr->setPostalCode(sohead.value("cohead_shiptozipcode").toString());
      _shipToAddr->setCountry(sohead.value("cohead_shiptocountry").toString());
      if ( (_mode == cNew) || (_mode == cEdit) )
        _ffShipto = sohead.value("cust_ffshipto").toBool();
	  else
	    _ffShipto = FALSE;
      _copyToShipto->setEnabled(_ffShipto);
	  _shipToName->setEnabled(_ffShipto);
	  _shipToNumber->setEnabled(_ffShipto);
	  _shipToAddr->setEnabled(_ffShipto);
      _ignoreShiptoSignals = FALSE;

	  sSave();
    }
    else if (sohead.lastError().type() != QSqlError::None)
    {
      systemError(this, sohead.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void returnAuthorization::sParseShipToNumber()
{
  q.prepare( "SELECT shipto_id "
             "FROM shiptoinfo "
             "WHERE ( (shipto_cust_id=:cust_id)"
             " AND (UPPER(shipto_num)=UPPER(:shipto_num)));" );
  q.bindValue(":cust_id", _cust->id());
  q.bindValue(":shipto_num", _shipToNumber->text());
  q.exec();
  if (q.first())
    populateShipto(q.value("shipto_id").toInt());
  else
  {
    if (q.lastError().type() != QSqlError::None)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    populateShipto(-1);
  }
}

void returnAuthorization::populateShipto(int pShiptoid)
{
  if (pShiptoid != -1)
  {
    XSqlQuery query;
    query.prepare( "SELECT shipto_num, shipto_name,"
		           " shipto_addr_id, shipto_taxauth_id, "
		           " shipto_salesrep_id, formatScrap(shipto_commission) AS commission "
                   "FROM shiptoinfo "
                   "WHERE (shipto_id=:shipto_id);" );
    query.bindValue(":shipto_id", pShiptoid);
    query.exec();
    if (query.first())
    {
	  _shiptoid = pShiptoid;
      _ignoreShiptoSignals = TRUE;
      _shipToNumber->setText(query.value("shipto_num"));
      _shipToName->setText(query.value("shipto_name"));
      _shipToAddr->setId(query.value("shipto_addr_id").toInt());
      _taxauth->setId(query.value("shipto_taxauth_id").toInt());
      _salesRep->setId(query.value("shipto_salesrep_id").toInt());
      _commission->setText(query.value("commission"));
      _ignoreShiptoSignals = FALSE;
    }
    else if (query.lastError().type() != QSqlError::None)
    {
      systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    _shipToNumber->clear();
    _shipToName->clear();
    _shipToAddr->clear();
  }



  _shiptoid = pShiptoid;
}

void returnAuthorization::sPopulateCustomerInfo()
{
    if (_cust->isValid())
    {
      XSqlQuery query;
      query.prepare( "SELECT custtype_code, cust_salesrep_id,"
                     "       formatScrap(cust_commprcnt) AS f_commission,"
                     "       cust_taxauth_id, cust_curr_id, "
                     "       cust_name, cntct_addr_id, "
                     "       cust_ffshipto, cust_ffbillto, "
					 "        COALESCE(shipto_id, -1) AS shiptoid "
                     "FROM custinfo "
					 "  LEFT OUTER JOIN shiptoinfo ON ((cust_id=shipto_cust_id) "
					 "                             AND (shipto_default)) "
					 "  LEFT OUTER JOIN cntct ON (cust_cntct_id=cntct_id), "
					 "  custtype "
                     "WHERE ( (cust_id=:cust_id) "
					 "AND (custtype_id=cust_custtype_id) );" );
      query.bindValue(":cust_id", _cust->id());
      query.exec();
      if (query.first())
      {
       	_custType->setText(query.value("custtype_code").toString());
        _salesRep->setId(query.value("cust_salesrep_id").toInt());
        _commission->setText(query.value("f_commission"));

        _custtaxauthid = query.value("cust_taxauth_id").toInt();
        _taxauth->setId(query.value("cust_taxauth_id").toInt());
        _currency->setId(query.value("cust_curr_id").toInt());

        _billToName->setText(query.value("cust_name"));
        _billToAddr->setId(query.value("cntct_addr_id").toInt());

        if ( (_mode == cNew) || (_mode == cEdit) )
          _ffBillto = query.value("cust_ffbillto").toBool();
        else
          _ffBillto = FALSE;
        _billToName->setEnabled(_ffBillto);
        _billToAddr->setEnabled(_ffBillto);

        if ( (_mode == cNew) || (_mode == cEdit) )
          _ffShipto = query.value("cust_ffshipto").toBool();
		else
		  _ffShipto = FALSE;
        _copyToShipto->setEnabled(_ffShipto);
		_shipToName->setEnabled(_ffShipto);
		_shipToNumber->setEnabled(_ffShipto);
		_shipToAddr->setEnabled(_ffShipto);
		populateShipto(query.value("shiptoid").toInt());
      }
      else if (query.lastError().type() != QSqlError::None)
      {
	systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
    else
    {
      _salesRep->setCurrentItem(-1);
      _taxauth->setId(-1);
      _custtaxauthid	= -1;
      _billToName->setEnabled(TRUE);
	  _billToAddr->setEnabled(TRUE);
      _billToName->clear();
      _billToAddr->clear();
	  _shipToNumber->setEnabled(TRUE);
      _shipToName->setEnabled(TRUE);
	  _shipToAddr->setEnabled(TRUE);
      _shipToNumber->clear();
      _shipToName->clear();
      _shipToAddr->clear();
    }
}

void returnAuthorization::sCheckAuthorizationNumber()
{
  if ( (_authNumber->text().length()) &&
       ( (_metrics->value("RANumberGeneration") == "O") || 
         (_metrics->value("RANumberGeneration") == "M")   ) )
  {
    _authNumber->setEnabled(FALSE);

    XSqlQuery query;
    query.prepare( "SELECT rahead_id, rahead_status "
                   "FROM rahead "
                   "WHERE (rahead_number=:rahead_number)" );
    query.bindValue(":rahead_number", _authNumber->text().toInt());
    query.exec();
    if (query.first())
    {
      _raheadid = query.value("rahead_id").toInt();

      _cust->setReadOnly(TRUE);

      populate();
     
      if (query.value("rahead_status").toString() == "C")
      {
        _authDate->setEnabled(FALSE);
        _salesRep->setEnabled(FALSE);
        _commission->setEnabled(FALSE);
        _taxauth->setEnabled(FALSE);
        _disposition->setEnabled(FALSE);
		_rsnCode->setEnabled(FALSE);
		_immediately->setEnabled(FALSE);
		_uponReceipt->setEnabled(FALSE);
		_creditBy->setEnabled(FALSE);
        _so->setEnabled(FALSE);
        _incident->setEnabled(FALSE);
		_project->setEnabled(FALSE);
        _customerPO->setEnabled(FALSE);
        _miscCharge->setEnabled(FALSE);
        _miscChargeDescription->setEnabled(FALSE);
        _miscChargeAccount->setReadOnly(TRUE);
        _freight->setEnabled(FALSE);
        _notes->setReadOnly(TRUE);
        _shipToNumber->setEnabled(FALSE);
        _shipToName->setEnabled(FALSE);
        _raitem->setEnabled(FALSE);
        _shipToList->hide();
        _save->hide();
        _new->hide();
        _delete->hide();
        _edit->hide();

        _mode = cView;
      }
      else
        _mode = cEdit;
    }
    else if (query.lastError().type() != QSqlError::None)
    {
      systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void returnAuthorization::sClearShiptoNumber()
{
  if (!_ignoreShiptoSignals)
  {
//  Convert the captive shipto to a free-form shipto
    _shipToNumber->clear();

    _shiptoid = -1;
  }
}

void returnAuthorization::sCopyToShipto()
{
  _shiptoid = -1;
  _shipToNumber->clear();
  _shipToName->setText(_billToName->text());
  if (_billToAddr->id() > 0)
    _shipToAddr->setId(_billToAddr->id());
  else
  {
    _shipToAddr->setLine1(_billToAddr->line1());
    _shipToAddr->setLine2(_billToAddr->line2());
    _shipToAddr->setLine3(_billToAddr->line1());
    _shipToAddr->setCity(_billToAddr->city());
    _shipToAddr->setState(_billToAddr->state());
    _shipToAddr->setPostalCode(_billToAddr->postalCode());
    _shipToAddr->setCountry(_billToAddr->country());
  }

  _taxauth->setId(_custtaxauthid);
}

void returnAuthorization::sNew()
{
  sSave();

  ParameterList params;
  params.append("mode", "new");
  params.append("rahead_id", _raheadid);

  returnAuthorizationItem newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void returnAuthorization::sEdit()
{
  ParameterList params;
  params.append("raitem_id", _raitem->id());


  if (_mode == cView)
    params.append("mode", "view");
  else
    params.append("mode", "edit");

  returnAuthorizationItem newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void returnAuthorization::sDelete()
{
  q.prepare( "SELECT raitem_qtyreceived+raitem_qtyshipped+raitem_qtycredited AS result "
             "FROM raitem "
             "WHERE (raitem_id=:raitem_id);" );
  q.bindValue(":raitem_id", _raitem->id());
  q.exec();
  if (q.first())
  {
    if (q.value("result").toDouble() > 0)
    {
      QMessageBox::information(this, "Line Item cannot be deleted",
                               tr("<p>This line item has transaction history and"
				                  " cannot be modified.") );
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (QMessageBox::question(this, "Delete current Line Item?",
                            tr("<p>Are you sure that you want to delete the "
			       "current Line Item?"),
			    QMessageBox::Yes | QMessageBox::Default,
			    QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
  {
    q.prepare( "DELETE FROM raitem "
               "WHERE (raitem_id=:raitem_id);" );
    q.bindValue(":raitem_id", _raitem->id());
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    sFillList();
  }
}

void returnAuthorization::sFillList()
{
  q.prepare( "SELECT raitem_id, raitem_linenumber, item_number,"
             "       (item_descrip1 || ' ' || item_descrip2), warehous_code,"
			 "       raitem_status, "
			 "       CASE WHEN (raitem_disposition='C') THEN 'Credit' "
			 "            WHEN (raitem_disposition='R') THEN 'Return' "
			 "            WHEN (raitem_disposition='P') THEN 'Replace' "
			 "            WHEN (raitem_disposition='V') THEN 'Service' "
			 "            WHEN (raitem_disposition='S') THEN 'Ship' "
			 "            ELSE 'Error' END AS disposition, "
			 "       formatboolyn(coitem_warranty),"
			 "       formatQty(COALESCE(coitem_qtyshipped,0)),"
			 "       formatQty(raitem_qtyauthorized),"
			 "       formatQty(raitem_qtyreceived),formatQty(raitem_qtyshipped),"       
             "       formatSalesPrice(raitem_unitprice),"
             "       formatMoney(round((raitem_qtyauthorized * raitem_qty_invuomratio) * (raitem_unitprice / raitem_price_invuomratio),2)),"
			 "       formatMoney(raitem_amtcredited),formatMoney(raitem_amtrefunded)"
             "FROM raitem "
			 " LEFT OUTER JOIN coitem ON (raitem_coitem_id=coitem_id), itemsite, item, whsinfo "
             "WHERE ( (raitem_itemsite_id=itemsite_id)"
             " AND (itemsite_item_id=item_id)"
             " AND (itemsite_warehous_id=warehous_id)"
             " AND (raitem_rahead_id=:rahead_id) ) "
             "ORDER BY raitem_linenumber;" );
  q.bindValue(":rahead_id", _raheadid);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  _raitem->populate(q);

  sCalculateSubtotal();
  recalculateTax();

  _currency->setEnabled(_raitem->topLevelItemCount() == 0);

  q.prepare("SELECT raitem_id "
		    "FROM raitem "
			"WHERE ( (raitem_rahead_id=:rahead_id) "
			"AND (raitem_coitem_id IS NOT NULL) "
			"AND (raitem_qtyauthorized > 0 ) );");
  q.bindValue(":rahead_id", _raheadid);
  q.exec();
  _so->setEnabled(!q.first());
}

void returnAuthorization::sCalculateSubtotal()
{
//  Determine the subtotal and line item tax
  XSqlQuery query;
  query.prepare( "SELECT SUM(round((raitem_qtyauthorized * raitem_qty_invuomratio) * (raitem_unitprice / raitem_price_invuomratio),2)) AS subtotal,"
               "       SUM(round((raitem_qtyauthorized * raitem_qty_invuomratio) * stdCost(item_id),2)) AS totalcost "
               "FROM raitem, itemsite, item "
               "WHERE ( (raitem_rahead_id=:rahead_id)"
               " AND (raitem_itemsite_id=itemsite_id)"
               " AND (raitem_status <> 'X')"
               " AND (itemsite_item_id=item_id) );" );
  query.bindValue(":rahead_id", _raheadid);
  query.exec();
  if (query.first())
    _subtotal->setLocalValue(query.value("subtotal").toDouble());
}

void returnAuthorization::sCalculateTotal()
{
  _total->setLocalValue(_subtotal->localValue() +
			_tax->localValue() +
			_freight->localValue() +
			_miscCharge->localValue() );
}

void returnAuthorization::populate()
{
  XSqlQuery rahead;
  rahead.prepare( "SELECT rahead.*, custtype_code, "
                  "       cust_name, cust_ffbillto, cust_ffshipto, shipto_num, "
                  "       formatScrap(rahead_commission) AS f_commission "
                  "FROM cust, custtype, rahead "
				  "  LEFT OUTER JOIN shiptoinfo ON (rahead_shipto_id=shipto_id)"
                  "WHERE ( (rahead_cust_id=cust_id)"
                  " AND (rahead_id=:rahead_id) "
				  " AND (cust_custtype_id=custtype_id) );" );
  rahead.bindValue(":rahead_id", _raheadid);
  rahead.exec();
  if (rahead.first())
  {
    _authNumber->setText(rahead.value("rahead_number"));
    _authDate->setDate(rahead.value("rahead_authdate").toDate(), true);
    _expireDate->setDate(rahead.value("rahead_expiredate").toDate());
    if (rahead.value("rahead_status").toString() == "O")
      _status->setText(tr("Open"));
    else
      _status->setText(tr("Closed"));

    _salesRep->setId(rahead.value("rahead_salesrep_id").toInt());
    _commission->setText(rahead.value("f_commission"));
    // do taxauth first so we can overwrite the result of the signal cascade
    _taxauth->setId(rahead.value("rahead_taxauth_id").toInt());
    if (rahead.value("rahead_tax_curr_id").isNull())
      _taxcurrid = rahead.value("rahead_curr_id").toInt();
    else
      _taxcurrid = rahead.value("rahead_tax_curr_id").toInt();
    if (!rahead.value("rahead_rsncode_id").isNull() && rahead.value("rahead_rsncode_id").toInt() != -1)
      _rsnCode->setId(rahead.value("rahead_rsncode_id").toInt());

    if (rahead.value("rahead_disposition").toString() == "C")
	  _disposition->setCurrentItem(0);
	else if (rahead.value("rahead_disposition").toString() == "R")
	  _disposition->setCurrentItem(1);
	else if (rahead.value("rahead_disposition").toString() == "P")
	  _disposition->setCurrentItem(2);
	else if (rahead.value("rahead_disposition").toString() == "V")
	  _disposition->setCurrentItem(3);
    else if (rahead.value("rahead_disposition").toString() == "M")
	  _disposition->setCurrentItem(4);
	
	if (rahead.value("rahead_timing").toString() == "I")
	  _immediately->setChecked(TRUE);
	else
	  _uponReceipt->setChecked(TRUE);

    if (rahead.value("rahead_creditmethod").toString() == "N")
	  _creditBy->setCurrentItem(0);
	else if (rahead.value("rahead_creditmethod").toString() == "M")
	  _creditBy->setCurrentItem(1);
	else if (rahead.value("rahead_creditmethod").toString() == "K")
	  _creditBy->setCurrentItem(2);
	else if (rahead.value("rahead_creditmethod").toString() == "C")
	  _creditBy->setCurrentItem(3);

    _cust->setId(rahead.value("rahead_cust_id").toInt());
	_custType->setText(rahead.value("custtype_code").toString());
	_ignoreSoSignals = TRUE;
	_so->setId(rahead.value("rahead_cohead_id").toInt());
	_ignoreSoSignals = FALSE;
    _incident->setId(rahead.value("rahead_incdt_id").toInt());
    _project->setId(rahead.value("rahead_prj_id").toInt());

    _ffBillto = rahead.value("cust_ffbillto").toBool();
    _billToName->setEnabled(_ffBillto);
    _billToAddr->setEnabled(_ffBillto);

    _billToName->setText(rahead.value("rahead_billtoname"));
    _billToAddr->setLine1(rahead.value("rahead_billtoaddress1").toString());
    _billToAddr->setLine2(rahead.value("rahead_billtoaddress2").toString());
    _billToAddr->setLine3(rahead.value("rahead_billtoaddress3").toString());
    _billToAddr->setCity(rahead.value("rahead_billtocity").toString());
    _billToAddr->setState(rahead.value("rahead_billtostate").toString());
    _billToAddr->setPostalCode(rahead.value("rahead_billtozip").toString());
    _billToAddr->setCountry(rahead.value("rahead_billtocountry").toString());

    _ffShipto = rahead.value("cust_ffshipto").toBool();
    _shipToName->setEnabled(_ffShipto);
	_shipToAddr->setEnabled(_ffShipto);
	_copyToShipto->setEnabled(_ffShipto);

	_ignoreShiptoSignals = TRUE;
    _shiptoid = rahead.value("rahead_shipto_id").toInt();
	_shipToNumber->setText(rahead.value("shipto_num").toString());
    _shipToName->setText(rahead.value("rahead_shipto_name"));
    _shipToAddr->setLine1(rahead.value("rahead_shipto_address1").toString());
    _shipToAddr->setLine2(rahead.value("rahead_shipto_address2").toString());
    _shipToAddr->setLine3(rahead.value("rahead_shipto_address3").toString());
    _shipToAddr->setCity(rahead.value("rahead_shipto_city").toString());
    _shipToAddr->setState(rahead.value("rahead_shipto_state").toString());
    _shipToAddr->setPostalCode(rahead.value("rahead_shipto_zipcode").toString());
    _shipToAddr->setCountry(rahead.value("rahead_shipto_country").toString());
	_ignoreShiptoSignals = FALSE;
	_customerPO->setText(rahead.value("rahead_custponumber"));

    _currency->setId(rahead.value("rahead_curr_id").toInt());
    _taxCache.setAdjId(rahead.value("rahead_adjtax_id").toInt());
    _taxCache.setAdjType(rahead.value("rahead_adjtaxtype_id").toInt());
    _taxCache.setAdj(rahead.value("rahead_adjtax_ratea").toDouble(),
		      rahead.value("rahead_adjtax_rateb").toDouble(),
		      rahead.value("rahead_adjtax_ratec").toDouble());
    _taxCache.setAdjPct(rahead.value("rahead_adjtax_pcta").toDouble(),
			 rahead.value("rahead_adjtax_pctb").toDouble(),
			 rahead.value("rahead_adjtax_pctc").toDouble());
    _freight->setLocalValue(rahead.value("rahead_freight").toDouble());
    _taxCache.setFreightId(rahead.value("rahead_freighttax_id").toInt());
    _taxCache.setFreightType(rahead.value("rahead_freighttaxtype_id").toInt());
    _taxCache.setFreight(rahead.value("rahead_freighttax_ratea").toDouble(),
		      rahead.value("rahead_freighttax_rateb").toDouble(),
		      rahead.value("rahead_freighttax_ratec").toDouble());
    _taxCache.setFreightPct(rahead.value("rahead_freighttax_pcta").toDouble(),
			 rahead.value("rahead_freighttax_pctb").toDouble(),
			 rahead.value("rahead_freighttax_pctc").toDouble());

    _miscCharge->setLocalValue(rahead.value("rahead_misc").toDouble());
    _miscChargeDescription->setText(rahead.value("rahead_misc_descrip"));
    _miscChargeAccount->setId(rahead.value("rahead_misc_accnt_id").toInt());

    _notes->setText(rahead.value("rahead_notes").toString());

    recalculateTax();

    sFillList();
  }
  else if (rahead.lastError().type() != QSqlError::None)
  {
    systemError(this, rahead.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void returnAuthorization::closeEvent(QCloseEvent *pEvent)
{
  if ( (_mode == cNew) && (_raheadid != -1) )
  {
    if ( (_metrics->value("RANumberGeneration") == "A") || 
         (_metrics->value("RANumberGeneration") == "O")   )
      q.prepare("SELECT releaseRaNumber(:number) AS result;");

    q.bindValue(":number", _authNumber->text().toInt());
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "DELETE FROM raitem "
               "WHERE (raitem_rahead_id=:rahead_id);"

               "DELETE FROM rahead "
               "WHERE (rahead_id=:rahead_id);" );
    q.bindValue(":rahead_id", _raheadid);
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  QMainWindow::closeEvent(pEvent);
}

void returnAuthorization::sTaxDetail()
{
  XSqlQuery taxq;
  taxq.prepare("UPDATE rahead SET rahead_taxauth_id=:taxauth,"
	       "  rahead_freight=:freight,"
	       "  rahead_freighttax_ratea=:freighta,"
	       "  rahead_freighttax_rateb=:freightb,"
	       "  rahead_freighttax_ratec=:freightc,"
	       "  rahead_authdate=:docdate "
	       "WHERE (rahead_id=:rahead_id);");
  if (_taxauth->isValid())
    taxq.bindValue(":taxauth",	_taxauth->id());
  taxq.bindValue(":freight",	_freight->localValue());
  taxq.bindValue(":freighta",	_taxCache.freight(0));
  taxq.bindValue(":freightb",	_taxCache.freight(1));
  taxq.bindValue(":freightc",	_taxCache.freight(2));
  taxq.bindValue(":docdate",	_authDate->date());
  taxq.bindValue(":rahead_id",	_raheadid);
  taxq.exec();
  if (taxq.lastError().type() != QSqlError::None)
  {
    systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  ParameterList params;
  params.append("order_id",	_raheadid);
  params.append("order_type",	"RA");

  if(cView == _mode)
    params.append("mode", "view");
  else
    params.append("mode", "edit");

  taxBreakdown newdlg(this, "", TRUE);
  if (newdlg.set(params) == NoError && newdlg.exec() == QDialog::Accepted)
    populate();
}

void returnAuthorization::recalculateTax()
{
  enum Rate { A = 0, B = 1, C = 2 };
  enum Part { Line = 0, Freight = 1, Adj = 2, Total = 3 };
  double _ttaxCache[3][4];	// [Rate] vs. [Part]

  XSqlQuery itemq;

  //  Determine the line item tax
  itemq.prepare( "SELECT SUM(ROUND(calculateTax(raitem_tax_id, ROUND((raitem_qtyauthorized * raitem_qty_invuomratio) * (raitem_unitprice / raitem_price_invuomratio), 2), 0, 'A'), 2)) AS itemtaxa,"
                 "       SUM(ROUND(calculateTax(raitem_tax_id, ROUND((raitem_qtyauthorized * raitem_qty_invuomratio) * (raitem_unitprice / raitem_price_invuomratio), 2), 0, 'B'), 2)) AS itemtaxb,"
                 "       SUM(ROUND(calculateTax(raitem_tax_id, ROUND((raitem_qtyauthorized * raitem_qty_invuomratio) * (raitem_unitprice / raitem_price_invuomratio), 2), 0, 'C'), 2)) AS itemtaxc "
                 "FROM raitem, itemsite, item "
                 "WHERE ((raitem_rahead_id=:rahead_id)"
                 "  AND  (raitem_itemsite_id=itemsite_id)"
                 "  AND  (itemsite_item_id=item_id));" );

  itemq.bindValue(":rahead_id", _raheadid);
  itemq.exec();
  if (itemq.first())
  {
    _ttaxCache[A][Line] = itemq.value("itemtaxa").toDouble();
    _ttaxCache[B][Line] = itemq.value("itemtaxb").toDouble();
    _ttaxCache[C][Line] = itemq.value("itemtaxc").toDouble();
  }
  else if (itemq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  _ttaxCache[A][Total] = _ttaxCache[A][Line] + _ttaxCache[A][Freight] + _ttaxCache[A][Adj];
  _ttaxCache[B][Total] = _ttaxCache[B][Line] + _ttaxCache[B][Freight] + _ttaxCache[B][Adj];
  _ttaxCache[C][Total] = _ttaxCache[C][Line] + _ttaxCache[C][Freight] + _ttaxCache[C][Adj];

  _tax->setLocalValue(_ttaxCache[A][Total] + _ttaxCache[B][Total] + _ttaxCache[C][Total]);
}

void returnAuthorization::sTaxAuthChanged()
{
  if (_raheadid == -1 || _taxauthidCache == _taxauth->id())
    return;

  XSqlQuery taxauthq;
  taxauthq.prepare("SELECT COALESCE(taxauth_curr_id, :curr_id) AS taxauth_curr_id "
		   "FROM taxauth "
		   "WHERE (taxauth_id=:taxauth_id);");
  if (_taxauth->isValid())
    taxauthq.bindValue(":taxauth_id", _taxauth->id());
  taxauthq.bindValue(":curr_id", _currency->id());
  taxauthq.exec();
  if (taxauthq.first())
    _taxcurrid = taxauthq.value("taxauth_curr_id").toInt();
  else if (taxauthq.lastError().type() != QSqlError::None)
  {
    _taxauth->setId(_taxauthidCache);
    systemError(this, taxauthq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  taxauthq.prepare("SELECT changeraTaxAuth(:rahead_id, :taxauth_id) AS result;");
  taxauthq.bindValue(":rahead_id", _raheadid);
  taxauthq.bindValue(":taxauth_id", _taxauth->id());
  taxauthq.exec();
  if (taxauthq.first())
  {
    int result = taxauthq.value("result").toInt();
    if (result < 0)
    {
      _taxauth->setId(_taxauthidCache);
      systemError(this, storedProcErrorLookup("changeraTaxAuth", result),
		  __FILE__, __LINE__);
      return;
    }
  }
  else if (taxauthq.lastError().type() != QSqlError::None)
  {
    _taxauth->setId(_taxauthidCache);
    systemError(this, taxauthq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _taxauthidCache = _taxauth->id();

  taxauthq.prepare("SELECT rahead_freighttax_id, rahead_adjtax_ratea,"
		   "       rahead_adjtax_rateb, rahead_adjtax_ratec "
		   "FROM rahead "
		   "WHERE (rahead_id=:rahead_id);");
  taxauthq.bindValue(":rahead_id", _raheadid);
  taxauthq.exec();
  if (taxauthq.first())
  {
    if (taxauthq.value("rahead_freighttax_id").isNull())
      _taxCache.setFreightId(-1);
    else
      _taxCache.setFreightId(taxauthq.value("rahead_freighttax_id").toInt());
    _taxCache.setAdj(taxauthq.value("rahead_adjtax_ratea").toDouble(),
		     taxauthq.value("rahead_adjtax_rateb").toDouble(),
		     taxauthq.value("rahead_adjtax_ratec").toDouble());
  }
  else if (taxauthq.lastError().type() != QSqlError::None)
  {
    systemError(this, taxauthq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFreightChanged();
}

void returnAuthorization::sFreightChanged()
{
  XSqlQuery freightq;
  freightq.prepare("SELECT calculateTax(:tax_id, :freight, 0, 'A') AS freighta,"
		   "     calculateTax(:tax_id, :freight, 0, 'B') AS freightb,"
		   "     calculateTax(:tax_id, :freight, 0, 'C') AS freightc;");
  freightq.bindValue(":tax_id", _taxCache.freightId());
  freightq.bindValue(":freight", _freight->localValue());
  freightq.exec();
  if (freightq.first())
  {
    _taxCache.setFreight(freightq.value("freighta").toDouble(),
			 freightq.value("freightb").toDouble(),
			 freightq.value("freightc").toDouble());
  }
  else if (freightq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, freightq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  recalculateTax();
}
