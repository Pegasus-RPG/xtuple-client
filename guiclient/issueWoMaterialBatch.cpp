/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "issueWoMaterialBatch.h"

#include <QSqlError>
#include <QVariant>
#include <QMessageBox>
#include <metasql.h>
#include "inputManager.h"
#include "distributeInventory.h"
#include "errorReporter.h"

issueWoMaterialBatch::issueWoMaterialBatch(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _captive = false;

  connect(_issue, SIGNAL(clicked()), this, SLOT(sIssue()));
  connect(_wo, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));

  _hasPush = false;
  _transDate->setEnabled(_privileges->check("AlterTransactionDates"));
  _transDate->setDate(omfgThis->dbDate(), true);

// Issue #22778 - Add hidden metric to allow issuing to Exploded WOs
  if (_metrics->boolean("IssueToExplodedWO"))
    _wo->setType(cWoExploded | cWoIssued | cWoReleased);
  else
    _wo->setType(cWoIssued | cWoReleased);

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _womatl->addColumn(tr("Item Number"),    _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  _womatl->addColumn(tr("Description"),    -1,          Qt::AlignLeft,   true,  "itemdescrip"   );
  _womatl->addColumn(tr("UOM"),            _uomColumn,  Qt::AlignCenter, true,  "uom_name" );
  _womatl->addColumn(tr("Issue Method"),   _itemColumn, Qt::AlignCenter, true,  "issuemethod" );
  _womatl->addColumn(tr("Picklist"),       _itemColumn, Qt::AlignCenter, true,  "picklist" );
  _womatl->addColumn(tr("Required"),       _qtyColumn,  Qt::AlignRight,  true,  "required"  );
  _womatl->addColumn(tr("Available QOH"),  _qtyColumn,  Qt::AlignRight,  true,  "availableqoh"  );
  _womatl->addColumn(tr("Short"),          _qtyColumn,  Qt::AlignRight,  true,  "short"  );

  _wo->setFocus();
}

issueWoMaterialBatch::~issueWoMaterialBatch()
{
  // no need to delete child widgets, Qt does it all for us
}

void issueWoMaterialBatch::languageChange()
{
  retranslateUi(this);
}

enum SetResponse issueWoMaterialBatch::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _captive = true;

    _wo->setId(param.toInt());
    _wo->setReadOnly(true);
  }

  return NoError;
}

void issueWoMaterialBatch::sIssue()
{
  if (!_transDate->isValid())
  {
    QMessageBox::critical(this, tr("Invalid date"),
                          tr("You must enter a valid transaction date.") );
    _transDate->setFocus();
    return;
  }
  
  QString sqlissue =
        ( "SELECT itemsite_id, "
          "       item_number, "
          "       warehous_code, "
          "       (COALESCE((SELECT SUM(itemloc_qty) "
          "                    FROM itemloc "
          "                   WHERE (itemloc_itemsite_id=itemsite_id)), 0.0) "
          "        >= roundQty(item_fractional, noNeg(itemuomtouom(itemsite_item_id, womatl_uom_id, NULL, womatl_qtyreq - womatl_qtyiss)))) AS isqtyavail "
          "  FROM womatl, itemsite, item, whsinfo "
          " WHERE ( (womatl_itemsite_id=itemsite_id) "
          "   AND (itemsite_item_id=item_id) "
          "   AND (itemsite_warehous_id=warehous_id) "
          "   AND (NOT ((item_type = 'R') OR (itemsite_controlmethod = 'N'))) "
          "   AND ((itemsite_controlmethod IN ('L', 'S')) OR (itemsite_loccntrl)) "
          "   AND (womatl_issuemethod IN ('S', 'M')) "
          "  <? if exists(\"pickItemsOnly\") ?>"
          "   AND (womatl_picklist) "
          "  <? endif ?>"
          "   AND (womatl_wo_id=<? value(\"wo_id\") ?>)); ");
  MetaSQLQuery mqlissue(sqlissue);
  ParameterList params;
  params.append("wo_id", _wo->id());
  if (!_nonPickItems->isChecked())
    params.append("pickItemsOnly", true);
  XSqlQuery issue = mqlissue.toQuery(params);
  while(issue.next())
  {
    if(!(issue.value("isqtyavail").toBool()))
    {
      QMessageBox::critical(this, tr("Insufficient Inventory"),
        tr("Item Number %1 in Site %2 is a Multiple Location or\n"
           "Lot/Serial controlled Item which is short on Inventory.\n"
           "This transaction cannot be completed as is. Please make\n"
           "sure there is sufficient Quantity on Hand before proceeding.")
          .arg(issue.value("item_number").toString())
          .arg(issue.value("warehous_code").toString()));
      return;
    }
  }

  sqlissue = ("SELECT COALESCE(bool_and(itemsite_qtyonhand >= roundQty(item_fractional, itemuomtouom(item_id, womatl_uom_id, NULL, roundQty(itemuomfractionalbyuom(item_id, womatl_uom_id), noNeg(CASE WHEN (womatl_qtyreq >= 0) THEN womatl_qtyreq - womatl_qtyiss ELSE womatl_qtyiss * -1 END))))), TRUE) AS isqtyavail "
              "FROM womatl "
              "JOIN itemsite ON (womatl_itemsite_id = itemsite_id) "
              "JOIN item ON (itemsite_item_id = item_id) "
              "WHERE (fetchMetricBool('DisallowNegativeInventory') OR itemsite_costmethod='A') "
              " AND (womatl_issuemethod IN ('S', 'M')) "
              " <? if exists('pickItemsOnly') ?> "
              " AND (womatl_picklist) "
              " <? endif ?> "
              " AND (womatl_wo_id=<? value('wo_id') ?>);");
  mqlissue.setQuery(sqlissue);
  issue = mqlissue.toQuery(params);
  if (issue.first() && ! issue.value("isqtyavail").toBool() &&
      QMessageBox::question(this, tr("Continue?"), tr("One or more items cannot be issued due to insufficient inventory. Issue all other items?"),
                               QMessageBox::No | QMessageBox::Default, QMessageBox::Yes) == QMessageBox::No)
        return;

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  QString sqlitems =
               ("SELECT womatl_id, womatl_wo_id, item_number, itemsite_id, formatWoNumber(womatl_wo_id) AS wo_number, "
                " CASE WHEN (womatl_qtyreq >= 0) THEN "
                "   roundQty(itemuomfractionalbyuom(item_id, womatl_uom_id), noNeg(womatl_qtyreq - womatl_qtyiss)) "
                " ELSE "
                "   roundQty(itemuomfractionalbyuom(item_id, womatl_uom_id), noNeg(womatl_qtyiss * -1)) "
                " END AS qty, "
                " isControlledItemsite(itemsite_id) AS controlled, "
                " roundQty(item_fractional, " // recreate _p.qty calculation from issueWoMaterial.sql
                "   itemuomtouom(item_id, womatl_uom_id, NULL, roundQty(itemuomfractionalbyuom( "
                "   item_id, womatl_uom_id), noNeg( "
                "   CASE WHEN (womatl_qtyreq >= 0) THEN womatl_qtyreq - womatl_qtyiss "
                "   ELSE womatl_qtyiss * -1 END)))) *-1 AS post_qty "
                "FROM womatl, itemsite, item "
                "WHERE((womatl_itemsite_id=itemsite_id) "
                " AND (itemsite_item_id=item_id) "
                " AND (womatl_issuemethod IN ('S', 'M')) "
                " <? if exists(\"pickItemsOnly\") ?> "
                " AND (womatl_picklist) "
                " <? endif ?> "
                " AND (womatl_wo_id=<? value(\"wo_id\") ?>)) "
                "ORDER BY womatl_id; ");
  MetaSQLQuery mqlitems(sqlitems);
  XSqlQuery items = mqlitems.toQuery(params);

  int succeeded = 0;
  QList<QString> failedItems;
  QList<QString> errors;
  while(items.next())
  { 
    // Stage distribution cleanup function to be called on error
    XSqlQuery cleanup;
    cleanup.prepare("SELECT deleteitemlocseries(:itemlocSeries, TRUE);");
    int itemlocSeries;

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
      failedItems.append(items.value("item_number").toString());
      errors.append("Failed to Retrieve the Next itemloc_series_seq");
      continue;
    }

    if (items.value("controlled").toBool())
    {
      // Create the parent itemlocdist record for each line item requiring distribution, call distributeInventory::seriesAdjust
      XSqlQuery parentItemlocdist;
      parentItemlocdist.prepare("SELECT createitemlocdistparent(:itemsite_id, :qty, 'WO', "
                                " :orderitemId, :itemlocSeries, NULL, NULL, 'IM');");
      parentItemlocdist.bindValue(":itemsite_id", items.value("itemsite_id").toInt());
      parentItemlocdist.bindValue(":qty", items.value("post_qty").toDouble());
      parentItemlocdist.bindValue(":orderitemId", items.value("womatl_id").toInt());
      parentItemlocdist.bindValue(":itemlocSeries", itemlocSeries);
      parentItemlocdist.exec();
      if (parentItemlocdist.first())
      {
        if (distributeInventory::SeriesAdjust(itemlocSeries, this, QString(), QDate(),
          QDate(), true) == XDialog::Rejected)
        {
          cleanup.exec();
          failedItems.append(items.value("item_number").toString());
          errors.append("Detail Distribution Cancelled");
          // If it's not the last item in the loop, ask the user to exit loop or continue
          if (items.at() != (items.size() -1))
          {
            if (QMessageBox::question(this,  tr("Material Issue"),
            tr("Posting distribution detail for item number %1 was cancelled but "
              "there are more items to issue. Continue issuing the remaining materials?")
            .arg(items.value("item_number").toString()),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
            {
              continue;
            }
            else
              break;
          }
          else
            continue;
        }
      }
      else
      {
        cleanup.exec();
        failedItems.append(items.value("item_number").toString());
        errors.append(tr("Error Creating itemlocdist Records. %1")
          .arg(parentItemlocdist.lastError().text()));
        continue;
      }
    }
  
    // Post inventory
    issue.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
    issue.prepare("SELECT issueWoMaterial(:womatl_id, :qty, :itemlocSeries, true, "
                  " :date, TRUE) AS result;");
    issue.bindValue(":womatl_id", items.value("womatl_id").toInt());
    issue.bindValue(":qty", items.value("qty").toDouble());
    issue.bindValue(":itemlocSeries", itemlocSeries);
    issue.bindValue(":date",  _transDate->date());
    issue.exec();
    if (issue.first())
    {
      int result = issue.value("result").toInt();
      if (result < 0 || result != itemlocSeries)
      {
        rollback.exec();
        cleanup.exec();
        failedItems.append(items.value("item_number").toString());
        errors.append(tr("Error Issuing Work Order Material; Work Order ID #%1. Database error: %2")
                             .arg(_wo->id()).arg(issue.lastError().text()));
        continue;
      }

      succeeded++;
      issue.exec("COMMIT;");      
    }
    else
    {
      rollback.exec();
      cleanup.exec();
      failedItems.append(items.value("item_number").toString());
      errors.append(issue.lastError().text());
    }
  }

  if (errors.size() > 0)
  {
    QMessageBox dlg(QMessageBox::Critical, "Errors Issuing Material", "", QMessageBox::Ok, this);
    dlg.setText(tr("%1 Items succeeded.\n%2 Items failed.").arg(succeeded).arg(failedItems.size()));

    QString details;
    for (int i=0; i<failedItems.size(); i++)
      details += tr("Item %1 failed with:\n%2\n").arg(failedItems[i]).arg(errors[i]);
    dlg.setDetailedText(details);

    dlg.exec();
  }

  if (succeeded == 0 && errors.size() == 0)
    QMessageBox::information( this, tr("Issue WO Material Batch"), tr("There is no Qty to Issue.") );

  omfgThis->sWorkOrdersUpdated(_wo->id(), true);

  if (_captive)
    accept();
  else
  {
    _wo->setId(-1);
    _womatl->clear();
    _wo->setFocus();
  }
}

void issueWoMaterialBatch::sFillList()
{
  _womatl->clear();
  _issue->setEnabled(false);

  if (_wo->isValid())
  {
    QTreeWidgetItem * hitem = _womatl->headerItem();
    if (_wo->method() == "A")
      hitem->setText(5, tr("Required"));
    else
      hitem->setText(5, tr("Returned"));
     
    XSqlQuery womatl;
    womatl.prepare( "SELECT womatl_id, item_number,"
                    " (item_descrip1 || ' ' || item_descrip2) AS itemdescrip, uom_name,"
                    " CASE WHEN (womatl_issuemethod = 'S') THEN :push"
                    "      WHEN (womatl_issuemethod = 'L') THEN :pull"
                    "      WHEN (womatl_issuemethod = 'M') THEN :mixed"
                    "      ELSE :error"
                    " END AS issuemethod,"
                    " womatl_picklist AS picklist,"
                    " CASE "
                    "   WHEN (womatl_qtyreq >= 0) THEN "
                    "     noNeg(womatl_qtyreq - womatl_qtyiss) "
                    "   ELSE "
                    "     (womatl_qtyiss * -1) "
                    "   END AS required,"
                    " qtyAvailable(itemsite_id) AS availableqoh,"
                    " abs(noneg((qtyAvailable(itemsite_id) - womatl_qtyreq) * -1)) AS short,"
                    " 'qty' AS required_xtnumericrole,"
                    " 'qty' AS availableqoh_xtnumericrole,"
                    " 'qty' AS short_xtnumericrole,"
                    " CASE WHEN (womatl_issuemethod = 'L') THEN 'blue'"
                    " END AS issuemethod_qtforegroundrole, "
                    " CASE WHEN (abs(noneg((qtyAvailable(itemsite_id) - womatl_qtyreq) * -1)) > 0.0) THEN 'red'"
                    " END AS short_qtforegroundrole "
                    "FROM womatl, itemsite, item, uom "
                    "WHERE ((womatl_itemsite_id=itemsite_id)"
                    " AND (itemsite_item_id=item_id)"
                    " AND (womatl_uom_id=uom_id)"
                    " AND (womatl_wo_id=:wo_id)) "
                    "ORDER BY item_number;" );
    womatl.bindValue(":push", tr("Push"));
    womatl.bindValue(":pull", tr("Pull"));
    womatl.bindValue(":mixed", tr("Mixed"));
    womatl.bindValue(":error", tr("Error"));
    womatl.bindValue(":wo_id", _wo->id());
    womatl.exec();
    _womatl->populate(womatl);
    _issue->setEnabled(true);
  }
}
