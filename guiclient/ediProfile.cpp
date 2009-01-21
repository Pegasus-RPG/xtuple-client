/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "ediProfile.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include "ediForm.h"

/*
 *  Constructs a ediProfile as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
ediProfile::ediProfile(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_cancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_type, SIGNAL(activated(int)), _stack, SLOT(raiseWidget(int)));
    connect(_accept, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_forms, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_forms, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_forms, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
ediProfile::~ediProfile()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void ediProfile::languageChange()
{
    retranslateUi(this);
}


void ediProfile::init()
{
  _mode = cNew;
  _ediprofileid = -1;

  _forms->addColumn(tr("Type"),           -1, Qt::AlignLeft, true, "ediform_type" );
  _forms->addColumn(tr("Output"), _itemColumn, Qt::AlignLeft, true, "ediform_output" );
}

enum SetResponse ediProfile::set( const ParameterList & pParams )
{
  QVariant param;
  bool     valid;

  param = pParams.value("ediprofile_id", &valid);
  if (valid)
  {
    _ediprofileid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      _mode = cNew;
    else if (param.toString() == "edit")
      _mode = cEdit;
  }

  return NoError;
}

bool ediProfile::save()
{
  if(_name->text().trimmed().isEmpty())
  {
    QMessageBox::critical( this, tr("Cannot Save EDI Profile"),
      tr("You must enter in a valid name for this profile.") );
    _name->setFocus();
    return false;
  }

  if(cNew == _mode)
    q.prepare("SELECT ediprofile_id"
              "  FROM ediprofile"
              " WHERE (ediprofile_name=:ediprofile_name); ");
  else if(cEdit == _mode)
    q.prepare("SELECT ediprofile_id"
              "  FROM ediprofile"
              " WHERE ((ediprofile_name=:ediprofile_name)"
              "   AND  (ediprofile_id!=:ediprofile_id)); ");
  q.bindValue(":ediprofile_id", _ediprofileid);
  q.bindValue(":ediprofile_name", _name->text().trimmed());
  q.exec();
  if(q.first())
  {
    QMessageBox::critical( this, tr("Cannot Save EDI Profile"),
      tr("The name you have entered is already in the system. Please choose a different name.") );
    _name->setFocus();
    return false;
  }

  // TODO: Add specific checking based on type

  if( (cNew == _mode) && (-1 == _ediprofileid) )
  {
    q.prepare("SELECT nextval('ediprofile_ediprofile_id_seq') AS result; ");
    q.exec();
    if(q.first())
      _ediprofileid = q.value("result").toInt();
    else
    {
      QMessageBox::critical( this, tr("Cannot Save EDI Profile"),
        tr("There was a database error preventing this record from being saved.") );
      return false;
    }
  }

  if(cNew == _mode)
    q.prepare("INSERT INTO ediprofile "
              "(ediprofile_id, ediprofile_name, ediprofile_notes,"
              " ediprofile_type, ediprofile_option1, ediprofile_option2,"
              " ediprofile_option3, ediprofile_option4, ediprofile_option5,"
              " ediprofile_emailhtml) "
              "VALUES(:ediprofile_id, :ediprofile_name, :ediprofile_notes,"
              " :ediprofile_type, :ediprofile_option1, :ediprofile_option2,"
              " :ediprofile_option3, :ediprofile_option4, :ediprofile_option5,"
              " :ediprofile_emailhtml);" );
  else
    q.prepare("UPDATE ediprofile"
              "   SET ediprofile_name=:ediprofile_name,"
              "       ediprofile_notes=:ediprofile_notes,"
              "       ediprofile_type=:ediprofile_type,"
              "       ediprofile_option1=:ediprofile_option1,"
              "       ediprofile_option2=:ediprofile_option2,"
              "       ediprofile_option3=:ediprofile_option3,"
              "       ediprofile_option4=:ediprofile_option4,"
              "       ediprofile_option5=:ediprofile_option5,"
              "       ediprofile_emailhtml=:ediprofile_emailhtml"
              " WHERE (ediprofile_id=:ediprofile_id); " );

  q.bindValue(":ediprofile_id", _ediprofileid);
  q.bindValue(":ediprofile_name", _name->text().trimmed());
  q.bindValue(":ediprofile_notes", _notes->toPlainText());

  switch(_type->currentIndex())
  {
    case 0: // Ftp
      q.bindValue(":ediprofile_type", "ftp");
      q.bindValue(":ediprofile_option1", _ftpServer->text());
      q.bindValue(":ediprofile_option2", _ftpLogin->text());
      q.bindValue(":ediprofile_option3", _ftpPassword->text());
      q.bindValue(":ediprofile_option4", _ftpDirectory->text());
      break;
    case 1: // Email
      q.bindValue(":ediprofile_type", "email");
      q.bindValue(":ediprofile_option1", _emailTo->text());
      q.bindValue(":ediprofile_option2", _emailSubject->text());
      q.bindValue(":ediprofile_option3", _emailBody->toPlainText());
      q.bindValue(":ediprofile_option4", _emailCC->text());
      q.bindValue(":ediprofile_emailhtml", QVariant(_emailHTML->isChecked()));
      break;
    default:
      QMessageBox::critical( this, tr("Cannot Save EDI Profile"),
        tr("The selected type for this profile is not recognized.") );
      _type->setFocus();
      return false;
  }

  if(!q.exec())
  {
    QMessageBox::critical( this, tr("Cannot Save EDI Profile"),
      tr("There was a database error preventing this record from being saved.") );
    return false;
  }

  if(cNew == _mode)
    _mode = cEdit;

  return true;
}

void ediProfile::sSave()
{
  if(save())
    accept();
}

void ediProfile::sNew()
{
  if(cNew == _mode)
  {
    if(QMessageBox::information( this, tr("Would you like to Save?"),
         tr("The Profile has not yet been saved to the database. Before continuing\n"
            "you must save the Profile so it exists on the database.\n\n"
            "Would you like to save before continuing?"),
         (QMessageBox::Cancel | QMessageBox::Escape),
         (QMessageBox::Ok | QMessageBox::Default) ) != QMessageBox::Ok)
      return;

    if(!save())
      return;
  }

  ParameterList params;
  params.append("mode", "new");
  params.append("ediprofile_id", _ediprofileid);

  ediForm newdlg(this, "", TRUE);
  newdlg.set(params);

  if(newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void ediProfile::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("ediprofile_id", _ediprofileid);
  params.append("ediform_id", _forms->id());

  ediForm newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void ediProfile::sDelete()
{
  q.prepare("SELECT deleteEDIForm(:ediform_id) AS RESULT;");
  q.bindValue(":ediform_id", _forms->id());
  q.exec();
  if(q.first())
  {
    switch(q.value("result").toInt())
    {
      case -1:
        QMessageBox::critical( this, tr("Cannot Delete EDI Form"),
          tr("The select EDI Form cannot be deleted because it is currently\n"
             "being referenced by one or more records.") );
        return;
      default:
        sFillList();
    }
  }
  else
    systemError( this, tr("A System Error occurred at ediProfile::%1.")
                       .arg(__LINE__) );
}

void ediProfile::populate()
{
  q.prepare("SELECT * "
            "  FROM ediprofile"
            " WHERE (ediprofile_id=:ediprofile_id); ");
  q.bindValue(":ediprofile_id", _ediprofileid);
  q.exec();
  if(q.first())
  {
    _name->setText(q.value("ediprofile_name").toString());
    _notes->setText(q.value("ediprofile_notes").toString());
    
    if("ftp" == q.value("ediprofile_type").toString())
    {
      _type->setCurrentIndex(0);
      _stack->raiseWidget(0);
      _ftpServer->setText(q.value("ediprofile_option1").toString());
      _ftpLogin->setText(q.value("ediprofile_option2").toString());
      _ftpPassword->setText(q.value("ediprofile_option3").toString());
      _ftpDirectory->setText(q.value("ediprofile_option4").toString());
    }
    else if("email" == q.value("ediprofile_type").toString())
    {
      _type->setCurrentIndex(1);
      _stack->raiseWidget(1);
      _emailTo->setText(q.value("ediprofile_option1").toString());
      _emailSubject->setText(q.value("ediprofile_option2").toString());
      _emailBody->setPlainText(q.value("ediprofile_option3").toString());
      _emailCC->setText(q.value("ediprofile_option4").toString());
      _emailHTML->setChecked(q.value("ediprofile_emailhtml").toBool());
    }
    else
    {
      QMessageBox::critical( this, tr("Unknown Type Encountered"),
        tr("An unknown type of %1 was encountered on the record you were trying to open.").arg(q.value("ediprofile_type").toString()) );
      return;
    }

    sFillList();
  }
}

void ediProfile::sFillList()
{
  q.prepare("SELECT ediform_id, ediform_type, ediform_output"
            "  FROM ediform"
            " WHERE (ediform_ediprofile_id=:ediprofile_id) "
            "ORDER BY ediform_type; ");
  q.bindValue(":ediprofile_id", _ediprofileid);
  q.exec();
  _forms->populate(q);
}
