/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <limits>

#include "xtreewidget.h"

#include <QAction>
#include <QApplication>
#include <QAbstractItemView>
#include <QClipboard>
#include <QDate>
#include <QDateTime>
#include <QDrag>
#include <QFileDialog>
#include <QFont>
#include <QHeaderView>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QSqlError>
#include <QSqlField>
#include <QSqlRecord>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextDocumentWriter>
#include <QTextEdit>
#include <QTextTable>
#include <QTextTableCell>
#include <QTextTableFormat>

#include "xtsettings.h"
#include "xsqlquery.h"
#include "format.h"

#define DEBUG false

/* make sure the colroles are kept in sync with
   QStringList knownroles in populate() below,
   both in count and order
   */
#define COLROLE_DISPLAY          0
#define COLROLE_TEXTALIGNMENT    1
#define COLROLE_BACKGROUND       2
#define COLROLE_FOREGROUND       3
#define COLROLE_TOOLTIP          4
#define COLROLE_STATUSTIP        5
#define COLROLE_FONT             6
#define COLROLE_KEY              7
#define COLROLE_RUNNING          8
#define COLROLE_RUNNINGINIT      9
#define COLROLE_GROUPRUNNING    10
#define COLROLE_TOTAL           11
#define COLROLE_NUMERIC         12
#define COLROLE_NULL            13
#define COLROLE_ID              14
// make sure COLROLE_COUNT = last COLROLE + 1
#define COLROLE_COUNT           15

#define ROWROLE_INDENT           0
#define ROWROLE_HIDDEN           1
// make sure ROWROLE_COUNT = last ROWROLE + 1
#define ROWROLE_COUNT            2


enum XTRole { RawRole = (Qt::UserRole + 1),
              ScaleRole,
              IdRole,
              RunningSetRole,
              RunningInitRole,
              TotalSetRole,
              TotalInitRole,
              // KeyRole,
              // GroupRunningRole,
              IndentRole
            };

//cint() and round() regarding Issue #8897
#include <cmath>

static double cint(double x){
  double intpart, fractpart;
  fractpart = modf (x, &intpart);

  if (fabs(fractpart) >= 0.5)
    return x>=0?ceil(x):floor(x);
  else
    return x<0?ceil(x):floor(x);
}

static double round(double r,unsigned places){
  double off=pow(10,places);
  return cint(r*off)/off;
}

XTreeWidget::XTreeWidget(QWidget *pParent) :
  QTreeWidget(pParent)
{
  _resizingInProcess = false;
  _forgetful = false;
  _forgetfulOrder = false;
  _settingsLoaded = false;
  _menu = new QMenu(this);
  _menu->setObjectName("_menu");
  _scol = -1;
  _sord = Qt::AscendingOrder;

  setContextMenuPolicy(Qt::CustomContextMenu);
  setSelectionBehavior(QAbstractItemView::SelectRows);
  header()->setStretchLastSection(false);
  header()->setClickable(true);
  //setMultiSelection(FALSE);

  connect(header(), SIGNAL(sectionClicked(int)), this, SLOT(sHeaderClicked(int)));
  connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(sSelectionChanged()));
  connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(sItemSelected(QTreeWidgetItem *, int)));
  connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(sShowMenu(const QPoint &)));
  connect(header(), SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(sShowHeaderMenu(const QPoint &)));
  connect(header(), SIGNAL(sectionResized(int, int, int)),
          this,     SLOT(sColumnSizeChanged(int, int, int)));

  emit valid(FALSE);
  setColumnCount(0);

  header()->setContextMenuPolicy(Qt::CustomContextMenu);

#ifdef Q_WS_MAC
  QFont f = font();
  f.setPointSize(f.pointSize() - 2);
  setFont(f);
#endif
}

XTreeWidget::~XTreeWidget()
{
  if(_x_preferences)
  {
    xtsettingsSetValue(_settingsName + "/isForgetful", _forgetful);
    xtsettingsSetValue(_settingsName + "/isForgetfulOrder", _forgetfulOrder);
    QString savedString;
    if(!_forgetful)
    {
      savedString = "";
      for(int i = 0; i < header()->count(); i++)
      {
        int w = -1;
        if(_defaultColumnWidths.contains(i))
          w = _defaultColumnWidths.value(i);
        if(!_stretch.contains(i) && header()->sectionSize(i) != w && !header()->isSectionHidden(i))
          savedString.append(QString::number(i) + "," + QString::number(header()->sectionSize(i)) + "|");
      }
      xtsettingsSetValue(_settingsName + "/columnWidths", savedString);
    }
    if(!_forgetfulOrder && header()->isSortIndicatorShown())
      savedString = QString::number(header()->sortIndicatorSection()) + " "
        + (header()->sortIndicatorOrder() == Qt::AscendingOrder ? "ASC" : "DESC" );
    else
      savedString = "-1,ASC";
    xtsettingsSetValue(_settingsName + "/sortOrder", savedString);
    savedString = "";
    for(int i = 0; i < header()->count(); i++)
    {
      savedString.append(QString::number(i) + "," + (header()->isSectionHidden(i)?"off":"on") + "|");
    }
    if(!savedString.isEmpty())
      _x_preferences->set(_settingsName + "/columnsShown", savedString);
    else
      _x_preferences->remove(_settingsName + "/columnsShown");
  }

  for (int i = 0; i < _roles.size(); i++)
    delete _roles.value(i);
  _roles.clear();
}

void XTreeWidget::populate(const QString &pSql, bool pUseAltId)
{
  qApp->setOverrideCursor(Qt::waitCursor);

  XSqlQuery query(pSql);
  populate(query, pUseAltId);

  qApp->restoreOverrideCursor();
}

void XTreeWidget::populate(const QString &pSql, int pIndex, bool pUseAltId)
{
  qApp->setOverrideCursor(Qt::waitCursor);

  XSqlQuery query(pSql);
  populate(query, pIndex, pUseAltId);

  qApp->restoreOverrideCursor();
}

void XTreeWidget::populate(XSqlQuery pQuery, bool pUseAltId)
{
  populate(pQuery, id(), pUseAltId);
}

void XTreeWidget::populate(XSqlQuery pQuery, int pIndex, bool pUseAltId)
{
  qApp->setOverrideCursor(Qt::waitCursor);

  if (pIndex < 0)
    pIndex = id();

  clear();

  if (pQuery.first())
  {
    int fieldCount = pQuery.count();
    if (fieldCount > 1)
    {
      XTreeWidgetItem *last = NULL;

      if (_roles.size() > 0) // xtreewidget columns are tied to query columns
      {
        int colIdx[_roles.size()];
        int colRole[fieldCount][COLROLE_COUNT];
        int rowRole[ROWROLE_COUNT];

        QSqlRecord currRecord = pQuery.record();

        // apply indent and hidden roles to col 0 if the caller requested them
        // keep synchronized with #define ROWROLE_* above
        if (rootIsDecorated())
        {
          rowRole[ROWROLE_INDENT] = currRecord.indexOf("xtindentrole");
          if (rowRole[ROWROLE_INDENT] < 0)
            rowRole[ROWROLE_INDENT] = 0;
        }
        else
          rowRole[ROWROLE_INDENT] = 0;

        rowRole[ROWROLE_HIDDEN] = currRecord.indexOf("xthiddenrole");
        if (rowRole[ROWROLE_HIDDEN] < 0)
          rowRole[ROWROLE_HIDDEN] = 0;

        // keep synchronized with #define COLROLE_* above
	QStringList knownroles;
	knownroles << "qtdisplayrole"      << "qttextalignmentrole"
		   << "qtbackgroundrole"   << "qtforegroundrole"
		   << "qttooltiprole"      << "qtstatustiprole"
		   << "qtfontrole"	   << "xtkeyrole"
		   << "xtrunningrole"	   << "xtrunninginit"
		   << "xtgrouprunningrole" << "xttotalrole"
                   << "xtnumericrole"      << "xtnullrole"
                   << "xtidrole";
	for (int wcol = 0; wcol < _roles.size(); wcol++)
	{
	  QVariantMap *role = _roles.value(wcol);
          if (! role)
          {
            qWarning("XTreeWidget::populate() there is no role for column %d", wcol);
            continue;
          }
	  QString colname = role->value("qteditrole").toString();
          colIdx[wcol] = currRecord.indexOf(colname);

	  for (int k = 0; k < knownroles.size(); k++)
	  {
            // apply Qt roles to a whole row by applying to each column
            colRole[wcol][k] = knownroles.at(k).startsWith("qt") ?
                                 currRecord.indexOf(knownroles.at(k)) :
                                 0;
            if (colRole[wcol][k] > 0)
            {
	      role->insert(knownroles.at(k),
			   QString(knownroles.at(k)));
            }
            else
              colRole[wcol][k] = 0;

            // apply column-specific roles second to override entire row settings
            if (currRecord.indexOf(colname + "_" + knownroles.at(k)) >=0)
	    {
              colRole[wcol][k] = currRecord.indexOf(colname + "_" + knownroles.at(k));
	      role->insert(knownroles.at(k),
			   QString(colname + "_" + knownroles.at(k)));
              if (knownroles.at(k) == "xtrunningrole")
                headerItem()->setData(wcol, Qt::UserRole, "xtrunningrole");
              else if (knownroles.at(k) == "xttotalrole")
                headerItem()->setData(wcol, Qt::UserRole, "xttotalrole");
	    }
	  }
	}

        if (rowRole[ROWROLE_INDENT])
          setIndentation(10);
        else
          setIndentation(0);

        int defaultScale = decimalPlaces("");
        QString yesStr = tr("Yes");
        QString noStr  = tr("No");

	do
	{
          int id = pQuery.value(0).toInt();
          int altId = (pUseAltId) ? pQuery.value(1).toInt() : -1;
          int indent = 0;
          int lastindent = 0;
          if (rowRole[ROWROLE_INDENT])
          {
            indent = pQuery.value(rowRole[ROWROLE_INDENT]).toInt();
            if (indent < 0)
              indent = 0;
            if (last)
            {
              lastindent = last->data(0, IndentRole).toInt();
              if (DEBUG) qDebug("getting xtindentrole from %p of %d", last, lastindent);
            }
          }
          if (DEBUG)
            qDebug("%s::populate() with id %d altId %d indent %d lastindent %d",
                     qPrintable(objectName()), id, altId, indent, lastindent);

          if (indent == 0)
	    last = new XTreeWidgetItem(this, id, altId);
          else if (lastindent < indent)
            last = new XTreeWidgetItem(last, id, altId);
          else if (lastindent == indent)
            last = new XTreeWidgetItem((XTreeWidgetItem*)(last->parent()), id, altId);
          else if (lastindent > indent)
          {
            XTreeWidgetItem *prev = (XTreeWidgetItem*)(last->parent());
            while (prev &&
                   prev->data(0, IndentRole).toInt() >= indent)
              prev = (XTreeWidgetItem*)(prev->parent());
            if (prev)
              last = new XTreeWidgetItem(prev, id, altId);
            else
              last = new XTreeWidgetItem(this, id, altId);
          }
          else
	    last = new XTreeWidgetItem(this, last, id, altId);

          if (rowRole[ROWROLE_INDENT])
            last->setData(0, IndentRole, indent);

          if (rowRole[ROWROLE_HIDDEN])
          {
            if (DEBUG)
              qDebug("%s::populate() found xthiddenrole, value = %s",
                     qPrintable(objectName()),
                     qPrintable(pQuery.value(rowRole[ROWROLE_HIDDEN]).toString()));
            last->setHidden(pQuery.value(rowRole[ROWROLE_HIDDEN]).toBool());
          }

          bool allNull = (indent > 0);
	  for (int col = 0; col < _roles.size(); col++)
	  {
	    QVariantMap *role = _roles.value(col);
            if (! role)
            {
              qWarning("XTreeWidget::populate() there is no role for column %d", col);
              continue;
            }
            QVariant    rawValue = pQuery.value(colIdx[col]);
            last->setData(col, RawRole, rawValue);

            // TODO: this isn't necessary for all columns so do less often?
            int scale = defaultScale;
            QString numericrole = "";
            if (colRole[col][COLROLE_NUMERIC])
            {
              numericrole = pQuery.value(colRole[col][COLROLE_NUMERIC]).toString();
              scale = decimalPlaces(numericrole);
            }
            if (colRole[col][COLROLE_NUMERIC] ||
                colRole[col][COLROLE_RUNNING] ||
                colRole[col][COLROLE_TOTAL])
              last->setData(col, ScaleRole, scale);

	    /* if qtdisplayrole IS NULL then let the raw value shine through.
	       this allows UNIONS to do interesting things, like put dates and
	       text into the same visual column without SQL errors.
	    */
	    if (colRole[col][COLROLE_DISPLAY] &&
		! pQuery.value(colRole[col][COLROLE_DISPLAY]).isNull())
            {
              /* this might not handle PostgreSQL NUMERICs properly
                 but at least it will try to handle INTEGERs and DOUBLEs
                 and it will avoid formatting sales order numbers with decimal
                 and group separators
              */
              QSqlField field = currRecord.field(role->value("qtdisplayrole").toString());
              if (field.type() == QVariant::Int)
                last->setData(col, Qt::DisplayRole,
                              QLocale().toString(field.value().toInt()));
              else if (field.type() == QVariant::Double)
                last->setData(col, Qt::DisplayRole,
                              QLocale().toString(field.value().toDouble(),
                                                 'f', scale));
              else
                last->setData(col, Qt::DisplayRole, field.value().toString());
            }
            else if (rawValue.isNull())
            {
              last->setData(col, Qt::DisplayRole,
                            colRole[col][COLROLE_NULL] ?
                            pQuery.value(colRole[col][COLROLE_NULL]).toString() :
                            "");
            }
            else if (colRole[col][COLROLE_NUMERIC] &&
                     ((numericrole == "percent") ||
                      (numericrole == "scrap")))
            {
              last->setData(col, Qt::DisplayRole,
                            QLocale().toString(rawValue.toDouble() * 100.0,
                                               'f', scale));
            }
            else if (colRole[col][COLROLE_NUMERIC] || rawValue.type() == QVariant::Double)
            {
              //Issue #8897
              last->setData(col, Qt::DisplayRole,
                            QLocale().toString(round(rawValue.toDouble(), scale),
                                               'f', scale));
            }
            else if (rawValue.type() == QVariant::Bool)
            {
	      last->setData(col, Qt::DisplayRole,
                            rawValue.toBool() ? yesStr : noStr);
            }
            else
            {
              last->setData(col, Qt::EditRole, rawValue);
            }

            if (indent)
            {
              if (! colRole[col][COLROLE_DISPLAY] ||
                   (colRole[col][COLROLE_DISPLAY] &&
                    pQuery.value(colRole[col][COLROLE_DISPLAY]).isNull()))
                allNull &= (rawValue.isNull() || rawValue.toString().isEmpty());
              else
                allNull &= pQuery.value(colRole[col][COLROLE_DISPLAY]).isNull() ||
                           pQuery.value(colRole[col][COLROLE_DISPLAY]).toString().isEmpty();

              if (DEBUG)
                qDebug("%s::populate() allNull = %d at %d for rawValue %s",
                       qPrintable(objectName()), allNull, col,
                       qPrintable(rawValue.toString()));
            }

	    if (colRole[col][COLROLE_FOREGROUND])
            {
              QVariant fg = pQuery.value(colRole[col][COLROLE_FOREGROUND]);
	      if (! fg.isNull())
                last->setData(col, Qt::ForegroundRole, namedColor(fg.toString()));
            }

	    if (colRole[col][COLROLE_BACKGROUND])
            {
	      QVariant bg = pQuery.value(colRole[col][COLROLE_BACKGROUND]);
              if (! bg.isNull())
                last->setData(col, Qt::BackgroundRole, namedColor(bg.toString()));
            }

	    if (colRole[col][COLROLE_TEXTALIGNMENT])
            {
              QVariant alignment = pQuery.value(colRole[col][COLROLE_TEXTALIGNMENT]);
              if (! alignment.isNull())
                last->setData(col, Qt::TextAlignmentRole, alignment);
            }

	    if (colRole[col][COLROLE_TOOLTIP])
            {
              QVariant tooltip = pQuery.value(colRole[col][COLROLE_TOOLTIP]);
              if (! tooltip.isNull() )
                last->setData(col, Qt::ToolTipRole, tooltip);
            }

	    if (colRole[col][COLROLE_STATUSTIP])
            {
              QVariant statustip = pQuery.value(colRole[col][COLROLE_STATUSTIP]);
              if (! statustip.isNull())
                last->setData(col, Qt::StatusTipRole, statustip);
            }

	    if (colRole[col][COLROLE_FONT])
            {
	      QVariant font = pQuery.value(colRole[col][COLROLE_FONT]);
              if (! font.isNull())
                last->setData(col, Qt::FontRole, font);
            }

            // TODO: can & should we move runninginit out of the nested loops?
	    if (colRole[col][COLROLE_RUNNINGINIT])
            {
              QVariant runninginit = pQuery.value(colRole[col][COLROLE_RUNNINGINIT]);
              if (! runninginit.isNull())
                last->setData(col, RunningInitRole, runninginit);
            }

	    if (colRole[col][COLROLE_ID])
            {
              QVariant id = pQuery.value(colRole[col][COLROLE_ID]);
              if (! id.isNull())
                last->setData(col, IdRole, id);
            }

	    if (colRole[col][COLROLE_RUNNING])
            {
              last->setData(col, RunningSetRole,
                            pQuery.value(colRole[col][COLROLE_RUNNING]).toInt());
            }

	    if (colRole[col][COLROLE_TOTAL])
            {
              last->setData(col, TotalSetRole,
                              pQuery.value(colRole[col][COLROLE_TOTAL]).toInt());
            }

	    /*
	    if (colRole[col][COLROLE_KEY])
	      last->setData(col, KeyRole, pQuery.value(colRole[col][COLROLE_KEY]));
	    if (colRole[col][COLROLE_GROUPRUNNING])
	      last->setData(col, GroupRunningRole, pQuery.value(colRole[col][COLROLE_GROUPRUNNING]));
	    */
	  }

          if (allNull && indent > 0)
          {
            qWarning("%s::populate() hiding indented row because it's empty",
                     qPrintable(objectName()));
            last->setHidden(true);
          }
	} while (pQuery.next());
        populateCalculatedColumns();
        if (sortColumn() >= 0 && header()->isSortIndicatorShown())
          sortItems(sortColumn(), header()->sortIndicatorOrder());
      }
      else // assume xtreewidget columns are defined 1-to-1 with query columns
      {
	do
	{
	  if (pUseAltId)
	    last = new XTreeWidgetItem(this, last, pQuery.value(0).toInt(), pQuery.value(1).toInt(), pQuery.value(2).toString());
	  else
	    last = new XTreeWidgetItem(this, last, pQuery.value(0).toInt(), pQuery.value(1));

	  if (fieldCount > ((pUseAltId) ? 3 : 2))
	    for (int counter = ((pUseAltId) ? 3 : 2); counter < fieldCount; counter++)
	      last->setText((counter - ((pUseAltId) ? 2 : 1)), pQuery.value(counter).toString());
	} while (pQuery.next());
      }

      setId(pIndex);
      emit valid(currentItem() != 0);
    }
  }
  else
    emit valid(FALSE);

  qApp->restoreOverrideCursor();

  emit populated();
}

void XTreeWidget::addColumn(const QString & pString, int pWidth, int pAlignment, bool pVisible, const QString pEditColumn, const QString pDisplayColumn)
{
  if(!_settingsLoaded)
  {
    _settingsLoaded = true;

    QString pname;
    if(window())
      pname = window()->objectName() + "/";
    _settingsName = pname + objectName();

    // Load any previously saved information about column widths
    _forgetful = xtsettingsValue(_settingsName + "/isForgetful").toBool();
    _forgetfulOrder = xtsettingsValue(_settingsName + "/isForgetfulOrder").toBool();

    QString savedString;
    QStringList savedParts;
    QString part, key, val;
    bool b1 = false, b2 = false;
    if(!_forgetful)
    {
      savedString = xtsettingsValue(_settingsName + "/columnWidths").toString();
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
          _savedColumnWidths.insert(k, v);
      }
    }
    if(!_forgetfulOrder)
    {
      part = xtsettingsValue(_settingsName + "/sortOrder", "-1,ASC").toString();
      key = part.left(part.indexOf(" "));
      val = part.right(part.length() - part.indexOf(" ") - 1);
      b1 = false;
      int k = key.toInt(&b1);
      if(b1)
      {
        _scol = k;
        _sord = ("ASC" == val ? Qt::AscendingOrder : Qt::DescendingOrder);
      }
    }

    // Load any previously saved column hidden/visible information
    if(_x_preferences)
    {
      savedString = _x_preferences->value(_settingsName + "/columnsShown");
      savedParts = savedString.split("|", QString::SkipEmptyParts);
      for(int i = 0; i < savedParts.size(); i++)
      {
        part = savedParts.at(i);
        key = part.left(part.indexOf(","));
        val = part.right(part.length() - part.indexOf(",") - 1);
        int c = key.toInt(&b1);
        if(b1 && (val == "on" || val == "off"))
          _savedVisibleColumns.insert(c, (val == "on" ? true : false));
      }
    }
  }

  int column = columnCount();
  setColumnCount(column + 1);

  QTreeWidgetItem * hitem = headerItem();
#ifdef Q_WS_MAC       // bug 6117
  hitem->setText(column, QString(pString).replace(QRegExp("\\s+"), " "));
#else
  hitem->setText(column, pString);
#endif
  hitem->setTextAlignment(column, pAlignment);

  if (! pEditColumn.isEmpty())
  {
    QVariantMap *roles = new QVariantMap();
    roles->insert("qteditrole",    pEditColumn);
    if (! pDisplayColumn.isEmpty())
      roles->insert("qtdisplayrole", pDisplayColumn);
    _roles.insert(column, roles);
  }

  _defaultColumnWidths.insert(column, pWidth);
  if(_savedColumnWidths.contains(column))
    pWidth = _savedColumnWidths.value(column);
  if(pWidth >= 0)
  {
    header()->resizeSection(column, pWidth);
    header()->setResizeMode(column, QHeaderView::Interactive);
  }
  else
  {
    header()->setResizeMode(column, QHeaderView::Interactive);
    _stretch.append(column);
  }
  setColumnVisible(column, _savedVisibleColumns.value(column, pVisible));
  if(_scol >= 0 && column == _scol)
  {
    if(!header()->isSortIndicatorShown())
      header()->setSortIndicatorShown(true);
    sortItems(_scol, _sord);
  }
}

int XTreeWidget::column(const QString pName) const
{
  int colIdx = -1;
  for (int i = 0; i < _roles.size(); i++)
  {
    if (_roles.value(i)->value("qteditrole").toString() == pName)
    {
      colIdx = i;
      break;
    }
  }
  return colIdx;
}

XTreeWidgetItem *XTreeWidget::currentItem() const
{
  return (XTreeWidgetItem*)QTreeWidget::currentItem();
}

void XTreeWidget::hideColumn(const QString &pColumn)
{
  int colnum = column(pColumn);

  if (colnum >= 0)
    QTreeWidget::hideColumn(colnum);
}

void XTreeWidget::showColumn(const QString &pColumn)
{
  int colnum = column(pColumn);

  if (colnum >= 0)
    QTreeWidget::showColumn(colnum);
}

XTreeWidgetItem *XTreeWidget::topLevelItem(int idx) const
{
  return (XTreeWidgetItem*)QTreeWidget::topLevelItem(idx);
}

bool XTreeWidgetItem::operator<(const XTreeWidgetItem &other) const
{
  QVariant v1 = data(treeWidget()->sortColumn(), RawRole);
  QVariant v2 = other.data(other.treeWidget()->sortColumn(), RawRole);

  bool returnVal = false;
  switch (v1.type())
  {
    case QVariant::Bool:
      returnVal = (! v1.toBool() && v1 != v2);
      break;
    case QVariant::Date:
      returnVal = (v1.toDate() < v2.toDate());
      break;
    case QVariant::DateTime:
      returnVal = (v1.toDateTime() < v2.toDateTime());
      break;
    case QVariant::Double:
      returnVal = (v1.toDouble() < v2.toDouble());
      break;
    case QVariant::Int:
      returnVal = (v1.toInt() < v2.toInt());
      break;
    case QVariant::LongLong:
      returnVal = (v1.toLongLong() < v2.toLongLong());
      break;
    case QVariant::String:
      if (v1.toString().toDouble() == 0.0 && v2.toDouble() == 0.0)
        returnVal = (v1.toString() < v2.toString());
      else
        returnVal = (v1.toDouble() < v2.toDouble());
      break;
    default:            returnVal = false;
  }

  if (DEBUG)
    qDebug("returning %d for %s < %s", returnVal,
           qPrintable(v1.toString()), qPrintable(v2.toString()));
  return returnVal;
}

bool XTreeWidgetItem::operator==(const XTreeWidgetItem &other) const
{
  QVariant v1 = data(treeWidget()->sortColumn(), RawRole);
  QVariant v2 = other.data(other.treeWidget()->sortColumn(), RawRole);

  bool returnVal = false;
  switch (v1.type())
  {
    case QVariant::Bool:
      returnVal = (v1.toBool() && v1 == v2);
      break;
    case QVariant::Date:
      returnVal = (v1.toDate() == v2.toDate());
      break;
    case QVariant::DateTime:
      returnVal = (v1.toDateTime() == v2.toDateTime());
      break;
    case QVariant::Double:
      returnVal = (v1.toDouble() == v2.toDouble());
      break;
    case QVariant::Int:
      returnVal = (v1.toInt() == v2.toInt());
      break;
    case QVariant::LongLong:
      returnVal = (v1.toLongLong() == v2.toLongLong());
      break;
    case QVariant::String:
      if (v1.toString().toDouble() == 0.0 && v2.toDouble() == 0.0)
        returnVal = (v1.toString() == v2.toString());
      else
        returnVal = (v1.toDouble() == v2.toDouble());
      break;
    default:            returnVal = false;
  }

  if (DEBUG)
    qDebug("returning %d for %s == %s", returnVal,
           qPrintable(v1.toString()), qPrintable(v2.toString()));
  return returnVal;
}

/* don't need this yet
bool XTreeWidgetItem::operator>(const XTreeWidgetItem &other) const
{
  return !(this < other || this == other);
}
*/

void XTreeWidget::sortItems(int column, Qt::SortOrder order)
{
  // if old style then maintain backwards compatibility
  if (_roles.size() <= 0)
  {
    QTreeWidget::sortItems(column, order);
    return;
  }

  if (column < 0 || column >= columnCount() ||
      headerItem()->data(column, Qt::UserRole).toString() == "xtrunningrole")
    return;

  header()->setSortIndicator(column, order);

  // simple insertion sort using binary search to find the right insertion pt
  QString totalrole("totalrole");
  int itemcount = topLevelItemCount();
  XTreeWidgetItem *prev = dynamic_cast<XTreeWidgetItem*>(topLevelItem(0));
  for (int i = 1; i < itemcount; i++)
  {
    XTreeWidgetItem *item = dynamic_cast<XTreeWidgetItem*>(topLevelItem(i));
    if (! item)
    {
      qWarning("removing a non-XTreWidgetItem from an XTreeWidget");
      takeTopLevelItem(i);
      itemcount--;
      i--;
    }
    else if (item->data(0, Qt::UserRole).toString() == totalrole)
    {
      if (DEBUG)
        qDebug("sortItems() removing row %d because it's a totalrole", i);
      takeTopLevelItem(i);
      itemcount--;
      i--;
    }
    else if (*item < *prev && order == Qt::AscendingOrder)
    {
      int left   = 0;
      int right  = i;
      int middle = 0;
      XTreeWidgetItem *test = 0;
      while (left <= right)
      {
        middle = (left + right) / 2;
        test = static_cast<XTreeWidgetItem*>(topLevelItem(middle));
        if (*test == *item)
          break;
        else if (*test < *item)
        {
          if (*item < *(static_cast<XTreeWidgetItem*>(topLevelItem(middle + 1))))
            break;
          else
          left = middle + 1;
        }
        else
          right = middle - 1;
      }
      // can't call takeTopLevelItem() until after < and == are done
      if (*item < *test || *item == *test)
      {
        if (DEBUG)
          qDebug("<= so moving %d to %d", i, middle);
        takeTopLevelItem(i);
        insertTopLevelItem(middle, item);
      }
      else
      {
        if (DEBUG)
          qDebug("> so moving %d to %d", i, middle + 1);
        takeTopLevelItem(i);
        insertTopLevelItem(middle + 1, item);
      }
    }
    else if (*item == *prev)
    {
      ; // nothing to do - make the > case easier to write
    }
    else if (!(*item < *prev) && order == Qt::DescendingOrder)
    {
      int left   = 0;
      int right  = i;
      int middle = 0;
      XTreeWidgetItem *test = 0;
      while (left <= right)
      {
        middle = (left + right) / 2;
        test = static_cast<XTreeWidgetItem*>(topLevelItem(middle));
        if (*test == *item)
          break;
        else if (!(*test < *item))
        {
          if (!(*item < *(static_cast<XTreeWidgetItem*>(topLevelItem(middle + 1)))))
            break;
          else
          left = middle + 1;
        }
        else
          right = middle - 1;
      }
      // can't call takeTopLevelItem() until after < and == are done
      if (!(*item < *test) || *item == *test)
      {
        if (DEBUG)
          qDebug(">= so moving %d to %d", i, middle);
        takeTopLevelItem(i);
        insertTopLevelItem(middle, item);
      }
      else
      {
        if (DEBUG)
          qDebug("< so moving %d to %d", i, middle + 1);
        takeTopLevelItem(i);
        insertTopLevelItem(middle + 1, item);
      }
    }
    // can't reuse item because the thing in position i may have changed
    prev = static_cast<XTreeWidgetItem*>(topLevelItem(i));
  }

  populateCalculatedColumns();
  emit resorted();
}

void XTreeWidget::populateCalculatedColumns()
{
  QMap<int, QMap<int, double> > totals; // <col <totalset, subtotal> >
  QMap<int, int> scales;        // keep scale for the col, not col[totalset]
  for (int col = 0; topLevelItem(0) &&
                    col < topLevelItem(0)->columnCount(); col++)
  {
    if (headerItem()->data(col, Qt::UserRole).toString() == "xtrunningrole")
    {
      QMap<int, double> subtotals;
      // assume that RunningSetRole exists if xtrunningrole exists
      for (int row = 0; row < topLevelItemCount(); row++)
      {
        int set = topLevelItem(row)->data(col, RunningSetRole).toInt();
        if (! subtotals.contains(set))
          subtotals[set] = topLevelItem(row)->data(col, RunningInitRole).toInt();
        subtotals[set] += topLevelItem(row)->data(col, RawRole).toDouble();
        topLevelItem(row)->setData(col, Qt::DisplayRole,
                                   QLocale().toString(subtotals[set], 'f',
                                                      topLevelItem(row)->data(col, ScaleRole).toInt()));
      }
    }
    else if (headerItem()->data(col, Qt::UserRole).toString() == "xttotalrole")
    {
      QMap<int, double> totalset;
      int colscale = -99999;
      // assume that TotalSetRole exists if xttotalrole exists
      for (int row = 0; row < topLevelItemCount(); row++)
      {
        int set = topLevelItem(row)->data(col, TotalSetRole).toInt();
        if (! totalset.contains(set))
          totalset[set] = topLevelItem(row)->data(col, TotalInitRole).toInt();
        totalset[set] += topLevelItem(row)->totalForItem(col, set);
        if (topLevelItem(row)->data(col, ScaleRole).toInt() > colscale)
          colscale = topLevelItem(row)->data(col, ScaleRole).toInt();
      }
      totals.insert(col, totalset);
      scales.insert(col, colscale);
    }
  }

  // punt: for now only report values of totalset[0] for each totaled col
  // TODO: figure out how to handle multiple totalsets
  if (totals.size() > 0)
  {
    XTreeWidgetItem *last = new XTreeWidgetItem(this, -1, -1,
                            (totals.size() == 1) ? tr("Total") : tr("Totals"));
    last->setData(0, Qt::UserRole, "totalrole");
    QMapIterator<int, QMap<int, double> > it(totals);
    while (it.hasNext())
    {
      it.next();
      last->setData(it.key(), Qt::DisplayRole,
                    QLocale().toString(it.value().value(0), 'f',
                                       scales.value(it.key())));
    }
  }
}

int XTreeWidget::id() const
{
  QList<QTreeWidgetItem*> items = selectedItems();
  if(items.count() > 0)
  {
    XTreeWidgetItem * item = (XTreeWidgetItem*)items.at(0);
    return item->_id;
  }
  return -1;
}

int XTreeWidget::id(const QString p) const
{
  QList<QTreeWidgetItem*> items = selectedItems();
  if(items.count() > 0)
  {
    int id = items.at(0)->data(column(p), IdRole).toInt();
    if (DEBUG)
      qDebug("XTreeWidget::id(%s - column %d) returning %d",
             qPrintable(p), column(p), id);
    return id;
  }
  return -1;
}

int XTreeWidget::altId() const
{
  QList<QTreeWidgetItem*> items = selectedItems();
  if(items.count() > 0)
  {
    XTreeWidgetItem * item = (XTreeWidgetItem*)items.at(0);
    return item->_altId;
  }
  return -1;
}

void XTreeWidget::setId(int pId)
{
  if (pId < 0)
    return;

  for (QModelIndex i = indexFromItem(topLevelItem(0)); i.isValid(); i = indexBelow(i))
  {
    XTreeWidgetItem *item = (XTreeWidgetItem*)itemFromIndex(i);
    if(item && item->id() == pId)
    {
      selectionModel()->setCurrentIndex(i,
                                        QItemSelectionModel::ClearAndSelect |
                                        QItemSelectionModel::Rows);
      return;
    }
  }
}

QString XTreeWidget::dragString() const { return _dragString; }
void XTreeWidget::setDragString(QString pDragString)
{
  _dragString = pDragString;
}

QString XTreeWidget::altDragString() const { return _altDragString; }
void XTreeWidget::setAltDragString(QString pAltDragString)
{
  _altDragString = pAltDragString;
}

void XTreeWidget::clear()
{
  emit valid(FALSE);

  QTreeWidget::clear();
}

void XTreeWidget::sSelectionChanged()
{
  QList<QTreeWidgetItem*> items = selectedItems();
  if(items.count() > 0)
  {
    XTreeWidgetItem * item = (XTreeWidgetItem*)items.at(0);
    emit valid(true);
    emit newId(item->_id);
  }
  else
  {
    emit valid(false);
    emit newId(-1);
  }
}

void XTreeWidget::sItemSelected(QTreeWidgetItem *pSelected, int)
{
  if(pSelected)
    emit itemSelected(((XTreeWidgetItem *)pSelected)->_id);
}

void XTreeWidget::sShowMenu(const QPoint &pntThis)
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)itemAt(pntThis);
  int logicalColumn = indexAt(pntThis).column();
  if (item)
  {
    _menu->clear();
    emit populateMenu(_menu, item);
    emit populateMenu(_menu, item, logicalColumn);

    bool disableExport = FALSE;
    if(_x_preferences)
      disableExport = (_x_preferences->value("DisableExportContents")=="t");
    if(!disableExport)
    {
      if (_menu->count())
        _menu->insertSeparator();

      _menu->insertItem(tr("Copy All"),  this, SLOT(sCopyVisibleToClipboard()));
      _menu->insertItem(tr("Copy Row"),  this, SLOT(sCopyRowToClipboard()));
      _menu->insertItem(tr("Copy Cell"),  this, SLOT(sCopyCellToClipboard()));
      _menu->insertSeparator();
      _menu->insertItem(tr("Export Contents..."),  this, SLOT(sExport()));
    }

    if(_menu->count())
      _menu->popup(mapToGlobal(pntThis));
  }
}

void XTreeWidget::sShowHeaderMenu(const QPoint &pntThis)
{
  _menu->clear();

  int logicalIndex = header()->logicalIndexAt(pntThis);
  int currentSize = header()->sectionSize(logicalIndex);
// If we have a default value and the current size is not equal to that default value
// then we want to show the menu items for resetting those values back to default
  if(_defaultColumnWidths.contains(logicalIndex)
     && (!_stretch.contains(logicalIndex))
     && (_defaultColumnWidths.value(logicalIndex) != currentSize) )
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

  if(_forgetfulOrder)
    _menu->insertItem(tr("Remember Sort Order"), this, SLOT(sToggleForgetfulnessOrder()));
  else
    _menu->insertItem(tr("Do Not Remember Sort Order"), this, SLOT(sToggleForgetfulnessOrder()));

  _menu->insertSeparator();

  QTreeWidgetItem * hitem = headerItem();
  for(int i = 0; i < header()->count(); i++)
  {
    QAction * act = _menu->addAction(hitem->text(i));
    act->setCheckable(true);
    act->setChecked(!header()->isSectionHidden(i));
    act->setEnabled(!_lockedColumns.contains(i));
    QMap<QString,QVariant> m;
    m.insert("command", QVariant("toggleColumnHidden"));
    m.insert("column", QVariant(i));
    act->setData(m);
    connect(_menu, SIGNAL(triggered(QAction*)), this, SLOT(popupMenuActionTriggered(QAction*)));
  }

  if(_menu->count())
    _menu->popup(mapToGlobal(pntThis));
}

void XTreeWidget::sExport()
{
  QString path = xtsettingsValue(_settingsName + "/exportPath").toString();
  QFileInfo fi(QFileDialog::getSaveFileName(this, tr("Export Save Filename"), path,
                     tr("Text CSV (*.csv);;Text (*.txt);;ODF Text Document (*.odt);;HTML Document (*.html)")));

  if (!fi.filePath().isEmpty())
  {
    QTextDocument *doc = new QTextDocument();
    QTextDocumentWriter writer;
    if(fi.suffix().isEmpty())
      fi.setFile(fi.filePath() += ".txt");
    xtsettingsSetValue(_settingsName + "/exportPath", fi.path());
    writer.setFileName(fi.filePath());

    if (fi.suffix() == "txt")
    {
      doc->setPlainText(toTxt());
      writer.setFormat("plaintext");
    }
    else if (fi.suffix() == "csv")
    {
      doc->setPlainText(toCsv());
      writer.setFormat("plaintext");
    }
    else if (fi.suffix() == "odt")
    {
      doc->setHtml(toHtml());
      writer.setFormat("odf");
    }
    else if (fi.suffix() == "html")
    {
      doc->setHtml(toHtml());
      writer.setFormat("HTML");
    }
    writer.write(doc);
  }
}

void XTreeWidget::mousePressEvent(QMouseEvent * event)
{
  if (event->button() == Qt::LeftButton)
    dragStartPosition = event->pos();
  QTreeWidget::mousePressEvent(event);
}

void XTreeWidget::mouseMoveEvent(QMouseEvent * event)
{
  if (!(event->buttons() & Qt::LeftButton) || (_dragString.isEmpty() && _altDragString.isEmpty()))
    return;
  if ((event->pos() - dragStartPosition).manhattanLength()
        < QApplication::startDragDistance())
    return;

  QString dragDescription;
  if (_dragString.length())
    dragDescription = _dragString + QString("%1").arg(id());

  if (_altDragString.length())
  {
    if (dragDescription.length())
      dragDescription += ",";

    dragDescription += _altDragString + QString("%1").arg(altId());
  }

  QDrag *drag = new QDrag(this);
  QMimeData *mimeData = new QMimeData;

  //mimeData->setData("text/plain", dragDescription.toLatin1());
  mimeData->setText(dragDescription);
  drag->setMimeData(mimeData);

  /*Qt::DropAction dropAction =*/ drag->start(Qt::CopyAction);

  QTreeWidget::mouseMoveEvent(event);
}

void XTreeWidget::sHeaderClicked(int column)
{
  //Qt::SortOrder sortOrder = Qt::DescendingOrder;
  if(!header()->isSortIndicatorShown())
    header()->setSortIndicatorShown(true);
  sortItems(column, header()->sortIndicatorOrder());
}

void XTreeWidget::sColumnSizeChanged(int logicalIndex, int /*oldSize*/, int /*newSize*/)
{
  if(_resizingInProcess || _stretch.count() < 1)
    return;

  if(_stretch.contains(logicalIndex))
    _stretch.remove(_stretch.indexOf(logicalIndex));

  _resizingInProcess = true;

  int usedSpace = 0;
  int stretchCount = 0;

  for(int i = 0; i < header()->count(); i++)
  {
    if(logicalIndex == i || !_stretch.contains(i))
      usedSpace += header()->sectionSize(i);
    else
      stretchCount++;
  }

  int w = viewport()->width();
  if(stretchCount > 0)
  {
    int leftover = (w - usedSpace) / stretchCount;

    if(leftover < 50)
      leftover = 50;

    for(int n = 0; n < _stretch.count(); n++)
      if(_stretch.at(n) != logicalIndex)
        header()->resizeSection(_stretch.at(n), leftover);
  }

  _resizingInProcess = false;
}

void XTreeWidget::resizeEvent(QResizeEvent * e)
{
  QTreeWidget::resizeEvent(e);

  sColumnSizeChanged(-1, 0, 0);
}

void XTreeWidget::sResetWidth()
{
  int w = _defaultColumnWidths.value(_resetWhichWidth);
  if(w >= 0)
    header()->resizeSection(_resetWhichWidth, w);
  else
  {
    if(!_stretch.contains(_resetWhichWidth))
      _stretch.append(_resetWhichWidth);
    sColumnSizeChanged(-1, 0, 0);
  }
}

void XTreeWidget::sResetAllWidths()
{
  QMapIterator<int, int> it(_defaultColumnWidths);
  bool autoSections = false;
  while (it.hasNext())
  {
    it.next();

    if(it.value() >= 0)
      header()->resizeSection(it.key(), it.value());
    else
    {
      if(!_stretch.contains(it.key()))
      {
        _stretch.append(it.key());
        autoSections = true;
      }
    }
  }
  if(autoSections)
    sColumnSizeChanged(-1, 0, 0);
}

void XTreeWidget::sToggleForgetfulness()
{
  _forgetful = !_forgetful;
}

void XTreeWidget::sToggleForgetfulnessOrder()
{
  _forgetfulOrder = !_forgetfulOrder;
}

void XTreeWidget::setColumnVisible(int pColumn, bool pVisible)
{
  if(pVisible)
    header()->showSection(pColumn);
  else
    header()->hideSection(pColumn);
}

void XTreeWidget::popupMenuActionTriggered(QAction * pAction)
{
  QMap<QString, QVariant> m = pAction->data().toMap();
  QString command = m.value("command").toString();
  if("toggleColumnHidden" == command)
  {
    setColumnVisible(m.value("column").toInt(), pAction->isChecked());
  }
  //else if (some other command to handle)
}

void XTreeWidget::setColumnCount(int p)
{
  for (int i = columnCount(); i > p; i--)
    _roles.remove(i - 1);
  QTreeWidget::setColumnCount(p);
}

void XTreeWidget::setColumnLocked(int pColumn, bool pLocked)
{
  if(pLocked)
  {
    if(!_lockedColumns.contains(pColumn))
      _lockedColumns.append(pColumn);
  }
  else
    _lockedColumns.removeAll(pColumn);
}

XTreeWidgetItem *XTreeWidget::findXTreeWidgetItemWithId(const XTreeWidget *ptree, const int pid)
{
  if (pid < 0)
    return 0;

  for (int i = 0; i < ptree->topLevelItemCount(); i++)
  {
    XTreeWidgetItem *item = ptree->topLevelItem(i);
    if (item->id() == pid)
      return item;
    else
    {
      item = findXTreeWidgetItemWithId(item, pid);
      if (item)
	return item;
    }
  }

  return 0;
}

XTreeWidgetItem *XTreeWidget::findXTreeWidgetItemWithId(const XTreeWidgetItem *ptreeitem, const int pid)
{
  if (pid < 0)
    return 0;

  for (int i = 0; i < ptreeitem->childCount(); i++)
  {
    XTreeWidgetItem *item = ptreeitem->child(i);
    if (item->id() == pid)
      return item;
    else
    {
      item = findXTreeWidgetItemWithId(item, pid);
      if (item)
	return item;
    }
  }

  return 0;
}

XTreeWidgetItem::XTreeWidgetItem( XTreeWidgetItem *itm, int pId, QVariant v0,
                              QVariant v1, QVariant v2,
                              QVariant v3, QVariant v4,
                              QVariant v5, QVariant v6,
                              QVariant v7, QVariant v8,
                              QVariant v9, QVariant v10 ) :
  QTreeWidgetItem(itm)
{
  constructor(pId, -1, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
}

XTreeWidgetItem::XTreeWidgetItem( XTreeWidgetItem *itm, int pId, int pAltId, QVariant v0,
                              QVariant v1, QVariant v2,
                              QVariant v3, QVariant v4,
                              QVariant v5, QVariant v6,
                              QVariant v7, QVariant v8,
                              QVariant v9, QVariant v10 ) :
  QTreeWidgetItem(itm)
{
  constructor(pId, pAltId, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
}

XTreeWidgetItem::XTreeWidgetItem( XTreeWidget *pParent, int pId, QVariant v0,
                              QVariant v1, QVariant v2,
                              QVariant v3, QVariant v4,
                              QVariant v5, QVariant v6,
                              QVariant v7, QVariant v8,
                              QVariant v9, QVariant v10 ) :
  QTreeWidgetItem(pParent)
{
  constructor(pId, -1, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
}

XTreeWidgetItem::XTreeWidgetItem( XTreeWidget *pParent, int pId, int pAltId, QVariant v0,
                              QVariant v1, QVariant v2,
                              QVariant v3, QVariant v4,
                              QVariant v5, QVariant v6,
                              QVariant v7, QVariant v8,
                              QVariant v9, QVariant v10 ) :
  QTreeWidgetItem(pParent)
{
  constructor(pId, pAltId, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
}

XTreeWidgetItem::XTreeWidgetItem( XTreeWidget *pParent, XTreeWidgetItem *itm, int pId, QVariant v0,
                              QVariant v1, QVariant v2,
                              QVariant v3, QVariant v4,
                              QVariant v5, QVariant v6,
                              QVariant v7, QVariant v8,
                              QVariant v9, QVariant v10 ) :
  QTreeWidgetItem(pParent, itm)
{
  constructor(pId, -1, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
}

XTreeWidgetItem::XTreeWidgetItem( XTreeWidget *pParent, XTreeWidgetItem *itm, int pId, int pAltId, QVariant v0,
                              QVariant v1, QVariant v2,
                              QVariant v3, QVariant v4,
                              QVariant v5, QVariant v6,
                              QVariant v7, QVariant v8,
                              QVariant v9, QVariant v10 ) :
  QTreeWidgetItem(pParent, itm)
{
  constructor(pId, pAltId, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
}

XTreeWidgetItem::XTreeWidgetItem( XTreeWidgetItem *pParent, XTreeWidgetItem *itm, int pId, QVariant v0,
                              QVariant v1, QVariant v2,
                              QVariant v3, QVariant v4,
                              QVariant v5, QVariant v6,
                              QVariant v7, QVariant v8,
                              QVariant v9, QVariant v10 ) :
  QTreeWidgetItem(pParent, itm)
{
  constructor(pId, -1, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
}

XTreeWidgetItem::XTreeWidgetItem( XTreeWidgetItem *pParent, XTreeWidgetItem *itm, int pId, int pAltId, QVariant v0,
                              QVariant v1, QVariant v2,
                              QVariant v3, QVariant v4,
                              QVariant v5, QVariant v6,
                              QVariant v7, QVariant v8,
                              QVariant v9, QVariant v10 ) :
  QTreeWidgetItem(pParent, itm)
{
  constructor(pId, pAltId, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
}

void XTreeWidgetItem::constructor(int pId, int pAltId, QVariant v0,
                                QVariant v1, QVariant v2,
                                QVariant v3, QVariant v4,
                                QVariant v5, QVariant v6,
                                QVariant v7, QVariant v8,
                                QVariant v9, QVariant v10 )
{
  _id = pId;
  _altId  = pAltId;

  if (!v0.isNull())
    setText(0, v0);

  if (!v1.isNull())
    setText(1, v1);

  if (!v2.isNull())
    setText(2, v2);

  if (!v3.isNull())
    setText(3, v3);

  if (!v4.isNull())
    setText(4, v4);

  if (!v5.isNull())
    setText(5, v5);

  if (!v6.isNull())
    setText(6, v6);

  if (!v7.isNull())
    setText(7, v7);

  if (!v8.isNull())
    setText(8, v8);

  if (!v9.isNull())
    setText(9, v9);

  if (!v10.isNull())
    setText(10, v10);

  if(treeWidget())
  {
    QTreeWidgetItem * header = treeWidget()->headerItem();
    for(int i = 0; i < header->columnCount(); i++)
      setTextAlignment(i, header->textAlignment(i));
  }
}

int XTreeWidgetItem::id(const QString p)
{
  int id = data(((XTreeWidget*)treeWidget())->column(p), IdRole).toInt();
  if (DEBUG)
    qDebug("XTreeWidgetItem::id(%s - column %d) returning %d",
           qPrintable(p), ((XTreeWidget*)treeWidget())->column(p), id);
  return id;
}

void XTreeWidgetItem::setTextColor(const QColor &pColor)
{
  for (int cursor = 0; cursor < columnCount(); cursor++)
    QTreeWidgetItem::setTextColor(cursor, pColor);
}

void XTreeWidgetItem::setText(int pColumn, const QVariant & pVariant)
{
  QTreeWidgetItem::setText(pColumn, pVariant.toString());
}

QString XTreeWidgetItem::text(const QString &pColumn) const
{
  return text(((XTreeWidget*)treeWidget())->column(pColumn));
}

QVariant XTreeWidgetItem::rawValue(const QString pName)
{
  int colIdx = ((XTreeWidget*)treeWidget())->column(pName);
  if (colIdx < 0)
    return QVariant();
  else
    return data(colIdx, RawRole);
}

/* Calculate the total for a particular XTreeWidgetItem, including any children.
   pcol is the column for which we want the total.
   prole is the value of xttotalrole for which we want the total.
   See elsewhere for the meaning of xttotalrole values.
*/
double XTreeWidgetItem::totalForItem(const int pcol, const int pset) const
{
  double total = 0.0;

  if (pset == data(pcol, TotalSetRole).toInt())
    total += data(pcol, RawRole).toDouble();
  for (int i = 0; i < childCount(); i++)
    total += child(i)->totalForItem(pcol, pset);
  return total;
}

void XTreeWidget::sCopyRowToClipboard()
{
  QMimeData *mime = new QMimeData();
  QClipboard * clipboard = QApplication::clipboard();
  QTextDocument *doc = new QTextDocument();
  QTextCursor *cursor = new QTextCursor(doc);
  QTextTableFormat tableFormat;
  QTextTableCell cell;
  QTextCharFormat format;
  QString color;
  QString font;
  XTreeWidgetItem * item = currentItem();
  int  counter;
  int  colcnt = 0;

  if (_x_preferences->boolean("CopyListsPlainText"))
  {
    QString line = "";
    for(int counter = 0; counter < item->columnCount(); counter++)
    {
      if(!QTreeWidget::isColumnHidden(counter))
       line = line + item->text(counter) + "\t";
    }
    clipboard->setText(line);
    return;
  }

  cursor->insertTable(1, 1,tableFormat);
  if(item)
  {
    for (counter = 0; counter < item->columnCount(); counter++)
    {
      if(!QTreeWidget::isColumnHidden(counter))
      {
        colcnt++;
        if (colcnt > 1)
        {
          cursor->currentTable()->appendColumns(1);
          cursor->movePosition(QTextCursor::NextCell);
        }
        cell = cursor->currentTable()->cellAt(cursor->position());
        format = cell.format();
        color = item->data(counter,Qt::BackgroundRole).toString();
        if (!color.isEmpty())
          format.setBackground(namedColor(color));
        color = item->data(counter,Qt::ForegroundRole).toString();
        if (!color.isEmpty())
          format.setForeground(namedColor(color));
        font = item->data(counter,Qt::FontRole).toString();
        if (!font.isEmpty())
          format.setFont(QFont(font));
        cell.setFormat(format);
        cursor->insertText(item->text(counter));
      }
    }
    mime->setHtml(doc->toHtml());
    clipboard->setMimeData(mime);
  }
}

void XTreeWidget::sCopyCellToClipboard()
{
  QTextEdit text;
  QMimeData *mime = new QMimeData();
  QTreeWidgetItem* item = currentItem();
  QClipboard * clipboard = QApplication::clipboard();
  QString color;
  QString font;
  int column = currentColumn();

  if(column > -1)
  {
     color = item->data(column,Qt::BackgroundRole).toString();
     if (!color.isEmpty())
        text.setTextBackgroundColor(namedColor(color));
     color = item->data(column,Qt::ForegroundRole).toString();
     if (!color.isEmpty())
        text.setTextColor(namedColor(color));
     font = item->data(column,Qt::FontRole).toString();
     if (!font.isEmpty())
        text.setFont(QFont(font));
     text.setText(item->text(column));
     if (_x_preferences->boolean("CopyListsPlainText"))
       mime->setText(text.toPlainText());
     else
       mime->setHtml(text.toHtml());
     clipboard->setMimeData(mime) ;
  }
}

void XTreeWidget::sCopyVisibleToClipboard()
{
  QMimeData *mime = new QMimeData();
  QClipboard * clipboard = QApplication::clipboard();

  if (_x_preferences->boolean("CopyListsPlainText"))
    mime->setText(toTxt());
  else
    mime->setHtml(toHtml());
  clipboard->setMimeData(mime);
}

QString XTreeWidget::toTxt() const
{
  QString line;
  QString opText;
  int     counter;

  QTreeWidgetItem * header = headerItem();
  for (counter = 0; counter < header->columnCount(); counter++)
  {
    if(!QTreeWidget::isColumnHidden(counter))
      line = line + header->text(counter).replace("\n"," ") + "\t";
  }
  opText = line + "\n";

  XTreeWidgetItem * item = topLevelItem(0);
  if(item)
  {
    QModelIndex idx = indexFromItem(item);
    while(idx.isValid())
    {
       item = (XTreeWidgetItem*)itemFromIndex(idx);
       if(item)
       {
         line = "";
         for (counter = 0; counter < item->columnCount(); counter++)
         {
           if(!QTreeWidget::isColumnHidden(counter))
             line = line + item->text(counter) + "\t";
         }
       }
       opText = opText + line + "\n";
       idx = indexBelow(idx);
    }
  }
   return opText;
}

QString XTreeWidget::toCsv() const
{
  QString line;
  QString opText;
  int  counter;
  int  colcount = 0;

  QTreeWidgetItem * header = headerItem();
  for (counter = 0; counter < header->columnCount(); counter++)
  {
    if(!QTreeWidget::isColumnHidden(counter))
    {
      if (colcount)
        line = line + ",";
      line = line + header->text(counter).replace("\"","\"\"").replace("\n"," ");
      colcount++;
    }
  }
  opText = line + "\n";

  XTreeWidgetItem * item = topLevelItem(0);
  if(item)
  {
    QModelIndex idx = indexFromItem(item);
    while(idx.isValid())
    {
       colcount = 0;
       item = (XTreeWidgetItem*)itemFromIndex(idx);
       if(item)
       {
         line = "";
         for (counter = 0; counter < item->columnCount(); counter++)
         {
           if(!QTreeWidget::isColumnHidden(counter))
           {
             if (colcount)
               line = line + ",";
             if (item->data(counter,Qt::DisplayRole).type() == QVariant::String)
               line = line + "\"";
             line = line + item->text(counter).replace("\"","\"\"");
             if (item->data(counter,Qt::DisplayRole).type() == QVariant::String)
               line = line + "\"";
             colcount++;
           }
         }
       }
       opText = opText + line + "\n";
       idx = indexBelow(idx);
    }
  }
   return opText;
}

QString XTreeWidget::toHtml() const
{
  QTextDocument *doc = new QTextDocument();
  QTextCursor *cursor = new QTextCursor(doc);
  QTextTableFormat tableFormat;
  QTextTableCell cell;
  QTextCharFormat format;
  QString color;
  QString font;
  int  counter;
  int  colcnt = 0;

  tableFormat.setHeaderRowCount(1);

  QTreeWidgetItem * header = headerItem();
  cursor->insertTable(1, 1,tableFormat);
  for (counter = 0; counter < header->columnCount(); counter++)
  {
    if(!QTreeWidget::isColumnHidden(counter))
    {
      colcnt++;
      if (colcnt > 1)
      {
        cursor->currentTable()->appendColumns(1);
        cursor->movePosition(QTextCursor::NextCell);
      }
      cell = cursor->currentTable()->cellAt(cursor->position());
      format = cell.format();
      format.setBackground(Qt::lightGray);
      cell.setFormat(format);
      cursor->insertText(header->text(counter));
    }
  }

  XTreeWidgetItem * item = topLevelItem(0);
  if(item)
  {
    QModelIndex idx = indexFromItem(item);
    while(idx.isValid())
    {
       item = (XTreeWidgetItem*)itemFromIndex(idx);
       if(item)
       {
         cursor->currentTable()->appendRows(1);
         cursor->movePosition(QTextCursor::PreviousRow);
         cursor->movePosition(QTextCursor::NextCell);
         for (counter = 0; counter < item->columnCount(); counter++)
         {
           if(!QTreeWidget::isColumnHidden(counter))
           {
             cell = cursor->currentTable()->cellAt(cursor->position());
             format = cell.format();
             color = item->data(counter,Qt::BackgroundRole).toString();
             if (!color.isEmpty())
               format.setBackground(namedColor(color));
             color = item->data(counter,Qt::ForegroundRole).toString();
             if (!color.isEmpty())
               format.setForeground(namedColor(color));
             font = item->data(counter,Qt::FontRole).toString();
             if (!font.isEmpty())
               format.setFont(QFont(font));
             cell.setFormat(format);
             cursor->insertText(item->text(counter));
             cursor->movePosition(QTextCursor::NextCell);
           }
         }
       }
       idx = indexBelow(idx);
    }
  }
  return doc->toHtml();
}

