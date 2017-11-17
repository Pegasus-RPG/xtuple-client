/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "creditMemo.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>
#include <metasql.h>

#include "mqlutil.h"
#include "errorReporter.h"
#include "creditMemoItem.h"
#include "invoiceList.h"
#include "storedProcErrorLookup.h"
#include "taxBreakdown.h"
#include "guiErrorCheck.h"

creditMemo::creditMemo(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_memoNumber, SIGNAL(editingFinished()), this, SLOT(sCheckCreditMemoNumber()));
  connect(_copyToShipto, SIGNAL(clicked()), this, SLOT(sCopyToShipto()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_invoiceList, SIGNAL(clicked()), this, SLOT(sInvoiceList()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_shipTo, SIGNAL(newId(int)), this, SLOT(populateShipto(int)));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_taxLit, SIGNAL(leftClickedURL(const QString&)), this, SLOT(sTaxDetail()));
  connect(_subtotal, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_tax, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_miscCharge, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_freight,	SIGNAL(valueChanged()),	this, SLOT(sFreightChanged()));
  connect(_taxzone,	SIGNAL(newID(int)),	this, SLOT(sTaxZoneChanged()));
  connect(_cust, SIGNAL(newCrmacctId(int)), _billToAddr, SLOT(setSearchAcct(int)));
  connect(_cust, SIGNAL(newCrmacctId(int)), _shipToAddr, SLOT(setSearchAcct(int)));
  connect(_cust, SIGNAL(newId(int)),        _shipTo,     SLOT(setCustid(int)));
  connect(_shipToName, SIGNAL(textChanged(QString)), this, SLOT(sConvertShipto()));
  connect(_shipToAddr, SIGNAL(changed()), this, SLOT(sConvertShipto()));

#ifndef Q_OS_MAC
  _invoiceList->setMaximumWidth(25);
#endif

  _custtaxzoneid        = -1;
  _freightCache         = 0;
  _taxzoneidCache       = -1;
  _NumberGen            = -1;

  _memoNumber->setValidator(omfgThis->orderVal());
  _commission->setValidator(omfgThis->scrapVal());

  _currency->setLabel(_currencyLit);

  _shipTo->setNameVisible(false);
  _shipTo->setDescriptionVisible(false);

  _cmitem->addColumn(tr("#"),           _seqColumn,   Qt::AlignCenter, true,  "cmitem_linenumber" );
  _cmitem->addColumn(tr("Item"),        _itemColumn,  Qt::AlignLeft,   true,  "item_number"   );
  _cmitem->addColumn(tr("Description"), -1,           Qt::AlignLeft,   true,  "description"   );
  _cmitem->addColumn(tr("Site"),        _whsColumn,   Qt::AlignCenter, true,  "warehous_code" );
  _cmitem->addColumn(tr("Qty UOM"),     _uomColumn,   Qt::AlignLeft,   true,  "qtyuom"   );
  _cmitem->addColumn(tr("Returned"),    _qtyColumn,   Qt::AlignRight,  true,  "cmitem_qtyreturned"  );
  _cmitem->addColumn(tr("Credited"),    _qtyColumn,   Qt::AlignRight,  true,  "cmitem_qtycredit"  );
  _cmitem->addColumn(tr("Price UOM"),   _uomColumn,   Qt::AlignLeft,   true,  "priceuom"   );
  _cmitem->addColumn(tr("Price"),       _priceColumn, Qt::AlignRight,  true,  "cmitem_unitprice"  );
  _cmitem->addColumn(tr("Extended"),    _moneyColumn, Qt::AlignRight,  true,  "extprice"  );

  _miscChargeAccount->setType(GLCluster::cRevenue | GLCluster::cExpense);
}

creditMemo::~creditMemo()
{
  // no need to delete child widgets, Qt does it all for us
}

void creditMemo::languageChange()
{
  retranslateUi(this);
}

enum SetResponse creditMemo::set(const ParameterList &pParams)
{
  XSqlQuery creditet;
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("cmhead_id", &valid);
  if (valid)
  {
    _cmheadid = param.toInt();
    _documents->setId(_cmheadid);
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    _mode = cNew;

    if (param.toString() == "new")
    {
      creditet.prepare("SELECT NEXTVAL('cmhead_cmhead_id_seq') AS cmhead_id;");
      creditet.exec();
      if (creditet.first())
      {
        _cmheadid = creditet.value("cmhead_id").toInt();
        _documents->setId(_cmheadid);
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Credit Memo Information"),
                                    creditet, __FILE__, __LINE__))
      {
        return UndefinedError;
      }

      setNumber();
      _memoDate->setDate(omfgThis->dbDate(), true);

      creditet.prepare("INSERT INTO cmhead ("
		"    cmhead_id, cmhead_number, cmhead_docdate, cmhead_posted"
		") VALUES ("
		"    :cmhead_id, :cmhead_number, :cmhead_docdate, false"
		");");
      creditet.bindValue(":cmhead_id",		_cmheadid);
      creditet.bindValue(":cmhead_number",	(!_memoNumber->text().isEmpty() ? _memoNumber->text() : QString("tmp%1").arg(_cmheadid)));
      creditet.bindValue(":cmhead_docdate",	_memoDate->date());
      creditet.exec();
      if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Credit Memo Information"),
                                    creditet, __FILE__, __LINE__))
      {
        return UndefinedError;
      }

      connect(_cust, SIGNAL(newId(int)), this, SLOT(sPopulateCustomerInfo()));
      connect(_cust, SIGNAL(valid(bool)), _new, SLOT(setEnabled(bool)));
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _memoNumber->setEnabled(false);
      _cust->setReadOnly(true);
      _invoiceNumber->setEnabled(false);
      _invoiceList->hide();

      _new->setEnabled(true);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _memoNumber->setEnabled(false);
      _memoDate->setEnabled(false);
      _cust->setReadOnly(true);
      _invoiceNumber->setEnabled(false);
      _salesRep->setEnabled(false);
      _commission->setEnabled(false);

      _billtoName->setEnabled(false);
      _billToAddr->setEnabled(false);

      _taxzone->setEnabled(false);
      _rsnCode->setEnabled(false);
      _customerPO->setEnabled(false);
      _hold->setEnabled(false);
      _miscCharge->setEnabled(false);
      _miscChargeDescription->setEnabled(false);
      _miscChargeAccount->setReadOnly(true);
      _freight->setEnabled(false);
      _comments->setEnabled(false);
      _invoiceList->hide();
      _shipTo->setEnabled(false);
      _shipToName->setEnabled(false);
      _shipToAddr->setEnabled(false);
      _currency->setEnabled(false);
      _shippingZone->setEnabled(false);
      _saleType->setEnabled(false);
      _project->setEnabled(false);
//      _documents->setReadOnly(true);
      _save->hide();
      _new->hide();
      _delete->hide();
      _edit->setText(tr("&View"));
      disconnect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
      connect(_edit, SIGNAL(clicked()), this, SLOT(sView()));
    }
  }

  param = pParams.value("cust_id", &valid);
  if(cNew == _mode && valid)
    _cust->setId(param.toInt());

  return NoError;
}

int creditMemo::id() const
{
  return _cmheadid;
}

int creditMemo::mode() const
{
  return _mode;
}

void creditMemo::setNumber()
{
  XSqlQuery creditetNumber;
  if ( (_metrics->value("CMNumberGeneration") == "A") ||
       (_metrics->value("CMNumberGeneration") == "O")   )
  {
    creditetNumber.prepare("SELECT fetchCmNumber() AS cmnumber;");
    creditetNumber.exec();
    if (creditetNumber.first())
    {
      _memoNumber->setText(creditetNumber.value("cmnumber").toString());
      _NumberGen = creditetNumber.value("cmnumber").toInt();

      if (_metrics->value("CMNumberGeneration") == "A")
        _memoNumber->setEnabled(false);
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Credit Memo Information"),
                                  creditetNumber, __FILE__, __LINE__))
    {
      return;
    }
  }
  else if (_metrics->value("CMNumberGeneration") == "S")
  {
    creditetNumber.prepare("SELECT fetchSoNumber() AS cmnumber;");
    creditetNumber.exec();
    if (creditetNumber.first())
    {
      _memoNumber->setText(creditetNumber.value("cmnumber").toString());
      _NumberGen = creditetNumber.value("cmnumber").toInt();
      _memoNumber->setEnabled(false);
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Credit Memo Information"),
                                  creditetNumber, __FILE__, __LINE__))
    {
      return;
    }
  }
  else
    _memoNumber->setFocus();
}

void creditMemo::sSave()
{
  //  Make sure that all of the required field have been populated
  QList<GuiErrorCheck>errors;
  errors<<GuiErrorCheck(_memoNumber->text().length() == 0, _memoNumber,
                        tr("<p>You must enter a valid Memo # for this Credit "
                           "Memo before you may save it."));

  if(GuiErrorCheck::reportErrors(this,tr("Cannot Save Credit Memo"),errors))
      return;

  if ( _mode == cNew &&
       ( (_metrics->value("CMNumberGeneration") == "O") ||
         (_metrics->value("CMNumberGeneration") == "M")   ) )
  {
    XSqlQuery query;
    query.prepare( "SELECT cmhead_id "
                   "FROM cmhead "
                   "WHERE (cmhead_number=:cmhead_number)"
                   " AND (cmhead_id !=:cmhead_id);" );
    query.bindValue(":cmhead_number", _memoNumber->text());
    query.bindValue(":cmhead_id", _cmheadid);
    query.exec();
    if (query.first())
    {
      QMessageBox::warning( this, tr("Invalid Memo # Entered"),
                            tr( "<p>The Memo # is already in use.  "
                            "You must enter an unused Memo # for this Credit "
                            "Memo before you may save it." ) );
      _memoNumber->setFocus();
      return;
    }
  }

  errors<<GuiErrorCheck(!_cust->isValid(), _cust,
                        tr("Please select a Customer before continuing.."))
        <<GuiErrorCheck(_total->localValue() < 0, _total,
                       tr("<p>The Total must be a positive value."))
        <<GuiErrorCheck(! _miscCharge->isZero() && !_miscChargeAccount->isValid(), _miscCharge,
                         tr("<p>You may not enter a Misc. Charge without "
                         "indicating the G/L Sales Account number for the "
                         "charge. Please set the Misc. Charge amount to 0 "
                         "or select a Misc. Charge Sales Account."));

  if(GuiErrorCheck::reportErrors(this,tr("Cannot Save Credit Memo"),errors))
      return;

  // save the cmhead
  if (!save())
    return;

  // save address info in case someone wants to use 'em again later
  // but don't make any global changes to the data and ignore errors
  _billToAddr->save(AddressCluster::CHANGEONE);
  _shipToAddr->save(AddressCluster::CHANGEONE);
  
  omfgThis->sCreditMemosUpdated();

  _cmheadid = -1;
  close();
}

bool creditMemo::save()
{
  if(_memoNumber->text().trimmed().length() == 0)
    return false;

  XSqlQuery creditave;
  creditave.prepare( "UPDATE cmhead "
	     "SET cmhead_invcnumber=:cmhead_invcnumber, cmhead_cust_id=:cmhead_cust_id,"
       "    cmhead_number=:cmhead_number,"
	     "    cmhead_custponumber=:cmhead_custponumber, cmhead_hold=:cmhead_hold,"
	     "    cmhead_billtoname=:cmhead_billtoname, cmhead_billtoaddress1=:cmhead_billtoaddress1,"
	     "    cmhead_billtoaddress2=:cmhead_billtoaddress2, cmhead_billtoaddress3=:cmhead_billtoaddress3,"
	     "    cmhead_billtocity=:cmhead_billtocity, cmhead_billtostate=:cmhead_billtostate,"
	     "    cmhead_billtozip=:cmhead_billtozip,"
	     "    cmhead_billtocountry=:cmhead_billtocountry,"
	     "    cmhead_shipto_id=:cmhead_shipto_id,"
	     "    cmhead_shipto_name=:cmhead_shipto_name, cmhead_shipto_address1=:cmhead_shipto_address1,"
	     "    cmhead_shipto_address2=:cmhead_shipto_address2, cmhead_shipto_address3=:cmhead_shipto_address3,"
	     "    cmhead_shipto_city=:cmhead_shipto_city, cmhead_shipto_state=:cmhead_shipto_state,"
	     "    cmhead_shipto_zipcode=:cmhead_shipto_zipcode,"
	     "    cmhead_shipto_country=:cmhead_shipto_country,"
	     "    cmhead_docdate=:cmhead_docdate,"
	     "    cmhead_salesrep_id=:cmhead_salesrep_id, cmhead_commission=:cmhead_commission,"
	     "    cmhead_misc=:cmhead_misc, cmhead_misc_accnt_id=:cmhead_misc_accnt_id,"
	     "    cmhead_misc_descrip=:cmhead_misc_descrip,"
	     "    cmhead_freight=:cmhead_freight,"
	     "    cmhead_taxzone_id=:cmhead_taxzone_id,"
	     "    cmhead_comments=:cmhead_comments, "
	     "    cmhead_rsncode_id=:cmhead_rsncode_id, "
             "    cmhead_curr_id=:cmhead_curr_id, "
             "    cmhead_prj_id=:cmhead_prj_id,"
             "    cmhead_shipzone_id=:cmhead_shipzone_id,"
             "    cmhead_saletype_id=:cmhead_saletype_id "
	     "WHERE (cmhead_id=:cmhead_id);" );

  creditave.bindValue(":cmhead_id", _cmheadid);
  creditave.bindValue(":cmhead_cust_id", _cust->id());
  creditave.bindValue(":cmhead_number", _memoNumber->text());
  creditave.bindValue(":cmhead_invcnumber", _invoiceNumber->invoiceNumber());
  creditave.bindValue(":cmhead_custponumber", _customerPO->text().trimmed());
  creditave.bindValue(":cmhead_billtoname", _billtoName->text().trimmed());
  creditave.bindValue(":cmhead_billtoaddress1",	_billToAddr->line1());
  creditave.bindValue(":cmhead_billtoaddress2",	_billToAddr->line2());
  creditave.bindValue(":cmhead_billtoaddress3",	_billToAddr->line3());
  creditave.bindValue(":cmhead_billtocity",	_billToAddr->city());
  creditave.bindValue(":cmhead_billtostate",	_billToAddr->state());
  creditave.bindValue(":cmhead_billtozip",	_billToAddr->postalCode());
  creditave.bindValue(":cmhead_billtocountry",	_billToAddr->country());
  if (_shipTo->id() > 0)
    creditave.bindValue(":cmhead_shipto_id",	_shipTo->id());
  creditave.bindValue(":cmhead_shipto_name", _shipToName->text().trimmed());
  creditave.bindValue(":cmhead_shipto_address1", _shipToAddr->line1());
  creditave.bindValue(":cmhead_shipto_address2", _shipToAddr->line2());
  creditave.bindValue(":cmhead_shipto_address3", _shipToAddr->line3());
  creditave.bindValue(":cmhead_shipto_city",	 _shipToAddr->city());
  creditave.bindValue(":cmhead_shipto_state",	 _shipToAddr->state());
  creditave.bindValue(":cmhead_shipto_zipcode",	 _shipToAddr->postalCode());
  creditave.bindValue(":cmhead_shipto_country",	 _shipToAddr->country());
  creditave.bindValue(":cmhead_docdate", _memoDate->date());
  creditave.bindValue(":cmhead_comments", _comments->toPlainText());
  creditave.bindValue(":cmhead_salesrep_id", _salesRep->id());
  creditave.bindValue(":cmhead_rsncode_id", _rsnCode->id());
  creditave.bindValue(":cmhead_hold",       QVariant(_hold->isChecked()));
  creditave.bindValue(":cmhead_commission", (_commission->toDouble() / 100));
  if (_miscChargeAccount->id() > 0)
    creditave.bindValue(":cmhead_misc", _miscCharge->localValue());
  creditave.bindValue(":cmhead_misc_accnt_id", _miscChargeAccount->id());
  creditave.bindValue(":cmhead_misc_descrip", _miscChargeDescription->text());
  creditave.bindValue(":cmhead_freight", _freight->localValue());
  if (_taxzone->isValid())
    creditave.bindValue(":cmhead_taxzone_id",	_taxzone->id());
  creditave.bindValue(":cmhead_curr_id", _currency->id());
  if (_project->isValid())
    creditave.bindValue(":cmhead_prj_id", _project->id());
  if(_shippingZone->isValid())
    creditave.bindValue(":cmhead_shipzone_id", _shippingZone->id());
  if(_saleType->isValid())
    creditave.bindValue(":cmhead_saletype_id", _saleType->id());
  creditave.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Credit Memo Information"),
                                creditave, __FILE__, __LINE__))
  {
    return false;
  }

  return true;
}

void creditMemo::sInvoiceList()
{
  ParameterList params;
  params.append("cust_id", _cust->id());
  params.append("invoiceNumber", _invoiceNumber->invoiceNumber());

  invoiceList newdlg(this, "", true);
  newdlg.set(params);
  int invoiceid = newdlg.exec();

  if (invoiceid != 0)
  {

    XSqlQuery sohead;
    sohead.prepare( "SELECT invchead.* "
                    "FROM invchead "
                    "WHERE (invchead_id=:invcid) "
                    "LIMIT 1;" );
    sohead.bindValue(":invcid", invoiceid);
    sohead.exec();
    if (sohead.first())
    {
      _cust->setEnabled(false);
      _billtoName->setEnabled(false);
      _billToAddr->setEnabled(false);

      _cust->setId(sohead.value("invchead_cust_id").toInt());
      _billtoName->setText(sohead.value("invchead_billto_name"));
      _billToAddr->setLine1(sohead.value("invchead_billto_address1").toString());
      _billToAddr->setLine2(sohead.value("invchead_billto_address2").toString());
      _billToAddr->setLine3(sohead.value("invchead_billto_address3").toString());
      _billToAddr->setCity(sohead.value("invchead_billto_city").toString());
      _billToAddr->setState(sohead.value("invchead_billto_state").toString());
      _billToAddr->setPostalCode(sohead.value("invchead_billto_zipcode").toString());
      _billToAddr->setCountry(sohead.value("invchead_billto_country").toString());

      _shipTo->setEnabled(false);
      _shipToName->setEnabled(false);
      _shipToAddr->setEnabled(false);
      _ignoreShiptoSignals = true;
      _shipToName->setText(sohead.value("invchead_shipto_name"));
      _shipToAddr->setLine1(sohead.value("invchead_shipto_address1").toString());
      _shipToAddr->setLine2(sohead.value("invchead_shipto_address2").toString());
      _shipToAddr->setLine3(sohead.value("invchead_shipto_address3").toString());
      _shipToAddr->setCity(sohead.value("invchead_shipto_city").toString());
      _shipToAddr->setState(sohead.value("invchead_shipto_state").toString());
      _shipToAddr->setPostalCode(sohead.value("invchead_shipto_zipcode").toString());
      _shipToAddr->setCountry(sohead.value("invchead_shipto_country").toString());
      _ignoreShiptoSignals = false;

      _invoiceNumber->setInvoiceNumber(sohead.value("invchead_invcnumber").toString());
      _salesRep->setId(sohead.value("invchead_salesrep_id").toInt());
      _commission->setDouble(sohead.value("invchead_commission").toDouble() * 100);

      _taxzoneidCache = sohead.value("invchead_taxzone_id").toInt();
      _taxzone->setId(sohead.value("invchead_taxzone_id").toInt());
      _customerPO->setText(sohead.value("invchead_ponumber"));
      _project->setId(sohead.value("invchead_prj_id").toInt());
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Credit Memo Information"),
                                  sohead, __FILE__, __LINE__))
    {
      return;
    }
  }
}

void creditMemo::populateShipto(int pShiptoid)
{
  if (pShiptoid != -1)
  {
    XSqlQuery query;
    query.prepare( "SELECT shipto_id, shipto_name, shipto_commission, "
                   "       COALESCE(shipto_addr_id, -1) AS shipto_addr_id, "
                   "       COALESCE(shipto_taxzone_id, -1) AS shipto_taxzone_id, "
                   "       COALESCE(shipto_salesrep_id, -1) AS shipto_salesrep_id, "
                   "       COALESCE(shipto_shipzone_id, -1) AS shipto_shipzone_id "
                   "FROM shiptoinfo "
                   "WHERE (shipto_id=:shipto_id);" );
    query.bindValue(":shipto_id", pShiptoid);
    query.exec();
    if (query.first())
    {
      _ignoreShiptoSignals = true;

      _shipTo->setId(query.value("shipto_id").toInt());
      _shipToName->setText(query.value("shipto_name"));
      _shipToAddr->setId(query.value("shipto_addr_id").toInt());

      _ignoreShiptoSignals = false;

      _taxzoneidCache = query.value("shipto_taxzone_id").toInt();
      _taxzone->setId(query.value("shipto_taxzone_id").toInt());
      _salesRep->setId(query.value("shipto_salesrep_id").toInt());
      _commission->setDouble(query.value("shipto_commission").toDouble() * 100);
      _shippingZone->setId(query.value("shipto_shipzone_id").toInt());
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Ship To Information"),
                                  query, __FILE__, __LINE__))
    {
      return;
    }
  }
  else
  {
    _shipTo->setId(-1);
    _shipToName->clear();
    _shipToAddr->clear();
  }
}

void creditMemo::sPopulateCustomerInfo()
{
  if (!_invoiceNumber->isValid())
  {
    if (_cust->isValid())
    {
      XSqlQuery query;
      query.prepare( "SELECT cust_salesrep_id,"
                     "       cust_commprcnt,"
                     "       cust_taxzone_id, cust_curr_id, "
                     "       cust_name, cntct_addr_id, "
                     "       cust_ffshipto, cust_ffbillto, "
                     "       COALESCE(shipto_id, -1) AS shiptoid "
                     "FROM custinfo LEFT OUTER JOIN cntct ON (cust_cntct_id=cntct_id) "
					 "              LEFT OUTER JOIN shiptoinfo ON ((shipto_cust_id=cust_id) "
					 "                                     AND (shipto_default)) "
                     "WHERE (cust_id=:cust_id);" );
      query.bindValue(":cust_id", _cust->id());
      query.exec();
      if (query.first())
      {
        _ffShipto = query.value("cust_ffshipto").toBool();
        _copyToShipto->setEnabled(_ffShipto);
		_shipToName->setEnabled(_ffShipto);
		_shipToAddr->setEnabled(_ffShipto);

        _salesRep->setId(query.value("cust_salesrep_id").toInt());
        _commission->setDouble(query.value("cust_commprcnt").toDouble() * 100);

        _taxzoneidCache = query.value("cust_taxzone_id").toInt();
        _custtaxzoneid = query.value("cust_taxzone_id").toInt();
        _taxzone->setId(query.value("cust_taxzone_id").toInt());
        _currency->setId(query.value("cust_curr_id").toInt());

        _billtoName->setText(query.value("cust_name"));
        _billToAddr->setId(query.value("cntct_addr_id").toInt());

        bool ffBillTo;
        if ( (_mode == cNew) || (_mode == cEdit) )
          ffBillTo = query.value("cust_ffbillto").toBool();
        else
          ffBillTo = false;

        _billtoName->setEnabled(ffBillTo);
        _billToAddr->setEnabled(ffBillTo);

        if ((cNew == _mode) && (query.value("shiptoid").toInt() != -1))
          populateShipto(query.value("shiptoid").toInt());
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Customer Information"),
                                    query, __FILE__, __LINE__))
      {
        return;
      }
    }
    else
    {
      _salesRep->setCurrentIndex(-1);
      _taxzoneidCache = -1;
      _custtaxzoneid	= -1;
      _taxzone->setId(-1);

      _shipToName->setEnabled(_ffShipto);
      _shipToAddr->setEnabled(_ffShipto);
      _shipTo->setId(-1);
      _shipToName->clear();
      _shipToAddr->clear();
    }
  }
}

void creditMemo::sCheckCreditMemoNumber()
{
qDebug("_numbergen->%d, memo#->%d", _NumberGen, _memoNumber->text().toInt());
  if (-1 != _NumberGen && _NumberGen != _memoNumber->text().toInt())
    sReleaseNumber();

  if ( (_memoNumber->text().length()) && _mode != cNew &&
       ( (_metrics->value("CMNumberGeneration") == "O") ||
         (_metrics->value("CMNumberGeneration") == "M")   ) )
  {
    _memoNumber->setEnabled(false);
    XSqlQuery query;
    query.prepare( "SELECT cmhead_id, cmhead_posted "
                   "FROM cmhead "
                   "WHERE (cmhead_number=:cmhead_number)" );
    query.bindValue(":cmhead_number", _memoNumber->text());
    query.exec();
    if (query.first())
    {
      _cmheadid = query.value("cmhead_id").toInt();

      _cust->setReadOnly(true);

      populate();

      if (query.value("cmhead_posted").toBool())
      {
        _memoDate->setEnabled(false);
        _invoiceNumber->setEnabled(false);
        _salesRep->setEnabled(false);
        _commission->setEnabled(false);
        _taxzone->setEnabled(false);
        _customerPO->setEnabled(false);
        _hold->setEnabled(false);
        _miscCharge->setEnabled(false);
        _miscChargeDescription->setEnabled(false);
        _miscChargeAccount->setReadOnly(true);
        _freight->setEnabled(false);
        _comments->setReadOnly(true);
        _invoiceList->hide();
        _shipTo->setEnabled(false);
        _shipToName->setEnabled(false);
        _cmitem->setEnabled(false);
        _save->hide();
        _new->hide();
        _delete->hide();
        _edit->hide();

        _mode = cView;
      }
      else
        _mode = cEdit;
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Credit Memo Information"),
                                  query, __FILE__, __LINE__))
    {
      return;
    }
  }
}

void creditMemo::sConvertShipto()
{
  if (!_ignoreShiptoSignals)
  {
//  Convert the captive shipto to a free-form shipto
    _shipTo->blockSignals(true);
    _shipTo->setId(-1);
    _shipTo->setCustid(_cust->id());
    _shipTo->blockSignals(false);
  }
}

void creditMemo::sCopyToShipto()
{
  _shipTo->setId(-1);
  _shipToName->setText(_billtoName->text());
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

  _taxzoneidCache = _custtaxzoneid;
  _taxzone->setId(_custtaxzoneid);
}

void creditMemo::sNew()
{
  if (!save())
    return;

  ParameterList params;
  params.append("mode", "new");
  params.append("cmhead_id", _cmheadid);

  creditMemoItem newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void creditMemo::sEdit()
{
  if (!save())
    return;

  ParameterList params;
  params.append("mode", "edit");
  params.append("cmhead_id", _cmheadid);
  params.append("cmitem_id", _cmitem->id());

  creditMemoItem newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void creditMemo::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cmhead_id", _cmheadid);
  params.append("cmitem_id", _cmitem->id());

  creditMemoItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void creditMemo::sDelete()
{
  XSqlQuery creditDelete;
  creditDelete.prepare( "SELECT cmhead_posted "
             "FROM cmitem, cmhead "
             "WHERE ( (cmitem_cmhead_id=cmhead_id)"
             " AND (cmitem_id=:cmitem_id) );" );
  creditDelete.bindValue(":cmitem_id", _cmitem->id());
  creditDelete.exec();
  if (creditDelete.first())
  {
    if (creditDelete.value("cmhead_posted").toBool())
    {
      QMessageBox::information(this, "Line Item cannot be delete",
                               tr("<p>This Sales Credit has been Posted and "
				"this cannot be modified.") );
      return;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Credit Memo Line Item"),
                                creditDelete, __FILE__, __LINE__))
  {
    return;
  }

  if (QMessageBox::question(this, "Delete current Line Item?",
                            tr("<p>Are you sure that you want to delete the "
			       "current Line Item?"),
			    QMessageBox::Yes | QMessageBox::Default,
			    QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
  {
    creditDelete.prepare( "DELETE FROM cmitem "
               "WHERE (cmitem_id=:cmitem_id);" );
    creditDelete.bindValue(":cmitem_id", _cmitem->id());
    creditDelete.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Credit Memo Line Item"),
                                  creditDelete, __FILE__, __LINE__))
    {
      return;
    }

    sFillList();
  }
}

void creditMemo::sFillList()
{
  MetaSQLQuery mql = mqlLoad("creditMemoItems", "list");

  ParameterList params;
  params.append("cmhead_id", _cmheadid);
  XSqlQuery creditFillList = mql.toQuery(params);
  _cmitem->populate(creditFillList, true);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Credit Memo Items"),
                             creditFillList, __FILE__, __LINE__))
    return;

  sCalculateSubtotal();
  sCalculateTax();

  _currency->setEnabled(_cmitem->topLevelItemCount() == 0);
}

void creditMemo::sCalculateSubtotal()
{
//  Determine the subtotal and line item tax
  XSqlQuery query;
  query.prepare( "SELECT SUM(round((cmitem_qtycredit * cmitem_qty_invuomratio) * (cmitem_unitprice / cmitem_price_invuomratio), 2)) AS subtotal "
                 "FROM cmitem "
                 "LEFT OUTER JOIN itemsite ON (cmitem_itemsite_id=itemsite_id) "
                 "LEFT OUTER JOIN item ON (itemsite_item_id=item_id) "
                 "WHERE (cmitem_cmhead_id=:cmhead_id);" );
  query.bindValue(":cmhead_id", _cmheadid);
  query.exec();
  if (query.first())
    _subtotal->setLocalValue(query.value("subtotal").toDouble());
}

void creditMemo::sCalculateTotal()
{
  _total->setLocalValue(_subtotal->localValue() +
			_tax->localValue() +
			_freight->localValue() +
			_miscCharge->localValue() );
}

void creditMemo::populate()
{
  XSqlQuery cmhead;
  cmhead.prepare( "SELECT cmhead.*,"
                  "       cust_name, cust_ffbillto, cust_ffshipto "
                  "FROM custinfo, cmhead "
                  "WHERE ( (cmhead_cust_id=cust_id)"
                  " AND (cmhead_id=:cmhead_id) );" );
  cmhead.bindValue(":cmhead_id", _cmheadid);
  cmhead.exec();
  if (cmhead.first())
  {
    if (cmhead.value("cmhead_posted").toBool())
      _status->setText(tr("Posted"));
    else
      _status->setText(tr("Unposted"));

    _memoNumber->setText(cmhead.value("cmhead_number"));
    _memoDate->setDate(cmhead.value("cmhead_docdate").toDate(), true);
    _cust->setId(cmhead.value("cmhead_cust_id").toInt());
    _shipTo->setId(cmhead.value("cmhead_shipto_id").toInt());

    _salesRep->setId(cmhead.value("cmhead_salesrep_id").toInt());
    _commission->setDouble(cmhead.value("cmhead_commission").toDouble() * 100);
    // do taxauth first so we can overwrite the result of the signal cascade
    _taxzoneidCache = cmhead.value("cmhead_taxzone_id").toInt();
    if (!cmhead.value("cmhead_taxzone_id").isNull() && cmhead.value("cmhead_taxzone_id").toInt() != -1)
      _taxzone->setId(cmhead.value("cmhead_taxzone_id").toInt());

    _customerPO->setText(cmhead.value("cmhead_custponumber"));
    _hold->setChecked(cmhead.value("cmhead_hold").toBool());

    _currency->setId(cmhead.value("cmhead_curr_id").toInt());
    _freightCache = cmhead.value("cmhead_freight").toDouble();
    _freight->setLocalValue(cmhead.value("cmhead_freight").toDouble());

    _miscCharge->setLocalValue(cmhead.value("cmhead_misc").toDouble());
    _miscChargeDescription->setText(cmhead.value("cmhead_misc_descrip"));
    _miscChargeAccount->setId(cmhead.value("cmhead_misc_accnt_id").toInt());

    _comments->setText(cmhead.value("cmhead_comments").toString());

    bool ffBillTo = cmhead.value("cust_ffbillto").toBool();
    _billtoName->setEnabled(ffBillTo);
    _billToAddr->setEnabled(ffBillTo);

    _billtoName->setText(cmhead.value("cmhead_billtoname"));
    _billToAddr->setLine1(cmhead.value("cmhead_billtoaddress1").toString());
    _billToAddr->setLine2(cmhead.value("cmhead_billtoaddress2").toString());
    _billToAddr->setLine3(cmhead.value("cmhead_billtoaddress3").toString());
    _billToAddr->setCity(cmhead.value("cmhead_billtocity").toString());
    _billToAddr->setState(cmhead.value("cmhead_billtostate").toString());
    _billToAddr->setPostalCode(cmhead.value("cmhead_billtozip").toString());
    _billToAddr->setCountry(cmhead.value("cmhead_billtocountry").toString());

    _ffShipto = cmhead.value("cust_ffshipto").toBool();
    _shipToName->setEnabled(_ffShipto);
	_shipToAddr->setEnabled(_ffShipto);

    _shipToName->setText(cmhead.value("cmhead_shipto_name"));
    _shipToAddr->setLine1(cmhead.value("cmhead_shipto_address1").toString());
    _shipToAddr->setLine2(cmhead.value("cmhead_shipto_address2").toString());
    _shipToAddr->setLine3(cmhead.value("cmhead_shipto_address3").toString());
    _shipToAddr->setCity(cmhead.value("cmhead_shipto_city").toString());
    _shipToAddr->setState(cmhead.value("cmhead_shipto_state").toString());
    _shipToAddr->setPostalCode(cmhead.value("cmhead_shipto_zipcode").toString());
    _shipToAddr->setCountry(cmhead.value("cmhead_shipto_country").toString());

    if (!cmhead.value("cmhead_rsncode_id").isNull() && cmhead.value("cmhead_rsncode_id").toInt() != -1)
      _rsnCode->setId(cmhead.value("cmhead_rsncode_id").toInt());

    if (! cmhead.value("cmhead_invcnumber").toString().isEmpty())
      _invoiceNumber->setInvoiceNumber(cmhead.value("cmhead_invcnumber").toString());

    if (!cmhead.value("cmhead_prj_id").isNull())
      _project->setId(cmhead.value("cmhead_prj_id").toInt());

    _shippingZone->setId(cmhead.value("cmhead_shipzone_id").toInt());
    _saleType->setId(cmhead.value("cmhead_saletype_id").toInt());

    sCalculateTax();
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Credit Memo Information"),
                                cmhead, __FILE__, __LINE__))
  {
    return;
  }

  sFillList();
}

void creditMemo::closeEvent(QCloseEvent *pEvent)
{
  XSqlQuery creditcloseEvent;
  if ( (_mode == cNew) && (_cmheadid != -1) )
  {
    creditcloseEvent.prepare("SELECT deleteCreditMemo(:cmhead_id) AS result;");
    creditcloseEvent.bindValue(":cmhead_id", _cmheadid);
    creditcloseEvent.exec();
    if (creditcloseEvent.first())
      ; // TODO: add error checking when function returns int instead of boolean
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Credit Memo"),
                                  creditcloseEvent, __FILE__, __LINE__))
    {
      return;
    }

    if (-1 != _NumberGen)
      sReleaseNumber();
  }

  XWidget::closeEvent(pEvent);
}

void creditMemo::sReleaseNumber()
{
  XSqlQuery creditReleaseNumber;
  if ( (_metrics->value("CMNumberGeneration") == "A") ||
       (_metrics->value("CMNumberGeneration") == "O")   )
    creditReleaseNumber.prepare("SELECT releaseCmNumber(:number) AS result;");
  else if (_metrics->value("CMNumberGeneration") == "S")
    creditReleaseNumber.prepare("SELECT releaseSoNumber(:number) AS result;");

  creditReleaseNumber.bindValue(":number", _NumberGen);
  creditReleaseNumber.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Releasing Credit Memo Number"),
                                creditReleaseNumber, __FILE__, __LINE__))
  {
    return;
  }
}

void creditMemo::sTaxDetail()
{
  if (!save())
    return;

  ParameterList params;
  params.append("order_id",	_cmheadid);
  params.append("order_type",	"CM");
  params.append("sense", -1);

  if(cView == _mode)
    params.append("mode", "view");
  else
    params.append("mode", "edit");

  taxBreakdown newdlg(this, "", true);
  if (newdlg.set(params) == NoError)
  {
    newdlg.exec();
    populate();
  }
}

void creditMemo::sCalculateTax()
{
  XSqlQuery taxq;
  taxq.prepare( "SELECT SUM(tax) * -1 AS tax "
                "FROM ("
                "SELECT ROUND(SUM(taxdetail_tax),2) AS tax "
                "FROM tax "
                " JOIN calculateTaxDetailSummary('CM', :cmhead_id, 'T') ON (taxdetail_tax_id=tax_id)"
	        "GROUP BY tax_id) AS data;" );
  taxq.bindValue(":cmhead_id", _cmheadid);
  taxq.exec();
  if (taxq.first())
    _tax->setLocalValue(taxq.value("tax").toDouble());
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Calculating Tax"),
                                taxq, __FILE__, __LINE__))
  {
    return;
  }
  // changing _tax fires sCalculateTotal()
}

void creditMemo::sTaxZoneChanged()
{
  if (_cmheadid != -1 && _taxzoneidCache != _taxzone->id())
  {
    if (!save())
      return;
    _taxzoneidCache = _taxzone->id();
    sCalculateTax();
  }
}

void creditMemo::sFreightChanged()
{
  if (_cmheadid != -1 && _freightCache != _freight->localValue())
  {
    if (!save())
      return;
    _freightCache = _freight->localValue();
    sCalculateTax();
    sCalculateTotal();
  }
}

