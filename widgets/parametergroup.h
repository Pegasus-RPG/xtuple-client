/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#ifndef patterngroup_h
#define patterngroup_h

#include <QGroupBox>
#include <QRadioButton>

#include "widgets.h"

class XComboBox;
class QLineEdit;
class ParameterList;
class XSqlQuery;

class XTUPLEWIDGETS_EXPORT ParameterGroup : public QGroupBox
{
  Q_OBJECT

  Q_ENUMS(ParameterGroupTypes)
  Q_ENUMS(ParameterGroupStates)

  Q_PROPERTY(bool fixedSize READ fixedSize WRITE setFixedSize)
  Q_PROPERTY(QString pattern READ pattern WRITE setPattern)
  Q_PROPERTY(enum ParameterGroupTypes type READ type WRITE setType)
  Q_PROPERTY(enum ParameterGroupStates state READ state WRITE setState)

  public:
    ParameterGroup(QWidget * = 0, const char * = 0);

    enum ParameterGroupTypes
    {
      AdhocGroup, PlannerCode, ProductCategory, ClassCode, ItemGroup, CostCategory,
      CustomerType, CustomerGroup, CurrencyNotBase, Currency, WorkCenter, User
    };

    enum ParameterGroupStates
    {
      All, Selected, Pattern
    };

    bool fixedSize() {return _fixed;};
    enum ParameterGroupStates state();
    Q_INVOKABLE int                       id();
    QString                   pattern();
    void                      appendValue(ParameterList &);
    void                      bindValue(XSqlQuery &);

    enum ParameterGroupTypes  type()       { return _type;                    }
    inline bool               isAll()      { return _all->isChecked();        }
    inline bool               isSelected() { return _selected->isChecked();   }
    inline bool               isPattern()  { return _usePattern->isChecked(); }

  public slots:
    void setFixedSize(bool);
    void setId(int);
    void setPattern(const QString &);
    void setType(enum ParameterGroupTypes);
    void setState(enum ParameterGroupStates);
    void repopulateSelected();

  signals:
    void updated();

  private:
    bool _fixed;
    enum ParameterGroupTypes _type;

    QButtonGroup *_buttonGroup;
    QRadioButton *_all;
    QRadioButton *_selected;
    QRadioButton *_usePattern;
    QWidget      *_selectedGroup;
    QWidget      *_patternGroup;
    XComboBox    *_items;
    QLineEdit    *_pattern;
};

#endif
