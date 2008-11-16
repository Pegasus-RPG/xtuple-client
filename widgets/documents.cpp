/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include <QDesktopServices>
#include <QDialog>
#include <QUrl>

#include <openreports.h>
#include <parameter.h>
#include <xsqlquery.h>

#include "file.h"
#include "imageview.h"
#include "imageAssignment.h"
#include "documents.h"


const Documents::DocumentMap Documents::_documentMap[] =
{
  DocumentMap( Uninitialized,     " "   ),
  DocumentMap( Address,           "ADDR"),
  DocumentMap( BBOMHead,          "BBH" ),
  DocumentMap( BBOMItem,          "BBI" ),
  DocumentMap( BOMHead,           "BMH" ),
  DocumentMap( BOMItem,           "BMI" ),
  DocumentMap( BOOHead,           "BOH" ),
  DocumentMap( BOOItem,           "BOI" ),
  DocumentMap( CRMAccount,        "CRMA"),
  DocumentMap( Contact,           "T"   ),
  DocumentMap( Customer,          "C"   ),
  DocumentMap( Employee,          "EMP" ),
  DocumentMap( Incident,          "INCDT"),
  DocumentMap( Item,              "I"   ),
  DocumentMap( ItemSite,          "IS"  ),
  DocumentMap( ItemSource,        "IR"  ),
  DocumentMap( Location,          "L"   ),
  DocumentMap( LotSerial,         "LS"   ),
  DocumentMap( Opportunity,       "OPP" ),
  DocumentMap( Project,           "J"   ),
  DocumentMap( PurchaseOrder,     "P"   ),
  DocumentMap( PurchaseOrderItem, "PI"  ),
  DocumentMap( ReturnAuth,        "RA"  ),
  DocumentMap( ReturnAuthItem,    "RI"  ),
  DocumentMap( Quote,             "Q"   ),
  DocumentMap( QuoteItem,         "QI"  ),
  DocumentMap( SalesOrder,        "S"   ),
  DocumentMap( SalesOrderItem,    "SI"  ),
  DocumentMap( TransferOrder,     "TO"  ),
  DocumentMap( TransferOrderItem, "TI"  ),
  DocumentMap( Vendor,            "V"   ),
  DocumentMap( Warehouse,         "WH"  ),
  DocumentMap( WorkOrder,         "W"   ),
};

Documents::Documents(QWidget *pParent) :
  QWidget(pParent)
{
  setupUi(this);
  
  _source = Uninitialized;
  _sourceid = -1;


  _images->addColumn(tr("Image Name"),  _itemColumn,   Qt::AlignLeft, true, "image_name" );
  _images->addColumn(tr("Description"), -1,            Qt::AlignLeft, true, "image_descrip" );
  _images->addColumn(tr("Purpose"),     _itemColumn*2, Qt::AlignLeft, true, "image_purpose" );

  _files->addColumn(tr("Title"),        _itemColumn, Qt::AlignLeft,true, "url_title");
  _files->addColumn(tr("URL"),          -1, Qt::AlignLeft,true, "url_url");

  connect(_deleteFile, SIGNAL(clicked()), this, SLOT(sDeleteFile()));
  connect(_editFile, SIGNAL(clicked()), this, SLOT(sEditFile()));
  connect(_newFile, SIGNAL(clicked()), this, SLOT(sNewFile()));
  connect(_viewFile, SIGNAL(clicked()), this, SLOT(sViewFile()));
  connect(_openFile, SIGNAL(clicked()), this, SLOT(sOpenFile()));
  
  connect(_openImage, SIGNAL(clicked()), this, SLOT(sOpenImage()));
  connect(_newImage, SIGNAL(clicked()), this, SLOT(sNewImage()));
  connect(_editImage, SIGNAL(clicked()), this, SLOT(sEditImage()));
  connect(_viewImage, SIGNAL(clicked()), this, SLOT(sViewImage()));
  connect(_printImage, SIGNAL(clicked()), this, SLOT(sPrintImage()));
  connect(_deleteImage, SIGNAL(clicked()), this, SLOT(sDeleteImage()));
  
  connect(_imagesButton, SIGNAL(toggled(bool)), this, SLOT(sImagesToggled(bool)));
  connect(_filesButton, SIGNAL(toggled(bool)), this, SLOT(sFilesToggled(bool)));
}

void Documents::setType(enum DocumentSources pSource)
{
  _source = pSource;
  if (pSource==Item)
    _images->showColumn(2);
  else
   _images->hideColumn(2);
}

void Documents::setId(int pSourceid)
{
  _sourceid = pSourceid;
  refresh();
}

void Documents::setReadOnly(bool pReadOnly)
{
  _newImage->setEnabled(!pReadOnly);
  _editImage->setEnabled(!pReadOnly);
  _deleteImage->setEnabled(!pReadOnly);
  
  _newFile->setEnabled(!pReadOnly);
  _editFile->setEnabled(!pReadOnly);
  _deleteFile->setEnabled(!pReadOnly);
}

void Documents::sNewFile()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("sourceType", _source);
  params.append("source_id", _sourceid);

  file newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
  refresh();
}

void Documents::sEditFile()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("url_id", _files->id());

  file newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
  refresh();
}

void Documents::sViewFile()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("url_id", _files->id());

  file newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void Documents::sDeleteFile()
{
  XSqlQuery q;
  q.prepare("DELETE FROM url"
            " WHERE (url_id=:url_id);" );
  q.bindValue(":url_id", _files->id());
  q.exec();
  refresh();
}

void Documents::sOpenFile()
{
  XSqlQuery q;
  q.prepare("SELECT url_url"
            "  FROM url"
            " WHERE (url_id=:url_id); ");
  q.bindValue(":url_id", _files->id());
  q.exec();
  if(q.first())
  {
    //If url scheme is missing, we'll assume it is "file" for now.
    QUrl url(q.value("url_url").toString());
    if (url.scheme().isEmpty())
      url.setScheme("file");
    QDesktopServices::openUrl(url);
  }
}

void Documents::sOpenImage()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("image_id", _images->altId());

  imageview newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void Documents::sNewImage()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("sourceType", _source);
  params.append("source_id", _sourceid);

  imageAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    refresh();
}

void Documents::sEditImage()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("imageass_id", _images->id());

  imageAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    refresh();
}

void Documents::sViewImage()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("imageass_id", _images->id());

  imageAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    refresh();
}

void Documents::sDeleteImage()
{
  XSqlQuery q;
  q.prepare( "DELETE FROM imageass "
             "WHERE (imageass_id=:imageass_id);" );
  q.bindValue(":imageass_id", _images->id());
  q.exec();

  refresh();
}

void Documents::sPrintImage()
{
  ParameterList params;
  params.append("image_id", _images->altId());

  orReport report("Image", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void Documents::refresh()
{
  if(-1 == _sourceid)
  {
    _images->clear();
    _files->clear();
    return;
  }

  XSqlQuery query;
  
  //Populate images
  QString sql( "SELECT imageass_id, imageass_image_id, image_name, firstLine(image_descrip) AS image_descrip,"
               "       CASE WHEN (imageass_purpose='I') THEN :inventory"
               "            WHEN (imageass_purpose='P') THEN :product"
               "            WHEN (imageass_purpose='E') THEN :engineering"
               "            WHEN (imageass_purpose='M') THEN :misc"
               "            ELSE :other"
               "       END AS image_purpose "
               "FROM imageass, image "
               "WHERE ( (imageass_image_id=image_id)"
               " AND (imageass_source=:source) "
               " AND (imageass_source_id=:sourceid) ) "
               "ORDER BY image_name; ");
  query.prepare(sql);
  query.bindValue(":inventory", tr("Inventory Description"));
  query.bindValue(":product", tr("Product Description"));
  query.bindValue(":engineering", tr("Engineering Reference"));
  query.bindValue(":misc", tr("Miscellaneous"));
  query.bindValue(":other", tr("Other"));
  query.bindValue(":sourceCust", _documentMap[Customer].ident);
  query.bindValue(":sourceContact", _documentMap[Contact].ident);
  query.bindValue(":sourceVend", _documentMap[Vendor].ident);
  query.bindValue(":source", _documentMap[_source].ident);
  query.bindValue(":sourceid", _sourceid);
  query.exec();
  _images->populate(query,TRUE);
  
  //Populate file urls
  sql = "SELECT url_id, url_title, url_url "
        "FROM url "
        "WHERE ((url_source_id=:sourceid) "
        "AND (url_source=:source)) "
        "ORDER BY url_title; ";
  query.prepare(sql);
  query.bindValue(":sourceCust", _documentMap[Customer].ident);
  query.bindValue(":sourceContact", _documentMap[Contact].ident);
  query.bindValue(":sourceVend", _documentMap[Vendor].ident);
  query.bindValue(":source", _documentMap[_source].ident);
  query.bindValue(":sourceid", _sourceid);
  query.exec();
  _files->populate(query);
  
}

void Documents::sImagesToggled(bool p)
{
  if (p)
  {
    disconnect(_filesButton, SIGNAL(toggled(bool)), this, SLOT(sFilesToggled(bool)));
    _filesButton->setChecked(!p);
    _documentsStack->setCurrentIndex(0);
    connect(_filesButton, SIGNAL(toggled(bool)), this, SLOT(sFilesToggled(bool)));
  }
}

void Documents::sFilesToggled(bool p)
{
  if (p)
  {
    disconnect(_imagesButton, SIGNAL(toggled(bool)), this, SLOT(sImagesToggled(bool)));
    _imagesButton->setChecked(!p);
    _documentsStack->setCurrentIndex(1);
    connect(_imagesButton, SIGNAL(toggled(bool)), this, SLOT(sImagesToggled(bool)));
  }
}


