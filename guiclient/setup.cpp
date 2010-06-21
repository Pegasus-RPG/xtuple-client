/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */
#include <QBuffer>
#include <QUiLoader>
#include <QMessageBox>

#include "getscreen.h"
#include "setup.h"
#include "xt.h"
#include "xtreewidget.h"

setup::setup(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  QPushButton* apply = _buttonBox->button(QDialogButtonBox::Apply);
  connect(apply, SIGNAL(clicked()), this, SLOT(apply()));
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(save()));
  connect(_modules, SIGNAL(currentIndexChanged(int)), this, SLOT(populate(int)));
  connect(_tree, SIGNAL(currentItemChanged(XTreeWidgetItem*,XTreeWidgetItem*)), this, SLOT(setCurrentIndex(XTreeWidgetItem*)));

  _tree->addColumn(QString(), -1, Qt::AlignLeft, true);
  _tree->setHeaderHidden(true);
}

setup::~setup()
{
}

enum SetResponse setup::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("module", &valid);
  if (valid)
    _modules->setCurrentIndex(param.toInt());
  else
    populate();

  return NoError;
}

/*!
  Add setup \a widget with XTreewidgetItem \a parent to the list of setup items and the stacked widget with
  a\ title.  The item on the list will be enabled according to the \a enabled value.  The value of \a mode will
  determined whether parameters passed open the widget in "edit" or "view" mode.  A save function on the
  widget triggered by the Apply and Save buttons can be specified by \a saveMethod.
*/
void setup::append(XTreeWidgetItem* parent, const QString &uiName, const QString &title, bool enabled, int mode, const QString &saveMethod)
{
  QBrush disabled(Qt::gray);

  // Set the item on the list
  XTreeWidgetItem* item  = new XTreeWidgetItem(parent, mode);
  item->setData(0, Qt::DisplayRole, QVariant(title));
  item->setData(0, Xt::RawRole, QVariant(uiName));

  if (!enabled)
  {
    item->setFlags(Qt::NoItemFlags);
    item->setForeground(0,disabled);
  }
  parent->addChild(item);

  // Store the save methad name
  if (!saveMethod.isEmpty())
    _methodMap.insert(uiName, saveMethod);
}

/*!
  Saves the current metric settings and repopulates the window.
*/
void setup::apply()
{
  save(false);
  populate(_modules->currentIndex());
}

void setup::languageChange()
{
  retranslateUi(this);
}

/*!
  Returns the mode value based on the privileges granted by checking \a editPriv and
  \a viewPriv.  If the user has edit privileges cEdit (2) will be returned, if only view
  privileges then cView (3) will be returned, otherwise 0;
  */
int setup::mode(const QString &editPriv, const QString &viewPriv)
{
  if (_privileges->check(editPriv))
    return cEdit;
  else if (_privileges->check(viewPriv))
    return cView;

  return 0;
}

/*!
  Populates the list of setup widgets filtered by \a module if specified.
  */
void setup::populate(int module)
{
  _tree->clear();
  _idxmap.clear();
  while (_stack->count())
  {
    QWidget* w = _stack->widget(0);
    _stack->removeWidget(w);
    w = 0;
  }

  // Configure
  XTreeWidgetItem* configItem = new XTreeWidgetItem(_tree, 0, -1, tr("Configure"));

  if (module == All || module == Accounting)
    append(configItem, "configureGL", tr("Accounting"), mode("ConfigureGL"), 0, "sSave()");

  if (module == All || module == System)
    append(configItem, "configureCC", tr("Credit Card"), mode("ConfigureCC"), 0, "sSave()");

  if (module == All || module == CRM)
    append(configItem, "configureCRM", tr("CRM"), mode("ConfigureCRM"), 0, "sSave()" );

  if (module == All || module == Inventory)
    append(configItem, "configureIM", tr("Inventory"), mode("ConfigureIM"), 0, "sSave()" );

  if (module == All || module == System)
    append(configItem, "databaseInformation", tr("Database"), mode("ConfigDatabaseInfo"), 0, "sSave()");

  if (module == All || module == System)
  {
    append(configItem, "configureIE", tr("Import/Export"), mode("ConfigureImportExport"), 0, "sSave()");
    append(configItem, "configureEncryption", tr("Encryption"), mode("ConfigureEncryption"), 0, "sSave()");
  }

  if (module == All || module == Sales)
    append(configItem, "configureSO", tr("Sales"), mode("ConfigureSO"), 0, "sSave()" );

  if (module == All || module == Manufacture)
    append(configItem, "configureWO", tr("Manufacture"), mode("ConfigureWO"), 0, "sSave()");

  if (module == All || module == Purchase)
    append(configItem, "configurePO", tr("Purchase"), mode("ConfigurePO"), 0, "sSave()" );

  if (module == All || module == Products)
    append(configItem, "configurePD", tr("Products"), mode("ConfigurePD"), 0, "sSave()" );

  if (module == All || module == Schedule)
    append(configItem, "configureMS", tr("Schedule"), mode("ConfigureMS"), 0, "sSave()" );

  if (!configItem->childCount())
    _tree->takeTopLevelItem(_tree->indexOfTopLevelItem(configItem));

  // Account Mappings
  XTreeWidgetItem* mapItem = new XTreeWidgetItem(_tree, 0, -1, tr("Account Mappings"));
  int modeVal;

  if (module == All || module == Accounting || module == Inventory)
  {
    modeVal = mode("MaintainCostCategories", "ViewCostCategories");
    append(mapItem,"costCategories", tr("Cost Categories"), modeVal, modeVal);
  }

  if (module == All || module == Accounting || module == Inventory || module == Purchase)
  {
    modeVal = mode("MaintainExpenseCategories", "ViewExpenseCategories");
    append(mapItem, "expenseCategories", tr("Expense Categories"), modeVal, modeVal);
  }

  if (module == All || module == Accounting || module == Purchase)
  {
    modeVal = mode("MaintainVendorAccounts", "ViewVendorAccounts");
    append(mapItem, "apAccountAssignments", tr("Payables Assignments"), modeVal, modeVal);
  }

  if (module == All || module == Accounting || module == Sales)
  {
    modeVal = mode("MaintainSalesAccount", "ViewSalesAccount");
    append(mapItem, "arAccountAssignments", tr("Receivables Assignments"), modeVal, modeVal);
    append(mapItem, "salesAccounts", tr("Sales Assignments"), modeVal, modeVal);

    modeVal = mode("MaintainSalesCategories", "ViewSalesCategories");
    append(mapItem, "salesCategories", tr("Sales Categories"), modeVal, modeVal);
  }

  if (!mapItem->childCount())
    _tree->takeTopLevelItem(_tree->indexOfTopLevelItem(mapItem));

  // Master Information
  XTreeWidgetItem* masterItem = new XTreeWidgetItem(_tree, 0, -1, tr("Master Information"));

  if (module == All || module == Accounting)
  {
    modeVal = mode("MaintainBankAccounts");
    append(masterItem, "bankAccounts", tr("Bank Accounts"), modeVal, modeVal);
  }

  if (module == All || module == Accounting)
  {
    modeVal = mode("MaintainAdjustmentTypes", "ViewAdjustmentTypes");
    append(masterItem,"bankAdjustmentTypes", tr("Bank Adjustment Types"), modeVal, modeVal);
  }

  if (module == All || module == System)
  {
    modeVal = mode("MaintainCalendars");
    append(masterItem, "calendars", tr("Calendars"), modeVal, modeVal);
  }

  if (module == All || module == Products || module == Inventory || module == CRM ||
      module == Sales || module == Accounting)
  {
    modeVal = mode("MaintainCharacteristics", "ViewCharacteristics");
    append(masterItem, "characteristics", tr("Characteristics"), modeVal, modeVal);
  }

  if (module == All || module == Accounting)
  {
    modeVal = mode("MaintainCheckFormats", "ViewCheckFormats");
    append(masterItem, "checkFormats", tr("Check Formats"), modeVal, modeVal);
  }

  if (module == All || module == Products)
  {
    modeVal = mode("MaintainClassCodes", "ViewClassCodes");
    append(masterItem, "classCodes", tr("Class Codes"), modeVal, modeVal);
  }

  if (module == All || module == System)
  {
    modeVal = mode("MaintainCommentTypes");
    append(masterItem, "commentTypes", tr("Comment Types"), modeVal, modeVal);
  }

  if (module == All || module == System)
  {
    modeVal = mode("MaintainCountries");
    append(masterItem, "countries", tr("Countries"), modeVal, modeVal);
  }

  if (module == All || module == System)
  {
    modeVal = mode("CreateNewCurrency");
    append(masterItem, "currencies", tr("Currencies"), modeVal, modeVal);
  }

  if (module == All || module == Sales)
  {
    modeVal = mode("MaintainCustomerMasters");
    append(masterItem, "customerFormAssignments", tr("Customer Form Assignments"), modeVal, modeVal);
  }

  if (module == All || module == Sales || module == Accounting)
  {
    modeVal = mode("MaintainCustomerTypes", "ViewCustomerTypes");
    append(masterItem, "customerTypes", tr("Customer Types"), modeVal, modeVal);
  }

  if (module == All || module == System)
  {
    modeVal = mode("MaintainCurrencyRates", "ViewCurrencyRates");
    append(masterItem, "exchangeRates", tr("Exchange Rates"), modeVal, modeVal);
  }

  if (module == All || module == System)
  {
    modeVal = mode("ViewDepartments", "MaintainDepartments");
    append(masterItem, "departments", tr("Departments"), modeVal, modeVal);
  }

  if (module == All || module == System)
  {
    modeVal = mode("MaintainForms");
    append(masterItem, "forms", tr("Forms"), modeVal, modeVal);
  }

  if (module == All || module == Products)
  {
    modeVal = mode("MaintainFreightClasses", "ViewFreightClasses");
    append(masterItem, "freightClasses", tr("Freight Classes"), modeVal, modeVal);
  }

  if (module == All || module == System)
  {
    modeVal = mode("MaintainImages");
    append(masterItem, "images", tr("Images"), modeVal, modeVal);
  }

  if (module == All || module == CRM)
  {
    modeVal = mode("MaintainIncidentCategories");
    append(masterItem, "incidentCategories", tr("Incident Categories"), modeVal, modeVal);
  }

  if (module == All || module == CRM)
  {
    modeVal = mode("MaintainIncidentPriorities");
    append(masterItem, "incidentPriorities", tr("Incident Priorities"), modeVal, modeVal);
  }

  if (module == All || module == CRM)
  {
    modeVal = mode("MaintainIncidentResolutions");
    append(masterItem, "incidentResolutions", tr("Incident Resolutions"), modeVal, modeVal);
  }

  if (module == All || module == CRM)
  {
    modeVal = mode("MaintainIncidentSeverities");
    append(masterItem, "incidentSeverities", tr("Incident Severities"), modeVal, modeVal);
  }

  if (module == All || module == System)
  {
    modeVal = mode("MaintainForms");
    append(masterItem, "labelForms", tr("Label Forms"), modeVal, modeVal);
  }

  if (module == All || module == System)
  {
    modeVal = mode("MaintainLocales");
    append(masterItem, "locales", tr("Locales"), modeVal, modeVal);
  }

  if ((module == All || module == Products) && _metrics->boolean("LotSerialControl"))
  {
    modeVal = mode("MaintainLotSerialSequences", "ViewLotSerialSequences");
    append(masterItem, "lotSerialSequences", tr("Lot/Serial Sequences"), modeVal, modeVal);
  }

  if (module == All || module == CRM)
  {
    modeVal = mode("MaintainOpportunitySources");
    append(masterItem, "opportunitySources", tr("Opportunity Sources"), modeVal, modeVal);
  }

  if (module == All || module == CRM)
  {
    modeVal = mode("MaintainOpportunityStages");
    append(masterItem, "opportunityStages", tr("Opportunity Stages"), modeVal, modeVal);
  }

  if (module == All || module == CRM)
  {
    modeVal = mode("MaintainOpportunityTypes");
    append(masterItem, "opportunityTypes", tr("Opportunity Types"), modeVal, modeVal);
  }

  if (module == All || module == Inventory || module == Schedule || module == Purchase)
  {
    modeVal = mode("MaintainPlannerCodes", "ViewPlannerCodes");
    append(masterItem, "plannerCodes", tr("Planner Codes"), modeVal, modeVal);
  }

  if (module == All || module == Products)
  {
    modeVal = mode("MaintainProductCategories", "ViewProductCategories");
    append(masterItem, "productCategories", tr("Product Categories"), modeVal, modeVal);
  }

  if (module == All || module == Inventory || module == Accounting)
  {
    modeVal = mode("MaintainReasonCodes");
    append(masterItem, "reasonCodes", tr("Reason Codes"), modeVal, modeVal);
  }

  if (module == All || module == Purchase)
  {
    modeVal = mode("MaintainRejectCodes", "ViewRejectCodes");
    append(masterItem, "rejectCodes", tr("Reject Codes"), modeVal, modeVal);
  }

  if (module == All || module == Sales)
  {
    modeVal = mode("MaintainSalesReps", "ViewSalesReps");
    append(masterItem, "salesReps", tr("Sales Reps"), modeVal, modeVal);
  }

  if (module == All || module == System)
  {
    modeVal = mode("MaintainStates");
    append(masterItem, "states", tr("States and Provinces"), modeVal, modeVal);
  }

  if (module == All || module == Sales)
  {
    modeVal = mode("MaintainShippingChargeTypes", "ViewShippingChargeTypes");
    append(masterItem, "shippingChargeTypes", tr("Shipping Charge Types"), modeVal, modeVal);
  }

  if (module == All || module == Sales)
  {
    modeVal = mode("MaintainShippingForms", "ViewShippingForms");
    append(masterItem, "shippingForms", tr("Shipping Forms"), modeVal, modeVal);
  }

  if (module == All || module == Sales)
  {
    modeVal = mode("MaintainShippingZones", "ViewShippingZones");
    append(masterItem, "shippingZones", tr("Shipping Zones"), modeVal, modeVal);
  }

  if (module == All || module == Sales)
  {
    modeVal = mode("MaintainShipVias", "ViewShipVias");
    append(masterItem, "shipVias", tr("Ship Vias"), modeVal, modeVal);
  }

  if (module == All || module == Inventory)
  {
    modeVal = mode("MaintainSiteTypes", "ViewSiteTypes");
    append(masterItem, "siteTypes", tr("Site Types"), modeVal, modeVal);
  }

  if (module == All || module == Sales)
  {
    modeVal = mode("MaintainTaxCodes", "ViewTaxCodes");
    append(masterItem, "taxCodes", tr("Tax Codes"), modeVal, modeVal);
  }

  if (module == All || module == Purchase || module == Sales || module == Accounting)
  {
    modeVal = mode("MaintainTerms", "ViewTerms");
    append(masterItem, "termses", tr("Terms"), modeVal, modeVal);
  }

  if (module == All || module == CRM)
  {
    modeVal = mode("MaintainTitles", "ViewTitles");
    append(masterItem, "honorifics", tr("Titles"), modeVal, modeVal);
  }

  if (module == All || module == Products)
  {
    modeVal = mode("MaintainUOMs", "ViewUOMs");
    append(masterItem, "uoms", tr("Units of Measure"), modeVal, modeVal);
  }

  if (module == All || module == Purchase || module == Accounting)
  {
    modeVal = mode("MaintainVendorTypes", "ViewVendorTypes");
    append(masterItem, "vendorTypes", tr("Vendor Types"), modeVal, modeVal);
  }

  if (!masterItem->childCount())
    _tree->takeTopLevelItem(_tree->indexOfTopLevelItem(masterItem));

  _tree->expandAll();
  if (_tree->topLevelItemCount())
    setCurrentIndex(_tree->topLevelItem(0));

  //Test
  qDebug("running test");
  QList<XTreeWidgetItem *> test = _tree->findItems("e",Qt::MatchContains,0,Xt::RawRole);
  qDebug("found count:%d", test.count());
  for (int i = 0; i < test.count(); i++)
    qDebug("found:" + test.at(i)->data(0,Qt::DisplayRole).toString());
}

/*! Emits the \a saving() signal which triggers any widgets to save that have a mapped \a savedMethod()
  specified by \sa append().  Also reloads metrics, privileges, preferences, and the menubar in the
  main application.  The screen will close if \a close is true.
  */
void setup::save(bool close)
{
  emit saving();
  _metrics->load();
  _privileges->load();
  _preferences->load();
  omfgThis->initMenuBar();

  if (close)
    accept();
}

void setup::setCurrentIndex(XTreeWidgetItem* item)
{
  QString uiName = item->data(0, Xt::RawRole ).toString();
  qDebug("setting class name " + uiName );

  if (_idxmap.contains(uiName))
  {
    _stack->setCurrentIndex(_idxmap.value(uiName));
    return;
  }
  else if (!item->isDisabled())
  {
    QWidget *w = xtGetScreen(uiName, this);
    if (!w)
    {
      XSqlQuery screenq;
      screenq.prepare("SELECT * "
                      "  FROM uiform "
                      " WHERE((uiform_name=:uiform_name)"
                      "   AND (uiform_enabled))"
                      " ORDER BY uiform_order DESC"
                      " LIMIT 1;");
      screenq.bindValue(":uiform_name", uiName);
      screenq.exec();
      if (screenq.first())
      {
        QUiLoader loader;
        QByteArray ba = screenq.value("uiform_source").toByteArray();
        QBuffer uiFile(&ba);
        if (!uiFile.open(QIODevice::ReadOnly))
          QMessageBox::critical(0, tr("Could not load UI"),
                                tr("<p>There was an error loading the UI Form "
                                   "from the database."));
        w = loader.load(&uiFile);
        uiFile.close();
      }
    }

    if (w)
    {
      // Hide buttons out of context here
      QWidget* close = w->findChild<QWidget*>("_close");
      if (close)
        close->hide();
      QWidget* buttons = w->findChild<QDialogButtonBox*>();
      if (buttons)
        buttons->hide();

      //Set mode if applicable
      if (item->id() && w->inherits("XDialog"))
      {
        XWidget* x = dynamic_cast<XWidget*>(w);
        ParameterList params;
        if (item->id() == cEdit)
          params.append("mode", "edit");
        else if (item->id() == cView)
          params.append("mode", "view");
        x->set(params);
      }


      //Connect save slot if applicable
      if (_methodMap.contains(uiName))
      {
        QString method = _methodMap.value(uiName);
        connect(this, SIGNAL(saving()), w, QString("1%1").arg(method).toUtf8().data());
      }

      int idx = _stack->count();
      _idxmap.insert(uiName,idx);
      _stack->addWidget(w);
      _stack->setCurrentIndex(idx);
      return;
    }
  }
  // Nothing here so try the next one
  XTreeWidgetItem* next = dynamic_cast<XTreeWidgetItem*>(_tree->itemBelow(item));
  if (next)
    setCurrentIndex(next);
}

