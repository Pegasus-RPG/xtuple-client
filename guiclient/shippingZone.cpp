/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "shippingZone.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a shippingZone as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
shippingZone::shippingZone(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
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
shippingZone::~shippingZone()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void shippingZone::languageChange()
{
    retranslateUi(this);
}


void shippingZone::init()
{
}

enum SetResponse shippingZone::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("shipzone_id", &valid);
  if (valid)
  {
    _shipzoneid = param.toInt();
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
      _description->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _name->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();
      _close->setFocus();
    }
  }

  return NoError;
}

void shippingZone::sSave()
{
  if (_name->text().length() == 0)
  {
    QMessageBox::information( this, tr("No Name Entered"),
                              tr("You must enter a valid name before saving this Shipping Zone.") );
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('shipzone_shipzone_id_seq') AS shipzone_id");
    if (q.first())
      _shipzoneid = q.value("shipzone_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO shipzone "
               "(shipzone_id, shipzone_name, shipzone_descrip) "
               "VALUES "
               "(:shipzone_id, :shipzone_name, :shipzone_descrip);" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE shipzone "
               "SET shipzone_name=:shipzone_name, shipzone_descrip=:shipzone_descrip "
               "WHERE (shipzone_id=:shipzone_id);" );

  q.bindValue(":shipzone_id", _shipzoneid);
  q.bindValue(":shipzone_name", _name->text());
  q.bindValue(":shipzone_descrip", _description->text());
  q.exec();

  done(_shipzoneid);
}

void shippingZone::sCheck()
{
  _name->setText(_name->text().trimmed());
  if ( (_mode == cNew) && (_name->text().length()) )
  {
    q.prepare( "SELECT shipzone_id "
               "FROM shipzone "
               "WHERE (UPPER(shipzone_name)=UPPER(:shipzone_name));" );
    q.bindValue(":shipzone_name", _name->text());
    q.exec();
    if (q.first())
    {
      _shipzoneid = q.value("shipzone_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void shippingZone::populate()
{
  q.prepare( "SELECT shipzone_name, shipzone_descrip "
             "FROM shipzone "
             "WHERE (shipzone_id=:shipzone_id);" );
  q.bindValue(":shipzone_id", _shipzoneid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("shipzone_name").toString());
    _description->setText(q.value("shipzone_descrip").toString());
  }
}

