/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "salesOrderSimple.h"
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

#include "creditCard.h"
#include "creditcardprocessor.h"
#include "crmacctcluster.h"
#include "customer.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "distributeInventory.h"
#include "issueLineToShipping.h"
#include "mqlutil.h"
#include "storedProcErrorLookup.h"
#include "taxBreakdown.h"
#include "printPackingList.h"
#include "printSoForm.h"

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

const struct {
    const char * full;
    QString abbr;
    bool    cc;
} _fundsTypes[] = {
    { QT_TRANSLATE_NOOP("cashReceipt", "Cash"),             "K", false },
    { QT_TRANSLATE_NOOP("cashReceipt", "Check"),            "C", false },
    { QT_TRANSLATE_NOOP("cashReceipt", "Certified Check"),  "T", false },
    { QT_TRANSLATE_NOOP("cashReceipt", "Wire Transfer"),    "W", false },
    { QT_TRANSLATE_NOOP("cashReceipt", "Other"),            "O", false }
};

salesOrderSimple::salesOrderSimple(QWidget *parent, const char *name, Qt::WindowFlags fl)
  : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_authorize,           SIGNAL(clicked()),                              this,         SLOT(sAuthorizeCC()));
  connect(_charge,              SIGNAL(clicked()),                              this,         SLOT(sChargeCC()));
  connect(_postCash,            SIGNAL(clicked()),                              this,         SLOT(sEnterCashPayment()));
  connect(_cust,                SIGNAL(newId(int)),                             this,         SLOT(sPopulateCustomerInfo(int)));
  connect(_upCC,                SIGNAL(clicked()),                              this,         SLOT(sMoveUp()));
  connect(_downCC,              SIGNAL(clicked()),                              this,         SLOT(sMoveDown()));
  connect(_editCC,              SIGNAL(clicked()),                              this,         SLOT(sEditCreditCard()));
  connect(_newCC,               SIGNAL(clicked()),                              this,         SLOT(sNewCreditCard()));
  connect(_viewCC,              SIGNAL(clicked()),                              this,         SLOT(sViewCreditCard()));
  
  connect(_orderNumber,         SIGNAL(editingFinished()),                      this,         SLOT(sHandleOrderNumber()));
  connect(_orderNumber,         SIGNAL(textChanged(const QString &)),           this,         SLOT(sSetUserEnteredOrderNumber()));
  connect(_save,                SIGNAL(clicked()),                              this,         SLOT(sSave()));
  connect(_soitem,              SIGNAL(populateMenu(QMenu*,QTreeWidgetItem *)), this,         SLOT(sPopulateMenu(QMenu *)));
  connect(_soitem,              SIGNAL(itemSelectionChanged()),                 this,         SLOT(sHandleButtons()));
  connect(_subtotal,            SIGNAL(valueChanged()),                         this,         SLOT(sCalculateTotal()));

  connect(_qty,                 SIGNAL(editingFinished()),                      this,         SLOT(sSaveLine()));
  
  _lineMode = cNew;
  _saved = false;

  _soheadid          = -1;
  _orderNumberGen    = 0;

  _numSelected       = 0;

  _captive       = false;

  _ignoreSignals = true;

  _orderNumber->setValidator(omfgThis->orderVal());
  _CCCVV->setValidator(new QIntValidator(100, 9999, this));
  _authCC = 0.0;

  _applDate->setDate(omfgThis->dbDate(), true);
  _distDate->setDate(omfgThis->dbDate(), true);

  _soitem->addColumn(tr("Item"),            _itemColumn,           Qt::AlignLeft,   true,  "item_number");
  _soitem->addColumn(tr("Description"),     -1,                    Qt::AlignLeft,   true,  "description");
  _soitem->addColumn(tr("Qty."),            _qtyColumn,            Qt::AlignRight,  true,  "coitem_qtyord");
  _soitem->addColumn(tr("Qty. UOM"),        _qtyColumn,            Qt::AlignCenter, false, "qty_uom");
  _soitem->addColumn(tr("Price"),           _priceColumn,          Qt::AlignRight,  true,  "coitem_price");
  _soitem->addColumn(tr("Price UOM"),       _priceColumn,          Qt::AlignCenter, false, "price_uom");
  _soitem->addColumn(tr("Ext. Price"),      _bigMoneyColumn,       Qt::AlignRight,  true,  "extprice");

  _cc->addColumn(tr("Sequence"),_itemColumn, Qt::AlignLeft, true, "ccard_seq");
  _cc->addColumn(tr("Type"),    _itemColumn, Qt::AlignLeft, true, "type");
  _cc->addColumn(tr("Number"),  _itemColumn, Qt::AlignRight,true, "f_number");
  _cc->addColumn(tr("Active"),  _itemColumn, Qt::AlignLeft, true, "ccard_active");
  _cc->addColumn(tr("Name"),    _itemColumn, Qt::AlignLeft, true, "ccard_name");
  _cc->addColumn(tr("Expiration Date"),  -1, Qt::AlignLeft, true, "expiration");

  _ignoreSignals = false;

  if (!_metrics->boolean("CCAccept") || !_privileges->check("ProcessCreditCards"))
  {
    _paymentInformation->removeTab(_paymentInformation->indexOf(_creditCardPage));
  }

  for (unsigned int i = 0; i < sizeof(_fundsTypes) / sizeof(_fundsTypes[1]); i++)
  {
    _fundsType->append(i, tr(_fundsTypes[i].full), _fundsTypes[i].abbr);
  }
    
  _bankaccnt->setType(XComboBox::ARBankAccounts);
  _salescat->setType(XComboBox::SalesCategoriesActive);
}

salesOrderSimple::~salesOrderSimple()
{
  // no need to delete child widgets, Qt does it all for us
}

void salesOrderSimple::languageChange()
{
  retranslateUi(this);
}

enum SetResponse salesOrderSimple:: set(const ParameterList &pParams)
{
  XSqlQuery setSales;
  XWidget::set(pParams);
  QVariant  param;
  bool      valid;

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      setObjectName("salesOrderSimple new");
      _mode = cNew;
      emit newModeState(1);

      _cust->setType(CLineEdit::ActiveCustomers);

      connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sHandleSalesOrderEvent(int, bool)));
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      emit newModeState(2);

      _cust->setType(CLineEdit::AllCustomers);

      connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sHandleSalesOrderEvent(int, bool)));
    }
    else if (param.toString() == "view")
    {
      setViewMode();
      _cust->setType(CLineEdit::AllCustomers);

    }
  }

  if (ISNEW(_mode))
  {
    _ignoreSignals = true;

    populateOrderNumber();
    if (_orderNumber->text().isEmpty())
      _orderNumber->setFocus();
    else
      _cust->setFocus();

    _ignoreSignals = false;

    setSales.exec("SELECT NEXTVAL('cohead_cohead_id_seq') AS head_id;");
    if (setSales.first())
    {
      _soheadid = setSales.value("head_id").toInt();
      emit newId(_soheadid);
    }
    else if (setSales.lastError().type() != QSqlError::NoError)
    {
      systemError(this, setSales.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }

    populateCCInfo();
    sFillCcardList();

    _captive = false;
    _close->setText("&Cancel");
  }
  else if (ISEDIT(_mode))
  {
    _captive = true;
    _orderNumber->setEnabled(false);
    _cust->setReadOnly(true);
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
  }

  param = pParams.value("cust_id", &valid);
  if (valid)
    _cust->setId(param.toInt());
  else
    // set to configured default cash customer
    _cust->setId(_metrics->value("DefaultSSOSCustId").toInt());

  param = pParams.value("sohead_id", &valid);
  if (valid)
  {
    _soheadid = param.toInt();
    emit newId(_soheadid);
    if (cEdit == _mode)
      setObjectName(QString("salesOrderSimple edit %1").arg(_soheadid));
    else if (cView == _mode)
      setObjectName(QString("salesOrderSimple view %1").arg(_soheadid));
    populate();
    populateCCInfo();
    sFillCcardList();
  }

  param = pParams.value("captive", &valid);
  if (valid)
    _captive = true;

  return NoError;
}

/** \return one of cNew, cEdit, cView, ...
    \todo   change possible modes to an enum in guiclient.h (and add cUnknown?)
 */
int salesOrderSimple::modeState() const
{
  if (ISNEW(_mode))
    return 1;
  else if (ISEDIT(_mode))
    return 2;
  else
    return 3;
}

void salesOrderSimple::sSave()
{
  if (save(false))
  {
    ParameterList params;
    params.append("sohead_id", _soheadid);
    
    printSoForm newdlgX(this, "", true);
    newdlgX.set(params);
    newdlgX.exec();

    if (_captive)
      close();
    else
      prepare();
  }
}

bool salesOrderSimple::save(bool partial)
{
  XSqlQuery saveSales;

  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(!_cust->isValid(), _cust,
                          tr("You must select a Customer for this order before you may save it.") )
         << GuiErrorCheck((_shipTo->id() == -1), _shipTo,
                          tr("You must select a Ship-To for this order before you may save it.") )
         << GuiErrorCheck(!partial && _total->localValue() < 0, _cust,
                          tr("<p>The Total must be a positive value.") )
         << GuiErrorCheck(!partial && _soitem->topLevelItemCount() == 0, _item,
                          tr("<p>You must create at least one Line Item for this order before you may save it.") )
         << GuiErrorCheck(_orderNumber->text().toInt() == 0, _orderNumber,
                          tr( "<p>You must enter a valid Number for this order before you may save it." ) )
         << GuiErrorCheck(_cashReceived->localValue() > 0.0, _postCash,
                          tr( "<p>You must Post Cash Payment before you may save it." ) )
  ;
  
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
      saveSales.bindValue(":cohead_id", _soheadid);
      saveSales.bindValue(":cohead_custponumber", _custPONumber->text());
      saveSales.exec();
      if (saveSales.first())
      {
        errors << GuiErrorCheck(true, _custPONumber,
                                tr("<p>This Customer does not use Blanket P/O "
                                   "Numbers and the P/O Number you entered has "
                                   "already been used for another Sales Order."
                                   "Please verify the P/O Number and either"
                                   "enter a new P/O Number or add to the"
                                   "existing Sales Order." ) );
      }
      else if (saveSales.lastError().type() != QSqlError::NoError)
      {
        systemError(this, saveSales.lastError().databaseText(), __FILE__, __LINE__);
        return false;
      }
    }
  }

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Sales Order"), errors))
      return false;

  MetaSQLQuery mql = mqlLoad("salesOrder", "simple");
  
  ParameterList params;
  params.append("id", _soheadid );
  params.append("number", _orderNumber->text());
  params.append("cust_id", _cust->id());
  params.append("custponumber", _custPONumber->text().trimmed());
  if (_shipTo->id() > 0)
    params.append("shipto_id", _shipTo->id());
  
  if ((_mode == cEdit) || ((_mode == cNew) && _saved))
    params.append("EditMode", true);
  else if (_mode == cNew)
    params.append("NewMode", true);
  saveSales = mql.toQuery(params);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Saving Order"),
                           saveSales, __FILE__, __LINE__))
  {
    return false;
  }

  // TODO: should this be done before saveSales.exec()?
  if ((cNew == _mode) && (!_saved)
      && ! _lock.acquire("cohead", _soheadid,
                         AppLock::Interactive))
  {
    return false;
  }

  _saved = true;

  if (!partial)
  {
    if ( (_mode == cNew) || (_mode == cEdit) )
    {
      omfgThis->sSalesOrdersUpdated(_soheadid);
      omfgThis->sProjectsUpdated(_soheadid);
    }
  }
  else
  {
    populateCCInfo();
    
  }

  emit saved(_soheadid);

  return true;
}

void salesOrderSimple::sSaveLine()
{
  XSqlQuery salesSave;
  QList<GuiErrorCheck> errors;
  errors
  << GuiErrorCheck(!(_qty->toDouble() > 0), _qty,
                   tr("<p>You must enter a valid Quantity Ordered before saving this Sales Order Item."))
  << GuiErrorCheck((_qty->toDouble() != (double)qRound(_qty->toDouble()) &&
                    _qty->validator()->inherits("QIntValidator")), _qty,
                   tr("This UOM for this Item does not allow fractional quantities. Please fix the quantity."))
  ;
  
  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Sales Order Item"), errors))
    return;
  
  // Save the cohead
  if (!save(true))
    return;
  
  MetaSQLQuery mql = mqlLoad("salesOrderItem", "simple");
  
  ParameterList params;
  params.append("id", _soitemid);
  params.append("sohead_id", _soheadid);
  params.append("qtyord", _qty->toDouble());
  params.append("item_id", _item->id());
  params.append("cust_id", _cust->id());
  params.append("shipto_id", _shipTo->id());
  if (_lineMode == cNew)
    params.append("NewMode", true);
  else if (_lineMode == cEdit)
    params.append("EditMode", true);
  salesSave = mql.toQuery(params);
  if (salesSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, salesSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
  prepareLine();
  sFillItemList();
  _item->setFocus();
}

void salesOrderSimple::sPopulateMenu(QMenu *pMenu)
{
  if ((_mode == cNew) || (_mode == cEdit))
  {
    QAction *menuItem;
    bool  didsomething = false;
    if (_numSelected == 1)
    {
      didsomething = true;
      pMenu->addAction(tr("Edit Line..."), this, SLOT(sEdit()));
      pMenu->addAction(tr("Delete Line..."), this, SLOT(sDelete()));
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
    if (didsomething)
      pMenu->addSeparator();
    menuItem = pMenu->addAction(tr("Item Workbench"), this, SLOT(sViewItemWorkbench()));
    menuItem->setEnabled(_privileges->check("ViewItemAvailabilityWorkbench"));
  }
}

void salesOrderSimple::populateOrderNumber()
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
      else if (populateSales.lastError().type() != QSqlError::NoError)
      {
            systemError(this, populateSales.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
    _userEnteredOrderNumber = false;
  }
}

void salesOrderSimple::sSetUserEnteredOrderNumber()
{
  _userEnteredOrderNumber = true;
}

void salesOrderSimple::sHandleOrderNumber()
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
  }
  else
  {
    XSqlQuery query;
    if ( (_mode == cNew) && (_userEnteredOrderNumber) )
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
          systemError(this, storedProcErrorLookup("deleteSO", result), __FILE__, __LINE__);
          return;
        }
      }
      else if (query.lastError().type() != QSqlError::NoError)
      {
          systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
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
        emit newModeState(2);
        _soheadid  = query.value("cohead_id").toInt();
        populate();
        _orderNumber->setEnabled(false);
        _cust->setReadOnly(true);
        populateCCInfo();
        sFillCcardList();
      }
      else
      {
        QString orderNumber = _orderNumber->text();
        if (_metrics->value("CONumberGeneration") == "O")
        {
          query.prepare( "SELECT releaseSoNumber(:orderNumber);" );
          query.bindValue(":orderNumber", _orderNumberGen);
          query.exec();
          _orderNumber->setText(orderNumber);
          _userEnteredOrderNumber = false;
          _orderNumber->setEnabled(false);
        }
        else
        {
          _orderNumber->setText(orderNumber);
          _orderNumber->setEnabled(false);
        }
      }
    }
  }
}

// Is the first SELECT here responsible for the bug where the Currency kept disappearing?
void salesOrderSimple::sPopulateCustomerInfo(int pCustid)
{
  if (_cust->isValid())
  {
    QString sql("SELECT cust_creditstatus, cust_usespos, cust_blanketpos,"
                "       COALESCE(shipto_id, -1) AS shiptoid,"
                "       cust_preferred_warehous_id "
                "FROM custinfo LEFT OUTER JOIN shiptoinfo ON ((shipto_cust_id=cust_id)"
                "                                         AND (shipto_default)) "
                "WHERE (cust_id=<? value(\"cust_id\") ?>);");

    MetaSQLQuery  mql(sql);
    ParameterList params;
    params.append("cust_id", pCustid);
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
          _shipTo->setCustid(-1);
          _cust->setFocus();
          return;
        }
      }

      sFillCcardList();
      _usesPos     = cust.value("cust_usespos").toBool();
      _blanketPos  = cust.value("cust_blanketpos").toBool();

      if (ISNEW(_mode) && cust.value("shiptoid").toInt() != -1)
        _shipTo->setId(cust.value("shiptoid").toInt());

    }
    else if (cust.lastError().type() != QSqlError::NoError)
    {
      systemError(this, cust.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    _shipTo->setCustid(-1);
  }
}

void salesOrderSimple::sDelete()
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
          systemError(this, storedProcErrorLookup("deleteSOItem", result),  __FILE__, __LINE__);
      }
      else if (deleteSales.lastError().type() != QSqlError::NoError)
        systemError(this, deleteSales.lastError().databaseText(),                   __FILE__, __LINE__);
      
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
              systemError(this, storedProcErrorLookup("deleteSO", result),  __FILE__, __LINE__);
          }
          else if (deleteSales.lastError().type() != QSqlError::NoError)
            systemError(this, deleteSales.lastError().databaseText(),               __FILE__, __LINE__);
          
          omfgThis->sSalesOrdersUpdated(_soheadid);
          _captive = false;
          prepare();
        }
      }
    }
  }
}

void salesOrderSimple::populate()
{
  if ( (_mode == cNew) || (_mode == cEdit) || (_mode == cView) )
  {
    XSqlQuery so;
    if (_mode == cEdit
        && !_lock.acquire("cohead", _soheadid,
                          AppLock::Interactive))
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
      _cust->setId(so.value("cohead_cust_id").toInt());
      _blanketPos = so.value("cust_blanketpos").toBool();
      _custPONumber->setText(so.value("cohead_custponumber"));

      emit populated();
      sFillItemList();
      // TODO - a partial save is not saving everything
      if (! ISVIEW(_mode))
        save(false);
    }
    else if (so.lastError().type() != QSqlError::NoError)
    {
      systemError(this, so.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void salesOrderSimple::sFillItemList()
{
  XSqlQuery fillSales;

  _soitem->clear();
  MetaSQLQuery mql = mqlLoad("salesOrderItems", "list");
  
  ParameterList params;
  params.append("excludeCancelled", true);
  params.append("sohead_id", _soheadid);
  XSqlQuery fl = mql.toQuery(params);
  _soitem->populate(fl, true);
  if (fl.lastError().type() != QSqlError::NoError)
  {
    systemError(this, fl.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
  _cust->setReadOnly(fl.size() || !ISNEW(_mode));

  //  Determine the subtotal
  fillSales.prepare( "SELECT SUM(round((coitem_qtyord * coitem_qty_invuomratio) * (coitem_price / coitem_price_invuomratio),2)) AS subtotal,"
                    "       SUM(round((coitem_qtyord * coitem_qty_invuomratio) * (coitem_unitcost / coitem_price_invuomratio),2)) AS totalcost "
                    "FROM cohead JOIN coitem ON (coitem_cohead_id=cohead_id) "
                    "WHERE ( (cohead_id=:head_id)"
                    " AND (coitem_status <> 'X') );" );
  fillSales.bindValue(":head_id", _soheadid);
  fillSales.exec();
  if (fillSales.first())
  {
    _subtotal->setLocalValue(fillSales.value("subtotal").toDouble());
  }
  else if (fillSales.lastError().type() != QSqlError::NoError)
  {
      systemError(this, fillSales.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sCalculateTax();  // triggers sCalculateTotal();
}

void salesOrderSimple::sCalculateTotal()
{
  double total = _subtotal->localValue() + _tax->localValue();
  _total->setLocalValue(total);
  _cashTotal->setLocalValue(total);

  double balance = total - _authCC;
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

bool salesOrderSimple::deleteForCancel()
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
        systemError(this, storedProcErrorLookup("deleteSO", result),
                    __FILE__, __LINE__);
    }
    else if (query.lastError().type() != QSqlError::NoError)
        systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);

    if ((_metrics->value("CONumberGeneration") == "A") ||
        (_metrics->value("CONumberGeneration") == "O"))
    {
      query.prepare( "SELECT releaseSONumber(:orderNumber);" );
      query.bindValue(":orderNumber", _orderNumber->text());
      query.exec();
      if (query.lastError().type() != QSqlError::NoError)
        systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
    }
  }

  if (! _lock.release())
    ErrorReporter::error(QtCriticalMsg, this, tr("Locking Error"),
                         _lock.lastError(), __FILE__, __LINE__);

  return true;
}

void salesOrderSimple::prepare()
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

  _ignoreSignals = true;
  _cust->setId(-1);
  _shipTo->setId(-1);
  _ignoreSignals = false;
  _custPONumber->clear();
  _subtotal->clear();
  _tax->clear();
  _total->clear();
  _cashTotal->clear();
  _balance->clear();
  _cashBalance->clear();
  _CCAmount->clear();
  _CCCVV->clear();

  if ( (_mode == cEdit) || (_mode == cNew) )
  {
    _mode = cNew;
    emit newModeState(1);
    setObjectName("salesOrderSimple new");
  }

  populateOrderNumber();
  if (_orderNumber->text().isEmpty())
    _orderNumber->setFocus();
  else
    _cust->setFocus();

  XSqlQuery headid;
  headid.exec("SELECT NEXTVAL('cohead_cohead_id_seq') AS _soheadid");

  if (headid.first())
  {
    _soheadid = headid.value("_soheadid").toInt();
    emit newId(_soheadid);
    populateCCInfo();
    sFillCcardList();
  }
  else if (headid.lastError().type() != QSqlError::NoError)
    systemError(this, headid.lastError().databaseText(), __FILE__, __LINE__);

  _soitem->clear();
  _cust->setReadOnly(false);

  _saved = false;
}

void salesOrderSimple::prepareLine()
{
  _item->setId(-1);
  _qty->clear();
  
  XSqlQuery salesprepare;
  //  Grab the next coitem_id
  salesprepare.exec("SELECT NEXTVAL('coitem_coitem_id_seq') AS _coitem_id");
  if (salesprepare.first())
    _soitemid = salesprepare.value("_coitem_id").toInt();
  else if (salesprepare.lastError().type() != QSqlError::NoError)
    systemError(this, salesprepare.lastError().databaseText(), __FILE__, __LINE__);
}

void salesOrderSimple::closeEvent(QCloseEvent *pEvent)
{
  if (!deleteForCancel())
  {
    pEvent->ignore();
    return;
  }

  disconnect(_orderNumber, SIGNAL(editingFinished()), this, SLOT(sHandleOrderNumber()));

  if (cNew == _mode && _saved)
    omfgThis->sSalesOrdersUpdated(-1);

  XWidget::closeEvent(pEvent);
}

void salesOrderSimple::sHandleSalesOrderEvent(int pSoheadid, bool)
{
  if (pSoheadid == _soheadid)
    sFillItemList();
}

void salesOrderSimple::sTaxDetail()
{
  XSqlQuery taxq;
  ParameterList params;
  params.append("order_id", _soheadid);
  params.append("order_type", "S");

  // mode => view since there are no fields to hold modified tax data
  params.append("mode", "view");

  taxBreakdown newdlg(this, "", true);
  if (newdlg.set(params) == NoError && newdlg.exec() == XDialog::Accepted)
  {
    populate();
  }
}

void salesOrderSimple::setViewMode()
{
  if (cEdit == _mode)
  {
    // Undo some changes set for the edit mode
    _captive = false;

    disconnect( omfgThis, SIGNAL(salesOrdersUpdated(int, bool)),  this, SLOT(sHandleSalesOrderEvent(int, bool)));
  }

  _paymentInformation->removeTab(_paymentInformation->indexOf(_cashPage));
  _paymentInformation->removeTab(_paymentInformation->indexOf(_creditCardPage));

  _mode = cView;
  emit newModeState(3);
  setObjectName(QString("salesOrderSimple view %1").arg(_soheadid));

  _orderNumber->setEnabled(false);
  _cust->setReadOnly(true);
  _custPONumber->setEnabled(false);
  _save->hide();
}

void salesOrderSimple::populateCCInfo()
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
//  populateSales.bindValue(":curr_id",   _authCC->id());
//  populateSales.bindValue(":effective", _authCC->effective());
  populateSales.exec();
  if (populateSales.first())
    _authCC = populateSales.value("amount").toDouble();
  else
    _authCC = 0.0;
}

void salesOrderSimple::sNewCreditCard()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("cust_id", _cust->id());

  creditCard newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillCcardList();
}

void salesOrderSimple::sEditCreditCard()
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

void salesOrderSimple::sViewCreditCard()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cust_id", _cust->id());
  params.append("ccard_id", _cc->id());

  creditCard newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void salesOrderSimple::sMoveUp()
{
  XSqlQuery moveSales;
  moveSales.prepare("SELECT moveCcardUp(:ccard_id) AS result;");
  moveSales.bindValue(":ccard_id", _cc->id());
  moveSales.exec();

    sFillCcardList();
}

void salesOrderSimple::sMoveDown()
{
  XSqlQuery moveSales;
  moveSales.prepare("SELECT moveCcardDown(:ccard_id) AS result;");
  moveSales.bindValue(":ccard_id", _cc->id());
  moveSales.exec();

    sFillCcardList();
}

void salesOrderSimple::sFillCcardList()
{
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
  if (cl.lastError().type() != QSqlError::NoError)
  {
    systemError(this, cl.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void salesOrderSimple::sAuthorizeCC()
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
                                           (_tax->isZero()),
                                           0, 0,
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

  populateCCInfo();
  sFillCcardList();
  _CCCVV->clear();
}

void salesOrderSimple::sChargeCC()
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
                                        (_tax->isZero()),
                                        0, 0,
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

  populateCCInfo();
  sFillCcardList();
  _CCCVV->clear();
}

bool salesOrderSimple::okToProcessCC()
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

void salesOrderSimple::sReturnStock()
{
  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");
  
  XSqlQuery returnSales;
  returnSales.exec("BEGIN;"); // because of possible lot, serial, or location distribution cancelations
  returnSales.prepare("SELECT returnItemShipments(:soitem_id) AS result;");
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    returnSales.bindValue(":soitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
    returnSales.exec();
    if (returnSales.first())
    {
      int result = returnSales.value("result").toInt();
      if (result < 0)
      {
        rollback.exec();
        systemError(this, storedProcErrorLookup("returnItemShipments", result) +
                    tr("<br>Line Item %1").arg(selected[i]->text(0)),
                    __FILE__, __LINE__);
        return;
      }
      if (distributeInventory::SeriesAdjust(returnSales.value("result").toInt(), this) == XDialog::Rejected)
      {
        rollback.exec();
        QMessageBox::information( this, tr("Return Stock"), tr("Transaction Canceled") );
        return;
      }

    }
    else if (returnSales.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      systemError(this, tr("Line Item %1\n").arg(selected[i]->text(0)) +
                  returnSales.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  returnSales.exec("COMMIT;");

    sFillItemList();
}

void salesOrderSimple::sIssueStock()
{
  bool update = false;
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem *soitem = (XTreeWidgetItem *)(selected[i]);
    if (soitem->altId() != 1 && soitem->altId() != 4)
    {
      ParameterList params;
      params.append("soitem_id", soitem->id());
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
  }
}

void salesOrderSimple::sIssueLineBalance()
{
  XSqlQuery issueSales;
  bool job = false;
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem *soitem = (XTreeWidgetItem *)(selected[i]);
    if (soitem->altId() != 1 && soitem->altId() != 4)
    {
      // sufficientInventoryToShipItem assumes line balance if qty not passed
      issueSales.prepare("SELECT itemsite_id, item_number, warehous_code, itemsite_costmethod, "
                  "       sufficientInventoryToShipItem('SO', coitem_id) AS isqtyavail "
                  "  FROM coitem JOIN itemsite ON (itemsite_id=coitem_itemsite_id)"
                  "              JOIN item ON (item_id=itemsite_item_id)"
                  "              JOIN whsinfo ON (warehous_id=itemsite_warehous_id) "
                  " WHERE (coitem_id=:soitem_id); ");
      issueSales.bindValue(":soitem_id", soitem->id());
      issueSales.exec();
      if (issueSales.lastError().type() != QSqlError::NoError)
      {
        systemError(this, issueSales.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
      while (issueSales.next())
      {
        if (issueSales.value("itemsite_costmethod").toString() == "J")
          job = true;

        if (issueSales.value("isqtyavail").toInt() < 0 &&
            issueSales.value("itemsite_costmethod").toString() != "J")
        {
          QMessageBox::critical(this, tr("Insufficient Inventory"),
                                      tr("<p>There is not enough Inventory to issue the amount required"
                                         " of Item %1 in Site %2.")
                                .arg(issueSales.value("item_number").toString())
                                .arg(issueSales.value("warehous_code").toString()) );
          return;
        }
      }

      issueSales.prepare("SELECT itemsite_id, itemsite_costmethod, item_number, warehous_code, "
                "       sufficientInventoryToShipItem('SO', coitem_id) AS isqtyavail "
                "  FROM coitem JOIN itemsite ON (itemsite_id=coitem_itemsite_id)"
                "              JOIN item ON (item_id=itemsite_item_id)"
                "              JOIN whsinfo ON (warehous_id=itemsite_warehous_id) "
                " WHERE ((coitem_id=:soitem_id) "
                "   AND (NOT ((item_type = 'R') OR (itemsite_controlmethod = 'N'))) "
                "   AND ((itemsite_controlmethod IN ('L', 'S')) OR (itemsite_loccntrl)));");
      issueSales.bindValue(":soitem_id", soitem->id());
      issueSales.exec();
      if (issueSales.lastError().type() != QSqlError::NoError)
      {
        systemError(this, issueSales.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
      while (issueSales.next())
      {
        if (issueSales.value("isqtyavail").toInt() < 0 && issueSales.value("itemsite_costmethod").toString() != "J")
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
      }

      int       invhistid      = 0;
      int       itemlocSeries  = 0;
      XSqlQuery rollback;
      rollback.prepare("ROLLBACK;");

      issueSales.exec("BEGIN;"); // because of possible lot, serial, or location distribution cancelations
      // If this is a lot/serial controlled job item, we need to post production first
      if (job)
      {
        XSqlQuery prod;
        prod.prepare("SELECT postSoItemProduction(:soitem_id, now()) AS result;");
        prod.bindValue(":soitem_id", _soitem->id());
        prod.exec();
        if (prod.lastError().type() != QSqlError::NoError)
        {
          rollback.exec();
          systemError(this, prod.lastError().databaseText(), __FILE__, __LINE__);
          return;
        }
        if (prod.first())
        {
          itemlocSeries = prod.value("result").toInt();

          if (itemlocSeries < 0)
          {
            rollback.exec();
                      systemError(this, storedProcErrorLookup("postProduction", itemlocSeries),
                        __FILE__, __LINE__);
            return;
          }
          else if (distributeInventory::SeriesAdjust(itemlocSeries, this) == XDialog::Rejected)
          {
            rollback.exec();
            QMessageBox::information( this, tr("Issue to Shipping"), tr("Issue Canceled") );
            return;
          }

          // Need to get the inventory history id so we can auto reverse the distribution when issuing
          prod.prepare("SELECT invhist_id "
                       "FROM invhist "
                       "WHERE ((invhist_series = :itemlocseries) "
                       " AND (invhist_transtype = 'RM')); ");
          prod.bindValue(":itemlocseries", itemlocSeries);
          prod.exec();
          if (prod.lastError().type() != QSqlError::NoError)
          {
            rollback.exec();
            systemError(this, prod.lastError().databaseText(), __FILE__, __LINE__);
            return;
          }
          if (prod.first())
            invhistid = prod.value("invhist_id").toInt();
          else
          {
            rollback.exec();
                      systemError(this, tr("Inventory history not found"),
                        __FILE__, __LINE__);
            return;
          }
        }
      }

      issueSales.prepare("SELECT issueLineBalanceToShipping('SO', :soitem_id, now(), :itemlocseries, :invhist_id) AS result;");
      ;
      issueSales.bindValue(":soitem_id", soitem->id());
      if (invhistid)
        issueSales.bindValue(":invhist_id", invhistid);
      if (itemlocSeries)
        issueSales.bindValue(":itemlocseries", itemlocSeries);
      issueSales.exec();
      if (issueSales.lastError().type() != QSqlError::NoError)
      {
        rollback.exec();
        systemError(this, issueSales.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
      if (issueSales.first())
      {
        int result = issueSales.value("result").toInt();
        if (result < 0)
        {
          rollback.exec();
                      systemError(this, storedProcErrorLookup("issueLineBalanceToShipping", result) +
                      tr("<br>Line Item %1").arg(selected[i]->text(0)),
                      __FILE__, __LINE__);
          return;
        }

        if (distributeInventory::SeriesAdjust(issueSales.value("result").toInt(), this) == XDialog::Rejected)
        {
          rollback.exec();
          QMessageBox::information( this, tr("Issue to Shipping"), tr("Transaction Canceled") );
          return;
        }

        issueSales.exec("COMMIT;");
      }
      else
      {
        rollback.exec();
        systemError(this, tr("Line Item %1\n").arg(selected[i]->text(0)) +
                    issueSales.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
  }

  sFillItemList();
}

void salesOrderSimple::sCalculateTax()
{
  XSqlQuery taxq;
  taxq.prepare( "SELECT SUM(tax) AS tax "
                "FROM ("
                "SELECT ROUND(SUM(taxdetail_tax),2) AS tax "
                "FROM tax "
                " JOIN calculateTaxDetailSummary(:type, :cohead_id, 'T') ON (taxdetail_tax_id=tax_id)"
                "GROUP BY tax_id) AS data;" );

  taxq.bindValue(":cohead_id", _soheadid);
  taxq.bindValue(":type","S");
  taxq.exec();
  if (taxq.first())
    _tax->setLocalValue(taxq.value("tax").toDouble());
  else if (taxq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sCalculateTotal();
}

void salesOrderSimple::sEnterCashPayment()
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
  else if (cashsave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, cashsave.lastError().databaseText(), __FILE__, __LINE__);
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
  else if (cashsave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, cashsave.lastError().databaseText(), __FILE__, __LINE__);
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
  cashsave.bindValue(":cashrcpt_usecustdeposit", true);
  cashsave.bindValue(":cashrcpt_discount", 0.0);
  cashsave.bindValue(":cashrcpt_curr_id", _cashReceived->id());
  if(_altAccnt->isChecked())
    cashsave.bindValue(":cashrcpt_salescat_id", _salescat->id());
  else
    cashsave.bindValue(":cashrcpt_salescat_id", -1);
  cashsave.exec();
  if (cashsave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, cashsave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
    
  // Post the Cash Receipt
  XSqlQuery cashPost;
  int journalNumber = -1;
    
  cashPost.exec("SELECT fetchJournalNumber('C/R') AS journalnumber;");
  if (cashPost.first())
    journalNumber = cashPost.value("journalnumber").toInt();
  else if (cashPost.lastError().type() != QSqlError::NoError)
  {
    systemError(this, cashPost.lastError().databaseText(), __FILE__, __LINE__);
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
      systemError(this, storedProcErrorLookup("postCashReceipt", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (cashPost.lastError().type() != QSqlError::NoError)
  {
    systemError(this, cashPost.lastError().databaseText(), __FILE__, __LINE__);
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
    if (cashPost.lastError().type() != QSqlError::NoError)
    {
      systemError(this, cashPost.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else if (cashPost.lastError().type() != QSqlError::NoError)
  {
    systemError(this, cashPost.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
  _cashReceived->clear();
}

void salesOrderSimple::sRecalculatePrice()
{
  if (QMessageBox::question(this, tr("Update all prices?"),
                            tr("Do you want to recalculate all prices for the order including:\n\t- Line items\n\t - Taxes\n\t - Freight ?"),
                            QMessageBox::Yes | QMessageBox::Escape,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    ParameterList params;
    QString       sql;
    QString       sqlchk;
    sql ="UPDATE coitem SET coitem_price=itemprice(item_id, "
    "cohead_cust_id, <? value(\"shipto_id\") ?>, coitem_qtyord, "
    "coitem_qty_uom_id, coitem_price_uom_id, "
    "cohead_curr_id,cohead_orderdate, "
    "<? if exists(\"UseSchedDate\") ?>"
    " coitem_scheddate "
    "<? else ?> "
    "<? value(\"asOf\") ?>"
    "<? endif ?> "
    "), "
    "coitem_custprice=itemprice(item_id, cohead_cust_id, "
    "<? value(\"shipto_id\") ?>,coitem_qtyord, coitem_qty_uom_id, "
    "coitem_price_uom_id, cohead_curr_id, cohead_orderdate, "
    "<? if exists(\"UseSchedDate\") ?>"
    " coitem_scheddate "
    "<? else ?> "
    "<? value(\"asOf\") ?>"
    "<? endif ?> "
    ") "
    "FROM cohead, item, itemsite "
    "WHERE ( (coitem_status NOT IN ('C','X')) "
    "AND (coitem_subnumber=0) "
    "AND (NOT coitem_firm) "
    "<? if exists(\"ignoreDiscounts\") ?>"
    "AND (coitem_price = coitem_custprice) "
    "<? endif ?>"
    "AND (itemsite_id=coitem_itemsite_id) "
    "AND (itemsite_item_id=item_id) "
    "AND (coitem_cohead_id=cohead_id) "
    "AND (cohead_id=<? value(\"cohead_id\") ?>) );";
    sqlchk ="SELECT MIN(itemprice(item_id, cohead_cust_id, "
    "                     <? value(\"shipto_id\") ?>, coitem_qtyord, "
    "                     coitem_qty_uom_id, coitem_price_uom_id, "
    "                     cohead_curr_id,cohead_orderdate, "
    "                     <? if exists(\"UseSchedDate\") ?> coitem_scheddate "
    "                     <? else ?> <? value(\"asOf\") ?>"
    "                     <? endif ?>)) AS pricechk "
    "FROM cohead, coitem, item, itemsite "
    "WHERE ( (coitem_cohead_id=cohead_id) "
    "  AND   (coitem_status NOT IN ('C','X')) "
    "  AND   (coitem_subnumber=0) "
    "  AND   (NOT coitem_firm) "
    "<? if exists(\"ignoreDiscounts\") ?>"
    "  AND   (coitem_price = coitem_custprice) "
    "<? endif ?>"
    "  AND   (itemsite_id=coitem_itemsite_id) "
    "  AND   (itemsite_item_id=item_id) "
    "  AND   (coitem_cohead_id=cohead_id) "
    "  AND   (cohead_id=<? value(\"cohead_id\") ?>) );";
    params.append("cohead_id", _soheadid);
    params.append("shipto_id", _shipTo->id());
    if (_metrics->boolean("IgnoreCustDisc"))
      params.append("ignoreDiscounts", true);
    if (_metrics->value("soPriceEffective") == "ScheduleDate")
      params.append("UseSchedDate", true);
    MetaSQLQuery mql(sql);
    MetaSQLQuery mqlchk(sqlchk);
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
    else if (itempricechk.lastError().type() != QSqlError::NoError)
    {
      systemError(this, itempricechk.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    XSqlQuery setitemprice = mql.toQuery(params);
    if (setitemprice.lastError().type() != QSqlError::NoError)
    {
      systemError(this, setitemprice.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    sFillItemList();
  }
}

void salesOrderSimple::newSalesOrder(int pCustid, QWidget *parent)
{
  // Check for an Item window in new mode already.
  if (pCustid == -1)
  {
    QWidgetList list = omfgThis->windowList();
    for (int i = 0; i < list.size(); i++)
    {
      QWidget *w = list.at(i);
      if (QString::compare(w->objectName(), "salesOrderSimple new")==0)
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
  
  salesOrderSimple *newdlg = new salesOrderSimple(parent);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

