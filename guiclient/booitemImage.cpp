/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "booitemImage.h"

#include <QSqlError>
#include <QVariant>

#include "image.h"

booitemImage::booitemImage(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));

  _image->addColumn(tr("Name"), _itemColumn, Qt::AlignLeft, true, "image_name");
  _image->addColumn(tr("Description"),   -1, Qt::AlignLeft, true, "descrip");

  sFillList();
}

booitemImage::~booitemImage()
{
  // no need to delete child widgets, Qt does it all for us
}

void booitemImage::languageChange()
{
  retranslateUi(this);
}

enum SetResponse booitemImage::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("booitem_id", &valid);
  if (valid)
    _booitemid = param.toInt();

  param = pParams.value("booimage_id", &valid);
  if (valid)
  {
    _booimageid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _purpose->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _save->setFocus();
      _purpose->setEnabled(FALSE);
    }
  }

  return NoError;
}

void booitemImage::sSave()
{
  QString purpose;
  if (_purpose->currentIndex() == 0)  
    purpose = "I";
  if (_purpose->currentIndex() == 1)  
    purpose = "P";
  if (_purpose->currentIndex() == 2)  
    purpose = "E";
  if (_purpose->currentIndex() == 3)  
    purpose = "M";

  XSqlQuery newImage;

  if (_mode == cNew)
  {
    _booimageid = -1;

    if (purpose != "M")
    {
      XSqlQuery booimageid;
      booimageid.prepare( "SELECT booimage_id "
                           "FROM booimage "
                           "WHERE ( (booimage_booitem_id=:booitem_id)"
                           " AND (booimage_purpose=:booimage_purpose) );" );
      booimageid.bindValue(":booitem_id", _booitemid);
      booimageid.bindValue(":booimage_purpose", purpose);
      booimageid.exec();
      if (booimageid.first())
      {
        _booimageid = booimageid.value("booimage_id").toInt();
        _mode = cEdit;
      }
      else if (booimageid.lastError().type() != QSqlError::NoError)
      {
	systemError(this, booimageid.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
      
    if (_booimageid == -1)
    {
      XSqlQuery booimageid;
      booimageid.prepare("SELECT NEXTVAL('booimage_booimage_id_seq') AS _booimage_id;");
      booimageid.exec();
      if (booimageid.first())
        _booimageid = booimageid.value("_booimage_id").toInt();
      else if (booimageid.lastError().type() != QSqlError::NoError)
      {
	systemError(this, booimageid.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }

    newImage.prepare( "INSERT INTO booimage "
                      "( booimage_id, booimage_booitem_id, booimage_purpose, booimage_image_id ) "
                      "VALUES "
                      "( :booimage_id, :booimage_booitem_id, :booimage_purpose, :booimage_image_id );" );
  }

  if (_mode == cEdit)
    newImage.prepare( "UPDATE booimage "
                      "SET booimage_purpose=:booimage_purpose, booimage_image_id=:booimage_image_id "
                      "WHERE (booimage_id=:booimage_id);" );

  newImage.bindValue(":booimage_id", _booimageid);
  newImage.bindValue(":booimage_booitem_id", _booitemid);
  newImage.bindValue(":booimage_image_id", _image->id());
  newImage.bindValue(":booimage_purpose", purpose);

  newImage.exec();
  if (newImage.lastError().type() != QSqlError::NoError)
  {
    systemError(this, newImage.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_booimageid);
}

void booitemImage::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  image newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void booitemImage::sFillList()
{
  XSqlQuery iq;
  iq.prepare("SELECT image_id, image_name, "
             "       firstLine(image_descrip) AS descrip "
             "FROM image "
             "ORDER BY image_name;" );
  iq.exec();
  _image->populate(iq);
  if (iq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, iq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void booitemImage::populate()
{
  XSqlQuery booimage;
  booimage.prepare( "SELECT booimage_purpose, booimage_image_id "
                     "FROM booimage "
                     "WHERE (booimage_id=:booimage_id);" );
  booimage.bindValue(":booimage_id", _booimageid);
  booimage.exec();
  if (booimage.first())
  {
    QString purpose = booimage.value("booimage_purpose").toString();

    if (purpose == "I")
      _purpose->setCurrentIndex(0);
    else if (purpose == "P")
      _purpose->setCurrentIndex(1);
    else if (purpose == "E")
      _purpose->setCurrentIndex(2);
    else if (purpose == "M")
      _purpose->setCurrentIndex(3);

    _image->setId(booimage.value("booimage_image_id").toInt());
  }
}
