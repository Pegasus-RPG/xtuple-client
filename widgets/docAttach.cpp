/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QVariant>
#include <QDialog>
#include <QString>

#include "documents.h"
#include "docAttach.h"

#include "imageview.h"

/*
 *  Constructs a docAttach as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 *
 *  This screen will receive the source type and id from the calling screen.
 *  Then the user will select a target type and id on this screen.
 *  When the user clicks Save, a new row will be entered into docass and
 *  the screen will return a docass_id to the calling screen.
 */

docAttach::docAttach(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : QDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _sourceid = -1;
  _targetid = -1;

}

/*
 *  Destroys the object and frees any allocated resources
 */

docAttach::~docAttach()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void docAttach::languageChange()
{
  retranslateUi(this);
}

void docAttach::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  //source type from document widget
  param = pParams.value("sourceType", &valid);
  if (valid)
   _source = (enum Documents::DocumentSources)param.toInt();

  //source id from document widget
  param = pParams.value("source_id", &valid);
  if (valid)
    _sourceid = param.toInt();
}


void docAttach::sSave()
{

  XSqlQuery newDocass;

  //if then series, type derived from the stack index. e.g.
  if (_docType->currentIndex() == 0)
  {
    _targettype = "INCDT";
    _targetid = _incdt->id();
  }
  else if (_docType->currentIndex() == 1)
  {
    _targettype = "C";
    _targetid = _cust->id();
  }
  else if (_docType->currentIndex() == 2)
  {
    _targettype = "V";
    _targetid = _vend->id();
  }
  else if (_docType->currentIndex() == 3)
  {
    _targettype = "I";
    _targetid = _item->id();
  }
  else if (_docType->currentIndex() == 4)
  {
    _targettype = "CRMA";
    _targetid = _crmacct->id();
  }
  else if (_docType->currentIndex() == 5)
  {
    _targettype = "J";
    _targetid = _proj->id();
  }


  //set the purpose
  if (_docAttachPurpose->currentIndex() == 0)
  _purpose = "A";
  else if (_docAttachPurpose->currentIndex() == 1)
  _purpose = "C";
  else if (_docAttachPurpose->currentIndex() == 2)
  _purpose = "S";
  else if (_docAttachPurpose->currentIndex() == 3)
  _purpose = "D";

    // insert into docass

  newDocass.prepare( "INSERT INTO docass "
                     "( docass_source_type, docass_source_id, docass_target_type, docass_target_id, docass_purpose ) "
                     "VALUES "
                     "( :docass_source_type, :docass_source_id, :docass_target_type, :docass_target_id, :docass_purpose );" );

  newDocass.bindValue(":docass_source_type", Documents::_documentMap[_source].ident);   //from documents widget
  newDocass.bindValue(":docass_source_id", _sourceid);                                  //from documents widget
  newDocass.bindValue(":docass_target_id", _targetid);                                  //from docAttach widget
  newDocass.bindValue(":docass_target_type", _targettype);                              //from docAttach widget
  newDocass.bindValue(":docass_purpose", _purpose);                                     //from docAttach widget

  newDocass.exec();

  accept();
}
