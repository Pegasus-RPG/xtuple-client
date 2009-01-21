/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "imageview.h"
#include "widgets.h"

#include <xsqlquery.h>

#include <QVariant>
#include <QImageWriter>
#include <QImageReader>
#include <QBuffer>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollArea>
#include <quuencode.h>

/*
 *  Constructs a image as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
imageview::imageview(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_fileList, SIGNAL(clicked()), this, SLOT(sFileList()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

#ifndef Q_WS_MAC
  _fileList->setMaximumWidth(25);
#endif

  _imageview = new QLabel();
  _imageview->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  QScrollArea * scrollArea = new QScrollArea();
  scrollArea->setWidgetResizable(true);
  scrollArea->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  scrollArea->setWidget(_imageview);
  QHBoxLayout *layout = new QHBoxLayout;
  layout->setMargin(0);
  layout->addWidget(scrollArea);
  _imageFrame->setLayout(layout);
}

/*
 *  Destroys the object and frees any allocated resources
 */
imageview::~imageview()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void imageview::languageChange()
{
  retranslateUi(this);
}

void imageview::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("image_id", &valid);
  if (valid)
  {
    _imageviewid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _fileName->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _filenameLit->hide();
      _fileName->hide();
      _fileList->hide();

      _name->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(FALSE);
      _descrip->setEnabled(FALSE);
      _filenameLit->hide();
      _fileName->hide();
      _fileList->hide();
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }
}

void imageview::populate()
{
  XSqlQuery image;
  image.prepare( "SELECT image_name, image_descrip, image_data "
                 "FROM image "
                 "WHERE (image_id=:image_id);" );
  image.bindValue(":image_id", _imageviewid);
  image.exec();
  if (image.first())
  {
    _name->setText(image.value("image_name").toString());
    _descrip->setText(image.value("image_descrip").toString());

    __imageview.loadFromData(QUUDecode(image.value("image_data").toString()));
    _imageview->setPixmap(QPixmap::fromImage(__imageview));
  }
}

void imageview::sSave()
{
  XSqlQuery newImage;

  if (_mode == cNew)
  {
    if (!__imageview.isNull())
    {
      XSqlQuery imageid("SELECT NEXTVAL('image_image_id_seq') AS _image_id");
      if (imageid.first())
        _imageviewid = imageid.value("_image_id").toInt();
//  ToDo
 
      QImageWriter imageIo;
      QBuffer  imageBuffer;
      QString  imageString;

      imageBuffer.open(QIODevice::ReadWrite);
      imageIo.setDevice(&imageBuffer);
      imageIo.setFormat("PNG");

      if (!imageIo.write(__imageview))
      {
//  ToDo - should issue an error here
        reject();
        return;
      }

      imageBuffer.close();
      imageString = QUUEncode(imageBuffer);

      newImage.prepare( "INSERT INTO image "
                        "(image_id, image_name, image_descrip, image_data) "
                        "VALUES "
                        "(:image_id, :image_name, :image_descrip, :image_data);" );
      newImage.bindValue(":image_id", _imageviewid);
      newImage.bindValue(":image_name", _name->text());
      newImage.bindValue(":image_descrip", _descrip->toPlainText());
      newImage.bindValue(":image_data", imageString);
    }
  }
  else if (_mode == cEdit)
  {
    newImage.prepare( "UPDATE image "
                      "SET image_name=:image_name, image_descrip=:image_descrip "
                      "WHERE (image_id=:image_id);" );
    newImage.bindValue(":image_id", _imageviewid);
    newImage.bindValue(":image_name", _name->text());
    newImage.bindValue(":image_descrip", _descrip->toPlainText());
  }

  newImage.exec();

  done(_imageviewid);
}

void imageview::sFileList()
{
  bool first = TRUE;
  bool havejpg = FALSE;
  QString frmtList = QString(tr("Images ("));
  QString ext = QString::null;
  QList<QByteArray> list = QImageReader::supportedImageFormats();
  for (int i = 0; i < list.size(); ++i)
  {
    if (!first)
      frmtList += QString(tr(" "));
    ext = (list.at(i)).lower();

    if (ext == "jpeg")
      ext = "jpg";

    if (ext != "jpg" || !havejpg)
      frmtList += QString(tr("*.")) + ext;

    if (ext == "jpg")
      havejpg = TRUE;

    first = FALSE;
  }

  frmtList += QString(tr(")"));
  if (first)
    frmtList = QString(tr("Images (*.png *.xpm *.jpg *.gif)")); // should I do this?

  _fileName->setText(QFileDialog::getOpenFileName( this, tr("Select Image File"), QString::null, frmtList));

  if (_fileName->text().length())
  {
    if(!__imageview.load(_fileName->text()))
      QMessageBox::warning(this, tr("Could not load file"),
                            tr( "Could not load the selected file.\n"
                                "The file is not an image, an unknown image format or is corrupt" ) );
    _imageview->setPixmap(QPixmap::fromImage(__imageview));
  }
}
