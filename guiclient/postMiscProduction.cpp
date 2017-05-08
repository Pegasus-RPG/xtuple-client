/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "postMiscProduction.h"

#include <QMessageBox>
#include <QVariant>
#include <QSqlError>

#include "distributeInventory.h"
#include "storedProcErrorLookup.h"
#include "errorReporter.h"

postMiscProduction::postMiscProduction(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));

  _captive = false;
  _controlled = false;
  _itemsiteid = -1;
  _sense = 1;
  _qty = 0;

  _item->setType(ItemLineEdit::cManufactured);
  _qtyToPost->setValidator(omfgThis->qtyVal());
  
  _immediateTransfer->setEnabled(_privileges->check("CreateInterWarehouseTrans"));
    
  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs")) 
  {
    _warehouseLit->hide();
    _warehouse->hide();
    _immediateTransfer->hide();
    _transferWarehouse->hide();
  }
  else
    _transferWarehouse->setEnabled(_immediateTransfer->isChecked());

  if (_preferences->boolean("XCheckBox/forgetful"))
    _backflush->setChecked(true);

  _nonPickItems->setEnabled(_backflush->isChecked() &&
			    _privileges->check("ChangeNonPickItems"));

  // TODO: unhide as part of implementation of 5847
  _nonPickItems->hide();
}

postMiscProduction::~postMiscProduction()
{
  // no need to delete child widgets, Qt does it all for us
}

void postMiscProduction::languageChange()
{
  retranslateUi(this);
}

enum SetResponse postMiscProduction::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = true;

  QVariant param;
  bool     valid;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _item->setItemsiteid(param.toInt());
    _warehouse->setEnabled(false);
    _item->setReadOnly(true);
  }

  return NoError;
}

void postMiscProduction::sPost()
{
  XSqlQuery postPost;
  if (!okToPost())  
    return;

  // todo - either move this down below or call close (delete) wo
  if (!createwo())
    return;

  int itemlocSeries = handleSeriesAdjustBeforePost();
  int twItemlocSeries = _immediateTransfer->isChecked() ? handleTransferSeriesAdjustBeforePost() : 0;

  // Stage cleanup function to be called on error
  XSqlQuery cleanup;
  cleanup.prepare("SELECT deleteitemlocseries(:itemlocSeries, TRUE), "
                  " CASE WHEN :twItemlocSeries IS NOT NULL THEN deleteitemlocseries(:twItemlocSeries, true) END;");
  cleanup.bindValue(":itemlocSeries", itemlocSeries);
  if (twItemlocSeries > 0)
    cleanup.bindValue(":twItemlocSeries", twItemlocSeries);

  // If the series aren't set properly, cleanup and exit. The methods that set them already displayed the error messages.
  if (itemlocSeries <= 0 || (_immediateTransfer->isChecked() && twItemlocSeries <= 0))
  {
    cleanup.exec();
    return;
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");
  postPost.exec("BEGIN;");
  if (!post(itemlocSeries))
  {
    rollback.exec();
    cleanup.exec();
    return;
  }
  if (!returntool(itemlocSeries))
  {
    rollback.exec();
    cleanup.exec();
    return;
  }
  if (!closewo())
  {
    rollback.exec();
    cleanup.exec();
    return;
  }
  if (_immediateTransfer->isChecked())
  {
    if (!transfer(twItemlocSeries))
    {
      rollback.exec();
      cleanup.exec();
      return;
    }
  }

  postPost.exec("COMMIT;");
  if (_captive)
    accept();
  else
    clear();
}

bool postMiscProduction::okToPost()
{
  _qty = _qtyToPost->toDouble();
  if (_disassembly->isChecked())
    _qty = _qty * -1;

  if (_qty == 0)
  {
    QMessageBox::warning( this, tr("Invalid Quantity"),
                        tr( "The quantity may not be zero." ) );
    return false;
  }

  if (_immediateTransfer->isChecked())
  {
    if (_warehouse->id() == _transferWarehouse->id())
    {
      QMessageBox::warning( this, tr("Cannot Post Immediate Transfer"),
                            tr( "Transaction canceled. Cannot post an immediate transfer for the newly posted production as the\n"
                                "transfer Site is the same as the production Site.  You must manually\n"
                                "transfer the production to the intended Site." ) );
      return false;
    }
  }

  _itemsiteid = 0;
  XSqlQuery itemsite;
  itemsite.prepare( "SELECT itemsite_id, isControlledItemsite(itemsite_id) AS controlled "
                    "FROM itemsite "
                    "WHERE ( (itemsite_item_id=:item_id)"
                    " AND (itemsite_warehous_id=:warehous_id) );" );
  itemsite.bindValue(":item_id", _item->id());
  if (_qty > 0)
    itemsite.bindValue(":warehous_id", _warehouse->id());
  else
    itemsite.bindValue(":warehous_id", _transferWarehouse->id());
  itemsite.exec();
  if (itemsite.first())
  {
    _itemsiteid = itemsite.value("itemsite_id").toInt();
    _controlled = itemsite.value("controlled").toBool();
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating Work Order"),
                                itemsite, __FILE__, __LINE__))
  {
    return false;
  }
  else
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Occurred"),
                         tr("%1: Itemsite not found")
                         .arg(windowTitle()),__FILE__,__LINE__);
    return false;
  }

  return true;
}

bool postMiscProduction::createwo()
{
  _woid = 0;
  XSqlQuery wo;
  wo.prepare( "SELECT createWo(fetchWoNumber(), :itemsite_id, :qty, CURRENT_DATE, CURRENT_DATE, :comments) AS result;" );
  wo.bindValue(":itemsite_id", _itemsiteid);
  wo.bindValue(":qty", _qty);
  wo.bindValue(":comments", (tr("Post Misc Production, Document Number ") +
                             _documentNum->text().trimmed() + ", " + _comments->toPlainText()));
  wo.exec();
  if (wo.first())
  {
    _woid = wo.value("result").toInt();
    if (_woid < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating Work Order"),
                             storedProcErrorLookup("createWo", _woid),
                             __FILE__, __LINE__);
      return false;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating Work Order"),
                                wo, __FILE__, __LINE__))
  {
    return false;
  }

  // Delete any Child W/O's created
  XSqlQuery child;
  child.prepare( "SELECT MAX(deleteWo(wo_id, true)) AS result "
                 "FROM wo "
                 "WHERE ((wo_ordtype='W') AND (wo_ordid=:ordid));");
  child.bindValue(":ordid", _woid);
  child.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Child Work Order"),
                                child, __FILE__, __LINE__))
  {
    return false;
  }

  return true;
}

bool postMiscProduction::post(int itemlocSeries)
{
  XSqlQuery post;
  post.prepare("SELECT postProduction(:wo_id, :qty, :backflushMaterials, :itemlocSeries, CURRENT_DATE, TRUE) AS result;");
  post.bindValue(":wo_id", _woid);
  post.bindValue(":qty", _qty);
  post.bindValue(":backflushMaterials", QVariant(_backflush->isChecked()));
  post.bindValue(":itemlocSeries", itemlocSeries);
  post.exec();
  if (post.first())
  {
    int result = post.value("result").toInt();
    if (result < 0 || result != itemlocSeries)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Production"),
                             storedProcErrorLookup("postProduction", result),
                             __FILE__, __LINE__);
      return false;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Production"),
                                post, __FILE__, __LINE__))
  {
    return false;
  }

  return true;
}

bool postMiscProduction::returntool(int itemlocSeries)
{
  XSqlQuery post;
  post.prepare("SELECT returnWoMaterial(womatl_id, womatl_qtyiss, :itemlocSeries, CURRENT_DATE, FALSE, TRUE, TRUE) AS result "
               "FROM womatl JOIN itemsite ON (itemsite_id=womatl_itemsite_id) "
               "            JOIN item ON (item_id=itemsite_item_id) "
               "WHERE (womatl_wo_id=:wo_id) "
               "  AND (item_type='T')"
               "  AND (womatl_qtyiss > 0);");
  post.bindValue(":wo_id", _woid);
  post.bindValue(":itemlocSeries", itemlocSeries);
  post.exec();
  if (post.first())
  {
    int result = post.value("result").toInt();
    if (result < 0 || result != itemlocSeries)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Returning Work Order Material"),
                             storedProcErrorLookup("returnWoMaterial", result),
                             __FILE__, __LINE__);
      return false;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Returning Work Order Material"),
                                post, __FILE__, __LINE__))
  {
    return false;
  }

  return true;
}

bool postMiscProduction::closewo()
{
  XSqlQuery close;
  close.prepare("SELECT closeWo(:wo_id, true, CURRENT_DATE) AS result;");
  close.bindValue(":wo_id", _woid);
  close.exec();
  if (close.first())
  {
    if (close.value("result").toInt() < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Closing Work Order"),
                             storedProcErrorLookup("closeWo", close.value("result").toInt()),
                             __FILE__, __LINE__);
      return false;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Closing Work Order"),
                                close, __FILE__, __LINE__))
  {
    return false;
  }

  return true;
}

bool postMiscProduction::transfer(int itemlocSeries)
{
  XSqlQuery posttransfer;
  posttransfer.prepare("SELECT interWarehouseTransfer( :item_id, :from_warehous_id, :to_warehous_id,"
                       "  :qty, 'W', COALESCE(:documentNumber, formatWoNumber(:wo_id)), 'Transfer from Misc. Production Posting', "
                       "  :itemlocSeries, "
                       "  now(), "
                       "  TRUE, "
                       "  true ) AS result;");
  posttransfer.bindValue(":item_id", _item->id());
  posttransfer.bindValue(":from_warehous_id", _warehouse->id());
  posttransfer.bindValue(":to_warehous_id", _transferWarehouse->id());
  posttransfer.bindValue(":qty", _qty * _sense);
  if (_documentNum->text().length() > 0)
    posttransfer.bindValue(":documentNumber", _documentNum->text().trimmed());
  else 
    posttransfer.bindValue(":wo_id", _woid);
  posttransfer.bindValue(":itemlocSeries", itemlocSeries);
  posttransfer.exec();
  if (!posttransfer.first())
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Occurred"),
                         tr("%1: A System Error occurred at interWarehousTransfer::%2, Item Site ID #%3, Site ID #%4 to Site ID #%5.")
                            .arg(windowTitle())
                            .arg(__LINE__)
                            .arg(_item->id())
                            .arg(_warehouse->id())
                            .arg(_transferWarehouse->id()),__FILE__,__LINE__);
    return false;
  }

  return true;
}

void postMiscProduction::clear()
{
  _item->setId(-1);
  _qtyToPost->clear();
  _documentNum->clear();
  _comments->clear();
  _close->setText(tr("&Close"));

  _item->setFocus();
}

int postMiscProduction::handleSeriesAdjustBeforePost()
{
  XSqlQuery parentItemlocdist;
  int itemlocSeries;
  bool hasControlledBackflushItems = false;

  // Stage cleanup function to be called on error
  XSqlQuery cleanup;
  cleanup.prepare("SELECT deleteitemlocseries(:itemlocSeries, TRUE);");

  // Series for issueToShipping
  XSqlQuery parentSeries;
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
    return -1;
  }

  // If backflush, createItemlocdistParent for each controlled material item
  if (_backflush->isChecked())
  {
    // Handle creation of itemlocdist records for each eligible backflush item (sql below from postProduction backflush handling)
    XSqlQuery backflushItems;
    backflushItems.prepare(
      "SELECT item.item_number, item.item_fractional, itemsite.itemsite_id, itemsite.itemsite_item_id, "
      " CASE WHEN :qty > 0 THEN " //issueWoMaterial qty = noNeg(expected - consumed)
      "   noNeg(((womatl_qtyfxd + ((roundQty(woitem.item_fractional, :qty) + wo_qtyrcv) * womatl_qtyper)) * (1 + womatl_scrap)) - "
      "   (womatl_qtyiss + "
      "   CASE WHEN (womatl_qtywipscrap >  ((womatl_qtyfxd + (roundQty(woitem.item_fractional, :qty) + wo_qtyrcv) * womatl_qtyper) * womatl_scrap)) "
      "        THEN (womatl_qtyfxd + (roundQty(woitem.item_fractional, :qty) + wo_qtyrcv) * womatl_qtyper) * womatl_scrap "
      "        ELSE womatl_qtywipscrap END)) "
      " ELSE " // returnWoMaterial qty = expected * -1
      "   roundQty(woitem.item_fractional, :qty) * womatl_qtyper * -1 " 
      " END AS qty, "
      " womatl_id, womatl_wo_id, womatl_uom_id "
      "FROM womatl, wo "
      " JOIN itemsite AS woitemsite ON wo_itemsite_id=woitemsite.itemsite_id "
      " JOIN item AS woitem ON woitemsite.itemsite_item_id=woitem.item_id, "
      " itemsite, item "
      "WHERE womatl_issuemethod IN ('L', 'M') "
      " AND womatl_wo_id=wo_id "
      " AND womatl_itemsite_id=itemsite.itemsite_id "
      " AND wo_id = :wo_id "
      " AND itemsite.itemsite_item_id=item.item_id "
      " AND isControlledItemsite(itemsite.itemsite_id) "
      "ORDER BY womatl_id;");
    backflushItems.bindValue(":wo_id", _woid);
    backflushItems.bindValue(":qty", _qty);
    backflushItems.exec();
    while (backflushItems.next())
    {
      // create the itemlocdist record for this controlled issueWoMaterial item
      XSqlQuery womatlItemlocdist;
      hasControlledBackflushItems = true;
      womatlItemlocdist.prepare("SELECT createItemlocdistParent(:itemsite_id, "
                                " COALESCE(itemuomtouom(:item_id, :womatl_uom_id, NULL, :qty), :qty), "
                                " 'WO', :wo_id, :itemlocSeries, NULL, NULL, 'IM') AS result;");
      womatlItemlocdist.bindValue(":itemsite_id", backflushItems.value("itemsite_id").toInt());
      womatlItemlocdist.bindValue(":item_id", backflushItems.value("itemsite_item_id").toInt());
      womatlItemlocdist.bindValue(":womatl_uom_id", backflushItems.value("womatl_uom_id").toInt());
      womatlItemlocdist.bindValue(":item_fractional", backflushItems.value("item_fractional").toBool());
      womatlItemlocdist.bindValue(":wo_id", _woid);
      womatlItemlocdist.bindValue(":itemlocSeries", itemlocSeries);
      if (_disassembly->isChecked()) // if it's disassembly, qty is already negative
        womatlItemlocdist.bindValue(":qty", backflushItems.value("qty").toDouble());
      else 
        womatlItemlocdist.bindValue(":qty", backflushItems.value("qty").toDouble() * -1);
      womatlItemlocdist.exec();
      if (!womatlItemlocdist.first())
      {
        cleanup.exec();
        QMessageBox::information( this, tr("Issue Line to Shipping"), 
          tr("Failed to Create an itemlocdist record for work order backflushed material item %1.")
          .arg(backflushItems.value("item_number").toString()) );
        return -1;
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating itemlocdist Records"),
        womatlItemlocdist, __FILE__, __LINE__))
      {
        cleanup.exec();
        return -1;
      }
    }
  } // backflush handling

  // If controlled item, createItemlocdistParent
  if (_controlled)
  {
    // create the RM itemlocdist record
    parentItemlocdist.prepare("SELECT createItemlocdistParent(wo_itemsite_id, roundQty(item_fractional, :qty), 'WO', :orderitemId, "
                              " :itemlocSeries, NULL, NULL, 'RM') AS result "
                              "FROM wo "
                              " JOIN itemsite ON wo_itemsite_id=itemsite_id "
                              " JOIN item ON itemsite_item_id=item_id "
                              "WHERE wo_id=:wo_id;");
    parentItemlocdist.bindValue(":wo_id", _woid);
    parentItemlocdist.bindValue(":qty", _qty);
    parentItemlocdist.bindValue(":orderitemId", _woid);
    parentItemlocdist.bindValue(":itemlocSeries", itemlocSeries);
    parentItemlocdist.exec();
    if (parentItemlocdist.lastError().type() != QSqlError::NoError)
    {
      cleanup.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating itemlocdist record for post "
        "production controlled item"), parentItemlocdist, __FILE__, __LINE__);
      return -1;
    }
  }

  // Distribute detail
  if ((_controlled || hasControlledBackflushItems) && 
      (distributeInventory::SeriesAdjust(itemlocSeries, this, QString(), QDate(), QDate(), true) ==
      XDialog::Rejected))
  {
    cleanup.exec();
    QMessageBox::information( this, tr("Post Production"), tr("Detail distribution was cancelled.") );
    return -1;
  }

  return itemlocSeries;
}

int postMiscProduction::handleTransferSeriesAdjustBeforePost()
{
  int twItemlocSeries = 0;
  int toWhItemsiteId = 0;
  bool toWhControlled = false;

  if (!_immediateTransfer->isChecked())
    return -1;

  // Stage cleanup function to be called on error
  XSqlQuery cleanup;
  cleanup.prepare("SELECT deleteitemlocseries(:itemlocSeries, TRUE);");

  XSqlQuery parentSeries;
  parentSeries.prepare("SELECT NEXTVAL('itemloc_series_seq') AS result;");
  parentSeries.exec();
  if (parentSeries.first() && parentSeries.value("result").toInt() > 0)
  {
    twItemlocSeries = parentSeries.value("result").toInt();
    cleanup.bindValue(":itemlocSeries", twItemlocSeries);
  }
  else 
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Failed to Retrieve the Next itemloc_series_seq"),
      parentSeries, __FILE__, __LINE__);
    return -1;
  }

  // Get TO warehouse itemsite and control values
  XSqlQuery toWh;
  toWh.prepare("SELECT itemsite_id, isControlledItemsite(itemsite_id) AS controlled "
               "FROM itemsite "
               "WHERE itemsite_warehous_id = :warehouseId "
               " AND itemsite_item_id = "
               " (SELECT itemsite_item_id FROM itemsite WHERE itemsite_id = :itemsiteId);");
  toWh.bindValue(":itemsiteId", _itemsiteid);
  if (_disassembly->isChecked())
    toWh.bindValue(":warehouseId", _warehouse->id());
  else
    toWh.bindValue(":warehouseId", _transferWarehouse->id());
  toWh.exec();
  if (toWh.first())
  {
    toWhControlled = toWh.value("controlled").toBool();
    toWhItemsiteId = toWh.value("itemsite_id").toInt();
  }
  else
  {
    cleanup.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error finding TO warehouse itemsite"),
      toWh, __FILE__, __LINE__);
    return -1;
  }

  if (!_controlled && !toWhControlled)
    return twItemlocSeries;

  XSqlQuery fromWhItemlocdist;
  fromWhItemlocdist.prepare("SELECT createItemlocdistParent(:itemsiteId, :qty, 'W', :orderitemId, "
                            "   :itemlocSeries, NULL, NULL, 'TW') AS result;");
  fromWhItemlocdist.bindValue(":itemsiteId", _itemsiteid);
  // if assembly, _qty is positive, make it negative because interWarehouseTransfer function will create invhist record with negative qty for From wh
  if (_assembly->isChecked())
    fromWhItemlocdist.bindValue(":qty", _qty * -1);
  else 
    fromWhItemlocdist.bindValue(":qty", _qty);
  fromWhItemlocdist.bindValue(":orderitemId", _woid);
  fromWhItemlocdist.bindValue(":itemlocSeries", twItemlocSeries);
  fromWhItemlocdist.exec();
  if (fromWhItemlocdist.first() && toWhControlled)
  {
    XSqlQuery toWhItemlocdist;
    toWhItemlocdist.prepare("SELECT createItemlocdistParent(:itemsiteId, :qty, 'W', :orderitemId, "
                            "   :itemlocSeries, NULL, :itemlocdistId, 'TW') AS result;");
    // if disassembly, _qty is negative, make it positive because interWarehouseTransfer function will create invhist record with positive qty for To wh
    if (_disassembly->isChecked())
      toWhItemlocdist.bindValue(":qty", _qty * -1);
    else 
      toWhItemlocdist.bindValue(":qty", _qty);
    toWhItemlocdist.bindValue(":orderitemId", _woid);
    toWhItemlocdist.bindValue(":itemlocSeries", twItemlocSeries);
    toWhItemlocdist.bindValue(":itemlocdistId", fromWhItemlocdist.value("result").toInt());
    toWhItemlocdist.bindValue(":itemsiteId", toWhItemsiteId);
    toWhItemlocdist.exec();
    if (toWhItemlocdist.lastError().type() != QSqlError::NoError)
    {
      cleanup.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating itemlocdist record For TO Warehouse for interwarehousetransfer"),
          toWhItemlocdist, __FILE__, __LINE__);
      return -1;
    }
  }
  else
  {
    cleanup.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating itemlocdist record for FROM Warehouse for interwarehousetransfer "),
      fromWhItemlocdist, __FILE__, __LINE__);
    return -1;
  }

    // Distribute detail
  if (distributeInventory::SeriesAdjust(twItemlocSeries, this, QString(), QDate(), QDate(), true) ==
      XDialog::Rejected)
  {
    cleanup.exec();
    QMessageBox::information( this, tr("Post Production"), tr("Detail distribution was cancelled.") );
    return -1;
  }

  return twItemlocSeries;
}

