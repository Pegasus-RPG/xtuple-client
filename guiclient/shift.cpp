/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "shift.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>

shift::shift(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
}

shift::~shift()
{
  // no need to delete child widgets, Qt does it all for us
}

void shift::languageChange()
{
  retranslateUi(this);
}

enum SetResponse shift::set(const ParameterList& pParams)
{
    QVariant	param;
    bool	valid;

    param = pParams.value("shift_id", &valid);
    if (valid)
    {
	_shiftid = param.toInt();
	populate();
    }

    param = pParams.value("mode", &valid);
    if (valid)
    {

	if (param.toString() == "new")
	{
    _mode = cNew;
    _number->setFocus();
    q.exec("SELECT NEXTVAL('shift_shift_id_seq') AS shift_id;");
    if (q.first())
      _shiftid =  q.value("shift_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2\n\n%3")
				.arg(__FILE__)
				.arg(__LINE__)
				.arg(q.lastError().databaseText()));
      return UndefinedError;
    }
	}
	else if (param.toString() == "edit")
	{
	    _mode = cEdit;
	    _name->setFocus();
	}
	else if (param.toString() == "view")
	{
	    _mode = cView;
	    _close->setText(tr("&Close"));
	    _number->setEnabled(false);
	    _name->setEnabled(false);
	    _save->hide();
	    _close->setFocus();
	}
    }

    return NoError;
}

void shift::sSave()
{
    QString number = _number->text().trimmed().toUpper();

    if (number.isEmpty())
    {
	QMessageBox::critical(this, tr("Cannot Save Shift"),
			      tr("You must enter a Shift Number"));
	_number->setFocus();
	return;
    }
    if (_name->text().trimmed().isEmpty())
    {
	QMessageBox::critical(this, tr("Cannot Save Shift"),
			      tr("You must enter a Shift Name"));
	_name->setFocus();
	return;
    }
    
  q.prepare( "SELECT shift_id "
             "FROM shift "
             "WHERE ( (shift_id<>:shift_id)"
             " AND (UPPER(shift_number)=UPPER(:shift_number)) );");
  q.bindValue(":shift_id", _shiftid);
  q.bindValue(":shift_number", number);
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Create Shift"),
			   tr( "A Shift with the entered number already exists."
			       "You may not create a Shift with this number." ) );
    _number->setFocus();
    return;
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

    if (_mode == cNew)
    {
	q.prepare("INSERT INTO shift "
		  "( shift_id, shift_number, shift_name ) "
		  "VALUES "
		  "( :shift_id, :shift_number, :shift_name );");
    }
    else
	if (_mode == cEdit)
	    q.prepare("UPDATE shift "
		      "SET shift_id=:shift_id, "
		      "    shift_number=:shift_number, "
		      "    shift_name=:shift_name "
		      "WHERE (shift_id=:shift_id);");
    q.bindValue(":shift_id",		_shiftid);
    q.bindValue(":shift_number",	number);
    q.bindValue(":shift_name",		_name->text().trimmed());

    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
	systemError(this, tr("A System Error occurred at %1::%2\n\n%3")
			    .arg(__FILE__)
			    .arg(__LINE__)
			    .arg(q.lastError().databaseText()));
	return;
    }

    close();
}

void shift::sClose()
{
    close();
}


void shift::populate()
{
    q.prepare("SELECT shift_number, shift_name "
	      "FROM shift "
	      "WHERE (shift_id=:shift_id);");
    q.bindValue(":shift_id", _shiftid);
    q.exec();
    if (q.first())
    {
	_number->setText(q.value("shift_number"));
	_name->setText(q.value("shift_name"));
    }
    else
	systemError(this, tr("A System Error occurred at %1::%2\n\n%3")
			    .arg(__FILE__)
			    .arg(__LINE__)
			    .arg(q.lastError().databaseText()));
}
