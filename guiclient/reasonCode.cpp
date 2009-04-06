/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "reasonCode.h"

#include <qvariant.h>
#include <qmessagebox.h>

const char *_docTypes[] = { "ARCM", "ARDM", "RA" };

/*
 *  Constructs a reasonCode as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
reasonCode::reasonCode(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
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
reasonCode::~reasonCode()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void reasonCode::languageChange()
{
    retranslateUi(this);
}


void reasonCode::init()
{
}

enum SetResponse reasonCode::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("rsncode_id", &valid);
  if (valid)
  {
    _rsncodeid = param.toInt();
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

void reasonCode::sSave()
{
  if (_code->text().length() == 0)
  {
    QMessageBox::information( this, tr("Invalid Reason Code"),
                              tr("You must enter a valid code for this Reason Code.") );
    _code->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('rsncode_rsncode_id_seq') AS rsncode_id");
    if (q.first())
      _rsncodeid = q.value("rsncode_id").toInt();

    q.prepare( "INSERT INTO rsncode "
               "(rsncode_id, rsncode_code, rsncode_descrip, rsncode_doctype) "
               "VALUES "
               "(:rsncode_id, :rsncode_code, :rsncode_descrip, :rsncode_doctype);" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE rsncode "
               "SET rsncode_code=:rsncode_code,"
               "    rsncode_descrip=:rsncode_descrip,"
               "    rsncode_doctype=:rsncode_doctype "
               "WHERE (rsncode_id=:rsncode_id);" );

  q.bindValue(":rsncode_id", _rsncodeid);
  q.bindValue(":rsncode_code", _code->text());
  q.bindValue(":rsncode_descrip", _description->text().trimmed());
  if (!_allDocTypes->isChecked())
    q.bindValue(":rsncode_doctype", _docTypes[_docType->currentIndex()]);
  q.exec();

  done(_rsncodeid);
}

void reasonCode::sCheck()
{
  _code->setText(_code->text().trimmed());
  if ((_mode == cNew) && (_code->text().length()))
  {
    q.prepare( "SELECT rsncode_id "
               "FROM rsncode "
               "WHERE (UPPER(rsncode_code)=UPPER(:rsncode_code));" );
    q.bindValue(":rsncode_code", _code->text());
    q.exec();
    if (q.first())
    {
      _rsncodeid = q.value("rsncode_id").toInt();
      _mode = cEdit;
      populate();

      _code->setEnabled(FALSE);
    }
  }
}

void reasonCode::populate()
{
  q.prepare( "SELECT rsncode.* "
             "FROM rsncode "
             "WHERE (rsncode_id=:rsncode_id);" );
  q.bindValue(":rsncode_id", _rsncodeid);
  q.exec();
  if (q.first())
  {
    _code->setText(q.value("rsncode_code").toString());
    _description->setText(q.value("rsncode_descrip").toString());
    if (q.value("rsncode_doctype").toString() == "ARCM")
    {
      _selectedDocType->setChecked(TRUE);
      _docType->setCurrentIndex(0);
    }
    else if (q.value("rsncode_doctype").toString() == "ARDM")
    {
      _selectedDocType->setChecked(TRUE);
      _docType->setCurrentIndex(1);
    }
    else if (q.value("rsncode_doctype").toString() == "RA")
    {
      _selectedDocType->setChecked(TRUE);
      _docType->setCurrentIndex(2);
    }
    else
      _allDocTypes->setChecked(TRUE);      
  }
} 
