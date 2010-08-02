/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef custCluster_h
#define custCluster_h

#include "widgets.h"
#include "virtualCluster.h"

class QLabel;
class QPushButton;
class QDragEnterEvent;
class QDropEvent;
class QMouseEvent;

class CustInfo;

#define __allCustomers    0x01
#define __activeCustomers 0x02

#define CRMACCT_ID      5
#define ISCUSTOMER      6

class XTUPLEWIDGETS_EXPORT CLineEdit : public VirtualClusterLineEdit
{
  Q_OBJECT

  Q_ENUMS(CLineEditTypes)

  Q_PROPERTY(CLineEditTypes	type READ type	    WRITE setType	)

  friend class CustInfo;

  public:
    CLineEdit(QWidget * = 0, const char * = 0);

    enum CLineEditTypes
    {
      AllCustomers, 		ActiveCustomers,
      AllProspects,		ActiveProspects,
      AllCustomersAndProspects,	ActiveCustomersAndProspects
    };

    inline CLineEditTypes	type()	    const { return _type;       }

  public slots:
    void setId(int);
    void sOpen();
    void setType(CLineEditTypes);

  protected slots:
    VirtualList*    listFactory();
    VirtualSearch*  searchFactory();

  signals:
    void newCrmacctId(int);

  private:
    CLineEditTypes	_type;
    int                 _crmacctId;
};

class XTUPLEWIDGETS_EXPORT CustCluster : public VirtualCluster
{
  Q_OBJECT

  Q_PROPERTY(CLineEdit::CLineEditTypes	type           READ type	  WRITE setType                          )

  public:
    CustCluster(QWidget *parent, const char *name = 0);

    inline CLineEdit::CLineEditTypes type()  const { return static_cast<CLineEdit*>(_number)->type();          };

    Q_INVOKABLE inline bool editMode() { return _editMode; }
    Q_INVOKABLE void   setType(CLineEdit::CLineEditTypes);
    
  public slots:
    void setEditMode(bool);

  signals:
    void newCrmacctId(int);

  private:
    bool _editMode;

};

#endif

