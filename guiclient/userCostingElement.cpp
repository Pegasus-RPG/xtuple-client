/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "userCostingElement.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a userCostingElement as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
userCostingElement::userCostingElement(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_acceptPO, SIGNAL(toggled(bool)), _useCostItem, SLOT(setDisabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
userCostingElement::~userCostingElement()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void userCostingElement::languageChange()
{
    retranslateUi(this);
}


void userCostingElement::init()
{
  _item->setType(ItemLineEdit::cCosting);
}

enum SetResponse userCostingElement::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("costelem_id", &valid);
  if (valid)
  {
    _costelemid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _name->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _name->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _name->setEnabled(FALSE);
      _active->setEnabled(FALSE);
      _acceptPO->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();
      _close->setFocus();
    }
  }

  return NoError;
}

void userCostingElement::sSave()
{
  if (_mode == cNew)
  {
    if (_name->text().length() == 0)
    {
      QMessageBox::critical( this, tr("Cannot Save Costing Element"),
                             tr( "You must enter a Name for this Costing Element." ) );
      _name->setFocus();
      return;
    }
    
    q.prepare( "SELECT costelem_id "
               "FROM costelem "
               "WHERE (costelem_type=:costelem_type);" );
    q.bindValue(":costelem_type", _name->text());
    q.exec();
    if (q.first())
    {
      QMessageBox::critical( this, tr("Cannot Save Costing Element"),
                             tr( "A Costing Elements with the entered code already exists.\n"
                                 "You may not create a Costing Element with this code." ) );
      _name->setFocus();
      return;
    }

    q.exec("SELECT NEXTVAL('costelem_costelem_id_seq') AS _costelem_id");
    if (q.first())
      _costelemid = q.value("_costelem_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO costelem "
               "( costelem_id, costelem_type, costelem_active,"
               "  costelem_sys, costelem_po, costelem_cost_item_id ) "
               "VALUES "
               "( :costelem_id, :costelem_type, :costelem_active,"
               "  FALSE, :costelem_po, :costelem_cost_item_id );" );
  }
  else if (_mode == cEdit)
  {
    q.prepare( "SELECT costelem_id "
               "FROM costelem "
               "WHERE ( (costelem_id <> :costelem_id)"
               "AND (costelem_type=:costelem_type) );" );
    q.bindValue(":costelem_id", _costelemid);
    q.bindValue(":costelem_type", _name->text());
    q.exec();
    if (q.first())
    {
      QMessageBox::critical( this, tr("Cannot Save Costing Element"),
                             tr( "A Costing Elements with the entered code already exists.\n"
                                 "You may not create a Costing Element with this code." ) );
      _name->setFocus();
      return;
    }

    q.prepare( "UPDATE costelem "
               "SET costelem_type=:costelem_type,"
               "    costelem_active=:costelem_active, costelem_po=:costelem_po,"
               "    costelem_cost_item_id=:costelem_cost_item_id "
               "WHERE (costelem_id=:costelem_id);" );
  }

  q.bindValue(":costelem_id", _costelemid);
  q.bindValue(":costelem_type", _name->text());
  q.bindValue(":costelem_active", QVariant(_active->isChecked()));
  q.bindValue(":costelem_po", QVariant(_acceptPO->isChecked()));

  if (_useCostItem->isChecked())
    q.bindValue(":costelem_cost_item_id", _item->id());
  else
    q.bindValue(":costelem_cost_item_id", -1);

  q.exec();

  done(_costelemid);
}

void userCostingElement::populate()
{
  q.prepare( "SELECT costelem_type, costelem_active,"
             "       costelem_po, costelem_cost_item_id "
             "FROM costelem "
             "WHERE (costelem_id=:costelem_id);" );
  q.bindValue(":costelem_id", _costelemid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("costelem_type").toString());
    _active->setChecked(q.value("costelem_active").toBool());

    if (q.value("costelem_po").toBool())
      _acceptPO->setChecked(TRUE);
    else
    {
      if (q.value("costelem_cost_item_id").toInt() != -1)
      {
        _useCostItem->setChecked(TRUE);
        _item->setId(q.value("costelem_cost_item_id").toInt());
      }
    }
  }
}

void userCostingElement::sCheck()
{
  _name->setText(_name->text().trimmed());
  if ((_mode == cNew) && (_name->text().length()))
  {
    q.prepare( "SELECT costelem_id "
               "FROM costelem "
               "WHERE (UPPER(costelem_type)= UPPER(:costelem_type));" );
    q.bindValue(":costelem_type", _name->text());
    q.exec();
    if (q.first())
    {
      _costelemid = q.value("costelem_id").toInt();
      _mode = cEdit;
      populate();
    }
  }
}

