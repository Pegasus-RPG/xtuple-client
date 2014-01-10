/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "characteristic.h"

#include <QMessageBox>
#include <QItemSelectionModel>
#include <QSqlError>
#include <QSqlField>
#include <QSqlIndex>
#include <QSqlRecord>
#include <QSqlTableModel>
#include <QVariant>

characteristic::characteristic(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _charid = -1;

  _charoptModel = new QSqlTableModel;
  _charoptModel->setTable("charopt");
  _charoptModel->setEditStrategy(QSqlTableModel::OnManualSubmit);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_name, SIGNAL(editingFinished()), this, SLOT(sCheck()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_charoptView, SIGNAL(clicked(QModelIndex)), this, SLOT(sCharoptClicked(QModelIndex)));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
}

characteristic::~characteristic()
{
  // no need to delete child widgets, Qt does it all for us
}

void characteristic::languageChange()
{
  retranslateUi(this);
}

enum SetResponse characteristic::set(const ParameterList &pParams)
{
  XSqlQuery characteristicet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;
  
  param = pParams.value("char_id", &valid);
  if (valid)
  {
    _charid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      characteristicet.exec("SELECT NEXTVAL('char_char_id_seq') AS char_id;");
      if (characteristicet.first())
        _charid = characteristicet.value("char_id").toInt();

      sFillList();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
// TODO
//      _mask->setEnabled(FALSE);
//      _validator->setEnabled(FALSE);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _name->setEnabled(false);
      _search->setEnabled(false);
      _useGroup->setEnabled(false);
      _order->setEnabled(false);
      _mask->setEnabled(false);
      _validator->setEnabled(false);

      _items->setEnabled(FALSE);
      _customers->setEnabled(FALSE);
      _lotSerial->setEnabled(FALSE);
      _addresses->setEnabled(FALSE);
      _crmaccounts->setEnabled(FALSE);
      _contacts->setEnabled(FALSE);
      _opportunity->setEnabled(FALSE);
      _employees->setEnabled(FALSE);
      _incidents->setEnabled(false);
      _projects->setEnabled(FALSE);
      _tasks->setEnabled(FALSE);
      _quotes->setEnabled(false);
      _salesorders->setEnabled(false);
      _invoices->setEnabled(false);
      _vendors->setEnabled(false);
      _purchaseorders->setEnabled(false);
      _vouchers->setEnabled(false);

      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void characteristic::sSave()
{
  XSqlQuery characteristicSave;
  if (_name->text().trimmed().isEmpty())
  {
    QMessageBox::critical(this, tr("Missing Name"),
			  tr("<p>You must name this Characteristic before "
			     "saving it."));
    _name->setFocus();
    return;
  }
  if (! (_items->isChecked()       || _customers->isChecked() ||
	 _lotSerial->isChecked()   || _addresses->isChecked() ||
	 _crmaccounts->isChecked() || _contacts->isChecked()  ||
         _opportunity->isChecked() || _employees->isChecked() ||
         _incidents->isChecked()   || _quotes->isChecked()    ||
         _salesorders->isChecked() || _invoices->isChecked()  ||
         _vendors->isChecked()     || _purchaseorders->isChecked() ||
         _vouchers->isChecked()    || _projects->isChecked()  ||
	     _tasks->isChecked()  ))

  {
    QMessageBox::critical(this, tr("Apply Characteristic"),
			  tr("<p>You must apply this Characteristic to at "
			     "least one type of application object."));
    _items->setFocus();
    return;
  }

  QStringList values;
  for (int i = 0; i < _charoptModel->rowCount(); i++)
  {
    QString data = _charoptModel->data(_charoptModel->index(i,2), Qt::EditRole).toString();
    if (values.contains(data))
    {
      QMessageBox::critical(this, tr("Error"), tr("Option list may not contain duplicates."));
      return;
    }
    values.append(data);
  }

  if (_mode == cNew)
  {
    characteristicSave.prepare( "INSERT INTO char "
               "( char_id, char_name, char_items, char_customers, "
               "  char_contacts, char_crmaccounts, char_addresses, "
               "  char_options, char_opportunity,"
               "  char_attributes, char_lotserial, char_employees,"
               "  char_incidents, char_projects, char_tasks, "
			   "  char_quotes, char_salesorders, char_invoices,"
               "  char_vendors, char_purchaseorders, char_vouchers,"
               "  char_notes, char_mask, char_validator, char_type, "
               "  char_order, char_search ) "
               "VALUES "
               "( :char_id, :char_name, :char_items, :char_customers, "
               "  :char_contacts, :char_crmaccounts, :char_addresses, "
               "  :char_options, :char_opportunity,"
               "  :char_attributes, :char_lotserial, :char_employees,"
               "  :char_incidents, :char_projects, :char_tasks, "
               "  :char_quotes, :char_salesorders, :char_invoices,"
               "  :char_vendors, :char_purchaseorders, :char_vouchers,"
               "  :char_notes, :char_mask, :char_validator, :char_type, "
               "  :char_order, :char_search );" );

    characteristicSave.bindValue(":char_type", _type->currentIndex());
  }
  else if (_mode == cEdit)
    characteristicSave.prepare( "UPDATE char "
               "SET char_name=:char_name, char_items=:char_items, "
               "    char_customers=:char_customers, "
               "    char_contacts=:char_contacts, "
               "    char_crmaccounts=:char_crmaccounts, "
               "    char_addresses=:char_addresses, "
               "    char_options=:char_options,"
               "    char_attributes=:char_attributes, "
               "    char_opportunity=:char_opportunity,"
               "    char_lotserial=:char_lotserial,"
               "    char_employees=:char_employees,"
               "    char_incidents=:char_incidents,"
               "    char_projects=:char_projects,"
               "    char_tasks=:char_tasks,"
               "    char_quotes=:char_quotes,"
               "    char_salesorders=:char_salesorders,"
               "    char_invoices=:char_invoices,"
               "    char_vendors=:char_vendors,"
               "    char_purchaseorders=:char_purchaseorders,"
               "    char_vouchers=:char_vouchers,"
               "    char_notes=:char_notes,"
               "    char_mask=:char_mask,"
               "    char_validator=:char_validator, "
               "    char_order=:char_order, "
               "    char_search=:char_search "
               "WHERE (char_id=:char_id);" );

  characteristicSave.bindValue(":char_id", _charid);
  characteristicSave.bindValue(":char_name", _name->text());
  characteristicSave.bindValue(":char_items",       QVariant(_items->isChecked()));
  characteristicSave.bindValue(":char_customers",   QVariant(_customers->isChecked()));
  characteristicSave.bindValue(":char_crmaccounts", QVariant(_crmaccounts->isChecked()));
  characteristicSave.bindValue(":char_contacts",	   QVariant(_contacts->isChecked()));
  characteristicSave.bindValue(":char_addresses",   QVariant(_addresses->isChecked()));
  characteristicSave.bindValue(":char_options",     QVariant(FALSE));
  characteristicSave.bindValue(":char_attributes",  QVariant(FALSE));
  characteristicSave.bindValue(":char_lotserial",   QVariant(_lotSerial->isChecked()));
  characteristicSave.bindValue(":char_opportunity", QVariant(_opportunity->isChecked()));
  characteristicSave.bindValue(":char_employees",   QVariant(_employees->isChecked()));
  characteristicSave.bindValue(":char_incidents",   QVariant(_incidents->isChecked()));
  characteristicSave.bindValue(":char_projects",    QVariant(_projects->isChecked()));
  characteristicSave.bindValue(":char_tasks",       QVariant(_tasks->isChecked()));
  characteristicSave.bindValue(":char_quotes",         QVariant(_quotes->isChecked()));
  characteristicSave.bindValue(":char_salesorders",    QVariant(_salesorders->isChecked()));
  characteristicSave.bindValue(":char_invoices",       QVariant(_invoices->isChecked()));
  characteristicSave.bindValue(":char_vendors",        QVariant(_vendors->isChecked()));
  characteristicSave.bindValue(":char_purchaseorders", QVariant(_purchaseorders->isChecked()));
  characteristicSave.bindValue(":char_vouchers",       QVariant(_vouchers->isChecked()));
  characteristicSave.bindValue(":char_notes",       _description->toPlainText().trimmed());
  if (_mask->currentText().trimmed().size() > 0)
    characteristicSave.bindValue(":char_mask",        _mask->currentText());
  if (_validator->currentText().trimmed().size() > 0)
    characteristicSave.bindValue(":char_validator",   _validator->currentText());
  characteristicSave.bindValue(":char_order", _order->value());
  characteristicSave.bindValue(":char_search", QVariant(_search->isChecked()));
  characteristicSave.exec();
  if (characteristicSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, characteristicSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  _charoptModel->submitAll();

  done(_charid);
}

void characteristic::sCheck()
{
  XSqlQuery characteristicCheck;
  _name->setText(_name->text().trimmed());
  if ((_mode == cNew) && (_name->text().trimmed().length()))
  {
    characteristicCheck.prepare( "SELECT char_id "
               "FROM char "
               "WHERE (UPPER(char_name)=UPPER(:char_name));" );
    characteristicCheck.bindValue(":char_name", _name->text());
    characteristicCheck.exec();
    if (characteristicCheck.first())
    {
      _charid = characteristicCheck.value("char_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void characteristic::populate()
{
  XSqlQuery characteristicpopulate;

  characteristicpopulate.prepare( "SELECT * "
             "FROM char "
             "WHERE (char_id=:char_id);" );
  characteristicpopulate.bindValue(":char_id", _charid);
  characteristicpopulate.exec();
  if (characteristicpopulate.first())
  {
    _name->setText(characteristicpopulate.value("char_name").toString());
    _items->setChecked(characteristicpopulate.value("char_items").toBool());
    _customers->setChecked(characteristicpopulate.value("char_customers").toBool());
    _contacts->setChecked(characteristicpopulate.value("char_contacts").toBool());
    _crmaccounts->setChecked(characteristicpopulate.value("char_crmaccounts").toBool());
    _addresses->setChecked(characteristicpopulate.value("char_addresses").toBool());
    _lotSerial->setChecked(characteristicpopulate.value("char_lotserial").toBool());
    _opportunity->setChecked(characteristicpopulate.value("char_opportunity").toBool());
    _employees->setChecked(characteristicpopulate.value("char_employees").toBool());
    _incidents->setChecked(characteristicpopulate.value("char_incidents").toBool());
    _projects->setChecked(characteristicpopulate.value("char_projects").toBool());
    _tasks->setChecked(characteristicpopulate.value("char_tasks").toBool());
    _quotes->setChecked(characteristicpopulate.value("char_quotes").toBool());
    _salesorders->setChecked(characteristicpopulate.value("char_salesorders").toBool());
    _invoices->setChecked(characteristicpopulate.value("char_invoices").toBool());
    _vendors->setChecked(characteristicpopulate.value("char_vendors").toBool());
    _purchaseorders->setChecked(characteristicpopulate.value("char_purchaseorders").toBool());
    _vouchers->setChecked(characteristicpopulate.value("char_vouchers").toBool());
    _description->setText(characteristicpopulate.value("char_notes").toString());
    _mask->setText(characteristicpopulate.value("char_mask").toString());
    _validator->setText(characteristicpopulate.value("char_validator").toString());
    _type->setCurrentIndex(characteristicpopulate.value("char_type").toInt());
    _type->setEnabled(false);
    _order->setValue(characteristicpopulate.value("char_order").toInt());
    _search->setChecked(characteristicpopulate.value("char_search").toBool());
  }
  else if (characteristicpopulate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, characteristicpopulate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void characteristic::sFillList()
{
  QString filter = QString("charopt_char_id=%1").arg(_charid);
  _charoptModel->setFilter(filter);
  _charoptModel->setSort(3, Qt::AscendingOrder);
  _charoptModel->select();
  _charoptModel->setHeaderData(2, Qt::Horizontal, QVariant(tr("Value")));
  _charoptModel->setHeaderData(3, Qt::Horizontal, QVariant(tr("Order")));

  _charoptView->setModel(_charoptModel);
  _charoptView->setColumnHidden(0, true);
  _charoptView->setColumnHidden(1, true);
}

void characteristic::sNew()
{
  int row = _charoptModel->rowCount();
  _charoptModel->insertRows(row,1);
  _charoptModel->setData(_charoptModel->index(row,1), QVariant(_charid));
  _charoptModel->setData(_charoptModel->index(row,3), 0);
  QModelIndex idx = _charoptModel->index(row,0);
  _charoptView->selectionModel()->select(QItemSelection(idx, idx),
                                         QItemSelectionModel::ClearAndSelect |
                                         QItemSelectionModel::Rows);
}

void characteristic::sDelete()
{
  int row = _charoptView->selectionModel()->currentIndex().row();
  QVariant value = _charoptModel->data(_charoptModel->index(row,2));

  // Validate
  XSqlQuery qry;
  qry.prepare("SELECT charass_id "
              "FROM charass "
              "WHERE ((charass_char_id=:char_id) "
              " AND (charass_value=:value));");
  qry.bindValue(":char_id", _charid);
  qry.bindValue(":value", value);
  qry.exec();
  if (qry.first())
  {
    QMessageBox::critical(this, tr("Error"), tr("This value is in use and can not be deleted."));
    return;
  }
  else if (qry.lastError().type() != QSqlError::NoError)
  {
    systemError(this, qry.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  _charoptModel->removeRows(row, 1);
  _charoptView->setRowHidden(row, QModelIndex(), true);
}

void characteristic::sCharoptClicked(QModelIndex idx)
{
  _delete->setEnabled(idx.isValid());
}


