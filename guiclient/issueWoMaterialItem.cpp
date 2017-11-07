/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "issueWoMaterialItem.h"

#include <QSqlError>
#include <QVariant>
#include <QMessageBox>
#include "guiErrorCheck.h"
#include <QValidator>

#include "inputManager.h"
#include "distributeInventory.h"
#include "storedProcErrorLookup.h"
#include "errorReporter.h"

issueWoMaterialItem::issueWoMaterialItem(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_womatl, SIGNAL(newId(int)), this, SLOT(sSetQOH(int)));
  connect(_qtyToIssue, SIGNAL(textChanged(const QString&)), this, SLOT(sPopulateQOH()));
  connect(_issue, SIGNAL(clicked()), this, SLOT(sIssue()));

  _captive = false;
  _transDate->setEnabled(_privileges->check("AlterTransactionDates"));
  _transDate->setDate(omfgThis->dbDate(), true);

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));
  omfgThis->inputManager()->notify(cBCItem, this, this, SLOT(sCatchItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, this, SLOT(sCatchItemsiteid(int)));

// Issue #22778 - Add hidden metric to allow issuing to Exploded WOs
  if (_metrics->boolean("IssueToExplodedWO"))
    _wo->setType(cWoExploded | cWoIssued | cWoReleased);
  else
    _wo->setType(cWoIssued | cWoReleased);

  _qtyToIssue->setValidator(omfgThis->qtyVal());
  _beforeQty->setPrecision(omfgThis->transQtyVal());
  _afterQty->setPrecision(omfgThis->transQtyVal());

  _wo->setFocus();
}

issueWoMaterialItem::~issueWoMaterialItem()
{
    // no need to delete child widgets, Qt does it all for us
}

void issueWoMaterialItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse issueWoMaterialItem::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _captive = true;

    _wo->setId(param.toInt());
    _wo->setEnabled(false);
  }

  param = pParams.value("womatl_id", &valid);
  if (valid)
  {
    _womatl->setId(param.toInt());
    _womatl->setEnabled(false);
  }

  param = pParams.value("qty", &valid);
  if (valid)
    _qtyToIssue->setDouble(param.toDouble());

  return NoError;
}

void issueWoMaterialItem::sCatchItemid(int pItemid)
{
  XSqlQuery issueCatchItemid;
  if (_wo->isValid())
  {
    issueCatchItemid.prepare( "SELECT womatl_id "
               "FROM womatl, itemsite "
               "WHERE ( (womatl_itemsite_id=itemsite_id)"
               " AND (womatl_wo_id=:wo_id)"
               " AND (itemsite_item_id=:item_id) );" );
    issueCatchItemid.bindValue(":wo_id", _wo->id());
    issueCatchItemid.bindValue(":item_id", pItemid);
    issueCatchItemid.exec();
    if (issueCatchItemid.first())
      _womatl->setId(issueCatchItemid.value("womatl_id").toInt());
    else
      audioReject();
  }
  else
    audioReject();
}

void issueWoMaterialItem::sCatchItemsiteid(int pItemsiteid)
{
  XSqlQuery issueCatchItemsiteid;
  if (_wo->isValid())
  {
    issueCatchItemsiteid.prepare( "SELECT womatl_id "
               "FROM womatl "
               "WHERE ((womatl_itemsite_id=:itemsite_id)"
	       "  AND  (womatl_wo_id=:wo_id));" );
    issueCatchItemsiteid.bindValue(":itemsite_id", pItemsiteid);
    issueCatchItemsiteid.bindValue(":wo_id", _wo->id());
    issueCatchItemsiteid.exec();
    if (issueCatchItemsiteid.first())
      _womatl->setId(issueCatchItemsiteid.value("womatl_id").toInt());
    else
      audioReject();
  }
  else
    audioReject();
}

void issueWoMaterialItem::sIssue()
{
  XSqlQuery issueIssue;
  int itemlocSeries;
  
  QList<GuiErrorCheck> errors;
  errors<< GuiErrorCheck(!_transDate->isValid(), _transDate,
                         tr("You must enter a valid transaction date."))
  ;
  if (GuiErrorCheck::reportErrors(this, tr("Invalid date"), errors))
    return;

  // Stage distribution cleanup function to be called on error
  XSqlQuery cleanup;
  cleanup.prepare("SELECT deleteitemlocseries(:itemlocSeries, TRUE);");

  // Get the parent series id
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
    return;
  }
  
  issueIssue.prepare("SELECT womatl_wo_id, womatl_id, itemsite_id, item_number, warehous_code, "
            "       (COALESCE((SELECT SUM(itemloc_qty) "
            "                    FROM itemloc "
            "                   WHERE (itemloc_itemsite_id=itemsite_id)), 0.0) >= roundQty(item_fractional, itemuomtouom(itemsite_item_id, womatl_uom_id, NULL, :qty))) AS isqtyavail, "
            "       formatWoNumber(womatl_wo_id) AS wo_number, "
            "       roundQty(item_fractional, itemuomtouom(itemsite_item_id, womatl_uom_id, NULL, :qty)) * -1 AS post_qty "
            "  FROM womatl, itemsite, item, whsinfo "
            " WHERE ((womatl_itemsite_id=itemsite_id) "
            "   AND (itemsite_item_id=item_id) "
            "   AND (itemsite_warehous_id=warehous_id) "
            "   AND (NOT ((item_type = 'R') OR (itemsite_controlmethod = 'N'))) "
            "   AND isControlledItemsite(itemsite_id) "
            "   AND (womatl_id=:womatl_id)) "
            " ORDER BY womatl_id; ");
  issueIssue.bindValue(":womatl_id", _womatl->id());
  issueIssue.bindValue(":qty", _qtyToIssue->toDouble());
  issueIssue.exec();
  // How could there be more than one record??
  if (issueIssue.first())
  {
    if(!(issueIssue.value("isqtyavail").toBool()))
    {
      QMessageBox::critical(this, tr("Insufficient Inventory"),
        tr("Item Number %1 in Site %2 is a Multiple Location or\n"
           "Lot/Serial controlled Item which is short on Inventory.\n"
           "This transaction cannot be completed as is. Please make\n"
           "sure there is sufficient Quantity on Hand before proceeding.")
          .arg(issueIssue.value("item_number").toString())
          .arg(issueIssue.value("warehous_code").toString()));
      return;
    }

    // Create the parent itemlocdist record for each line item requiring distribution, call distributeInventory::seriesAdjust
    XSqlQuery parentItemlocdist;
    parentItemlocdist.prepare("SELECT createitemlocdistparent(:itemsite_id, :qty, 'WO', "
                              " :orderitemId, :itemlocSeries, NULL, NULL, 'IM');");
    parentItemlocdist.bindValue(":itemsite_id", issueIssue.value("itemsite_id").toInt());
    parentItemlocdist.bindValue(":qty", issueIssue.value("post_qty").toDouble());
    parentItemlocdist.bindValue(":orderitemId", issueIssue.value("womatl_id").toInt());
    parentItemlocdist.bindValue(":itemlocSeries", itemlocSeries);
    parentItemlocdist.exec();
    if (parentItemlocdist.first())
    {
      if (distributeInventory::SeriesAdjust(itemlocSeries, this, QString(), QDate(),
        QDate(), true) == XDialog::Rejected)
      {
        cleanup.exec();
        QMessageBox::information( this, tr("Material Issue"), tr("Detail Distribution was Cancelled") );
        return;
      }
    }
    else
    {
      cleanup.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating itemlocdist Records"),
                              parentItemlocdist, __FILE__, __LINE__);
      return;
    }
  }

  // Wrap issueWoMaterial and INSERT in a transaction in case insert fails.
  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  // Post inventory
  issueIssue.exec("BEGIN;");
  issueIssue.prepare("SELECT issueWoMaterial(:womatl_id, :qty, :itemlocSeries, true, :date, true) AS result;");
  issueIssue.bindValue(":womatl_id", _womatl->id());
  issueIssue.bindValue(":qty", _qtyToIssue->toDouble());
  issueIssue.bindValue(":itemlocSeries", itemlocSeries);
  issueIssue.bindValue(":date",  _transDate->date());
  issueIssue.exec();
  if (issueIssue.first())
  {
    int result = issueIssue.value("result").toInt();
    if (result < 0 || result != itemlocSeries)
    {
      rollback.exec();
      cleanup.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Issuing Material To Work Order #: %1")
                           .arg(_wo->id()),
                            storedProcErrorLookup("issueWoMaterial", result),
                            __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    rollback.exec();
    cleanup.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Issuing Material to Work Order #: %1")
                         .arg(_wo->id()),
                         issueIssue, __FILE__, __LINE__);
    return;
  }

  issueIssue.exec("COMMIT;");

  if (_captive)
    close();
  else
  {
    _close->setText(tr("Close"));
    _qtyToIssue->clear();
    _womatl->setId(-1);
    _womatl->setWoid(_wo->id());
    _womatl->setFocus();
  }
}

void issueWoMaterialItem::sSetQOH(int pWomatlid)
{
  if (pWomatlid == -1)
  {
    _cachedQOH = 0.0;
    _beforeQty->clear();
    _afterQty->clear();
  }
  else
  {
    if (_wo->method() == "A")
      _qtyToIssue->setDouble(_womatl->qtyRequired() - _womatl->qtyIssued());
    else
      _qtyToIssue->setDouble(_womatl->qtyIssued() * -1);
    
    XSqlQuery qoh;
    qoh.prepare( "SELECT itemuomtouom(itemsite_item_id, NULL, womatl_uom_id, qtyAvailable(itemsite_id)) AS availableqoh,"
                 "       uom_name "
                 "  FROM womatl, itemsite, uom"
                 " WHERE((womatl_itemsite_id=itemsite_id)"
                 "   AND (womatl_uom_id=uom_id)"
                 "   AND (womatl_id=:womatl_id) );" );
    qoh.bindValue(":womatl_id", pWomatlid);
    qoh.exec();
    if (qoh.first())
    {
      _uomQty->setText(qoh.value("uom_name").toString());
      _cachedQOH = qoh.value("availableqoh").toDouble();
      _beforeQty->setDouble(_cachedQOH);
    }
    else
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Setting Quantity On Hand Information"),
                         qoh, __FILE__, __LINE__);
  }
  sPopulateQOH();
}

void issueWoMaterialItem::sPopulateQOH()
{
  _afterQty->setDouble(_cachedQOH - _qtyToIssue->toDouble());
}

