/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef glCluster_h
#define glCluster_h

#include <QWidget>

#include "widgets.h"
#include "xlineedit.h"

class QLineEdit;
class QPushButton;
class QKeyEvent;
class QFocusEvent;

class XTUPLEWIDGETS_EXPORT GLCluster : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(QString fieldName      READ fieldName      WRITE setFieldName)
  Q_PROPERTY(QString defaultNumber  READ defaultNumber                        DESIGNABLE false)
  Q_PROPERTY(QString number         READ number         WRITE setNumber       DESIGNABLE false)
    
  public:
    GLCluster(QWidget *parent, const char *name = 0);

    enum Type {
      cUndefined  = 0x00,

      cAsset      = 0x01,
      cLiability  = 0x02,
      cExpense    = 0x04,
      cRevenue    = 0x08,
      cEquity     = 0x10
    };
   
    QString defaultNumber() const    { return QString();} //Default to an empty string
    QString fieldName()  const       { return _fieldName;}
    QString number()     const       { return _number;};
    inline void setType(unsigned int pType) { _type = pType; }
    inline unsigned int type()  const       { return _type; }
    inline bool showExternal()              { return _showExternal; }
    inline void setShowExternal(bool p)     { _showExternal = p; }

    Q_INVOKABLE void setReadOnly(bool);

  public slots:
    bool isValid();
    int id();
    void setDataWidgetMap(XDataWidgetMapper* m);
    void setId(int);
    void setEnabled(bool);
    void setFieldName(QString name)         { _fieldName=name;}
    void setNumber(QString number);

  protected:
    void keyPressEvent(QKeyEvent *);
    void focusInEvent(QFocusEvent *);

  private slots:
    void sEllipses();
    void sSearch();
    void sList();
    void sParse();
    void sTextChanged(const QString &);

  signals:
    void newId(int);
    void valid(bool);

  private:
    int  _accntid;
    bool _mapped;
    bool _valid;
    bool _parsed;
    bool _showExternal;
    unsigned int _type;
    QString      _fieldName;
    QString      _number;

    QLineEdit   *_company;
    QLineEdit   *_profit;
    XLineEdit   *_main;
    QLineEdit   *_sub;
    QLineEdit   *_account;
    QPushButton *_list;
    
    XDataWidgetMapper *_mapper;
};

#endif

