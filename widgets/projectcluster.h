/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef _projectCluster_h

#define _projectCluster_h

#include "virtualCluster.h"

class XTUPLEWIDGETS_EXPORT ProjectLineEdit : public VirtualClusterLineEdit
{
    Q_OBJECT

    Q_ENUMS(ProjectType)

    Q_PROPERTY(ProjectType projectType READ type WRITE setType )
    
    public:
      enum ProjectType {
        Undefined,
        SalesOrder,
        WorkOrder,
        PurchaseOrder,
      };
      
      ProjectLineEdit(QWidget*, const char* = 0);
      ProjectLineEdit(enum ProjectType pPrjType, QWidget *pParent, const char *pName);
       
      virtual enum ProjectType type() const { return _type;     }
      
    public slots:
      virtual void setType(enum ProjectType ptype);
      
    private:
      enum ProjectType _type;

};

class XTUPLEWIDGETS_EXPORT ProjectCluster : public VirtualCluster
{
    Q_OBJECT

    Q_ENUMS(ProjectLineEdit::ProjectType)

    Q_PROPERTY(ProjectLineEdit::ProjectType projectType READ type WRITE setType )
    
    public:
      ProjectCluster(QWidget*, const char* = 0);
      
      enum ProjectLineEdit::ProjectType type();
      
    public slots:
      virtual void setType(enum ProjectLineEdit::ProjectType ptype);

};

#endif
