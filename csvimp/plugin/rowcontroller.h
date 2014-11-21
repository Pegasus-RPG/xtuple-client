/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __ROWCONTROLLER_H__
#define __ROWCONTROLLER_H__

#include <QObject>

class QComboBox;
class QSpinBox;
class QTableWidget;
class QTableWidgetItem;

class RowController : public QObject
{
  Q_OBJECT

  public:
    RowController(QTableWidget * table, int row, QObject* parent, const char * name = 0);
    virtual ~RowController();

    void setAction(QComboBox*);
    void setColumn(QSpinBox*);
    void setIfNull(QComboBox*);
    void setAltColumn(QSpinBox*);
    void setAltIfNull(QComboBox*);
    void setAltValue(QTableWidgetItem*);

  public slots:
    void finishSetup();

  protected slots:
    void valueChanged(int row, int col);

  private:
    int _row;
    QComboBox        *_action;    // 4
    QSpinBox         *_column;    // 5
    QComboBox        *_ifNull;    // 6
    QSpinBox         *_altColumn; // 7
    QComboBox        *_altIfNull; // 8
    QTableWidgetItem *_altValue;  // 9
};

#endif

