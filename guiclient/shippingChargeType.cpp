/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "shippingChargeType.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a shippingChargeType as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
shippingChargeType::shippingChargeType(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
shippingChargeType::~shippingChargeType()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void shippingChargeType::languageChange()
{
    retranslateUi(this);
}


void shippingChargeType::init()
{
}

enum SetResponse shippingChargeType::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("shipchrg_id", &valid);
  if (valid)
  {
    _shipchrgid = param.toInt();
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

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _customerFreight->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void shippingChargeType::sCheck()
{
  _name->setText(_name->text().trimmed());
  if ((_mode == cNew) && (_name->text().trimmed().length()))
  {
    q.prepare( "SELECT shipchrg_id "
               "FROM shipchrg "
               "WHERE (UPPER(shipchrg_name)=UPPER(:shipchrg_name));" );
    q.bindValue(":shipchrg_name", _name->text());
    q.exec();
    if (q.first())
    {
      _shipchrgid = q.value("shipchrg_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void shippingChargeType::sSave()
{
  if (_name->text().length() == 0)
  {
      QMessageBox::warning( this, tr("Cannot Save Shipping Charge"),
                            tr("You must enter a valid Name.") );
      return;
  }
  
  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('shipchrg_shipchrg_id_seq') AS shipchrg_id;");
    if (q.first())
      _shipchrgid = q.value("shipchrg_id").toInt();

    q.prepare( "INSERT INTO shipchrg "
               "(shipchrg_id, shipchrg_name, shipchrg_descrip, shipchrg_custfreight) "
               "VALUES "
               "(:shipchrg_id, :shipchrg_name, :shipchrg_descrip, :shipchrg_custfreight);" );
  }
  else if (_mode == cEdit)
  {
    q.prepare( "SELECT shipchrg_id "
               "FROM shipchrg "
               "WHERE ( (shipchrg_id<>:shipchrg_id)"
               " AND (UPPER(shipchrg_name)=UPPER(:shipchrg_name)) );" );
    q.bindValue(":shipchrg_id", _shipchrgid);
    q.bindValue(":shipchrg_name", _name->text());
    q.exec();
    if (q.first())
    {
      QMessageBox::critical( this, tr("Cannot Save Shipping Charge Type"),
                             tr( "The new Shipping Charge Type information cannot be saved as the new Shipping Charge Type that you\n"
                                 "entered conflicts with an existing Shipping Charge Type.  You must uniquely name this Shipping Charge Type\n"
                                 "before you may save it." ) );
      return;
    }

    q.prepare( "UPDATE shipchrg "
               "SET shipchrg_name=:shipchrg_name, shipchrg_descrip=:shipchrg_descrip,"
               "    shipchrg_custfreight=:shipchrg_custfreight "
               "WHERE (shipchrg_id=:shipchrg_id);" );
  }

  q.bindValue(":shipchrg_id", _shipchrgid);
  q.bindValue(":shipchrg_name", _name->text().trimmed());
  q.bindValue(":shipchrg_descrip", _description->text().trimmed());
  q.bindValue(":shipchrg_custfreight", QVariant(_customerFreight->isChecked()));
  q.exec();

  done(_shipchrgid);
}

void shippingChargeType::populate()
{
  q.prepare( "SELECT shipchrg_name, shipchrg_descrip, shipchrg_custfreight "
             "FROM shipchrg "
             "WHERE (shipchrg_id=:shipchrg_id);" );
  q.bindValue(":shipchrg_id", _shipchrgid);
  q.exec();
  if (q.first()) 
  {
    _name->setText(q.value("shipchrg_name").toString());
    _description->setText(q.value("shipchrg_descrip").toString());
    _customerFreight->setChecked(q.value("shipchrg_custfreight").toBool());
  }
}
