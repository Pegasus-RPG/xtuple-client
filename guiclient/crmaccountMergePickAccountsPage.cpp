/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "crmaccountMergePickAccountsPage.h"

#include <QSqlError>
#include <QString>
#include <QStringList>

#include <mqlutil.h>

#include "crmaccountMerge.h"
#include "errorReporter.h"

class CrmaccountMergePickAccountsPagePrivate
{
  public:
    CrmaccountMergePickAccountsPagePrivate(QWidget *parent)
      : _parent(parent),
        _targetid_cache(-1)
    {
    }

    QList<int> _ids;
    QList<int> _ids_cache;
    QWidget    *_parent;
    int        _targetid_cache;
};

CrmaccountMergePickAccountsPage::CrmaccountMergePickAccountsPage(QWidget *parent)
  : QWizardPage(parent)
{
  setupUi(this);

  _data = new CrmaccountMergePickAccountsPagePrivate(this);

  connect(_query,               SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_sources,SIGNAL(itemSelectionChanged()), this, SLOT(sHandleButtons()));
  connect(_target,            SIGNAL(valid(bool)), this, SIGNAL(completeChanged()));

  _sources->addColumn(tr("Number"), _orderColumn, Qt::AlignLeft,  true, "crmacct_number");
  _sources->addColumn(tr("Name"),             -1, Qt::AlignLeft,  true, "crmacct_name");
  _sources->addColumn(tr("First"),            50, Qt::AlignLeft,  true, "cntct_first_name" );
  _sources->addColumn(tr("Last"),             -1, Qt::AlignLeft,  true, "cntct_last_name" );
  _sources->addColumn(tr("Phone"),           100, Qt::AlignLeft,  true, "cntct_phone" );
  _sources->addColumn(tr("Email"),           100, Qt::AlignLeft,  true, "cntct_email" );
  _sources->addColumn(tr("Address"),          -1, Qt::AlignLeft,  false, "addr_line1" );
  _sources->addColumn(tr("City"),             75, Qt::AlignLeft,  false, "addr_city" );
  _sources->addColumn(tr("State"),            50, Qt::AlignLeft,  false, "addr_state" );
  _sources->addColumn(tr("Country"),         100, Qt::AlignLeft,  false, "addr_country" );
  _sources->addColumn(tr("Postal Code"),      75, Qt::AlignLeft,  false, "addr_postalcode" );
  _sources->addColumn(tr("Customer"),  _ynColumn, Qt::AlignCenter,true,  "cust");
  _sources->addColumn(tr("Prospect"),  _ynColumn, Qt::AlignCenter,true,  "prospect");
  _sources->addColumn(tr("Vendor"),    _ynColumn, Qt::AlignCenter,true,  "vend");
  _sources->addColumn(tr("Competitor"),_ynColumn, Qt::AlignCenter,false, "competitor");
  _sources->addColumn(tr("Partner"),   _ynColumn, Qt::AlignCenter,false, "partner");
  _sources->addColumn(tr("Tax Auth."), _ynColumn, Qt::AlignCenter,false, "taxauth");
  _sources->addColumn(tr("User"),      _ynColumn, Qt::AlignCenter,false, "usr");
  _sources->addColumn(tr("Employee"),  _ynColumn, Qt::AlignCenter,false, "emp");
  _sources->addColumn(tr("Sales Rep"), _ynColumn, Qt::AlignCenter,false, "salesrep");

  _filter->append(tr("Show Inactive"),          "showInactive",           ParameterWidget::Exists);
  _filter->append(tr("Account Number Pattern"), "crmacct_number_pattern", ParameterWidget::Text);
  _filter->append(tr("Account Name Pattern"),   "crmacct_name_pattern",   ParameterWidget::Text);
  _filter->append(tr("Contact Name Pattern"),   "cntct_name_pattern",     ParameterWidget::Text);
  _filter->append(tr("Phone Pattern"),          "cntct_phone_pattern",    ParameterWidget::Text);
  _filter->append(tr("Email Pattern"),          "cntct_email_pattern",    ParameterWidget::Text);
  _filter->append(tr("Street Pattern"),         "addr_street_pattern",    ParameterWidget::Text);
  _filter->append(tr("City Pattern"),           "addr_city_pattern",      ParameterWidget::Text);
  _filter->append(tr("State Pattern"),          "addr_state_pattern",     ParameterWidget::Text);
  _filter->append(tr("Postal Code Pattern"),    "addr_postalcode_pattern",ParameterWidget::Text);
  _filter->append(tr("Country Pattern"),        "addr_country_pattern",   ParameterWidget::Text);
  _filter->applyDefaultFilterSet();

  registerField("_target", _target, "text", "currentIndexChanged(QString)");
}

CrmaccountMergePickAccountsPage::~CrmaccountMergePickAccountsPage()
{
  if (_data)
    delete _data;
}

void CrmaccountMergePickAccountsPage::cleanupPage()
{
  _data->_targetid_cache = _target->id();
  _data->_ids_cache      = _data->_ids;
}

void CrmaccountMergePickAccountsPage::initializePage()
{
  qDebug("CrmaccountMergePickAccountsPage::initializePage() entered");
  _data->_targetid_cache = _target->id();
}

bool CrmaccountMergePickAccountsPage::isComplete() const
{
  return _sources->selectedItems().size() > 1 && _target->isValid();
}

bool CrmaccountMergePickAccountsPage::validatePage()
{
  XSqlQuery begin("BEGIN;");
  XSqlQuery rollback;

  rollback.prepare("ROLLBACK;");

  QStringList deleteme;
  for (int n = 0; n < _data->_ids_cache.size(); n++)
    if (! _data->_ids.contains(_data->_ids_cache[n]))
      deleteme << QString::number(_data->_ids_cache[n]);
  if (deleteme.size() > 0)
  {
    XSqlQuery delq;
    delq.prepare(QString("DELETE FROM crmacctsel "
                         "WHERE crmacctsel_dest_crmacct_id=%1"
                         "  AND crmacctsel_src_crmacct_id IN (%2);")
                 .arg(_data->_targetid_cache).arg(deleteme.join(", ")));
    delq.exec();
    if (delq.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Clearing Old Selections"),
                           delq, __FILE__, __LINE__);
      return false;
    }
  }

  if (_target->id() != _data->_targetid_cache)
  {
    XSqlQuery updq;
    updq.prepare("UPDATE crmacctsel SET crmacctsel_dest_crmacct_id=:new,"
                 " crmacctsel_mrg_crmacct_active         = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_cntct_id_1     = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_cntct_id_2     = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_competitor_id  = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_cust_id        = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_emp_id         = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_name           = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_notes          = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_owner_username = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_parent_id      = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_partner_id     = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_prospect_id    = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_salesrep_id    = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_taxauth_id     = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_type           = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_usr_username   = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_vend_id        = (crmacctsel_src_crmacct_id=:new) "
                 " WHERE crmacctsel_dest_crmacct_id=:old;");
    updq.bindValue(":new", _target->id());
    updq.bindValue(":old", _data->_targetid_cache);
    updq.exec();
    if (updq.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Updating Selections"),
                           updq, __FILE__, __LINE__);
      return false;
    }
  }

  QStringList addme;
  for (int n = 0; n < _data->_ids.size(); n++)
    if (! _data->_ids_cache.contains(_data->_ids[n]))
      addme << QString::number(_data->_ids[n]);
  if (addme.size() > 0)
  {
    MetaSQLQuery mql("INSERT INTO crmacctsel VALUES ("
                     " <? value('srcid') ?>,"
                     " <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>"
                     ");");
    ParameterList params;
    params.append("destid", _target->id());

    QString srcidstr("srcid");

    for (int i = 0; i < addme.size(); i++)
    {
      params.remove(srcidstr);
      params.append(srcidstr, addme[i]);
      XSqlQuery insq = mql.toQuery(params);
      if (insq.lastError().type() != QSqlError::NoError)
      {
        rollback.exec();
        ErrorReporter::error(QtCriticalMsg, this, tr("Inserting New Selections"),
                             insq, __FILE__, __LINE__);
        return false;
      }
    }
  }

  XSqlQuery commit("COMMIT;");

  _data->_targetid_cache = _target->id();
  _data->_ids_cache      = _data->_ids;

  wizard()->page(crmaccountMerge::Page_PickTask)->initializePage();
  setField("_existingMerge", field("_target"));

  return true;
}

void CrmaccountMergePickAccountsPage::sFillList()
{
  QString errmsg;
  bool ok = true;
  MetaSQLQuery mql = MQLUtil::mqlLoad("crmaccounts", "detail", errmsg, &ok);
  if (!ok &&
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Getting CRM Accounts"),
                           errmsg, __FILE__, __LINE__))
    return;

  // save the current target and selection
  int prevtarget = _target->id();
  QList<XTreeWidgetItem*> items = _sources->selectedItems();
  QList<int> ids = _data->_ids;

  // repopulate
  _target->clear();
  XSqlQuery qry = mql.toQuery(_filter->parameters());
  _sources->populate(qry);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Getting CRM Account"),
                           qry, __FILE__, __LINE__))
    return;

  // restore the selection
  if (ids.size())
  {
    for (int i = _sources->topLevelItemCount() - 1; i >= 0; i--)
    {
      XTreeWidgetItem *item = dynamic_cast<XTreeWidgetItem*>(_sources->topLevelItem(i));
      if (item)
      {
        for (int j = ids.size() - 1; j >= 0; j--)
        {
          if (item->id() == ids[j])
            _sources->setCurrentItem(item, 0,
                                     QItemSelectionModel::Select |
                                     QItemSelectionModel::Rows);
            _target->append(item->id(), item->text("crmacct_number"));
        }
      }
    }
    _target->setId(prevtarget);
  }
}

void CrmaccountMergePickAccountsPage::sHandleButtons()
{
  QList<XTreeWidgetItem *> items = _sources->selectedItems();
  _target->setEnabled(items.size() > 0);

  int targetid = _target->id();

  _target->clear();
  _data->_ids.clear();
  for (int i = 0; i < items.size(); i++)
  {
    _target->append(items[i]->id(), items[i]->text("crmacct_number"));
    _data->_ids << items[i]->id();
  }
  _target->setId(targetid);

  emit completeChanged();
}
