/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "csvaddmapinputdialog.h"

#include <QMessageBox>
#include <QSqlError>

#include "xsqlquery.h"

#define DEBUG false

CSVAddMapInputDialog::CSVAddMapInputDialog(QWidget *parent, Qt::WindowFlags f)
  : QDialog(parent, f)
{
  setupUi(this);

  connect(_schema, SIGNAL(currentIndexChanged(int)), this, SLOT(populateTable()));

  populateSchema();
  _schema->setCurrentIndex(0);
}

CSVAddMapInputDialog::~CSVAddMapInputDialog()
{
}

void CSVAddMapInputDialog::languageChange()
{
  retranslateUi(this);
}

QString CSVAddMapInputDialog::mapname() const
{
  return _name->text();
}

void CSVAddMapInputDialog::populateSchema()
{
  XSqlQuery nspq("SELECT '[ All schemas ]' AS nspname, 1 AS seq"
                 " UNION "
                 "SELECT 'public', 2"
                 " UNION "
                 "SELECT nspname, 3"
                 "  FROM pg_namespace"
                 " WHERE ((nspname !~ '^pg_')"
                 "   AND  (nspname != 'public')"
                 "   AND  (nspname != 'information_schema'))"
                 " ORDER BY seq, nspname;");
  if (nspq.exec())
    _schema->clear();
  while (nspq.next())
    _schema->addItem(nspq.value("nspname").toString());
  if (nspq.lastError().type() != QSqlError::NoError)
  {
    QMessageBox::critical(this, tr("Database Error"), nspq.lastError().text());
    return;
  }
}

void CSVAddMapInputDialog::populateTable()
{
  XSqlQuery relq;
  if (_schema->currentIndex() == 0)
    relq.prepare("SELECT CASE nspname WHEN 'public' THEN relname"
                 "                    ELSE nspname || '.' || relname"
                 "       END AS relname,"
                 "       CASE nspname WHEN 'public' THEN 0 ELSE 1 END AS seq"
                 "  FROM pg_class"
                 "  JOIN pg_namespace ON (relnamespace=pg_namespace.oid)"
                 " WHERE ((relkind IN ('r', 'v'))"
                 "   AND  (nspname !~ '^pg_')"
                 "   AND  (nspname != 'information_schema'))"
                 " ORDER BY seq, relname;");
  else
  {
    relq.prepare("SELECT relname"
                 "  FROM pg_class"
                 "  JOIN pg_namespace ON (relnamespace=pg_namespace.oid)"
                 " WHERE ((relkind IN ('r', 'v'))"
                 "   AND  (nspname = :nspname))"
                 " ORDER BY relname;");
    relq.bindValue(":nspname", _schema->currentText());
  }
  if (relq.exec())
    _table->clear();
  while (relq.next())
    _table->addItem(relq.value("relname").toString());
  if (relq.lastError().type() != QSqlError::NoError)
  {
    QMessageBox::critical(this, tr("Database Error"), relq.lastError().text());
    return;
  }
}

QString CSVAddMapInputDialog::qualifiedTable() const
{
  if (_schema->currentIndex() == 0)
    return _table->currentText();
  else
    return QString("%1.%2").arg(_schema->currentText(), _table->currentText());
}

QString CSVAddMapInputDialog::schema() const
{
  if (_schema->currentIndex() <= 0)
    return QString::null;
  else
    return _schema->currentText();
}

void CSVAddMapInputDialog::setMapname(const QString mapname)
{
  if (DEBUG) qDebug("setMapname(%s) entered", qPrintable(mapname));
  _name->setText(mapname);
}

void CSVAddMapInputDialog::setSchema(const QString schema)
{
  if (DEBUG) qDebug("setSchema(%s) entered", qPrintable(schema));
  _schema->setCurrentIndex(_schema->findData(schema, Qt::DisplayRole));
  if (_schema->currentIndex() < 0)
    _schema->setCurrentIndex(0);
}

void CSVAddMapInputDialog::setTable(const QString table)
{
  if (DEBUG) qDebug("setTable(%s) entered", qPrintable(table));
  _table->setCurrentIndex(_table->findData(table, Qt::DisplayRole));
  if (_table->currentIndex() < 0 && table.contains("."))
  {
    QString unqualrelname = table.right(table.length() -
                                        table.lastIndexOf(".") - 1);
    if (DEBUG)
      qDebug("stripped . off %s to get %s",
             qPrintable(table), qPrintable(unqualrelname));
    _table->setCurrentIndex(_table->findData(unqualrelname, Qt::DisplayRole));
  }
}

QString CSVAddMapInputDialog::table() const
{
  return _table->currentText();
}

QString CSVAddMapInputDialog::unqualifiedTable() const
{
  QString result = _table->currentText();
  if (result.contains("."))
  {
    QString unqualrelname = result.right(result.length() -
                                         result.lastIndexOf(".") - 1);
    if (DEBUG)
      qDebug("stripped . off %s to get %s",
             qPrintable(result), qPrintable(unqualrelname));
    result = unqualrelname;
  }

  return result;
}
