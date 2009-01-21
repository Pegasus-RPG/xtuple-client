/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef _shipmentCluster_h
#define _shipmentCluster_h

#include "virtualCluster.h"

class ShipmentList : public VirtualList
{
    Q_OBJECT

    public:
        ShipmentList(QWidget*, Qt::WindowFlags = 0);
};

class ShipmentSearch : public VirtualSearch
{
    Q_OBJECT

    public:
        ShipmentSearch(QWidget*, Qt::WindowFlags = 0);
};

class XTUPLEWIDGETS_EXPORT ShipmentClusterLineEdit : public VirtualClusterLineEdit
{
    Q_OBJECT

    Q_ENUMS(ShipmentType)

    Q_PROPERTY(ShipmentType type READ type WRITE setType )

    public:
      // TODO: make these flags instead of a simple enum?
        enum ShipmentType { All, SalesOrder, TransferOrder };
        ShipmentClusterLineEdit(QWidget*, const char* = 0);
        virtual ShipmentType type();

    public slots:
        virtual void setId(const int);
        virtual void setType(QString);
        virtual void setType(ShipmentType);

    protected slots:
        virtual VirtualList*   listFactory();
        virtual VirtualSearch* searchFactory();

    private:
        ShipmentType _type;
};

class XTUPLEWIDGETS_EXPORT ShipmentCluster : public VirtualCluster
{
    Q_OBJECT

    public:
        ShipmentCluster(QWidget*, const char* = 0);
        virtual ShipmentClusterLineEdit::ShipmentType type();

    public slots:
        virtual void limitToOrder(const int);
        virtual void removeOrderLimit();
        virtual void setId(const int);
        virtual void setType(QString);
        virtual void setType(ShipmentClusterLineEdit::ShipmentType);
};

#endif
