/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xtablewidget.h"
#include "xsqltablemodel.h"

#include <QBuffer>
#include <QHeaderView>
#include <QMessageBox>
#include <QModelIndex>
#include <QPalette>
#include <QScriptEngine>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlField>
#include <QSqlIndex>
#include <QSqlRelationalTableModel>
#include <QStack>
#include <QTreeWidgetItem>

#include "format.h"
#include "xtsettings.h"
#include "xsqlrelationaldelegate.h"

#define DEBUG true

XTableWidget::XTableWidget(QWidget *parent) : 
  QTableView(parent)
{
  _keyColumns        = 1;
  _resizingInProcess = false;
  _settingsLoaded    = false;
  _forgetful         = false;

  _menu = new QMenu(this);
  _menu->setObjectName("_menu");

  setContextMenuPolicy(Qt::CustomContextMenu);
  verticalHeader()->setDefaultSectionSize(20);
  horizontalHeader()->setStretchLastSection(false);
  horizontalHeader()->setClickable(true);
  horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu); 

  setAlternatingRowColors(true);
  QPalette muted = palette();
  muted.setColor(QPalette::AlternateBase, QColor(0xEE, 0xEE, 0xEE));
  setPalette(muted);

  _mapper = new XDataWidgetMapper(this);
  _model = new XSqlTableModel(this);
  _model->setEditStrategy(QSqlTableModel::OnManualSubmit);
  
  connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(sShowMenu(const QPoint &)));
  connect(horizontalHeader(), SIGNAL(customContextMenuRequested(const QPoint &)),
                    SLOT(sShowHeaderMenu(const QPoint &)));
  connect(horizontalHeader(), SIGNAL(sectionResized(int, int, int)),
          this,     SLOT(sColumnSizeChanged(int, int, int)));
  connect(_model, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SLOT(handleDataChanged(const QModelIndex, const QModelIndex)));
  _windowName = window()->objectName();
}

XTableWidget::~XTableWidget()
{
  if(_x_preferences)
  {
    // window()->objectName() isn't available here, at least for scripted windows!
    QString settingsPrefix = _windowName + "/" + objectName();

    xtsettingsSetValue(settingsPrefix + "/isForgetful", _forgetful);
    QString savedString;
    if(!_forgetful)
    {
      savedString = "";
      for(int i = 0; i < horizontalHeader()->count(); i++)
      {
        int w = -1;
        ColumnProps *cp = _columnByName.value(columnNameFromLogicalIndex(i), 0);
        if (cp)
        {
          if (!cp->stretchy)
            w = cp->savedWidth;
          _columnByName.remove(cp->columnName);
          delete cp;
        }
        if (w >= 0 && ! horizontalHeader()->isSectionHidden(i))
          savedString.append(QString::number(i) + "," +
                             QString::number(horizontalHeader()->sectionSize(i)) + "|");
      }
      xtsettingsSetValue(settingsPrefix + "/columnWidths", savedString);
    }
    savedString = "";
    for(int i = 0; i < horizontalHeader()->count(); i++)
    {
      savedString.append(QString::number(i) + "," + (horizontalHeader()->isSectionHidden(i)?"off":"on") + "|");
    }
    if(!savedString.isEmpty())
      _x_preferences->set(settingsPrefix + "/columnsShown", savedString);
    else
      _x_preferences->remove(settingsPrefix + "/columnsShown");
  }
}

QString XTableWidget::columnNameFromLogicalIndex(const int logicalIndex) const
{
  for (QMap<QString, ColumnProps*>::const_iterator it = _columnByName.constBegin();
       it != _columnByName.constEnd(); it++)
  {
    if (it.value()->logicalIndex == logicalIndex)
      return it.value()->columnName;
  }
  return QString();
}

QString XTableWidget::filter()
{
  return _model->filter();
}

void XTableWidget::handleDataChanged(const QModelIndex topLeft, const QModelIndex lowerRight)
{
  int col;
  int row;

  // Emit data changed signal
  for (col = topLeft.column(); col <= lowerRight.column(); col++) {
    for (row = topLeft.row(); row <= lowerRight.row(); row++)
      emit dataChanged(row, col);
  }
}

void XTableWidget::sShowMenu(const QPoint &pntThis)
{
  QModelIndex item = indexAt(pntThis);
  if (item.isValid())
  {
    _menu->clear();
    emit populateMenu(_menu, item);

    bool disableExport = FALSE;
    if(_x_preferences)
      disableExport = (_x_preferences->value("DisableExportContents")=="t");
    if(!disableExport)
    {
      if (_menu->count())
        _menu->insertSeparator();

      _menu->insertItem(tr("Export Contents..."),  this, SLOT(sExport()));
    }

    if(_menu->count())
      _menu->popup(mapToGlobal(pntThis));
  }
}


void XTableWidget::sShowHeaderMenu(const QPoint &pntThis)
{
  _menu->clear();

  int logicalIndex = horizontalHeader()->logicalIndexAt(pntThis);
  int currentSize = horizontalHeader()->sectionSize(logicalIndex);
// If we have a default value and the current size is not equal to that default value
// then we want to show the menu items for resetting those values back to default
  ColumnProps *cp = _columnByName.value(columnNameFromLogicalIndex(logicalIndex), 0);
  if (cp && (cp->defaultWidth > 0) && (cp->defaultWidth != currentSize) )
  {
    _resetWhichWidth = logicalIndex;
    _menu->insertItem(tr("Reset this Width"), this, SLOT(sResetWidth()));
  }

  _menu->insertItem(tr("Reset all Widths"), this, SLOT(sResetAllWidths()));
  _menu->insertSeparator();
  if(_forgetful)
    _menu->insertItem(tr("Remember Widths"), this, SLOT(sToggleForgetfulness()));
  else
    _menu->insertItem(tr("Do Not Remember Widths"), this, SLOT(sToggleForgetfulness()));

  _menu->insertSeparator();

  for(int i = 0; i < horizontalHeader()->count(); i++)
  {
    QAction *act = _menu->addAction(QTableView::model()->headerData(i, Qt::Horizontal).toString());
    act->setCheckable(true);
    act->setChecked(! horizontalHeader()->isSectionHidden(i));

    ColumnProps *tmp = _columnByName.value(columnNameFromLogicalIndex(i), 0);
    act->setEnabled(tmp ?  ! tmp->locked : true);

    QMap<QString,QVariant> m;
    m.insert("command", QVariant("toggleColumnHidden"));
    m.insert("column", QVariant(i));
    act->setData(m);
    connect(_menu, SIGNAL(triggered(QAction*)), this, SLOT(popupMenuActionTriggered(QAction*)));
  }

  if(_menu->count())
    _menu->popup(mapToGlobal(pntThis));
}

bool XTableWidget::throwScriptException(const QString &message)
{
   QObject *ancestor = this;
   QScriptEngine *engine = 0;
   for ( ; ancestor; ancestor = ancestor->parent())
   {
     engine = ancestor->findChild<QScriptEngine*>();
     if (engine)
       break;
   } 
   if (engine)
   {
      QScriptContext *ctx = engine->currentContext();
      ctx->throwError(message);
      return true;
   }
   return false;
}

int XTableWidget::rowCount()
{
  return _model->rowCount();
}

int XTableWidget::rowCountVisible()
{
  int counter = 0;
  for (int i = 0; i < _model->rowCount(); i++)
  {
    if (!isRowHidden(i))
      counter++;
  }
  return counter;
}

QVariant XTableWidget::value(int row, int column)
{
  return model()->data(model()->index(row,column));
}

QVariant XTableWidget::selectedValue(int column)
{
  QModelIndexList selected=selectedIndexes();
  return value(selected.first().row(),column);
}

void XTableWidget::select()
{
  setTable();
  _model->select();
  
  emit valid(FALSE);

}

void XTableWidget::selectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
  if (!selected.indexes().isEmpty())
  {
    emit rowSelected(selected.indexes().first().row());
    emit valid(true);
  }
  else
    emit valid(FALSE);
  QTableView::selectionChanged(selected, deselected);
}

void XTableWidget::insert()
{ 
  int row=_model->rowCount();
  _model->insertRows(row,1);
  //Set default values for foreign keys
  for (int i = 0; i < _idx.count(); ++i)
    _model->setData(_model->index(row,i),_idx.value(i));
  selectRow(row);
}

void XTableWidget::populate(int p)
{   
  XSqlTableModel *t=static_cast<XSqlTableModel*>(_mapper->model());
  if (t)
  {
    _idx=t->primaryKey();

    for (int i = 0; i < _idx.count(); ++i) {
      _idx.setValue(i, t->data(t->index(p,i)));
      setColumnHidden(i,true);
    }
    
    QString clause = QString(_model->database().driver()->sqlStatement(QSqlDriver::WhereStatement, t->tableName(),_idx, false)).mid(6);
    _model->setFilter(clause);
    if (!_model->query().isActive()) {
      _model->select();
    }
  }
}

void XTableWidget::removeSelected()
{
  QModelIndexList selected=selectedIndexes();

  for (int i = selected.count() -1 ; i > -1; i--) {
    if (selected.value(i).column() == 1) { // Only once per row
      _model->removeRow(selected.value(i).row());
      hideRow(selected.value(i).row());
    }
  }
}

void XTableWidget::revertAll()
{
  _model->revertAll();
  if (_mapper)
    populate(_mapper->currentIndex());
}

void XTableWidget::save()
{ 
  if(!_model->submitAll())
  {
    if(!throwScriptException(_model->lastError().databaseText()))
          QMessageBox::critical(this, tr("Error Saving %1").arg(_tableName),
                            tr("Error saving %1 at %2::%3\n\n%4")
                            .arg(_tableName).arg(__FILE__).arg(__LINE__)
                            .arg(_model->lastError().databaseText()));
  }
  else
    emit saved();
}

void XTableWidget::selectRow(int index)
{
  selectionModel()->select(QItemSelection(model()->index(index,0),model()->index(index,0)),
                                        QItemSelectionModel::ClearAndSelect |
                                        QItemSelectionModel::Rows);
}

void XTableWidget::setDataWidgetMap(XDataWidgetMapper* mapper)
{  
  setTable();
  _mapper=mapper; 
}

void XTableWidget::setFilter(const QString filter)
{  
  _model->setFilter(filter);
}

void XTableWidget::setForegroundColor(int row, int col, QString color)
{
  _model->setData(_model->index(row,col), namedColor(color), Qt::ForegroundRole);
}

void XTableWidget::setFormat(const QString column, int format)
{
  setColumnRole(column, XSqlTableModel::FormatRole, QVariant(format));
}

void XTableWidget::setModel(XSqlTableModel * model)
{
  QTableView::setModel(model);

  for (int i = 0; i < QTableView::model()->columnCount(); ++i)
  {
    QString h = QTableView::model()->headerData(i, Qt::Horizontal,
                                               Qt::DisplayRole).toString();
    QTableView::model()->setHeaderData(i, Qt::Horizontal, h, Qt::UserRole);

    ColumnProps *cp = _columnByName.value(h, 0);

    if (! cp)
    {
      cp = new ColumnProps;
      cp->columnName   = h;
      cp->logicalIndex = i;
      cp->defaultWidth = -1;
      cp->savedWidth   = -1;
      cp->stretchy     = true;
      cp->locked       = false;
      cp->visible      = true;
      cp->label        = h;
      cp->fromSettings = false;
      cp->alignment    = QTableView::model()->headerData(i, Qt::Horizontal,
                                                Qt::TextAlignmentRole).toInt();

      cp->label.replace(0, 1, cp->label.left(1).toUpper());
      for (int j = 0; (j = cp->label.indexOf("_", j)) >= 0; /* empty */)
      {
        cp->label.replace(j,     1, " ");
        cp->label.replace(j + 1, 1, cp->label.mid(j + 1, 1).toUpper());
      }

      _columnByName.insert(cp->columnName, cp);
    }

    QTableView::model()->setHeaderData(i, Qt::Horizontal, cp->label,
                                      Qt::DisplayRole);
    QTableView::model()->setHeaderData(i, Qt::Horizontal, cp->alignment,
                                      Qt::TextAlignmentRole);
  }

  if (! _settingsLoaded)
  {
    QString settingsPrefix = _windowName + "/" + objectName();
    _settingsLoaded = true;

    _forgetful = xtsettingsValue(settingsPrefix + "/isForgetful").toBool();
    QString savedString;
    QStringList savedParts;
    QString part, key, val;
    bool b1 = false, b2 = false;
    if(!_forgetful)
    {
      savedString = xtsettingsValue(settingsPrefix + "/columnWidths").toString();
      savedParts = savedString.split("|", QString::SkipEmptyParts);
      for(int i = 0; i < savedParts.size(); i++)
      {
        part = savedParts.at(i);
        key = part.left(part.indexOf(","));
        val = part.right(part.length() - part.indexOf(",") - 1);
        b1 = false;
        b2 = false;
        int k = key.toInt(&b1);
        int v = val.toInt(&b2);
        if(b1 && b2)
        {
          ColumnProps *cp = _columnByName.value(columnNameFromLogicalIndex(k), 0);
          if (cp)
          {
            cp->savedWidth = v;
            cp->stretchy   = false;
            cp->fromSettings = true;
          }
          horizontalHeader()->resizeSection(k, v);
        }
      }
    }

    // Load any previously saved column hidden/visible information
    if(_x_preferences)
    {
      savedString = _x_preferences->value(settingsPrefix + "/columnsShown");
      savedParts = savedString.split("|", QString::SkipEmptyParts);
      for(int i = 0; i < savedParts.size(); i++)
      {
        part = savedParts.at(i);
        key = part.left(part.indexOf(","));
        val = part.right(part.length() - part.indexOf(",") - 1);
        int c = key.toInt(&b1);
        if(b1 && (val == "on" || val == "off"))
        {
          ColumnProps *cp = _columnByName.value(columnNameFromLogicalIndex(c), 0);
          if (cp)
          {
            cp->visible = (val == "on");
            cp->fromSettings = true;
          }
          horizontalHeader()->setSectionHidden(c, (val != "on"));
        }
      }
    }
  }
  emit newModel(model);
}

void XTableWidget::setRowForegroundColor(int row, QString color)
{
  for (int col = 0; col < _model->query().record().count(); col++)
    setForegroundColor(row, col, color);
}

void XTableWidget::setTable()
{
  if (_model->tableName() == _tableName)
    return;
      
  if (!_tableName.isEmpty())
  {
    QString tablename=_tableName;
    if (!_schemaName.isEmpty())
      tablename = _schemaName + "." + tablename;
    _model->setTable(tablename,_keyColumns);
    
    setModel(_model);
    setItemDelegate(new XSqlRelationalDelegate(this));
  //  setRelations();
  }
}

void XTableWidget::setTextAlignment(int column, int alignment)
{
  _model->setColumnRole(column, Qt::TextAlignmentRole, QVariant(alignment));
}

void XTableWidget::setTextAlignment(const QString column, int alignment)
{
  setColumnRole(column, Qt::TextAlignmentRole, QVariant(alignment));
}

void XTableWidget::setValue(int row, int column, QVariant value)
{
  _model->setData(_model->index(row,column), value);
}

void XTableWidget::resizeEvent(QResizeEvent * e)
{
  QTableView::resizeEvent(e);

  sColumnSizeChanged(-1, 0, 0);
}

void XTableWidget::sResetWidth()
{
  ColumnProps *cp = _columnByName.value(columnNameFromLogicalIndex(_resetWhichWidth), 0);

  int w = cp ? cp->defaultWidth : -1;

  if(w >= 0)
    horizontalHeader()->resizeSection(_resetWhichWidth, w);
  else
    sColumnSizeChanged(-1, 0, 0);
}

void XTableWidget::sResetAllWidths()
{
  bool autoSections = false;
  for (int i = 0; i < horizontalHeader()->length(); i++)
  {
    ColumnProps *cp = _columnByName.value(columnNameFromLogicalIndex(i), 0);
    int width = cp ? cp->defaultWidth : -1;
    if (width >= 0)
      horizontalHeader()->resizeSection(i, width);
    else
    {
    if (cp)
        cp->stretchy = true;
      autoSections = true;
    }
  }
  if(autoSections)
    sColumnSizeChanged(-1, 0, 0);
}

void XTableWidget::sToggleForgetfulness()
{
  _forgetful = !_forgetful;
}


void XTableWidget::sColumnSizeChanged(int logicalIndex, int /*oldSize*/, int newSize)
{
  ColumnProps *cp = _columnByName.value(columnNameFromLogicalIndex(logicalIndex), 0);
  if (cp)
  {
    cp->savedWidth = newSize;
    if (newSize < 0)
      cp->stretchy = true;
    else if (! _resizingInProcess && cp->stretchy)
      cp->stretchy = false;
  }

  if (_resizingInProcess)
    return;

  _resizingInProcess = true;

  int usedSpace = 0;
  QVector<int> stretch;

  for(int i = 0; i < horizontalHeader()->count(); i++)
  {
    ColumnProps *tmpcp = _columnByName.value(columnNameFromLogicalIndex(i), 0);

    if (horizontalHeader()->isSectionHidden(i))
      continue;
    else if (logicalIndex == i && newSize < 0)
      stretch.append(i);
    else if (logicalIndex == i)
      usedSpace += newSize;
    else if (tmpcp && ! tmpcp->stretchy)
      usedSpace += horizontalHeader()->sectionSize(i);
    else
      stretch.append(i);
  }

  if(stretch.size() > 0)
  {
    int leftover = (viewport()->width() - usedSpace) / stretch.size();

    if(leftover < 50)
      leftover = 50;

    for (int n = 0; n < stretch.size(); n++)
      horizontalHeader()->resizeSection(stretch.at(n), leftover);
  }

  _resizingInProcess = false;
}

/* this is intended to be similar to XTreeWidget::addColumn(...)
   but differs in several important ways:
   - this doesn't actually add columns to the tree view, it only defines how
     they should appear by default if the model contains columns with the given
     colname.
   - as a result it has the opposite effect on visibility - any column not
     explicitly mentioned in a setColumn call /is visible/ in an XTableWidget
   - the alignment parameter only affects the header
TODO: figure out how to set the alignment of individual elements
 */
 
void XTableWidget::setColumn(const QString &label, int width, int alignment, bool visible, const QString &colname)
{
  ColumnProps *cp = _columnByName.value(colname, 0);

  if (! cp)
  {
    cp = new ColumnProps();
    cp->columnName = colname;
    cp->fromSettings = false;
    _columnByName.insert(cp->columnName, cp);
  }

  cp->defaultWidth = width;
  cp->locked       = false;
  cp->visible      = visible;
  cp->label        = label;
  cp->alignment    = alignment;

  if (cp->fromSettings)
    cp->stretchy   = (width == -1 || cp->savedWidth == -1);
  else
  {
    cp->savedWidth = -1;
    cp->stretchy   = (width == -1);
  }

  cp->logicalIndex = -1;
  int colnum = -1;
  for (colnum = 0; colnum < horizontalHeader()->count(); colnum++)
  {
    if (colname == QTableView::model()->headerData(colnum, Qt::Horizontal,
                                                  Qt::UserRole).toString())
    {
      cp->logicalIndex = colnum;
      QTableView::model()->setHeaderData(colnum, Qt::Horizontal, colname,
                                        Qt::UserRole);
      QTableView::model()->setHeaderData(colnum, Qt::Horizontal, label,
                                        Qt::DisplayRole);
      QTableView::model()->setHeaderData(colnum, Qt::Horizontal, alignment,
                                        Qt::TextAlignmentRole);
    /* setData below always returns false => failure. why?
      for (int i = 0; i < QTableView::model()->rowCount(); i++)
        bool test = QTableView::model()->setData(QTableView::model()->index(i, colnum),
                                                alignment, Qt::TextAlignmentRole);
      break;
      */
    }
  }

  horizontalHeader()->setResizeMode(colnum, QHeaderView::Interactive);

  if (! cp->fromSettings)
    setColumnVisible(colnum, visible);

  if (_forgetful || ! cp->fromSettings)
    horizontalHeader()->resizeSection(colnum, width);
}

void XTableWidget::setColumnLocked(const QString &pColname, bool pLocked)
{
  if (_columnByName.value(pColname))
    _columnByName.value(pColname)->locked = pLocked;
}

void XTableWidget::setColumnLocked(int pColumn, bool pLocked)
{
  ColumnProps *cp = _columnByName.value(columnNameFromLogicalIndex(pColumn), 0);
  if (cp)
    cp->locked = pLocked;
}


void XTableWidget::setColumnRole(const QString column, int role, const QVariant value)
{
  for (int colnum = 0; colnum < horizontalHeader()->count(); colnum++) {
    if (column == QTableView::model()->headerData(colnum, Qt::Horizontal,
                                                  Qt::UserRole).toString()) {
      _model->setColumnRole(colnum, role, value);
    }
  }
}

void XTableWidget::popupMenuActionTriggered(QAction * pAction)
{
  QMap<QString, QVariant> m = pAction->data().toMap();
  QString command = m.value("command").toString();
  if("toggleColumnHidden" == command)
    setColumnVisible(m.value("column").toInt(), pAction->isChecked());
  //else if (some other command to handle)
}

void XTableWidget::setColumnVisible(int pColumn, bool pVisible)
{
  if(pVisible)
    horizontalHeader()->showSection(pColumn);
  else
    horizontalHeader()->hideSection(pColumn);

  ColumnProps *cp = _columnByName.value(columnNameFromLogicalIndex(pColumn), 0);
  if (cp)
    cp->visible = pVisible;
}

/*
void XTableWidget::setRelations()
{
  if (DEBUG) qDebug("setRelations() entered with table  %s.%s",
                    qPrintable(_schemaName), qPrintable(_tableName));
  QSqlRelationalTableModel *model = qobject_cast<QSqlRelationalTableModel*>(_model);
  if (! model)
    return;

  // _fkeymap is for cases where there's strict adherence to naming conventions:
  //    othertable[_qualifier]_basetable_id -> basetable_id
  // special cases are handled individually below
  // TODO: make _fkeymap static so it can be shared
  if (_fkeymap.size() == 0)
  {
    _fkeymap.insert("acalitem",    "acalitem_name");
    _fkeymap.insert("accnt",       "accnt_number");
    _fkeymap.insert("addr",        "addr_number");
    _fkeymap.insert("alarm",       "alarm_number");
    _fkeymap.insert("bankaccnt",   "bankaccnt_name");
    _fkeymap.insert("bankadjtype", "bankadjtype_name");
    _fkeymap.insert("budghead",    "budghead_name");
    _fkeymap.insert("calhead",     "calhead_name");
    _fkeymap.insert("carrier",     "carrier_name");
    _fkeymap.insert("char",        "char_name");
    _fkeymap.insert("checkhead",   "checkhead_number");
    _fkeymap.insert("classcode",   "classcode_code");
    _fkeymap.insert("cmd",         "cmd_title");
    _fkeymap.insert("cmdarg",      "cmdarg_arg");
    _fkeymap.insert("cmhead",      "cmhead_number");
    _fkeymap.insert("cmnttype",    "cmnttype_name");
    _fkeymap.insert("cntct",       "cntct_number");
    _fkeymap.insert("cntslip",     "cntslip_number");
    _fkeymap.insert("cohead",      "cohead_number");
    _fkeymap.insert("company",     "company_number");
    _fkeymap.insert("costcat",     "costcat_code");
    _fkeymap.insert("costelem",    "costelem_type");
    _fkeymap.insert("country",     "country_abbr");
    _fkeymap.insert("crmacct",     "crmacct_number");
    _fkeymap.insert("curr",        "curr_symbol");
    _fkeymap.insert("curr_symbol", "curr_abbr");
    _fkeymap.insert("custgrp",     "custgrp_name");
    _fkeymap.insert("custinfo",    "cust_number");
    _fkeymap.insert("custtype",    "custtype_code");
    _fkeymap.insert("dept",        "dept_number");
    _fkeymap.insert("destination", "destination_name");
    _fkeymap.insert("ediform",     "ediform_file");
    _fkeymap.insert("ediprofile",  "ediprofile_name");
    _fkeymap.insert("emp",         "emp_code");
    _fkeymap.insert("empgrp",      "empgrp_name");
    _fkeymap.insert("evnttype",    "evnttype_name");
    _fkeymap.insert("expcat",      "expcat_code");
    _fkeymap.insert("flhead",      "flhead_name");
    _fkeymap.insert("form",        "form_name");
    _fkeymap.insert("freightclass","freightclass_code");
    _fkeymap.insert("grp",         "grp_name");
    _fkeymap.insert("hnfc",        "hnfc_code");
    _fkeymap.insert("image",       "image_name");
    _fkeymap.insert("incdt",       "incdt_number");
    _fkeymap.insert("incdtcat",    "incdtcat_name");
    _fkeymap.insert("incdtpriority",  "incdtpriority_name");
    _fkeymap.insert("incdtresolution","incdtresolution_name");
    _fkeymap.insert("incdtseverity",  "incdtseverity_name");
    _fkeymap.insert("invchead",    "invchead_invcnumber");
    _fkeymap.insert("ipshead",     "ipshead_name");
    _fkeymap.insert("item",        "item_number");
    _fkeymap.insert("itemalias",   "itemalias_number");
    _fkeymap.insert("itemgrp",     "itemgrp_name");
    _fkeymap.insert("jrnluse",     "jrnluse_number");
    _fkeymap.insert("labelform",   "labelform_name");
    _fkeymap.insert("lang",        "lang_name");
    _fkeymap.insert("lbrrate",     "lbrrate_code");
    _fkeymap.insert("locale",      "locale_code");
    _fkeymap.insert("location",    "location_name");
    _fkeymap.insert("ls",          "ls_number");
    _fkeymap.insert("lsreg",       "lsreg_number");
    _fkeymap.insert("metric",      "metric_name");
    _fkeymap.insert("metricenc",   "metricenc_name");
    _fkeymap.insert("ophead",      "ophead_name");
    _fkeymap.insert("opsource",    "opsource_name");
    _fkeymap.insert("opstage",     "opstage_name");
    _fkeymap.insert("optype",      "optype_name");
    _fkeymap.insert("orderseq",    "orderseq_name");
    _fkeymap.insert("period",      "period_name");
    _fkeymap.insert("pkghead",     "pkghead_name");
    _fkeymap.insert("plancode",    "plancode_code");
    _fkeymap.insert("planord",     "planord_number");
    _fkeymap.insert("pohead",      "pohead_number");
    _fkeymap.insert("potype",      "potype_name");
    _fkeymap.insert("pr",          "pr_number");
    _fkeymap.insert("prftcntr",    "prftcntr_number");
    _fkeymap.insert("prj",         "prj_number");
    _fkeymap.insert("prjtask",     "prjtask_number");
    _fkeymap.insert("prodcat",     "prodcat_code");
    _fkeymap.insert("prospect",    "prospect_number");
    _fkeymap.insert("pschhead",    "pschhead_number");
    _fkeymap.insert("quhead",      "quhead_number");
    _fkeymap.insert("rahead",      "rahead_number");
    _fkeymap.insert("regtype",     "regtype_code");
    _fkeymap.insert("rev",         "rev_number");
    _fkeymap.insert("rjctcode",    "rjctcode_code");
    _fkeymap.insert("rsncode",     "rsncode_code");
    _fkeymap.insert("sale",        "sale_name");
    _fkeymap.insert("salehead",    "salehead_number");
    _fkeymap.insert("salescat",    "salescat_name");
    _fkeymap.insert("salesrep",    "salesrep_number");
    _fkeymap.insert("script",      "script_name");
    _fkeymap.insert("shift",       "shift_number");
    _fkeymap.insert("shipchrg",    "shipchrg_name");
    _fkeymap.insert("shipform",    "shipform_name");
    _fkeymap.insert("shiphead",    "shiphead_number");
    _fkeymap.insert("shiptoinfo",  "shipto_name");
    _fkeymap.insert("shipvia",     "shipvia_code");
    _fkeymap.insert("shipzone",    "shipzone_name");
    _fkeymap.insert("sitetype",    "sitetype_name");
    _fkeymap.insert("stdjrnl",     "stdjrnl_name");
    _fkeymap.insert("stdjrnlgrp",  "stdjrnlgrp_name");
    _fkeymap.insert("stdopn",      "stdopn_number");
    _fkeymap.insert("subaccnt",    "subaccnt_number");
    _fkeymap.insert("subaccnttype","subaccnttype_code");
    _fkeymap.insert("tax",         "tax_code");
    _fkeymap.insert("taxauth",     "taxauth_code");
    _fkeymap.insert("taxreg",      "taxreg_number");
    _fkeymap.insert("taxtype",     "taxtype_name");
    _fkeymap.insert("terminal",    "terminal_number");
    _fkeymap.insert("terms",       "terms_code");
    _fkeymap.insert("todoitem",    "todoitem_name");
    _fkeymap.insert("tohead",      "tohead_number");
    _fkeymap.insert("uiform",      "uiform_name");
    _fkeymap.insert("uom",         "uom_name");
    _fkeymap.insert("uomtype",     "uomtype_name");
    _fkeymap.insert("url",         "url_url");
    _fkeymap.insert("usr",         "usr_username");
    _fkeymap.insert("vendaddrinfo","vendaddr_code");
    _fkeymap.insert("vendinfo",    "vend_number");
    _fkeymap.insert("vendtype",    "vendtype_code");
    _fkeymap.insert("vohead",      "vohead_number");
    _fkeymap.insert("whsezone",    "whsezone_name");
    _fkeymap.insert("whsinfo",     "warehous_code");
    _fkeymap.insert("wo",          "wo_number");
    _fkeymap.insert("wrkcnt",      "wrkcnt_code");
    _fkeymap.insert("xsltmap",     "xsltmap_name");
    _fkeymap.insert("yearperiod",  "yearperiod_start");
  }

  for (int i = 0; i < model->record().count(); i++)
  {
    QString colname = model->record().field(i).name();
    if (DEBUG) qDebug("looking for fkey to %s", qPrintable(colname));

    if (colname.endsWith("curr_id"))
      model->setRelation(i, QSqlRelation("curr_symbol", "curr_id", "curr_abbr"));
    else if (colname.endsWith("cust_id"))
      model->setRelation(i, QSqlRelation("custinfo", "cust_id", "cust_number"));
    else if (colname.endsWith("shipto_id"))
      model->setRelation(i, QSqlRelation("shiptoinfo","shipto_id","shipto_name"));
    else if (colname.endsWith("vend_id"))
      model->setRelation(i, QSqlRelation("vendinfo", "vend_id", "vend_number"));
    else if (colname.endsWith("vendaddr_id"))
      model->setRelation(i, QSqlRelation("vendaddrinfo","vendaddr_id","vendaddr_code"));
    else if (colname.endsWith("warehous_id"))
      model->setRelation(i, QSqlRelation("whsinfo", "warehous_id", "warehous_code"));
    else if (colname.endsWith("_id"))
    {
      colname.replace(QRegExp(".*_([^_]+)_id$"), "\\1");

      if (DEBUG) qDebug("%s is an id", qPrintable(colname));
      if (! _fkeymap.value(colname, QString()).isEmpty())
      {
        if (DEBUG) qDebug("setting relation(%s, %s, %s)",
                          qPrintable(colname), qPrintable(colname + "_id"),
                          qPrintable(_fkeymap.value(colname)));
        model->setRelation(i, QSqlRelation(colname, colname + "_id",
                                           _fkeymap.value(colname)));
      }
    }
  }
}
*/