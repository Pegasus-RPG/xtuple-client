/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef documents_h
#define documents_h

#include <QWidget>

#include <xsqlquery.h>

#include "ui_documents.h"

class XTUPLEWIDGETS_EXPORT Documents : public QWidget, public Ui::documents
{
  Q_OBJECT

  Q_ENUMS(DocumentSources)

  Q_PROPERTY(DocumentSources type READ type WRITE setType)

  friend class image;
  friend class file;

  public:
    Documents(QWidget *);

    // if you add to this then add to the _documentMap[] below
    enum DocumentSources
    {
      Uninitialized,
      Address,          BBOMHead,           BBOMItem,
      BOMHead,          BOMItem,            BOOHead,
      BOOItem,          CRMAccount,         Contact, 
      Customer,         Employee,           Incident,   
      Item,             ItemSite,           ItemSource,
      Location,		LotSerial,          Opportunity,
      Project,		PurchaseOrder,      PurchaseOrderItem,
      ReturnAuth,       ReturnAuthItem,     Quote, 
      QuoteItem,        SalesOrder,         SalesOrderItem,
      TransferOrder,	TransferOrderItem,  Vendor,
      Warehouse,	WorkOrder
    };

    inline int  sourceid()             { return _sourceid; }
    inline enum DocumentSources type() { return _source;   }

    struct DocumentMap
    {
      enum DocumentSources source;
      QString             ident;

      DocumentMap(enum DocumentSources s, const QString & i)
      {
        source = s;
        ident = i;
      }
    };
    static const struct DocumentMap _documentMap[]; // see Documents.cpp for init

  public slots:
    void setType(enum DocumentSources);
    void setId(int);
    void setReadOnly(bool);
    
    void sFilesToggled(bool p);
    void sImagesToggled(bool p);

    void sOpenFile();
    void sNewFile();
    void sEditFile();
    void sViewFile();
    void sDeleteFile();
    
    void sOpenImage();
    void sPrintImage();
    void sNewImage();
    void sEditImage();
    void sViewImage();
    void sDeleteImage();
    
    void refresh();

  private:
    enum DocumentSources _source;
    int                  _sourceid;

};

#endif
