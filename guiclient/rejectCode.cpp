/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "rejectCode.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a rejectCode as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
rejectCode::rejectCode(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_code, SIGNAL(lostFocus()), this, SLOT(sCheck()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
rejectCode::~rejectCode()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void rejectCode::languageChange()
{
    retranslateUi(this);
}


void rejectCode::init()
{
}

enum SetResponse rejectCode::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("rjctcode_id", &valid);
  if (valid)
  {
    _rjctcodeid = param.toInt();
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
      _description->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _code->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();
      _close->setFocus();
    }
  }

  return NoError;
}

void rejectCode::sSave()
{
  if (_code->text().length() == 0)
  {
    QMessageBox::information( this, tr("Invalid Reject Code"),
                              tr("You must enter a valid code for this Reject Code.") );
    _code->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('rjctcode_rjctcode_id_seq') AS rjctcode_id");
    if (q.first())
      _rjctcodeid = q.value("rjctcode_id").toInt();

    q.prepare( "INSERT INTO rjctcode "
               "(rjctcode_id, rjctcode_code, rjctcode_descrip) "
               "VALUES "
               "(:rjctcode_id, :rjctcode_code, :rjctcode_descrip);" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE rjctcode "
               "SET rjctcode_code=:rjctcode_code,"
               "    rjctcode_descrip=:rjctcode_descrip "
               "WHERE (rjctcode_id=:rjctcode_id);" );

  q.bindValue(":rjctcode_id", _rjctcodeid);
  q.bindValue(":rjctcode_code", _code->text());
  q.bindValue(":rjctcode_descrip", _description->text().trimmed());
  q.exec();

  done(_rjctcodeid);
}

void rejectCode::sCheck()
{
  _code->setText(_code->text().trimmed());
  if ((_mode == cNew) && (_code->text().length()))
  {
    q.prepare( "SELECT rjctcode_id "
               "FROM rjctcode "
               "WHERE (UPPER(rjctcode_code)=UPPER(:rjctcode_code));" );
    q.bindValue(":rjctcode_code", _code->text());
    q.exec();
    if (q.first())
    {
      _rjctcodeid = q.value("rjctcode_id").toInt();
      _mode = cEdit;
      populate();

      _code->setEnabled(FALSE);
    }
  }
}

void rejectCode::populate()
{
  q.prepare( "SELECT rjctcode_code, rjctcode_descrip "
             "FROM rjctcode "
             "WHERE (rjctcode_id=:rjctcode_id);" );
  q.bindValue(":rjctcode_id", _rjctcodeid);
  q.exec();
  if (q.first())
  {
    _code->setText(q.value("rjctcode_code").toString());
    _description->setText(q.value("rjctcode_descrip").toString());
  }
} 
