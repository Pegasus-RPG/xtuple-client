/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "crmaccountMergePickDataPage.h"

#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include <mqlutil.h>

#include "crmaccount.h"
#include "errorReporter.h"
#include "format.h"

static struct {
  QString  title;      // user-visible column header in xtreewidget
  int      width;      // column width in xtreewidget
  int      align;      // column alignment in xtreewidget
  QString  querycol;   // column in the query that populates the xtreewidget
  QString  mergecol;   // column in the merge-selection table (e.g. crmacctsel_mrg_crmacct_active)
  bool     multiple;   // merge can combine data from multiple records for this column
} mergeUiDescription[] = {
  { QT_TRANSLATE_NOOP("CrmaccountMergePickDataPage", "Number"),  _orderColumn, Qt::AlignLeft,   "crmacct_number", "",                                      false },
  { QT_TRANSLATE_NOOP("CrmaccountMergePickDataPage", "Name"),              -1, Qt::AlignLeft,   "crmacct_name",   "crmacctsel_mrg_crmacct_name",           false },
  { QT_TRANSLATE_NOOP("CrmaccountMergePickDataPage", "Active"),     _ynColumn, Qt::AlignCenter, "crmacct_active", "crmacctsel_mrg_crmacct_active",         false },
  { QT_TRANSLATE_NOOP("CrmaccountMergePickDataPage", "Type"),              -1, Qt::AlignLeft,   "crmacct_type",   "crmacctsel_mrg_crmacct_type",           false },
  { QT_TRANSLATE_NOOP("CrmaccountMergePickDataPage", "Primary Contact"),   -1, Qt::AlignLeft,   "primary",        "crmacctsel_mrg_crmacct_cntct_id_1",     false },
  { QT_TRANSLATE_NOOP("CrmaccountMergePickDataPage", "Secondary Contact"), -1, Qt::AlignLeft,   "secondary",      "crmacctsel_mrg_crmacct_cntct_id_2",     false },
  { QT_TRANSLATE_NOOP("CrmaccountMergePickDataPage", "Owner"),   _orderColumn, Qt::AlignLeft,   "owner",          "crmacctsel_mrg_crmacct_owner_username", false },
  { QT_TRANSLATE_NOOP("CrmaccountMergePickDataPage", "Parent"),  _orderColumn, Qt::AlignLeft,   "parent",         "crmacctsel_mrg_crmacct_parent_id",      false },
  { QT_TRANSLATE_NOOP("CrmaccountMergePickDataPage", "Customer"),   _ynColumn, Qt::AlignCenter, "cust",           "crmacctsel_mrg_crmacct_cust_id",        false },
  { QT_TRANSLATE_NOOP("CrmaccountMergePickDataPage", "Prospect"),   _ynColumn, Qt::AlignCenter, "prospect",       "crmacctsel_mrg_crmacct_prospect_id",    false },
  { QT_TRANSLATE_NOOP("CrmaccountMergePickDataPage", "Vendor"),     _ynColumn, Qt::AlignCenter, "vend",           "crmacctsel_mrg_crmacct_vend_id",        false },
  { QT_TRANSLATE_NOOP("CrmaccountMergePickDataPage", "Competitor"), _ynColumn, Qt::AlignCenter, "competitor",     "crmacctsel_mrg_crmacct_competitor_id",  false },
  { QT_TRANSLATE_NOOP("CrmaccountMergePickDataPage", "Partner"),    _ynColumn, Qt::AlignCenter, "partner",        "crmacctsel_mrg_crmacct_partner_id",     false },
  { QT_TRANSLATE_NOOP("CrmaccountMergePickDataPage", "Tax Auth."),  _ynColumn, Qt::AlignCenter, "taxauth",        "crmacctsel_mrg_crmacct_taxauth_id",     false },
  { QT_TRANSLATE_NOOP("CrmaccountMergePickDataPage", "User"),    _orderColumn, Qt::AlignCenter, "usr",            "crmacctsel_mrg_crmacct_usr_username",   false },
  { QT_TRANSLATE_NOOP("CrmaccountMergePickDataPage", "Employee"),   _ynColumn, Qt::AlignCenter, "emp",            "crmacctsel_mrg_crmacct_emp_id",         false },
  { QT_TRANSLATE_NOOP("CrmaccountMergePickDataPage", "Sales Rep"),  _ynColumn, Qt::AlignCenter, "salesrep",       "crmacctsel_mrg_crmacct_salesrep_id",    false },
  { QT_TRANSLATE_NOOP("CrmaccountMergePickDataPage", "Notes"),             -1, Qt::AlignLeft,   "notes",          "crmacctsel_mrg_crmacct_notes",          true  }
};

class CrmaccountMergePickDataPagePrivate {
  public:
    CrmaccountMergePickDataPagePrivate(QWidget *parent = 0)
      : _destid(-1),
        _itemForMenu(0),
        _parent(parent)
    {
    }

    static bool isSelected(XTreeWidgetItem *item, int col)
    {
      return item->data(col, Qt::ForegroundRole).isValid();
    }

    int              _destid;
    XTreeWidgetItem *_itemForMenu;
    QWidget         *_parent;
};

CrmaccountMergePickDataPage::CrmaccountMergePickDataPage(QWidget *parent)
  : QWizardPage(parent),
    _data(0)
{
  setupUi(this);

  _data = new CrmaccountMergePickDataPagePrivate(this);

  for (unsigned int i = 0; i < sizeof(mergeUiDescription) / sizeof(mergeUiDescription[0]); i++)
    _sources->addColumn(mergeUiDescription[i].title, mergeUiDescription[i].width,
                        mergeUiDescription[i].align,  true, mergeUiDescription[i].querycol);

  connect(_deselect, SIGNAL(clicked()), this, SLOT(sDeselect()));
  connect(_select,   SIGNAL(clicked()), this, SLOT(sSelect()));
  connect(_sources, SIGNAL(populateMenu(QMenu *, XTreeWidgetItem *)), this, SLOT(sPopulateMenu(QMenu *, XTreeWidgetItem *)));

  setCommitPage(true);
}

CrmaccountMergePickDataPage::~CrmaccountMergePickDataPage()
{
  if (_data)
    delete _data;
}

void CrmaccountMergePickDataPage::cleanupPage()
{
  disconnect(omfgThis, SIGNAL(crmAccountsUpdated(int)), this, SLOT(sFillList()));
}

void CrmaccountMergePickDataPage::initializePage()
{
  XSqlQuery getq;
  getq.prepare("SELECT crmacct_id FROM crmacct WHERE crmacct_number=:number;");
  getq.bindValue(":number", field("_existingMerge").toString());
  getq.exec();
  if (getq.first())
    _data->_destid = getq.value("crmacct_id").toInt();
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting CRM Account"),
                                getq, __FILE__, __LINE__))
    return;

  sFillList();

  connect(omfgThis, SIGNAL(crmAccountsUpdated(int)), this, SLOT(sFillList()));
}

bool CrmaccountMergePickDataPage::isComplete() const
{
  for (int col = 0; col < _sources->columnCount(); col++)
  {
    bool found = false;
    for (int row = 0; ! found && row < _sources->topLevelItemCount(); row++)
      found = _data->isSelected(_sources->topLevelItem(row), col);
    if (! found)
      return false;
  }

  return true;
}

bool CrmaccountMergePickDataPage::validatePage()
{
  if (QMessageBox::question(this, tr("Perform this merge?"),
                                tr("Are you sure you want to merge the "
                                   "CRM Accounts as described here?"),
                                QMessageBox::No | QMessageBox::Default,
                                QMessageBox::Yes) == QMessageBox::No)
    return false;

  XSqlQuery mrgq;
  mrgq.prepare("SELECT mergecrmaccts(crmacctsel_dest_crmacct_id, FALSE)"
               "  FROM crmacctsel"
               " WHERE (crmacctsel_dest_crmacct_id=:destid);");
  mrgq.bindValue(":destid", _data->_destid);
  mrgq.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Merging"),
                              mrgq, __FILE__, __LINE__))
    return false;

  disconnect(omfgThis, SIGNAL(crmAccountsUpdated(int)), this, SLOT(sFillList()));
  return true;
}

void CrmaccountMergePickDataPage::sDeselect()
{
  QColor invalidColor;
  MetaSQLQuery srcm("UPDATE crmacctsel"
                    "   SET <? literal('colname') ?>=<? value('value') ?>"
                    " WHERE (crmacctsel_src_crmacct_id=<? value('srcid') ?>);");

  foreach(QModelIndex cell, _sources->selectionModel()->selectedIndexes())
  {
    ParameterList params;
    params.append("colname", mergeUiDescription[cell.column()].mergecol);
    params.append("value",   QVariant(false));
    params.append("srcid",   _sources->topLevelItem(cell.row())->id());
    XSqlQuery srcq = srcm.toQuery(params);
    ErrorReporter::error(QtCriticalMsg, this, tr("Updating Merge Sources"),
                         srcq, __FILE__, __LINE__);
  }
  _sources->selectionModel()->clear();
  sFillList();
}

bool CrmaccountMergePickDataPage::sDelete()
{
  if (QMessageBox::question(this, tr("Delete CRM Account?"),
                            tr("Are you sure you want to delete CRM Account %1?")
                               .arg(_sources->currentItem()->text("crmacct_number")),
                            QMessageBox::No | QMessageBox::Default,
                            QMessageBox::Yes) == QMessageBox::No)
    return false;

  XSqlQuery begin("BEGIN;");
  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  XSqlQuery delq;
  delq.prepare("DELETE FROM crmacctsel WHERE crmacctsel_src_crmacct_id=:id;");
  delq.bindValue(":id", _data->_itemForMenu->id());
  delq.exec();
  if (delq.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Merge Data"),
                         delq, __FILE__, __LINE__);
    return false;
  }

  delq.prepare("DELETE FROM crmacct WHERE crmacct_id=:id;");
  delq.bindValue(":id", _data->_itemForMenu->id());
  delq.exec();
  if (delq.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting CRM Account"),
                         delq, __FILE__, __LINE__);
    return false;
  }

  XSqlQuery commit("COMMIT;");

  omfgThis->sCrmAccountsUpdated(_data->_itemForMenu->id());
  return true;
}

void CrmaccountMergePickDataPage::sEdit()
{
  ParameterList params;
  params.append("mode",       "edit");
  params.append("crmacct_id", _data->_itemForMenu->id());

  crmaccount *newdlg = new crmaccount();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void CrmaccountMergePickDataPage::sFillList()
{
  QString errmsg;
  bool    ok = false;
  MetaSQLQuery mql = MQLUtil::mqlLoad("crmaccountmerge", "pickdatasources", errmsg, &ok);
  if (!ok &&
      ErrorReporter::error(QtCriticalMsg, this, tr("Getting CRM Accounts"),
                           errmsg, __FILE__, __LINE__))
    return;

  ParameterList params;
  params.append("destid",       _data->_destid);
  params.append("individual",   _individual->text());
  params.append("organization", _organization->text());
  params.append("unknown",      tr("[Unknown]"));

  XSqlQuery getq;
  getq = mql.toQuery(params);
  _sources->populate(getq, true);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting CRM Account"),
                           getq, __FILE__, __LINE__))
    return;

  sHandlePreview();
}

void CrmaccountMergePickDataPage::sHandlePreview()
{
  _notes->clear();
  for (int row = _sources->topLevelItemCount() - 1; row >= 0; row--)
  {
    XTreeWidgetItem *item = _sources->topLevelItem(row);

    // TODO: how can we loop over mergeUiDescription here?
    if (_data->isSelected(item, _sources->column("crmacct_number")))
      _number->setText(item->data(_sources->column("crmacct_number"), Qt::DisplayRole));

    if (_data->isSelected(item, _sources->column("crmacct_name")))
      _name->setText(item->data(_sources->column("crmacct_name"), Qt::DisplayRole));

    if (_data->isSelected(item, _sources->column("crmacct_active")))
      _active->setChecked(item->rawValue("crmacct_active").toBool());

    if (_data->isSelected(item, _sources->column("primary")))
      _primary->setId(item->data(_sources->column("primary"), Xt::IdRole).toInt());
    if (_data->isSelected(item, _sources->column("secondary")))
      _secondary->setId(item->data(_sources->column("secondary"), Xt::IdRole).toInt());
    if (_data->isSelected(item, _sources->column("parent")))
      _parent->setId(item->data(_sources->column("parent"), Xt::IdRole).toInt());
    if (_data->isSelected(item, _sources->column("owner")))
      _owner->setUsername(item->data(_sources->column("owner"),
                                     Qt::DisplayRole).toString());
    if (_data->isSelected(item, _sources->column("cust")))
      _cust->setId(item->data(_sources->column("cust"), Xt::IdRole).toInt());
    if (_data->isSelected(item, _sources->column("prospect")))
      _prospect->setId(item->data(_sources->column("prospect"), Xt::IdRole).toInt());
    if (_data->isSelected(item, _sources->column("vend")))
      _vendor->setId(item->data(_sources->column("vend"), Xt::IdRole).toInt());
    if (_data->isSelected(item, _sources->column("taxauth")))
      _taxauth->setId(item->data(_sources->column("taxauth"), Xt::IdRole).toInt());
    if (_data->isSelected(item, _sources->column("usr")))
      _user->setUsername(item->data(_sources->column("usr"), Xt::IdRole).toString());
    if (_data->isSelected(item, _sources->column("emp")))
      _emp->setId(item->data(_sources->column("emp"), Xt::IdRole).toInt());
    if (_data->isSelected(item, _sources->column("salesrep")))
      _salesrep->setId(item->data(_sources->column("salesrep"), Xt::IdRole).toInt());
    if (_data->isSelected(item, _sources->column("competitor")))
      _competitor->setChecked(item->rawValue("competitor").toBool());
    if (_data->isSelected(item, _sources->column("partner")))
      _partner->setChecked(item->rawValue("partner").toBool());

    if (_data->isSelected(item, _sources->column("crmacct_type")))
    {
      if (item->rawValue("crmacct_type").toString() == "I")
        _individual->setChecked(true);
      else if (item->rawValue("crmacct_type").toString() == "O")
        _organization->setChecked(true);
    }

    if (_data->isSelected(item, _sources->column("notes")))
      _notes->setPlainText(_notes->toPlainText()
                           + (_notes->toPlainText().trimmed().isEmpty() ? "" : "\n")
                           + item->data(_sources->column("notes"),
                                        Qt::ToolTipRole).toString());
  }

  emit completeChanged();
}

void CrmaccountMergePickDataPage::sPopulateMenu(QMenu *pMenu, XTreeWidgetItem *pItem)
{
  Q_UNUSED(pItem);
  _data->_itemForMenu = pItem;

  QAction *menuItem;
  menuItem = pMenu->addAction(tr("Edit CRM Account"), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainAllCRMAccounts"));

  menuItem = pMenu->addAction(tr("View CRM Account"), this, SLOT(sView()));
  menuItem->setEnabled(_privileges->check("ViewAllCRMAccounts"));

  menuItem = pMenu->addAction(tr("Delete CRM Account"), this, SLOT(sDelete()));
  menuItem->setEnabled(pItem->id() != _data->_destid &&
                       _privileges->check("MaintainAllCRMAccounts"));
  qDebug("_sources->id(): %d, _data->_destid: %d",
         _sources->id(), _data->_destid);
}

void CrmaccountMergePickDataPage::sSelect()
{
  QColor invalidColor;
  MetaSQLQuery srcm("UPDATE crmacctsel"
                    "   SET <? literal('colname') ?>=<? value('srcval') ?>"
                    " WHERE (crmacctsel_src_crmacct_id=<? value('srcid') ?>);");
  MetaSQLQuery destm("UPDATE crmacctsel"
                     "   SET <? literal('colname') ?>=<? value('destval') ?>"
                     " WHERE ((crmacctsel_dest_crmacct_id=<? value('destid') ?>)"
                     "    AND (crmacctsel_src_crmacct_id!=<? value('srcid') ?>));");

  foreach(QModelIndex cell, _sources->selectionModel()->selectedIndexes())
  {
    if (mergeUiDescription[cell.column()].mergecol.isEmpty())
      continue;

    ParameterList params;
    params.append("colname", mergeUiDescription[cell.column()].mergecol);
    params.append("srcval",  QVariant(true));
    params.append("srcid",   _sources->topLevelItem(cell.row())->id());
    params.append("destval", QVariant(false));
    params.append("destid",  _sources->topLevelItem(cell.row())->altId());

    XSqlQuery srcq = srcm.toQuery(params);
    ErrorReporter::error(QtCriticalMsg, this, tr("Updating Merge Sources"),
                         srcq, __FILE__, __LINE__);

    if (! mergeUiDescription[cell.column()].multiple)
    {
      XSqlQuery destq = destm.toQuery(params);
      ErrorReporter::error(QtCriticalMsg, this, tr("Updating Merge Destination"),
                           destq, __FILE__, __LINE__);
    }
  }
  _sources->selectionModel()->clear();

  sFillList();
}

void CrmaccountMergePickDataPage::sView()
{
  ParameterList params;
  params.append("mode",       "view");
  params.append("crmacct_id", _data->_itemForMenu->id());

  crmaccount *newdlg = new crmaccount();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}
