/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "issueLineToShipping.h"

#include <QApplication>
#include <QSqlError>
#include <QVariant>
#include <QValidator>

#include <metasql.h>

#include "xmessagebox.h"
#include "distributeInventory.h"
#include "storedProcErrorLookup.h"
#include "errorReporter.h"

#define DEBUG false

issueLineToShipping::issueLineToShipping(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_issue, SIGNAL(clicked()), this, SLOT(sIssue()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));

  _itemsiteId = 0;
  _controlled = false;
  _requireInventory = false;
  _snooze = false;
  _transTS = QDateTime::currentDateTime();
  _item->setReadOnly(true);
  if(!_metrics->boolean("EnableSOReservations"))
  {
    _qtyReservedLit->hide();
    _qtyReserved->hide();
  }

  _qtyToIssue->setValidator(omfgThis->qtyVal());
  _qtyOrdered->setPrecision(omfgThis->qtyVal());
  _qtyShipped->setPrecision(omfgThis->qtyVal());
  _qtyReturned->setPrecision(omfgThis->qtyVal());
  _qtyReserved->setPrecision(omfgThis->qtyVal());
  _balance->setPrecision(omfgThis->qtyVal());
  _qtyAtShip->setPrecision(omfgThis->qtyVal());
  
  adjustSize();
}

issueLineToShipping::~issueLineToShipping()
{
  // no need to delete child widgets, Qt does it all for us
}

void issueLineToShipping::languageChange()
{
  retranslateUi(this);
}

enum SetResponse issueLineToShipping::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("transTS", &valid);
  if (valid)
    _transTS = param.toDateTime();

  param = pParams.value("order_type", &valid);
  if (valid)
  {
    _ordertype = param.toString();
    if (_ordertype == "SO")
      _orderNumberLit->setText(tr("Sales Order #:"));
    else if (_ordertype == "TO")
      _orderNumberLit->setText(tr("Transfer Order #:"));
  }

  param = pParams.value("order_id", &valid);
  if (valid)
  {
    _itemid = param.toInt();
    populate();
  }

  // TODO: deprecate by remoing from salesOrder and transferOrder windows
  param = pParams.value("soitem_id", &valid);
  if (valid)
  {
    _itemid = param.toInt();
    _ordertype = "SO";
    _orderNumberLit->setText(tr("Sales Order #:"));
    populate();
  }

  // TODO: deprecate by remoing from salesOrder and transferOrder windows
  param = pParams.value("toitem_id", &valid);
  if (valid)
  {
    _itemid = param.toInt();
    _ordertype = "TO";
    _orderNumberLit->setText(tr("Transfer Order #:"));
    populate();
  }

  if (pParams.inList("requireInventory"))
    _requireInventory = true;

  param = pParams.value("qty", &valid);
  if (valid)
    _qtyToIssue->setDouble(param.toDouble());

  _snooze = pParams.inList("snooze");

  if(pParams.inList("issue"))
    sIssue();

  return NoError;
}

void issueLineToShipping::sIssue()
{
  XSqlQuery issueIssue;
  if (_qtyToIssue->toDouble() <= 0)
  {
    XMessageBox::message( (isVisible() ? this : parentWidget()), QMessageBox::Warning, tr("Invalid Quantity to Issue to Shipping"),
                          tr(  "<p>Please enter a non-negative, non-zero value to indicate the amount "
                               "of Stock you wish to Issue to Shipping for this Order Line." ),
                          QString::null, QString::null, _snooze );
    _qtyToIssue->setFocus();
    return;
  }

  if(_requireInventory || ("SO" == _ordertype && _metrics->boolean("EnableSOReservations")))
  {
    issueIssue.prepare("SELECT sufficientInventoryToShipItem(:ordertype, :orderitemid, :orderqty) AS result;");
    issueIssue.bindValue(":ordertype",   _ordertype);
    issueIssue.bindValue(":orderitemid", _itemid);
    issueIssue.bindValue(":orderqty",  _qtyToIssue->toDouble());
    issueIssue.exec();
    if (issueIssue.first())
    {
      int result = issueIssue.value("result").toInt();
      if (result < 0)
      {
        ParameterList errp;
        if (_ordertype == "SO")
          errp.append("soitem_id", _itemid);
        else if (_ordertype == "TO")
          errp.append("toitem_id", _itemid);

        QString errs = "<? if exists(\"soitem_id\") ?>"
            "SELECT item_number, warehous_code "
            "  FROM coitem, item, itemsite, whsinfo "
            " WHERE ((coitem_itemsite_id=itemsite_id)"
            "   AND  (itemsite_item_id=item_id)"
            "   AND  (itemsite_warehous_id=warehous_id)"
            "   AND  (coitem_id=<? value(\"soitem_id\") ?>));"
            "<? elseif exists(\"toitem_id\")?>"
            "SELECT item_number, tohead_srcname AS warehous_code "
            "  FROM toitem, tohead, item "
            " WHERE ((toitem_item_id=item_id)"
            "   AND  (toitem_tohead_id=tohead_id)"
            "   AND  (toitem_id=<? value(\"toitem_id\") ?>));"
            "<? endif ?>" ;
        MetaSQLQuery errm(errs);
        issueIssue = errm.toQuery(errp);
        if (! issueIssue.first() && issueIssue.lastError().type() != QSqlError::NoError)
            ErrorReporter::error(QtCriticalMsg, this, tr("Error Issuing Item"),
                                 issueIssue, __FILE__, __LINE__);
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Issuing Item"),
                                 storedProcErrorLookup("sufficientInventoryToShipItem", result)
                                 .arg(issueIssue.value("item_number").toString())
                                 .arg(issueIssue.value("warehous_code").toString()),
                                 __FILE__, __LINE__);
        return;
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Issuing Item"),
                                  issueIssue, __FILE__, __LINE__))
    {
      return;
    }
  }

  // check to see if we are over issuing
  ParameterList params;
  if (_ordertype == "SO")
    params.append("soitem_id", _itemid);
  else if (_ordertype == "TO")
    params.append("toitem_id", _itemid);
  params.append("qty", _qtyToIssue->toDouble());

  QString sql = "<? if exists(\"soitem_id\") ?>"
                "SELECT  itemsite_costmethod, "
                "  (<? value(\"qty\") ?> * coitem_qty_invuomratio) AS issuelineqty, "
                "  roundQty(item_fractional, (<? value(\"qty\") ?> * coitem_qty_invuomratio)) AS postprodqty, "
                "  (noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned) <"
                "           (COALESCE(SUM(shipitem_qty), 0) + <? value(\"qty\") ?>)) AS overship"
                "  FROM coitem LEFT OUTER JOIN"
                "        ( shipitem JOIN shiphead"
                "          ON ( (shipitem_shiphead_id=shiphead_id) AND (NOT shiphead_shipped) )"
                "        ) ON  (shipitem_orderitem_id=coitem_id)"
                "  JOIN itemsite ON (coitem_itemsite_id=itemsite_id) "
                "  JOIN item ON itemsite_item_id = item_id "
                " WHERE (coitem_id=<? value(\"soitem_id\") ?>)"
                " GROUP BY coitem_qtyord, coitem_qtyshipped, coitem_qtyreturned, coitem_qty_invuomratio, item_fractional, "
                "   itemsite_costmethod, itemsite_controlmethod;"
                "<? elseif exists(\"toitem_id\") ?>"
                "SELECT false AS postprod,"
                "  <? value(\"qty\") ?> AS issuelineqty, "
                "  (noNeg(toitem_qty_ordered - toitem_qty_shipped) <"
                "           (COALESCE(SUM(shipitem_qty), 0) + <? value(\"qty\") ?>)) AS overship, "
                "  NULL AS itemsite_costmethod "
                "  FROM toitem LEFT OUTER JOIN"
                "        ( shipitem JOIN shiphead"
                "          ON ( (shipitem_shiphead_id=shiphead_id) AND (NOT shiphead_shipped) )"
                "        ) ON  (shipitem_orderitem_id=toitem_id)"
                " WHERE (toitem_id=<? value(\"toitem_id\") ?>)"
                " GROUP BY toitem_qty_ordered, toitem_qty_shipped;"
                "<? endif ?>";
  MetaSQLQuery mql(sql);
  issueIssue = mql.toQuery(params);
  if (issueIssue.next() && issueIssue.value("overship").toBool())
  {
    if(XMessageBox::message( (isVisible() ? this : parentWidget()) , QMessageBox::Question, tr("Inventory Overshipped"),
        tr("<p>You have selected to ship more inventory than required. Do you want to continue?"),
        tr("Yes"), tr("No"), _snooze, 0, 1) == 1)
      return;
  }
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Issuing Item"),
                                issueIssue, __FILE__, __LINE__))
  {
    return;
  }


  
  int invhistid = 0;
  int itemlocSeries;
  int postProdItemlocSeries = -1;
  int postProdItemlocdistId = 0;

  XSqlQuery parentItemlocdist;
  XSqlQuery parentSeries;
  XSqlQuery issue;
  XSqlQuery cleanup;
  XSqlQuery postProdCleanup;
  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  // Stage cleanup function to be called on error
  cleanup.prepare("SELECT deleteitemlocseries(:itemlocSeries, TRUE);");
  postProdCleanup.prepare("SELECT deleteitemlocseries(:itemlocSeries, TRUE);");

  qDebug() << "issueLineToShipping _ordertype: " << _ordertype << " itemsite_costmethod: " << issueIssue.value("itemsite_costmethod").toString();
  qDebug() << "issueLineToShipping issuelineqty: << " << issueIssue.value("issuelineqty").toDouble();
  qDebug() << "issueLineToShipping postprodqty: << " << issueIssue.value("postprodqty").toDouble();

  // If this is a lot/serial controlled job item, we need to post production first
  if (_ordertype == "SO" && 
    issueIssue.value("itemsite_costmethod").toString() == "J" && 
    _qtyToIssue->toDouble() > 0)
  {
    bool hasControlledBackflushItems = false;
    // Series for postSoItemProduction and backflush (issueMaterial) items
    parentSeries.prepare("SELECT NEXTVAL('itemloc_series_seq') AS result;");
    parentSeries.exec();
    if (parentSeries.first() && parentSeries.value("result").toInt() > 0)
    {
      postProdItemlocSeries = parentSeries.value("result").toInt();
      postProdCleanup.bindValue(":itemlocSeries", postProdItemlocSeries);
    }
    else
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Failed to Retrieve the Next itemloc_series_seq"),
        parentSeries, __FILE__, __LINE__);
      return;
    }

    // Controlled backflush items that require issueMaterial through postSoItemProduction (postProduction).
    // Create an itemlocdist record for each.
    XSqlQuery backflushItems;
    backflushItems.prepare(
      "SELECT item_number, itemsite_id, itemsite_item_id, womatl_id, "
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
    backflushItems.bindValue(":qty", issueIssue.value("postprodqty").toDouble());
    backflushItems.bindValue(":coitem_id", _itemid);
    backflushItems.exec();
    while (backflushItems.next())
    {
      qDebug() << "backflushItems qtytoissue: " << backflushItems.value("qtyToIssue").toDouble();
      if (backflushItems.value("qtyToIssue").toDouble() > 0)
      {
        hasControlledBackflushItems = true;
        parentItemlocdist.prepare("SELECT createItemlocdistParent(:itemsite_id, itemuomtouom(:item_id, womatl_uom_id, NULL, :qty) * -1, 'WO', :orderitem_id, "
                                  " :itemlocSeries, NULL, NULL, 'IM') AS result "
                                  "FROM womatl "
                                  "WHERE womatl_id = :womatl_id;");
        parentItemlocdist.bindValue(":itemsite_id", backflushItems.value("itemsite_id").toInt());
        parentItemlocdist.bindValue(":item_id", backflushItems.value("itemsite_item_id").toInt());
        parentItemlocdist.bindValue(":womatl_id", backflushItems.value("womatl_id").toInt());
        parentItemlocdist.bindValue(":qty", backflushItems.value("qtyToIssue").toDouble());
        // TODO - replace this with wo_id so that createLotSerial can reproduce the (now null) invhist_ordnumber 
        parentItemlocdist.bindValue(":orderitem_id", _itemid);
        parentItemlocdist.bindValue(":itemlocSeries", postProdItemlocSeries);
        parentItemlocdist.exec();
        if (!parentItemlocdist.first())
        {
          postProdCleanup.exec();
          QMessageBox::information( this, tr("Issue Line to Shipping"), 
            tr("Failed to Create an itemlocdist record for work order material item %1 to be backflushed")
            .arg(backflushItems.value("item_number").toString()) );
          return;
        }
        else 
        {
          QMessageBox::information( this, tr("Issue to Shipping"), 
            tr("Failed to create the itemlocdist record for backflush work order material item %1")
            .arg(backflushItems.value("item_number").toString()) );
          postProdCleanup.exec();
          return;
        }
      }
    }

    // If Job item is controlled, create the itemlocdist record for postProduction transaction
    if (_controlled)
    {
      // Create the post production itemlocdist parent
      parentItemlocdist.prepare("SELECT createItemlocdistParent(:itemsite_id, :qty, "
                                "'WO', :orderitem_id, :itemlocSeries, NULL, NULL, 'RM') AS result;");
      parentItemlocdist.bindValue(":itemsite_id", _itemsiteId);
      parentItemlocdist.bindValue(":qty", issueIssue.value("issuelineqty").toDouble());
      // TODO - replace this with wo_id so that createLotSerial can reproduce the (now null) invhist_ordnumber 
      parentItemlocdist.bindValue(":orderitem_id", _itemid);
      parentItemlocdist.bindValue(":itemlocSeries", postProdItemlocSeries);
      parentItemlocdist.exec();
      if (parentItemlocdist.first())
      {
        postProdItemlocdistId = parentItemlocdist.value("result").toInt();
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating itemlocdist Records"),
                                parentItemlocdist, __FILE__, __LINE__))
      {
        postProdCleanup.exec();
        return;
      }
    }

    if (DEBUG)
      qDebug() << "issueLineToShipping job item _controlled: " << _controlled << " hasControlledBackflushItems: " << hasControlledBackflushItems;

    // Distribute detail for post production and any controlled backflush items
    if (_controlled || hasControlledBackflushItems)
    {
      if (distributeInventory::SeriesAdjust(postProdItemlocSeries, this, QString(), QDate(),
        QDate(), true) == XDialog::Rejected)
      {
        postProdCleanup.exec();
        QMessageBox::information( this, tr("Issue to Shipping"), tr("Issue Canceled") );
        return;
      }
      // does postDistDetail(postProdItemlocSeries) need to occur before the below createItemlocdistParent so that the itemlocdist records from the first are deleted from itemlocdist?
    }
  } // job item

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
    if (postProdItemlocSeries > 0)
      postProdCleanup.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Failed to Retrieve the Next itemloc_series_seq"),
      parentSeries, __FILE__, __LINE__);
    return;
  }

  // Before issueToShipping, if controlled item: create the parent itemlocdist record, call distributeInventory::seriesAdjust
  if (_controlled)
  {
    parentItemlocdist.prepare("SELECT createItemlocdistParent(:itemsite_id, :qty, :orderType, :orderitemId, "
      ":itemlocSeries, NULL, :postProdItemlocdistId, 'SH', :postProdItemlocSeries) AS result;");
    parentItemlocdist.bindValue(":itemsite_id", _itemsiteId);
    parentItemlocdist.bindValue(":qty", issueIssue.value("issuelineqty").toDouble());
    parentItemlocdist.bindValue(":orderType", _ordertype);
    parentItemlocdist.bindValue(":orderitemId", _itemid);
    parentItemlocdist.bindValue(":itemlocSeries", itemlocSeries);
    if (postProdItemlocdistId > 0)
      parentItemlocdist.bindValue(":postProdItemlocdistId", postProdItemlocdistId);
    if (postProdItemlocSeries > 0)
      parentItemlocdist.bindValue(":postProdItemlocSeries", postProdItemlocSeries);
    parentItemlocdist.exec();
    if (parentItemlocdist.first())
    {
      // Distribute inventory for controlled item being issued to shipping
      if (distributeInventory::SeriesAdjust(itemlocSeries, this, QString(), QDate(), QDate(), true)
        == XDialog::Rejected)
      {
        cleanup.exec();
        if (postProdItemlocSeries > 0)
          postProdCleanup.exec();
        QMessageBox::information( this, tr("Issue to Shipping"), tr("Issue Canceled") );
        return;
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating itemlocdist Records"),
                              parentItemlocdist, __FILE__, __LINE__))
    {
      cleanup.exec();
      if (postProdItemlocSeries > 0)
        postProdCleanup.exec();
      return;
    }
  }
  
  // Wrap remaining sql in a transaction block - perform postSoItemProduction if Job item, then issue to shipping
  issue.exec("BEGIN;");

  // postSoItemProduction
  if (_ordertype == "SO" && 
    issueIssue.value("itemsite_costmethod").toString() == "J" && 
    _qtyToIssue->toDouble() > 0)
  {
    qDebug() << "issueLineToShipping postSoItemProduction";
    XSqlQuery prod;
    prod.prepare("SELECT postSoItemProduction(:soitem_id, :qty, :ts, :itemlocSeries, TRUE) AS result;");
    prod.bindValue(":soitem_id", _itemid);
    prod.bindValue(":qty", _qtyToIssue->toDouble());
    prod.bindValue(":ts", _transTS);
    prod.bindValue(":itemlocSeries", postProdItemlocSeries);
    prod.exec();
    if (prod.first())
    {
      int result = prod.value("result").toInt();

      if (result < 0 || result != postProdItemlocSeries)
      {
        rollback.exec();
        cleanup.exec();
        postProdCleanup.exec();
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Production"),
                               storedProcErrorLookup("postProduction", postProdItemlocSeries),
                               __FILE__, __LINE__);
        return;
      }

      // Need to get the inventory history id so we can auto reverse the distribution when issuing
      prod.prepare("SELECT invhist_id "
                "FROM invhist "
                "WHERE ((invhist_series = :itemlocseries) "
                " AND (invhist_transtype = 'RM')); ");
      prod.bindValue(":itemlocseries" , postProdItemlocSeries);
      prod.exec();
      if (prod.first())
        invhistid = prod.value("invhist_id").toInt();
      else
      {
        rollback.exec();
        cleanup.exec();
        postProdCleanup.exec();
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Occurred"),
                             tr("Inventory history not found")
                             .arg(windowTitle()),__FILE__,__LINE__);
        return;
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Production for Job Item"),
      prod, __FILE__,__LINE__))
      return;
  }

  // issue to shipping
  issue.prepare("SELECT issueToShipping(:ordertype, :lineitem_id, :qty, :itemlocseries, :ts, :invhist_id, false, true) AS result;");
  issue.bindValue(":ordertype",   _ordertype);
  issue.bindValue(":lineitem_id", _itemid);
  issue.bindValue(":qty",         _qtyToIssue->toDouble());
  issue.bindValue(":itemlocseries", itemlocSeries);
  issue.bindValue(":ts",          _transTS);
  if (invhistid)
    issue.bindValue(":invhist_id", invhistid);
  issue.exec();

  if (issue.first())
  {
    int result = issue.value("result").toInt();
    if (result < 0 || result != itemlocSeries)
    {
      rollback.exec();
      cleanup.exec();
      if (postProdItemlocSeries > 0)
        postProdCleanup.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Issuing Item"),
                             storedProcErrorLookup("issueToShipping", result),
                             __FILE__, __LINE__);
      return;
    }
    else
    {
      // TODO - move the below into issueToShipping function?

      // If Transfer Order then insert special pre-assign records for the lot/serial#
      // so they are available when the Transfer Order is received
      if (_ordertype == "TO")
      {
        XSqlQuery lsdetail;
        lsdetail.prepare("INSERT INTO lsdetail "
                       "            (lsdetail_itemsite_id, lsdetail_created, lsdetail_source_type, "
    	  			     "             lsdetail_source_id, lsdetail_source_number, lsdetail_ls_id, lsdetail_qtytoassign, "
                                             "             lsdetail_expiration, lsdetail_warrpurc ) "
    			     "SELECT invhist_itemsite_id, NOW(), 'TR', "
    			     "       :orderitemid, invhist_ordnumber, invdetail_ls_id, (invdetail_qty * -1.0), "
                                             "       invdetail_expiration, invdetail_warrpurc "
    			     "FROM invhist JOIN invdetail ON (invdetail_invhist_id=invhist_id) "
    			     "WHERE (invhist_series=:itemlocseries);");
        lsdetail.bindValue(":orderitemid", _itemid);
        lsdetail.bindValue(":itemlocseries", itemlocSeries);
        lsdetail.exec();
        if (lsdetail.lastError().type() != QSqlError::NoError)
        {
          rollback.exec();
          cleanup.exec();
          if (postProdItemlocSeries > 0)
            postProdCleanup.exec();
          ErrorReporter::error(QtCriticalMsg, this, tr("Error Issuing Item"),
                               lsdetail, __FILE__, __LINE__);
          return;
        }
      }

      issue.exec("COMMIT;");
      accept();
    }
  }
  else if (issue.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    cleanup.exec();
    if (postProdItemlocSeries > 0)
      postProdCleanup.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Issuing Item"),
                         issue, __FILE__, __LINE__);
    return;
  }
}

void issueLineToShipping::populate()
{
  ParameterList itemp;
  if (_ordertype == "SO")
    itemp.append("soitem_id", _itemid);
  else if (_ordertype == "TO")
    itemp.append("toitem_id", _itemid);
  itemp.append("ordertype", _ordertype);

  // TODO: make this an orderitem select
  QString sql = "<? if exists(\"soitem_id\") ?>"
		"SELECT cohead_number AS order_number,"
		"       itemsite_item_id AS item_id,"
		"       warehous_code, uom_name,"
		"       coitem_qtyord AS qtyordered,"
    "       coitem_qtyshipped AS qtyshipped,"
    "       coitem_qtyreturned AS qtyreturned,"
    "       (coitem_qtyreserved / coitem_qty_invuomratio) AS qtyreserved,"
		"       noNeg(coitem_qtyord - coitem_qtyshipped +"
		"             coitem_qtyreturned) AS balance, "
    "       isControlledItemsite(itemsite_id) AS controlled, itemsite_id "
        "FROM cohead, coitem, itemsite, item, whsinfo, uom "
		"WHERE ((coitem_cohead_id=cohead_id)"
		"  AND  (coitem_itemsite_id=itemsite_id)"
		"  AND  (coitem_status <> 'X')"
                "  AND  (coitem_qty_uom_id=uom_id)"
		"  AND  (itemsite_item_id=item_id)"
		"  AND  (itemsite_warehous_id=warehous_id)"
		"  AND  (coitem_id=<? value(\"soitem_id\") ?>) );"
		"<? elseif exists(\"toitem_id\") ?>"
		"SELECT tohead_number AS order_number,"
		"       toitem_item_id AS item_id,"
		"       warehous_code, toitem_uom AS uom_name,"
		"       toitem_qty_ordered AS qtyordered,"
		"       toitem_qty_shipped AS qtyshipped,"
		"       0 AS qtyreturned,"
    "       0 AS qtyreserved,"
		"       noNeg(toitem_qty_ordered -"
		"             toitem_qty_shipped) AS balance, "
    "       isControlledItemsite(itemsite_id) AS controlled, itemsite_id "
        "FROM tohead, toitem, whsinfo, itemsite "
		"WHERE ((toitem_tohead_id=tohead_id)"
		"  AND  (toitem_status <> 'X')"
		"  AND  (tohead_src_warehous_id=warehous_id)"
    "  AND  (itemsite_item_id = toitem_item_id AND itemsite_warehous_id = warehous_id) "
		"  AND  (toitem_id=<? value(\"toitem_id\") ?>) );"
		"<? endif ?>";

  MetaSQLQuery itemm(sql);
  XSqlQuery itemq = itemm.toQuery(itemp);

  if (itemq.first())
  {
    _orderNumber->setText(itemq.value("order_number").toString());
    _item->setId(itemq.value("item_id").toInt());
    _warehouse->setText(itemq.value("warehous_code").toString());
    _shippingUOM->setText(itemq.value("uom_name").toString());
    _qtyOrdered->setDouble(itemq.value("qtyordered").toDouble());
    _qtyShipped->setDouble(itemq.value("qtyshipped").toDouble());
    _qtyReturned->setDouble(itemq.value("qtyreturned").toDouble());
    _qtyReserved->setDouble(itemq.value("qtyreserved").toDouble());
    _balance->setDouble(itemq.value("balance").toDouble());

    _itemsiteId = itemq.value("itemsite_id").toInt();
    _controlled = itemq.value("controlled").toBool();
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Item Information"),
                                itemq, __FILE__, __LINE__))
  {
    return;
  }

  ParameterList shipp;
  shipp.append("ordertype", _ordertype);
  shipp.append("orderitem_id", _itemid);

  sql = "SELECT shiphead_id AS misc_id,"
        "       SUM(shipitem_qty) AS qtyatship "
        "FROM shiphead, shipitem "
        "WHERE ((shipitem_shiphead_id=shiphead_id)"
        "  AND  (NOT shiphead_shipped)"
        "  AND  (shiphead_order_type=<? value(\"ordertype\") ?>)"
        "  AND  (shipitem_orderitem_id=<? value(\"orderitem_id\") ?>) ) "
        "GROUP BY shiphead_id;" ;

  MetaSQLQuery shipm(sql);
  XSqlQuery shipq = shipm.toQuery(shipp);

  if (shipq.first())
  {
    _shipment->setType(_ordertype);
    _shipment->setId(shipq.value("misc_id").toInt());
    _qtyAtShip->setDouble(shipq.value("qtyatship").toDouble());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Item Information"),
                                shipq, __FILE__, __LINE__))
  {
    return;
  }
  
  if (_item->isFractional())
    _qtyToIssue->setValidator(omfgThis->transQtyVal());
  else
    _qtyToIssue->setValidator(new QIntValidator(this));

  if (_qtyAtShip->toDouble() == 0.0)
  {
    if (itemq.value("qtyreserved").toDouble() > 0.0)
      _qtyToIssue->setDouble(itemq.value("qtyreserved").toDouble());
    else
      _qtyToIssue->setDouble(itemq.value("balance").toDouble());
  }
}
