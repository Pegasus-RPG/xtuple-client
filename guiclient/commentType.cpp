/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "commentType.h"

#include <QMessageBox>
#include <QVariant>
#include <QSqlError>

commentType::commentType(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
}

commentType::~commentType()
{
  // no need to delete child widgets, Qt does it all for us
}

void commentType::languageChange()
{
  retranslateUi(this);
}

enum SetResponse commentType::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("cmnttype_id", &valid);
  if (valid)
  {
    _cmnttypeid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      _mode = cNew;
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
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void commentType::sSave()
{
  if (_name->text().length() == 0)
  {
    QMessageBox::information( this, tr("Cannot Save Comment Type"),
                              tr("You must enter a valid Comment Type before saving this Item Type.") );
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('cmnttype_cmnttype_id_seq') AS cmnttype_id");
    if (q.first())
      _cmnttypeid = q.value("cmnttype_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO cmnttype "
               "( cmnttype_id, cmnttype_name, cmnttype_descrip ) "
               "VALUES "
               "( :cmnttype_id, :cmnttype_name, :cmnttype_descrip );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE cmnttype "
               "SET cmnttype_name=:cmnttype_name, cmnttype_descrip=:cmnttype_descrip "
               "WHERE (cmnttype_id=:cmnttype_id);" );

  q.bindValue(":cmnttype_id", _cmnttypeid);
  q.bindValue(":cmnttype_name", _name->text());
  q.bindValue(":cmnttype_descrip", _description->text());
  q.exec();

  done(_cmnttypeid);
}

void commentType::sCheck()
{
  _name->setText(_name->text().trimmed());
  if ( (_mode == cNew) && (_name->text().length()) )
  {
    q.prepare( "SELECT cmnttype_id "
               "FROM cmnttype "
               "WHERE (UPPER(cmnttype_name)=UPPER(:cmnttype_name));" );
    q.bindValue(":cmnttype_name", _name->text());
    q.exec();
    if (q.first())
    {
      _cmnttypeid = q.value("cmnttype_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void commentType::populate()
{
  q.prepare( "SELECT cmnttype_name, cmnttype_descrip "
             "FROM cmnttype "
             "WHERE (cmnttype_id=:cmnttype_id);" );
  q.bindValue(":cmnttype_id", _cmnttypeid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("cmnttype_name"));
    _description->setText(q.value("cmnttype_descrip"));
  }
}
