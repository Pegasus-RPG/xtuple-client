/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2018 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "salesOrder.h"
#include <stdlib.h>

#include <QAction>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include <metasql.h>

#include "characteristicAssignment.h"
#include "creditCard.h"
#include "creditcardprocessor.h"
#include "crmacctcluster.h"
#include "customer.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "distributeInventory.h"
#include "issueLineToShipping.h"
#include "mqlutil.h"
#include "salesOrderItem.h"
#include "storedProcErrorLookup.h"
#include "taxBreakdown.h"
#include "freightBreakdown.h"
#include "printPackingList.h"
#include "printSoForm.h"
#include "printQuote.h"
#include "prospect.h"
#include "allocateARCreditMemo.h"
#include "reserveSalesOrderItem.h"
#include "dspReservations.h"
#include "purchaseRequest.h"
#include "purchaseOrder.h"
#include "workOrder.h"
#include "itemAvailabilityWorkbench.h"

// why did someone invent modetype & modestate for scripting vs. rewriting the macros below or passing _mode?
enum OrderModeType  { QuoteMode = 1, OrderMode = 2 };

#define cNewQuote   (0x20 | cNew)
#define cEditQuote  (0x20 | cEdit)
#define cViewQuote  (0x20 | cView)

#define ISQUOTE(mode) (((mode) & 0x20) == 0x20)
#define ISORDER(mode) (!ISQUOTE(mode))

#define ISNEW(mode)   (((mode) & 0x0F) == cNew)
#define ISEDIT(mode)  (((mode) & 0x0F) == cEdit)
#define ISVIEW(mode)  (((mode) & 0x0F) == cView)

#define cClosed       0x01
#define cActiveOpen   0x02
#define cInactiveOpen 0x04
#define cCanceled     0x08

#define iDontUpdate   1
#define iAskToUpdate  2
#define iJustUpdate   3

salesOrder::salesOrder(QWidget *parent, const char *name, Qt::WindowFlags fl)
  : XDocumentWindow(parent, name, fl),
    _saved         (false),
    _saving        (false),
    _calcfreight   (false),
    _orderNumberGen(0),
    _freightCache  (0),
    _userEnteredOrderNumber(false),
    _ignoreSignals (true),
    _blanketPos    (false),
    _usesPos       (false),
    _captive       (false),
    _holdOverride  (false),
    _soheadid      (-1),
    _numSelected   (0),
    _custtaxzoneid (-1),
    _taxzoneidCache(-1),
    _crmacctid     (-1)
{
  setupUi(widget());
  setWindowTitle(tr("Sales Order"));

  _dspShipmentsBySalesOrder = new dspShipmentsBySalesOrder(this, "dspShipmentsBySalesOrder", Qt::Widget);
  _dspShipmentsBySalesOrder->setObjectName("dspShipmentsBySalesOrder");
  _shipmentsPage->layout()->addWidget(_dspShipmentsBySalesOrder);
  _dspShipmentsBySalesOrder->setCloseVisible(false);
  _dspShipmentsBySalesOrder->findChild<OrderCluster*>("_salesOrder")->setEnabled(false);

  sCheckValidContacts();

  connect(_action,              SIGNAL(clicked()),                              this,         SLOT(sAction()));
  connect(_authorize,           SIGNAL(clicked()),                              this,         SLOT(sAuthorizeCC()));
  connect(_charge,              SIGNAL(clicked()),                              this,         SLOT(sChargeCC()));
  connect(_postCash,            SIGNAL(clicked()),                              this,         SLOT(sEnterCashPayment()));
  connect(_clear,               SIGNAL(pressed()),                              this,         SLOT(sClear()));
  connect(_copyToShipto,        SIGNAL(clicked()),                              this,         SLOT(sCopyToShipto()));
  connect(_cust,                SIGNAL(newId(int)),                             this,         SLOT(sPopulateCustomerInfo(int)));
  connect(_cancel,              SIGNAL(clicked()),                              this,         SLOT(sCancel()));
  connect(_delete,              SIGNAL(clicked()),                              this,         SLOT(sDelete()));
  connect(_downCC,              SIGNAL(clicked()),                              this,         SLOT(sMoveDown()));
  connect(_edit,                SIGNAL(clicked()),                              this,         SLOT(sEdit()));
  connect(_editCC,              SIGNAL(clicked()),                              this,         SLOT(sEditCreditCard()));
  connect(_new,                 SIGNAL(clicked()),                              this,         SLOT(sNew()));
  connect(_newCC,               SIGNAL(clicked()),                              this,         SLOT(sNewCreditCard()));
  connect(_orderNumber,         SIGNAL(editingFinished()),                      this,         SLOT(sHandleOrderNumber()));
  connect(_orderNumber,         SIGNAL(textChanged(const QString &)),           this,         SLOT(sSetUserEnteredOrderNumber()));
  connect(_save,                SIGNAL(clicked()),                              this,         SLOT(sSave()));
  connect(_saveAndAdd,          SIGNAL(clicked()),                              this,         SLOT(sSaveAndAdd()));
  connect(_close,               SIGNAL(clicked()),                              this,         SLOT(close()));
  connect(_shippingCharges,     SIGNAL(newID(int)),                             this,         SLOT(sHandleShipchrg(int)));
  connect(_shipToAddr,          SIGNAL(changed()),                              this,         SLOT(sConvertShipTo()));
  connect(_shipToName,          SIGNAL(textChanged(const QString &)),           this,         SLOT(sConvertShipTo()));
  connect(_shipTo,              SIGNAL(newId(int)),                             this,         SLOT(sParseShipToNumber()));
  connect(_showCanceled,        SIGNAL(toggled(bool)),                          this,         SLOT(sFillItemList()));
  connect(_soitem,              SIGNAL(populateMenu(QMenu*,QTreeWidgetItem *)), this,         SLOT(sPopulateMenu(QMenu *)));
  connect(_soitem,              SIGNAL(itemSelectionChanged()),                 this,         SLOT(sHandleButtons()));
  connect(_taxZone,             SIGNAL(newID(int)),                             this,         SLOT(sTaxZoneChanged()));
  connect(_taxLit,              SIGNAL(leftClickedURL(const QString &)),        this,         SLOT(sTaxDetail()));
  connect(_freightLit,          SIGNAL(leftClickedURL(const QString &)),        this,         SLOT(sFreightDetail()));
  connect(_upCC,                SIGNAL(clicked()),                              this,         SLOT(sMoveUp()));
  connect(_viewCC,              SIGNAL(clicked()),                              this,         SLOT(sViewCreditCard()));
  connect(_warehouse,           SIGNAL(newID(int)),                             this,         SLOT(sPopulateFOB(int)));
  connect(_issueStock,          SIGNAL(clicked()),                              this,         SLOT(sIssueStock()));
  connect(_issueLineBalance,    SIGNAL(clicked()),                              this,         SLOT(sIssueLineBalance()));
  connect(_reserveStock,        SIGNAL(clicked()),                              this,         SLOT(sReserveStock()));
  connect(_reserveLineBalance,  SIGNAL(clicked()),                              this,         SLOT(sReserveLineBalance()));
  connect(_subtotal,            SIGNAL(valueChanged()),                         this,         SLOT(sCalculateTotal()));
  connect(_miscCharge,          SIGNAL(valueChanged()),                         this,         SLOT(sCalculateTotal()));
  connect(_freight,             SIGNAL(valueChanged()),                         this,         SLOT(sFreightChanged()));
  if (_privileges->check("ApplyARMemos"))
    connect(_allocatedCMLit,    SIGNAL(leftClickedURL(const QString &)),        this,         SLOT(sCreditAllocate()));
  connect(_allocatedCM,         SIGNAL(valueChanged()),                         this,         SLOT(sCalculateTotal()));
  connect(_outstandingCM,       SIGNAL(valueChanged()),                         this,         SLOT(sCalculateTotal()));
  connect(_authCC,              SIGNAL(valueChanged()),                         this,         SLOT(sCalculateTotal()));
  connect(_more,                SIGNAL(clicked()),                              this,         SLOT(sHandleMore()));
  connect(_orderDate,           SIGNAL(newDate(QDate)),                         this,         SLOT(sOrderDateChanged()));
  connect(_shipDate,            SIGNAL(newDate(QDate)),                         this,         SLOT(sShipDateChanged()));
  connect(_cust,                SIGNAL(newCrmacctId(int)),                      _billToAddr,  SLOT(setSearchAcct(int)));
  connect(_cust,                SIGNAL(newCrmacctId(int)),                      _shipToAddr,  SLOT(setSearchAcct(int)));
  connect(_newCharacteristic,   SIGNAL(clicked()),                              this,         SLOT(sNewCharacteristic()));
  connect(_editCharacteristic,  SIGNAL(clicked()),                              this,         SLOT(sEditCharacteristic()));
  connect(_deleteCharacteristic,SIGNAL(clicked()),                              this,         SLOT(sDeleteCharacteristic()));
  connect(_holdType,            SIGNAL(newID(int)),                             this,         SLOT(sHoldTypeChanged()));

  connect(_billToAddr,          SIGNAL(addressChanged(QString,QString,QString,QString,QString,QString, QString)),
          _billToCntct, SLOT(setNewAddr(QString,QString,QString,QString,QString,QString, QString)));

  connect(_shipToAddr,          SIGNAL(addressChanged(QString,QString,QString,QString,QString,QString, QString)),
          _shipToCntct, SLOT(setNewAddr(QString,QString,QString,QString,QString,QString, QString)));

  setFreeFormShipto(false);

  _holdType->append(0, tr("None"),     "N");
  _holdType->append(1, tr("Credit"),   "C");
  _holdType->append(2, tr("Shipping"), "S");
  _holdType->append(3, tr("Packing"),  "P");
  if (_metrics->boolean("EnableReturnAuth"))
    _holdType->append(4, tr("Return"),   "R");
  _holdType->append(5, tr("Tax"),      "T");

  _orderCurrency->setLabel(_orderCurrencyLit);

  _shipTo->setNameVisible(false);
  _shipTo->setDescriptionVisible(false);

  _orderNumber->setValidator(omfgThis->orderVal());
  _CCCVV->setValidator(new QIntValidator(100, 9999, this));
  _weight->setValidator(omfgThis->weightVal());
  _commission->setValidator(omfgThis->percentVal());
  _marginPercent->setValidator(omfgThis->percentVal());

  _applDate->setDate(omfgThis->dbDate(), true);
  _distDate->setDate(omfgThis->dbDate(), true);

  _soitem->addColumn(tr("#"),               _seqColumn,            Qt::AlignCenter, true,  "f_linenumber");
  _soitem->addColumn(tr("Kit Seq. #"),      _seqColumn,            Qt::AlignRight,  false, "coitem_subnumber");
  _soitem->addColumn(tr("Item"),            _itemColumn,           Qt::AlignLeft,   true,  "item_number");
  _soitem->addColumn(tr("Customer P/N"),    _itemColumn,           Qt::AlignLeft,   false, "item_number_cust");
  _soitem->addColumn(tr("Type"),            _itemColumn,           Qt::AlignLeft,   false, "item_type");
  _soitem->addColumn(tr("Description"),     -1,                    Qt::AlignLeft,   true,  "description");
  _soitem->addColumn(tr("Site"),            _whsColumn,            Qt::AlignCenter, true,  "warehous_code");
  _soitem->addColumn(tr("Status"),          _statusColumn,         Qt::AlignCenter, true,  "enhanced_status");
  _soitem->addColumn(tr("Firm"),            0,                     Qt::AlignCenter, false, "coitem_firm");
  _soitem->addColumn(tr("Sched. Date"),     _dateColumn,           Qt::AlignCenter, true,  "coitem_scheddate");
  _soitem->addColumn(tr("Ordered"),         _qtyColumn,            Qt::AlignRight,  true,  "coitem_qtyord");
  _soitem->addColumn(tr("Selling UOM"),         (int)(_uomColumn*1.5), Qt::AlignLeft,   true,  "qty_uom");
  _soitem->addColumn(tr("Shipped"),         _qtyColumn,            Qt::AlignRight,  true,  "qtyshipped");
  _soitem->addColumn(tr("At Shipping"),     _qtyColumn,            Qt::AlignRight,  false, "qtyatshipping");
  _soitem->addColumn(tr("Balance"),         _qtyColumn,            Qt::AlignRight,  false, "balance");
  _soitem->addColumn(tr("Price"),           _priceColumn,          Qt::AlignRight,  true,  "coitem_price");
  _soitem->addColumn(tr("Extended"),        _priceColumn,          Qt::AlignRight,  true,  "extprice");
  _soitem->addColumn(tr("Cust. Price"),     _priceColumn,          Qt::AlignRight,  false, "coitem_custprice");
  _soitem->addColumn(tr("Cust. Discount"),  _priceColumn,          Qt::AlignRight,  false, "discountfromcust");
  _soitem->addColumn(tr("List Price"),      _priceColumn,          Qt::AlignRight,  false, "coitem_listprice");
  _soitem->addColumn(tr("List Discount"),   _priceColumn,          Qt::AlignRight,  false, "discountfromlist");
  if (_privileges->check("ViewSOItemUnitCost"))
    _soitem->addColumn(tr("Unit Cost"),       _costColumn,           Qt::AlignRight,  false, "coitem_unitcost");
  if (_privileges->check("ShowMarginsOnSalesOrder"))
  {
    _soitem->addColumn(tr("Margin"),          _priceColumn,          Qt::AlignRight,  false, "margin");
    _soitem->addColumn(tr("Margin %"),        _prcntColumn,          Qt::AlignRight,  false, "marginpercent");
  }
  _soitem->addColumn(tr("Prod. Weight"),    _qtyColumn,            Qt::AlignRight,  false, "prodweight");
  _soitem->addColumn(tr("Pkg. Weight"),     _qtyColumn,            Qt::AlignRight,  false, "packweight");
  _soitem->addColumn(tr("Supply Type"),     _itemColumn,           Qt::AlignCenter, false, "spplytype");
  _soitem->addColumn(tr("Order Number"),    _itemColumn,           Qt::AlignCenter, false, "ordrnumbr");
  _soitem->addColumn(tr("Available QOH"),   _qtyColumn,            Qt::AlignCenter, true,  "availableqoh");
  if (_metrics->boolean("EnableSOReservations"))
  {
    _soitem->addColumn(tr("Reserved"),      _qtyColumn,            Qt::AlignCenter, true,  "reserved");
    _soitem->addColumn(tr("Reservable"),    _qtyColumn,            Qt::AlignCenter, true,  "reservable");
  }

  _charass->addColumn(tr("Characteristic"), _itemColumn, Qt::AlignLeft, true, "char_name" );
  _charass->addColumn(tr("Value"),          -1,          Qt::AlignLeft, true, "charass_value" );

  _cc->addColumn(tr("Sequence"),_itemColumn, Qt::AlignLeft, true, "ccard_seq");
  _cc->addColumn(tr("Type"),    _itemColumn, Qt::AlignLeft, true, "type");
  _cc->addColumn(tr("Number"),  _itemColumn, Qt::AlignRight,true, "f_number");
  _cc->addColumn(tr("Active"),  _itemColumn, Qt::AlignLeft, true, "ccard_active");
  _cc->addColumn(tr("Name"),    _itemColumn, Qt::AlignLeft, true, "ccard_name");
  _cc->addColumn(tr("Expiration Date"),  -1, Qt::AlignLeft, true, "expiration");

  _printSO->setChecked(_metrics->boolean("DefaultPrintSOOnSave"));

  _quotestatusLit->hide();
  _quotestaus->hide();

  sPopulateFOB(_warehouse->id());

  _ignoreSignals = false; // don't move this to initializer section above

  if (!_privileges->check("ShowMarginsOnSalesOrder"))
  {
    _margin->hide();
    _marginLit->hide();
    _marginPercent->hide();
    _marginPercentLit->hide();
  }

  _project->setType(ProjectLineEdit::SalesOrder);
  if (!_metrics->boolean("UseProjects"))
  {
    _projectLit->hide();
    _project->hide();
  }

  if (!_metrics->boolean("MultiWhs"))
  {
    _shippingWhseLit->hide();
    _warehouse->hide();
  }

  if (!_metrics->boolean("CCAccept") || !_privileges->check("ProcessCreditCards"))
  {
    _paymentInformation->removeTab(_paymentInformation->indexOf(_creditCardPage));
  }

  if (_metrics->boolean("EnableSOReservations"))
  {
    _requireInventory->setChecked(true);
    _requireInventory->setEnabled(false);
  }

  if (!_metrics->boolean("AlwaysShowSaveAndAdd"))
    _saveAndAdd->hide();

  _more->setChecked(_preferences->boolean("SoShowAll"));

  if(_metrics->boolean("DefaultSOLineItemsTab"))
    _salesOrderInformation->setCurrentIndex(1);
  else
    _salesOrderInformation->setCurrentIndex(0);


  _miscChargeAccount->setType(GLCluster::cRevenue | GLCluster::cExpense);

  _fundsType->populate("SELECT fundstype_id, fundstype_name, fundstype_code FROM fundstype WHERE NOT fundstype_creditcard;");

  _bankaccnt->setType(XComboBox::ARBankAccounts);
  _salescat->setType(XComboBox::SalesCategoriesActive);

  sHandleMore();
}

salesOrder::~salesOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

void salesOrder::languageChange()
{
  retranslateUi(this);
}

enum SetResponse salesOrder:: set(const ParameterList &pParams)
{
  XSqlQuery setSales;
  XDocumentWindow::set(pParams);
  QVariant  param;
  bool      valid;

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      setObjectName("salesOrder new");
      _mode = cNew;
      emit newModeType(OrderMode);
      emit newModeState(cNew);

      _cust->setType(CLineEdit::ActiveCustomers);
      _salesRep->setType(XComboBox::SalesRepsActive);
      _comments->setType(Comments::SalesOrder);
      _project->setAllowedStatuses(ProjectLineEdit::Concept |  ProjectLineEdit::InProcess);
      _calcfreight = _metrics->boolean("CalculateFreight");

      connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sHandleSalesOrderEvent(int, bool)));
      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));
    }
    else if (param.toString() == "newQuote")
    {
      _mode = cNewQuote;
      emit newModeType(QuoteMode);
      emit newModeState(cNew);

      _cust->setType(CLineEdit::ActiveCustomersAndProspects);
      _salesRep->setType(XComboBox::SalesRepsActive);
      _project->setAllowedStatuses(ProjectLineEdit::Concept |  ProjectLineEdit::InProcess);
      _calcfreight = _metrics->boolean("CalculateFreight");
      _action->hide();

      _CCAmount->hide();
      _CCCVVLit->hide();
      _CCCVV->hide();
      _newCC->hide();
      _editCC->hide();
      _viewCC->hide();
      _upCC->hide();
      _downCC->hide();
      _authorize->hide();
      _charge->hide();
      _paymentInformation->removeTab(_paymentInformation->indexOf(_cashPage));
      _quotestatusLit->show();
      _quotestaus->show();
      _quotestaus->setText("Open");

      connect(omfgThis, SIGNAL(quotesUpdated(int, bool)), this, SLOT(sHandleSalesOrderEvent(int, bool)));
      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      emit newModeType(OrderMode);
      emit newModeState(cEdit);

      if (_metrics->boolean("AlwaysShowSaveAndAdd"))
        _saveAndAdd->setEnabled(true);
      else
        _saveAndAdd->hide();
      _comments->setType(Comments::SalesOrder);
      _cust->setType(CLineEdit::AllCustomers);

      connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sHandleSalesOrderEvent(int, bool)));
      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));
    }
    else if (param.toString() == "editQuote")
    {
      _mode = cEditQuote;
      emit newModeType(QuoteMode);
      emit newModeState(cEdit);

      _cust->setType(CLineEdit::AllCustomersAndProspects);
      _action->setEnabled(false);
      _action->hide();

      _CCAmount->hide();
      _CCCVVLit->hide();
      _CCCVV->hide();
      _newCC->hide();
      _editCC->hide();
      _viewCC->hide();
      _upCC->hide();
      _downCC->hide();
      _authorize->hide();
      _charge->hide();
      _paymentInformation->removeTab(_paymentInformation->indexOf(_cashPage));
      _quotestatusLit->show();
      _quotestaus->show();

      connect(omfgThis, SIGNAL(quotesUpdated(int, bool)), this, SLOT(sHandleSalesOrderEvent(int, bool)));
      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));
    }
    else if (param.toString() == "view")
    {
      setViewMode();
      _cust->setType(CLineEdit::AllCustomers);

      _issueStock->hide();
      _issueLineBalance->hide();

      _reserveStock->hide();
      _reserveLineBalance->hide();
    }
    else if (param.toString() == "viewQuote")
    {
      _mode = cViewQuote;
      emit newModeType(QuoteMode);
      emit newModeState(cView);

      _orderNumber->setEnabled(false);
      _packDate->setEnabled(false);
      _shipDate->setEnabled(false);
      _cust->setReadOnly(true);
      _warehouse->setEnabled(false);
      _salesRep->setEnabled(false);
      _commission->setEnabled(false);
      _taxZone->setEnabled(false);
      _terms->setEnabled(false);
      _terms->setType(XComboBox::Terms);
      _fob->setEnabled(false);
      _shipVia->setEnabled(false);
      _shippingCharges->setEnabled(false);
      _shippingForm->setEnabled(false);
      _miscCharge->setEnabled(false);
      _miscChargeDescription->setEnabled(false);
      _miscChargeAccount->setReadOnly(true);
      _freight->setEnabled(false);
      _orderComments->setEnabled(false);
      _shippingComments->setEnabled(false);
      _custPONumber->setEnabled(false);
      _holdType->setEnabled(false);
      _saleType->setEnabled(false);
      _shippingZone->setEnabled(false);
      _edit->setText(tr("View"));
      _cust->setType(CLineEdit::AllCustomersAndProspects);
      _comments->setReadOnly(true);
//      _documents->setReadOnly(true);
      _copyToShipto->setEnabled(false);
      _orderCurrency->setEnabled(false);
      _newCharacteristic->setEnabled(false);
      _paymentInformation->removeTab(_paymentInformation->indexOf(_cashPage));
      _save->hide();
      _clear->hide();
      _action->hide();
      _delete->hide();
      _quotestatusLit->show();
      _quotestaus->show();
    }

    if (_mode == cNew || _mode == cNewQuote)
    {
       param = pParams.value("prj_id", &valid);
       if (valid)
         _project->setId(param.toInt());
    }
  }

  // argument must match source_docass
  _documents->setType(ISQUOTE(_mode) ? "Q" : "S");

  sHandleMore();

  if (ISNEW(_mode))
  {
    _ignoreSignals = true;

    populateOrderNumber();
    if (_orderNumber->text().isEmpty())
      _orderNumber->setFocus();
    else
      _cust->setFocus();

    _ignoreSignals = false;

    if (ISORDER(_mode))
      setSales.exec("SELECT NEXTVAL('cohead_cohead_id_seq') AS head_id;");
    else  // (ISQUOTE(_mode))
      setSales.exec("SELECT NEXTVAL('quhead_quhead_id_seq') AS head_id;");
    if (setSales.first())
    {
      _soheadid = setSales.value("head_id").toInt();
      emit newId(_soheadid);
      _comments->setId(_soheadid);
      _documents->setId(_soheadid);
      _orderDateCache = omfgThis->dbDate();
      _orderDate->setDate(_orderDateCache, true);
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Sales Order Information"),
                                  setSales, __FILE__, __LINE__))
    {
      return UndefinedError;
    }

    if (ISORDER(_mode))
    {
      populateCMInfo();
      populateCCInfo();
      sFillCcardList();
    }

    _captive = false;
    _edit->setEnabled(false);
    _action->setEnabled(false);
    _delete->setEnabled(false);
    _close->setText("&Cancel");

    connect(_cust, SIGNAL(valid(bool)), _new, SLOT(setEnabled(bool)));
  }
  else if (ISEDIT(_mode))
  {
    _captive = true;
    _orderNumber->setEnabled(false);
    _cust->setReadOnly(true);
    _orderCurrency->setEnabled(false);

    connect(_cust, SIGNAL(valid(bool)), _new, SLOT(setEnabled(bool)));

  }
  else if (ISVIEW(_mode))
  {
    _CCAmount->hide();
    _CCCVVLit->hide();
    _CCCVV->hide();
    _newCC->hide();
    _editCC->hide();
    _viewCC->hide();
    _upCC->hide();
    _downCC->hide();
    _authorize->hide();
    _charge->hide();
    _project->setReadOnly(true);

  }

  if (ISQUOTE(_mode))
  {
    setWindowTitle(tr("Quote"));

    _comments->setType(Comments::Quote);

    _saveAndAdd->hide();
    _fromQuote->hide();
    _fromQuoteLit->hide();
    _shipComplete->hide();
    _allocatedCM->hide();
    _allocatedCMLit->hide();
    _outstandingCM->hide();
    _outstandingCMLit->hide();
    _authCC->hide();
    _authCCLit->hide();
    _balance->hide();
    _balanceLit->hide();
    _holdType->hide();
    _holdTypeLit->hide();
    _shippingCharges->hide();
    _shippingChargesLit->hide();
    _shippingForm->hide();
    _shippingFormLit->hide();

    _salesOrderInformation->removeTab(_salesOrderInformation->indexOf(_paymentPage));
    _salesOrderInformation->removeTab(_salesOrderInformation->indexOf(_shipmentsPage));
    _showCanceled->hide();
    _cancel->hide();
    _total->setBaseVisible(true);
  }
  else
  {
    _expireLit->hide();
    _expire->hide();
  }

  if (_metrics->boolean("HideSOMiscCharge"))
  {
    _miscChargeDescriptionLit->hide();
    _miscChargeDescription->hide();
    _miscChargeAccountLit->hide();
    _miscChargeAccount->hide();
    _miscChargeAmountLit->hide();
    _miscCharge->hide();
  }

  if (ISQUOTE(_mode) || !_metrics->boolean("EnableSOShipping"))
  {
    _requireInventory->hide();
    _issueStock->hide();
    _issueLineBalance->hide();
    _amountAtShippingLit->hide();
    _amountAtShipping->hide();
    _soitem->hideColumn("qtyatshipping");
    _soitem->hideColumn("balance");
    _soitem->setSelectionMode(QAbstractItemView::ExtendedSelection);
  }
  else
    _soitem->setSelectionMode(QAbstractItemView::ExtendedSelection);

  if (ISQUOTE(_mode) || !_metrics->boolean("EnableSOReservations"))
  {
    _reserveStock->hide();
    _reserveLineBalance->hide();
  }

  param = pParams.value("cust_id", &valid);
  if (valid)
  {
    _cust->setId(param.toInt());
    param = pParams.value("shipto_id", &valid);
    if (valid)
      _shipTo->setId(param.toInt());
  }

  param = pParams.value("ophead_id", &valid);
  if (valid)
    _opportunity->setId(param.toInt());

  param = pParams.value("sohead_id", &valid);
  if (valid)
  {
    _soheadid = param.toInt();
    emit newId(_soheadid);
    if (cEdit == _mode)
      setObjectName(QString("salesOrder edit %1").arg(_soheadid));
    else if (cView == _mode)
      setObjectName(QString("salesOrder view %1").arg(_soheadid));
    populate();
    populateCMInfo();
    populateCCInfo();
    sFillCcardList();
  }

  param = pParams.value("quhead_id", &valid);
  if (valid)
  {
    _soheadid = param.toInt();
    emit newId(_soheadid);
    populate();
  }

  if ( (pParams.inList("enableSaveAndAdd")) && ((_mode == cNew) || (_mode == cEdit)) )
  {
    _saveAndAdd->show();
    _saveAndAdd->setEnabled(true);
  }

  if (ISNEW(_mode) || ISEDIT(_mode))
  {
    _orderDate->setEnabled(_privileges->check("OverrideSODate"));
    _packDate->setEnabled(_privileges->check("AlterPackDate"));
  }
  else
  {
    _orderDate->setEnabled(false);
    _packDate->setEnabled(false);
  }

  param = pParams.value("captive", &valid);
  if (valid)
    _captive = true;

  return NoError;
}

/** \return one of cNew, cEdit, cView, ...
    \todo   change possible modes to an enum in guiclient.h (and add cUnknown?)
 */
int salesOrder::modeState() const
{
  if (ISNEW(_mode))
    return cNew;
  else if (ISEDIT(_mode))
    return cEdit;
  else
    return cView;
}

/** \return one of isOrder, isQuote, ...
 */
int salesOrder::modeType() const
{
  if (ISQUOTE(_mode))
    return QuoteMode;
  else
    return OrderMode;
}

void salesOrder::sSave()
{
  _saving = true;

  if (save(false))
  {
    if (_printSO->isChecked())
    {
      if (ISQUOTE(_mode))
      {
        ParameterList params;
        params.append("quhead_id", _soheadid);

        printQuote newdlgX(this, "", true);
        newdlgX.set(params);
        newdlgX.exec();
      }
      else
      {
        ParameterList params;
        params.append("sohead_id", _soheadid);

        printSoForm newdlgX(this, "", true);
        newdlgX.set(params);
        newdlgX.exec();
      }
    }

    if (_captive)
      close();
    else
      clear();
  }
}

void salesOrder::sSaveAndAdd()
{
  XSqlQuery saveSales;
  _saving = true;

  if (save(false))
  {
    saveSales.prepare("SELECT addToPackingListBatch(:sohead_id) AS result;");
    saveSales.bindValue(":sohead_id", _soheadid);
    saveSales.exec();

    if (_printSO->isChecked())
    {
      if (ISQUOTE(_mode))
      {
        ParameterList params;
        params.append("quhead_id", _soheadid);

        printQuote newdlgS(this, "", true);
        newdlgS.set(params);
        newdlgS.exec();
      }
      else
      {
        ParameterList params;
        params.append("sohead_id", _soheadid);

        printPackingList newdlgP(this, "", true);
        newdlgP.set(params);
        newdlgP.exec();

        printSoForm newdlgS(this, "", true);
        newdlgS.set(params);
        newdlgS.exec();
      }
    }

    if (_captive)
      close();
    else
      clear();
  }
}

bool salesOrder::save(bool partial)
{
  XSqlQuery saveSales;

  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(!_orderDate->isValid(), _orderDate,
                          tr("You must enter an Order Date for this order before you may save it.") )
         << GuiErrorCheck((!_shipDate->isValid()) && (_metrics->value("soPriceEffective") == "ScheduleDate"), _shipDate,
                          tr("You must enter an Scheduled Date for this order before you may save it.") )
         << GuiErrorCheck(!_cust->isValid(), _cust,
                          tr("You must select a Customer for this order before you may save it.") )
         << GuiErrorCheck(!_salesRep->isValid(), _salesRep,
                          tr("You must select a Sales Rep. for this order before you may save it.") )
         << GuiErrorCheck(!_terms->isValid(), _terms,
                          tr("You must select the Terms for this order before you may save it.") )
         << GuiErrorCheck((_shipTo->id() == -1) && (!_shipToName->isEnabled()), _shipTo,
                          tr("You must select a Ship-To for this order before you may save it.") )
         << GuiErrorCheck(!partial && _total->localValue() < 0, _cust,
                          tr("<p>The Total must be a positive value.") )
         << GuiErrorCheck(_orderNumber->text().toInt() == 0, _orderNumber,
                          tr( "<p>You must enter a valid Number for this order before you may save it." ) )
         << GuiErrorCheck((!_miscCharge->isZero()) && (!_miscChargeAccount->isValid()), _miscChargeAccount,
                          tr("<p>You may not enter a Misc. Charge without "
                             "indicating the G/L Sales Account number for the "
                             "charge.  Please set the Misc. Charge amount to 0 "
                             "or select a Misc. Charge Sales Account." ) )
         << GuiErrorCheck(_cashReceived->localValue() > 0.0, _postCash,
                          tr( "<p>You must Post Cash Payment before you may save it." ) )
         << GuiErrorCheck(!partial && !_project->isValid() && _metrics->boolean("RequireProjectAssignment"), _project,
                          tr("<p>You must enter a Project for this order before you may save it."))
  ;
  if (!(_metrics->boolean("EnableRentals") && ISORDER(_mode))) {
      errors << GuiErrorCheck(!partial && _soitem->topLevelItemCount() == 0, _new,
                           tr("<p>You must create at least one Line Item for this order before you may save it.") );
  }

  if (_opportunity->isValid())
  {
    saveSales.prepare( "SELECT crmacct_cust_id, crmacct_prospect_id "
               "FROM crmacct JOIN ophead ON (crmacct_id = ophead_crmacct_id) "
               "WHERE (ophead_id = :ophead_id);" );
    saveSales.bindValue(":ophead_id", _opportunity->id());
    saveSales.exec();
    if (saveSales.first())
    {
      if (saveSales.value("crmacct_cust_id").toInt() == 0 && saveSales.value("crmacct_prospect_id").toInt() == 0)
      {
        errors << GuiErrorCheck(true, _opportunity,
                              tr("Only opportunities from Customers or Prospects can be related.") );
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Sales Order Information"),
                                  saveSales, __FILE__, __LINE__))
    {
      return false;
    }
  }

  if (ISORDER(_mode))
  {
    if (_usesPos && !partial)
    {
      if (_custPONumber->text().trimmed().length() == 0)
      {
        errors << GuiErrorCheck(true, _custPONumber,
                                tr("You must enter a Customer P/O for this Sales Order before you may save it.") );
      }

      if (!_blanketPos)
      {
        saveSales.prepare( "SELECT cohead_id"
                   "  FROM cohead"
                   " WHERE ((cohead_cust_id=:cohead_cust_id)"
                   "   AND  (cohead_id<>:cohead_id)"
                   "   AND  (UPPER(cohead_custponumber) = UPPER(:cohead_custponumber)) )"
                   " UNION "
                   "SELECT quhead_id"
                   "  FROM quhead"
                   " WHERE ((quhead_cust_id=:cohead_cust_id)"
                   "   AND  (quhead_number<>:fromquote)"
                   "   AND  (quhead_id<>:cohead_id)"
                   "   AND  (UPPER(quhead_custponumber) = UPPER(:cohead_custponumber)) );" );
        saveSales.bindValue(":cohead_cust_id", _cust->id());
        saveSales.bindValue(":fromquote", _fromQuote->text());
        saveSales.bindValue(":cohead_id", _soheadid);
        saveSales.bindValue(":cohead_custponumber", _custPONumber->text());
        saveSales.exec();
        if (saveSales.first())
        {
          errors << GuiErrorCheck(true, _custPONumber,
                                tr("<p>This Customer does not use Blanket P/O "
                                     "Numbers and the P/O Number you entered has "
                                     "already been used for another Sales Order. "
                                     "Please verify the P/O Number and either "
                                     "enter a new P/O Number or add to the "
                                     "existing Sales Order." ) );
        }
        else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Sales Order Information"),
                                      saveSales, __FILE__, __LINE__))
        {
          return false;
        }
      }
    }

//  S/O Credit Check
    if (_saving &&
        _metrics->boolean("CreditCheckSOOnSave") && !creditLimitCheck() && _holdType->code() != "C")
    {
      if (QMessageBox::question(this, tr("Sales Order Credit Check"),
                      tr("<p>The customer has exceeded their credit limit "
                         "and this order will be placed on Credit Hold.\n"
                         "Do you wish to continue saving the order?"),
                      QMessageBox::Yes | QMessageBox::No,
                      QMessageBox::No) == QMessageBox::Yes)
        _holdType->setCode("C");
      else
        return false;
    }
  }

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Sales Order"), errors))
      return false;

  if ((_mode == cEdit) || ((_mode == cNew) && _saved))
    saveSales.prepare( "UPDATE cohead "
               "SET cohead_custponumber=:custponumber, cohead_shipto_id=:shipto_id, cohead_cust_id=:cust_id,"
               "    cohead_billtoname=:billtoname, cohead_billtoaddress1=:billtoaddress1,"
               "    cohead_billtoaddress2=:billtoaddress2, cohead_billtoaddress3=:billtoaddress3,"
               "    cohead_billtocity=:billtocity, cohead_billtostate=:billtostate, cohead_billtozipcode=:billtozipcode,"
               "    cohead_billtocountry=:billtocountry,"
               "    cohead_shiptoname=:shiptoname, cohead_shiptoaddress1=:shiptoaddress1,"
               "    cohead_shiptoaddress2=:shiptoaddress2, cohead_shiptoaddress3=:shiptoaddress3,"
               "    cohead_shiptocity=:shiptocity, cohead_shiptostate=:shiptostate, cohead_shiptozipcode=:shiptozipcode,"
               "    cohead_shiptocountry=:shiptocountry,"
               "    cohead_orderdate=:orderdate, cohead_packdate=:packdate,"
               "    cohead_salesrep_id=:salesrep_id, cohead_commission=:commission,"
               "    cohead_taxzone_id=:taxzone_id, cohead_terms_id=:terms_id,"
               "    cohead_fob=:fob, cohead_shipvia=:shipvia, cohead_warehous_id=:warehous_id,"
               "    cohead_freight=:freight, cohead_calcfreight=:calcfreight,"
               "    cohead_misc=:misc, cohead_misc_accnt_id=:misc_accnt_id, cohead_misc_descrip=:misc_descrip,"
               "    cohead_holdtype=:holdtype,"
               "    cohead_ordercomments=:ordercomments, cohead_shipcomments=:shipcomments,"
               "    cohead_shipchrg_id=:shipchrg_id, cohead_shipform_id=:shipform_id,"
               "    cohead_prj_id=:prj_id,"
               "    cohead_ophead_id=:ophead_id,"
               "    cohead_curr_id = :curr_id,"
               "    cohead_shipcomplete=:cohead_shipcomplete,"
               "    cohead_shipto_cntct_id=:shipto_cntct_id,"
               "    cohead_shipto_cntct_honorific=:shipto_cntct_honorific,"
               "    cohead_shipto_cntct_first_name=:shipto_cntct_first_name,"
               "    cohead_shipto_cntct_middle=:shipto_cntct_middle,"
               "    cohead_shipto_cntct_last_name=:shipto_cntct_last_name,"
               "    cohead_shipto_cntct_suffix=:shipto_cntct_suffix,"
               "    cohead_shipto_cntct_phone=:shipto_cntct_phone,"
               "    cohead_shipto_cntct_title=:shipto_cntct_title,"
               "    cohead_shipto_cntct_fax=:shipto_cntct_fax,"
               "    cohead_shipto_cntct_email=:shipto_cntct_email,"
               "    cohead_billto_cntct_id=:billto_cntct_id,"
               "    cohead_billto_cntct_honorific=:billto_cntct_honorific,"
               "    cohead_billto_cntct_first_name=:billto_cntct_first_name,"
               "    cohead_billto_cntct_middle=:billto_cntct_middle,"
               "    cohead_billto_cntct_last_name=:billto_cntct_last_name,"
               "    cohead_billto_cntct_suffix=:billto_cntct_suffix,"
               "    cohead_billto_cntct_phone=:billto_cntct_phone,"
               "    cohead_billto_cntct_title=:billto_cntct_title,"
               "    cohead_billto_cntct_fax=:billto_cntct_fax,"
               "    cohead_billto_cntct_email=:billto_cntct_email, "
               "    cohead_shipzone_id=:shipzone_id,"
               "    cohead_saletype_id=:saletype_id "
               "WHERE (cohead_id=:id);" );
  else if (_mode == cNew)
  {
    saveSales.prepare("INSERT INTO cohead "
              "(cohead_id, cohead_number, cohead_cust_id,"
              "    cohead_custponumber, cohead_shipto_id,"
              "    cohead_billtoname, cohead_billtoaddress1,"
              "    cohead_billtoaddress2, cohead_billtoaddress3,"
              "    cohead_billtocity, cohead_billtostate, cohead_billtozipcode,"
              "    cohead_billtocountry,"
              "    cohead_shiptoname, cohead_shiptoaddress1,"
              "    cohead_shiptoaddress2, cohead_shiptoaddress3,"
              "    cohead_shiptocity, cohead_shiptostate, cohead_shiptozipcode,"
              "    cohead_shiptocountry,"
              "    cohead_orderdate, cohead_packdate,"
              "    cohead_salesrep_id, cohead_commission,"
              "    cohead_taxzone_id, cohead_terms_id,"
              "    cohead_fob, cohead_shipvia, cohead_warehous_id,"
              "    cohead_freight, cohead_calcfreight,"
              "    cohead_misc, cohead_misc_accnt_id, cohead_misc_descrip,"
              "    cohead_holdtype,"
              "    cohead_ordercomments, cohead_shipcomments,"
              "    cohead_shipchrg_id, cohead_shipform_id,"
              "    cohead_prj_id, cohead_ophead_id,"
              "    cohead_curr_id,"
              "    cohead_shipcomplete,"
              "    cohead_shipto_cntct_id,"
              "    cohead_shipto_cntct_honorific,"
              "    cohead_shipto_cntct_first_name,"
              "    cohead_shipto_cntct_middle,"
              "    cohead_shipto_cntct_last_name,"
              "    cohead_shipto_cntct_suffix,"
              "    cohead_shipto_cntct_phone,"
              "    cohead_shipto_cntct_title,"
              "    cohead_shipto_cntct_fax,"
              "    cohead_shipto_cntct_email,"
              "    cohead_billto_cntct_id,"
              "    cohead_billto_cntct_honorific,"
              "    cohead_billto_cntct_first_name,"
              "    cohead_billto_cntct_middle,"
              "    cohead_billto_cntct_last_name,"
              "    cohead_billto_cntct_suffix,"
              "    cohead_billto_cntct_phone,"
              "    cohead_billto_cntct_title,"
              "    cohead_billto_cntct_fax,"
              "    cohead_billto_cntct_email,"
              "    cohead_shipzone_id, cohead_saletype_id)"
              "    VALUES (:id,:number, :cust_id,"
              "    :custponumber,:shipto_id,"
              "    :billtoname, :billtoaddress1,"
              "    :billtoaddress2, :billtoaddress3,"
              "    :billtocity, :billtostate, :billtozipcode,"
              "    :billtocountry,"
              "    :shiptoname, :shiptoaddress1,"
              "    :shiptoaddress2, :shiptoaddress3,"
              "    :shiptocity, :shiptostate, :shiptozipcode,"
              "    :shiptocountry,"
              "    :orderdate, :packdate,"
              "    :salesrep_id, :commission,"
              "    :taxzone_id, :terms_id,"
              "    :fob, :shipvia, :warehous_id,"
              "    :freight, :calcfreight,"
              "    :misc, :misc_accnt_id, :misc_descrip,"
              "    :holdtype,"
              "    :ordercomments, :shipcomments,"
              "    :shipchrg_id, :shipform_id,"
              "    :prj_id, :ophead_id,"
              "    :curr_id,"
              "    :cohead_shipcomplete,"
              "    :shipto_cntct_id,"
              "    :shipto_cntct_honorific,"
              "    :shipto_cntct_first_name,"
              "    :shipto_cntct_middle,"
              "    :shipto_cntct_last_name,"
              "    :shipto_cntct_suffix,"
              "    :shipto_cntct_phone,"
              "    :shipto_cntct_title,"
              "    :shipto_cntct_fax,"
              "    :shipto_cntct_email,"
              "    :billto_cntct_id,"
              "    :billto_cntct_honorific,"
              "    :billto_cntct_first_name,"
              "    :billto_cntct_middle,"
              "    :billto_cntct_last_name,"
              "    :billto_cntct_suffix,"
              "    :billto_cntct_phone,"
              "    :billto_cntct_title,"
              "    :billto_cntct_fax,"
              "    :billto_cntct_email,"
              "    :shipzone_id, :saletype_id) ");
  }
  else if ((_mode == cEditQuote) || ((_mode == cNewQuote) && _saved))
    saveSales.prepare( "UPDATE quhead "
               "SET quhead_custponumber=:custponumber, quhead_shipto_id=:shipto_id, quhead_cust_id=:cust_id,"
               "    quhead_billtoname=:billtoname, quhead_billtoaddress1=:billtoaddress1,"
               "    quhead_billtoaddress2=:billtoaddress2, quhead_billtoaddress3=:billtoaddress3,"
               "    quhead_billtocity=:billtocity, quhead_billtostate=:billtostate, quhead_billtozip=:billtozipcode,"
               "    quhead_billtocountry=:billtocountry,"
               "    quhead_shiptoname=:shiptoname, quhead_shiptoaddress1=:shiptoaddress1,"
               "    quhead_shiptoaddress2=:shiptoaddress2, quhead_shiptoaddress3=:shiptoaddress3,"
               "    quhead_shiptocity=:shiptocity, quhead_shiptostate=:shiptostate, quhead_shiptozipcode=:shiptozipcode,"
               "    quhead_shiptocountry=:shiptocountry,"
               "    quhead_quotedate=:orderdate, quhead_packdate=:packdate,"
               "    quhead_salesrep_id=:salesrep_id, quhead_commission=:commission,"
               "    quhead_taxzone_id=:taxzone_id, quhead_terms_id=:terms_id,"
               "    quhead_shipvia=:shipvia, quhead_fob=:fob,"
               "    quhead_freight=:freight, quhead_calcfreight=:calcfreight,"
               "    quhead_misc=:misc, quhead_misc_accnt_id=:misc_accnt_id, quhead_misc_descrip=:misc_descrip,"
               "    quhead_ordercomments=:ordercomments, quhead_shipcomments=:shipcomments,"
               "    quhead_prj_id=:prj_id, quhead_ophead_id=:ophead_id, quhead_warehous_id=:warehous_id,"
               "    quhead_curr_id = :curr_id, quhead_expire=:expire,"
               "    quhead_shipto_cntct_id=:shipto_cntct_id,"
               "    quhead_shipto_cntct_honorific=:shipto_cntct_honorific,"
               "    quhead_shipto_cntct_first_name=:shipto_cntct_first_name,"
               "    quhead_shipto_cntct_middle=:shipto_cntct_middle,"
               "    quhead_shipto_cntct_last_name=:shipto_cntct_last_name,"
               "    quhead_shipto_cntct_suffix=:shipto_cntct_suffix,"
               "    quhead_shipto_cntct_phone=:shipto_cntct_phone,"
               "    quhead_shipto_cntct_title=:shipto_cntct_title,"
               "    quhead_shipto_cntct_fax=:shipto_cntct_fax,"
               "    quhead_shipto_cntct_email=:shipto_cntct_email,"
               "    quhead_billto_cntct_id=:billto_cntct_id,"
               "    quhead_billto_cntct_honorific=:billto_cntct_honorific,"
               "    quhead_billto_cntct_first_name=:billto_cntct_first_name,"
               "    quhead_billto_cntct_middle=:billto_cntct_middle,"
               "    quhead_billto_cntct_last_name=:billto_cntct_last_name,"
               "    quhead_billto_cntct_suffix=:billto_cntct_suffix,"
               "    quhead_billto_cntct_phone=:billto_cntct_phone,"
               "    quhead_billto_cntct_title=:billto_cntct_title,"
               "    quhead_billto_cntct_fax=:billto_cntct_fax,"
               "    quhead_billto_cntct_email=:billto_cntct_email,"
               "    quhead_shipzone_id=:shipzone_id,"
               "    quhead_saletype_id=:saletype_id "
               "WHERE (quhead_id=:id);" );
  else if (_mode == cNewQuote)
    saveSales.prepare( "INSERT INTO quhead ("
               "    quhead_id, quhead_number, quhead_cust_id,"
               "    quhead_custponumber, quhead_shipto_id,"
               "    quhead_billtoname, quhead_billtoaddress1,"
               "    quhead_billtoaddress2, quhead_billtoaddress3,"
               "    quhead_billtocity, quhead_billtostate, quhead_billtozip,"
               "    quhead_billtocountry,"
               "    quhead_shiptoname, quhead_shiptoaddress1,"
               "    quhead_shiptoaddress2, quhead_shiptoaddress3,"
               "    quhead_shiptocity, quhead_shiptostate, quhead_shiptozipcode,"
               "    quhead_shiptocountry,"
               "    quhead_quotedate, quhead_packdate,"
               "    quhead_salesrep_id, quhead_commission,"
               "    quhead_taxzone_id, quhead_terms_id,"
               "    quhead_shipvia, quhead_fob,"
               "    quhead_freight, quhead_calcfreight,"
               "    quhead_misc, quhead_misc_accnt_id, quhead_misc_descrip,"
               "    quhead_ordercomments, quhead_shipcomments,"
               "    quhead_prj_id, quhead_ophead_id, quhead_warehous_id,"
               "    quhead_curr_id, quhead_expire,"
               "    quhead_shipto_cntct_id,"
               "    quhead_shipto_cntct_honorific,"
               "    quhead_shipto_cntct_first_name,"
               "    quhead_shipto_cntct_middle,"
               "    quhead_shipto_cntct_last_name,"
               "    quhead_shipto_cntct_suffix,"
               "    quhead_shipto_cntct_phone,"
               "    quhead_shipto_cntct_title,"
               "    quhead_shipto_cntct_fax,"
               "    quhead_shipto_cntct_email,"
               "    quhead_billto_cntct_id,"
               "    quhead_billto_cntct_honorific,"
               "    quhead_billto_cntct_first_name,"
               "    quhead_billto_cntct_middle,"
               "    quhead_billto_cntct_last_name,"
               "    quhead_billto_cntct_suffix,"
               "    quhead_billto_cntct_phone,"
               "    quhead_billto_cntct_title,"
               "    quhead_billto_cntct_fax,"
               "    quhead_billto_cntct_email,"
               "    quhead_status,"
               "    quhead_shipzone_id, quhead_saletype_id)"
               "    VALUES ("
               "    :id, :number, :cust_id,"
               "    :custponumber, :shipto_id,"
               "    :billtoname, :billtoaddress1,"
               "    :billtoaddress2, :billtoaddress3,"
               "    :billtocity, :billtostate, :billtozipcode,"
               "    :billtocountry,"
               "    :shiptoname, :shiptoaddress1,"
               "    :shiptoaddress2, :shiptoaddress3,"
               "    :shiptocity, :shiptostate, :shiptozipcode,"
               "    :shiptocountry,"
               "    :orderdate, :packdate,"
               "    :salesrep_id, :commission,"
               "    :taxzone_id, :terms_id,"
               "    :shipvia, :fob,"
               "    :freight, :calcfreight,"
               "    :misc, :misc_accnt_id, :misc_descrip,"
               "    :ordercomments, :shipcomments,"
               "    :prj_id, :ophead_id, :warehous_id,"
               "    :curr_id, :expire,"
               "    :shipto_cntct_id,"
               "    :shipto_cntct_honorific,"
               "    :shipto_cntct_first_name,"
               "    :shipto_cntct_middle,"
               "    :shipto_cntct_last_name,"
               "    :shipto_cntct_suffix,"
               "    :shipto_cntct_phone,"
               "    :shipto_cntct_title,"
               "    :shipto_cntct_fax,"
               "    :shipto_cntct_email,"
               "    :billto_cntct_id,"
               "    :billto_cntct_honorific,"
               "    :billto_cntct_first_name,"
               "    :billto_cntct_middle,"
               "    :billto_cntct_last_name,"
               "    :billto_cntct_suffix,"
               "    :billto_cntct_phone,"
               "    :billto_cntct_title,"
               "    :billto_cntct_fax,"
               "    :billto_cntct_email,"
               "    :quhead_status,"
               "    :shipzone_id, :saletype_id) ");
  saveSales.bindValue(":id", _soheadid );
  saveSales.bindValue(":number", _orderNumber->text());
  saveSales.bindValue(":orderdate", _orderDate->date());

  if (_packDate->isValid())
    saveSales.bindValue(":packdate", _packDate->date());
  else
    saveSales.bindValue(":packdate", _orderDate->date());

  saveSales.bindValue(":cust_id", _cust->id());
  if (_warehouse->id() != -1)
    saveSales.bindValue(":warehous_id", _warehouse->id());
  saveSales.bindValue(":custponumber", _custPONumber->text().trimmed());
  if (_shipTo->id() > 0)
    saveSales.bindValue(":shipto_id", _shipTo->id());
  saveSales.bindValue(":billtoname",                _billToName->text());
  saveSales.bindValue(":billtoaddress1",        _billToAddr->line1());
  saveSales.bindValue(":billtoaddress2",        _billToAddr->line2());
  saveSales.bindValue(":billtoaddress3",        _billToAddr->line3());
  saveSales.bindValue(":billtocity",                _billToAddr->city());
  saveSales.bindValue(":billtostate",                _billToAddr->state());
  saveSales.bindValue(":billtozipcode",                _billToAddr->postalCode());
  saveSales.bindValue(":billtocountry",                _billToAddr->country());
  saveSales.bindValue(":shiptoname",                _shipToName->text());
  saveSales.bindValue(":shiptoaddress1",        _shipToAddr->line1());
  saveSales.bindValue(":shiptoaddress2",        _shipToAddr->line2());
  saveSales.bindValue(":shiptoaddress3",        _shipToAddr->line3());
  saveSales.bindValue(":shiptocity",                _shipToAddr->city());
  saveSales.bindValue(":shiptostate",                _shipToAddr->state());
  saveSales.bindValue(":shiptozipcode",                _shipToAddr->postalCode());
  saveSales.bindValue(":shiptocountry",                _shipToAddr->country());
  saveSales.bindValue(":ordercomments", _orderComments->toPlainText());
  saveSales.bindValue(":shipcomments", _shippingComments->toPlainText());
  saveSales.bindValue(":fob", _fob->text());
  saveSales.bindValue(":shipvia", _shipVia->currentText());

  if (_shipToCntct->isValid())
    saveSales.bindValue(":shipto_cntct_id", _shipToCntct->id());

  saveSales.bindValue(":shipto_cntct_honorific", _shipToCntct->honorific());
  saveSales.bindValue(":shipto_cntct_first_name", _shipToCntct->first());
  saveSales.bindValue(":shipto_cntct_middle", _shipToCntct->middle());
  saveSales.bindValue(":shipto_cntct_last_name", _shipToCntct->last());
  saveSales.bindValue(":shipto_cntct_suffix", _shipToCntct->suffix());
  saveSales.bindValue(":shipto_cntct_phone", _shipToCntct->phone());
  saveSales.bindValue(":shipto_cntct_title", _shipToCntct->title());
  saveSales.bindValue(":shipto_cntct_fax", _shipToCntct->fax());
  saveSales.bindValue(":shipto_cntct_email", _shipToCntct->emailAddress());

  if (_billToCntct->isValid())
    saveSales.bindValue(":billto_cntct_id", _billToCntct->id());

  saveSales.bindValue(":billto_cntct_honorific", _billToCntct->honorific());
  saveSales.bindValue(":billto_cntct_first_name", _billToCntct->first());
  saveSales.bindValue(":billto_cntct_middle", _billToCntct->middle());
  saveSales.bindValue(":billto_cntct_last_name", _billToCntct->last());
  saveSales.bindValue(":billto_cntct_suffix", _billToCntct->suffix());
  saveSales.bindValue(":billto_cntct_phone", _billToCntct->phone());
  saveSales.bindValue(":billto_cntct_title", _billToCntct->title());
  saveSales.bindValue(":billto_cntct_fax", _billToCntct->fax());
  saveSales.bindValue(":billto_cntct_email", _billToCntct->emailAddress());

  if (_salesRep->id() != -1)
    saveSales.bindValue(":salesrep_id", _salesRep->id());
  if (_taxZone->isValid())
    saveSales.bindValue(":taxzone_id", _taxZone->id());
  if (_terms->id() != -1)
    saveSales.bindValue(":terms_id", _terms->id());
  saveSales.bindValue(":shipchrg_id", _shippingCharges->id());
  if(_shippingForm->id() > 0)
    saveSales.bindValue(":shipform_id", _shippingForm->id());
  saveSales.bindValue(":freight", _freight->localValue());
  saveSales.bindValue(":calcfreight", _calcfreight);
  saveSales.bindValue(":commission", (_commission->toDouble() / 100.0));
  saveSales.bindValue(":misc", _miscCharge->localValue());
  if (_miscChargeAccount->id() != -1)
    saveSales.bindValue(":misc_accnt_id", _miscChargeAccount->id());
  saveSales.bindValue(":misc_descrip", _miscChargeDescription->text().trimmed());
  saveSales.bindValue(":curr_id", _orderCurrency->id());
  saveSales.bindValue(":cohead_shipcomplete", QVariant(_shipComplete->isChecked()));
  if (_project->isValid())
    saveSales.bindValue(":prj_id", _project->id());
  if (_opportunity->isValid())
    saveSales.bindValue(":ophead_id", _opportunity->id());
  if (_expire->isValid())
    saveSales.bindValue(":expire", _expire->date());

  saveSales.bindValue(":holdtype", _holdType->code());

  if(_shippingZone->isValid())
    saveSales.bindValue(":shipzone_id", _shippingZone->id());
  if(_saleType->isValid())
    saveSales.bindValue(":saletype_id", _saleType->id());
  saveSales.bindValue(":quhead_status", "O");

  saveSales.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Saving Order"),
                           saveSales, __FILE__, __LINE__))
  {
    return false;
  }

  // TODO: should this be done before saveSales.exec()?
  if (ISNEW(_mode) && (!_saved)
      && ! _lock.acquire(ISORDER(_mode) ? "cohead" : "quhead", _soheadid,
                         AppLock::Interactive))
  {
    return false;
  }

  _saved = true;

  if (!partial)
  {
    if ((cNew == _mode) && _metrics->boolean("AutoAllocateCreditMemos"))
    {
      sAllocateCreditMemos();
    }

    if ( (_mode == cNew) || (_mode == cEdit) )
    {
      disconnect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sHandleSalesOrderEvent(int, bool)));
      omfgThis->sSalesOrdersUpdated(_soheadid);
      connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sHandleSalesOrderEvent(int, bool)));
      omfgThis->sProjectsUpdated(_soheadid);
    }
    else if ( (_mode == cNewQuote) || (_mode == cEditQuote) )
      omfgThis->sQuotesUpdated(-1);
  }
  else
  {
    populateCMInfo();
    populateCCInfo();

    if (_mode == cNew && _metrics->boolean("AutoCreateProjectsForOrders"))
    {
      saveSales.prepare("SELECT cohead_prj_id "
                        "FROM cohead "
                        "WHERE cohead_id=:head_id;");
      saveSales.bindValue(":head_id", _soheadid);
      saveSales.exec();
      if (saveSales.first())
        _project->setId(saveSales.value("cohead_prj_id").toInt());
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Sales Order Information"),
                                    saveSales, __FILE__, __LINE__))
      {
        return false;
      }
    }
    if (_mode == cNewQuote && _metrics->boolean("AutoCreateProjectsForOrders"))
    {
      saveSales.prepare("SELECT quhead_prj_id "
                        "FROM quhead "
                        "WHERE quhead_id=:head_id;");
      saveSales.bindValue(":head_id", _soheadid);
      saveSales.exec();
      if (saveSales.first())
        _project->setId(saveSales.value("quhead_prj_id").toInt());
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Sales Order Information"),
                                    saveSales, __FILE__, __LINE__))
      {
        return false;
      }
    }
  }

  emit saved(_soheadid);

  return true;
}

void salesOrder::sPopulateMenu(QMenu *pMenu)
{
  if (_mode == cView &&
      _numSelected == 1 &&
      _lineMode == cClosed)
        pMenu->addAction(tr("Open Line..."), this, SLOT(sAction()));

  if ((_mode == cNew) || (_mode == cEdit))
  {
    QAction *menuItem;
    bool  didsomething = false;
    if (_numSelected == 1)
    {
      didsomething = true;
      if (_lineMode == cClosed)
        pMenu->addAction(tr("Open Line..."), this, SLOT(sAction()));
      else if (_lineMode == cActiveOpen)
      {
        if (_lineFirm == 1)
        {
          pMenu->addAction(tr("Edit Line..."), this, SLOT(sEdit()));
          menuItem = pMenu->addAction(tr("Firm Line..."), this, SLOT(sFirm()));
          menuItem->setEnabled(_privileges->check("FirmSalesOrder"));
        }
        else
        {
          menuItem = pMenu->addAction(tr("Soften Line..."), this, SLOT(sSoften()));
          menuItem->setEnabled(_privileges->check("FirmSalesOrder"));
        }
        pMenu->addAction(tr("Close Line..."), this, SLOT(sAction()));
      }
      else if (_lineMode == cInactiveOpen)
      {
        if (_lineFirm == 1)
        {
          pMenu->addAction(tr("Edit Line..."), this, SLOT(sEdit()));
          menuItem = pMenu->addAction(tr("Firm Line..."), this, SLOT(sFirm()));
          menuItem->setEnabled(_privileges->check("FirmSalesOrder"));
        }
        else
        {
          menuItem = pMenu->addAction(tr("Soften Line..."), this, SLOT(sSoften()));
          menuItem->setEnabled(_privileges->check("FirmSalesOrder"));
        }
        pMenu->addAction(tr("Close Line..."), this, SLOT(sAction()));
        pMenu->addAction(tr("Cancel Line..."), this, SLOT(sCancel()));

        XSqlQuery invhist;
        invhist.prepare("SELECT 1 "
                        "  FROM invhist "
                        " WHERE invhist_ordnumber=formatSoNumber(:soitemid) "
                        "   AND invhist_ordtype='SO';");
        invhist.bindValue(":soitemid", _soitem->id());
        invhist.exec();
        if (!invhist.first())
        {
          if(ErrorReporter::error(QtCriticalMsg, this, "Error checking invhist",
                                  invhist, __FILE__, __LINE__))
            return;
          pMenu->addAction(tr("Delete Line..."), this, SLOT(sDelete()));
        }
      }
    }

    if (_metrics->boolean("EnableSOReservations"))
    {
      if (didsomething)
        pMenu->addSeparator();

      menuItem = pMenu->addAction(tr("Show Reservations..."), this, SLOT(sShowReservations()));

      pMenu->addSeparator();

      menuItem = pMenu->addAction(tr("Unreserve Stock"), this, SLOT(sUnreserveStock()));
      menuItem->setEnabled(_privileges->check("MaintainReservations"));
      menuItem = pMenu->addAction(tr("Reserve Stock..."), this, SLOT(sReserveStock()));
      menuItem->setEnabled(_privileges->check("MaintainReservations"));
      menuItem = pMenu->addAction(tr("Reserve Line Balance"), this, SLOT(sReserveLineBalance()));
      menuItem->setEnabled(_privileges->check("MaintainReservations"));

      didsomething = true;
    }

    if (_metrics->boolean("EnableSOShipping"))
    {
      if (didsomething)
        pMenu->addSeparator();

      menuItem = pMenu->addAction(tr("Return Stock"), this, SLOT(sReturnStock()));
      menuItem->setEnabled(_privileges->check("IssueStockToShipping"));
      menuItem = pMenu->addAction(tr("Issue Stock..."), this, SLOT(sIssueStock()));
      menuItem->setEnabled(_privileges->check("IssueStockToShipping"));
      menuItem = pMenu->addAction(tr("Issue Line Balance"), this, SLOT(sIssueLineBalance()));
      menuItem->setEnabled(_privileges->check("IssueStockToShipping"));

      didsomething = true;
    }
    XSqlQuery createOrder;
    createOrder.prepare( "SELECT coitem_order_type "
                         "FROM coitem "
                         "WHERE (coitem_id=:coitem_id);");
    QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      createOrder.bindValue(":coitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
      createOrder.exec();
      if (createOrder.first())
      {
        if (didsomething)
          pMenu->addSeparator();
        if (createOrder.value("coitem_order_type").toString() == "P")
        {
          XSqlQuery checkPO;
          checkPO.prepare( "SELECT pohead_id "
                           "FROM pohead JOIN poitem ON (pohead_id=poitem_pohead_id) "
                           "     RIGHT OUTER JOIN coitem ON (poitem_id=coitem_order_id) "
                           "WHERE (coitem_id=:coitem_id);" );
          checkPO.bindValue(":coitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
          checkPO.exec();
          if (checkPO.first())
          {
            menuItem = pMenu->addAction(tr("View Purchase Order..."), this, SLOT(sViewPO()));
            menuItem->setEnabled(_privileges->check("ViewPurchaseOrders"));

            menuItem = pMenu->addAction(tr("Edit Purchase Order..."), this, SLOT(sMaintainPO()));
            menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));
          }
          else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Purchase Order Information"),
                                        checkPO, __FILE__, __LINE__))
          {
            return;
          }
        }
        else if (createOrder.value("coitem_order_type").toString() == "R")
        {
          XSqlQuery checkPR;
          checkPR.prepare( "SELECT pr_id "
                           "FROM pr JOIN coitem ON (pr_id=coitem_order_id) "
                           "WHERE (coitem_id=:coitem_id);" );
          checkPR.bindValue(":coitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
          checkPR.exec();
          if (checkPR.first())
          {
            menuItem = pMenu->addAction(tr("Release P/R..."), this, SLOT(sReleasePR()));
            menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));

            menuItem = pMenu->addAction(tr("View Purchase Request..."), this, SLOT(sViewPR()));
            menuItem->setEnabled(_privileges->check("ViewPurchaseRequests"));
          }
          else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Purchase Request Information"),
                                        checkPR, __FILE__, __LINE__))
          {
            return;
          }
        }
        else if (createOrder.value("coitem_order_type").toString() == "W")
        {
          XSqlQuery checkWO;
          checkWO.prepare( "SELECT wo_id "
                           "FROM wo JOIN coitem ON (wo_id=coitem_order_id)"
                           "WHERE (coitem_id=:coitem_id);" );
          checkWO.bindValue(":coitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
          checkWO.exec();
          if (checkWO.first())
          {
            menuItem = pMenu->addAction(tr("View Work Order..."), this, SLOT(sViewWO()));
            menuItem->setEnabled(_privileges->check("ViewWorkOrders"));

            menuItem = pMenu->addAction(tr("Edit Work Order..."), this, SLOT(sMaintainWO()));
            menuItem->setEnabled(_privileges->check("MaintainWorkOrders"));
          }
          else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Work Order Information"),
                                        checkWO, __FILE__, __LINE__))
          {
            return;
          }
        }
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Sales Order Information"),
                                    createOrder, __FILE__, __LINE__))
      {
        return;
      }
      didsomething = true;
    }
    if (didsomething)
      pMenu->addSeparator();
    menuItem = pMenu->addAction(tr("Item Workbench"), this, SLOT(sViewItemWorkbench()));
    menuItem->setEnabled(_privileges->check("ViewItemAvailabilityWorkbench"));
  }
}

void salesOrder::populateOrderNumber()
{
  XSqlQuery populateSales;
  if (_mode == cNew)
  {
    if ( (_metrics->value("CONumberGeneration") == "A") ||
         (_metrics->value("CONumberGeneration") == "O")   )
    {
      populateSales.exec("SELECT fetchSoNumber() AS sonumber;");
      if (populateSales.first())
      {
        _orderNumber->setText(populateSales.value("sonumber").toString());
        _orderNumberGen = populateSales.value("sonumber").toInt();
        _userEnteredOrderNumber = false;

        if (_metrics->value("CONumberGeneration") == "A")
          _orderNumber->setEnabled(false);
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Sales Order Information"),
                                    populateSales, __FILE__, __LINE__))
      {
        return;
      }
    }
    _userEnteredOrderNumber = false;
  }
  else if (_mode == cNewQuote)
  {
    if ( (_metrics->value("QUNumberGeneration") == "A") ||
         (_metrics->value("QUNumberGeneration") == "O") ||
         (_metrics->value("QUNumberGeneration") == "S") )
    {
      if (_metrics->value("QUNumberGeneration") == "S")
        populateSales.prepare("SELECT fetchSoNumber() AS qunumber;");
      else
        populateSales.prepare("SELECT fetchQuNumber() AS qunumber;");

      populateSales.exec();
      if (populateSales.first())
      {
        _orderNumber->setText(populateSales.value("qunumber").toString());
        _orderNumberGen = populateSales.value("qunumber").toInt();
        _userEnteredOrderNumber = false;

        if ( (_metrics->value("QUNumberGeneration") == "A") ||
             (_metrics->value("QUNumberGeneration") == "S") )
          _orderNumber->setEnabled(false);
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Quote Information"),
                                    populateSales, __FILE__, __LINE__))
      {
        return;
      }
      _userEnteredOrderNumber = false;
    }
  }
}

void salesOrder::sSetUserEnteredOrderNumber()
{
  _userEnteredOrderNumber = true;
}

void salesOrder::sHandleOrderNumber()
{
  if (_ignoreSignals || !isActiveWindow())
    return;

  if (_orderNumber->text().length() == 0)
  {
    if (_mode == cNew)
    {
      if ( (_metrics->value("CONumberGeneration") == "A") ||
           (_metrics->value("CONumberGeneration") == "O") )
        populateOrderNumber();
      else
      {
        QMessageBox::warning( this, tr("Enter S/O #"),
                              tr( "You must enter a S/O # for this Sales Order before you may continue." ) );
        _orderNumber->setFocus();
        return;
      }
    }
    else if (_mode == cNewQuote)
    {
      if ( (_metrics->value("QUNumberGeneration") == "A") ||
           (_metrics->value("QUNumberGeneration") == "O") ||
           (_metrics->value("QUNumberGeneration") == "S") )
        populateOrderNumber();
      else
      {
        QMessageBox::warning( this, tr("Enter Quote #"),
                              tr( "You must enter a Quote # for this Quote before you may continue." ) );
        _orderNumber->setFocus();
        return;
      }
    }
  }
  else
  {
    XSqlQuery query;
    if (_mode == cNew && _userEnteredOrderNumber)
    {
      query.prepare("SELECT deleteSO(:sohead_id, :sohead_number ::text) AS result;");
      query.bindValue(":sohead_id", _soheadid);
      if (_orderNumberGen)
        query.bindValue(":sohead_number", _orderNumberGen);
      else
        query.bindValue(":sohead_number", _orderNumber->text());

      query.exec();
      if (query.first())
      {
        int result = query.value("result").toInt();
        if (result < 0)
        {
          ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Sales Order Information"),
                                 storedProcErrorLookup("deleteSO", result),
                                 __FILE__, __LINE__);
          return;
        }
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Sales Order Information"),
                                    query, __FILE__, __LINE__))
      {
        return;
      }

      query.prepare( "SELECT cohead_id "
                     "FROM cohead "
                     "WHERE (cohead_number=:cohead_number);" );
      query.bindValue(":cohead_number", _orderNumber->text());
      query.exec();
      if (query.first())
      {
        _mode      = cEdit;
        emit newModeType(OrderMode);
        emit newModeState(cEdit);
        _soheadid  = query.value("cohead_id").toInt();
        populate();
        _orderNumber->setEnabled(false);
        _cust->setReadOnly(true);
        populateCMInfo();
        populateCCInfo();
        sFillCcardList();
      }
      else
      {
        _orderNumber->setEnabled(false);
        if (_metrics->value("CONumberGeneration") == "O")
        {
          _userEnteredOrderNumber = false;
        }
      }
    }
    else if ((_mode == cNewQuote) && (_userEnteredOrderNumber))
    {
      query.prepare("SELECT deleteQuote(:quhead_id, :quhead_number) AS result;");
      query.bindValue(":quhead_id", _soheadid);
      query.bindValue(":quhead_number", _orderNumberGen);
      query.exec();
      if (query.first())
      {
        int result = query.value("result").toInt();
        if (result < 0)
        {
          ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Quote Information"),
                                 storedProcErrorLookup("deleteQuote", result),
                                 __FILE__, __LINE__);
          return;
        }
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Quote Information"),
                                    query, __FILE__, __LINE__))
      {
        return;
      }

      query.prepare( "SELECT quhead_id "
                     "FROM quhead "
                     "WHERE (quhead_number=:quhead_number);" );
      query.bindValue(":quhead_number", _orderNumber->text());
      query.exec();
      if (query.first())
      {
        QMessageBox::warning( this, tr("Quote Order Number Already exists."),
                              tr( "<p>The Quote Order Number you have entered "
                                    "already exists. Please enter a new one." ) );
          clear();
        _orderNumber->setFocus();
        return;
      }
      else
      {
        QString orderNumber = _orderNumber->text();
        if ((_metrics->value("QUNumberGeneration") == "S") ||
            (_metrics->value("QUNumberGeneration") == "A"))
        {
          clear();
          _orderNumber->setText(orderNumber);
          _userEnteredOrderNumber = false;
          _orderNumber->setEnabled(false);
        }
        else
        {
          _orderNumber->setText(orderNumber);
          _mode = cNewQuote;
          emit newModeType(QuoteMode);
          emit newModeState(cNew);
          _orderNumber->setEnabled(false);
        }
      }
    }
  }
}

void salesOrder::sPopulateFOB(int pWarehousid)
{
  XSqlQuery fob;
  fob.prepare( "SELECT warehous_fob "
               "FROM whsinfo "
               "WHERE (warehous_id=:warehous_id);" );
  fob.bindValue(":warehous_id", pWarehousid);
  fob.exec();
  if (fob.first())
    _fob->setText(fob.value("warehous_fob"));
}

// Is the first SELECT here responsible for the bug where the Currency kept disappearing?
void salesOrder::sPopulateCustomerInfo(int pCustid)
{
  _holdType->setCode("N");

  if (_cust->isValid())
  {
    QString sql("SELECT cust_name, addr_id, "
                "       cust_salesrep_id, cust_shipchrg_id, cust_shipform_id,"
                "       cust_commprcnt AS commission,"
                "       cust_creditstatus, cust_terms_id,"
                "       cust_taxzone_id, cust_cntct_id,"
                "       cust_ffshipto, cust_ffbillto, cust_usespos,"
                "       cust_blanketpos, cust_shipvia,"
                "       COALESCE(shipto_id, -1) AS shiptoid,"
                "       cust_preferred_warehous_id, "
                "       cust_curr_id, COALESCE(crmacct_id,-1) AS crmacct_id, "
                "       true AS iscustomer "
                "FROM custinfo "
                "  LEFT OUTER JOIN cntct  ON (cust_cntct_id=cntct_id) "
                "  LEFT OUTER JOIN addr   ON (cntct_addr_id=addr_id) "
                "  LEFT OUTER JOIN shiptoinfo ON ((shipto_cust_id=cust_id)"
                "                         AND (shipto_default)) "
                "LEFT OUTER JOIN crmacct ON (crmacct_cust_id = cust_id) "
                "WHERE (cust_id=<? value('cust_id') ?>) "
                "<? if exists('isQuote') ?>"
                "UNION "
                "SELECT prospect_name AS cust_name, addr_id, "
                "       prospect_salesrep_id AS cust_salesrep_id, NULL AS cust_shipchrg_id,"
                "       NULL AS cust_shipform_id,"
                "       0.0 AS commission,"
                "       NULL AS cust_creditstatus, NULL AS cust_terms_id,"
                "       prospect_taxzone_id AS cust_taxzone_id, prospect_cntct_id AS cust_cntct_id, "
                "       true AS cust_ffshipto, true AS cust_ffbillto, "
                "       NULL AS cust_usespos, NULL AS cust_blanketpos,"
                "       NULL AS cust_shipvia,"
                "       -1 AS shiptoid,"
                "       prospect_warehous_id AS cust_preferred_warehous_id, "
                "       NULL AS cust_curr_id, COALESCE(crmacct_id,-1) AS crmacct_id, "
                "       false AS iscustomer "
                "FROM prospect "
                "  LEFT OUTER JOIN cntct  ON (prospect_cntct_id=cntct_id) "
                "  LEFT OUTER JOIN addr   ON (cntct_addr_id=addr_id) "
                "  LEFT OUTER JOIN crmacct ON (crmacct_prospect_id = prospect_id) "
                "WHERE (prospect_id=<? value('cust_id') ?>) "
                "<? endif ?>"
                ";" );

    MetaSQLQuery  mql(sql);
    ParameterList params;
    params.append("cust_id", pCustid);
    if (ISQUOTE(_mode))
      params.append("isQuote");
    XSqlQuery cust = mql.toQuery(params);
    if (cust.first())
    {
      if (_mode == cNew)
      {
        if ( (cust.value("cust_creditstatus").toString() == "H") &&
             (!_privileges->check("CreateSOForHoldCustomer")) )
        {
          QMessageBox::warning( this, tr("Selected Customer on Credit Hold"),
                                tr( "<p>The selected Customer has been placed "
                                      "on a Credit Hold and you do not have "
                                      "privilege to create Sales Orders for "
                                      "Customers on Credit Hold.  The selected "
                                      "Customer must be taken off of Credit Hold "
                                      "before you may create a new Sales Order "
                                      "for the Customer." ) );
          _cust->setId(-1);
          _billToAddr->setId(-1);
          _billToName->clear();
          _shipTo->setCustid(-1);
          _cust->setFocus();
          return;
        }

        if ( (cust.value("cust_creditstatus").toString() == "W") &&
             (!_privileges->check("CreateSOForWarnCustomer")) )
        {
          QMessageBox::warning( this, tr("Selected Customer on Credit Warning"),
                                tr( "<p>The selected Customer has been placed on "
                                      "a Credit Warning and you do not have "
                                      "privilege to create Sales Orders for "
                                      "Customers on Credit Warning.  The "
                                      "selected Customer must be taken off of "
                                      "Credit Warning before you may create a "
                                      "new Sales Order for the Customer." ) );
          _cust->setId(-1);
          _billToAddr->setId(-1);
          _billToName->clear();
          _shipTo->setCustid(-1);
          _cust->setFocus();
          return;
        }

        if ( (cust.value("cust_creditstatus").toString() == "H") || (cust.value("cust_creditstatus").toString() == "W") )
          _holdType->setCode("C");
      }

      if (_holdType->code() != "N" && !_privileges->check("OverrideSOHoldType"))
        _holdType->setEnabled(false);
      else
        _holdType->setEnabled(true);

      _billToName->setText(cust.value("cust_name").toString());
      _billToAddr->setId(cust.value("addr_id").toInt());
      sFillCcardList();
      _usesPos     = cust.value("cust_usespos").toBool();
      _blanketPos  = cust.value("cust_blanketpos").toBool();
      _salesRep->setId(cust.value("cust_salesrep_id").toInt());
      _shippingCharges->setId(cust.value("cust_shipchrg_id").toInt());
      _shippingForm->setId(cust.value("cust_shipform_id").toInt());
      _commission->setDouble(cust.value("commission").toDouble() * 100);
      _terms->setId(cust.value("cust_terms_id").toInt());
      _custtaxzoneid = cust.value("cust_taxzone_id").toInt();

      _billToCntct->setId(cust.value("cust_cntct_id").toInt());
      _billToCntct->setSearchAcct(cust.value("crmacct_id").toInt());
      _shipToCntct->setSearchAcct(cust.value("crmacct_id").toInt());
      _crmacctid=cust.value("crmacct_id").toInt();

      if (ISNEW(_mode))
        _taxZone->setId(cust.value("cust_taxzone_id").toInt());
      _shipVia->setText(cust.value("cust_shipvia"));

      _orderCurrency->setId(cust.value("cust_curr_id").toInt());

      if (cust.value("cust_preferred_warehous_id").toInt() > 0)
      {
        _warehouse->setId(cust.value("cust_preferred_warehous_id").toInt());
        _custWhs = cust.value("cust_preferred_warehous_id").toInt();
      }
      else
        _custWhs = -1;

      setFreeFormShipto(cust.value("cust_ffshipto").toBool());
      _shipTo->setCustid(pCustid);

      if (ISNEW(_mode) && cust.value("shiptoid").toInt() != -1)
        populateShipto(cust.value("shiptoid").toInt());
      else
      {
        _ignoreSignals = true;
        _shipTo->setId(cust.value("shiptoid").toInt());
        _shipToName->clear();
        _shipToAddr->clear();
        _shipToCntct->clear();
        _ignoreSignals = false;
      }

      if ((_mode == cNew) || (_mode == cNewQuote ) || (_mode == cEdit) || (_mode == cEditQuote ))
      {
        bool ffBillTo = cust.value("cust_ffbillto").toBool();
        _billToName->setEnabled(ffBillTo);
        _billToAddr->setEnabled(ffBillTo);
        _billToCntct->setEnabled(ffBillTo);

        if (!cust.value("iscustomer").toBool())
          _shipTo->setEnabled(false);
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Customer Information"),
                                  cust, __FILE__, __LINE__))
    {
      return;
    }
  }
  else
  {
    _salesRep->setCurrentIndex(-1);
    _commission->clear();
    _terms->setCurrentIndex(-1);
    _taxZone->setCurrentIndex(-1);
    _taxzoneidCache  = -1;
    _custtaxzoneid   = -1;
    _shipTo->setCustid(-1);
    _shipToName->clear();
    _shipToAddr->clear();
    _shipToCntct->clear();
    _billToCntct->clear();
    _billToName->clear();
  }
}

void salesOrder::sParseShipToNumber()
{
  if (_ignoreSignals)
    return;

  populateShipto(_shipTo->id());
  if (_soitem->topLevelItemCount())
    sRecalculatePrice();
}

void salesOrder::populateShipto(int pShiptoid)
{
  if (pShiptoid != -1)
  {
    XSqlQuery shipto;
    shipto.prepare( "SELECT shipto_num, shipto_name, shipto_addr_id, "
                    "       cntct_phone, shipto_cntct_id, shipto_shipzone_id,"
                    "       shipto_shipvia, shipto_shipcomments, shipto_comments,"
                    "       shipto_shipchrg_id, shipto_shipform_id,"
                    "       COALESCE(shipto_taxzone_id, -1) AS shipto_taxzone_id,"
                    "       shipto_preferred_warehous_id, "
                    "       shipto_salesrep_id, shipto_commission AS commission "
                    "FROM shiptoinfo LEFT OUTER JOIN "
                    "     cntct ON (shipto_cntct_id = cntct_id) "
                    "WHERE (shipto_id=:shipto_id);" );
    shipto.bindValue(":shipto_id", pShiptoid);
    shipto.exec();
    if (shipto.first())
    {
      //  Populate the dlg with the shipto information
      _ignoreSignals=true;
      if (_shipTo->id() != pShiptoid)
        _shipTo->setId(pShiptoid);
      _shipToName->setText(shipto.value("shipto_name").toString());
      _shipToAddr->setId(shipto.value("shipto_addr_id").toInt());
      _shipToCntct->setId(shipto.value("shipto_cntct_id").toInt());
      _shippingCharges->setId(shipto.value("shipto_shipchrg_id").toInt());
      _shippingForm->setId(shipto.value("shipto_shipform_id").toInt());
      _salesRep->setId(shipto.value("shipto_salesrep_id").toInt());
      _commission->setDouble(shipto.value("commission").toDouble() * 100);
      _shipVia->setText(shipto.value("shipto_shipvia"));
      _shippingZone->setId(shipto.value("shipto_shipzone_id").toInt());
      _shippingComments->setText(shipto.value("shipto_shipcomments").toString());
      _orderComments->setText(shipto.value("shipto_comments").toString());
      if (shipto.value("shipto_taxzone_id").toInt() > 0)
        _taxZone->setId(shipto.value("shipto_taxzone_id").toInt());
      if (shipto.value("shipto_preferred_warehous_id").toInt() > 0 )
        _warehouse->setId(shipto.value("shipto_preferred_warehous_id").toInt());
      else if (_custWhs > 0)
        _warehouse->setId(_custWhs);

      _ignoreSignals=false;
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Ship To Information"),
                                  shipto, __FILE__, __LINE__))
    {
      return;
    }
  }
  else
  {
    _shipTo->setId(-1);
    _shipTo->setCustid(_cust->id());
    _shipToName->clear();
    _shipToAddr->clear();
    _shipToCntct->clear();
    _shippingComments->clear();

    // Reset Sales Order fields back to Customer defaults
    QString custSql("SELECT cust_salesrep_id, cust_shipchrg_id, cust_shipform_id,"
                "       cust_commprcnt AS commission,"
                "       cust_taxzone_id, cust_shipvia "
                "FROM custinfo "
                "WHERE (cust_id=<? value('cust_id') ?>); ");
    MetaSQLQuery  mql(custSql);
    ParameterList params;
    params.append("cust_id", _cust->id());
    XSqlQuery custDefaults = mql.toQuery(params);
    if (custDefaults.first())
    {
      _salesRep->setId(custDefaults.value("cust_salesrep_id").toInt());
      _shippingCharges->setId(custDefaults.value("cust_shipchrg_id").toInt());
      _shippingForm->setId(custDefaults.value("cust_shipform_id").toInt());
      _commission->setDouble(custDefaults.value("commission").toDouble() * 100);
      _custtaxzoneid = custDefaults.value("cust_taxzone_id").toInt();
      _taxZone->setId(custDefaults.value("cust_taxzone_id").toInt());
      _shipVia->setText(custDefaults.value("cust_shipvia"));
    }
    else if (custDefaults.lastError().type() != QSqlError::NoError)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Customer Defaults Lookup"),
                         custDefaults.lastError(), __FILE__, __LINE__);
      return;
    }
  }

  if (_saved)
    save(true);
}

void salesOrder::sConvertShipTo()
{
  if (!_ignoreSignals)
  {
    //  Convert the captive shipto to a free-form shipto
    _shipTo->blockSignals(true);
    _shipTo->setId(-1);
    _shipTo->setCustid(_cust->id());
    _shipTo->blockSignals(false);
  }
}

void salesOrder::sNew()
{
  if ( !_saved && ((_mode == cNew) || (_mode == cNewQuote)) )
  {
    if (!save(true))
      return;
  }

  // Double check some values
  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck((!_shipDate->isValid()) && (_metrics->value("soPriceEffective") == "ScheduleDate"), _shipDate,
         tr("You must enter an Scheduled Date for this order before you may save it.") )
  ;

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Sales Order"), errors))
    return;

  ParameterList params;
  params.append("sohead_id", _soheadid);
  params.append("cust_id", _cust->id());
  params.append("shipto_id", _shipTo->id());
  params.append("shipto_name", _shipToName->text());
  params.append("orderNumber", _orderNumber->text());
  params.append("curr_id", _orderCurrency->id());
  params.append("orderDate", _orderDate->date());
  params.append("taxzone_id", _taxZone->id());
  params.append("shipzone_id", _shippingZone->id());
  params.append("saletype_id", _saleType->id());
  if (_warehouse->id() != -1)
    params.append("warehous_id", _warehouse->id());
  if (_shipDate->isValid())
    params.append("shipDate", _shipDate->date());

  if ((_mode == cNew) || (_mode == cEdit))
    params.append("mode", "new");
  else if ((_mode == cNewQuote) || (_mode == cEditQuote))
    params.append("mode", "newQuote");

  salesOrderItem newdlg(this);
  newdlg.set(params);

  newdlg.exec();
}

void salesOrder::sCopyToShipto()
{
  _shipTo->setId(-1);
  _shipTo->setCustid(_cust->id());
  _shipToName->setText(_billToName->text());
  _shipToAddr->setId(_billToAddr->id());
  if (_billToAddr->id() <= 0)
  {
    _shipToAddr->setLine1(_billToAddr->line1());
    _shipToAddr->setLine2(_billToAddr->line2());
    _shipToAddr->setLine3(_billToAddr->line3());
    _shipToAddr->setCity(_billToAddr->city());
    _shipToAddr->setState(_billToAddr->state());
    _shipToAddr->setPostalCode(_billToAddr->postalCode());
    _shipToAddr->setCountry(_billToAddr->country());
  }

  _shipToCntct->setId(_billToCntct->id());
  _taxZone->setId(_custtaxzoneid);
}

void salesOrder::sEdit()
{
  ParameterList params;
  params.append("soitem_id", _soitem->id());
  params.append("cust_id", _cust->id());
  params.append("shipto_id", _shipTo->id());
  params.append("shipto_name", _shipToName->text());
  params.append("orderNumber", _orderNumber->text());
  params.append("curr_id", _orderCurrency->id());
  params.append("orderDate", _orderDate->date());
  params.append("taxzone_id", _taxZone->id());
  params.append("shipzone_id", _shippingZone->id());
  params.append("saletype_id", _saleType->id());

  if (_mode == cView)
    params.append("mode", "view");
  else if (_mode == cViewQuote)
    params.append("mode", "viewQuote");
  else if (((_mode == cNew) || (_mode == cEdit)) &&
           _soitem->currentItem()->rawValue("coitem_subnumber").toInt() != 0)
    params.append("mode", "view");
  else if ((_mode == cNew) || (_mode == cEdit))
    params.append("mode", "edit");
  else if ((_mode == cNewQuote) || (_mode == cEditQuote))
    params.append("mode", "editQuote");

  salesOrderItem newdlg(this);
  newdlg.set(params);
  newdlg.exec();

  if ( ( (_mode == cNew) || (_mode == cNewQuote) || (_mode == cEdit) || (_mode == cEditQuote) ) )
    sFillItemList();
}

void salesOrder::sHandleButtons()
{
  XTreeWidgetItem *selected = 0;

  QList<XTreeWidgetItem *> selectedlist = _soitem->selectedItems();
  _numSelected = selectedlist.size();
  if (_numSelected > 0)
    selected = (XTreeWidgetItem *)(selectedlist[0]);

  if (selected)
  {
    _issueStock->setEnabled(_privileges->check("IssueStockToShipping"));
    _issueLineBalance->setEnabled(_privileges->check("IssueStockToShipping"));
    _reserveStock->setEnabled(_privileges->check("MaintainReservations"));
    _reserveLineBalance->setEnabled(_privileges->check("MaintainReservations"));

    if (_numSelected == 1)
    {
      if (!selected->rawValue("coitem_firm").toBool())
      {
        _lineFirm = 1;
        _edit->setEnabled(true);
        _delete->setEnabled(true);
      }
      else
        _lineFirm = 0;

      int lineMode = selected->altId();

      if (ISQUOTE(_mode))
      {
        _action->setText(tr("Close"));
        _action->setEnabled(false);
        _delete->setEnabled(true);
      }
      else
      {
        if (lineMode == 1)
        {
          _lineMode = cClosed;

          _action->setText(tr("Open"));
          _action->setEnabled(true);
          _cancel->setEnabled(false);
          _delete->setEnabled(false);
        }
        else if (lineMode == 2)
        {
          _lineMode = cActiveOpen;

          _action->setText(tr("Close"));
          _action->setEnabled(true);
          _cancel->setEnabled(false);
          _delete->setEnabled(false);
        }
        else if (lineMode == 3)
        {
          _lineMode = cInactiveOpen;

          _action->setText(tr("Close"));
          _action->setEnabled(true);
          _cancel->setEnabled(true);
          _delete->setEnabled(true);
        }
        else if (lineMode == 4)
        {
          _lineMode = cCanceled;

          _action->setEnabled(false);
          _cancel->setEnabled(false);
          _delete->setEnabled(false);
        }
        else
        {
          _action->setEnabled(false);
          _cancel->setEnabled(false);
          _delete->setEnabled(false);
        }

        if (1 == lineMode ||                                  // closed
            4 == lineMode ||                                  // cancelled
            selected->rawValue("item_type").toString() == "K" // kit item
            )
        {
          _issueStock->setEnabled(false);
          _issueLineBalance->setEnabled(false);
          _reserveStock->setEnabled(false);
          _reserveLineBalance->setEnabled(false);
          for (int i = 0; i < selected->childCount(); i++)
          {
            if (selected->child(i)->altId() == 1 ||
                selected->child(i)->altId() == 2 ||
                selected->child(i)->altId() == 4)
            {
              _delete->setEnabled(false);
              break;
            }
          }
        }

        if (selected->rawValue("coitem_subnumber").toInt() != 0)
        {
          _edit->setText(tr("View"));
          _cancel->setEnabled(false);
          _delete->setEnabled(false);
        }
        else if (cNew == _mode || cEdit == _mode || cNewQuote == _mode || cEditQuote == _mode)
        {
          _edit->setText(tr("&Edit"));
        }

        XSqlQuery invhist;
        invhist.prepare("SELECT 1 "
                        "  FROM invhist "
                        " WHERE invhist_ordnumber=formatSoNumber(:soitemid) "
                        "   AND invhist_ordtype='SO';");
        invhist.bindValue(":soitemid", selected->id());
        invhist.exec();
        if (invhist.first())
          _delete->setEnabled(false);
        else if(ErrorReporter::error(QtCriticalMsg, this, "Error checking invhist",
                                     invhist, __FILE__, __LINE__))
          return;
      }
    }
    else
    {
      _lineMode = 0;
      if (_mode == cView)
        _edit->setEnabled(true);
      else
        _edit->setEnabled(false);
      _action->setEnabled(false);
      _cancel->setEnabled(false);
      _delete->setEnabled(false);
    }
  }
  else
  {
    _edit->setEnabled(false);
    _action->setEnabled(false);
    _cancel->setEnabled(false);
    _delete->setEnabled(false);
    _issueStock->setEnabled(false);
    _issueLineBalance->setEnabled(false);
    _reserveStock->setEnabled(false);
    _reserveLineBalance->setEnabled(false);
  }
}

void salesOrder::sFirm()
{
  XSqlQuery firmSales;
  if (_lineMode == cCanceled)
    return;

  if ( (_mode == cNew) || (_mode == cEdit) )
  {
    firmSales.prepare( "UPDATE coitem "
               "SET coitem_firm=true "
               "WHERE (coitem_id=:coitem_id);" );
    firmSales.bindValue(":coitem_id", _soitem->id());
    firmSales.exec();
    sFillItemList();
  }
}

void salesOrder::sSoften()
{
  XSqlQuery softenSales;
  if (_lineMode == cCanceled)
    return;

  if ( (_mode == cNew) || (_mode == cEdit) )
  {
    softenSales.prepare( "UPDATE coitem "
               "SET coitem_firm=false "
               "WHERE (coitem_id=:coitem_id);" );
    softenSales.bindValue(":coitem_id", _soitem->id());
    softenSales.exec();
    sFillItemList();
  }
}

void salesOrder::sAction()
{
  XSqlQuery actionSales;
  if (_lineMode == cCanceled)
    return;

  int _soitemid = _soitem->id();

  if (_lineMode == cClosed)
  {
    actionSales.prepare( "UPDATE coitem "
                        "SET coitem_status='O' "
                        "WHERE (coitem_id=:coitem_id);" );
    actionSales.bindValue(":coitem_id", _soitemid);
    actionSales.exec();
  }
  else if ( (_mode == cNew) || (_mode == cEdit) )
  {
    actionSales.prepare( "SELECT qtyAtShipping(:coitem_id) AS atshipping;");
    actionSales.bindValue(":coitem_id", _soitemid);
    actionSales.exec();
    if (actionSales.first() && actionSales.value("atshipping").toDouble() > 0)
    {
      QMessageBox::information(this, tr("Cannot Close Item"),
                               tr("The item cannot be Closed at this time as there is inventory at shipping.") );
      return;
    }
    if (_metrics->boolean("EnableSOReservations"))
      sUnreserveStock();
    actionSales.prepare( "UPDATE coitem "
                        "SET coitem_status='C' "
                        "WHERE (coitem_id=:coitem_id);" );
    actionSales.bindValue(":coitem_id", _soitemid);
    actionSales.exec();
  }

  sFillItemList();
}

void salesOrder::sCancel()
{
  XSqlQuery existpo;
  existpo.prepare("SELECT poitem_id "
                  "  FROM coitem JOIN poitem ON coitem_order_type='P' "
                  "                         AND coitem_order_id=poitem_id "
                  " WHERE coitem_id=:soitem_id;");
  existpo.bindValue(":soitem_id", _soitem->id());
  existpo.exec();
  if (existpo.first() && QMessageBox::question(this, tr("Delete Purchase Order Item?"),
                                               tr("Do you wish to delete the Purchase Order Item "
                                                  "linked to this Sales Order Item? The associated "
                                                  "Purchase Order will also be deleted if no other "
                                                  "Purchase Order Item exists for that Purchase "
                                                  "Order."),
                                               QMessageBox::Yes,
                                               QMessageBox::No | QMessageBox::Default)
                         == QMessageBox::Yes)
  {
    XSqlQuery deletepo;
    deletepo.prepare("SELECT deletepoitem(:poitemid) AS result;");
    deletepo.bindValue(":poitemid", existpo.value("poitem_id").toInt());
    deletepo.exec();
    if (deletepo.first() && deletepo.value("result").toInt() < 0)
    {
      XSqlQuery closepo;
      closepo.prepare("UPDATE poitem "
                      "   SET poitem_status='C' "
                      " WHERE poitem_id=:poitemid");
      closepo.bindValue(":poitem", existpo.value("poitem_id").toInt());
      closepo.exec();
      if (ErrorReporter::error(QtCriticalMsg, this, tr("Error closing P/O Line"),
                               closepo, __FILE__, __LINE__))
        return;
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error deleting P/O Item"),
                                  deletepo, __FILE__, __LINE__))
      return;
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Linked P/O"),
                                existpo, __FILE__, __LINE__))
    return;

  XSqlQuery salesCancel;
  salesCancel.prepare("UPDATE coitem "
                      "   SET coitem_status='X' "
                      " WHERE coitem_id=:soitem_id;");
  salesCancel.bindValue(":soitem_id", _soitem->id());
  salesCancel.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Cancelling Item"),
                                salesCancel, __FILE__, __LINE__))
    return;

  sFillItemList();
}

void salesOrder::sDelete()
{
  if (QMessageBox::question(this, tr("Delete Selected Line Item?"),
                            tr("<p>Are you sure that you want to delete the "
                               "selected Line Item?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    int _soitemid = _soitem->id();

    XSqlQuery deleteSales;
    if ( (_mode == cEdit) || (_mode == cNew) )
    {
      if (_metrics->boolean("EnableSOReservations"))
        sUnreserveStock();

      deleteSales.prepare( "SELECT deleteSOItem(:soitem_id) AS result;");
      deleteSales.bindValue(":soitem_id", _soitemid);
      deleteSales.exec();
      if (deleteSales.first())
      {
        int result = deleteSales.value("result").toInt();
        if (result == -20)
          QMessageBox::information(this, tr("Cannot Delete Related Purchase Order"),
                                   storedProcErrorLookup("deleteSOItem", result));
        else if (result < 0)
          ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Sales Order Information"),
                                 storedProcErrorLookup("deleteSOItem", result),
                                 __FILE__, __LINE__);
      }
      else if (deleteSales.lastError().type() != QSqlError::NoError)
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Sales Order Information"),
                           deleteSales, __FILE__, __LINE__);

      sFillItemList();

      if (_soitem->topLevelItemCount() == 0)
      {
        if (QMessageBox::question(this, tr("Cancel Sales Order?"),
                                  tr("<p>You have deleted all of the Line "
                                     "Items for this Sales Order. Would you "
                                     "like to cancel this Sales Order?"),
                                  QMessageBox::Yes,
                                  QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
        {
          deleteSales.prepare( "SELECT deleteSO(:sohead_id, :sohead_number) AS result;");
          deleteSales.bindValue(":sohead_id", _soheadid);
          deleteSales.bindValue(":sohead_number", _orderNumber->text());
          deleteSales.exec();
          if (deleteSales.first())
          {
            int result = deleteSales.value("result").toInt();
            if (result < 0)
              ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Sales Order Information"),
                                   storedProcErrorLookup("deleteSO", result),
                                   __FILE__, __LINE__);
          }
          else if (deleteSales.lastError().type() != QSqlError::NoError)
            ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Sales Order Information"),
                             deleteSales, __FILE__, __LINE__);

          omfgThis->sSalesOrdersUpdated(_soheadid);
          _captive = false;
          clear();
        }
      }
    }
    else if ( (_mode == cNewQuote) || (_mode == cEditQuote) )
    {
      deleteSales.prepare( "SELECT deleteQuoteItem(:quitem_id) AS result;");
      deleteSales.bindValue(":quitem_id", _soitem->id());
      deleteSales.exec();
      sFillItemList();

      if (_soitem->topLevelItemCount() == 0)
      {
        if ( QMessageBox::question(this, tr("Cancel Quote?"),
                                   tr("<p>You have deleted all of the order "
                                      "lines for this Quote. Would you like to "
                                      "cancel this Quote?."),
                                   QMessageBox::Yes,
                                   QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
        {
          deleteSales.prepare("SELECT deleteQuote(:quhead_id, :quhead_number) AS result;");
          deleteSales.bindValue(":quhead_id", _soheadid);
          deleteSales.bindValue(":quhead_number", _orderNumber->text());
          deleteSales.exec();
          if (deleteSales.first() && (deleteSales.value("result").toInt() < 0))
            ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Quote"),
                               deleteSales, __FILE__, __LINE__);
          else if (deleteSales.lastError().type() != QSqlError::NoError)
            ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Quote"),
                               deleteSales, __FILE__, __LINE__);

          omfgThis->sQuotesUpdated(_soheadid);
          _captive = false;
          clear();
        }
      }
    }
  }
}

void salesOrder::populate()
{
  if (ISORDER(_mode))
  {
    XSqlQuery so;
    if (ISEDIT(_mode) && ! _lock.acquire("cohead", _soheadid, AppLock::Interactive))
    {
      setViewMode();
    }
    so.prepare( "SELECT cohead.*,"
                "       COALESCE(cohead_shipto_id,-1) AS cohead_shipto_id,"
                "       cohead_commission AS commission,"
                "       COALESCE(cohead_taxzone_id,-1) AS taxzone_id,"
                "       COALESCE(cohead_warehous_id,-1) as cohead_warehous_id,"
                "       COALESCE(cohead_shipzone_id,-1) as cohead_shipzone_id,"
                "       COALESCE(cohead_saletype_id,-1) as cohead_saletype_id,"
                "       cust_name, cust_ffshipto, cust_blanketpos,"
                "       COALESCE(cohead_misc_accnt_id,-1) AS cohead_misc_accnt_id,"
                "       CASE WHEN(cohead_wasquote) THEN COALESCE(cohead_quote_number, cohead_number)"
                "            ELSE formatBoolYN(cohead_wasquote)"
                "       END AS fromQuote,"
                "       COALESCE(cohead_prj_id,-1) AS cohead_prj_id, "
                "       COALESCE(cohead_ophead_id,-1) AS cohead_ophead_id "
                "FROM custinfo, cohead "
                "WHERE ( (cohead_cust_id=cust_id)"
                " AND (cohead_id=:cohead_id) );" );
    so.bindValue(":cohead_id", _soheadid);
    so.exec();
    if (so.first())
    {
      if (so.value("cohead_status").toString() == "C")
        setViewMode();

      _orderNumber->setText(so.value("cohead_number").toString());
      _orderNumber->setEnabled(false);

      _orderDateCache = so.value("cohead_orderdate").toDate();
      _orderDate->setDate(_orderDateCache, true);
      _packDate->setDate(so.value("cohead_packdate").toDate());

      _fromQuote->setText(so.value("fromQuote").toString());

      _cust->setId(so.value("cohead_cust_id").toInt());

      setFreeFormShipto(so.value("cust_ffshipto").toBool());
      _blanketPos = so.value("cust_blanketpos").toBool();

      _warehouse->setId(so.value("cohead_warehous_id").toInt());
      _salesRep->setId(so.value("cohead_salesrep_id").toInt());
      _commission->setDouble(so.value("commission").toDouble() * 100);
      _taxzoneidCache = so.value("taxzone_id").toInt();
      _taxZone->setId(so.value("taxzone_id").toInt());
      _terms->setId(so.value("cohead_terms_id").toInt());
      _orderCurrency->setId(so.value("cohead_curr_id").toInt());
      _project->setId(so.value("cohead_prj_id").toInt());
      _opportunity->setId(so.value("cohead_ophead_id").toInt());

      _shipToCntct->setId(so.value("cohead_shipto_cntct_id").toInt());
      _shipToCntct->setHonorific(so.value("cohead_shipto_cntct_honorific").toString());
      _shipToCntct->setFirst(so.value("cohead_shipto_cntct_first_name").toString());
      _shipToCntct->setMiddle(so.value("cohead_shipto_cntct_middle").toString());
      _shipToCntct->setLast(so.value("cohead_shipto_cntct_last_name").toString());
      _shipToCntct->setSuffix(so.value("cohead_shipto_cntct_suffix").toString());
      _shipToCntct->setPhone(so.value("cohead_shipto_cntct_phone").toString());
      _shipToCntct->setTitle(so.value("cohead_shipto_cntct_title").toString());
      _shipToCntct->setFax(so.value("cohead_shipto_cntct_fax").toString());
      _shipToCntct->setEmailAddress(so.value("cohead_shipto_cntct_email").toString());

      _billToCntct->setId(so.value("cohead_billto_cntct_id").toInt());
      _billToCntct->setHonorific(so.value("cohead_billto_cntct_honorific").toString());
      _billToCntct->setFirst(so.value("cohead_billto_cntct_first_name").toString());
      _billToCntct->setMiddle(so.value("cohead_billto_cntct_middle").toString());
      _billToCntct->setLast(so.value("cohead_billto_cntct_last_name").toString());
      _billToCntct->setSuffix(so.value("cohead_billto_cntct_suffix").toString());
      _billToCntct->setPhone(so.value("cohead_billto_cntct_phone").toString());
      _billToCntct->setTitle(so.value("cohead_billto_cntct_title").toString());
      _billToCntct->setFax(so.value("cohead_billto_cntct_fax").toString());
      _billToCntct->setEmailAddress(so.value("cohead_billto_cntct_email").toString());

      _billToName->setText(so.value("cohead_billtoname").toString());
      if (_billToAddr->line1() !=so.value("cohead_billtoaddress1").toString() ||
          _billToAddr->line2() !=so.value("cohead_billtoaddress2").toString() ||
          _billToAddr->line3() !=so.value("cohead_billtoaddress3").toString() ||
          _billToAddr->city()  !=so.value("cohead_billtocity").toString() ||
          _billToAddr->state() !=so.value("cohead_billtostate").toString() ||
          _billToAddr->postalCode()!=so.value("cohead_billtozipcode").toString() ||
          _billToAddr->country()!=so.value("cohead_billtocountry").toString() )
      {
        _billToAddr->setId(-1);

        _billToAddr->setLine1(so.value("cohead_billtoaddress1").toString());
        _billToAddr->setLine2(so.value("cohead_billtoaddress2").toString());
        _billToAddr->setLine3(so.value("cohead_billtoaddress3").toString());
        _billToAddr->setCity(so.value("cohead_billtocity").toString());
        _billToAddr->setState(so.value("cohead_billtostate").toString());
        _billToAddr->setPostalCode(so.value("cohead_billtozipcode").toString());
        _billToAddr->setCountry(so.value("cohead_billtocountry").toString());
      }

      _ignoreSignals=true;
      _shipToName->setText(so.value("cohead_shiptoname").toString());
      if (_shipToAddr->line1() !=so.value("cohead_shiptoaddress1").toString() ||
          _shipToAddr->line2() !=so.value("cohead_shiptoaddress2").toString() ||
          _shipToAddr->line3() !=so.value("cohead_shiptoaddress3").toString() ||
          _shipToAddr->city()  !=so.value("cohead_shiptocity").toString() ||
          _shipToAddr->state() !=so.value("cohead_shiptostate").toString() ||
          _shipToAddr->postalCode()!=so.value("cohead_shiptozipcode").toString() ||
          _shipToAddr->country()!=so.value("cohead_shiptocountry").toString() )
      {
        _shipToAddr->setId(-1);

        _shipToAddr->setLine1(so.value("cohead_shiptoaddress1").toString());
        _shipToAddr->setLine2(so.value("cohead_shiptoaddress2").toString());
        _shipToAddr->setLine3(so.value("cohead_shiptoaddress3").toString());
        _shipToAddr->setCity(so.value("cohead_shiptocity").toString());
        _shipToAddr->setState(so.value("cohead_shiptostate").toString());
        _shipToAddr->setPostalCode(so.value("cohead_shiptozipcode").toString());
        _shipToAddr->setCountry(so.value("cohead_shiptocountry").toString());
      }

      _shipTo->setId(so.value("cohead_shipto_id").toInt());
      _shipTo->setCustid(_cust->id());
      _ignoreSignals=false;

      if (ISVIEW(_mode))
        _shipTo->setEnabled(false);

      _custPONumber->setText(so.value("cohead_custponumber"));
      _shipVia->setText(so.value("cohead_shipvia"));

      _fob->setText(so.value("cohead_fob"));
      _holdType->setCode(so.value("cohead_holdtype").toString());
      if (ISVIEW(_mode) || (_holdType->code() != "N" && !_privileges->check("OverrideSOHoldType")))
        _holdType->setEnabled(false);
      else
        _holdType->setEnabled(true);
       if (ISEDIT(_mode))
         _holdTypeCache = _holdType->code();

      _miscCharge->setLocalValue(so.value("cohead_misc").toDouble());
      _miscChargeDescription->setText(so.value("cohead_misc_descrip"));
      _miscChargeAccount->setId(so.value("cohead_misc_accnt_id").toInt());

      _orderComments->setText(so.value("cohead_ordercomments").toString());
      _shippingComments->setText(so.value("cohead_shipcomments").toString());
      _shippingCharges->setId(so.value("cohead_shipchrg_id").toInt());
      _shippingForm->setId(so.value("cohead_shipform_id").toInt());
      _shippingZone->setId(so.value("cohead_shipzone_id").toInt());
      _saleType->setId(so.value("cohead_saletype_id").toInt());

      _calcfreight = so.value("cohead_calcfreight").toBool();
      // Auto calculated _freight is populated in sFillItemList
      if (!_calcfreight)
      {
        disconnect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
        _freight->setLocalValue(so.value("cohead_freight").toDouble());
        connect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
      }

      _shipComplete->setChecked(so.value("cohead_shipcomplete").toBool());

      _comments->setId(_soheadid);
      _documents->setId(_soheadid);

      // Check for link to Return Authorization
      if (_metrics->boolean("EnableReturnAuth"))
      {
        so.prepare("SELECT rahead_number "
                  "FROM rahead "
                  "WHERE (rahead_new_cohead_id=:sohead_id);");
        so.bindValue(":sohead_id",_soheadid);
        so.exec();
        if (so.first())
        {
          _fromQuoteLit->setText(tr("From Return Authorization:"));
          _fromQuote->setText(so.value("rahead_number").toString());
        }
      }
      sPopulateShipments();
      sFillCharacteristic();
      emit populated();
      sFillItemList();
      // TODO - a partial save is not saving everything
      if (! ISVIEW(_mode))
        save(false);
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Sales Order Information"),
                                  so, __FILE__, __LINE__))
    {
      return;
    }
  }
  else if (ISQUOTE(_mode))
  {
    XSqlQuery qu;
    if (ISEDIT(_mode) && ! _lock.acquire("quhead", _soheadid, AppLock::Interactive))
    {
      setViewMode();
    }
    qu.prepare( "SELECT quhead.*,"
                "       COALESCE(quhead_shipto_id,-1) AS quhead_shipto_id,"
                "       quhead_commission AS commission,"
                "       COALESCE(quhead_taxzone_id, -1) AS quhead_taxzone_id,"
                "       COALESCE(quhead_shipzone_id,-1) as quhead_shipzone_id,"
                "       COALESCE(quhead_saletype_id,-1) as quhead_saletype_id,"
                "       cust_ffshipto, cust_blanketpos,"
                "       COALESCE(quhead_misc_accnt_id,-1) AS quhead_misc_accnt_id, "
                "       COALESCE(quhead_prj_id,-1) AS quhead_prj_id, "
                "       COALESCE(quhead_ophead_id,-1) AS quhead_ophead_id, "
                "       CASE WHEN quhead_status IN ('O','') THEN 'Open' "
                "         ELSE CASE WHEN quhead_status ='C' THEN 'Converted' "
                "         END "
                "       END AS status "
                "FROM quhead, custinfo "
                "WHERE ( (quhead_cust_id=cust_id)"
                " AND (quhead_id=:quhead_id) )"
                "UNION "
                "SELECT quhead.*,"
                "       COALESCE(quhead_shipto_id,-1) AS quhead_shipto_id,"
                "       quhead_commission AS commission,"
                "       COALESCE(quhead_taxzone_id, -1) AS quhead_taxzone_id,"
                "       COALESCE(quhead_shipzone_id,-1) as quhead_shipzone_id,"
                "       COALESCE(quhead_saletype_id,-1) as quhead_saletype_id,"
                "       true AS cust_ffshipto, NULL AS cust_blanketpos,"
                "       COALESCE(quhead_misc_accnt_id, -1) AS quhead_misc_accnt_id, "
                "       COALESCE(quhead_prj_id,-1) AS quhead_prj_id, "
                "       COALESCE(quhead_ophead_id,-1) AS quhead_ophead_id, "
                "       CASE WHEN quhead_status IN ('O','') THEN 'Open' "
                "         ELSE CASE WHEN quhead_status ='C' THEN 'Converted' "
                "          END "
                "       END AS status "
                "FROM quhead, prospect "
                "WHERE ( (quhead_cust_id=prospect_id)"
                " AND (quhead_id=:quhead_id) )"
                ";" );
    qu.bindValue(":quhead_id", _soheadid);
    qu.exec();
    if (qu.first())
    {
      _orderNumber->setText(qu.value("quhead_number").toString());
      _orderNumber->setEnabled(false);

      _orderDateCache = qu.value("quhead_quotedate").toDate();
      _orderDate->setDate(_orderDateCache, true);
      _packDate->setDate(qu.value("quhead_packdate").toDate());
      if (!qu.value("quhead_expire").isNull())
        _expire->setDate(qu.value("quhead_expire").toDate());

      _cust->setId(qu.value("quhead_cust_id").toInt());

      setFreeFormShipto(qu.value("cust_ffshipto").toBool());
      _blanketPos = qu.value("cust_blanketpos").toBool();

      _warehouse->setId(qu.value("quhead_warehous_id").toInt());
      _salesRep->setId(qu.value("quhead_salesrep_id").toInt());
      _commission->setDouble(qu.value("commission").toDouble() * 100);
      _taxzoneidCache = qu.value("quhead_taxzone_id").toInt();
      _taxZone->setId(qu.value("quhead_taxzone_id").toInt());
      _terms->setId(qu.value("quhead_terms_id").toInt());
      _orderCurrency->setId(qu.value("quhead_curr_id").toInt());
      _project->setId(qu.value("quhead_prj_id").toInt());
      _opportunity->setId(qu.value("quhead_ophead_id").toInt());

      _billToName->setText(qu.value("quhead_billtoname").toString());
      _billToAddr->setLine1(qu.value("quhead_billtoaddress1").toString());
      _billToAddr->setLine2(qu.value("quhead_billtoaddress2").toString());
      _billToAddr->setLine3(qu.value("quhead_billtoaddress3").toString());
      _billToAddr->setCity(qu.value("quhead_billtocity").toString());
      _billToAddr->setState(qu.value("quhead_billtostate").toString());
      _billToAddr->setPostalCode(qu.value("quhead_billtozip").toString());
      _billToAddr->setCountry(qu.value("quhead_billtocountry").toString());

      _shipToCntct->setId(qu.value("quhead_shipto_cntct_id").toInt());
      _shipToCntct->setHonorific(qu.value("quhead_shipto_cntct_honorific").toString());
      _shipToCntct->setFirst(qu.value("quhead_shipto_cntct_first_name").toString());
      _shipToCntct->setMiddle(qu.value("quhead_shipto_cntct_middle").toString());
      _shipToCntct->setLast(qu.value("quhead_shipto_cntct_last_name").toString());
      _shipToCntct->setSuffix(qu.value("quhead_shipto_cntct_suffix").toString());
      _shipToCntct->setPhone(qu.value("quhead_shipto_cntct_phone").toString());
      _shipToCntct->setTitle(qu.value("quhead_shipto_cntct_title").toString());
      _shipToCntct->setFax(qu.value("quhead_shipto_cntct_fax").toString());
      _shipToCntct->setEmailAddress(qu.value("quhead_shipto_cntct_email").toString());

      _billToCntct->setId(qu.value("quhead_billto_cntct_id").toInt());
      _billToCntct->setHonorific(qu.value("quhead_billto_cntct_honorific").toString());
      _billToCntct->setFirst(qu.value("quhead_billto_cntct_first_name").toString());
      _billToCntct->setMiddle(qu.value("quhead_billto_cntct_middle").toString());
      _billToCntct->setLast(qu.value("quhead_billto_cntct_last_name").toString());
      _billToCntct->setSuffix(qu.value("quhead_billto_cntct_suffix").toString());
      _billToCntct->setPhone(qu.value("quhead_billto_cntct_phone").toString());
      _billToCntct->setTitle(qu.value("quhead_billto_cntct_title").toString());
      _billToCntct->setFax(qu.value("quhead_billto_cntct_fax").toString());
      _billToCntct->setEmailAddress(qu.value("quhead_billto_cntct_email").toString());

      _ignoreSignals=true;
      _shipToName->setText(qu.value("quhead_shiptoname").toString());
      if (_shipToAddr->line1() !=qu.value("quhead_shiptoaddress1").toString() ||
          _shipToAddr->line2() !=qu.value("quhead_shiptoaddress2").toString() ||
          _shipToAddr->line3() !=qu.value("quhead_shiptoaddress3").toString() ||
          _shipToAddr->city()  !=qu.value("quhead_shiptocity").toString() ||
          _shipToAddr->state() !=qu.value("quhead_shiptostate").toString() ||
          _shipToAddr->postalCode()!=qu.value("quhead_shiptozipcode").toString() ||
          _shipToAddr->country()!=qu.value("quhead_shiptocountry").toString() )
      {
        _shipToAddr->setId(-1);

        _shipToAddr->setLine1(qu.value("quhead_shiptoaddress1").toString());
        _shipToAddr->setLine2(qu.value("quhead_shiptoaddress2").toString());
        _shipToAddr->setLine3(qu.value("quhead_shiptoaddress3").toString());
        _shipToAddr->setCity(qu.value("quhead_shiptocity").toString());
        _shipToAddr->setState(qu.value("quhead_shiptostate").toString());
        _shipToAddr->setPostalCode(qu.value("quhead_shiptozipcode").toString());
        _shipToAddr->setCountry(qu.value("quhead_shiptocountry").toString());
      }

      _shipTo->setId(qu.value("quhead_shipto_id").toInt());
      _shipTo->setCustid(_cust->id());
      _ignoreSignals=false;

      if (_mode == cViewQuote)
        _shipTo->setEnabled(false);

      _custPONumber->setText(qu.value("quhead_custponumber"));
      _shipVia->setText(qu.value("quhead_shipvia"));
      _shippingZone->setId(qu.value("quhead_shipzone_id").toInt());
      _saleType->setId(qu.value("quhead_saletype_id").toInt());

      _fob->setText(qu.value("quhead_fob"));

      _calcfreight = qu.value("quhead_calcfreight").toBool();
      // Auto calculated _freight is populated in sFillItemList
      if (!_calcfreight)
      {
        disconnect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
        _freight->setLocalValue(qu.value("quhead_freight").toDouble());
        connect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
      }

      _miscCharge->setLocalValue(qu.value("quhead_misc").toDouble());
      _miscChargeDescription->setText(qu.value("quhead_misc_descrip"));
      _miscChargeAccount->setId(qu.value("quhead_misc_accnt_id").toInt());

      _orderComments->setText(qu.value("quhead_ordercomments").toString());
      _shippingComments->setText(qu.value("quhead_shipcomments").toString());

      _quotestaus->setText(qu.value("status"));

      _comments->setId(_soheadid);
      _documents->setId(_soheadid);
      sFillCharacteristic();
      sFillItemList();
      emit populated();
      // TODO - a partial save is not saving everything
      if (! ISVIEW(_mode))
        save(false);
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Quote Information"),
                                  qu, __FILE__, __LINE__))
    {
      return;
    }
  }
}

void salesOrder::sFillItemList()
{
  XSqlQuery fillSales;
  if (ISORDER(_mode))
    fillSales.prepare( "SELECT COALESCE(getSoSchedDate(:head_id),:ship_date) AS shipdate;" );
  else
    fillSales.prepare( "SELECT COALESCE(getQuoteSchedDate(:head_id),:ship_date) AS shipdate;" );

  fillSales.bindValue(":head_id", _soheadid);
  fillSales.bindValue(":ship_date", _shipDate->date());
  fillSales.exec();
  if (fillSales.first())
  {
    _shipDateCache = fillSales.value("shipdate").toDate();
    _shipDate->setDate(_shipDateCache);

    if (ISNEW(_mode) && !_packDate->isValid())
      _packDate->setDate(fillSales.value("shipdate").toDate());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Sales Order Information"),
                                fillSales, __FILE__, __LINE__))
  {
    return;
  }

  _soitem->clear();
  if (ISORDER(_mode))
  {
    MetaSQLQuery mql = mqlLoad("salesOrderItems", "list");

    ParameterList params;
    if (!_showCanceled->isChecked())
      params.append("excludeCancelled", true);

    params.append("sohead_id", _soheadid);
    if (_metrics->boolean("EnableSOReservations"))
      params.append("includeReservations");
    XSqlQuery fl = mql.toQuery(params);
    _soitem->populate(fl, true);
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retreiving Sales Order Information"),
                                  fl, __FILE__, __LINE__))
    {
      return;
    }

    _cust->setReadOnly(fl.size() || !ISNEW(_mode));
    _amountAtShipping->setLocalValue(0.0);
    QString sql = "SELECT ROUND(((COALESCE(SUM(shipitem_qty),0)-coitem_qtyshipped) *"
          "                  coitem_qty_invuomratio) *"
          "           (coitem_price / coitem_price_invuomratio),2) AS shippingAmount "
          "  FROM coitem LEFT OUTER JOIN "
          "       (shipitem JOIN shiphead ON (shipitem_shiphead_id=shiphead_id"
          "                               AND shiphead_order_id=:cohead_id"
          "                               AND shiphead_order_type='SO')) ON (shipitem_orderitem_id=coitem_id)"
          " WHERE ((coitem_cohead_id=:cohead_id)";

    if (!_showCanceled->isChecked())
      sql += " AND (coitem_status != 'X') ";

    sql += ") GROUP BY coitem_id, coitem_qtyshipped, coitem_qty_invuomratio,"
           "coitem_price, coitem_price_invuomratio;";
    fillSales.prepare(sql);
    fillSales.bindValue(":cohead_id", _soheadid);
    fillSales.bindValue(":cust_id", _cust->id());
    fillSales.exec();
    while (fillSales.next())
      _amountAtShipping->setLocalValue(_amountAtShipping->localValue() +
                                       fillSales.value("shippingAmount").toDouble());
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Sales Order Information"),
                                  fillSales, __FILE__, __LINE__))
    {
      return;
    }
  }
  else if (ISQUOTE(_mode))
  {
    MetaSQLQuery mql = mqlLoad("quoteItems", "list");

    ParameterList params;
    params.append("quhead_id", _soheadid);
    XSqlQuery fl = mql.toQuery(params);
    _cust->setReadOnly(fl.size() || !ISNEW(_mode));
    _soitem->populate(fl);
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Sales Order Information"),
                                  fl, __FILE__, __LINE__))
    {
      return;
    }
  }

  //  Determine the subtotal
  if (ISORDER(_mode))
    fillSales.prepare( "SELECT SUM(round((coitem_qtyord * coitem_qty_invuomratio) * (coitem_price / coitem_price_invuomratio),2)) AS subtotal,"
                       "       SUM(round((coitem_qtyord * coitem_qty_invuomratio) * (coitem_unitcost / coitem_price_invuomratio),2)) AS totalcost "
                       "FROM cohead JOIN coitem ON (coitem_cohead_id=cohead_id) "
                       "WHERE ( (cohead_id=:head_id)"
                       " AND (coitem_status <> 'X') );" );
  else
    fillSales.prepare( "SELECT SUM(round((quitem_qtyord * quitem_qty_invuomratio) * (quitem_price / quitem_price_invuomratio),2)) AS subtotal,"
                       "       SUM(round((quitem_qtyord * quitem_qty_invuomratio) * (quitem_unitcost / quitem_price_invuomratio),2)) AS totalcost "
                       "FROM quhead JOIN quitem ON (quitem_quhead_id=quhead_id) "
                       "WHERE (quhead_id=:head_id);" );
  fillSales.bindValue(":head_id", _soheadid);
  fillSales.exec();
  if (fillSales.first())
  {
    _subtotal->setLocalValue(fillSales.value("subtotal").toDouble());
    _margin->setLocalValue(fillSales.value("subtotal").toDouble() - fillSales.value("totalcost").toDouble());
    if (_subtotal->localValue() > 0.0)
      _marginPercent->setDouble(_margin->localValue() / _subtotal->localValue() * 100.0);
    else
      _marginPercent->setDouble(0.0);
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Sales Order Information"),
                                fillSales, __FILE__, __LINE__))
  {
    return;
  }

  if (ISORDER(_mode))
    fillSales.prepare("SELECT SUM(COALESCE(coitem_qtyord * coitem_qty_invuomratio, 0.00) *"
              "           COALESCE(item_prodweight, 0.00)) AS netweight,"
              "       SUM(COALESCE(coitem_qtyord * coitem_qty_invuomratio, 0.00) *"
              "           (COALESCE(item_prodweight, 0.00) +"
              "            COALESCE(item_packweight, 0.00))) AS grossweight "
              "FROM coitem, itemsite, item, cohead "
              "WHERE ((coitem_itemsite_id=itemsite_id)"
              " AND (itemsite_item_id=item_id)"
              " AND (coitem_cohead_id=cohead_id)"
              " AND (coitem_status<>'X')"
              " AND (coitem_cohead_id=:head_id)) "
              "GROUP BY cohead_freight;");
  else if (ISQUOTE(_mode))
    fillSales.prepare("SELECT SUM(COALESCE(quitem_qtyord * quitem_qty_invuomratio, 0.00) *"
              "           COALESCE(item_prodweight, 0.00)) AS netweight,"
              "       SUM(COALESCE(quitem_qtyord * quitem_qty_invuomratio, 0.00) *"
              "           (COALESCE(item_prodweight, 0.00) +"
              "            COALESCE(item_packweight, 0.00))) AS grossweight "
              "  FROM quitem, item, quhead "
              " WHERE ( (quitem_item_id=item_id)"
              "   AND   (quitem_quhead_id=quhead_id)"
              "   AND   (quitem_quhead_id=:head_id)) "
              " GROUP BY quhead_freight;");
  fillSales.bindValue(":head_id", _soheadid);
  fillSales.exec();
  if (fillSales.first())
    _weight->setDouble(fillSales.value("grossweight").toDouble());
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Sales Order Information"),
                                fillSales, __FILE__, __LINE__))
  {
    return;
  }

  if (_calcfreight)
  {
    if (ISORDER(_mode))
      fillSales.prepare("SELECT SUM(freightdata_total) AS freight "
                "FROM freightDetail('SO', :head_id, :cust_id, :shipto_id, :orderdate, :shipvia, :curr_id);");
    else if (ISQUOTE(_mode))
      fillSales.prepare("SELECT SUM(freightdata_total) AS freight "
                "FROM freightDetail('QU', :head_id, :cust_id, :shipto_id, :orderdate, :shipvia, :curr_id);");
    fillSales.bindValue(":head_id", _soheadid);
    fillSales.bindValue(":cust_id", _cust->id());
    fillSales.bindValue(":shipto_id", _shipTo->id());
    fillSales.bindValue(":orderdate", _orderDate->date());
    fillSales.bindValue(":shipvia", _shipVia->currentText());
    fillSales.bindValue(":curr_id", _orderCurrency->id());
    fillSales.exec();
    if (fillSales.first())
    {
      disconnect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
      _freight->setLocalValue(fillSales.value("freight").toDouble());
      connect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
      _freightCache = _freight->localValue();
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Sales Order Information"),
                                  fillSales, __FILE__, __LINE__))
    {
      return;
    }
  }

  sCalculateTax();  // triggers sCalculateTotal();

  _orderCurrency->setEnabled(_soitem->topLevelItemCount() == 0);
}

void salesOrder::sCalculateTotal()
{
  double total = _subtotal->localValue() + _tax->localValue() + _miscCharge->localValue() + _freight->localValue();
  _total->setLocalValue(total);
  _cashTotal->setLocalValue(total);

  double balance = total - _allocatedCM->localValue() - _authCC->localValue();
  if (balance < 0)
    balance = 0;
  _balance->setLocalValue(balance);
  _cashBalance->setLocalValue(balance);
  _CCAmount->setLocalValue(balance);
  if (ISVIEW(_mode) || balance==0)
  {
    _authorize->hide();
    _charge->hide();
  }
  else
  {
    _authorize->setVisible(_metrics->boolean("CCEnablePreauth"));
    _charge->setVisible(_metrics->boolean("CCEnableCharge"));
  }
}

bool salesOrder::deleteForCancel()
{
  XSqlQuery query;

  if (ISNEW(_mode) &&
      _soitem->topLevelItemCount() > 0 &&
      !_captive)
  {
    int answer;
    if (_mode == cNew)
      answer = QMessageBox::question(this, tr("Delete Sales Order?"),
                                      tr("<p>Are you sure you want to delete this "
                                          "Sales Order and its associated Line Items?"),
                                     QMessageBox::Yes,
                                     QMessageBox::No | QMessageBox::Default);
    else
      answer = QMessageBox::question(this, tr("Delete Quote?"),
                                      tr("<p>Are you sure you want to delete this "
                                          "Quote and its associated Line Items?"),
                                     QMessageBox::Yes,
                                     QMessageBox::No | QMessageBox::Default);
    if (answer == QMessageBox::No)
      return false;
  }

  if (_mode == cNew &&
      !_captive)
  {
    query.prepare("SELECT deleteSO(:sohead_id, :sohead_number) AS result;");
    query.bindValue(":sohead_id", _soheadid);
    query.bindValue(":sohead_number", _orderNumber->text());
    query.exec();
    if (query.first())
    {
      int result = query.value("result").toInt();
      if (result < 0)
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Sales Order"),
                               storedProcErrorLookup("deleteSO", result),
                               __FILE__, __LINE__);
    }
    else if (query.lastError().type() != QSqlError::NoError)
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Sales Order"),
                         query, __FILE__, __LINE__);
  }
  else if (_mode == cNewQuote &&
           !_captive)
  {
    query.prepare("SELECT deleteQuote(:head_id, :quhead_number) AS result;");
    query.bindValue(":head_id", _soheadid);
    query.bindValue(":quhead_number", _orderNumberGen);
    query.exec();
    if (query.first())
    {
      int result = query.value("result").toInt();
      if (result < 0)
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Quote"),
                             storedProcErrorLookup("deleteQuote", result),
                             __FILE__, __LINE__);
    }
    else if (query.lastError().type() != QSqlError::NoError)
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Quote"),
                         query, __FILE__, __LINE__);

    if ((_metrics->value("QUNumberGeneration") == "S") ||
        (_metrics->value("QUNumberGeneration") == "A") ||
        (_metrics->value("QUNumberGeneration") == "O"))
    {
      if (_metrics->value("QUNumberGeneration") == "S")
        query.prepare( "SELECT releaseSoNumber(:orderNumber);" );
      else
        query.prepare( "SELECT releaseQUNumber(:orderNumber);" );
      query.bindValue(":orderNumber", _orderNumberGen);
      query.exec();
      if (query.lastError().type() != QSqlError::NoError)
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Quote"),
                       query, __FILE__, __LINE__);
    }
  }

  if (! _lock.release())
    ErrorReporter::error(QtCriticalMsg, this, tr("Locking Error"),
                         _lock.lastError(), __FILE__, __LINE__);

  return true;
}

void salesOrder::sClear()
{
  if (!deleteForCancel())
    return;

  _captive = false;
  clear();
}

void salesOrder::clear()
{
  XSqlQuery clearSales;
  if (! _lock.release())
    ErrorReporter::error(QtCriticalMsg, this, tr("Locking Error"),
                         _lock.lastError(), __FILE__, __LINE__);

  if(_metrics->boolean("DefaultSOLineItemsTab"))
    _salesOrderInformation->setCurrentIndex(1);
  else
    _salesOrderInformation->setCurrentIndex(0);

  _orderNumber->setEnabled(true);
  _orderNumberGen = 0;
  _orderNumber->clear();

  _shipDate->clear();
  _ignoreSignals = true;
  _cust->setId(-1);
  _shipTo->setId(-1);
  _ignoreSignals = false;
  _warehouse->setId(_preferences->value("PreferredWarehouse").toInt());
  _salesRep->setCurrentIndex(-1);
  _commission->clear();
  _billToAddr->setId(-1);
  _shipToAddr->setId(-1);
  _billToName->clear();
  _shipToName->clear();
  _taxZone->setCurrentIndex(-1);
  _taxzoneidCache  = -1;
  _custtaxzoneid   = -1;
  _terms->setCurrentIndex(-1);
  _shipVia->setCurrentIndex(-1);
  _shippingCharges->setCurrentIndex(-1);
  _shippingForm->setCurrentIndex(-1);
  _holdType->setCode("N");
  _holdType->setEnabled(true);
  _calcfreight   = _metrics->boolean("CalculateFreight");
  _freightCache  = 0;
  disconnect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
  _freight->clear();
  connect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
  _orderComments->clear();
  _shippingComments->clear();
  _charass->clear();
  _custPONumber->clear();
  _miscCharge->clear();
  _miscChargeDescription->clear();
  _miscChargeAccount->setId(-1);
  _subtotal->clear();
  _tax->clear();
  _miscCharge->clear();
  _total->clear();
  _cashTotal->clear();
  _orderCurrency->setCurrentIndex(0);
  _orderCurrency->setEnabled(true);
  _weight->clear();
  _allocatedCM->clear();
  _outstandingCM->clear();
  _authCC->clear();
  _balance->clear();
  _cashBalance->clear();
  _CCAmount->clear();
  _CCCVV->clear();
  _project->setId(-1);
  _fromQuoteLit->setText(tr("From Quote:"));

  _fromQuote->setText(tr("No"));

  _shipComplete->setChecked(false);

  if ( (_mode == cEdit) || (_mode == cNew) )
  {
    _mode = cNew;
    emit newModeType(OrderMode);
    emit newModeState(cNew);
    setObjectName("salesOrder new");
    _orderDateCache = omfgThis->dbDate();
    _orderDate->setDate(_orderDateCache, true);
  }
  else if ( (_mode == cEditQuote) || (_mode == cNewQuote) )
  {
    _mode = cNewQuote;
    emit newModeType(QuoteMode);
    emit newModeState(cNew);
  }

  populateOrderNumber();
  if (_orderNumber->text().isEmpty())
    _orderNumber->setFocus();
  else
    _cust->setFocus();

  XSqlQuery headid;
  if (ISORDER(_mode))
    headid.exec("SELECT NEXTVAL('cohead_cohead_id_seq') AS _soheadid");
  else
    headid.exec("SELECT NEXTVAL('quhead_quhead_id_seq') AS _soheadid");

  if (headid.first())
  {
    _soheadid = headid.value("_soheadid").toInt();
    emit newId(_soheadid);
    _comments->setId(_soheadid);
    _documents->setId(_soheadid);
    if (ISORDER(_mode))
    {
      populateCMInfo();
      populateCCInfo();
      sFillCcardList();
    }
  }
  else if (headid.lastError().type() != QSqlError::NoError) {
    if (ISORDER(_mode))
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Sales Order Information"),
                   headid, __FILE__, __LINE__);
    else
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Quote Information"),
                     headid, __FILE__, __LINE__);
  }

  _soitem->clear();
  _cust->setReadOnly(false);

  _saved = false;
}

void salesOrder::closeEvent(QCloseEvent *pEvent)
{
  if (!deleteForCancel())
  {
    pEvent->ignore();
    return;
  }

  disconnect(_orderNumber, SIGNAL(editingFinished()), this, SLOT(sHandleOrderNumber()));

  if (cNew == _mode && _saved)
    omfgThis->sSalesOrdersUpdated(-1);
  else if (cNewQuote == _mode && _saved)
    omfgThis->sQuotesUpdated(-1);

  _preferences->set("SoShowAll", _more->isChecked());

  XDocumentWindow::closeEvent(pEvent);
}

void salesOrder::sHandleShipchrg(int pShipchrgid)
{
  if ( (_mode == cView) || (_mode == cViewQuote) )
    _freight->setEnabled(false);
  else
  {
    XSqlQuery query;
    query.prepare( "SELECT shipchrg_custfreight "
                   "FROM shipchrg "
                   "WHERE (shipchrg_id=:shipchrg_id);" );
    query.bindValue(":shipchrg_id", pShipchrgid);
    query.exec();
    if (query.first())
    {
      if (query.value("shipchrg_custfreight").toBool())
      {
        _calcfreight = _metrics->boolean("CalculateFreight");
        _freight->setEnabled(true);
      }
      else
      {
        _calcfreight   = false;
        _freightCache  = 0;
        _freight->setEnabled(false);
        disconnect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
        _freight->clear();
        connect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
        sCalculateTax();
      }
    }
  }
}

void salesOrder::sHandleSalesOrderEvent(int pSoheadid, bool)
{
  if (pSoheadid == _soheadid)
    sFillItemList();
}

void salesOrder::sTaxDetail()
{
  XSqlQuery taxq;
  if (!ISVIEW(_mode))
  {
    if (ISORDER(_mode))
      taxq.prepare("UPDATE cohead SET cohead_taxzone_id=:taxzone_id, "
                   "  cohead_freight=:freight,"
                   "  cohead_orderdate=:date "
                   "WHERE (cohead_id=:head_id);");
    else
      taxq.prepare("UPDATE quhead SET quhead_taxzone_id=:taxzone_id, "
                   "  quhead_freight=:freight,"
                   "  quhead_quotedate=:date "
                   "WHERE (quhead_id=:head_id);");
    if (_taxZone->isValid())
      taxq.bindValue(":taxzone_id",        _taxZone->id());
    taxq.bindValue(":freight",        _freight->localValue());
    taxq.bindValue(":date",        _orderDate->date());
    taxq.bindValue(":head_id", _soheadid);
    taxq.exec();
    if (taxq.lastError().type() != QSqlError::NoError)
    {
      if (ISORDER(_mode))
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Updating Sales Order Information"),
                           taxq, __FILE__, __LINE__);
      else
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Updating Quote Information"),
                           taxq, __FILE__, __LINE__);
      return;
    }
  }

  ParameterList params;
  params.append("order_id", _soheadid);
  if (ISORDER(_mode))
    params.append("order_type", "S");
  else
    params.append("order_type", "Q");

  // mode => view since there are no fields to hold modified tax data
  params.append("mode", "view");

  taxBreakdown newdlg(this, "", true);
  if (newdlg.set(params) == NoError && newdlg.exec() == XDialog::Accepted)
  {
    populate();
  }
}

void salesOrder::sFreightDetail()
{
  ParameterList params;
  params.append("calcfreight", _calcfreight);
  if (ISORDER(_mode))
    params.append("order_type", "SO");
  else
    params.append("order_type", "QU");
  params.append("order_id", _soheadid);
  params.append("document_number", _orderNumber->text());
  params.append("cust_id", _cust->id());
  params.append("shipto_id", _shipTo->id());
  params.append("orderdate", _orderDate->date());
  params.append("shipvia", _shipVia->currentText());
  params.append("curr_id", _orderCurrency->id());

  // mode => view since there are no fields to hold modified freight data
  params.append("mode", "view");

  freightBreakdown newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
  if (_saved)
    populate();
}

void salesOrder::setFreeFormShipto(bool pFreeForm)
{
  bool ffShipto = pFreeForm;

  if (_mode == cView || _mode == cViewQuote)
    ffShipto = false;

  _shipToName->setEnabled(ffShipto);
  _shipToAddr->setEnabled(ffShipto);
  _shipToCntct->setEnabled(ffShipto);

  _copyToShipto->setEnabled(ffShipto);
}

void salesOrder::setViewMode()
{
  if (cEdit == _mode)
  {
    // Undo some changes set for the edit mode
    _captive = false;

    disconnect( _cust,    SIGNAL(valid(bool)),                    _new, SLOT(setEnabled(bool)));
    disconnect( omfgThis, SIGNAL(salesOrdersUpdated(int, bool)),  this, SLOT(sHandleSalesOrderEvent(int, bool)));

    _new->setEnabled(false);
  }

  _paymentInformation->removeTab(_paymentInformation->indexOf(_cashPage));
  _paymentInformation->removeTab(_paymentInformation->indexOf(_creditCardPage));

  _mode = ISORDER(_mode) ? cView : cViewQuote;
  emit newModeType(ISORDER(_mode) ? OrderMode : QuoteMode);
  emit newModeState(cView);
  setObjectName(QString("salesOrder view %1").arg(_soheadid));

  _orderNumber->setEnabled(false);
  _packDate->setEnabled(false);
  _shipDate->setEnabled(false);
  _opportunity->setEnabled(false);
  _cust->setReadOnly(true);
  _warehouse->setEnabled(false);
  _salesRep->setEnabled(false);
  _commission->setEnabled(false);
  _taxZone->setEnabled(false);
  _terms->setEnabled(false);
  _fob->setEnabled(false);
  _shipVia->setEnabled(false);
  _shippingCharges->setEnabled(false);
  _shippingForm->setEnabled(false);
  _miscCharge->setEnabled(false);
  _miscChargeDescription->setEnabled(false);
  _miscChargeAccount->setReadOnly(true);
  _miscChargeAccount->setEnabled(false);
  _freight->setEnabled(false);
  _orderComments->setEnabled(false);
  _shippingComments->setEnabled(false);
  _custPONumber->setEnabled(false);
  _holdType->setEnabled(false);
  _edit->setText(tr("View"));
  _comments->setType(Comments::SalesOrder);
  _comments->setReadOnly(true);
  _documents->setType("S");
  // _documents->setReadOnly(true); 20996, 25319, 26431
  _shipComplete->setEnabled(false);
  setFreeFormShipto(false);
  _orderCurrency->setEnabled(false);
  _printSO->setEnabled(false);
  _shippingZone->setEnabled(false);
  _saleType->setEnabled(false);
  _newCharacteristic->setEnabled(false);
  _save->hide();
  _clear->hide();
  _project->setReadOnly(true);
  if (_metrics->boolean("AlwaysShowSaveAndAdd"))
    _saveAndAdd->setEnabled(false);
  else
    _saveAndAdd->hide();
  _action->hide();
  _cancel->hide();
  _delete->hide();
}

/** @brief Delete a Sales Order by internal id.

    deleteSalesOrder method deletes a single sales order given its id.
    This enforces the common rules of the user interface, such as asking
    whether the user really wants to delete the order and providing
    alternatives if the order cannot be deleted for business reasons.

    @param pId    The internal id of the sales order to delete
    @param parent The parent window, if any, requesting the delete

    @return true if the sales order was deleted or closed, otherwise false
 */
bool salesOrder::deleteSalesOrder(int pId, QWidget *parent)
{
  // TODO: move to the delete trigger?
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkSOSitePrivs(:coheadid) AS result;");
    check.bindValue(":coheadid", pId);
    check.exec();
    if (check.first() && ! check.value("result").toBool())
    {
      QMessageBox::critical(parent, tr("Access Denied"),
                            tr("You may not delete this Sales Order as it "
                               "refers to a Site for which you have not been "
                               "granted privileges.")) ;
      return false;
    }
  }
  
  //ensure no line items have an associated open PO (drop shipped items)
  XSqlQuery qtyDropshippedq;
  qtyDropshippedq.prepare("select coalesce(bool_or((poitem_id) is not null),'f') as opendropship"
                         "  from coitem left outer join poitem on (poitem_id=coitem_order_id)"
                         "  where coitem_order_type = 'P'"
                         "  and coitem_order_id is not null"
                         "  and coitem_order_id > 0"
                         "  and poitem_status != 'C'"
                         "  and coitem_cohead_id=:coheadid");
  qtyDropshippedq.bindValue(":coheadid", pId);
  qtyDropshippedq.exec();
  if (qtyDropshippedq.first() && qtyDropshippedq.value("opendropship").toBool())
  {
    QMessageBox::critical(parent, tr("Open Dropship"),
                          tr("You may not delete this Sales Order as it "
                             "has one or more dropshipped line items "
                             "on a Purchase Order.")) ;
    return false;
  }
  else if (ErrorReporter::error(QtCriticalMsg, parent,
                              tr("Getting Linked PO Items"),
                              qtyDropshippedq, __FILE__, __LINE__))
  return false;
  

  XSqlQuery atshippingq;
  atshippingq.prepare("SELECT BOOL_OR(qtyAtShipping(coitem_id) > 0) AS atshipping"
                      "  FROM coitem"
                      " WHERE (coitem_cohead_id=:coheadid);");
  atshippingq.bindValue(":coheadid", pId);
  atshippingq.exec();
  if (atshippingq.first() && atshippingq.value("atshipping").toBool())
  {
    QMessageBox::critical(parent, tr("At Shipping"),
                          tr("You may not delete this Sales Order as it "
                             "has one or more unshipped line items with "
                             "inventory at shipping.")) ;
    return false;
  }
  else if (ErrorReporter::error(QtCriticalMsg, parent,
                                tr("Getting At Shipping Information"),
                                atshippingq, __FILE__, __LINE__))
    return false;

  QString question = tr("<p>Are you sure that you want to completely "
			 "delete the selected Sales Order?");
  XSqlQuery woq;
  woq.prepare("SELECT BOOL_OR(woStarted(coitem_order_id)) AS workstarted"
              "    FROM coitem"
              "   WHERE ((coitem_order_type='W')"
              "      AND (coitem_cohead_id=:coheadid));");
  woq.bindValue(":coheadid", pId);
  woq.exec();
  if (woq.first() && woq.value("workstarted").toBool())
    question = tr("<p>A work order for one of the line items on the selected "
                  "Sales Order is already in progress. Are you sure that you "
                  "want to completely delete the Sales Order?");
  else if (ErrorReporter::error(QtCriticalMsg, parent,
                                tr("Getting Work Order Information"),
                                woq, __FILE__, __LINE__))
    return false;

  if (QMessageBox::question(parent, tr("Delete Sales Order?"), question,
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    XSqlQuery delq;
    delq.prepare("SELECT deleteSo(:sohead_id) AS result;");
    delq.bindValue(":sohead_id", pId);
    delq.exec();
    if (delq.first())
    {
      bool cancelInstead = false;
      int result = delq.value("result").toInt();
      if (result == -1 && _privileges->check("ProcessCreditCards"))
      {
        if (QMessageBox::question(parent, tr("Cannot Delete Sales Order"),
                                   storedProcErrorLookup("deleteSo", result) +
                                   "<br>Would you like to refund the amount "
                                   "charged and cancel the Sales Order instead?",
                                   QMessageBox::Yes | QMessageBox::Default,
                                   QMessageBox::No) == QMessageBox::Yes)
        {
          CreditCardProcessor *cardproc = CreditCardProcessor::getProcessor();
          if (! cardproc)
            QMessageBox::critical(parent, tr("Credit Card Processing Error"),
                                  CreditCardProcessor::errorMsg());
          else
          {
            // TODO: must we loop and generate a distinct credit for each ccpay?
            XSqlQuery ccq;
            ccq.prepare("SELECT ccpay_id, ccpay_ccard_id, ccpay_curr_id,"
                        "       SUM(ccpay_amount     * sense) AS amount,"
                        "       SUM(ccpay_r_tax      * sense) AS tax,"
                        "       SUM(ccpay_r_shipping * sense) AS freight,"
                        "       (SELECT cohead_number"
                        "          FROM cohead"
                        "         WHERE cohead_id=:coheadid) AS docnum"
                        "  FROM (SELECT ccpay_id, ccpay_ccard_id, ccpay_curr_id,"
                        "             CASE WHEN ccpay_status = 'C' THEN  1"
                        "                  WHEN ccpay_status = 'R' THEN -1"
                        "             END AS sense,"
                        "             ccpay_amount,"
                        "             CASE WHEN ccpay_r_tax = ''    THEN 0"
                        "                  WHEN ccpay_r_tax IS NULL THEN 0"
                        "                  ELSE CAST(ccpay_r_tax AS NUMERIC)"
                        "             END AS ccpay_r_tax,"
                        "             CASE WHEN ccpay_r_shipping = ''    THEN 0"
                        "                  WHEN ccpay_r_shipping IS NULL THEN 0"
                        "                  ELSE CAST(ccpay_r_shipping AS NUMERIC)"
                        "             END AS ccpay_r_shipping"
                        "      FROM ccpay JOIN payco ON (ccpay_id=payco_ccpay_id)"
                        "      WHERE ((ccpay_status IN ('C', 'R'))"
                        "        AND  (payco_cohead_id=:coheadid))"
                        "      ) AS dummy "
                        "GROUP BY ccpay_id, ccpay_ccard_id, ccpay_curr_id;");
            ccq.bindValue(":coheadid", pId);
            ccq.exec();
            if (ccq.first())
            do
            {
              QString docnum = ccq.value("docnum").toString();
              QString refnum = docnum;
              int ccpayid    = ccq.value("ccpay_id").toInt();
              int coheadid   = pId;
              int returnVal = cardproc->credit(ccq.value("ccpay_ccard_id").toInt(),
                                               "-2",
                                               ccq.value("amount").toDouble(),
                                               ccq.value("tax").toDouble(),
                                               true,
                                               ccq.value("freight").toDouble(),
                                               0,
                                               ccq.value("ccpay_curr_id").toInt(),
                                               docnum, refnum, ccpayid,
                                               "cohead", coheadid);
              if (returnVal < 0)
              {
                QMessageBox::critical(parent, tr("Credit Card Processing Error"),
                                      cardproc->errorMsg());
                return false;
              }
              else if (returnVal > 0)
              {
                QMessageBox::warning(parent, tr("Credit Card Processing Warning"),
                                     cardproc->errorMsg());
                cancelInstead = true;
              }
              else if (! cardproc->errorMsg().isEmpty())
              {
                QMessageBox::information(parent, tr("Credit Card Processing Note"),
                                     cardproc->errorMsg());
                cancelInstead = true;
              }
              else
                cancelInstead = true;
            } while (ccq.next());
            else if (ErrorReporter::error(QtCriticalMsg, parent,
                                          tr("Credit Card Processing Error"),
                                          ccq, __FILE__, __LINE__))
              return false;
            else
            {
              ErrorReporter::error(QtCriticalMsg, parent,
                                   tr("Credit Card Processing Error"),
                                   tr("Could not find the ccpay records!"),
                                   __FILE__, __LINE__);
              return false;
            }

          }
        }
      }
      else if (result == -2 || result == -5)
      {
        if ( QMessageBox::question(parent, tr("Cannot Delete Sales Order"),
                                   storedProcErrorLookup("deleteSo", result) +
                                   "<br>Would you like to Cancel the selected "
                                   "Sales Order instead?",
                                   QMessageBox::Yes | QMessageBox::Default,
                                   QMessageBox::No) == QMessageBox::Yes)
          cancelInstead = true;
      }
      else if (result == -20)
        QMessageBox::information(parent, "Cannot Delete Purchase Order",
                                 storedProcErrorLookup("deleteSo", result));
      else if (result < 0)
      {
        ErrorReporter::error(QtCriticalMsg, parent, tr("Error Deleting Sales Order"),
                               storedProcErrorLookup("deleteSo", result),
                               __FILE__, __LINE__);
        return false;
      }

      if (cancelInstead)
      {
      	//cancel all items with a status that is not already CLOSED
        XSqlQuery cancelq;
        cancelq.prepare( "UPDATE coitem "
                   "SET coitem_status='X' "
                   "WHERE ((coitem_status <> 'C')"
                   "  AND  (coitem_cohead_id=:sohead_id));" );
        cancelq.bindValue(":sohead_id", pId);
        cancelq.exec();
        if (ErrorReporter::error(QtCriticalMsg, parent, tr("Error Cancelling"),
                                 cancelq, __FILE__, __LINE__))
          return false;
      }

      omfgThis->sSalesOrdersUpdated(-1);
      omfgThis->sProjectsUpdated(-1);

      return true;
    }
    else if (ErrorReporter::error(QtCriticalMsg, parent,
                                  tr("Deleting Sales Order"),
                                  delq, __FILE__, __LINE__))
      return false;
  }

  return false;
}

void salesOrder::newSalesOrder(int pCustid, QWidget *parent)
{
  // Check for an Item window in new mode already.
  if (pCustid == -1)
  {
    QWidgetList list = omfgThis->windowList();
    for (int i = 0; i < list.size(); i++)
    {
      QWidget *w = list.at(i);
      if (QString::compare(w->objectName(), "salesOrder new")==0)
      {
        w->setFocus();
        if (omfgThis->showTopLevel())
        {
          w->raise();
          w->activateWindow();
        }
        return;
      }
    }
  }

  // If none found then create one.
  ParameterList params;
  params.append("mode", "new");
  if (pCustid != -1)
    params.append("cust_id", pCustid);

  salesOrder *newdlg = new salesOrder(parent);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void salesOrder::editSalesOrder( int pId, bool enableSaveAndAdd, QWidget *parent )
{
  // Check for an Item window in edit mode for the specified salesOrder already.
  QString     n    = QString("salesOrder edit %1").arg(pId);
  QWidgetList list = omfgThis->windowList();
  for (int i = 0; i < list.size(); i++)
  {
    QWidget *w = list.at(i);
    if (QString::compare(w->objectName(), n)==0)
    {
      w->setFocus();
      if (omfgThis->showTopLevel())
      {
        w->raise();
        w->activateWindow();
      }
      return;
    }
  }

  // If none found then create one.
  ParameterList params;
  params.append("mode", "edit");
  params.append("sohead_id", pId);
  if (enableSaveAndAdd)
    params.append("enableSaveAndAdd");

  salesOrder *newdlg = new salesOrder(parent);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void salesOrder::viewSalesOrder( int pId, QWidget *parent )
{
  // Check for an Item window in edit mode for the specified salesOrder already.
  QString     n    = QString("salesOrder view %1").arg(pId);
  QWidgetList list = omfgThis->windowList();
  for (int i = 0; i < list.size(); i++)
  {
    QWidget *w = list.at(i);
    if (QString::compare(w->objectName(), n)==0)
    {
      w->setFocus();
      if (omfgThis->showTopLevel())
      {
        w->raise();
        w->activateWindow();
      }
      return;
    }
  }

  // If none found then create one.
  ParameterList params;
  params.append("mode", "view");
  params.append("sohead_id", pId);

  salesOrder *newdlg = new salesOrder(parent);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void salesOrder::sNewCharacteristic()
{
  ParameterList params;
  params.append("mode", "new");
  if (ISQUOTE(_mode))
    params.append("quhead_id", _soheadid);
  else
    params.append("sohead_id", _soheadid);

  characteristicAssignment newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillCharacteristic();
}

void salesOrder::sEditCharacteristic()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("charass_id", _charass->id());

  characteristicAssignment newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillCharacteristic();
}

void salesOrder::sDeleteCharacteristic()
{
  XSqlQuery itemDelete;
  itemDelete.prepare( "DELETE FROM charass "
                     "WHERE (charass_id=:charass_id);" );
  itemDelete.bindValue(":charass_id", _charass->id());
  itemDelete.exec();

  sFillCharacteristic();
}

void salesOrder::sFillCharacteristic()
{
  XSqlQuery charassq;
  charassq.prepare( "SELECT charass_id, char_name, "
                    " CASE WHEN char_type < 2 THEN "
                    "   charass_value "
                    " ELSE "
                    "   formatDate(charass_value::date) "
                    "END AS charass_value "
                    "FROM charass JOIN char ON (char_id=charass_char_id) "
                    "WHERE ( (charass_target_type=:target_type)"
                    "  AND   (charass_target_id=:target_id) ) "
                    "ORDER BY char_order, char_name;" );
  charassq.bindValue(":target_id", _soheadid);
  if (ISQUOTE(_mode))
    charassq.bindValue(":target_type", "QU");
  else
    charassq.bindValue(":target_type", "SO");
  charassq.exec();
  _charass->populate(charassq);
}

void salesOrder::populateCMInfo()
{
  XSqlQuery populateSales;
  if (cNew != _mode && cEdit != _mode && cView != _mode)
    return;

  // Allocated C/M's and posted Invoices for partial shipments
  populateSales.prepare("SELECT COALESCE(SUM(currToCurr(aropenalloc_curr_id, :curr_id,"
                        "                               aropenalloc_amount, :effective)),0) AS amount"
                        "  FROM aropenalloc JOIN aropen ON (aropen_id=aropenalloc_aropen_id) "
                        " WHERE ( (aropenalloc_doctype='S')"
                        "  AND    (aropenalloc_doc_id=:doc_id) );");
  populateSales.bindValue(":doc_id",    _soheadid);
  populateSales.bindValue(":curr_id",   _allocatedCM->id());
  populateSales.bindValue(":effective", _allocatedCM->effective());
  populateSales.exec();
  if (populateSales.first())
    _allocatedCM->setLocalValue(populateSales.value("amount").toDouble());
  else
    _allocatedCM->setLocalValue(0);

  populateSales.prepare("SELECT COALESCE(SUM(currToCurr(invchead_curr_id, :curr_id,"
                        "                               calcInvoiceAmt(invchead_id), :effective)),0) AS amount"
						" FROM invchead where invchead_id IN"
						"      (select invchead_id from coitem join invcitem on (invcitem_coitem_id=coitem_id) join invchead on (invchead_id=invcitem_invchead_id and invchead_posted) where (coitem_cohead_id=:doc_id) group by invchead_id)");

  populateSales.bindValue(":doc_id",    _soheadid);
  populateSales.bindValue(":curr_id",   _allocatedCM->id());
  populateSales.bindValue(":effective", _allocatedCM->effective());
  populateSales.exec();
  if (populateSales.first())
    _allocatedCM->setLocalValue(_allocatedCM->localValue() + populateSales.value("amount").toDouble());

  // Unallocated C/M's
  populateSales.prepare("SELECT SUM(amount) AS f_amount"
                        " FROM (SELECT aropen_id,"
                        "              noNeg(currToCurr(aropen_curr_id, :curr_id, (aropen_amount - aropen_paid), :effective) - "
                        "                    SUM(currToCurr(aropenalloc_curr_id, :curr_id, COALESCE(aropenalloc_amount,0), :effective))) AS amount "
                        "       FROM cohead JOIN aropen ON (aropen_cust_id=cohead_cust_id) "
                        "                   LEFT OUTER JOIN aropenalloc ON (aropenalloc_aropen_id=aropen_id)"
                        "       WHERE ( (aropen_doctype IN ('C', 'R'))"
                        "         AND   (aropen_open)"
                        "         AND   (cohead_id=:cohead_id) )"
                        "       GROUP BY aropen_id, aropen_amount, aropen_paid, aropen_curr_id) AS data; ");
  populateSales.bindValue(":cohead_id", _soheadid);
  populateSales.bindValue(":curr_id",   _outstandingCM->id());
  populateSales.bindValue(":effective", _outstandingCM->effective());
  populateSales.exec();
  if (populateSales.first())
    _outstandingCM->setLocalValue(populateSales.value("f_amount").toDouble());
  else
    _outstandingCM->setLocalValue(0);
}

void salesOrder::populateCCInfo()
{
  XSqlQuery populateSales;
  if (cNew != _mode && cEdit != _mode && cView != _mode)
    return;

  int ccValidDays = _metrics->value("CCValidDays").toInt();
  if (ccValidDays < 1)
    ccValidDays = 7;

  populateSales.prepare("SELECT COALESCE(SUM(currToCurr(payco_curr_id, :curr_id,"
            "                               payco_amount, :effective)),0) AS amount"
            "  FROM ccpay, payco"
            " WHERE ( (ccpay_status = 'A')"
            "   AND   (date_part('day', CURRENT_TIMESTAMP - ccpay_transaction_datetime) < :ccValidDays)"
            "   AND   (payco_ccpay_id=ccpay_id)"
            "   AND   (payco_cohead_id=:cohead_id) ); ");
  populateSales.bindValue(":cohead_id", _soheadid);
  populateSales.bindValue(":ccValidDays", ccValidDays);
  populateSales.bindValue(":curr_id",   _authCC->id());
  populateSales.bindValue(":effective", _authCC->effective());
  populateSales.exec();
  if (populateSales.first())
    _authCC->setLocalValue(populateSales.value("amount").toDouble());
  else
    _authCC->setLocalValue(0);
}

void salesOrder::sNewCreditCard()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("cust_id", _cust->id());

  creditCard newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillCcardList();
}

void salesOrder::sEditCreditCard()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cust_id", _cust->id());
  params.append("ccard_id", _cc->id());

  creditCard newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillCcardList();
}

void salesOrder::sViewCreditCard()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cust_id", _cust->id());
  params.append("ccard_id", _cc->id());

  creditCard newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void salesOrder::sMoveUp()
{
  XSqlQuery moveSales;
  moveSales.prepare("SELECT moveCcardUp(:ccard_id) AS result;");
  moveSales.bindValue(":ccard_id", _cc->id());
  moveSales.exec();

    sFillCcardList();
}

void salesOrder::sMoveDown()
{
  XSqlQuery moveSales;
  moveSales.prepare("SELECT moveCcardDown(:ccard_id) AS result;");
  moveSales.bindValue(":ccard_id", _cc->id());
  moveSales.exec();

    sFillCcardList();
}

void salesOrder::sFillCcardList()
{
  if (_cust->id() == -1 || ISQUOTE(_mode) ||
      ! _metrics->boolean("CCAccept") || ! _privileges->check("ProcessCreditCards"))
  {
    _cc->clear();
    return;
  }

  XSqlQuery fillSales;
  fillSales.prepare( "SELECT expireCreditCard(:cust_id, setbytea(:key));");
  fillSales.bindValue(":cust_id", _cust->id());
  fillSales.bindValue(":key", omfgThis->_key);
  fillSales.exec();

  MetaSQLQuery  mql = mqlLoad("creditCards", "detail");
  ParameterList params;
  params.append("cust_id",         _cust->id());
  params.append("masterCard",      tr("MasterCard"));
  params.append("visa",            tr("VISA"));
  params.append("americanExpress", tr("American Express"));
  params.append("discover",        tr("Discover"));
  params.append("other",           tr("Other"));
  params.append("key",             omfgThis->_key);
  params.append("activeonly",      true);
  XSqlQuery cl = mql.toQuery(params);
  _cc->populate(cl);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Credit Card Information"),
                                cl, __FILE__, __LINE__))
  {
    return;
  }
}

void salesOrder::sAuthorizeCC()
{
  if (!okToProcessCC())
    return;

  CreditCardProcessor *cardproc = CreditCardProcessor::getProcessor();
  if (!cardproc)
  {
    QMessageBox::critical(this, tr("Credit Card Processing Error"),
                          CreditCardProcessor::errorMsg());
    return;
  }

  if (!cardproc->errorMsg().isEmpty())
  {
    QMessageBox::warning( this, tr("Credit Card Error"), cardproc->errorMsg() );
    return;
  }

  _authorize->setEnabled(false);
  _charge->setEnabled(false);

  int     ccpayid    = -1;
  QString sonumber   = _orderNumber->text();
  QString ponumber   = _custPONumber->text();
  int     returnVal  = cardproc->authorize(_cc->id(), _CCCVV->text(),
                                           _CCAmount->localValue(),
                                           _tax->localValue(),
                                           (_tax->isZero() && _taxZone->id() == -1),
                                           _freight->localValue(), 0,
                                           _CCAmount->id(),
                                           sonumber, ponumber, ccpayid,
                                           QString("cohead"), _soheadid);
  if (returnVal < 0)
    QMessageBox::critical(this, tr("Credit Card Processing Error"),
                          cardproc->errorMsg());
  else if (returnVal > 0)
    QMessageBox::warning(this, tr("Credit Card Processing Warning"),
                         cardproc->errorMsg());
  else if (!cardproc->errorMsg().isEmpty())
    QMessageBox::information(this, tr("Credit Card Processing Note"),
                             cardproc->errorMsg());
  else
    _CCAmount->clear();

  _authorize->setEnabled(true);
  _charge->setEnabled(true);

  populateCMInfo();
  populateCCInfo();
  sFillCcardList();
  _CCCVV->clear();
}

void salesOrder::sChargeCC()
{
  if (!okToProcessCC())
    return;

  CreditCardProcessor *cardproc = CreditCardProcessor::getProcessor();
  if (!cardproc)
  {
    QMessageBox::critical(this, tr("Credit Card Processing Error"),
                          CreditCardProcessor::errorMsg());
    return;
  }

  if (!cardproc->errorMsg().isEmpty())
  {
    QMessageBox::warning( this, tr("Credit Card Error"), cardproc->errorMsg() );
    return;
  }

  _authorize->setEnabled(false);
  _charge->setEnabled(false);

  int     ccpayid    = -1;
  QString ordernum   = _orderNumber->text();
  QString refnum     = _custPONumber->text();
  int     returnVal  = cardproc->charge(_cc->id(), _CCCVV->text(),
                                        _CCAmount->localValue(),
                                        _tax->localValue(),
                                        (_tax->isZero() && _taxZone->id() == -1),
                                        _freight->localValue(), 0,
                                        _CCAmount->id(),
                                        ordernum, refnum, ccpayid,
                                        QString("cohead"), _soheadid);
  if (returnVal < 0)
    QMessageBox::critical(this, tr("Credit Card Processing Error"),
                          cardproc->errorMsg());
  else if (returnVal > 0)
    QMessageBox::warning(this, tr("Credit Card Processing Warning"),
                         cardproc->errorMsg());
  else if (!cardproc->errorMsg().isEmpty())
    QMessageBox::information(this, tr("Credit Card Processing Note"),
                             cardproc->errorMsg());
  else
    _CCAmount->clear();

  _authorize->setEnabled(true);
  _charge->setEnabled(true);

  populateCMInfo();
  populateCCInfo();
  sFillCcardList();
  _CCCVV->clear();
}

bool salesOrder::okToProcessCC()
{
  XSqlQuery okSales;
  if (_usesPos)
  {
    if (_custPONumber->text().trimmed().length() == 0)
    {
      QMessageBox::warning( this, tr("Cannot Process Credit Card Transaction"),
                              tr("<p>You must enter a Customer P/O for this "
                                 "Sales Order before you may process a credit"
                                 "card transaction.") );
      _custPONumber->setFocus();
      return false;
    }

    if (!_blanketPos)
    {
      okSales.prepare( "SELECT cohead_id"
                 "  FROM cohead"
                 " WHERE ((cohead_cust_id=:cohead_cust_id)"
                 "   AND  (cohead_id<>:cohead_id)"
                 "   AND  (UPPER(cohead_custponumber) = UPPER(:cohead_custponumber)) )"
                 " UNION "
                 "SELECT quhead_id"
                 "  FROM quhead"
                 " WHERE ((quhead_cust_id=:cohead_cust_id)"
                 "   AND  (quhead_id<>:cohead_id)"
                 "   AND  (UPPER(quhead_custponumber) = UPPER(:cohead_custponumber)) );" );
      okSales.bindValue(":cohead_cust_id", _cust->id());
      okSales.bindValue(":cohead_id", _soheadid);
      okSales.bindValue(":cohead_custponumber", _custPONumber->text());
      okSales.exec();
      if (okSales.first())
      {
        QMessageBox::warning( this, tr("Cannot Process Credit Card Transaction"),
                              tr("<p>This Customer does not use Blanket P/O "
                                   "Numbers and the P/O Number you entered has "
                                   "already been used for another Sales Order. "
                                   "Please verify the P/O Number and either "
                                   "enter a new P/O Number or add to the "
                                   "existing Sales Order." ) );
        _custPONumber->setFocus();
        return false;
      }
    }
  }

  return true;
}

void salesOrder::sReturnStock()
{
  XSqlQuery returnSales;
  returnSales.prepare("SELECT returnItemShipments(:soitem_id) AS result;");
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    returnSales.bindValue(":soitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
    returnSales.exec();
    if (returnSales.lastError().type() != QSqlError::NoError)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Return Item Information"),
                           returnSales, __FILE__, __LINE__);
      continue;
    }
  }

  sFillItemList();
}

void salesOrder::sIssueStock()
{
  if (!creditLimitCheckIssue())
    return;

  bool update = false;
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem *soitem = (XTreeWidgetItem *)(selected[i]);
    if (soitem->altId() != 1 && soitem->altId() != 4)
    {
      ParameterList params;
      params.append("soitem_id", soitem->id());

      if (_requireInventory->isChecked())
        params.append("requireInventory");

      issueLineToShipping newdlg(this, "", true);
      newdlg.set(params);
      if (newdlg.exec() != XDialog::Rejected)
        update = true;
    }
  }

  if (update)
  {
    sFillItemList();
    sPopulateShipments();
  }
}

void salesOrder::sIssueLineBalance()
{
  if (!creditLimitCheckIssue())
    return;

  XSqlQuery issueSales;
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem *soitem = (XTreeWidgetItem *)(selected[i]);
    if (soitem->altId() != 1 && soitem->altId() != 4)
    {
      // sufficientInventoryToShipItem assumes line balance if qty not passed
      issueSales.prepare("SELECT itemsite.itemsite_id, item.item_number, warehous_code, itemsite.itemsite_costmethod, "
                         "  sufficientInventoryToShipItem('SO', coitem_id) AS isqtyavail, "
                         "  isControlledItemsite(itemsite.itemsite_id) AS controlled, "
                         "  isControlledItemsite(wo_itemsite_id) AS woItemControlled, "
                         "  wo_itemsite_id, "
                         "  calcIssueToShippingLineBalance('SO', coitem_id) AS balance, "
                         "  coitem_qty_invuomratio, wo_id, "
                         "  CASE WHEN wo_id IS NOT NULL THEN "
                         "    roundQty(woitem.item_fractional, calcIssueToShippingLineBalance('SO', coitem_id) "
                         "    * coitem_qty_invuomratio) "
                         "  ELSE NULL END AS postprodqty "
                         "FROM coitem JOIN itemsite ON (itemsite_id=coitem_itemsite_id) "
                         "  JOIN item ON (item_id=itemsite_item_id) "
                         "  JOIN whsinfo ON (warehous_id=itemsite_warehous_id) "
                         "  LEFT OUTER JOIN wo ON coitem_id = wo_ordid AND wo_ordtype = 'S' "
                         "  LEFT OUTER JOIN itemsite AS woitemsite ON woitemsite.itemsite_id = wo_itemsite_id "
                         "  LEFT OUTER JOIN item AS woitem ON woitemsite.itemsite_item_id = woitem.item_id "
                         "WHERE (coitem_id=:soitem_id);");
      issueSales.bindValue(":soitem_id", soitem->id());
      issueSales.exec();
      if (!issueSales.first() || ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Item Information"),
                                    issueSales, __FILE__, __LINE__))
      {
        return;
      }
      // Validate business logic
      if (_requireInventory->isChecked() &&
          issueSales.value("isqtyavail").toInt() < 0 &&
          issueSales.value("itemsite_costmethod").toString() != "J")
      {
        QMessageBox::critical(this, tr("Insufficient Inventory"),
                                    tr("<p>There is not enough Inventory to issue the amount required"
                                       " of Item %1 in Site %2.")
                              .arg(issueSales.value("item_number").toString())
                              .arg(issueSales.value("warehous_code").toString()) );
        return;
      }
      // Validate more business logic
      if (issueSales.value("controlled").toBool() && 
          issueSales.value("isqtyavail").toInt() < 0 && 
          issueSales.value("itemsite_costmethod").toString() != "J")
      {
        QMessageBox::critical(this, tr("Insufficient Inventory"),
                                tr("<p>Item Number %1 in Site %2 is a Multiple Location or "
                                   "Lot/Serial controlled Item which is short on Inventory. "
                                   "This transaction cannot be completed as is. Please make "
                                   "sure there is sufficient Quantity on Hand before proceeding.")
                              .arg(issueSales.value("item_number").toString())
                              .arg(issueSales.value("warehous_code").toString()));
        return;
      }

      double balance = issueSales.value("balance").toDouble();
      bool jobItem = (issueSales.value("itemsite_costmethod").toString() == "J" && balance > 0);
      bool controlled = issueSales.value("controlled").toBool();
      bool hasControlledBackflushItems = false;
      int itemsiteId = issueSales.value("itemsite_id").toInt();
      int invhistid = 0;
      int itemlocSeries = 0;

      XSqlQuery parentItemlocdist;
      XSqlQuery womatlItemlocdist;
      XSqlQuery parentSeries;
      XSqlQuery issue;
      XSqlQuery cleanup;
      XSqlQuery rollback;
      rollback.prepare("ROLLBACK;");

      // Stage cleanup functions to be called on error
      cleanup.prepare("SELECT deleteitemlocseries(:itemlocSeries, TRUE);");

      // Series for issueToShipping
      parentSeries.prepare("SELECT NEXTVAL('itemloc_series_seq') AS result;");
      parentSeries.exec();
      if (parentSeries.first() && parentSeries.value("result").toInt() > 0)
      {
        itemlocSeries = parentSeries.value("result").toInt();
        cleanup.bindValue(":itemlocSeries", itemlocSeries);
      }
      else
      {
        ErrorReporter::error(QtCriticalMsg, this, tr("Failed to Retrieve the Next itemloc_series_seq"),
          parentSeries, __FILE__, __LINE__);
        return;
      }

      // Stage this here so that, if job item, some of the params can be overriden with WO relavant values.
      parentItemlocdist.prepare("SELECT createItemlocdistParent(:itemsite_id, :qty, :orderType, :orderitemId, "
        ":itemlocSeries, NULL, NULL, :transType) AS result;");
      parentItemlocdist.bindValue(":itemsite_id", itemsiteId);
      parentItemlocdist.bindValue(":qty", balance * issueSales.value("coitem_qty_invuomratio").toDouble() * -1);
      parentItemlocdist.bindValue(":orderitemId", soitem->id());
      parentItemlocdist.bindValue(":itemlocSeries", itemlocSeries);
      parentItemlocdist.bindValue(":orderType", "SO");
      parentItemlocdist.bindValue(":transType", "SH");
      
      // If this is a lot/serial controlled job item, we need to post production first
      if (jobItem)
      {
        // Handle creation of itemlocdist records for each eligible backflush item (sql below from postProduction backflush handling)
        XSqlQuery backflushItems;
        backflushItems.prepare(
          "SELECT item_number, item_fractional, itemsite_id, itemsite_item_id, womatl_id, womatl_wo_id, "
          // issueMaterial qty = noNeg(expected - consumed)
          " noNeg(((womatl_qtyfxd + ((:qty + wo_qtyrcv) * womatl_qtyper)) * (1 + womatl_scrap)) - "
          "   (womatl_qtyiss + "
          "   CASE WHEN (womatl_qtywipscrap >  ((womatl_qtyfxd + (:qty + wo_qtyrcv) * womatl_qtyper) * womatl_scrap)) "
          "        THEN (womatl_qtyfxd + (:qty + wo_qtyrcv) * womatl_qtyper) * womatl_scrap "
          "        ELSE womatl_qtywipscrap END)) AS qtyToIssue "
          "FROM womatl, wo, itemsite, item "
          "WHERE womatl_issuemethod IN ('L', 'M') "
          " AND womatl_wo_id=wo_id "
          " AND womatl_itemsite_id=itemsite_id "
          " AND wo_ordid = :coitem_id "
          " AND wo_ordtype = 'S' "
          " AND itemsite_item_id=item_id "
          " AND isControlledItemsite(itemsite_id) "
          "ORDER BY womatl_id;");
        backflushItems.bindValue(":qty", issueSales.value("postprodqty").toDouble());
        backflushItems.bindValue(":coitem_id", soitem->id());
        backflushItems.exec();
        while (backflushItems.next())
        {
          if (backflushItems.value("qtyToIssue").toDouble() > 0)
          {
            hasControlledBackflushItems = true;
            womatlItemlocdist.prepare("SELECT createItemlocdistParent(:itemsite_id, roundQty(:item_fractional, itemuomtouom(:item_id, womatl_uom_id, NULL, :qty)) * -1, 'WO', womatl_wo_id, "
                                      " :itemlocSeries, NULL, NULL, 'IM') AS result "
                                      "FROM womatl "
                                      "WHERE womatl_id = :womatl_id;");
            womatlItemlocdist.bindValue(":itemsite_id", backflushItems.value("itemsite_id").toInt());
            womatlItemlocdist.bindValue(":item_id", backflushItems.value("itemsite_item_id").toInt());
            womatlItemlocdist.bindValue(":item_fractional", backflushItems.value("item_fractional").toBool());
            womatlItemlocdist.bindValue(":womatl_id", backflushItems.value("womatl_id").toInt());
            womatlItemlocdist.bindValue(":qty", backflushItems.value("qtyToIssue").toDouble());
            womatlItemlocdist.bindValue(":itemlocSeries", itemlocSeries);
            womatlItemlocdist.exec();
            if (!womatlItemlocdist.first())
            {
              cleanup.exec();
              QMessageBox::information( this, tr("Issue Line to Shipping"), 
                tr("Failed to Create an itemlocdist record for work order backflushed material item %1.")
                .arg(backflushItems.value("item_number").toString()) );
              return;
            }
            else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating itemlocdist Records"),
              womatlItemlocdist, __FILE__, __LINE__))
            {
              cleanup.exec();
              return;
            }
          }
        }

        // If it's a controlled job item, set the relavant params
        if (issueSales.value("woItemControlled").toBool())
        {
          parentItemlocdist.bindValue(":itemsite_id", issueSales.value("wo_itemsite_id").toInt());
          parentItemlocdist.bindValue(":orderitemId", issueSales.value("wo_id").toInt());
          parentItemlocdist.bindValue(":orderType", "WO");
          parentItemlocdist.bindValue(":transType", "RM");
          parentItemlocdist.bindValue(":qty", issueSales.value("postprodqty").toDouble());
        } 
      }

      // Create the itemlocdist record if controlled item and distribute detail if controlled or controlled backflush items
      if (controlled || (issueSales.value("woItemControlled").toBool() && jobItem) || hasControlledBackflushItems)
      {
        // If controlled item, execute the sql to create the parent itemlocdist record 
        // (for WO post prod item if job, else for issue to shipping transaction).
        if (controlled || (issueSales.value("woItemControlled").toBool() && jobItem))
        {
          parentItemlocdist.exec();
          if (!parentItemlocdist.first())
          {
            cleanup.exec();
            QMessageBox::information( this, tr("Issue to Shipping"), tr("Error creating itemlocdist records for controlled item") );
            return;
          }
          else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating itemlocdist Records"),
                                    parentItemlocdist, __FILE__, __LINE__))
          {
            cleanup.exec();
            return;
          }
        }

        if (distributeInventory::SeriesAdjust(itemlocSeries, this, QString(), QDate(),
          QDate(), true) == XDialog::Rejected)
        {
          cleanup.exec();
          QMessageBox::information( this, tr("Issue to Shipping"), tr("Issue Canceled") );
          return;
        }
      } 

      // Wrap remaining sql in a transaction block - perform postSoItemProduction if Job item, then issue to shipping
      issue.exec("BEGIN;");

      // postSoItemProduction if Job item
      if (jobItem)
      {
        XSqlQuery prod;
        prod.prepare("SELECT postSoItemProduction(:soitem_id, now(), :itemlocSeries, TRUE) AS result;");
        prod.bindValue(":soitem_id", soitem->id());
        prod.bindValue(":itemlocSeries", itemlocSeries);
        prod.exec();
        if (prod.first())
        {
          int result = prod.value("result").toInt();

          if (result < 0 || result != itemlocSeries)
          {
            rollback.exec();
            cleanup.exec();
            ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Production"),
                                   storedProcErrorLookup("postProduction", result),
                                   __FILE__, __LINE__);
            return;
          }

          // If controlled item, get the inventory history from post production trans. 
          // so we can create itemlocdist records for issue to shipping transaction and auto-distribute to them in postInvTrans.
          if (issueSales.value("woItemControlled").toBool())
          {
            prod.prepare("SELECT invhist_id "
                         "FROM invhist "
                         "WHERE ((invhist_series = :itemlocseries) "
                         " AND (invhist_transtype = 'RM')); ");
            prod.bindValue(":itemlocseries" , itemlocSeries);
            prod.exec();
            if (prod.first())
              invhistid = prod.value("invhist_id").toInt();
            else
            {
              rollback.exec();
              cleanup.exec();
              ErrorReporter::error(QtCriticalMsg, this, tr("Error Occurred"),
                                   tr("Inventory history not found")
                                   .arg(windowTitle()),__FILE__,__LINE__);
              return;
            }
          }
        }
        else
        {
          rollback.exec();
          cleanup.exec();
          ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Production for Job Item"),
            prod, __FILE__,__LINE__);
          return;
        }
      }

      // issueToShipping instead of issueLineBalanceToShipping because we have already calculated the balance
      issueSales.prepare("SELECT issueToShipping('SO', :soitem_id, :qty, :itemlocseries, now(), "
                         ":invhist_id, FALSE, TRUE) AS result;");
      issueSales.bindValue(":soitem_id", soitem->id());
      issueSales.bindValue(":qty", balance);
      issueSales.bindValue(":itemlocseries", itemlocSeries);
      if (invhistid > 0)
        issueSales.bindValue(":invhist_id", invhistid);
      issueSales.exec();
      if (issueSales.lastError().type() != QSqlError::NoError)
      {
        rollback.exec();
        cleanup.exec();
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Item Information"),
                             issueSales, __FILE__, __LINE__);
        return;
      }
      if (issueSales.first())
      {
        int result = issueSales.value("result").toInt();
        if (result < 0 || result != itemlocSeries)
        {
          rollback.exec();
          cleanup.exec();
          ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Item Information"),
                               storedProcErrorLookup("issueLineBalanceToShipping", result),
                               __FILE__, __LINE__);
          return;
        }

        issueSales.exec("COMMIT;");
      }
      else
      {
        rollback.exec();
        cleanup.exec();
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Item Information"),
                             issueSales, __FILE__, __LINE__);
        return;
      }
    }
  }

  sFillItemList();
  sPopulateShipments();
}

void salesOrder::sFreightChanged()
{
  if (_freight->localValue() == _freightCache)
    return;

  if (_freight->isEnabled())
  {
    if (_calcfreight)
    {
      int answer;
      answer = QMessageBox::question(this, tr("Manual Freight?"),
                                     tr("<p>Manually editing the freight will disable "
                                          "automatic Freight recalculations.  Are you "
                                          "sure you want to do this?"),
                                     QMessageBox::Yes,
                                     QMessageBox::No | QMessageBox::Default);
      if (answer == QMessageBox::Yes)
        _calcfreight = false;
      else
      {
        disconnect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
        _freight->setLocalValue(_freightCache);
        connect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
      }
    }
    else if ( (!_calcfreight) &&
              (_freight->localValue() == 0) &&
              (_metrics->boolean("CalculateFreight")))
    {
      int answer;
      answer = QMessageBox::question(this, tr("Automatic Freight?"),
                                     tr("<p>Manually clearing the freight will enable "
                                          "automatic Freight recalculations.  Are you "
                                          "sure you want to do this?"),
                                     QMessageBox::Yes,
                                     QMessageBox::No | QMessageBox::Default);
      if (answer == QMessageBox::Yes)
      {
        _calcfreight = true;
        disconnect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
        _freight->setLocalValue(_freightCache);
        connect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
      }
    }
    else
      _freightCache = _freight->localValue();
  }

  save(true);

  sCalculateTax();
}

void salesOrder::sCalculateTax()
{
  XSqlQuery taxq;
  taxq.prepare( "SELECT SUM(tax) AS tax "
                "FROM ("
                "SELECT ROUND(SUM(taxdetail_tax),2) AS tax "
                "FROM tax "
                " JOIN calculateTaxDetailSummary(:type, :cohead_id, 'T') ON (taxdetail_tax_id=tax_id)"
                "GROUP BY tax_id) AS data;" );

  taxq.bindValue(":cohead_id", _soheadid);
  if (ISQUOTE(_mode))
    taxq.bindValue(":type","Q");
  else
    taxq.bindValue(":type","S");
  taxq.exec();
  if (taxq.first())
    _tax->setLocalValue(taxq.value("tax").toDouble());
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Tax Information"),
                                taxq, __FILE__, __LINE__))
  {
    return;
  }
  sCalculateTotal();
}

void salesOrder::sTaxZoneChanged()
{
  if (_taxZone->id() != _taxzoneidCache && _saved)
    save(true);

  sCalculateTax();
  _taxzoneidCache=_taxZone->id();
}

void salesOrder::sReserveStock()
{
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    ParameterList params;
    params.append("soitem_id", ((XTreeWidgetItem *)(selected[i]))->id());

    reserveSalesOrderItem newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }

  sFillItemList();
}

void salesOrder::sReserveLineBalance()
{
  XSqlQuery reserveSales;
  reserveSales.prepare("SELECT reserveSoLineBalance(:soitem_id) AS result;");
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    reserveSales.bindValue(":soitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
    reserveSales.exec();
    if (reserveSales.first())
    {
      int result = reserveSales.value("result").toInt();
      if (result < 0)
      {
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Sales Order Information"),
                               storedProcErrorLookup("reserveSoLineBalance", result),
                               __FILE__, __LINE__);
        return;
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Sales Order Information"),
                                  reserveSales, __FILE__, __LINE__))
    {
      return;
    }
  }

  sFillItemList();
}

void salesOrder::sUnreserveStock()
{
  XSqlQuery unreserveSales;
  unreserveSales.prepare("SELECT unreserveSoLineQty(:soitem_id) AS result;");
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    unreserveSales.bindValue(":soitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
    unreserveSales.exec();
    if (unreserveSales.first())
    {
      int result = unreserveSales.value("result").toInt();
      if (result < 0)
      {
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Sales Order Information"),
                               storedProcErrorLookup("unreservedSoLineQty", result),
                               __FILE__, __LINE__);
        return;
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Sales Order Information"),
                                  unreserveSales, __FILE__, __LINE__))
    {
      return;
    }
  }

  sFillItemList();
}

void salesOrder::sShowReservations()
{
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    ParameterList params;
    params.append("soitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
    params.append("run");

    dspReservations *newdlg = new dspReservations();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void salesOrder::sEnterCashPayment()
{
  XSqlQuery cashsave;

  if (_cashReceived->baseValue() >  _balance->baseValue() &&
      QMessageBox::question(this, tr("Overapplied?"),
                            tr("The Cash Payment is more than the Balance.  Do you want to continue?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    return;

  int _bankaccnt_curr_id = -1;
  QString _bankaccnt_currAbbr;
  cashsave.prepare( "SELECT bankaccnt_curr_id, "
                    "       currConcat(bankaccnt_curr_id) AS currAbbr "
                    "  FROM bankaccnt "
                    " WHERE (bankaccnt_id=:bankaccnt_id);");
  cashsave.bindValue(":bankaccnt_id", _bankaccnt->id());
  cashsave.exec();
  if (cashsave.first())
  {
    _bankaccnt_curr_id = cashsave.value("bankaccnt_curr_id").toInt();
    _bankaccnt_currAbbr = cashsave.value("currAbbr").toString();
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Bank Account Information"),
                                cashsave, __FILE__, __LINE__))
  {
    return;
  }

  if (_cashReceived->currencyEnabled() && _cashReceived->id() != _bankaccnt_curr_id &&
      QMessageBox::question(this, tr("Bank Currency?"),
                            tr("<p>This Sales Order is specified in %1 while the "
                               "Bank Account is specified in %2. Do you wish to "
                               "convert at the current Exchange Rate?"
                               "<p>If not, click NO "
                               "and change the Bank Account in the POST TO field.")
                            .arg(_cashReceived->currAbbr())
                            .arg(_bankaccnt_currAbbr),
                            QMessageBox::Yes|QMessageBox::Escape,
                            QMessageBox::No |QMessageBox::Default) != QMessageBox::Yes)
  {
    _bankaccnt->setFocus();
    return;
  }

  QString _cashrcptnumber;
  int _cashrcptid = -1;

  cashsave.exec("SELECT fetchCashRcptNumber() AS number, NEXTVAL('cashrcpt_cashrcpt_id_seq') AS cashrcpt_id;");
  if (cashsave.first())
  {
    _cashrcptnumber = cashsave.value("number").toString();
    _cashrcptid = cashsave.value("cashrcpt_id").toInt();
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Cash Receipt Information"),
                                cashsave, __FILE__, __LINE__))
  {
    return;
  }

  cashsave.prepare( "INSERT INTO cashrcpt "
                    "( cashrcpt_id, cashrcpt_cust_id, cashrcpt_distdate, cashrcpt_amount,"
                    "  cashrcpt_fundstype, cashrcpt_bankaccnt_id, cashrcpt_curr_id, "
                    "  cashrcpt_usecustdeposit, cashrcpt_docnumber, cashrcpt_docdate, "
                    "  cashrcpt_notes, cashrcpt_salescat_id, cashrcpt_number, cashrcpt_applydate, cashrcpt_discount ) "
                    "VALUES "
                    "( :cashrcpt_id, :cashrcpt_cust_id, :cashrcpt_distdate, :cashrcpt_amount,"
                    "  :cashrcpt_fundstype, :cashrcpt_bankaccnt_id, :cashrcpt_curr_id, "
                    "  :cashrcpt_usecustdeposit, :cashrcpt_docnumber, :cashrcpt_docdate, "
                    "  :cashrcpt_notes, :cashrcpt_salescat_id, :cashrcpt_number, :cashrcpt_applydate, :cashrcpt_discount );" );
  cashsave.bindValue(":cashrcpt_id", _cashrcptid);
  cashsave.bindValue(":cashrcpt_number", _cashrcptnumber);
  cashsave.bindValue(":cashrcpt_cust_id", _cust->id());
  cashsave.bindValue(":cashrcpt_amount", _cashReceived->localValue());
  cashsave.bindValue(":cashrcpt_fundstype", _fundsType->code());
  cashsave.bindValue(":cashrcpt_docnumber", _docNumber->text());
  cashsave.bindValue(":cashrcpt_docdate", _docDate->date());
  cashsave.bindValue(":cashrcpt_bankaccnt_id", _bankaccnt->id());
  cashsave.bindValue(":cashrcpt_distdate", _distDate->date());
  cashsave.bindValue(":cashrcpt_applydate", _applDate->date());
  cashsave.bindValue(":cashrcpt_notes", "Sales Order Cash Payment");
  cashsave.bindValue(":cashrcpt_usecustdeposit", _metrics->boolean("EnableCustomerDeposits"));
  cashsave.bindValue(":cashrcpt_discount", 0.0);
  cashsave.bindValue(":cashrcpt_curr_id", _cashReceived->id());
  if(_altAccnt->isChecked())
    cashsave.bindValue(":cashrcpt_salescat_id", _salescat->id());
  else
    cashsave.bindValue(":cashrcpt_salescat_id", -1);
  cashsave.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Updating Cash Receipt Information"),
                                cashsave, __FILE__, __LINE__))
  {
    return;
  }

  // Post the Cash Receipt
  XSqlQuery cashPost;
  int journalNumber = -1;

  cashPost.exec("SELECT fetchJournalNumber('C/R') AS journalnumber;");
  if (cashPost.first())
    journalNumber = cashPost.value("journalnumber").toInt();
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Cash Receipt Information"),
                                cashPost, __FILE__, __LINE__))
  {
    return;
  }

  cashPost.prepare("SELECT postCashReceipt(:cashrcpt_id, :journalNumber) AS result;");
  cashPost.bindValue(":cashrcpt_id", _cashrcptid);
  cashPost.bindValue(":journalNumber", journalNumber);
  cashPost.exec();
  if (cashPost.first())
  {
    int result = cashPost.value("result").toInt();
    if (result < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Cash Receipt Information"),
                             storedProcErrorLookup("postCashReceipt", result),
                             __FILE__, __LINE__);
      return;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Cash Receipt Information"),
                                cashPost, __FILE__, __LINE__))
  {
    return;
  }

  // Find the Customer Deposit C/M and Allocate
  cashPost.prepare("SELECT cashrcptitem_aropen_id FROM cashrcptitem WHERE cashrcptitem_cashrcpt_id=:cashrcpt_id;");
  cashPost.bindValue(":cashrcpt_id", _cashrcptid);
  cashPost.exec();
  if (cashPost.first())
  {
    int aropenid = cashPost.value("cashrcptitem_aropen_id").toInt();
    cashPost.prepare("INSERT INTO aropenalloc"
                     "      (aropenalloc_aropen_id, aropenalloc_doctype, aropenalloc_doc_id, "
                     "       aropenalloc_amount, aropenalloc_curr_id)"
                     "VALUES(:aropen_id, 'S', :doc_id, :amount, :curr_id);");
    cashPost.bindValue(":doc_id", _soheadid);
    cashPost.bindValue(":aropen_id", aropenid);
    if (_cashReceived->localValue() >  _balance->localValue())
    {
      cashPost.bindValue(":amount", _balance->localValue());
      cashPost.bindValue(":curr_id", _balance->id());
    }
    else
    {
      cashPost.bindValue(":amount", _cashReceived->localValue());
      cashPost.bindValue(":curr_id", _cashReceived->id());
    }
    cashPost.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Updating Cash Receipt Information"),
                                  cashPost, __FILE__, __LINE__))
    {
      return;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Updating Cash Receipt Information"),
                                cashPost, __FILE__, __LINE__))
  {
    return;
  }

  _cashReceived->clear();
  populateCMInfo();
}

void salesOrder::sCreditAllocate()
{
    ParameterList params;
    params.append("doctype", "S");
    params.append("cohead_id", _soheadid);
    params.append("cust_id", _cust->id());
    params.append("total",  _total->localValue());
    params.append("balance",  _balance->localValue());
    params.append("curr_id",   _balance->id());
    params.append("effective", _balance->effective());

    allocateARCreditMemo newdlg(this, "", true);
    if (newdlg.set(params) == NoError && newdlg.exec() == XDialog::Accepted)
    {
        populateCMInfo();
    }
}

void salesOrder::sAllocateCreditMemos()
{
  XSqlQuery allocateSales;
  // Determine the balance I need to select
  // This is the same as in sCalculateTotal except that the Unallocated amount is not included.
  double  balance      = (_subtotal->localValue() + _tax->localValue() + _miscCharge->localValue() + _freight->localValue())-
                         _allocatedCM->localValue() - _authCC->localValue();
  double  initBalance  = balance;
  if (balance > 0)
  {
    // Get the list of Unallocated CM's with amount
    allocateSales.prepare("SELECT aropen_id,"
              "       noNeg(aropen_amount - aropen_paid - SUM(COALESCE(aropenalloc_amount,0))) AS amount,"
              "       currToCurr(aropen_curr_id, :curr_id,"
              "                  noNeg(aropen_amount - aropen_paid - SUM(COALESCE(aropenalloc_amount,0))), :effective) AS amount_cocurr"
              "  FROM cohead, aropen LEFT OUTER JOIN aropenalloc ON (aropenalloc_aropen_id=aropen_id)"
              " WHERE ( (aropen_cust_id=cohead_cust_id)"
              "   AND   (aropen_doctype IN ('C', 'R'))"
              "   AND   (aropen_open)"
              "   AND   (cohead_id=:cohead_id) )"
              " GROUP BY aropen_id, aropen_duedate, aropen_amount, aropen_paid, aropen_curr_id "
              "HAVING (noNeg(aropen_amount - aropen_paid - SUM(COALESCE(aropenalloc_amount,0))) > 0)"
              " ORDER BY aropen_duedate; ");
    allocateSales.bindValue(":cohead_id", _soheadid);
    allocateSales.bindValue(":curr_id",   _balance->id());
    allocateSales.bindValue(":effective", _balance->effective());
    allocateSales.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Accounts Receivable Information"),
                                  allocateSales, __FILE__, __LINE__))
    {
      return;
    }

    double    amount     = 0.0;
    double    initAmount = 0.0;
    XSqlQuery allocCM;
    allocCM.prepare("INSERT INTO aropenalloc"
                    "      (aropenalloc_aropen_id, aropenalloc_doctype, aropenalloc_doc_id, "
                    "       aropenalloc_amount, aropenalloc_curr_id)"
                    "VALUES(:aropen_id, 'S', :doc_id, :amount, :curr_id);");

    while (balance > 0.0 && allocateSales.next())
    {
      amount     = allocateSales.value("amount").toDouble();
      initAmount = _outstandingCM->localValue();

      if (amount <= 0.0)  // if this credit memo does not have a positive value just ignore it
        continue;

      if (amount > balance) // make sure we don't apply more to a credit memo than we have left.
        amount = balance;
      // apply credit memo's to this sales order until the balance is 0.
      allocCM.bindValue(":doc_id", _soheadid);
      allocCM.bindValue(":aropen_id", allocateSales.value("aropen_id").toInt());
      allocCM.bindValue(":amount", amount);
      allocCM.bindValue(":curr_id", _balance->id());
      allocCM.exec();
      if (allocCM.lastError().type() == QSqlError::NoError)
        balance -= amount;
      else
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Updating Accounts Receivable Information"),
                           allocCM, __FILE__, __LINE__);
    }
    _outstandingCM->setLocalValue(initAmount-(initBalance-balance));
    _balance->setLocalValue(initBalance-(initBalance-balance));
    _allocatedCM->setLocalValue(initBalance-balance);
  }
}

void salesOrder::sCheckValidContacts()
{
  if (_shipToCntct->isValid())
    _shipToCntct->setEnabled(true);
  else
    _shipToCntct->setEnabled(false);

  if (_billToCntct->isValid())
    _billToCntct->setEnabled(true);
  else
    _billToCntct->setEnabled(false);
}

void salesOrder::sHandleMore()
{
  _warehouse->setVisible(_more->isChecked());
  _shippingWhseLit->setVisible(_more->isChecked());
  _commissionLit->setVisible(_more->isChecked());
  _commission->setVisible(_more->isChecked());
  _commissionPrcntLit->setVisible(_more->isChecked());
  _taxZoneLit->setVisible(_more->isChecked());
  _taxZone->setVisible(_more->isChecked());
  _shipDateLit->setVisible(_more->isChecked());
  _shipDate->setVisible(_more->isChecked());
  _packDateLit->setVisible(_more->isChecked());
  _packDate->setVisible(_more->isChecked());
  _saleTypeLit->setVisible(_more->isChecked());
  _saleType->setVisible(_more->isChecked());

  if (ISORDER(_mode))
  {
    _shippingCharges->setVisible(_more->isChecked());
    _shippingChargesLit->setVisible(_more->isChecked());
    _shippingForm->setVisible(_more->isChecked());
    _shippingFormLit->setVisible(_more->isChecked());
  }
  else
  {
    _expireLit->setVisible(_more->isChecked());
    _expire->setVisible(_more->isChecked());
  }
}

void salesOrder::sRecalculatePrice()
{
  if (QMessageBox::question(this, tr("Update all prices?"),
                            tr("Do you want to recalculate all prices for the order including:\n\t- Line items\n\t - Taxes\n\t - Freight ?"),
                            QMessageBox::Yes | QMessageBox::Escape,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    ParameterList params;
    QString       sql;
    QString       sqlchk;
    if (ISORDER(_mode))
    {
      sql ="UPDATE coitem"
           "  SET coitem_price=roundSale(itemPrice(item_id, cohead_cust_id,"
           "                                       <? value('shipto_id') ?>, coitem_qtyord,"
           "                                       coitem_qty_uom_id, coitem_price_uom_id,"
           "                                       cohead_curr_id, cohead_orderdate,"
           "                                       <? if exists('UseSchedDate') ?> coitem_scheddate"
           "                                       <? else ?> <? value('asOf') ?>"
           "                                       <? endif ?>"
           "                                      )),"
           "      coitem_custprice=roundSale(itemPrice(item_id, cohead_cust_id,"
           "                                           <? value('shipto_id') ?>, coitem_qtyord,"
           "                                           coitem_qty_uom_id, coitem_price_uom_id,"
           "                                           cohead_curr_id, cohead_orderdate,"
           "                                           <? if exists('UseSchedDate') ?> coitem_scheddate"
           "                                           <? else ?> <? value('asOf') ?>"
           "                                           <? endif ?>"
           "                                          )) "
           "FROM cohead, item, itemsite "
           "WHERE ( (coitem_status NOT IN ('C','X'))"
           "  AND   (coitem_subnumber=0)"
           "  AND   (NOT coitem_firm)"
           "<? if exists('ignoreDiscounts') ?>"
           "  AND   (coitem_price = coitem_custprice)"
           "<? endif ?>"
           "  AND   (itemsite_id=coitem_itemsite_id)"
           "  AND   (itemsite_item_id=item_id)"
           "  AND   (coitem_cohead_id=cohead_id)"
           "  AND   (cohead_id=<? value('cohead_id') ?>) );";
      sqlchk ="SELECT MIN(itemPrice(item_id, cohead_cust_id,"
              "                     <? value('shipto_id') ?>, coitem_qtyord,"
              "                     coitem_qty_uom_id, coitem_price_uom_id,"
              "                     cohead_curr_id, cohead_orderdate,"
              "                     <? if exists('UseSchedDate') ?> coitem_scheddate"
              "                     <? else ?> <? value('asOf') ?>"
              "                     <? endif ?>)) AS pricechk "
              "FROM cohead, coitem, item, itemsite "
              "WHERE ( (coitem_cohead_id=cohead_id) "
              "  AND   (coitem_status NOT IN ('C','X')) "
              "  AND   (coitem_subnumber=0) "
              "  AND   (NOT coitem_firm) "
              "<? if exists('ignoreDiscounts') ?>"
              "  AND   (coitem_price = coitem_custprice) "
              "<? endif ?>"
              "  AND   (itemsite_id=coitem_itemsite_id) "
              "  AND   (itemsite_item_id=item_id) "
              "  AND   (coitem_cohead_id=cohead_id) "
              "  AND   (cohead_id=<? value('cohead_id') ?>) );";
    }
    else
    {
      sql ="UPDATE quitem"
           "  SET quitem_price=roundSale(itemPrice(item_id, quhead_cust_id,"
           "                                       <? value('shipto_id') ?>, quitem_qtyord,"
           "                                       quitem_qty_uom_id, quitem_price_uom_id,"
           "                                       quhead_curr_id, quhead_quotedate,"
           "                                       <? if exists('UseSchedDate') ?> quitem_scheddate"
           "                                       <? else ?> <? value('asOf') ?>"
           "                                       <? endif ?>"
           "                                      )),"
           "      quitem_custprice=roundSale(itemPrice(item_id, quhead_cust_id,"
           "                                           <? value('shipto_id') ?>, quitem_qtyord,"
           "                                           quitem_qty_uom_id, quitem_price_uom_id,"
           "                                           quhead_curr_id, quhead_quotedate,"
           "                                           <? if exists('UseSchedDate') ?> quitem_scheddate"
           "                                           <? else ?> <? value('asOf') ?>"
           "                                           <? endif ?>"
           "                                          )) "
           "FROM quhead, item, itemsite "
           "WHERE ( (itemsite_id=quitem_itemsite_id)"
           "<? if exists('ignoreDiscounts') ?>"
           "  AND   (quitem_price = quitem_custprice)"
           "<? endif ?>"
           "  AND   (itemsite_item_id=item_id)"
           "  AND   (quitem_quhead_id=quhead_id)"
           "  AND   (quhead_id=<? value('cohead_id') ?>) );";
      sqlchk ="SELECT MIN(itemprice(item_id, quhead_cust_id, "
              "                     <? value('shipto_id') ?>, quitem_qtyord, "
              "                     quitem_qty_uom_id, quitem_price_uom_id, "
              "                     quhead_curr_id,quhead_quotedate, "
              "                     <? if exists('UseSchedDate') ?> quitem_scheddate "
              "                     <? else ?> <? value('asOf') ?>"
              "                     <? endif ?>)) AS pricechk "
              "FROM quhead, quitem, item, itemsite "
              "WHERE ( (quitem_quhead_id=quhead_id) "
              "  AND   (itemsite_id=quitem_itemsite_id) "
              "<? if exists('ignoreDiscounts') ?>"
              "  AND   (quitem_price = quitem_custprice) "
              "<? endif ?>"
              "  AND   (itemsite_item_id=item_id) "
              "  AND   (quitem_quhead_id=quhead_id) "
              "  AND   (quhead_id=<? value('cohead_id') ?>) );";
    }
    params.append("cohead_id", _soheadid);
    params.append("shipto_id", _shipTo->id());
    if (_metrics->boolean("IgnoreCustDisc"))
      params.append("ignoreDiscounts", true);
    if (_metrics->value("soPriceEffective") == "ScheduleDate")
      params.append("UseSchedDate", true);
    MetaSQLQuery mql(sql);
    MetaSQLQuery mqlchk(sqlchk);
    if (_metrics->value("soPriceEffective") == "OrderDate")
    {
      if (!_orderDate->isValid())
      {
        QMessageBox::critical(this,tr("Order Date Required"),tr("Prices can not be recalculated without a valid Order Date."));
        _orderDate->setFocus();
        return;
      }
      params.append("asOf", _orderDate->date());
    }
    else if (_metrics->value("soPriceEffective") == "ScheduleDate")
    {
      if (!_orderDate->isValid())
      {
        QMessageBox::critical(this,tr("Schedule Date Required"),tr("Prices can not be recalculated without a valid Schedule Date."));
        _shipDate->setFocus();
        return;
      }
      params.append("asOf", _shipDate->date());
    }
    else
      params.append("asOf", omfgThis->dbDate());

    XSqlQuery itempricechk = mqlchk.toQuery(params);
    if (itempricechk.first())
    {
      if (itempricechk.value("pricechk").toDouble() == -9999.0)
      {
        // User expected an update, so let them know and reset
        QMessageBox::critical(this, tr("Customer Cannot Buy at Quantity"),
                              tr("<p>One or more items are marked as exclusive and "
                                   "no qualifying price schedule was found. " ) );
        return;
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Item Information"),
                                  itempricechk, __FILE__, __LINE__))
    {
      return;
    }

    XSqlQuery setitemprice = mql.toQuery(params);
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Item Information"),
                                  setitemprice, __FILE__, __LINE__))
    {
      return;
    }

    _calcfreight = _metrics->boolean("CalculateFreight");
    sFillItemList();
  }
}

void salesOrder::sOrderDateChanged()
{
  if (_orderDate->date() == _orderDateCache ||
      !_orderDate->isValid())
    return;
  else if (!_soitem->topLevelItemCount() ||
           !_orderDateCache.isValid() ||
           _metrics->value("soPriceEffective") != "OrderDate")
  {
    _orderDateCache = _orderDate->date();
    return;
  }

  sRecalculatePrice();
  _orderDateCache = _orderDate->date();
  omfgThis->sSalesOrdersUpdated(_soheadid);
}

void salesOrder::sShipDateChanged()
{
  XSqlQuery salesShipDateChanged;
  if (_shipDate->date() == _shipDateCache ||
      !_shipDate->isValid())
    return;
  else if (!_soitem->topLevelItemCount())
  {
    _shipDateCache = _shipDate->date();
    return;
  }

  QString       sql;
  XSqlQuery     upd;
  ParameterList params;
  params.append("cohead_id", _soheadid);
  params.append("newDate", _shipDate->date());
  params.append("ignoreCustDisc", _metrics->value("IgnoreCustDisc"));

  if (QMessageBox::question(this, tr("Update all schedule dates?"),
                                  tr("Changing this date will update the Schedule Date on all editable line items. "
                               "Is this what you want to do?"),
                            QMessageBox::Yes | QMessageBox::Default,
                            QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
  {
    // Validate first
    if (ISORDER(_mode))
    {
      sql = "SELECT DISTINCT valid FROM ( "
            "  SELECT customerCanPurchase(itemsite_item_id, cohead_cust_id, "
            "                             cohead_shipto_id, <? value('newDate') ?>) AS valid "
            "  FROM cohead "
            "   JOIN coitem ON (cohead_id=coitem_cohead_id) "
            "   JOIN itemsite ON (coitem_itemsite_id=itemsite_id) "
            "   WHERE ( (cohead_id=<? value('cohead_id') ?>) "
            "   AND (coitem_status NOT IN ('C','X')) "
            "   AND (coitem_subnumber = 0))"
            ") data "
            "ORDER BY valid; ";
    }
    else
    {
      sql = "SELECT DISTINCT valid FROM ( "
            "  SELECT customerCanPurchase(itemsite_item_id, quhead_cust_id, "
            "                             quhead_shipto_id, <? value('newDate') ?>) AS valid "
            "  FROM quhead "
            "   JOIN quitem ON (quhead_id=quitem_quhead_id) "
            "   JOIN itemsite ON (quitem_itemsite_id=itemsite_id) "
            "   WHERE (quhead_id=<? value('cohead_id') ?>) "
            ") data "
            "ORDER BY valid; ";
    }

    MetaSQLQuery vmql(sql);
    upd = vmql.toQuery(params);
    if (upd.first())
    {
      if (upd.size() == 2)  // Both valid and invalid records
      {
        if (QMessageBox::warning(this, tr("Can not reschedule all Items"),
                                  tr("Some exclusive items may not be rescheduled because there is no "
                                    "valid price schedule for the date entered.  Proceed rescheduling "
                                    "only qualifying items?"),
                                 QMessageBox::Yes | QMessageBox::Default,
                                 QMessageBox::No | QMessageBox::Escape) == QMessageBox::No)
        {
          _shipDate->setDate(_shipDateCache);
          return;
        }
      }
      else if (!upd.value("valid").toBool())  // No valid items
      {
        QMessageBox::warning(this, tr("Can not reschedule Items"),
                                  tr("No Items can be rescheduled because there are no "
                                "valid price schedules for the date entered."));
        _shipDate->setDate(_shipDateCache);
        return;
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Item Information"),
                                  upd, __FILE__, __LINE__))
    {
      _shipDate->setDate(_shipDateCache);
      return;
    }

    // Now execute
    if (ISORDER(_mode))
    {
      sql = "UPDATE coitem SET coitem_scheddate = <? value('newDate') ?> "
            "FROM cohead,item,itemsite "
            "WHERE ( (coitem_status NOT IN ('C','X'))"
            "  AND (NOT coitem_firm)"
            "  AND (itemsite_id=coitem_itemsite_id) "
            "  AND (itemsite_item_id=item_id) "
            "  AND (cohead_id=<? value('cohead_id') ?>) "
            "  AND (coitem_cohead_id=cohead_id) "
            "  AND (customerCanPurchase(itemsite_item_id, cohead_cust_id, cohead_shipto_id, <? value('newDate') ?>) ) );";

      // Ask about work orders if applicable
      XSqlQuery wo;
      QString woSql("SELECT wo_id "
                 "FROM wo "
                 "  JOIN coitem ON (coitem_order_id=wo_id) AND (coitem_order_type='W') "
                 "  JOIN cohead ON (cohead_id=coitem_cohead_id) "
                 "  JOIN itemsite ON (coitem_itemsite_id=itemsite_id) "
                 "WHERE ((cohead_id=<? value('cohead_id') ?>) "
                 "  AND (coitem_status NOT IN ('C','X')) "
                 "  AND (NOT coitem_firm)"
                 "  AND (wo_status<>'C')"
                 "  AND (customerCanPurchase(itemsite_item_id, cohead_cust_id, cohead_shipto_id, <? value('newDate') ?>) ) );");
      MetaSQLQuery woMql(woSql);
      wo = woMql.toQuery(params);
      if(wo.first())
      {
        if (QMessageBox::question(this, tr("Reschedule Work Order?"),
                                tr("<p>Should any associated work orders "
                                   "be rescheduled to reflect this change?"),
                                QMessageBox::Yes | QMessageBox::Default,
                                QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
        {
          sql = sql +
                "SELECT changeWoDates(wo_id, "
                "                     wo_startdate + (<? value('newDate') ?> - wo_duedate),"
                "                     <? value('newDate') ?>, true) AS result "
                "FROM cohead JOIN coitem ON (coitem_cohead_id=cohead_id AND coitem_order_type='W') "
                "            JOIN wo ON (wo_id=coitem_order_id) "
                "            JOIN itemsite ON (itemsite_id=coitem_itemsite_id) "
                "WHERE ( (coitem_status NOT IN ('C','X'))"
                "  AND (NOT coitem_firm)"
                "  AND (wo_status <> 'C') "
                "  AND (cohead_id=<? value('cohead_id') ?>)"
                "  AND (customerCanPurchase(itemsite_item_id, cohead_cust_id, cohead_shipto_id, <? value('newDate') ?>) ) )";
        }
      }

      // Ask about purchase orders if applicable
      XSqlQuery po;
      QString poSql("SELECT poitem_id "
                    "FROM poitem "
                    "  JOIN coitem ON (coitem_order_id=poitem_id) AND (coitem_order_type='P') "
                    "  JOIN cohead ON (cohead_id=coitem_cohead_id) "
                    "  JOIN itemsite ON (coitem_itemsite_id=itemsite_id) "
                    "WHERE ((cohead_id=<? value('cohead_id') ?>) "
                    "  AND (coitem_status NOT IN ('C','X')) "
                    "  AND (NOT coitem_firm)"
                    "  AND (poitem_status<>'C')"
                    "  AND (customerCanPurchase(itemsite_item_id, cohead_cust_id, cohead_shipto_id, <? value('newDate') ?>) ) );");
      MetaSQLQuery poMql(poSql);
      po = poMql.toQuery(params);
      if(po.first())
      {
        if (QMessageBox::question(this, tr("Reschedule Purchase Order?"),
                                  tr("<p>Should any associated Purchase Orders "
                                     "be rescheduled to reflect this change?"),
                                  QMessageBox::Yes | QMessageBox::Default,
                                  QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
        {
          sql = sql +
          "SELECT changePoitemDueDate(poitem_id, "
          "                     <? value('newDate') ?>, true) AS result "
          "FROM cohead JOIN coitem ON (coitem_cohead_id=cohead_id AND coitem_order_type='P') "
          "            JOIN poitem ON (poitem_id=coitem_order_id) "
          "            JOIN itemsite ON (itemsite_id=coitem_itemsite_id) "
          "WHERE ( (coitem_status NOT IN ('C','X'))"
          "  AND (NOT coitem_firm)"
          "  AND (poitem_status <> 'C') "
          "  AND (cohead_id=<? value('cohead_id') ?>)"
          "  AND (customerCanPurchase(itemsite_item_id, cohead_cust_id, cohead_shipto_id, <? value('newDate') ?>) ) )";
        }
      }
    }
    else
    {
      sql = "UPDATE quitem SET quitem_scheddate = <? value('newDate') ?> "
            "FROM quhead,item,itemsite "
            "WHERE ( (itemsite_id=quitem_itemsite_id) "
            "  AND (itemsite_item_id=item_id) "
            "  AND (quhead_id=<? value('cohead_id') ?>) "
            "  AND (quitem_quhead_id=quhead_id) "
            "  AND (customerCanPurchase(itemsite_item_id, quhead_cust_id, quhead_shipto_id, <? value('newDate') ?>) ) );";
    }

    MetaSQLQuery mql(sql);
    upd = mql.toQuery(params);
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Updating Item Information"),
                                  upd, __FILE__, __LINE__))
    {
      _shipDate->setDate(_shipDateCache);
      return;
    }
  }
  else
  {
    _shipDate->setDate(_shipDateCache);
    return;
  }

  if (_metrics->value("soPriceEffective") == "ScheduleDate")
    sRecalculatePrice();
  _shipDateCache = _shipDate->date();
  omfgThis->sSalesOrdersUpdated(_soheadid);
}

void salesOrder::sViewWO()
{
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XSqlQuery wo;
    wo.prepare("SELECT coitem_order_id FROM coitem WHERE (coitem_id=:soitem_id);");
    wo.bindValue(":soitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
    wo.exec();
    if (wo.first())
    {
      ParameterList params;
      params.append("wo_id",  wo.value("coitem_order_id").toInt());
      params.append("mode", "view");

      workOrder *newdlg = new workOrder();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Item Information"),
                                  wo, __FILE__, __LINE__))
    {
      return;
    }
  }
}

void salesOrder::sMaintainWO()
{
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XSqlQuery wo;
    wo.prepare("SELECT coitem_order_id FROM coitem WHERE (coitem_id=:soitem_id);");
    wo.bindValue(":soitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
    wo.exec();
    if (wo.first())
    {
      ParameterList params;
      params.append("wo_id",  wo.value("coitem_order_id").toInt());
      params.append("mode", "edit");

      workOrder *newdlg = new workOrder();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Item Information"),
                                  wo, __FILE__, __LINE__))
    {
      return;
    }
  }
}

void salesOrder::sViewPO()
{
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XSqlQuery po;
    po.prepare("SELECT poitem_pohead_id "
               "FROM coitem JOIN poitem ON (coitem_order_id = poitem_id) "
               "WHERE (coitem_id=:soitem_id);");
    po.bindValue(":soitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
    po.exec();
    if (po.first())
    {
      ParameterList params;
      params.append("pohead_id", po.value("poitem_pohead_id").toInt());
      params.append("mode", "view");

      purchaseOrder *newdlg = new purchaseOrder();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Item Information"),
                                  po, __FILE__, __LINE__))
    {
      return;
    }
  }
}

void salesOrder::sMaintainPO()
{
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XSqlQuery po;
    po.prepare("SELECT poitem_pohead_id "
               "FROM coitem JOIN poitem ON (coitem_order_id = poitem_id) "
               "WHERE (coitem_id=:soitem_id);");
    po.bindValue(":soitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
    po.exec();
    if (po.first())
    {
      ParameterList params;
      params.append("pohead_id", po.value("poitem_pohead_id").toInt());
      params.append("mode", "edit");

      purchaseOrder *newdlg = new purchaseOrder();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Item Information"),
                                  po, __FILE__, __LINE__))
    {
      return;
    }
  }
}

void salesOrder::sReleasePR()
{
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XSqlQuery pr;
    pr.prepare("SELECT coitem_order_id FROM coitem WHERE (coitem_id=:soitem_id);");
    pr.bindValue(":soitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
    pr.exec();
    if (pr.first())
    {
      ParameterList params;
      params.append("mode", "releasePr");
      params.append("pr_id", pr.value("coitem_order_id").toInt());
      purchaseOrder *newdlg = new purchaseOrder();
      if (newdlg->set(params) == NoError)
        omfgThis->handleNewWindow(newdlg);
      else
        delete newdlg;
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Item Information"),
                                  pr, __FILE__, __LINE__))
    {
      return;
    }
  }
}

void salesOrder::sViewPR()
{
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XSqlQuery pr;
    pr.prepare("SELECT pr_itemsite_id, pr_qtyreq, pr_duedate, pr_order_id, coitem_order_id "
               "FROM pr JOIN coitem ON (pr_id = coitem_order_id) "
               "WHERE (coitem_id=:soitem_id);");
    pr.bindValue(":soitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
    pr.exec();
    if (pr.first())
    {
      ParameterList params;
      params.append("mode", "view");
      params.append("pr_id", pr.value("coitem_order_id").toInt());
      purchaseRequest *newdlg = new purchaseRequest();
      if (newdlg->set(params) == NoError)
        omfgThis->handleNewWindow(newdlg);
      else
        delete newdlg;
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Item Information"),
                                  pr, __FILE__, __LINE__))
    {
      return;
    }
  }
}

void salesOrder::sPopulateShipments()
{
  _dspShipmentsBySalesOrder->findChild<OrderCluster*>("_salesOrder")->setId(_soheadid);
  _dspShipmentsBySalesOrder->sFillList();
  _dspShipmentsBySalesOrder->list()->expandAll();
}

void salesOrder::sViewItemWorkbench()
{
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XSqlQuery item;
    item.prepare("SELECT itemsite_item_id FROM coitem join itemsite on itemsite_id=coitem_itemsite_id WHERE (coitem_id=:soitem_id);");
    item.bindValue(":soitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
    item.exec();
    if (item.first())
    {
      ParameterList params;
      params.append("item_id",  item.value("itemsite_item_id").toInt());
      itemAvailabilityWorkbench *newdlg = new itemAvailabilityWorkbench();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Item Information"),
                                  item, __FILE__, __LINE__))
    {
      return;
    }
  }
}

bool salesOrder::creditLimitCheck()
{
  XSqlQuery creditCheck;
  double    customerCurrent;

  if (_holdOverride)
    return true;

  creditCheck.prepare("SELECT * FROM creditlimitcheck(:cust_id);");
  creditCheck.bindValue(":cust_id", _cust->id());
  creditCheck.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Sales Order Credit Check"),
                           creditCheck, __FILE__, __LINE__))
    return false;

  if (creditCheck.first())
  {
    customerCurrent = creditCheck.value("creditcheck_bookings").toDouble() +
                      creditCheck.value("creditcheck_aropen").toDouble();

    // The Credit Check
    if (customerCurrent <= creditCheck.value("creditcheck_limit").toDouble())
      return true;
    else
      return false;
  }

  return false;
}

bool salesOrder::creditLimitCheckIssue()
{
  if (_metrics->boolean("CreditCheckSOOnSave") && !creditLimitCheck() && _holdType->code() != "C")
  {
        QMessageBox::warning(this, tr("Sales Order Credit Check"),
                             tr("<p>The customer has exceeded their credit limit "
                                "and this order will be placed on Credit Hold."));
        _holdType->setCode("C");
  }

  save(true);

  return true;
}

void salesOrder::sHoldTypeChanged()
{
  if (ISEDIT(_mode) && _holdTypeCache == "C" && _holdType->code() != "C")
    _holdOverride = true;
}
