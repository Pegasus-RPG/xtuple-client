/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QVariant>
#include <QDialog>
#include <QMessageBox>
#include <QString>
#include <QUrl>

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
  connect(_docType, SIGNAL(currentIndexChanged(int)), this, SLOT(sHandleButtons()));

  _sourceid = -1;
  _targetid = -1;

  _po->setAllowedTypes(OrderLineEdit::Purchase);
  _so->setAllowedTypes(OrderLineEdit::Sales);
  adjustSize();
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

void docAttach::sHandleButtons()
{
  _save->disconnect();

  if (_docType->currentIndex() == 1)
  {
    connect(_cntct, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
    _save->setEnabled(_cntct->isValid());
  }
  else if (_docType->currentIndex() == 2)
  {
    connect(_crmacct, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
    _save->setEnabled(_crmacct->isValid());
  }
  else if (_docType->currentIndex() == 3)
  {
    connect(_cust, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
    _save->setEnabled(_cust->isValid());
  }
  else if (_docType->currentIndex() == 4)
  {
    connect(_emp, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
    _save->setEnabled(_emp->isValid());
  }
  else if (_docType->currentIndex() == 5)
  {
    _save->setEnabled(true);
  }
  else if (_docType->currentIndex() == 6)
  {
    connect(_img, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
    _save->setEnabled(_img->isValid());
  }
  else if (_docType->currentIndex() == 7)
  {
    connect(_incdt, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
    _save->setEnabled(_incdt->isValid());
  }
  else if (_docType->currentIndex() == 8)
  {
    connect(_item, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
    _save->setEnabled(_item->isValid());
  }
  else if (_docType->currentIndex() == 9)
  {
    connect(_opp, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
    _save->setEnabled(_opp->isValid());
  }
  else if (_docType->currentIndex() == 10)
  {
    connect(_proj, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
    _save->setEnabled(_proj->isValid());
  }
  else if (_docType->currentIndex() == 11)
  {
    connect(_po, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
    _save->setEnabled(_po->isValid());
  }
  else if (_docType->currentIndex() == 12)
  {
    connect(_so, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
    _save->setEnabled(_so->isValid());
  }
  else if (_docType->currentIndex() == 13)
  {
    connect(_vend, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
    _save->setEnabled(_vend->isValid());
  }
  else if (_docType->currentIndex() == 14)
  {
    _save->setEnabled(true);
  }
  else if (_docType->currentIndex() == 15)
  {
    connect(_wo, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
    _save->setEnabled(_wo->isValid());
  }
  else
    return;

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
}

void docAttach::sSave()
{  
  XSqlQuery newDocass;
  QString title;
  QUrl url;

  //set the purpose
  if (_docAttachPurpose->currentIndex() == 0)
    _purpose = "S";
  else if (_docAttachPurpose->currentIndex() == 1)
    _purpose = "A";
  else if (_docAttachPurpose->currentIndex() == 2)
    _purpose = "C";
  else if (_docAttachPurpose->currentIndex() == 3)
    _purpose = "D";

  //if then series, type derived from the stack index. e.g.
  if (_docType->currentIndex() == 1)
  {
    _targettype = "T";
    _targetid = _cntct->id();
  }
  else if (_docType->currentIndex() == 2)
  {
    _targettype = "CRMA";
    _targetid = _crmacct->id();
  }
  else if (_docType->currentIndex() == 3)
  {
    _targettype = "C";
    _targetid = _cust->id();
  }
  else if (_docType->currentIndex() == 4)
  {
    _targettype = "EMP";
    _targetid = _emp->id();
  }
  else if (_docType->currentIndex() == 5)
  {
     _targettype = "URL";
     title = _filetitle->text();
     url = QUrl(_file->text());
     url.setScheme("file");
  }
  else if (_docType->currentIndex() == 6)
  {
     _targettype = "IMG";
     _targetid = _img->id();
  }
  else if (_docType->currentIndex() == 7)
  {
    _targettype = "INCDT";
    _targetid = _incdt->id();
  }
  else if (_docType->currentIndex() == 8)
  {
    _targettype = "I";
    _targetid = _item->id();
  }
  else if (_docType->currentIndex() == 9)
  {
    _targettype = "OPP";
    _targetid = _opp->id();
  }
  else if (_docType->currentIndex() == 10)
  {
    _targettype = "J";
    _targetid = _proj->id();
  }
  else if (_docType->currentIndex() == 11)
  {
    _targettype = "P";
    _targetid = _po->id();
  }
  else if (_docType->currentIndex() == 12)
  {
    _targettype = "S";
    _targetid = _so->id();
  }
  else if (_docType->currentIndex() == 13)
  {
    _targettype = "V";
    _targetid = _vend->id();
  }
  else if (_docType->currentIndex() == 14)
  {
     _targettype = "URL";
     title = _urltitle->text();
     url = QUrl(_url->text());
     url.setScheme("http");
  }
  else if (_docType->currentIndex() == 15)
  {
    _targettype = "W";
    _targetid = _wo->id();
  }

  if (_targettype == "IMG")
  {
     // For now images are handled differently because of legacy structures...
    newDocass.prepare( "INSERT INTO imageass "
                       "( imageass_source, imageass_source_id, imageass_image_id, imageass_purpose ) "
                       "VALUES "
                       "( :docass_source_type, :docass_source_id, :docass_target_id, :docass_purpose );" );
  }
  else if (_targettype == "URL")
  {
    if(!url.isValid())
    {
      QMessageBox::warning( this, tr("Must Specify valid path"),
                            tr("You must specify a path before you may save.") );
      return;
    }

    // For now urls are handled differently because of legacy structures...
    newDocass.prepare( "INSERT INTO url "
                       "( url_source, url_source_id, url_title, url_url ) "
                       "VALUES "
                       "( :docass_source_type, :docass_source_id, :title, :url );" );
    newDocass.bindValue(":title", title);
    newDocass.bindValue(":url", url.toString());
  }
  else
  {
    newDocass.prepare( "INSERT INTO docass "
                       "( docass_source_type, docass_source_id, docass_target_type, docass_target_id, docass_purpose ) "
                       "VALUES "
                       "( :docass_source_type, :docass_source_id, :docass_target_type, :docass_target_id, :docass_purpose );" );
    newDocass.bindValue(":docass_target_type", _targettype);
  }

  newDocass.bindValue(":docass_source_type", Documents::_documentMap[_source].ident);
  newDocass.bindValue(":docass_source_id", _sourceid);
  newDocass.bindValue(":docass_target_id", _targetid);
  newDocass.bindValue(":docass_purpose", _purpose);

  newDocass.exec();

  accept();
}
