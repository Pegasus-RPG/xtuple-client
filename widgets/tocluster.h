/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef toCluster_h
#define toCluster_h

#include "wcombobox.h"
#include "xlineedit.h"

#include "widgets.h"

class QLabel;
class QPushButton;

class ToCluster;

//  Possible Transfer Order Status
#define cToOpen               0x01
#define cToClosed             0x02
#define cToAtShipping         0x04

class XTUPLEWIDGETS_EXPORT ToLineEdit : public XLineEdit
{
  Q_OBJECT

friend class ToCluster;

  public:
    ToLineEdit(QWidget *, const char * = 0);

  public slots:
    void setId(int);
    void setNumber(int);
    void clear();
    void sParse();

  signals:
    void valid(bool);
    void newId(int);
    void newSrcwhs(int);
    void newDstwhs(int);
    void numberChanged(int);
    void numberChanged(const QString &);

  private:
    int  _dstwhs;
    int  _number;
    int  _srcwhs;
    int  _type;
};


class XTUPLEWIDGETS_EXPORT ToCluster : public QWidget
{
  Q_OBJECT

  public:
    ToCluster(QWidget *, const char * = 0);
    ToCluster(int, QWidget *);

    inline int  dstwhs()           { return _toNumber->_dstwhs;  }
    inline int  id()               { return _toNumber->_id;      }
    inline int  number()           { return _toNumber->text().toInt();  }
    inline int  srcwhs()           { return _toNumber->_srcwhs;  }
    inline bool isValid()          { return _toNumber->_valid;   }
    inline void setType(int pType) { _toNumber->_type = pType;   }

  public slots:
    void setId(int);

  signals:
    void newId(int);
    void newSrcwhs(int);
    void newDstwhs(int);
    void valid(bool);

  private slots:
    void sList();

  private:
    void constructor();

    WComboBox    *_dstwhs;
    QPushButton  *_list;
    WComboBox    *_srcwhs;
    ToLineEdit   *_toNumber;
};

#endif
