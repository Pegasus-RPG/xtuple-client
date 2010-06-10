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
    populate((enum Modules)param.toInt());
  else
    populate();

  return NoError;
}

/*!
  Add setup \a widget with XTreewidgetItem \a parent to the list of setup items and the stacked widget with
  a\ title.  The item on the list will be disabled if the user does not have\a privilege granted.  Optionally
  a save function on the widget triggered by the Apply and Save buttons can be specified by \a member.
*/
void setup::append(XTreeWidgetItem* parent, const QString &uiName, const QString &title, const QString &privileges, const QString &method)
{
  QBrush disabled(Qt::gray);

  // Parse the privileges
  bool enable = false;
  QStringList privlist = privileges.split(' ', QString::SkipEmptyParts);
  for (int i = 0; i < privlist.size(); ++i)
  {
    bool tb = true;
    QStringList privandlist = privlist.at(i).split('+', QString::SkipEmptyParts);
    if(privandlist.size() > 1)
    {
      for(int ii = 0; ii < privandlist.size(); ++ii)
        tb = tb && _privileges->boolean(privandlist.at(ii));
    }
    else
      tb = enable || _privileges->boolean(privlist.at(i));
    enable = enable || tb;
  }

  // Set the item on the list
  XTreeWidgetItem* item  = new XTreeWidgetItem(parent, -1);
  item->setData(0, Qt::DisplayRole, QVariant(title));
  item->setData(0, Xt::RawRole, QVariant(uiName));

  if (!enable)
  {
    item->setFlags(Qt::NoItemFlags);
    item->setForeground(0,disabled);
  }
  parent->addChild(item);

  // Store the save methad name
  if (!method.isEmpty())
    _methodMap.insert(uiName, method);
}

void setup::apply()
{
  save(false);
  populate(_modules->currentIndex());
}

void setup::languageChange()
{
  retranslateUi(this);
}

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
    append(configItem, "configureGL", tr("Accounting"), "ConfigureGL", "sSave()");

  if (module == All || module == Sales)
    append(configItem, "configureSO", tr("Sales"), "ConfigureSO", "sSave()" );

  if (module == All || module == CRM)
    append(configItem, "configureCRM", tr("CRM"), "ConfigureCRM", "sSave()" );

  if (module == All || module == Manufacture)
    append(configItem, "configureWO", tr("Manufacture"), "ConfigureWO", "sSave()");

  if (module == All || module == Purchase)
    append(configItem, "configurePO", tr("Purchase"), "ConfigurePO", "sSave()" );

  if (module == All || module == Schedule)
    append(configItem, "configureMS", tr("Schedule"), "ConfigureMS", "sSave()" );

  if (module == All || module == Inventory)
    append(configItem, "configureIM", tr("Inventory"), "ConfigureIM", "sSave()" );

  if (module == All || module == Products)
    append(configItem, "configurePD", tr("Products"), "ConfigurePD", "sSave()" );

  if (module == All || module == System)
  {
    append(configItem, "configureIE", tr("Import/Export"), "ConfigureImportExport", "sSave()");
    append(configItem, "configureEncryption", tr("Encryption"), "ConfigureEncryption", "sSave()");
    append(configItem, "configureCC", tr("Credit Card"), "ConfigureCC", "sSave()");
  }

  _tree->addTopLevelItem(configItem);

  // Account Mappings
  XTreeWidgetItem* mapItem = new XTreeWidgetItem(_tree, 0, -1, tr("Account Mappings"));

  if (module == All || module == Accounting || module == Inventory)
    append(mapItem,"costCategories", tr("Cost Categories"), "MaintainCostCategories ViewCostCategories");

  if (module == All || module == Accounting || module == Inventory || module == Purchase)
    append(mapItem, "expenseCategories", tr("Expense Categories"), "MaintainExpenseCategories ViewExpenseCategories");

  if (module == All || module == Accounting || module == Purchase)
    append(mapItem, "apAccountAssignments", tr("Payables Assignments"), "MaintainVendorAccounts ViewVendorAccounts");

  if (module == All || module == Accounting || module == Sales)
  {
    append(mapItem, "arAccountAssignments", tr("Receivables Assignments"), "MaintainSalesAccount ViewSalesAccount");
    append(mapItem, "salesCategories", tr("Sales Categories"), "MaintainSalesCategories ViewSalesCategories");
  }

  _tree->addTopLevelItem(mapItem);

  // Master Information
  XTreeWidgetItem* masterItem = new XTreeWidgetItem(_tree, 0, -1, tr("Master Information"));

  if (module == All || module == Accounting)
    append(masterItem, "bankAccounts", tr("Bank Accounts"), "MaintainBankAccounts ViewBankAccounts");

  _tree->addTopLevelItem(masterItem);

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

