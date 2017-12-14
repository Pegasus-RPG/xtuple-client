/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "workOrder.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include <metasql.h>

#include "changeWoQty.h"
#include "closeWo.h"
#include "correctProductionPosting.h"
#include "dspInventoryAvailabilityByWorkOrder.h"
#include "dspRunningAvailability.h"
#include "dspInventoryAvailability.h"
#include "dspSubstituteAvailabilityByItem.h"
#include "distributeInventory.h"
#include "explodeWo.h"
#include "itemCharacteristicDelegate.h"
#include "implodeWo.h"
#include "inputManager.h"
#include "issueWoMaterialItem.h"
#include "mqlutil.h"
#include "postProduction.h"
#include "printWoTraveler.h"
#include "printWoTraveler.h"
#include "returnWoMaterialBatch.h"
#include "returnWoMaterialItem.h"
#include "reprioritizeWo.h"
#include "rescheduleWo.h"
#include "returnWoMaterialItem.h"
#include "storedProcErrorLookup.h"
#include "substituteList.h"
#include "scrapWoMaterialFromWIP.h"
#include "woMaterialItem.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"

#define DEBUG false

static QVariant _booEnabled = QVariant();

workOrder::workOrder(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  XSqlQuery workOrder;
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSaveClicked()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sPopulateLeadTime(int)));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sPopulateItemChar(int)));
  connect(_dueDate, SIGNAL(newDate(const QDate&)), this, SLOT(sUpdateStartDate()));
  connect(_leadTime, SIGNAL(valueChanged(int)), this, SLOT(sUpdateStartDate()));
  connect(_assembly, SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
  connect(_showMaterials, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_showOperations, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_indented, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_woIndentedList, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem*)));
  connect(_woNumber, SIGNAL(textEdited(QString)), this, SLOT(sNumberChanged()));

  _bomRevision->setMode(RevisionLineEdit::Use);
  _bomRevision->setType("BOM");
  _booRevision->setMode(RevisionLineEdit::Use);
  _booRevision->setType("BOO");

  _captive = false;
  _planordid = -1;
  _woid = -1;
  _sense = 1;
  _wonumber = -1;
  _oldPriority = _priority->value();

  _lastWarehousid = -1;
  _lastItemid = -1;
  _comments->setReadOnly(true);
//  _documents->setReadOnly(true);
  _woNumber->setValidator(omfgThis->orderVal());
  _qty->setValidator(omfgThis->qtyVal());
  _qtyReceived->setPrecision(omfgThis->qtyVal());
  _postedValue->setPrecision(omfgThis->costVal());
  _wipValue->setPrecision(omfgThis->costVal());
  _rcvdValue->setPrecision(omfgThis->costVal());

  _printTraveler->setEnabled(_privileges->check("PrintWorkOrderPaperWork"));

  if (_metrics->value("WONumberGeneration") == "A")
    _woNumber->setFocusPolicy(Qt::NoFocus);

  _project->setType(ProjectLineEdit::WorkOrder);
  if(!_metrics->boolean("UseProjects"))
  {
    _projectLit->hide();
    _project->hide();
  }

  _itemchar = new QStandardItemModel(0, 2, this);
  _itemchar->setHeaderData( 0, Qt::Horizontal, tr("Name"), Qt::DisplayRole);
  _itemchar->setHeaderData( 1, Qt::Horizontal, tr("Value"), Qt::DisplayRole);

  _itemcharView->setModel(_itemchar);
  ItemCharacteristicDelegate * delegate = new ItemCharacteristicDelegate(this);
  _itemcharView->setItemDelegate(delegate);

  _wocharView->setType("W");

  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }

  if (_booEnabled.isNull())
  {
    XSqlQuery boohead("SELECT 1 "
                      "  FROM pg_class "
                      " WHERE relname='boohead' "
                      "   AND relkind='r';");

    if (boohead.first())
      _booEnabled = QVariant(true);
    else
    {
      _booEnabled = QVariant(false);
      ErrorReporter::error(QtCriticalMsg, this, tr("Error checking for boohead"),
                           boohead, __FILE__, __LINE__);
    }
  }

  if (!_booEnabled.toBool())
  {
   _booGroup->hide();
   _showOperations->setChecked(false);
   _showOperations->hide();
  }

  if (!_metrics->boolean("RevControl"))
   _tabs->removeTab(_tabs->indexOf(_revision));
   
  if (!_privileges->boolean("ViewCosts"))
  {
   //_tabs->removeTab(_tabs->indexOf(_costing));
   _costGroup->hide();
  }

  if (_metrics->value("JobItemCosDefault") == "P")
    _proportional->setChecked(true);

  _woIndentedList->addColumn(tr("Order#"),          _orderColumn,   Qt::AlignLeft      , true,   "wonumber");
  _woIndentedList->addColumn(tr("Item#"),           _itemColumn,    Qt::AlignLeft      , true,   "wodata_itemnumber" );
  _woIndentedList->addColumn(tr("Description"),      -1,            Qt::AlignLeft      , true,   "wodata_descrip");
  _woIndentedList->addColumn(tr("Status"),          _statusColumn,  Qt::AlignCenter    , true,   "wodata_status");
  _woIndentedList->addColumn(tr("Qty Per."),        _qtyColumn,     Qt::AlignRight     , false,  "qtyper");
  _woIndentedList->addColumn(tr("Ord/Req."),        _qtyColumn,     Qt::AlignRight     , true,   "qtyordreq");
  _woIndentedList->addColumn(tr("UOM"),             _uomColumn,     Qt::AlignLeft      , true,   "wodata_qtyuom");
  _woIndentedList->addColumn(tr("Issued"),          _qtyColumn,     Qt::AlignRight     , true,   "qtyiss");
  _woIndentedList->addColumn(tr("Scrap"),           _qtyColumn,     Qt::AlignRight     , false,  "scrap");
  _woIndentedList->addColumn(tr("Received"),        _qtyColumn,     Qt::AlignRight     , true,   "qtyrcv");
  _woIndentedList->addColumn(tr("Available QOH"),   _qtyColumn,     Qt::AlignRight     , false,  "qoh");
  _woIndentedList->addColumn(tr("Short"),           _qtyColumn,     Qt::AlignRight     , false,  "short");
  if (_booEnabled.toBool())
  {
    _woIndentedList->addColumn(tr("Setup Remain."),           _qtyColumn,     Qt::AlignRight     , false,  "wodata_setup");
    _woIndentedList->addColumn(tr("Run Remain."),             _qtyColumn,     Qt::AlignRight     , false,  "wodata_run");
  }
  _woIndentedList->addColumn(tr("Start Date"),      _dateColumn,    Qt::AlignCenter    , false,  "wodata_startdate");
  _woIndentedList->addColumn(tr("Due Date"),        _dateColumn,    Qt::AlignCenter    , true,   "wodata_duedate");
  _woIndentedList->addColumn(tr("Reference"),        100,           Qt::AlignLeft      , false,  "wodata_ref");
  _woIndentedList->addColumn(tr("Notes"),            100,           Qt::AlignLeft      , false,  "wodata_notes");
}

workOrder::~workOrder()
{
    // no need to delete child widgets, Qt does it all for us
}

void workOrder::languageChange()
{
    retranslateUi(this);
}

enum SetResponse workOrder::set(const ParameterList &pParams)
{
  XSqlQuery setWork;
  XWidget::set(pParams);
  _captive = true;

  QVariant param;
  bool     valid;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _item->setItemsiteid(param.toInt());
    _item->setEnabled(false);
    _warehouse->setEnabled(false);
  }

  param = pParams.value("qty", &valid);
  if (valid)
    _qty->setText(formatQty(param.toDouble()));

  param = pParams.value("dueDate", &valid);
  if (valid)
  {
    _dueDate->setDate(param.toDate());
  }

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _woid = param.toInt();
    _wocharView->setId(_woid);
    emit newId(_woid);
  }

  param = pParams.value("planord_id", &valid);
  if (valid)
    _planordid = param.toInt();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      emit newMode(_mode);

      _item->setType(ItemLineEdit::cGeneralPurchased | ItemLineEdit::cGeneralManufactured | ItemLineEdit::cActive);
      _item->setDefaultType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cActive);
      _qtyReceivedLit->clear();
      _assembly->setEnabled(true);
      _disassembly->setEnabled(true);

      populateWoNumber();
      
      connect(_woNumber, SIGNAL(editingFinished()), this, SLOT(sCreate()));
      connect(_item, SIGNAL(privateIdChanged(int)), this, SLOT(sCreate()));
      connect(_qty, SIGNAL(editingFinished()), this, SLOT(sCreate()));
      connect(_dueDate, SIGNAL(newDate(const QDate&)), this, SLOT(sCreate()));
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      emit newMode(_mode);

      _item->setType(ItemLineEdit::cGeneralPurchased | ItemLineEdit::cGeneralManufactured |
                         ItemLineEdit::cActive);
                         
      populate();
      connect(_priority, SIGNAL(valueChanged(int)), this, SLOT(sReprioritizeParent()));
      connect(_qty, SIGNAL(editingFinished()), this, SLOT(sChangeParentQty()));
      connect(_startDate, SIGNAL(newDate(const QDate&)), this, SLOT(sRescheduleParent()));
      connect(_dueDate, SIGNAL(newDate(const QDate&)), this, SLOT(sRescheduleParent()));
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      emit newMode(_mode);

      _item->setQuery("SELECT DISTINCT item_id, item_number, item_descrip1, item_descrip2,"
                      "                (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
                      "       item_active, item_config, item_type, uom_name "
                      "FROM item JOIN uom ON (item_inv_uom_id=uom_id) ");
      populate();

      _woNumber->setEnabled(false);
      _item->setReadOnly(true);
      _bomRevision->setEnabled(false);
      _booRevision->setEnabled(false);
      _warehouse->setEnabled(false);
      _priority->setEnabled(false);
      _qty->setEnabled(false);
      _startDate->setEnabled(false);
      _dueDate->setEnabled(false);
      _productionNotes->setReadOnly(true);
      _save->hide();
      _leadTimeLit->hide();
      _leadTime->hide();
      _daysLit->hide();
      _printTraveler->hide();
      _bottomSpacer->hide();
      _close->setText(tr("&Close"));
      _project->setEnabled(false);
      _itemcharView->setEnabled(false);
      _wocharView->setEnabled(false);
      _jobCosGroup->setEnabled(false);
    }

    else if (param.toString() == "release")
    {
      _mode = cRelease;
      emit newMode(_mode);
      //_tabs->removePage(_tabs->page(3));

      setWork.prepare( "SELECT planord_itemsite_id, planord_duedate,"
                 "       CASE WHEN(planord_mps) THEN 'P'"
                 "            ELSE 'M'"
                 "       END AS sourcetype,"
                 "       CASE WHEN(planord_mps) THEN planord_pschitem_id"
                 "            ELSE planord_id"
                 "       END AS sourceid,"
                 "       formatQty(planord_qty) AS qty "
                 "FROM planord "
                 "WHERE (planord_id=:planord_id);" );
      setWork.bindValue(":planord_id", _planordid);
      setWork.exec();
      if (setWork.first())
      {
        _item->setReadOnly(true);
        _warehouse->setEnabled(false);

        _planordtype=setWork.value("sourcetype").toString();
        _sourceid=setWork.value("sourceid").toInt();
        _qty->setText(setWork.value("qty").toString());
        _dueDate->blockSignals(true);
        _dueDate->setDate(setWork.value("planord_duedate").toDate());
        _dueDate->blockSignals(false);
        _item->setItemsiteid(setWork.value("planord_itemsite_id").toInt());

        sUpdateStartDate();
        populateWoNumber();

        _qty->setEnabled(false);
        _qtyReceivedLit->clear();
        _startDate->setEnabled(false);
        _dueDate->setEnabled(false);
        _wipValueLit->hide();
        _wipValue->hide();
        _leadTimeLit->hide();
        _leadTime->hide();
        _daysLit->hide();

        sCreate();
      }
      else
      {
        ErrorReporter::error(QtCriticalMsg, this, tr("A System Error Occured, Planned Order ID %1")
                             .arg(_planordid),
                             setWork, __FILE__, __LINE__);
        close();
      }
     }
   }

  if (_mode == cNew)
  {
    param = pParams.value("prj_id", &valid);
    if (valid)
      _project->setId(param.toInt());
  }

  return NoError;
}

int workOrder::id() const
{
  return _woid;
}

/** \return one of cNew, cEdit, cView, ...
 \todo   change possible modes to an enum in guiclient.h (and add cUnknown?)
 */
int workOrder::mode() const
{
  return _mode;
}

void workOrder::sCreate()
{
  XSqlQuery workCreate;
  if (_woNumber->text().length() &&
      _item->isValid() &&
      _warehouse->id() != -1 &&
      _qty->text().length() &&
      _dueDate->isValid())
  {
    workCreate.prepare( "SELECT itemsite_id, itemsite_costmethod "
               "FROM itemsite "
               "WHERE ( (itemsite_item_id=:item_id)"
               " AND (itemsite_warehous_id=:warehous_id) );" );
    workCreate.bindValue(":item_id", _item->id());
    workCreate.bindValue(":warehous_id", _warehouse->id());
    workCreate.exec();
    if (!workCreate.first())
    {
      QMessageBox::warning(this, tr("Invalid Site"),
          tr("<p>The selected Site for this Work Order is not "
             "a \"Supplied At\" Site. You must select a different "
             "Site before creating the Work Order.") );
      return;
    }
    else
    {
      if (DEBUG)
      qDebug("cost %s", qPrintable(workCreate.value("itemsite_costmethod").toString()));
      if (workCreate.value("itemsite_costmethod").toString() == "J")
      {
        QMessageBox::critical(this,tr("Invalid Item"),
                              tr("Item %1 is set to Job Costing on Item Site %2.  "
                                 "Work Orders for Job Cost Item Sites may only be created "
                                 "by Sales Orders.")
                              .arg(_item->number())
                              .arg(_warehouse->currentText()));
        _item->setId(-1);
        _item->setFocus();
        return;
      }

      if (workCreate.value("itemsite_costmethod").toString() != "J" &&
          workCreate.value("itemsite_costmethod").toString() != "A")
        _jobCosGroup->setEnabled(false);
    }
  
    int itemsiteid = workCreate.value("itemsite_id").toInt();
    double orderQty = _qty->toDouble();
  
    if (_assembly->isChecked())
    {
      workCreate.prepare("SELECT validateOrderQty(:itemsite_id, :qty, true) AS qty;");
      workCreate.bindValue(":itemsite_id", itemsiteid);
      workCreate.bindValue(":qty", _qty->toDouble());
      workCreate.exec();
      if (!workCreate.first())
      {
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Validating Order Quantity"),
                             workCreate, __FILE__, __LINE__);
        return;
      }
  
      orderQty = workCreate.value("qty").toDouble();
      if (orderQty != _qty->toDouble())
      {
        if ( QMessageBox::warning( this, tr("Invalid Order Quantitiy"),
                                   tr( "Order Parameters for this Item do not allow a quantitiy of %1 to be created.\n"
                                       "You must create an order for at least %2 of this item.\n"
                                       "Do you want to update the order quantity and create the order?" )
                                   .arg(formatQty(_qty->toDouble()))
                                   .arg(formatQty(orderQty)),
                                   tr("&Yes"), tr("&No"), 0, 1 ) == 1)
        {
          _qty->clear();
          _qty->setFocus();
          return; 
        }
      }
    }
  
    workCreate.prepare( "SELECT createWo( :woNumber, :itemsite_id, :priority, :orderQty * :sense,"
               "                 COALESCE(:startDate, date(:dueDate) - :leadTime), :dueDate, "
               "                 :productionNotes, :ordtype, :ordid, :prj_id,"
               "                 :bom_rev_id, :boo_rev_id, :wo_cosmethod) AS result;" );
    workCreate.bindValue(":woNumber", _woNumber->text().toInt());
    workCreate.bindValue(":itemsite_id", itemsiteid);
    workCreate.bindValue(":priority", _priority->value());
    workCreate.bindValue(":orderQty", orderQty);
    workCreate.bindValue(":sense", _sense);
    workCreate.bindValue(":leadTime", _leadTime->value());
    workCreate.bindValue(":dueDate", _dueDate->date());
    if (_startDate->isValid())
      workCreate.bindValue(":startDate", _startDate->date());
    workCreate.bindValue(":productionNotes", _productionNotes->toPlainText());
    workCreate.bindValue(":prj_id", _project->id());
    workCreate.bindValue(":bom_rev_id", _bomRevision->id());
    workCreate.bindValue(":boo_rev_id", _booRevision->id());
    if (_todate->isChecked() && _jobCosGroup->isEnabled())
      workCreate.bindValue(":wo_cosmethod",QString("D"));
    else if (_proportional->isChecked() && _jobCosGroup->isEnabled())
      workCreate.bindValue(":wo_cosmethod",QString("P"));

    if(cRelease == _mode)
    {
      workCreate.bindValue(":ordtype", _planordtype);
      workCreate.bindValue(":ordid", _sourceid);
    }
    else
    {
      workCreate.bindValue(":ordtype", QString(""));
      workCreate.bindValue(":ordid", -1);
    }
    workCreate.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating Work Order %1")
                                  .arg(_woNumber->text()),
                                  workCreate, __FILE__, __LINE__))
    {
      return;
    }

    if (!workCreate.first())
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating Work Order %1")
                                        .arg(_woNumber->text()),
                                        workCreate, __FILE__, __LINE__);
      return;
    }
  
    _woid = workCreate.value("result").toInt();
    _wocharView->setId(_woid);
    emit newId(_woid);
  
    workCreate.prepare("SELECT updateCharAssignment('W', :target_id, :char_id, :char_value);");
  
    QModelIndex idx1, idx2;
    for(int i = 0; i < _itemchar->rowCount(); i++)
    {
      idx1 = _itemchar->index(i, 0);
      idx2 = _itemchar->index(i, 1);
      workCreate.bindValue(":target_id", _woid);
      workCreate.bindValue(":char_id", _itemchar->data(idx1, Qt::UserRole));
      workCreate.bindValue(":char_value", _itemchar->data(idx2, Qt::DisplayRole));
      workCreate.exec();
    }
  
    if (_woid == -1)
    {
      QMessageBox::critical( this, tr("Work Order not Created"),
                             tr( "There was an error creating the work order.\n"
                                 "Make sure the itemsite you are creating the work order in is set to allow manufacturing of the item." ));

      return;
    }
    else if (_woid == -2 || _woid == -3)
      QMessageBox::critical( this, tr("Work Order not Exploded"),
                             tr( "The Work Order was created but not Exploded as Component Items defined in the Bill of Materials for\n"
                                 "the selected Work Order Item are not valid in the selected  Work Order Site.\n"
                                 "You must create valid Item Sites for these Component Items before you may explode this Work Order." ));
    else if (_woid == -4)
      QMessageBox::critical( this, tr("Work Order not Exploded"),
                             tr( "The Work Order was created but not Exploded as the Work Order status is not Open\n"));
    
    // if explosion failed then we don't have a valid wo_id.  retrieve using wo_number
    if (_woid < 0)
    {
      workCreate.prepare("SELECT wo_id FROM wo WHERE (wo_number=:woNumber);");
      workCreate.bindValue(":woNumber", _woNumber->text().toInt());
      workCreate.exec();
      if (workCreate.first())
      {
        _woid = workCreate.value("wo_id").toInt();
        _wocharView->setId(_woid);
        emit newId(_woid);
      }
      else
        // give up
        close();
    }

    if (_woid > 0)
    {
      if ((_mode == cNew) || (_mode == cRelease))
      {
        disconnect(_woNumber, SIGNAL(editingFinished()), this, SLOT(sCreate()));
        disconnect(_item, SIGNAL(privateIdChanged(int)), this, SLOT(sCreate()));
        disconnect(_qty, SIGNAL(editingFinished()), this, SLOT(sCreate()));
        disconnect(_dueDate, SIGNAL(newDate(const QDate&)), this, SLOT(sCreate()));

        connect(_priority, SIGNAL(valueChanged(int)), this, SLOT(sReprioritizeParent()));
        connect(_qty, SIGNAL(editingFinished()), this, SLOT(sChangeParentQty()));
        connect(_startDate, SIGNAL(newDate(const QDate&)), this, SLOT(sRescheduleParent()));
        connect(_dueDate, SIGNAL(newDate(const QDate&)), this, SLOT(sRescheduleParent()));
      }

      populate();
      omfgThis->sWorkOrdersUpdated(_woid, true);
    }
  }
}

void workOrder::sSaveClicked()
{
  if (!sSave())
    return;

  XSqlQuery workSave;
  if (_mode == cRelease)
  {
    workSave.prepare("SELECT releasePlannedOrder(:planord_id, false) AS result;");
    workSave.bindValue(":planord_id", _planordid);
    workSave.exec();
  }

  omfgThis->sWorkOrdersUpdated(_woid, true);

  if (_printTraveler->isChecked() &&
      _printTraveler->isVisible())
  {
    ParameterList params;
    params.append("wo_id", _woid);

    printWoTraveler newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }

  emit saved(_woid);

  if (_captive)
    close();
  else if (cNew == _mode)
  {
    populateWoNumber();
    _item->setId(-1);
    _qty->clear();
    _dueDate->setNull();
    _leadTime->setValue(0);
    _startDate->clear();
    _productionNotes->clear();
    _itemchar->removeRows(0, _itemchar->rowCount());
    _close->setText(tr("&Close"));
    _item->setFocus();
  }
}

bool workOrder::sSave()
{
  XSqlQuery workSave;

  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck((!_qty->text().length()) || (_qty->toDouble() == 0.0), _qty,
                          tr( "You have entered an invalid Qty. Ordered.\n"
                              "Please correct before creating this Work Order"  ) )
         << GuiErrorCheck(!_dueDate->isValid(), _dueDate,
                          tr( "You have entered an invalid Due Date.\n"
                              "Please correct before updating this Work Order"  ) )
         << GuiErrorCheck(!_startDate->isValid(), _startDate,
                          tr( "You have entered an invalid Start Date.\n"
                              "Please correct before updating this Work Order"  ) )
         << GuiErrorCheck(!_project->isValid() && _metrics->boolean("RequireProjectAssignment"), _project,
                          tr("<p>You must enter a Project for this order before you may save it."))
     ;

  if (_metrics->boolean("RevControl"))
  {
    workSave.prepare("SELECT rev_status"
                     "  FROM rev"
                     " WHERE (rev_id=:boo_rev_id); ");
    workSave.bindValue(":boo_rev_id", _booRevision->id());
    workSave.exec();
    if (workSave.first())
    {
      QString revstatus = workSave.value("rev_status").toString();
      if ((revstatus != "A") && (revstatus != "S"))
        errors << GuiErrorCheck(true, _booRevision,
                                tr( "You have selected an invalid BOO Revision.\n"
                                   "Please correct before creating this Work Order"  ) );
    }
  }

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Work Order"), errors))
    return false;

  workSave.prepare("UPDATE wo"
            "   SET wo_prodnotes=:productionNotes,"
            "       wo_prj_id=:prj_id,"
            "       wo_bom_rev_id=:bom_rev_id,"
            "       wo_boo_rev_id=:boo_rev_id,"
            "       wo_cosmethod=:wo_cosmethod"
            " WHERE (wo_id=:wo_id); ");
  workSave.bindValue(":wo_id", _woid);
  workSave.bindValue(":productionNotes", _productionNotes->toPlainText());
  workSave.bindValue(":prj_id", _project->id());
  workSave.bindValue(":bom_rev_id", _bomRevision->id());
  workSave.bindValue(":boo_rev_id", _booRevision->id());
  if (_todate->isChecked() && _jobCosGroup->isEnabled())
    workSave.bindValue(":wo_cosmethod",QString("D"));
  else if (_proportional->isChecked() && _jobCosGroup->isEnabled())
    workSave.bindValue(":wo_cosmethod",QString("P"));
  workSave.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Work Order Information"),
                                workSave, __FILE__, __LINE__))
  {
    return false;
  }

  workSave.prepare("SELECT updateCharAssignment('W', :target_id, :char_id, :char_value);");
  QModelIndex idx1, idx2;
  for(int i = 0; i < _itemchar->rowCount(); i++)
  {
    idx1 = _itemchar->index(i, 0);
    idx2 = _itemchar->index(i, 1);
    workSave.bindValue(":target_id", _woid);
    workSave.bindValue(":char_id", _itemchar->data(idx1, Qt::UserRole));
    workSave.bindValue(":char_value", _itemchar->data(idx2, Qt::DisplayRole));
    workSave.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Work Order Information"),
                                  workSave, __FILE__, __LINE__))
    {
      return false;
    }
  }

  return true;
}

void workOrder::sUpdateStartDate()
{
  if ((_warehouse->id() <= 0) || !_dueDate->isValid())
    return;

  if (_leadTime == 0)
  {
    _startDate->setDate(_dueDate->date());
    return;
  }

  XSqlQuery startDate;
  if (_metrics->boolean("UseSiteCalendar"))
      startDate.prepare("SELECT calculateNextWorkingDate(:warehous_id, :dueDate, (:leadTime * -1)) AS startdate;");
  else
      startDate.prepare("SELECT (DATE(:dueDate) - :leadTime) AS startdate;");
  startDate.bindValue(":dueDate", _dueDate->date());
  startDate.bindValue(":leadTime", _leadTime->value());
  startDate.bindValue(":warehous_id", _warehouse->id());
  startDate.exec();
  if (startDate.first())
    _startDate->setDate(startDate.value("startdate").toDate());
  else
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Updating Start Date"),
                       startDate, __FILE__, __LINE__);
}

void workOrder::sPopulateItemChar( int pItemid )
{
  XSqlQuery workPopulateItemChar;
  _itemchar->removeRows(0, _itemchar->rowCount());
  if (pItemid != -1)
  {
    workPopulateItemChar.prepare( "SELECT DISTINCT char_id, char_name,"
               "       COALESCE(b.charass_value, (SELECT c.charass_value FROM charass c WHERE ((c.charass_target_type='I') AND (c.charass_target_id=:item_id) AND (c.charass_default) AND (c.charass_char_id=char_id)) LIMIT 1)) AS charass_value"
               "   FROM (SELECT DISTINCT char_id, char_type, char_name, char_order "
               "         FROM charass, char, charuse"
               "         WHERE ((charass_char_id=char_id)"
               "         AND (charuse_char_id=char_id AND charuse_target_type = 'W') "
               "         AND (charass_target_type='I') "
               "         AND (charass_target_id=:item_id) ) "
               "         UNION SELECT char_id, char_type, char_name, char_order"
               "         FROM charass, char "
               "         WHERE ((charass_char_id=char_id)"
               "         AND  (charass_target_type = 'W' AND charass_target_id=:wo_id))   ) AS data "
               "   LEFT OUTER JOIN charass b ON ((:wo_id=b.charass_target_id)"
               "                                  AND ('W'=b.charass_target_type)"
               "                                  AND (b.charass_char_id=char_id))"
               "   LEFT OUTER JOIN item     i1 ON (i1.item_id=:item_id)"
               "   LEFT OUTER JOIN charass  i2 ON ((i1.item_id=i2.charass_target_id)"
               "                                   AND ('I'=i2.charass_target_type)"
               "                                   AND (i2.charass_char_id=char_id)"
               "                                   AND (i2.charass_default))"
               " ORDER BY char_name;" );
    workPopulateItemChar.bindValue(":item_id", pItemid);
    workPopulateItemChar.bindValue(":wo_id", _woid);
    workPopulateItemChar.exec();
    int row = 0;
    QModelIndex idx;
    while(workPopulateItemChar.next())
    {
      _itemchar->insertRow(_itemchar->rowCount());
      idx = _itemchar->index(row, 0);
      _itemchar->setData(idx, workPopulateItemChar.value("char_name"), Qt::DisplayRole);
      _itemchar->setData(idx, workPopulateItemChar.value("char_id"), Qt::UserRole);
      idx = _itemchar->index(row, 1);
      _itemchar->setData(idx, workPopulateItemChar.value("charass_value"), Qt::DisplayRole);
      _itemchar->setData(idx, pItemid, Xt::IdRole);
      _itemchar->setData(idx, pItemid, Qt::UserRole);
      row++;
    }

    sPopulateLeadTime(_warehouse->id());
  }
}

void workOrder::sPopulateLeadTime(int pWarehousid)
{
  XSqlQuery workPopulateLeadTime;
  if (pWarehousid < 0 || ! _item->isValid() ||
		(_lastWarehousid==pWarehousid && _lastItemid==_item->id()))
    return;

  _lastItemid = _item->id();
  _lastWarehousid = pWarehousid;

  workPopulateLeadTime.prepare( "SELECT itemsite_leadtime, itemsite_cosdefault "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id) );" );
  workPopulateLeadTime.bindValue(":item_id", _item->id());
  workPopulateLeadTime.bindValue(":warehous_id", pWarehousid);
  workPopulateLeadTime.exec();

  if (workPopulateLeadTime.first())
  {
    _leadTime->setValue(workPopulateLeadTime.value("itemsite_leadtime").toInt());
    if (workPopulateLeadTime.value("itemsite_cosdefault").toString() == "D")
	  _todate->setChecked(true);
    if (workPopulateLeadTime.value("itemsite_cosdefault").toString() == "P")
	  _proportional->setChecked(true);
  }
  else
  {
    QMessageBox::warning(this, tr("Invalid Site"),
        tr("<p>The selected Site for this Work Order is not "
           "a \"Supplied At\" Site. You must select a different "
           "Site before creating the Work Order.") );
    _warehouse->setEnabled(true);
  }
}

void workOrder::populateWoNumber()
{
  XSqlQuery workpopulateWoNumber;
  QString generationMethod = _metrics->value("WONumberGeneration");

  if ((generationMethod == "A") || (generationMethod == "O"))
  {
    workpopulateWoNumber.exec("SELECT fetchWoNumber() AS woNumber;");
    if (workpopulateWoNumber.first())
    {
      _woNumber->setText(workpopulateWoNumber.value("woNumber").toString());
      _wonumber = workpopulateWoNumber.value("woNumber").toInt();
    }
    else
    {
      _woNumber->setText("Error");

      ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Work Order Information"),
                           workpopulateWoNumber, __FILE__, __LINE__);

      return;
    }
  }

  if (generationMethod == "M")
  {
    _woNumber->setEnabled(true);
  }
  else if (generationMethod == "O")
  {
    _woNumber->setEnabled(true);
  }
  else if (generationMethod == "A")
  {
    _woNumber->setEnabled(false);
  } 
}

void workOrder::sClose()
{
  XSqlQuery workClose;
  if (_woid > 0)
  {
    if ((_mode == cNew) || (_mode == cRelease))
    {
      workClose.prepare("SELECT deleteWo(:wo_id,true);");
      workClose.bindValue(":wo_id", _woid);
      workClose.exec();
      omfgThis->sWorkOrdersUpdated(_woid, true);
    }
  }
  if (_wonumber > 0)
  {
    if ( ( (_mode == cNew) || (_mode == cRelease)) &&
        ((_metrics->value("WONumberGeneration") == "A") || (_metrics->value("WONumberGeneration") == "O")) )
    {
      workClose.prepare("SELECT releaseWoNumber(:woNumber);");
      workClose.bindValue(":woNumber", _wonumber);
      workClose.exec();
    }
  }

  close();
}

void workOrder::sNumberChanged()
{
  XSqlQuery workNumberChanged;
  if(_wonumber != -1 && _wonumber != _woNumber->text().toInt())
  {
    workNumberChanged.prepare("SELECT releaseWoNumber(:woNumber);");
    workNumberChanged.bindValue(":woNumber", _wonumber);
    workNumberChanged.exec();
  }
}

void workOrder::sHandleButtons()
{
  if (_assembly->isChecked())
    _sense = 1;
  else
    _sense = -1;
}

void workOrder::sFillList()
{
  MetaSQLQuery mql = mqlLoad("workOrder", "detail");

  ParameterList params;
  params.append("wo_id", _woid);
  params.append("showops", QVariant(_booEnabled.toBool() && _showOperations->isChecked()));
  params.append("showmatl", QVariant(_showMaterials->isChecked()));
  params.append("showindent", QVariant(_indented->isChecked()));
  XSqlQuery workFillList = mql.toQuery(params);
  _woIndentedList->populate(workFillList, true);
  _woIndentedList->expandAll();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Work Order Information"),
                                workFillList, __FILE__, __LINE__))
  {
    return;
  }
}

void workOrder::sPostProduction()
{
  if (!sSave())
    return;
  ParameterList params;
  params.append("wo_id", _woIndentedList->id());

  postProduction newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
  int currentId = _woIndentedList->id();
  int currentAltId = _woIndentedList->altId();
  omfgThis->sWorkOrdersUpdated(_woIndentedList->id(), true);
  populate();
  _woIndentedList->setId(currentId,currentAltId);
}

void workOrder::sCorrectProductionPosting()
{
  if (!sSave())
    return;
  ParameterList params;
  params.append("wo_id", _woIndentedList->id());

  correctProductionPosting newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
  int currentId = _woIndentedList->id();
  int currentAltId = _woIndentedList->altId();
  omfgThis->sWorkOrdersUpdated(_woIndentedList->id(), true);
  populate();
  _woIndentedList->setId(currentId,currentAltId);}

void workOrder::sReleaseWO()
{
  if (!sSave())
    return;
  XSqlQuery workReleaseWO;
  workReleaseWO.prepare("SELECT releaseWo(:wo_id, false);");
  workReleaseWO.bindValue(":wo_id", _woIndentedList->id());
  workReleaseWO.exec();

  int currentId = _woIndentedList->id();
  int currentAltId = _woIndentedList->altId();
  omfgThis->sWorkOrdersUpdated(_woIndentedList->id(), true);
  populate();
  _woIndentedList->setId(currentId,currentAltId);
}

void workOrder::sRecallWO()
{
  if (!sSave())
    return;
  XSqlQuery workRecallWO;
  workRecallWO.prepare("SELECT recallWo(:wo_id, false);");
  workRecallWO.bindValue(":wo_id", _woIndentedList->id());
  workRecallWO.exec();

  int currentId = _woIndentedList->id();
  int currentAltId = _woIndentedList->altId();
  omfgThis->sWorkOrdersUpdated(_woIndentedList->id(), true);
  populate();
  _woIndentedList->setId(currentId,currentAltId);
}

void workOrder::sExplodeWO()
{
  if (!sSave())
    return;
  ParameterList params;
  params.append("wo_id", _woIndentedList->id());

  explodeWo newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
  int currentId = _woIndentedList->id();
  int currentAltId = _woIndentedList->altId();
  omfgThis->sWorkOrdersUpdated(_woIndentedList->id(), true);
  populate();
  _woIndentedList->setId(currentId,currentAltId);
}

void workOrder::sImplodeWO()
{
  if (!sSave())
    return;
  ParameterList params;
  params.append("wo_id", _woIndentedList->id());

  implodeWo newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
  int currentId = _woIndentedList->id();
  int currentAltId = _woIndentedList->altId();
  omfgThis->sWorkOrdersUpdated(_woIndentedList->id(), true);
  populate();
  _woIndentedList->setId(currentId,currentAltId);
}

void workOrder::sDeleteWO()
{
  XSqlQuery workDeleteWO;
  workDeleteWO.prepare( "SELECT wo_ordtype "
             "FROM wo "
             "WHERE (wo_id=:wo_id);" );
  workDeleteWO.bindValue(":wo_id", _woIndentedList->id());
  workDeleteWO.exec();
  if (workDeleteWO.first())
  {
    QString question;
    if (workDeleteWO.value("wo_ordtype") == "W")
      question = tr("<p>The Work Order that you selected to delete is a child "
		    "of another Work Order.  If you delete the selected Work "
		    "Order then the Work Order Materials Requirements for the "
		    "Component Item will remain but the Work Order to relieve "
		    "that demand will not. Are you sure that you want to "
		    "delete the selected Work Order?" );
    else if (workDeleteWO.value("wo_ordtype") == "S")
      question = tr("<p>The Work Order that you selected to delete was created "
		    "to satisfy Sales Order demand. If you delete the selected "
		    "Work Order then the Sales Order demand will remain but "
		    "the Work Order to relieve that demand will not. Are you "
		    "sure that you want to delete the selected Work Order?" );
    else
      question = tr("<p>Are you sure that you want to delete the selected "
		    "Work Order?");
    if (QMessageBox::question(this, tr("Delete Work Order?"),
                              question,
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    {
      return;
    }

    workDeleteWO.prepare("SELECT deleteWo(:wo_id, true) AS returnVal;");
    workDeleteWO.bindValue(":wo_id", _woIndentedList->id());
    workDeleteWO.exec();

    if (workDeleteWO.first())
      omfgThis->sWorkOrdersUpdated(-1, true);
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Work Order Information"),
                         workDeleteWO, __FILE__, __LINE__))
    {
      return;
    }
  }
  populate();
}

void workOrder::sCloseWO()
{
  if (!sSave())
    return;
  ParameterList params;
  params.append("wo_id", _woIndentedList->id());

  closeWo newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
  int currentId = _woIndentedList->id();
  int currentAltId = _woIndentedList->altId();
  omfgThis->sWorkOrdersUpdated(_woIndentedList->id(), true);
  populate();
  _woIndentedList->setId(currentId,currentAltId);
}

void workOrder::sPrintTraveler()
{
  if (!sSave())
    return;
  ParameterList params;
  params.append("wo_id", _woIndentedList->id());

  printWoTraveler newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
  int currentId = _woIndentedList->id();
  int currentAltId = _woIndentedList->altId();
  omfgThis->sWorkOrdersUpdated(_woIndentedList->id(), true);
  populate();
  _woIndentedList->setId(currentId,currentAltId);
}

void workOrder::sInventoryAvailabilityByWorkOrder()
{
  ParameterList params;
  params.append("wo_id", _woIndentedList->id());
  params.append("run");

  dspInventoryAvailabilityByWorkOrder *newdlg = new dspInventoryAvailabilityByWorkOrder(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void workOrder::sReprioritizeParent()
{
  if (!sSave())
    return;
  XSqlQuery workReprioritizeParent;
  if(_priority->value() != _oldPriority)
  {
    if ( QMessageBox::warning( this, tr("Change Priority"),
                               tr( "A priority change from %1 to %2 will update all work order requirements.  "
                                   "Are you sure you want to change the work order priority?" )
                               .arg(QString().setNum(_oldPriority))
                               .arg(QString().setNum(_priority->value())),
                               tr("&Yes"), tr("&No"), QString::null, 0, 1 ) == 1 )
    {
      _priority->setValue(_oldPriority);
      return;
    }
  
    workReprioritizeParent.prepare("SELECT reprioritizeWo(:wo_id, :newPriority, :reprioritizeChildren);");
    workReprioritizeParent.bindValue(":wo_id", _woid);
    workReprioritizeParent.bindValue(":newPriority", _priority->value());
    workReprioritizeParent.bindValue(":reprioritizeChildren", true);
    workReprioritizeParent.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Reprioritizing Work Order Information"),
                                  workReprioritizeParent, __FILE__, __LINE__))
    {
      return;
    }
    else
      _oldPriority=_priority->value();
    populate();
    omfgThis->sWorkOrdersUpdated(_woid, true);
  }
}

void workOrder::sRescheduleParent()
{
  if (!sSave())
    return;
  XSqlQuery workRescheduleParent;
  if(_startDate->date() != _oldStartDate || _dueDate->date() != _oldDueDate)
  {
    if ( QMessageBox::warning( this, tr("Change Date"),
                               tr( "Changing the start or due date will update all work order requirements.  "
                                   "Are you sure you want to reschedule all dates?" ),
                               tr("&Yes"), tr("&No"), QString::null, 0, 1 ) == 1 )
    {
      _startDate->setDate(_oldStartDate);
      _dueDate->setDate(_oldDueDate);
      QMessageBox::warning( this, tr("Change Date"),
                            tr( "Changing the due date may change the Bill of Material components that are effective.\n"
                                "You may want to consider imploding and exploding the Work Order.\n" ) );
      return;
    }
  
    workRescheduleParent.prepare("SELECT changeWoDates(:wo_id, :startDate, :dueDate, :rescheduleChildren);");
    workRescheduleParent.bindValue(":wo_id", _woid);
    workRescheduleParent.bindValue(":startDate", _startDate->date());
    workRescheduleParent.bindValue(":dueDate", _dueDate->date());
    workRescheduleParent.bindValue(":rescheduleChildren", true);
    workRescheduleParent.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Rescheduling Work Order Information"),
                                  workRescheduleParent, __FILE__, __LINE__))
    {
      return;
    }
    else
    {
      _oldStartDate=_startDate->date();
      _oldDueDate=_dueDate->date();
    }
  }
  populate();
  omfgThis->sWorkOrdersUpdated(_woid, true);
}

void workOrder::sChangeParentQty()
{
  if (!sSave())
    return;
  XSqlQuery workChangeParentQty;
  if(_qty->text().toDouble() != _oldQty)
  {
    double newQty = _qty->toDouble();
    workChangeParentQty.prepare( "SELECT validateOrderQty(wo_itemsite_id, :qty, true) AS qty "
               "FROM wo "
               "WHERE (wo_id=:wo_id);" );
    workChangeParentQty.bindValue(":wo_id", _woid);
    workChangeParentQty.bindValue(":qty", newQty);
    workChangeParentQty.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Validating Work Order Quantity"),
                                  workChangeParentQty, __FILE__, __LINE__))
    {
      return;
    }
    else if (workChangeParentQty.first())
    {
      if (workChangeParentQty.value("qty").toDouble() != newQty)
      {
        if ( QMessageBox::warning( this, tr("Invalid Order Qty"),
                                   tr( "The new Order Quantity that you have entered does not meet the Order Parameters set "
                                       "for the parent Item Site for this Work Order.  In order to meet the Item Site Order "
                                       "Parameters the new Order Quantity must be increased to %1. "
                                       "Do you want to change the Order Quantity for this Work Order to %2?" )
                                   .arg(formatQty(workChangeParentQty.value("qty").toDouble()))
                                   .arg(formatQty(workChangeParentQty.value("qty").toDouble())),
                                   tr("&Yes"), tr("&No"), QString::null, 0, 1 ) == 1 )
          {
            _qty->setText(_oldQty);
            _qty->setFocus();
            return;
          }
          else
            newQty = workChangeParentQty.value("qty").toDouble();
      }
      else if ( QMessageBox::warning( this, tr("Change Qty"),
                                 tr( "A quantity change from %1 to %2 will update all work order requirements.  "
                                     "Are you sure you want to change the work order quantity?" )
                                 .arg(formatQty(_oldQty))
                                 .arg(formatQty(_qty->text().toDouble())),
                                 tr("&Yes"), tr("&No"), QString::null, 0, 1 ) == 1 )
      {
        _qty->setText(_oldQty);
        return;
      }

      workChangeParentQty.prepare("SELECT changeWoQty(:wo_id, :qty * :sense, true);");
      workChangeParentQty.bindValue(":wo_id", _woid);
      workChangeParentQty.bindValue(":qty", newQty);
      workChangeParentQty.bindValue(":sense", _sense);
      workChangeParentQty.exec();
      if (ErrorReporter::error(QtCriticalMsg, this, tr("Change Work Order Quantity"),
                               workChangeParentQty, __FILE__, __LINE__))
      {
        return;
      }
      else
        _oldQty=(_qty->text().toDouble() * _sense);
    }
    populate();
    omfgThis->sWorkOrdersUpdated(_woid, true);
  }
}

void workOrder::sReprioritizeWo()
{
  if (!sSave())
    return;
  ParameterList params;
  params.append("wo_id", _woIndentedList->id());

  reprioritizeWo newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
  populate();
  omfgThis->sWorkOrdersUpdated(_woIndentedList->id(), true);
}

void workOrder::sRescheduleWO()
{
  if (!sSave())
    return;
  ParameterList params;
  params.append("wo_id", _woIndentedList->id());

  rescheduleWo newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
  populate();
  omfgThis->sWorkOrdersUpdated(_woIndentedList->id(), true);
}

void workOrder::sChangeWOQty()
{
  if (!sSave())
    return;
  ParameterList params;
  params.append("wo_id", _woIndentedList->id());

  changeWoQty newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
  populate();
  omfgThis->sWorkOrdersUpdated(_woIndentedList->id(), true);
}

void workOrder::sDspRunningAvailability()
{
  XSqlQuery workDspRunningAvailability;
  workDspRunningAvailability.prepare("SELECT wo_itemsite_id FROM wo WHERE (wo_id=:id);");
  workDspRunningAvailability.bindValue(":id", _woIndentedList->id());
  workDspRunningAvailability.exec();
  if (workDspRunningAvailability.first())
  {
    ParameterList params;
    params.append("itemsite_id", workDspRunningAvailability.value("wo_itemsite_id"));
    params.append("run");

    dspRunningAvailability *newdlg = new dspRunningAvailability(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Work Order Information"),
                                workDspRunningAvailability, __FILE__, __LINE__))
  {
    return;
  }
}

void workOrder::sReturnMatlBatch()
{
  if (!sSave())
    return;
  XSqlQuery workReturnMatlBatch;
  workReturnMatlBatch.prepare( "SELECT wo_qtyrcv, wo_status IN ('E', 'I') AS can_return "
             "FROM wo "
             "WHERE (wo_id=:wo_id);" );
  workReturnMatlBatch.bindValue(":wo_id", _woIndentedList->id());
  workReturnMatlBatch.exec();
  if (workReturnMatlBatch.first())
  {
    if (workReturnMatlBatch.value("wo_qtyrcv").toDouble() != 0)
    {
      QMessageBox::warning( this, tr("Cannot return Work Order Material"),
                            tr( "This Work Order has had material received against it\n"
                                "and thus the material issued against it cannot be returned.\n"
                                "You must instead return each Work Order Material item individually.\n" ) );
    }
    else if (!workReturnMatlBatch.value("can_return").toBool())
    {
      QMessageBox::warning( this, tr("Cannot return Work Order Material"),
                            tr( "Work Order must have status of Exploded or In-Process." ) );
    }
    else
    {
      // Create itemlocdist records for controlled items with qty to return
      XSqlQuery items;
      items.prepare("SELECT womatl_id, womatl_uom_id, womatl_qtyreq, womatl_qtyiss, womatl_issuemethod, "
                    " CASE WHEN wo_qtyord >= 0 THEN womatl_qtyiss "
                    "   ELSE ((womatl_qtyreq - womatl_qtyiss) * -1) "
                    " END AS qty, "
                    " itemsite_id, itemsite_item_id, item_number, "
                    " isControlledItemsite(itemsite_id) AS controlled "
                    "FROM wo, womatl, itemsite, item "
                    "WHERE wo_id=womatl_wo_id "
                    " AND womatl_itemsite_id=itemsite_id "
                    " AND itemsite_item_id=item_id "
                    " AND womatl_wo_id=:wo_id "
                    " AND ((wo_qtyord < 0) OR (womatl_issuemethod IN ('S','M'))) "
                    "ORDER BY womatl_id;");
      items.bindValue(":wo_id", _woIndentedList->id());
      items.exec();

      int succeeded = 0;
      QList<QString> failedItems;
      QList<QString> errors;
      while(items.next())
      {
        if (items.value("qty").toDouble() == 0.0)
          continue;

        int itemlocSeries;

        // Stage distribution cleanup function to be called on error
        XSqlQuery cleanup;
        cleanup.prepare("SELECT deleteitemlocseries(:itemlocSeries, TRUE);");

        // Generate a series id to be used for itemlocdist if controlled, and for returnWoMaterial()
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

        // If controlled and backflush item has relevant qty for returnWoMaterial
        if (items.value("controlled").toBool() && 
           (items.value("womatl_qtyreq").toDouble() >= 0 ? 
            items.value("womatl_qtyiss").toDouble() >= items.value("qty").toDouble() : 
            items.value("womatl_qtyiss").toDouble() <= items.value("qty").toDouble()))
        {
          // Create the parent itemlocdist record for each line item requiring distribution and call distributeInventory::seriesAdjust
          XSqlQuery parentItemlocdist;
          parentItemlocdist.prepare("SELECT createitemlocdistparent(:itemsite_id, "
                                    " COALESCE(itemuomtouom(:itemsite_item_id, :womatl_uom_id, NULL, :qty), :qty), "
                                    "'WO', :orderitemId, :itemlocSeries, NULL, NULL, 'IM');");
          parentItemlocdist.bindValue(":itemsite_id", items.value("itemsite_id").toInt());
          parentItemlocdist.bindValue(":itemsite_item_id", items.value("itemsite_item_id").toInt());
          parentItemlocdist.bindValue(":womatl_uom_id", items.value("womatl_uom_id").toInt());
          parentItemlocdist.bindValue(":qty", items.value("qty").toDouble());
          parentItemlocdist.bindValue(":orderitemId", _woIndentedList->id());
          parentItemlocdist.bindValue(":itemlocSeries", itemlocSeries);
          parentItemlocdist.exec();
          if (parentItemlocdist.first())
          {
            if (distributeInventory::SeriesAdjust(itemlocSeries, this, QString(), QDate(),
              QDate(), true) == XDialog::Rejected)
            {
              cleanup.exec();
              // If it's not the last item in the loop, ask the user to exit loop or continue
              if (items.at() != (items.size() -1))
              {
                if (QMessageBox::question(this,  tr("Return WO Material"),
                tr("Posting distribution detail for item number %1 was cancelled but "
                  "there are more items to return. Continue returning the remaining materials?")
                .arg(items.value("item_number").toString()),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
                {
                  failedItems.append(items.value("item_number").toString());
                  errors.append("Detail Distribution Cancelled");
                  continue;
                }
                else
                {
                  failedItems.append(items.value("item_number").toString());
                  errors.append("Detail Distribution Cancelled");
                  break;
                }
              }
              else 
              {
                failedItems.append(items.value("item_number").toString());
                errors.append("Detail Distribution Cancelled");
                continue;
              }
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

        XSqlQuery returnReturn;
        returnReturn.prepare("SELECT returnWoMaterial(:womatl_id, :qty, :itemlocSeries, now(), FALSE, TRUE, TRUE) AS result;");
        returnReturn.bindValue(":womatl_id", items.value("womatl_id").toInt());
        returnReturn.bindValue(":qty", items.value("qty").toDouble());
        returnReturn.bindValue(":itemlocSeries", itemlocSeries);
        returnReturn.exec();
        if (returnReturn.first())
        {
          if (returnReturn.value("result").toInt() < 0)
          {
            cleanup.exec();
            failedItems.append(items.value("item_number").toString());
            errors.append(tr("Return WO Material failed. %1")
              .arg(returnReturn.lastError().text()));
            continue;
          }
        }
        else
        {
          cleanup.exec();
          failedItems.append(items.value("item_number").toString());
          errors.append(tr("Error Returning Work Order Material Batch. %")
            .arg(returnReturn.lastError().text()));
          continue;
        }
        succeeded++;
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
    }
  }
  populate();
}

void workOrder::sIssueMatlBatch()
{
  if (!sSave())
    return;
  XSqlQuery items;
  items.prepare("SELECT itemsite_id, itemsite_item_id, item_number, item_fractional, "
                "       isControlledItemsite(itemsite_id) AS controlled, "
                "       warehous_code, "
                "       womatl_id, womatl_uom_id, womatl_qtyreq, womatl_qtyiss, womatl_issuemethod, "
                "       CASE WHEN (womatl_qtyreq >= 0) THEN "
                "         roundQty(itemuomfractionalbyuom(item_id, womatl_uom_id), noNeg(womatl_qtyreq - womatl_qtyiss)) "
                "       ELSE "
                "         roundQty(itemuomfractionalbyuom(item_id, womatl_uom_id), noNeg(womatl_qtyiss * -1)) "
                "       END AS qty, "
                "       (COALESCE((SELECT SUM(itemloc_qty) "
                "                    FROM itemloc "
                "                   WHERE (itemloc_itemsite_id=itemsite_id)), 0.0) "
                "        >= roundQty(item_fractional, noNeg(itemuomtouom(itemsite_item_id, womatl_uom_id, NULL, womatl_qtyreq - womatl_qtyiss)))) AS isqtyavail "
                "  FROM wo, womatl, itemsite, item, whsinfo "
                " WHERE ( (wo_id=womatl_wo_id) "
                "   AND (womatl_itemsite_id=itemsite_id) "
                "   AND (itemsite_item_id=item_id) "
                "   AND (itemsite_warehous_id=warehous_id) "
                "   AND (womatl_issuemethod IN ('S', 'M')) "
                "   AND (womatl_wo_id=:wo_id)); ");
  items.bindValue(":wo_id", _woIndentedList->id());
  items.exec();

  int succeeded = 0;
  QList<QString> failedItems;
  QList<QString> errors;
  while (items.next())
  {
    if (items.value("controlled").toBool() && !items.value("isqtyavail").toBool())
    {
      failedItems.append(items.value("item_number").toString());
      errors.append(tr("Item Number %1 in Site %2 is a Multiple Location or\n"
           "Lot/Serial controlled Item which is short on Inventory.\n"
           "This transaction cannot be completed as is. Please make\n"
           "sure there is sufficient Quantity on Hand before proceeding.")
          .arg(items.value("item_number").toString())
          .arg(items.value("warehous_code").toString()));
      continue;
    }

    if (items.value("qty").toDouble() == 0.0)
      continue;

    int itemlocSeries;

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
      failedItems.append(items.value("item_number").toString());
      errors.append("Failed to Retrieve the Next itemloc_series_seq");
      continue;
    }

    // If controlled and backflush item has relevant qty for returnWoMaterial
    if (items.value("controlled").toBool())
    {
      // Create the parent itemlocdist record for each line item requiring distribution and call distributeInventory::seriesAdjust
      XSqlQuery parentItemlocdist;
      parentItemlocdist.prepare("SELECT createitemlocdistparent(:itemsite_id, "
                                " roundQty(:item_fractional, itemuomtouom(:itemsite_item_id, :womatl_uom_id, NULL, :qty)) * -1, "
                                "'WO', :orderitemId, :itemlocSeries, NULL, NULL, 'IM');");
      parentItemlocdist.bindValue(":itemsite_id", items.value("itemsite_id").toInt());
      parentItemlocdist.bindValue(":item_fractional", items.value("item_fractional").toBool());
      parentItemlocdist.bindValue(":itemsite_item_id", items.value("itemsite_item_id").toInt());
      parentItemlocdist.bindValue(":womatl_uom_id", items.value("womatl_uom_id").toInt());
      parentItemlocdist.bindValue(":qty", items.value("qty").toDouble());
      parentItemlocdist.bindValue(":orderitemId", _woIndentedList->id());
      parentItemlocdist.bindValue(":itemlocSeries", itemlocSeries);
      parentItemlocdist.exec();
      if (parentItemlocdist.first())
      {
        if (distributeInventory::SeriesAdjust(itemlocSeries, this, QString(), QDate(),
          QDate(), true) == XDialog::Rejected)
        {
          cleanup.exec();
          // If it's not the last item in the loop, ask the user to exit loop or continue
          if (items.at() != (items.size() -1))
          {
            if (QMessageBox::question(this,  tr("Issue WO Material"),
            tr("Posting distribution detail for item number %1 was cancelled but "
              "there are more items to issue. Continue issuing the remaining materials?")
            .arg(items.value("item_number").toString()),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
            {
              failedItems.append(items.value("item_number").toString());
              errors.append("Detail Distribution Cancelled");
              continue;
            }
            else
            {
              failedItems.append(items.value("item_number").toString());
              errors.append("Detail Distribution Cancelled");
              break;
            }
          }
          else 
          {
            failedItems.append(items.value("item_number").toString());
            errors.append("Detail Distribution Cancelled");
            continue;
          }
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

    // issueWoMaterial
    XSqlQuery issue;
    issue.prepare("SELECT issueWoMaterial(:womatl_id, :qty, :itemlocSeries, now(), NULL::INTEGER, NULL::NUMERIC, TRUE, TRUE) AS result;");
    issue.bindValue(":womatl_id", items.value("womatl_id").toInt());
    issue.bindValue(":qty", items.value("qty").toDouble());
    issue.bindValue(":itemlocSeries", itemlocSeries);
    issue.exec();
    if (issue.first())
    {
      if (issue.value("result").toInt() < 0)
      {
        cleanup.exec();
        failedItems.append(items.value("item_number").toString());
        errors.append(tr("Error issuing Work Order Material. %")
          .arg(issue.lastError().text()));
        continue;
      }
      else
      {
        omfgThis->sWorkOrdersUpdated(_woIndentedList->id(), true);
        continue;
      }
    }
    else
    {
      cleanup.exec();
      failedItems.append(items.value("item_number").toString());
      errors.append(tr("Error issuing Work Order Material. %")
        .arg(issue.lastError().text()));
      continue;
    }

    succeeded++;
  }

  populate();

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
}

void workOrder::sIssueMatl()
{
  if (!sSave())
    return;
  XSqlQuery workIssueMatl;
  issueWoMaterialItem newdlg(this);
  ParameterList params;
  workIssueMatl.prepare("SELECT womatl_wo_id AS wo_id FROM womatl "
            " WHERE (womatl_id=:womatl_id) ");
        workIssueMatl.bindValue(":womatl_id", _woIndentedList->id());
        workIssueMatl.exec();
        if (workIssueMatl.first())
            params.append("wo_id", workIssueMatl.value("wo_id").toInt());
  params.append("womatl_id", _woIndentedList->id());
  if (newdlg.set(params) == NoError)
    newdlg.exec();
  int currentId = _woIndentedList->id();
  int currentAltId = _woIndentedList->altId();
  omfgThis->sWorkOrdersUpdated(_woIndentedList->id(), true);
  populate();
  _woIndentedList->setId(currentId,currentAltId);
}

void workOrder::sReturnMatl()
{
  if (!sSave())
    return;
  returnWoMaterialItem newdlg(this);
  ParameterList params;
  params.append("womatl_id", _woIndentedList->id());
  if (newdlg.set(params) == NoError)
    newdlg.exec();
  int currentId = _woIndentedList->id();
  int currentAltId = _woIndentedList->altId();
  omfgThis->sWorkOrdersUpdated(_woIndentedList->id(), true);
  populate();
  _woIndentedList->setId(currentId,currentAltId);
}

void workOrder::sScrapMatl()
{
  if (!sSave())
    return;
  scrapWoMaterialFromWIP newdlg(this);
  ParameterList params;
  params.append("womatl_id", _woIndentedList->id());
  if (newdlg.set(params) == NoError)
    newdlg.exec();
  int currentId = _woIndentedList->id();
  int currentAltId = _woIndentedList->altId();
  omfgThis->sWorkOrdersUpdated(_woIndentedList->id(), true);
  populate();
  _woIndentedList->setId(currentId,currentAltId);
}

void workOrder::sNewMatl()
{
  if (!sSave())
    return;
  ParameterList params;
  params.append("mode", "new");
  params.append("wo_id", _woIndentedList->id());

  woMaterialItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
  int currentId = _woIndentedList->id();
  int currentAltId = _woIndentedList->altId();
  omfgThis->sWorkOrdersUpdated(_woIndentedList->id(), true);
  populate();
  _woIndentedList->setId(currentId,currentAltId);
}

void workOrder::sEditMatl()
{
  if (!sSave())
    return;
  ParameterList params;
  params.append("mode", "edit");
  params.append("womatl_id", _woIndentedList->id());

  woMaterialItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
  int currentId = _woIndentedList->id();
  int currentAltId = _woIndentedList->altId();
  omfgThis->sWorkOrdersUpdated(_woIndentedList->id(), true);
  populate();
  _woIndentedList->setId(currentId,currentAltId);
}

void workOrder::sViewMatl()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("womatl_id", _woIndentedList->id());
  woMaterialItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void workOrder::sDeleteMatl()
{
  if (!sSave())
    return;
  XSqlQuery workDeleteMatl;
  int womatlid = _woIndentedList->id();
  if (_woIndentedList->currentItem()->data(7, Qt::UserRole).toMap().value("raw").toDouble() > 0)
  {
    if(_privileges->check("ReturnWoMaterials"))
    {
      if (QMessageBox::question(this, tr("W/O Material Requirement cannot be Deleted"),
                                tr("<p>This W/O Material Requirement cannot "
                                   "be deleted as it has has material issued "
                                   "to it. You must return this material to "
                                   "stock before you can delete this Material "
                                   "Requirement. Would you like to return this "
                                   "material to stock now?"  ),
                                QMessageBox::Yes,
                                QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
      {
        ParameterList params;
        params.append("womatl_id", womatlid);

        returnWoMaterialItem newdlg(omfgThis, "", true);
        newdlg.set(params);

        newdlg.exec();
        populate();

        workDeleteMatl.prepare("SELECT womatl_qtyiss AS qtyissued "
                               "FROM womatl "
                               "WHERE (womatl_id=:womatl_id) ");
        workDeleteMatl.bindValue(":womatl_id", womatlid);
        workDeleteMatl.exec();
        if (!workDeleteMatl.first() || workDeleteMatl.value("qtyissued").toInt() != 0)
          return;
      }
      else
        return;
    }
    else
    {
      QMessageBox::critical(this, tr("W/O Material Requirement cannot be Deleted"),
                            tr("<p>This W/O Material Requirement cannot be "
                               "deleted as it has material issued to it. "
                               "You must return this material to stock before "
                               "you can delete this Material Requirement." ) );
      return;
    }
  }

  workDeleteMatl.prepare("SELECT wo_id AS woid "
                         "FROM womatl JOIN wo ON (wo_ordtype='W' AND"
                         "                        wo_ordid=womatl_wo_id AND"
                         "                        wo_itemsite_id=womatl_itemsite_id) "
                         "WHERE (womatl_id=:womatl_id) ");
  workDeleteMatl.bindValue(":womatl_id", womatlid);
  workDeleteMatl.exec();
  if (workDeleteMatl.first())
  {
    int woid = workDeleteMatl.value("woid").toInt();
    if (QMessageBox::question(this, tr("Child W/O Exists"),
                              tr("<p>This W/O Material Requirement is "
                                 "associated with a Child Work Order. "
                                 "Would you like to delete this "
                                 "Child Work Order now?"  ),
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
    {
      workDeleteMatl.prepare("SELECT deleteWo(:wo_id, true) AS result;");
      workDeleteMatl.bindValue(":wo_id", woid);
      workDeleteMatl.exec();
      if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Work Order Information"),
                                    workDeleteMatl, __FILE__, __LINE__))
      {
        return;
      }
    }
  }

  workDeleteMatl.prepare("SELECT deleteWoMaterial(:womatl_id);");
  workDeleteMatl.bindValue(":womatl_id", womatlid);
  workDeleteMatl.exec();
  if (workDeleteMatl.first())
  {
    int result = workDeleteMatl.value("result").toInt();
    if (result < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Work Order Material Information"),
                             storedProcErrorLookup("deleteWoMaterial", result),
                             __FILE__, __LINE__);
      return;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Work Order Material Information"),
                                workDeleteMatl, __FILE__, __LINE__))
  {
    return;
  }
  
  workDeleteMatl.prepare("SELECT womatl_wo_id AS woid "
                         "FROM womatl "
                         "WHERE (womatl_id=:womatl_id) ");
  workDeleteMatl.bindValue(":womatl_id", womatlid);
  workDeleteMatl.exec();
  if (workDeleteMatl.first())
     omfgThis->sWorkOrderMaterialsUpdated(workDeleteMatl.value("woid").toInt(), womatlid, true);
           
  omfgThis->sWorkOrdersUpdated(_woIndentedList->id(), true);
  populate();
}

void workOrder::sViewMatlAvailability()
{
  XSqlQuery workViewMatlAvailability;
  workViewMatlAvailability.prepare( "SELECT womatl_itemsite_id, womatl_duedate "
             "FROM womatl "
             "WHERE (womatl_id=:womatl_id);" );
  workViewMatlAvailability.bindValue(":womatl_id", _woIndentedList->id());
  workViewMatlAvailability.exec();
  if (workViewMatlAvailability.first())
  {
    ParameterList params;
    params.append("itemsite_id", workViewMatlAvailability.value("womatl_itemsite_id"));
    params.append("byDate", workViewMatlAvailability.value("womatl_duedate"));
    params.append("run");

    dspInventoryAvailability *newdlg = new dspInventoryAvailability();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Work Order Material Information"),
                                workViewMatlAvailability, __FILE__, __LINE__))
  {
    return;
  }
}

void workOrder::sViewMatlSubstituteAvailability()
{
  XSqlQuery workViewMatlSubstituteAvailability;
  workViewMatlSubstituteAvailability.prepare( "SELECT womatl_itemsite_id, womatl_duedate "
             "FROM womatl "
             "WHERE (womatl_id=:womatl_id);" );
  workViewMatlSubstituteAvailability.bindValue(":womatl_id", _woIndentedList->id());
  workViewMatlSubstituteAvailability.exec();
  if (workViewMatlSubstituteAvailability.first())
  {
    ParameterList params;
    params.append("itemsite_id", workViewMatlSubstituteAvailability.value("womatl_itemsite_id"));
    params.append("byDate", workViewMatlSubstituteAvailability.value("womatl_duedate"));
    params.append("run");

    dspSubstituteAvailabilityByItem *newdlg = new dspSubstituteAvailabilityByItem(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Work Order Material Information"),
                                workViewMatlSubstituteAvailability, __FILE__, __LINE__))
  {
    return;
  }
}

void workOrder::sSubstituteMatl()
{
  if (!sSave())
    return;
  XSqlQuery workSubstituteMatl;
  int womatlid = _woIndentedList->id();

  XSqlQuery sub;
  sub.prepare( "SELECT itemuomtouom(itemsite_item_id, womatl_uom_id, NULL, womatl_qtyper) AS qtyper, womatl_wo_id,"
               "       womatl_scrap, womatl_issuemethod,"
               "       womatl_duedate, womatl_bomitem_id, "
               "       womatl_notes, womatl_ref, womatl_wooper_id "
               "FROM womatl JOIN itemsite ON (womatl_itemsite_id=itemsite_id) "
               "WHERE (womatl_id=:womatl_id);" );
  sub.bindValue(":womatl_id", womatlid);
  sub.exec();
  if (sub.first())
  {
    ParameterList params;
    params.append("womatl_id", womatlid);
    params.append("byDate", sub.value("womatl_duedate"));
    params.append("run");

    substituteList substitute(this, "", true);
    substitute.set(params);
    int result = substitute.exec();
    if (result != XDialog::Rejected)
    {
      ParameterList params;
      params.append("mode", "new");
      params.append("wo_id", sub.value("womatl_wo_id"));
      params.append("bomitem_id", sub.value("womatl_bomitem_id"));
      params.append("item_id", result);
      params.append("qtyPer", (sub.value("qtyper").toDouble() * substitute._uomratio));
      params.append("scrap", sub.value("womatl_scrap"));
      params.append("notes", sub.value("womatl_notes"));
      params.append("reference", sub.value("womatl_ref"));
      params.append("wooper_id", sub.value("womatl_wooper_id"));

      if (sub.value("womatl_issuemethod").toString() == "S")
        params.append("issueMethod", "push");
      else if (sub.value("womatl_issuemethod").toString() == "L")
        params.append("issueMethod", "pull");
      else if (sub.value("womatl_issuemethod").toString() == "M")
        params.append("issueMethod", "mixed");

      woMaterialItem newdlg(this, "", true);
      newdlg.set(params);
      if (newdlg.exec() != XDialog::Rejected)
      {
        workSubstituteMatl.prepare( "DELETE FROM womatl "
                   "WHERE (womatl_id=:womatl_id);" );
        workSubstituteMatl.bindValue(":womatl_id", womatlid);
        workSubstituteMatl.exec();

        workSubstituteMatl.prepare("SELECT womatl_wo_id AS woid "
                  "FROM womatl "
                  "WHERE (womatl_id=:womatl_id) ");
        workSubstituteMatl.bindValue(":womatl_id", womatlid);
        workSubstituteMatl.exec();
        if (workSubstituteMatl.first())
           omfgThis->sWorkOrderMaterialsUpdated(workSubstituteMatl.value("woid").toInt(), womatlid, true);
      }
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Work Order Material Information"),
                                workSubstituteMatl, __FILE__, __LINE__))
  {
    return;
  }
  omfgThis->sWorkOrdersUpdated(_woIndentedList->id(), true);
  populate();
}

void workOrder::sPopulateMenu(QMenu *pMenu,  QTreeWidgetItem *selected)
{
  QString  status(selected->text(3));
  double qtyiss = _woIndentedList->rawValue("qtyiss").toDouble();
  QAction *menuItem;

  //Check if row is a work order and id is vaild
  if(_woIndentedList->altId() == 1 && _woIndentedList->id() > -1)
  {
    if (_mode != cView)
    {
      if (status == "O")
      {
        menuItem = pMenu->addAction(tr("Explode..."), this, SLOT(sExplodeWO()));
        if (!_privileges->check("ExplodeWorkOrders"))
          menuItem->setEnabled(false);
      }
      else if (status == "E")
      {
        menuItem = pMenu->addAction(tr("Implode..."), this, SLOT(sImplodeWO()));
        if (!_privileges->check("ImplodeWorkOrders"))
          menuItem->setEnabled(false);
          
        menuItem = pMenu->addAction(tr("Release"), this, SLOT(sReleaseWO()));
        if (!_privileges->check("ReleaseWorkOrders"))
          menuItem->setEnabled(false);
      }
      else if (status == "R")
      {
        menuItem = pMenu->addAction(tr("Recall"), this, SLOT(sRecallWO()));
        if (!_privileges->check("RecallWorkOrders"))
          menuItem->setEnabled(false);
      }
      
      if (((XTreeWidgetItem *)selected)->QTreeWidgetItem::parent() && ((status == "O") || (status == "E")))
      {
        menuItem = pMenu->addAction(tr("Delete..."), this, SLOT(sDeleteWO()));
        if (!_privileges->check("DeleteWorkOrders"))
          menuItem->setEnabled(false);
      }
      else
      {
       if ((status != "C"))
       {
         menuItem = pMenu->addAction(tr("Close..."), this, SLOT(sCloseWO()));
         if (!_privileges->check("CloseWorkOrders"))
           menuItem->setEnabled(false);
       }
        pMenu->addSeparator();
      }

      if ((status == "E") || (status == "R") || (status == "I"))
      {
        menuItem = pMenu->addAction(tr("Print Traveler..."), this, SLOT(sPrintTraveler()));
        if (!_privileges->check("PrintWorkOrderPaperWork"))
          menuItem->setEnabled(false);

        pMenu->addSeparator();
        
        if (status == "O" ||status == "E" || status == "R" || status == "I")
        {
          menuItem = pMenu->addAction(tr("New Material..."), this, SLOT(sNewMatl()));
          if (!_privileges->check("MaintainWoMaterials"))
            menuItem->setEnabled(false);
              
          if ((_metrics->boolean("IssueToExplodedWO") && status == "E") || status == "R" || status == "I")
          {
            menuItem = pMenu->addAction(tr("Issue Batch..."), this, SLOT(sIssueMatlBatch()));
            if (!_privileges->check("IssueWoMaterials"))
              menuItem->setEnabled(false);
          }

          if (status == "I")
          {
            menuItem = pMenu->addAction(tr("Return Batch..."), this, SLOT(sReturnMatlBatch()));
            if (!_privileges->check("ReturnWoMaterials"))
              menuItem->setEnabled(false);
          }
          pMenu->addSeparator();
        }
      }
    }
        
    if (_mode != cView)
    {
      menuItem = pMenu->addAction(tr("Post Production..."), this, SLOT(sPostProduction()));
      if (!_privileges->check("PostProduction"))
        menuItem->setEnabled(false);

      if (status == "I")
      {
        menuItem = pMenu->addAction(tr("Correct Production Posting..."), this, SLOT(sCorrectProductionPosting()));
        if (!_privileges->check("PostProduction"))
          menuItem->setEnabled(false);
      }

      pMenu->addSeparator();
    }

    pMenu->addSeparator();

    menuItem = pMenu->addAction(tr("Running Availability..."), this, SLOT(sDspRunningAvailability()));

    if ((status == "E") || (status == "R") || (status == "I"))
    {
      menuItem = pMenu->addAction(tr("Inventory Availability..."), this, SLOT(sInventoryAvailabilityByWorkOrder()));
      if (!_privileges->check("ViewInventoryAvailability"))
        menuItem->setEnabled(false);
    }

    if (_mode != cView)
    {
      if ((status == "O") || (status == "E"))
      {
        pMenu->addSeparator();

        menuItem = pMenu->addAction(tr("Reprioritize..."), this, SLOT(sReprioritizeWo()));
        if (!_privileges->check("ReprioritizeWorkOrders"))
          menuItem->setEnabled(false);

        menuItem = pMenu->addAction(tr("Reschedule..."), this, SLOT(sRescheduleWO()));
        if (!_privileges->check("RescheduleWorkOrders"))
          menuItem->setEnabled(false);

        menuItem = pMenu->addAction(tr("Change Quantity..."), this, SLOT(sChangeWOQty()));
        if (!_privileges->check("ChangeWorkOrderQty"))
          menuItem->setEnabled(false);
      }
    }
  }
  
  //Check a womatl row is selected and the id is vaild
  if(_woIndentedList->altId() == 2 && _woIndentedList->id() > -1)
  {
    if (_mode != cView)
    {
      if (status == "O" || status == "E" || status == "R" || status == "I")
      {
         menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEditMatl()));
         if (!_privileges->check("MaintainWoMaterials"))
          menuItem->setEnabled(false);
      }
    }
    
    menuItem = pMenu->addAction(tr("View..."), this, SLOT(sViewMatl()));
    
    if (_mode != cView)
    {
      if (status == "O" || status == "E")
      {
          menuItem = pMenu->addAction(tr("Delete..."), this, SLOT(sDeleteMatl()));
          if (!_privileges->check("MaintainWoMaterials"))
            menuItem->setEnabled(false);
      }
      
      pMenu->addSeparator();

      if ((_metrics->boolean("IssueToExplodedWO") && status == "E") || status == "R" || status == "I")
      {
        menuItem = pMenu->addAction(tr("Issue..."), this, SLOT(sIssueMatl()));
        if (!_privileges->check("IssueWoMaterials"))
          menuItem->setEnabled(false);
      }
      if (status == "I")
      {
          menuItem = pMenu->addAction(tr("Return..."), this, SLOT(sReturnMatl()));
          if (!_privileges->check("ReturnWoMaterials"))
            menuItem->setEnabled(false);

          menuItem = pMenu->addAction(tr("Scrap..."), this, SLOT(sScrapMatl()));
          if (!_privileges->check("ScrapWoMaterials"))
            menuItem->setEnabled(false);
      }
      
      if ((status == "O" || status == "E") && qtyiss == 0.0)
      {
          menuItem = pMenu->addAction(tr("Substitute..."), this, SLOT(sSubstituteMatl()));
          if (!_privileges->check("MaintainWoMaterials"))
            menuItem->setEnabled(false);
      }
      
      pMenu->addSeparator();
    }

    menuItem = pMenu->addAction(tr("Availability..."), this, SLOT(sViewMatlAvailability()));

    menuItem = pMenu->addAction(tr("Substitute Availability..."), this, SLOT(sViewMatlSubstituteAvailability()));
  }

}

void workOrder::populate()
{

  XSqlQuery workpopulate;
  XSqlQuery wo;
  wo.prepare( "SELECT wo_itemsite_id, wo_priority, wo_status,"
              "       formatWoNumber(wo_id) AS f_wonumber,"
              "       wo_qtyord,"
              "       wo_qtyrcv,"
              "       wo_startdate, wo_duedate,"
              "       wo_wipvalue,"
              "       wo_postedvalue,"
              "       wo_postedvalue-wo_wipvalue AS rcvdvalue,"
              "       wo_prodnotes, wo_prj_id, "
              "       wo_bom_rev_id, wo_boo_rev_id, "
              "       wo_cosmethod "
              "FROM wo "
              "WHERE (wo_id=:wo_id);" );
  wo.bindValue(":wo_id", _woid);
  wo.exec();
  if (wo.first())
  {

    _oldPriority = wo.value("wo_priority").toInt();
    _oldStartDate = wo.value("wo_startdate").toDate();
    _oldDueDate = wo.value("wo_duedate").toDate();

    _woNumber->setText(wo.value("f_wonumber").toString());
    _item->setItemsiteid(wo.value("wo_itemsite_id").toInt());
    _priority->setValue(_oldPriority);
    _postedValue->setText(wo.value("wo_postedvalue").toDouble());
    _rcvdValue->setText(wo.value("rcvdvalue").toDouble());
    _wipValue->setText(wo.value("wo_wipvalue").toDouble());
    if (wo.value("wo_qtyord").toDouble() < 0)
      _disassembly->setChecked(true);
    _oldQty = (wo.value("wo_qtyord").toDouble() * _sense);
    _qty->setDouble(wo.value("wo_qtyord").toDouble() * _sense);
    _qtyReceived->setDouble(wo.value("wo_qtyrcv").toDouble());
    _startDate->setDate(wo.value("wo_startdate").toDate());
    _dueDate->setDate(wo.value("wo_duedate").toDate());
    _productionNotes->setText(wo.value("wo_prodnotes").toString());
    _comments->setId(_woid);
    _documents->setId(_woid);
    _project->setId(wo.value("wo_prj_id").toInt());
    _bomRevision->setId(wo.value("wo_bom_rev_id").toInt());
    _booRevision->setId(wo.value("wo_boo_rev_id").toInt());

    if (wo.value("wo_cosmethod").toString() == "D")
      _todate->setChecked(true);
    else if (wo.value("wo_cosmethod").toString() == "P")
      _proportional->setChecked(true);
    else
      _jobCosGroup->hide();

    sFillList();

    // If the W/O is Closed or Released don't allow changing some items.
    if(wo.value("wo_status").toString() == "C" || wo.value("wo_status") == "R")
    {
      _qty->setEnabled(false);
      _dueDate->setEnabled(false);
      _startDate->setEnabled(false);
    }
    else
    {
      _qty->setEnabled(true);
      _dueDate->setEnabled(true);
      _startDate->setEnabled(true);
    }
    
    _woNumber->setEnabled(false);
    _item->setReadOnly(true);
    _bomRevision->setEnabled(wo.value("wo_status").toString() == "O" && _privileges->boolean("UseInactiveRevisions"));
    _booRevision->setEnabled(wo.value("wo_status").toString() == "O" && _privileges->boolean("UseInactiveRevisions"));
    if ((_mode == cNew || _mode == cRelease) &&
       (wo.value("wo_status").toString() == "I" ||
        wo.value("wo_status").toString() == "C"))
    {
      if (_mode == cRelease)
      {
        workpopulate.prepare("SELECT releasePlannedOrder(:planord_id, false) AS result;");
        workpopulate.bindValue(":planord_id", _planordid);
        workpopulate.exec();
      }
      _mode = cEdit;
      emit newMode(_mode);
    }
    _assembly->setEnabled(false);
    _disassembly->setEnabled(false);
    _warehouse->setEnabled(false);
    if (_mode != cView)
      _comments->setReadOnly(false);
    _documents->setReadOnly(false);
    _leadTimeLit->hide();
    _leadTime->hide();
    _daysLit->hide();
    if (_mode != cNew)
      _printTraveler->hide();
    _bottomSpacer->hide();

    _save->setEnabled(true);
    
    emit populated();
  }
  else if (wo.lastError().type() != QSqlError::NoError)
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Work Order Information for WO #: %1")
                         .arg(_woid),
                         wo, __FILE__, __LINE__);
    if(_captive)
      close();
  }
}


