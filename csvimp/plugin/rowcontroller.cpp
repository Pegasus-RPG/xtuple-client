/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "rowcontroller.h"

#include <QComboBox>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>

RowController::RowController(QTableWidget *table, int row, QObject *parent, const char * name)
  : QObject(parent)
{
  setObjectName(name ?
                name : QString("_rowController%1").arg(row).toLatin1().data());
  _row = row;
  _action = 0;
  _column = 0;
  _ifNull = 0;
  _altColumn = 0;
  _altIfNull = 0;
  _altValue = 0;
  connect(table, SIGNAL(cellChanged(int, int)), this, SLOT(valueChanged(int, int)));
}

RowController::~RowController()
{
  _row = 0;
  _action = 0;
  _column = 0;
  _ifNull = 0;
  _altColumn = 0;
  _altIfNull = 0;
  _altValue = 0;
}

void RowController::setAction(QComboBox *combo)
{
  _action = combo;
  connect(_action, SIGNAL(currentIndexChanged(int)), this, SLOT(finishSetup()));
}

void RowController::setColumn(QSpinBox *spinner)
{
  _column = spinner;
  connect(_column, SIGNAL(valueChanged(int)), this, SLOT(finishSetup()));
}

void RowController::setIfNull(QComboBox *combo)
{
  _ifNull = combo;
  connect(_ifNull, SIGNAL(currentIndexChanged(int)), this, SLOT(finishSetup()));
}

void RowController::setAltColumn(QSpinBox *spinner)
{
  _altColumn = spinner;
  connect(_altColumn, SIGNAL(valueChanged(int)), this, SLOT(finishSetup()));
}

void RowController::setAltIfNull(QComboBox *combo)
{
  _altIfNull = combo;
  connect(_altIfNull, SIGNAL(currentIndexChanged(int)), this, SLOT(finishSetup()));
}

void RowController::setAltValue(QTableWidgetItem *item)
{
  _altValue = item;
}

void RowController::finishSetup()
{
  if(!(_action && _column && _ifNull && _altColumn && _altIfNull && _altValue))
  {
    qDebug("RowController::finishSetup() called when not all values set.");
    return;
  }

  QString str = _action->currentText();
  if(str == "Default" || str == "UseEmptyString" || str == "UseNull")
  {
    _column->setEnabled(false);
    _ifNull->setEnabled(false);
    _altColumn->setEnabled(false);
    _altIfNull->setEnabled(false);
    _altValue->setFlags(_altValue->flags() & (~ Qt::ItemIsEditable));
  }
  else if(str == "UseColumn")
  {
    _column->setEnabled(true);
    _ifNull->setEnabled(true);

    str = _ifNull->currentText();
    if(str == "Nothing" || str == "UseDefault" || str == "UseEmptyString")
    {
      _altColumn->setEnabled(false);
      _altIfNull->setEnabled(false);
      _altValue->setFlags(_altValue->flags() & (~ Qt::ItemIsEditable));
    }
    else if(str == "UseAlternateValue")
    {
      _altColumn->setEnabled(false);
      _altIfNull->setEnabled(false);
      _altValue->setFlags(_altValue->flags() | Qt::ItemIsEditable);
    }
    else if(str == "UseAlternateColumn")
    {
      _altColumn->setEnabled(true);
      _altIfNull->setEnabled(true);

      str = _altIfNull->currentText();
      if(str == "UseAlternateValue")
        _altValue->setFlags(_altValue->flags() | Qt::ItemIsEditable);
      else
        _altValue->setFlags(_altValue->flags() & (~ Qt::ItemIsEditable));
    }
  }
  else if(str == "UseAlternateValue")
  {
    _column->setEnabled(false);
    _ifNull->setEnabled(false);
    _altColumn->setEnabled(false);
    _altIfNull->setEnabled(false);
    _altValue->setFlags(_altValue->flags() | Qt::ItemIsEditable);
  }
}

void RowController::valueChanged(int row, int col)
{
  if(row == _row && (col == 4 || col == 6 || col == 8))
    finishSetup();
}
