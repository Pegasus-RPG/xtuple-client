/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "laborRate.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <qvalidator.h>

/*
 *  Constructs a laborRate as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
laborRate::laborRate(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_code, SIGNAL(lostFocus()), this, SLOT(sCheck()));

    _rate->setValidator(omfgThis->moneyVal());

    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
laborRate::~laborRate()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void laborRate::languageChange()
{
    retranslateUi(this);
}


void laborRate::init()
{
}

enum SetResponse laborRate::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("lbrrate_id", &valid);
  if (valid)
  {
    _lbrrateid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _code->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _code->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _rate->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void laborRate::sCheck()
{
  _code->setText(_code->text().trimmed());
  if ((_mode == cNew) && (_code->text().length()))
  {
    q.prepare( "SELECT lbrrate_id "
               "FROM lbrrate "
               "WHERE (UPPER(lbrrate_code)=UPPER(:lbrrate_code));" );
    q.bindValue(":lbrrate_code", _code->text());
    q.exec();
    if (q.first())
    {
      _lbrrateid = q.value("lbrrate_id").toInt();
      _mode = cEdit;
      populate();

      _code->setEnabled(FALSE);
    }
  }
}

void laborRate::sSave()
{
  if (_code->text().length() == 0)
  {
    QMessageBox::critical( this, tr("Cannot Add Labor Rate"),
                           tr("You must enter a Code for the new Labor Rate.") );
    _code->setFocus();
    return;
  }

  if (_rate->toDouble() == 0)
  {
    QMessageBox::critical( this, tr("Cannot Add Labor Rate"),
                           tr("You must enter a Rate for the new Labor Rate.") );
    _rate->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('lbrrate_lbrrate_id_seq') AS lbrrate_id;");
    if (q.first())
      _lbrrateid = q.value("lbrrate_id").toInt();

    q.prepare( "INSERT INTO lbrrate "
               "(lbrrate_id, lbrrate_code, lbrrate_descrip, lbrrate_rate) "
               "VALUES "
               "(:lbrrate_id, :lbrrate_code, :lbrrate_descrip, :lbrrate_rate);" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE lbrrate "
               "SET lbrrate_code=:lbrrate_code, lbrrate_descrip=:lbrrate_descrip,"
               "    lbrrate_rate=:lbrrate_rate "
               "WHERE (lbrrate_id=:lbrrate_id);" );

  q.bindValue(":lbrrate_id", _lbrrateid);
  q.bindValue(":lbrrate_code", _code->text());
  q.bindValue(":lbrrate_descrip", _description->text().trimmed());
  q.bindValue(":lbrrate_rate", _rate->toDouble());
  q.exec();

  done(_lbrrateid);
}

void laborRate::populate()
{
  q.prepare( "SELECT lbrrate_code, lbrrate_descrip,"
             "       lbrrate_rate "
             "FROM lbrrate "
             "WHERE (lbrrate_id=:lbrrate_id);" );
  q.bindValue(":lbrrate_id", _lbrrateid);
  q.exec();
  if (q.first())
  {
    _code->setText(q.value("lbrrate_code").toString());
    _description->setText(q.value("lbrrate_descrip").toString());
    _rate->setDouble(q.value("lbrrate_rate").toDouble());
  }
}

