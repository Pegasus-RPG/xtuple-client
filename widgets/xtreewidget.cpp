/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
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
#include <QProgressBar>
#include <QPushButton>
#include <QSqlError>
#include <QSqlRecord>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextDocumentWriter>
#include <QTextEdit>
#include <QTextTable>
#include <QTextTableCell>
#include <QTextTableFormat>
#include <QtScript>
#include <QMessageBox>

#include "xtreewidgetprogress.h"
#include "xtsettings.h"
#include "xsqlquery.h"
#include "format.h"

#define DEBUG false

#define WORKERINTERVAL 0
#define WORKERROWS     500

/* make sure the colroles are kept in sync with
   QStringList knownroles in populate() below,
   both in count and order
   */
#define COLROLE_DISPLAY       0
#define COLROLE_TEXTALIGNMENT 1
#define COLROLE_BACKGROUND    2
#define COLROLE_FOREGROUND    3
#define COLROLE_TOOLTIP       4
#define COLROLE_STATUSTIP     5
#define COLROLE_FONT          6
#define COLROLE_KEY           7
#define COLROLE_RUNNING       8
#define COLROLE_RUNNINGINIT   9
#define COLROLE_GROUPRUNNING  10
#define COLROLE_TOTAL         11
#define COLROLE_NUMERIC       12
#define COLROLE_NULL          13
#define COLROLE_ID            14
// make sure COLROLE_COUNT = last COLROLE + 1
#define COLROLE_COUNT         15

#define yesStr QObject::tr("Yes")
#define noStr  QObject::tr("No")

GuiClientInterface *XTreeWidget::_guiClientInterface = 0;

static QTreeWidgetItem *searchChildren(XTreeWidgetItem *item, int pId);

// cint() and round() regarding Issue #8897
#include <cmath>

static double cint(double x)
{
  double intpart, fractpart;
  fractpart = modf(x, &intpart);

  if (fabs(fractpart) >= 0.5)
    return x>=0 ? ceil(x) : floor(x);
  else
    return x<0 ? ceil(x) : floor(x);
}

static double round(double r, int places)
{
  double off=pow(10.0,places);
  return cint(r*off)/off;
}

XTreeWidget::XTreeWidget(QWidget *pParent) :
  QTreeWidget(pParent)
{
  _resizingInProcess = false;
  _forgetful         = false;
  _forgetfulOrder    = false;
  _settingsLoaded    = false;
  _menu    = new QMenu(this);
  _menu->setObjectName("_menu");
  _savedId = false; // was -1;
  _linear  = false;
  _alwaysLinear = true;

  _colIdx     = 0;  // querycol = _colIdx[xtreecol]
  _colRole    = 0;  // querycol = _colRole[xtreecol][roleid]
  _fieldCount = 0;
  _last       = 0;
  for (int i = 0; i < ROWROLE_COUNT; i++)
    _rowRole[i] = 0;
  _progress = 0;
  _subtotals = 0;

  setUniformRowHeights(true); //#13439 speed improvement if all rows are known to be the same height
  setContextMenuPolicy(Qt::CustomContextMenu);
  setSelectionBehavior(QAbstractItemView::SelectRows);
  header()->setStretchLastSection(false);
  header()->setSectionsClickable(true);
  if (_x_preferences)
    setAlternatingRowColors(!_x_preferences->boolean("NoAlternatingRowColors"));

  connect(header(),       SIGNAL(sectionClicked(int)),                                      this, SLOT(sHeaderClicked(int)));
  connect(this,           SIGNAL(itemSelectionChanged()),                                   this, SLOT(sSelectionChanged()));
  connect(this,           SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),                this, SLOT(sItemSelected(QTreeWidgetItem *, int)));
  connect(this,           SIGNAL(customContextMenuRequested(const QPoint &)),               SLOT(sShowMenu(const QPoint &)));
  connect(header(),       SIGNAL(customContextMenuRequested(const QPoint &)),               SLOT(sShowHeaderMenu(const QPoint &)));
  connect(header(),       SIGNAL(sectionResized(int, int, int)),
          this,     SLOT(sColumnSizeChanged(int, int, int)));
  connect(this,           SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem *)),  SLOT(sCurrentItemChanged(QTreeWidgetItem*, QTreeWidgetItem *)));
  connect(this,           SIGNAL(itemChanged(QTreeWidgetItem*, int)),                       SLOT(sItemChanged(QTreeWidgetItem*, int)));
  connect(this,           SIGNAL(itemClicked(QTreeWidgetItem*, int)),                       SLOT(sItemClicked(QTreeWidgetItem*, int)));
  connect(&_workingTimer, SIGNAL(timeout()), this, SLOT(populateWorker()));

  emit valid(false);
  setColumnCount(0);

  header()->setContextMenuPolicy(Qt::CustomContextMenu);

#ifdef Q_OS_MAC
  QFont f = font();
  f.setPointSize(f.pointSize() - 2);
  setFont(f);
#endif

  // Shortcuts for xtreewidget
  QAction* menuAct = new QAction(this);
  menuAct->setShortcut(QKeySequence(tr("Ctrl+M")));
  connect(menuAct, SIGNAL(triggered()), this, SLOT(sShowMenu()));
  addAction(menuAct);

}

XTreeWidget::~XTreeWidget()
{
  qApp->restoreOverrideCursor();

  cleanupAfterPopulate();

  if (_subtotals)
  {
    for (int i = 0; i < _subtotals->size(); i++)
    {
      delete (*_subtotals)[i];
      _subtotals->replace(i, 0);
    }
    delete _subtotals;
    _subtotals = 0;
  }

  if (_x_preferences)
  {
      xtsettingsSetValue( _settingsName + "/isForgetful",       _forgetful);
      xtsettingsSetValue( _settingsName + "/isForgetfulOrder",  _forgetfulOrder);
    QString savedString;
    if (!_forgetful)
    {
      savedString = "";
      for (int i = 0; i < header()->count(); i++)
      {
        int w = -1;
        if (_defaultColumnWidths.contains(i))
          w = _defaultColumnWidths.value(i);
        if (!_stretch.contains(i) && header()->sectionSize(i) != w && !header()->isSectionHidden(i))
          savedString.append(QString::number(i) + "," + QString::number(header()->sectionSize(i)) + "|");
      }
      xtsettingsSetValue(_settingsName + "/columnWidths", savedString);
    }
    if (!_forgetfulOrder && !_sort.isEmpty())
    {
      savedString = "";
      QPair<int, Qt::SortOrder> sort;
      foreach (sort, _sort)
      {
        savedString += QString::number(sort.first) + " " +
                       (sort.second == Qt::AscendingOrder ? "ASC" : "DESC") + "|";
      }
    }
    else
      savedString = "-1,ASC";
    xtsettingsSetValue(_settingsName + "/sortOrder", savedString);
  }

  for (int i = 0; i < _roles.size(); i++)
    delete _roles.value(i);
  _roles.clear();
}

void XTreeWidget::populate(const QString &pSql, bool pUseAltId)
{
  XSqlQuery query(pSql);
  populate(query, pUseAltId);
}

void XTreeWidget::populate(const QString &pSql, int pIndex, bool pUseAltId)
{
  XSqlQuery query(pSql);
  populate(query, pIndex, pUseAltId);
}

void XTreeWidget::populate(XSqlQuery pQuery, bool pUseAltId, PopulateStyle popstyle)
{
  populate(pQuery, id(), pUseAltId, popstyle);
}

void XTreeWidget::populate(XSqlQuery pQuery, int pIndex, bool pUseAltId, PopulateStyle popstyle)
{
  XTreeWidgetPopulateParams args;
  args._workingQuery     = pQuery;
  args._workingIndex     = pIndex;
  args._workingUseAlt    = pUseAltId;
  args._workingPopstyle  = popstyle;

  pQuery.seek(-1);

  if (popstyle == Replace)
  {
    clear();
    _workingParams.clear();
  }
  _workingParams.append(args);

  _linear = _alwaysLinear;
  if (_linear)
    populateWorker();
  else if (! _workingTimer.isActive())
    _workingTimer.start(WORKERINTERVAL);
}

void XTreeWidget::populateWorker()
{
  if (_workingParams.isEmpty())
  {
    if (DEBUG) qDebug("populateWorker called when no arguments were given.");
    if (_workingTimer.isActive())
      _workingTimer.stop();
    return;
  }

  XTreeWidgetPopulateParams args = _workingParams.first();
  XSqlQuery     pQuery     = args._workingQuery;
  int           pIndex     = args._workingIndex;
  bool          pUseAltId  = args._workingUseAlt;
  //PopulateStyle popstyle   = args._workingPopstyle;

  QList<XTreeWidgetItem*> topLevelItems; //#13439

  if (_linear)
    qApp->setOverrideCursor(Qt::WaitCursor);

  if (pIndex < 0)
    pIndex = id();

  // TODO: remove this? i've only found one usage
  if (_roles.size() <= 0)  // old-style populate by column/result order
  {
    if (DEBUG)
      qDebug("%s::populate() old-style", qPrintable(objectName()));
    if (pQuery.first())
    {
      _fieldCount = pQuery.count();
      do
      {
        if (pUseAltId)
          _last = new XTreeWidgetItem(this, _last, pQuery.value(0).toInt(),
                                     pQuery.value(1).toInt(),
                                     pQuery.value(2).toString());
        else
          _last = new XTreeWidgetItem(this, _last, pQuery.value(0).toInt(),
                                     pQuery.value(1));

        if (_fieldCount > ((pUseAltId) ? 3 : 2))
          for (int col = ((pUseAltId) ? 3 : 2); col < _fieldCount; col++)
            _last->setText((col - ((pUseAltId) ? 2 : 1)),
                          pQuery.value(col).toString());
      } while (pQuery.next());
    }
  }

  /* initialize if we haven't started reading data from the query,
     taking into account that some places call xsqlquery::first() before
     xtreewidget::populate()
   */
  if (pQuery.at() == QSql::BeforeFirstRow || (pQuery.at() == 0 && ! _colIdx))
  {
    if (pQuery.first())
    {
      cleanupAfterPopulate(); // plug memory leaks if last populate() never finished

      _fieldCount = pQuery.count();
      // TODO: rewrite code to use a qmap or some other structure that
      //       doesn't require initializing a Vector or new'd array values
      _colIdx = new QVector<int>(_roles.size());

      _colRole = new QVector<int *>(_roles.size(), 0);
      for (int ref = 0; ref < _roles.size(); ++ref)
        (*_colRole)[ref] = new int[COLROLE_COUNT];

      if (! _subtotals)
      {
        _subtotals = new QList<QMap<int, double> *>();
        for (int i = 0; i < _fieldCount; i++)
          _subtotals->append(new QMap<int, double>());
      }

      QSqlRecord  currRecord = pQuery.record();

      // apply indent, hidden and delete roles to col 0 if the caller requested them
      // keep synchronized with #define ROWROLE_* above
      if (rootIsDecorated())
      {
        _rowRole[ROWROLE_INDENT] = currRecord.indexOf("xtindentrole");
        if (_rowRole[ROWROLE_INDENT] < 0)
          _rowRole[ROWROLE_INDENT] = 0;
      }
      else
        _rowRole[ROWROLE_INDENT] = 0;

      _rowRole[ROWROLE_HIDDEN] = currRecord.indexOf("xthiddenrole");
      if (_rowRole[ROWROLE_HIDDEN] < 0)
        _rowRole[ROWROLE_HIDDEN] = 0;

      _rowRole[ROWROLE_DELETED] = currRecord.indexOf("xtdeletedrole");
      if (_rowRole[ROWROLE_DELETED] < 0)
        _rowRole[ROWROLE_DELETED] = 0;

      // keep synchronized with #define COLROLE_* above
      // TODO: get rid of COLROLE_* above and replace this QStringList
      // with a map or vector of known roles and their Qt:: role or Xt
      // enum values
      QStringList knownroles;
      knownroles << "qtdisplayrole"      << "qttextalignmentrole"<<
      "qtbackgroundrole"   << "qtforegroundrole"<<
      "qttooltiprole"      << "qtstatustiprole"<<
      "qtfontrole" << "xtkeyrole"<<
      "xtrunningrole"      << "xtrunninginit"<<
      "xtgrouprunningrole" << "xttotalrole"<<
      "xtnumericrole" << "xtnullrole"<<
      "xtidrole";
      for (int wcol = 0; wcol < _roles.size(); wcol++)
      {
        QVariantMap *role = _roles.value(wcol);
        if (!role)
        {
          qWarning("XTreeWidget::populate() there is no role for column %d", wcol);
          continue;
        }
        QString colname = role->value("qteditrole").toString();
        (*_colIdx)[wcol] = currRecord.indexOf(colname);

        for (int k = 0; k < knownroles.size(); k++)
        {
          // apply Qt roles to a whole row by applying to each column
          (*_colRole)[wcol][k] = knownroles.at(k).startsWith("qt") ?
                             currRecord.indexOf(knownroles.at(k)) :
                             0;
          if ((*_colRole)[wcol][k] > 0)
          {
            role->insert(knownroles.at(k),
                          QString(knownroles.at(k)));
          }
          else
            (*_colRole)[wcol][k] = 0;

          // apply column-specific roles second to override entire row settings
          if (currRecord.indexOf(colname + "_" + knownroles.at(k)) >=0)
          {
            (*_colRole)[wcol][k] = currRecord.indexOf(colname + "_" + knownroles.at(k));
            role->insert(knownroles.at(k),
                          QString(colname + "_" + knownroles.at(k)));
            if (knownroles.at(k) == "xtrunningrole")
              headerItem()->setData(wcol, Qt::UserRole, "xtrunningrole");
            else if (knownroles.at(k) == "xttotalrole")
                          headerItem()->setData(wcol, Qt::UserRole, "xttotalrole");
          }
        }

        // Negative NUMERIC ROLE => default for column instead of column index
        // see below
        if (!(*_colRole)[wcol][COLROLE_NUMERIC] &&
                          headerItem()->data(wcol, Xt::ScaleRole).isValid())
        {
          bool  ok;
          int   tmpscale = headerItem()->data(wcol, Xt::ScaleRole).toInt(&ok);
          if (ok)
          {
            if (DEBUG)
              qDebug("setting _colRole[%d][COLROLE_NUMERIC]: %d", wcol, 0-tmpscale);
            (*_colRole)[wcol][COLROLE_NUMERIC] = 0 - tmpscale;
          }
        }
      }

      if (_rowRole[ROWROLE_INDENT])
        setIndentation( 10);
      else
        setIndentation( 0);

      if (! _linear && ! _progress)
      {
        _progress = new XTreeWidgetProgress(this);
        connect(_progress, SIGNAL(cancel()), &_workingTimer, SLOT(stop()));
      }
      if (_progress)
      {
        _progress->setValue(0);
        _progress->setMaximum(pQuery.size());
        _progress->show();
      }
    }
  }

  int defaultScale = decimalPlaces("");
  int cnt = 0;

  if (pQuery.at() >= 0) // if the query returned any rows at all
    do
    {
      ++cnt;
      if (!_linear && cnt % WORKERROWS == 0)
      {
        this->addTopLevelItems(topLevelItems); //#13439
        _progress->setValue(pQuery.at());
        return;
      }

      int id         = pQuery.value(0).toInt();
      int altId      = (pUseAltId) ? pQuery.value(1).toInt() : -1;
      int indent     = 0;
      int lastindent = 0;
      if (_rowRole[ROWROLE_INDENT])
      {
        indent = pQuery.value(_rowRole[ROWROLE_INDENT]).toInt();
        if (indent < 0)
          indent = 0;
        if (_last)
        {
          lastindent = _last->data(0, Xt::IndentRole).toInt();
          if (DEBUG)
                qDebug("getting Xt::IndentRole from %p of %d", _last, lastindent);
        }
      }
      if (DEBUG)
                qDebug("%s::populate() with id %d altId %d indent %d lastindent %d",
                qPrintable(objectName()), id, altId, indent, lastindent);

      QObject *parentItem = 0;
      XTreeWidgetItem *previousItem = _last;
      _last = new XTreeWidgetItem((XTreeWidgetItem*)0, id, altId);

      if (indent == 0)
        parentItem = this;
      else if (lastindent < indent)
        parentItem = previousItem;
      else if (lastindent == indent)
        parentItem = dynamic_cast<XTreeWidgetItem*>(previousItem->QTreeWidgetItem::parent());
      else if (lastindent > indent)
      {
        XTreeWidgetItem *prev = (XTreeWidgetItem *)(previousItem->QTreeWidgetItem::parent());
        while (prev &&
               prev->data(0, Xt::IndentRole).toInt() >= indent)
          prev = (XTreeWidgetItem *)(prev->QTreeWidgetItem::parent());
        if (prev)
          parentItem = prev;
        else
          parentItem = this;
      }
      else
        parentItem = this;

      if (_rowRole[ROWROLE_INDENT])
        _last->setData(0, Xt::IndentRole, indent);

      if (_rowRole[ROWROLE_HIDDEN])
      {
        if (DEBUG)
          qDebug("%s::populate() found xthiddenrole, value = %s",
                  qPrintable( objectName()),
                  qPrintable( pQuery.value(_rowRole[ROWROLE_HIDDEN]).toString()));
        _last->setHidden(pQuery.value(_rowRole[ROWROLE_HIDDEN]).toBool());
      }

      bool allNull = (indent > 0);
      for (int col = 0; col < _roles.size(); col++)
      {
        QVariantMap *role = _roles.value(col);
        if (!role)
        {
          qWarning("XTreeWidget::populate() there is no role for column %d", col);
          continue;
        }

        QVariant rawValue;
        if(_colIdx->at(col) >=0)  //#13439 optimization - only try to retrieve value if index is valid
          rawValue = pQuery.value(_colIdx->at(col));

        _last->setData(col, Xt::RawRole, rawValue);

        // TODO: this isn't necessary for all columns so do less often?
        int     scale        = defaultScale;
        QString numericrole  = "";
        if ((*_colRole)[col][COLROLE_NUMERIC])
        {
          // Negative NUMERIC ROLE => default for column instead of column index
          // see above
          if ((*_colRole)[col][COLROLE_NUMERIC] < 0)
            scale = 0 - (*_colRole)[col][COLROLE_NUMERIC];
          else
          {
            numericrole  = pQuery.value((*_colRole)[col][COLROLE_NUMERIC]).toString();
            scale        = decimalPlaces(numericrole);
          }
        }

        if ((*_colRole)[col][COLROLE_NUMERIC] ||
            (*_colRole)[col][COLROLE_RUNNING] ||
            (*_colRole)[col][COLROLE_TOTAL])
          _last->setData(col, Xt::ScaleRole, scale);

        /* if qtdisplayrole IS NULL then let the raw value shine through.
           this allows UNIONS to do interesting things, like put dates and
           text into the same visual column without SQL errors.
        */
        if ((*_colRole)[col][COLROLE_DISPLAY] &&
            !pQuery.value((*_colRole)[col][COLROLE_DISPLAY]).isNull())
        {
          /* this might not handle PostgreSQL NUMERICs properly
             but at least it will try to handle INTEGERs and DOUBLEs
             and it will avoid formatting sales order numbers with decimal
             and group separators
          */
          QVariant field = pQuery.value((*_colRole)[col][COLROLE_DISPLAY]);
          if (field.type() == QVariant::Int)
            _last->setData(col, Qt::DisplayRole,
                          QLocale().toString(field.toInt()));
          else if (field.type() == QVariant::Double)
            _last->setData(col, Qt::DisplayRole,
                          QLocale().toString(field.toDouble(),
                                             'f', scale));
          else
            _last->setData(col, Qt::DisplayRole, field.toString());
        }
        else if (rawValue.isNull())
        {
          _last->setData(col, Qt::DisplayRole,
                        (*_colRole)[col][COLROLE_NULL] ?
                        pQuery.value((*_colRole)[col][COLROLE_NULL]).toString() :
                        "");
        }
        else if ((*_colRole)[col][COLROLE_NUMERIC] &&
                 ((numericrole == "percent") ||
                  (numericrole == "scrap")))
        {
          _last->setData(col, Qt::DisplayRole,
                          QLocale().toString(rawValue.toDouble() * 100.0,
                                           'f', scale));
        }
        else if ((*_colRole)[col][COLROLE_NUMERIC] || rawValue.type() == QVariant::Double)
        {
          // Issue #8897
          _last->setData(col, Qt::DisplayRole,
                          QLocale().toString(round(rawValue.toDouble(), scale),
                                           'f', scale));
        }
        else if (rawValue.type() == QVariant::Bool)
        {
          _last->setData(col, Qt::DisplayRole,
                        rawValue.toBool() ? yesStr : noStr);
        }
        else
        {
          _last->setData(col, Qt::EditRole, rawValue);
        }

        if (indent)
        {
          if (!(*_colRole)[col][COLROLE_DISPLAY] ||
              ((*_colRole)[col][COLROLE_DISPLAY] &&
               pQuery.value((*_colRole)[col][COLROLE_DISPLAY]).isNull()))
            allNull &= (rawValue.isNull() || rawValue.toString().isEmpty());
          else
            allNull &= pQuery.value((*_colRole)[col][COLROLE_DISPLAY]).isNull() ||
                       pQuery.value((*_colRole)[col][COLROLE_DISPLAY]).toString().isEmpty();

          if (DEBUG)
            qDebug("%s::populate() allNull = %d at %d for rawValue %s",
                    qPrintable( objectName()), allNull, col,
                    qPrintable( rawValue.toString()));
        }

        if ((*_colRole)[col][COLROLE_FOREGROUND])
        {
          QVariant fg = pQuery.value((*_colRole)[col][COLROLE_FOREGROUND]);
          if (!fg.isNull())
            _last->setData(col, Qt::ForegroundRole, namedColor(fg.toString()));
        }

        if ((*_colRole)[col][COLROLE_BACKGROUND])
        {
          QVariant bg = pQuery.value((*_colRole)[col][COLROLE_BACKGROUND]);
          if (!bg.isNull())
            _last->setData(col, Qt::BackgroundRole, namedColor(bg.toString()));
        }

        if ((*_colRole)[col][COLROLE_TEXTALIGNMENT])
        {
          QVariant alignment = pQuery.value((*_colRole)[col][COLROLE_TEXTALIGNMENT]);
          if (!alignment.isNull())
            _last->setData(col, Qt::TextAlignmentRole, alignment);
        }
        else
          _last->setData(col, Qt::TextAlignmentRole, headerItem()->textAlignment(col));

        if ((*_colRole)[col][COLROLE_TOOLTIP])
        {
          QVariant tooltip = pQuery.value((*_colRole)[col][COLROLE_TOOLTIP]);
          if (!tooltip.isNull() )
            _last->setData(col, Qt::ToolTipRole, tooltip);
        }

        if ((*_colRole)[col][COLROLE_STATUSTIP])
        {
          QVariant statustip = pQuery.value((*_colRole)[col][COLROLE_STATUSTIP]);
          if (!statustip.isNull())
            _last->setData(col, Qt::StatusTipRole, statustip);
        }

        if ((*_colRole)[col][COLROLE_FONT])
        {
          QVariant font = pQuery.value((*_colRole)[col][COLROLE_FONT]);
          if (!font.isNull())
            _last->setData(col, Qt::FontRole, font);
        }

        if ((*_colRole)[col][COLROLE_RUNNINGINIT])
        {
          QVariant runninginit = pQuery.value((*_colRole)[col][COLROLE_RUNNINGINIT]);
          if (!runninginit.isNull())
            _last->setData(col, Xt::RunningInitRole, runninginit);
        }

        if ((*_colRole)[col][COLROLE_ID])
        {
          QVariant id = pQuery.value((*_colRole)[col][COLROLE_ID]);
          if (!id.isNull())
            _last->setData(col, Xt::IdRole, id);
        }

        if ((*_colRole)[col][COLROLE_RUNNING])
        {
          int set = pQuery.value((*_colRole)[col][COLROLE_RUNNING]).toInt();
          _last->setData(col, Xt::RunningSetRole, set);
          /* performance hack - populateCalculatedColumns will repeat this
             but only redraw if necessary. redraw is much slower than recalc. */
          if (! _subtotals->at(col)->contains(set))
          {
            if ((*_colRole)[col][COLROLE_RUNNINGINIT])
              (*_subtotals)[col]->insert(set, pQuery.value((*_colRole)[col][COLROLE_RUNNINGINIT]).toDouble());
            else
              (*_subtotals)[col]->insert(set, 0.0);
          }
          (*(*_subtotals)[col])[set] += rawValue.toDouble();
          _last->setData(col, Qt::DisplayRole,
                         QLocale().toString((*_subtotals)[col]->value(set), 'f', scale));
        }

        if ((*_colRole)[col][COLROLE_TOTAL])
        {
          _last->setData(col, Xt::TotalSetRole,
                        pQuery.value((*_colRole)[col][COLROLE_TOTAL]).toInt());
        }

        if (_rowRole[ROWROLE_DELETED])
        {
          if (DEBUG)
            qDebug("%s::populate() found xtdeleterole, value = %s",
                    qPrintable( objectName()),
                    qPrintable( pQuery.value(_rowRole[ROWROLE_DELETED]).toString()));
          if (pQuery.value(_rowRole[ROWROLE_DELETED]).toBool())
          {
            _last->setData(col,Xt::DeletedRole, QVariant(true));
            QFont font = _last->font(col);
            font.setStrikeOut(true);
            _last->setFont(col, font);
            _last->setTextColor(Qt::gray);
          }
        }
        /*
        if ((*_colRole)[col][COLROLE_KEY])
          _last->setData(col, KeyRole, pQuery.value((*_colRole)[col][COLROLE_KEY]));
        if ((*_colRole)[col][COLROLE_GROUPRUNNING])
          _last->setData(col, GroupRunningRole, pQuery.value((*_colRole)[col][COLROLE_GROUPRUNNING]));
        */
      }

      if (allNull && indent > 0)
      {
        qWarning("%s::populate() hiding indented row because it's empty",
                 qPrintable(objectName()));
        _last->setHidden(true);
      }

      if (qobject_cast<XTreeWidget*>(parentItem))
      {
        //#13439 optimization - do not add items to 'this' until the very end
        if(parentItem == this)
          topLevelItems.append(_last);
        else
          qobject_cast<XTreeWidget*>(parentItem)->addTopLevelItem(_last);
      }
      else if (qobject_cast<XTreeWidgetItem*>(parentItem))
        qobject_cast<XTreeWidgetItem*>(parentItem)->addChild(_last);

    } while (pQuery.next());

  this->addTopLevelItems(topLevelItems); //#13439

  setId(pIndex);
  emit valid(currentItem() != 0);

  // clean up. we won't reach here until the query is done, even if ! _linear
  {
    _workingTimer.stop();

    if (_workingParams.size())
      _workingParams.takeFirst();

    cleanupAfterPopulate();

    populateCalculatedColumns();
    if (!_sort.isEmpty())
      sortItems(sortColumn(), header()->sortIndicatorOrder());

    if (DEBUG)
      qDebug("%s::populateWorker() done", qPrintable(objectName()));
    emit populated();
  }

  if (_linear)
    qApp->restoreOverrideCursor();
}

void XTreeWidget::cleanupAfterPopulate()
{
  if (_progress)
    _progress->hide();

  for (unsigned int i = 0; i < sizeof(_rowRole) / sizeof(_rowRole[0]); i++)
    _rowRole[i] = 0;

  _last = 0;

  // TODO: get rid of this when the code is rewritten
  //       as per above's todo about the QVector<int*>
  if (_colRole)
  {
    for (int ref = 0; ref < _roles.size(); ++ref)
    {
      if ((*_colRole)[ref])
        delete [] (*_colRole)[ref];
    }
    delete _colRole;
    _colRole = 0;
  }

  if (_colIdx)
    delete _colIdx;
  _colIdx = 0;

  _fieldCount = 0;
}

void XTreeWidget::addColumn(const QString &pString, int pWidth, int pAlignment, bool pVisible, const QString pEditColumn, const QString pDisplayColumn, const int scale)
{
  if (!_settingsLoaded)
  {
    _settingsLoaded = true;

    QString pname;
    if (window())
    {
      pname = window()->objectName() + "/";
      if (parentWidget())
      {
        if (parentWidget()->objectName() != window()->objectName())
          pname += parentWidget()->objectName() + "/";
      }
    }
    _settingsName = pname + objectName();

    // Load any previously saved information about column widths
    _forgetful       = xtsettingsValue(_settingsName + "/isForgetful").toBool();
    _forgetfulOrder  = xtsettingsValue(_settingsName + "/isForgetfulOrder").toBool();

    QString     savedString;
    QStringList savedParts;
    QString     part, key, val;
    bool        b1 = false, b2 = false;
    if (!_forgetful)
    {
      savedString  = xtsettingsValue(_settingsName + "/columnWidths").toString();
      savedParts   = savedString.split("|", QString::SkipEmptyParts);
      for (int i = 0; i < savedParts.size(); i++)
      {
        part = savedParts.at(i);
        key  = part.left(part.indexOf(","));
        val  = part.right(part.length() - part.indexOf(",") - 1);
        b1   = false;
        b2   = false;
        int k  = key.toInt(&b1);
        int v  = val.toInt(&b2);
        if (b1 && b2)
          _savedColumnWidths.insert(k, v);
      }
    }
    if (!_forgetfulOrder)
    {
      savedString = xtsettingsValue(_settingsName + "/sortOrder", "-1,ASC").toString();
      savedParts  = savedString.split("|", QString::SkipEmptyParts);
      foreach (part, savedParts)
      {
        key  = part.left(part.indexOf(" "));
        val  = part.right(part.length() - part.indexOf(" ") - 1);
        b1   = false;
        int k = key.toInt(&b1);
        if (b1)
          _sort.append(qMakePair(k, "ASC" == val ? Qt::AscendingOrder : Qt::DescendingOrder));
      }
    }

    // Load any previously saved column hidden/visible information
    if (_x_preferences)
    {
      savedString = _x_preferences->value(_settingsName + "/columnsShown");
      if (!savedString.contains("on"))
        savedString = "";
      savedParts = savedString.split("|", QString::SkipEmptyParts);
      for (int i = 0; i < savedParts.size(); i++)
      {
        part = savedParts.at(i);
        key  = part.left(part.indexOf(","));
        val  = part.right(part.length() - part.indexOf(",") - 1);
        int c = key.toInt(&b1);
        if (b1 && (val == "on" || val == "off"))
          _savedVisibleColumns.insert(c, (val == "on" ? true : false));
      }
    }
  }

  int column = columnCount();
  setColumnCount(column + 1);

  QTreeWidgetItem *hitem = headerItem();
  hitem->setText(column, "   " + pString);
  hitem->setTextAlignment(column, pAlignment);
  hitem->setData(column, Xt::ScaleRole, scale);

  if (!pEditColumn.isEmpty())
  {
    QVariantMap *roles = new QVariantMap();
    roles->insert("qteditrole",    pEditColumn);
    if (!pDisplayColumn.isEmpty())
      roles->insert("qtdisplayrole", pDisplayColumn);

    _roles.insert(column, roles);
  }

  _defaultColumnWidths.insert(column, pWidth);
  if (_savedColumnWidths.contains(column))
    pWidth = _savedColumnWidths.value(column);
  if (pWidth >= 0)
  {
    header()->resizeSection(column, pWidth);
    header()->setSectionResizeMode(column, QHeaderView::Interactive);
  }
  else
  {
    header()->setSectionResizeMode(column, QHeaderView::Interactive);
    _stretch.append(column);
  }
  bool forgetCache = _forgetful;
  _forgetful = true;
  setColumnVisible(column, _savedVisibleColumns.value(column, pVisible));
  _forgetful = forgetCache;
  if (_sort.contains(qMakePair(column, Qt::AscendingOrder)) ||
      _sort.contains(qMakePair(column, Qt::DescendingOrder)))
  {
    if (_roles.size() <= 0 && _sort[0].first == column)
    {
      if (!header()->isSortIndicatorShown())
        header()->setSortIndicatorShown(true);

      sortItems(_sort[0].first, _sort[0].second);
    }
    if (_roles.size() > 0)
      sortItems();
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

QString XTreeWidget::column(const int pIndex) const
{
  if (pIndex >= 0 && pIndex < _roles.size())
    return _roles.value(pIndex)->value("qteditrole").toString();
  else
    return QString();
}

XTreeWidgetItem *XTreeWidget::currentItem() const
{
  return (XTreeWidgetItem *)QTreeWidget::currentItem();
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
  return (XTreeWidgetItem *)QTreeWidget::topLevelItem(idx);
}

QString XTreeWidgetItem::toString() const
{
  return QString(id());
}

bool XTreeWidgetItem::operator<(const XTreeWidgetItem &other) const
{
  bool returnVal = false;
  bool sorted    = false;

  QPair<int, Qt::SortOrder> sort;
  foreach (sort, ((XTreeWidget*)treeWidget())->sortColumnOrder())
  {
    if (sort.first < 0 || sort.first >= columnCount() ||
        treeWidget()->headerItem()->data(sort.first, Qt::UserRole).toString() == "xtrunningrole")
      break;

    QVariant v1 = data(sort.first, Xt::RawRole);
    QVariant v2 = other.data(sort.first, Xt::RawRole);

    // bugs 17968 & 32496: sort strings according to user expectations AND preserve raw role [
    bool ok1, ok2;
    (void)v1.toString().toDouble(&ok1); // we want ok1 & ok2, not the numeric value
    (void)v2.toString().toDouble(&ok2);

    if (v1.type() == QVariant::String && ! ok1 && v2.type() == QVariant::String && ! ok2)
    {
      QVariant d1 = data(sort.first,       Qt::DisplayRole);
      QVariant d2 = other.data(sort.first, Qt::DisplayRole);

      (void)d1.toString().toDouble(&ok1);
      (void)d2.toString().toDouble(&ok2);

      if (d1.type() == QVariant::String && ! ok1 && d2.type() == QVariant::String && ! ok2)
      {
        v1 = d1;
        v2 = d2;
      }
    } // ] end 17968/32496

    if (sorted || v1==v2)
      continue;

    switch (v1.type())
    {
      case QVariant::Bool:
        sorted = true;
        returnVal = !v1.toBool();
        break;

      case QVariant::Date:
        sorted = true;
        returnVal = (v1.toDate() < v2.toDate());
        break;

      case QVariant::DateTime:
        sorted = true;
        returnVal = (v1.toDateTime() < v2.toDateTime());
        break;

      case QVariant::Double:
        sorted = true;
        returnVal = (v1.toDouble() < v2.toDouble());
        break;

      case QVariant::Int:
        sorted = true;
        returnVal = (v1.toInt() < v2.toInt());
        break;

      case QVariant::LongLong:
        sorted = true;
        returnVal = (v1.toLongLong() < v2.toLongLong());
        break;

      case QVariant::String:
        sorted = true;
        if (v1.toString().toDouble() == 0.0 && v2.toDouble() == 0.0)
          returnVal = (v1.toString() < v2.toString());
        else if (v1.toString().toDouble() == 0.0 && v2.toDouble(&ok2)) //v1 is string, v2 is number
          returnVal = false; //the number should always be treated as greater than a string
        else if (v1.toDouble(&ok1) && v2.toString().toDouble() == 0.0)
          returnVal = true;
        else
          returnVal = (v1.toDouble() < v2.toDouble());
        break;

      default:
        break;
    }

    if (sort.second==Qt::DescendingOrder)
      returnVal = !returnVal;

    if (DEBUG)
      qDebug("returning %d for %s < %s", returnVal,
             qPrintable(v1.toString()), qPrintable(v2.toString()));
  }

  return returnVal;
}

bool XTreeWidgetItem::operator==(const XTreeWidgetItem &other) const
{
  bool returnVal = true;
  QPair<int, Qt::SortOrder> sort;
  foreach (sort, ((XTreeWidget*)treeWidget())->sortColumnOrder())
  {
    QVariant v1 = data(sort.first, Xt::RawRole);
    QVariant v2 = other.data(sort.first, Xt::RawRole);

    if (v1!=v2)
      returnVal = false;

    if (DEBUG)
      qDebug("returning %d for %s == %s", returnVal,
             qPrintable(v1.toString()), qPrintable(v2.toString()));
  }

  return returnVal;
}

/* don't need this yet
bool XTreeWidgetItem::operator>(const XTreeWidgetItem &other) const
{
  return !(this < other || this == other);
}
*/
void XTreeWidget::mergeSort(int low, int high){
    int mid;
    if(low<high){
        mid=(low+high)/2;
        // Split the data into two half.
        mergeSort(low, mid);
        mergeSort(mid+1, high);

        // Merge them to get sorted output.
        merge(low, high, mid);
    }
}
void XTreeWidget::merge(int low, int high, int mid){

    QList<QTreeWidgetItem *> temp;
    int i = low, j = mid + 1;//i is for left-hand,j is for right-hand
    int k = 0;//k is for the temporary array

   while(i <= mid && j <=high){
        XTreeWidgetItem *lhsItem = static_cast<XTreeWidgetItem *>(topLevelItem(i));
        XTreeWidgetItem *rhsItem = static_cast<XTreeWidgetItem *>(topLevelItem(j));

        if(*lhsItem < *rhsItem || *lhsItem == *rhsItem)
            temp.insert(k++, static_cast<XTreeWidgetItem *>(topLevelItem(i++)->clone()));
        else
            temp.insert(k++, static_cast<XTreeWidgetItem *>(topLevelItem(j++)->clone()));
    }
    //rest elements of left-half
    while(i <= mid)
        temp.insert(k++, static_cast<XTreeWidgetItem *>(topLevelItem(i++)->clone()));
    //rest elements of right-half
    while(j <= high)
        temp.insert(k++, static_cast<XTreeWidgetItem *>(topLevelItem(j++)->clone()));

    //copy the mergered temporary array to the original array
    for(k = 0, i = low; i <= high; ++i, ++k){
        QTreeWidgetItem *item =  topLevelItem(i);
        //XTreeWidgetItem *delItem = (XTreeWidgetItem*)item;
        *item = *temp.at(k);

    }
}
void XTreeWidget::sortItems(int column, Qt::SortOrder order)
{
  // if old style then maintain backwards compatibility
  if (_roles.size() <= 0)
  {
    QTreeWidget::sortItems(column, order);
    return;
  }

  for (int column=0; column<columnCount(); column++)
  {
    if (!QRegExp("[\\d ][\\d ] ").exactMatch(headerItem()->text(column).left(3)))
      headerItem()->setText(column, "   " + headerItem()->text(column));
    headerItem()->setIcon(column, QIcon());
    headerItem()->setText(column, "   " + headerItem()->text(column).mid(3));
  }

  bool sortable = false;
  int priority = 0;
  QPair<int, Qt::SortOrder> sort;
  foreach (sort, _sort)
  {
    if (sort.first < 0 || sort.first >= columnCount() ||
        headerItem()->data(sort.first, Qt::UserRole).toString() == "xtrunningrole")
      continue;
    sortable = true;
    priority++;

    if (sort.second == Qt::AscendingOrder)
      headerItem()->setIcon(sort.first, QIcon(QPixmap(":/widgets/images/arrow_down.png")));
    else
      headerItem()->setIcon(sort.first, QIcon(QPixmap(":/widgets/images/arrow_up.png")));

    if (_sort.size()>1)
      headerItem()->setText(sort.first, (priority < 10 ? " " : "") + QString::number(priority) +
                                        " " + headerItem()->text(sort.first).mid(3));
  }

  if (!sortable)
    return;

  int previd = id();

  // simple insertion sort using binary search to find the right insertion pt
  QString totalrole("totalrole");
  int     itemcount      = topLevelItemCount();
   
#ifdef USENEWSORT
  mergeSort(0,itemcount-1);
#else
  XTreeWidgetItem *prev  = dynamic_cast<XTreeWidgetItem *>(topLevelItem(0));
  for (int i = 1; i < itemcount; i++)
  {
    XTreeWidgetItem *item = dynamic_cast<XTreeWidgetItem *>(topLevelItem(i));
    if (!item)
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
    else if (*item < *prev)
    {
      int left   = 0;
      int right  = i;
      int middle = 0;
      XTreeWidgetItem *test = 0;
      while (left <= right)
      {
        middle = (left + right) / 2;
        test   = static_cast<XTreeWidgetItem *>(topLevelItem(middle));
        if (*test == *item)
          break;
        else if (*test < *item)
        {
          if (*item < *(static_cast<XTreeWidgetItem *>(topLevelItem(middle + 1))))
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
    // can't reuse item because the thing in position i may have changed
    prev = static_cast<XTreeWidgetItem *>(topLevelItem(i));
  }
#endif
  populateCalculatedColumns();

  setId(previd);
  emit resorted();
}

QList<QPair<int, Qt::SortOrder> > XTreeWidget::sortColumnOrder()
{
  return _sort;
}

void XTreeWidget::populateCalculatedColumns()
{
  QMap<int, QMap<int, double> > totals; // <col <totalset, subtotal> >
  QMap<int, int> scales;                // keep scale for the col, not col[totalset]
  for (int col = 0; topLevelItem(0) &&
       col < topLevelItem(0)->columnCount(); col++)
  {
    if (headerItem()->data(col, Qt::UserRole).toString() == "xtrunningrole")
    {
      QMap<int, double> subtotals;
      // assume that Xt::RunningSetRole exists if xtrunningrole exists
      for (int row = 0; row < topLevelItemCount(); row++)
      {
        int set = topLevelItem(row)->data(col, Xt::RunningSetRole).toInt();
        if (!subtotals.contains(set))
          subtotals[set] = topLevelItem(row)->data(col, Xt::RunningInitRole).toDouble();
        subtotals[set] += topLevelItem(row)->data(col, Xt::RawRole).toDouble();

        // setData apparently knows if the value hasn't changed
        topLevelItem(row)->setData(col, Qt::DisplayRole,
                                   QLocale().toString(subtotals[set], 'f',
                                                      topLevelItem(row)->data(col, Xt::ScaleRole).toInt()));
      }
    }
    else if (headerItem()->data(col, Qt::UserRole).toString() == "xttotalrole")
    {
      QMap<int, double> totalset;
      int colscale = -99999;
      // assume that Xt::TotalSetRole exists if xttotalrole exists
      for (int row = 0; row < topLevelItemCount(); row++)
      {
        int set = topLevelItem(row)->data(col, Xt::TotalSetRole).toInt();
        if (!totalset.contains(set))
          totalset[set] = topLevelItem(row)->data(col, Xt::TotalInitRole).toInt();
        totalset[set] += topLevelItem(row)->totalForItem(col, set);
        if (topLevelItem(row)->data(col, Xt::ScaleRole).toInt() > colscale)
          colscale = topLevelItem(row)->data(col, Xt::ScaleRole).toInt();
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
  QList<XTreeWidgetItem *> items = selectedItems();
  if (items.count() > 0)
  {
    XTreeWidgetItem *item = items.at(0);
    return item->_id;
  }
  return -1;
}

int XTreeWidget::id(const QString p) const
{
  QList<XTreeWidgetItem *> items = selectedItems();
  if (items.count() > 0)
  {
    int id = items.at(0)->data(column(p), Xt::IdRole).toInt();
    if (DEBUG)
      qDebug("XTreeWidget::id(%s - column %d) returning %d",
             qPrintable(p), column(p), id);
    return id;
  }
  return -1;
}

int XTreeWidget::altId() const
{
  QList<XTreeWidgetItem *> items = selectedItems();
  if (items.count() > 0)
  {
    XTreeWidgetItem *item = items.at(0);
    return item->_altId;
  }
  return -1;
}

/*!
  Selects a row with a matching value \a pId on the first column in the result set.
  If \a pClear is true then any previous selections are cleared.
*/
void XTreeWidget::setId(int pId, bool pClear)
{
  if (pId < 0)
    return;

  QItemSelectionModel::SelectionFlag flag;
  if (pClear)
    flag = QItemSelectionModel::ClearAndSelect;
  else
    flag = QItemSelectionModel::Select;

  XTreeWidgetItem *item  = (XTreeWidgetItem *)topLevelItem(0);
  QTreeWidgetItem *found = 0;
  while (item)
  {
    if (item && item->id() == pId)
      found = item;
    else if (item && item->childCount() > 0)
      found = searchChildren(item, pId);
    if (found)
      break;
    item = (XTreeWidgetItem *)itemBelow(item);
  }
  if (found)
  {
    scrollToItem(found);
    QModelIndex i = indexFromItem(found);
      selectionModel()->setCurrentIndex(i,
                                      flag |
                                      QItemSelectionModel::Rows);
  }
}

// This is a static local function to help searching children recursively
QTreeWidgetItem *searchChildren(XTreeWidgetItem *parent, int pId)
{
  if (!parent)
    return 0;

  XTreeWidgetItem *item  = 0;
  QTreeWidgetItem *found = 0;
  for (int i = 0; i < parent->childCount(); ++i)
  {
    item = (XTreeWidgetItem *)parent->child(i);
    if (item && item->id() == pId)
      found = item;
    else if (item && item->childCount() > 0)
      found = searchChildren(item, pId);
    if (found)
      return found;
  }
  return 0;
}

/*!
Selects a row with a matching values \a pId and \a pAltId on the first and second columns
respectively in the result set. If \a pClear is true then any previous selections are cleared.
*/
void XTreeWidget::setId(int pId, int pAltId, bool pClear)
{
  if (pId < 0)
    return;

  QItemSelectionModel::SelectionFlag flag;
  if (pClear)
    flag = QItemSelectionModel::ClearAndSelect;
  else
    flag = QItemSelectionModel::Select;

  for (QModelIndex i = indexFromItem(topLevelItem(0)); i.isValid(); i = indexBelow(i))
  {
    XTreeWidgetItem *item = (XTreeWidgetItem *)itemFromIndex(i);
    if (item && item->id() == pId && item->altId() == pAltId)
    {
      selectionModel()->setCurrentIndex(i,
                                        flag |
                                        QItemSelectionModel::Rows);
      return;
    }
  }
}

QString XTreeWidget:: dragString() const { return _dragString; }
void XTreeWidget::    setDragString(QString pDragString)
{
  _dragString = pDragString;
}

QString XTreeWidget:: altDragString() const { return _altDragString; }
void XTreeWidget::    setAltDragString(QString pAltDragString)
{
  _altDragString = pAltDragString;
}

bool XTreeWidget::populateLinear() { return _alwaysLinear; }
void XTreeWidget::setPopulateLinear(bool alwaysLinear)
{
  _alwaysLinear = alwaysLinear;
}

void XTreeWidget::clear()
{
  if (DEBUG)
    qDebug("%s::clear()", qPrintable(objectName()));
  if (! _workingTimer.isActive())
    _workingParams.clear();
  if (_subtotals)
  {
    for (int i = 0; i < _subtotals->size(); i++)
    {
      delete (*_subtotals)[i];
      _subtotals->replace(i, 0);
    }
    delete _subtotals;
    _subtotals = 0;
  }
  emit valid(false);
  _savedId = false; // was -1;

  QTreeWidget::clear();
}

void XTreeWidget::sSelectionChanged()
{
  QList<XTreeWidgetItem *> items = selectedItems();
  if (items.count() > 0)
  {
    XTreeWidgetItem *item = items.at(0);
    emit  valid(true);
    emit  newId(item->_id);
  }
  else
  {
    emit  valid(false);
    emit  newId(-1);
  }
}

void XTreeWidget::sItemSelected()
{
  QList<XTreeWidgetItem*> selected = selectedItems();
  if (selected.count())
   sItemSelected(selected.at(0), 0);
}

void XTreeWidget::sItemSelected(QTreeWidgetItem *pSelected, int)
{
  if (pSelected)
    emit itemSelected(((XTreeWidgetItem *)pSelected)->_id);
}

void XTreeWidget::sShowMenu()
{
  QList<XTreeWidgetItem*> selected = selectedItems();
  if (selected.count())
  {
    QRect rect = this->visualItemRect(selected.at(0));
    QPoint point = rect.bottomRight();
    sShowMenu(point);
  }
}

void XTreeWidget::sShowMenu(const QPoint &pntThis)
{
  XTreeWidgetItem *item  = (XTreeWidgetItem *)itemAt(pntThis);
  int logicalColumn      = indexAt(pntThis).column();
  if (item)
  {
    _menu->clear();
    if (item->data(0, Qt::UserRole).toString() != "totalrole")
    {
      emit  populateMenu(_menu, (QTreeWidgetItem *)item);
      emit  populateMenu(_menu, (QTreeWidgetItem *)item, logicalColumn);
      emit  populateMenu(_menu, item);
      emit  populateMenu(_menu, item, logicalColumn);
    }

    bool disableExport = false;
    if (_x_preferences)
      disableExport = (_x_preferences->value("DisableExportContents")=="t");
    if (!disableExport)
    {
      _menu->addSeparator();
      QMenu* copyMenu = _menu->addMenu(tr("Copy to Clipboard"));
      copyMenu->addAction(tr("All"),  this, SLOT(sCopyVisibleToClipboard()));
      copyMenu->addAction(tr("Row"),  this, SLOT(sCopyRowToClipboard()));
      copyMenu->addAction(tr("Cell"),  this, SLOT(sCopyCellToClipboard()));
      copyMenu->addAction(tr("Column"),this, SLOT(sCopyColumnToClipboard()));
      _menu->addSeparator();
      _menu->addAction(tr("Export As..."),  this, SLOT(sExport()));
    }

    if(! _menu->isEmpty())
      _menu->popup(mapToGlobal(pntThis));
  }
}

void XTreeWidget::sShowHeaderMenu(const QPoint &pntThis)
{
  _menu->clear();

  int logicalIndex = header()->logicalIndexAt(pntThis);
  int currentSize  = header()->sectionSize(logicalIndex);
// If we have a default value and the current size is not equal to that default value
// then we want to show the menu items for resetting those values back to default
  if (_defaultColumnWidths.contains(logicalIndex) &&
      (!_stretch.contains(logicalIndex)) &&
      (_defaultColumnWidths.value(logicalIndex) != currentSize) )
  {
    _resetWhichWidth = logicalIndex;
    _menu->addAction(tr("Reset this Width"), this, SLOT(sResetWidth()));
  }

  _menu->addAction(tr("Reset all Widths"), this, SLOT(sResetAllWidths()));
  _menu->addSeparator();
  if (_forgetful)
    _menu->addAction(tr("Remember Widths"), this, SLOT(sToggleForgetfulness()));
  else
    _menu->addAction(tr("Do Not Remember Widths"), this, SLOT(sToggleForgetfulness()));

  if (_forgetfulOrder)
    _menu->addAction(tr("Remember Sort Order"), this, SLOT(sToggleForgetfulnessOrder()));
  else
    _menu->addAction(tr("Do Not Remember Sort Order"), this, SLOT(sToggleForgetfulnessOrder()));

  _menu->addSeparator();

  QTreeWidgetItem *hitem = headerItem();
  for (int i = 0; i < header()->count(); i++)
  {
    QAction *act = _menu->addAction(hitem->text(i));
    act->setCheckable(true);
    act->setChecked(!header()->isSectionHidden(i));
    act->setEnabled(!_lockedColumns.contains(i));
    QMap<QString,QVariant> m;
    m.insert("command", QVariant("toggleColumnHidden"));
    m.insert("column", QVariant(i));
    act->setData(m);
    connect(_menu, SIGNAL(triggered(QAction *)), this, SLOT(popupMenuActionTriggered(QAction *)));
  }

  if(! _menu->isEmpty())
    _menu->popup(mapToGlobal(pntThis));
}

void XTreeWidget::sExport()
{
  QString   path = xtsettingsValue(_settingsName + "/exportPath").toString();
  QString selectedFilter;
  QFileInfo fi(QFileDialog::getSaveFileName(this, tr("Export Save Filename"), path,
                                            tr("Text CSV (*.csv);;Text VCF (*.vcf);;Text (*.txt);;ODF Text Document (*.odt);;HTML Document (*.html)"), &selectedFilter));
  QString defaultSuffix;
  if(selectedFilter.contains("csv"))
    defaultSuffix = ".csv";
  else if(selectedFilter.contains("vcf"))
    defaultSuffix = ".vcf";
  else if(selectedFilter.contains("odt"))
    defaultSuffix = ".odt";
  else if(selectedFilter.contains("html"))
    defaultSuffix = ".html";
  else
    defaultSuffix = ".txt";

  if (!fi.filePath().isEmpty())
  {
    QTextDocument       *doc = new QTextDocument();
    QTextDocumentWriter writer;
    if (fi.suffix().isEmpty())
      fi.setFile(fi.filePath() += defaultSuffix);
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
    else if (fi.suffix() == "vcf")
    {
      doc->setPlainText(toVcf());
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

void XTreeWidget::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
    dragStartPosition = event->pos();
  QTreeWidget::mousePressEvent(event);
}

void XTreeWidget::mouseMoveEvent(QMouseEvent *event)
{
  if (!(event->buttons() & Qt::LeftButton) || (_dragString.isEmpty() && _altDragString.isEmpty()))
    return;

  if ((event->pos() - dragStartPosition).manhattanLength()<
      QApplication::startDragDistance())
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

  QDrag     *drag      = new QDrag(this);
  QMimeData *mimeData  = new QMimeData;

  // mimeData->setData("text/plain", dragDescription.toLatin1());
  mimeData->setText(dragDescription);
  drag->setMimeData(mimeData);

  /*Qt::DropAction dropAction =*/ drag->start(Qt::CopyAction);

  QTreeWidget::mouseMoveEvent(event);
}

void XTreeWidget::sHeaderClicked(int column)
{
  if (_roles.size() <= 0)
  {
    _sort.clear();
    _sort.append(qMakePair(column, header()->sortIndicatorOrder()));

    if (!header()->isSortIndicatorShown())
      header()->setSortIndicatorShown(true);

    sortItems(column, header()->sortIndicatorOrder());
    return;
  }

  bool multisort = QGuiApplication::keyboardModifiers() == Qt::ShiftModifier;

  int sort = -1;
  for (int i=0; i<_sort.size(); i++)
    if (_sort.at(i).first==column)
      sort = i;

  if (!multisort && sort==-1)
    _sort.clear();
  if (multisort && sort!=-1)
  {
    _sort.removeAt(sort);
    sortItems();
    return;
  }

  if (sort==-1)
    _sort.append(qMakePair(column, Qt::AscendingOrder));
  else if (_sort.at(sort).second==Qt::DescendingOrder)
    _sort[sort].second = Qt::AscendingOrder;
  else
    _sort[sort].second = Qt::DescendingOrder;

  sortItems();
}

void XTreeWidget::sColumnSizeChanged(int logicalIndex, int /*oldSize*/, int /*newSize*/)
{
  if (_resizingInProcess || _stretch.count() < 1)
    return;

  if (_stretch.contains(logicalIndex))
    _stretch.remove(_stretch.indexOf(logicalIndex));

  _resizingInProcess = true;

  int usedSpace    = 0;
  int stretchCount = 0;

  for (int i = 0; i < header()->count(); i++)
  {
    if (logicalIndex == i || !_stretch.contains(i))
      usedSpace += header()->sectionSize(i);
    else
      stretchCount++;
  }

  int w = viewport()->width();
  if (stretchCount > 0)
  {
    int leftover = (w - usedSpace) / stretchCount;

    if (leftover < 50)
      leftover = 50;

    for (int n = 0; n < _stretch.count(); n++)
      if (_stretch.at(n) != logicalIndex)
        header()->resizeSection(_stretch.at(n), leftover);
  }

  _resizingInProcess = false;
}

void XTreeWidget::resizeEvent(QResizeEvent *e)
{
  QTreeWidget::resizeEvent(e);

  sColumnSizeChanged(-1, 0, 0);
}

void XTreeWidget::sResetWidth()
{
  int w = _defaultColumnWidths.value(_resetWhichWidth);
  if (w >= 0)
    header()->resizeSection(_resetWhichWidth, w);
  else
  {
    if (!_stretch.contains(_resetWhichWidth))
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

    if (it.value() >= 0)
      header()->resizeSection(it.key(), it.value());
    else
    {
      if (!_stretch.contains(it.key()))
      {
        _stretch.append(it.key());
        autoSections = true;
      }
    }
  }
  if (autoSections)
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
  if (pVisible)
    header()->showSection(pColumn);
  else
    header()->hideSection(pColumn);

  // Save changes to db
  if (!_forgetful)
  {
    QString savedString = "";
    for (int i = 0; i < header()->count(); i++)
    {
      savedString.append(QString::number(i) + "," + (header()->isSectionHidden(i) ? "off" : "on") + "|");
    }
    if (!savedString.isEmpty())
      _x_preferences->set(_settingsName + "/columnsShown", savedString);
    else
      _x_preferences->remove(_settingsName + "/columnsShown");
  }
}

void XTreeWidget::popupMenuActionTriggered(QAction *pAction)
{
  QMap<QString, QVariant> m = pAction->data().toMap();
  QString command = m.value("command").toString();
  if ("toggleColumnHidden" == command)
  {
    setColumnVisible(m.value("column").toInt(), pAction->isChecked());
  }
  // else if (some other command to handle)
}

void XTreeWidget::setColumnCount(int p)
{
  for (int i = columnCount(); i > p; i--)
    _roles.remove(i - 1);
  QTreeWidget::setColumnCount(p);
}

void XTreeWidget::setColumnLocked(int pColumn, bool pLocked)
{
  if (pLocked)
  {
    if (!_lockedColumns.contains(pColumn))
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

XTreeWidgetItem::XTreeWidgetItem( XTreeWidgetItem *itm, int pId, QVariant v0,QVariant v1, QVariant v2,QVariant v3, QVariant v4,QVariant v5, QVariant v6,QVariant v7, QVariant v8,QVariant v9, QVariant v10 ) :
  QObject(), QTreeWidgetItem(itm)
{
  constructor(pId, -1, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
}

XTreeWidgetItem::XTreeWidgetItem( XTreeWidgetItem *itm, int pId, int pAltId, QVariant v0,QVariant v1, QVariant v2,QVariant v3, QVariant v4,QVariant v5, QVariant v6,QVariant v7, QVariant v8,QVariant v9, QVariant v10 ) :
  QObject(), QTreeWidgetItem(itm)
{
  constructor(pId, pAltId, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
}

XTreeWidgetItem::XTreeWidgetItem( XTreeWidget *pParent, int pId, QVariant v0,QVariant v1, QVariant v2,QVariant v3, QVariant v4,QVariant v5, QVariant v6,QVariant v7, QVariant v8,QVariant v9, QVariant v10 ) :
  QObject(), QTreeWidgetItem(pParent)
{
  constructor(pId, -1, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
}

XTreeWidgetItem::XTreeWidgetItem( XTreeWidget *pParent, int pId, int pAltId, QVariant v0,QVariant v1, QVariant v2,QVariant v3, QVariant v4,QVariant v5, QVariant v6,QVariant v7, QVariant v8,QVariant v9, QVariant v10 ) :
  QObject(), QTreeWidgetItem(pParent)
{
  constructor(pId, pAltId, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
}

XTreeWidgetItem::XTreeWidgetItem( XTreeWidget *pParent, XTreeWidgetItem *itm, int pId, QVariant v0,QVariant v1, QVariant v2,QVariant v3, QVariant v4,QVariant v5, QVariant v6,QVariant v7, QVariant v8,QVariant v9, QVariant v10 ) :
  QObject(), QTreeWidgetItem(pParent, itm)
{
  constructor(pId, -1, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
}

XTreeWidgetItem::XTreeWidgetItem( XTreeWidget *pParent, XTreeWidgetItem *itm, int pId, int pAltId, QVariant v0,QVariant v1, QVariant v2,QVariant v3, QVariant v4,QVariant v5, QVariant v6,QVariant v7, QVariant v8,QVariant v9, QVariant v10 ) :
  QObject(), QTreeWidgetItem(pParent, itm)
{
  constructor(pId, pAltId, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
}

XTreeWidgetItem::XTreeWidgetItem( XTreeWidgetItem *pParent, XTreeWidgetItem *itm, int pId, QVariant v0,QVariant v1, QVariant v2,QVariant v3, QVariant v4,QVariant v5, QVariant v6,QVariant v7, QVariant v8,QVariant v9, QVariant v10 ) :
  QObject(), QTreeWidgetItem(pParent, itm)
{
  constructor(pId, -1, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
}

XTreeWidgetItem::XTreeWidgetItem( XTreeWidgetItem *pParent, XTreeWidgetItem *itm, int pId, int pAltId, QVariant v0,QVariant v1, QVariant v2,QVariant v3, QVariant v4,QVariant v5, QVariant v6,QVariant v7, QVariant v8,QVariant v9, QVariant v10 ) :
  QObject(), QTreeWidgetItem(pParent, itm)
{
  constructor(pId, pAltId, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
}

void XTreeWidgetItem::constructor(int pId, int pAltId, QVariant v0,QVariant v1, QVariant v2,QVariant v3, QVariant v4,QVariant v5, QVariant v6,QVariant v7, QVariant v8,QVariant v9, QVariant v10 )
{
  _id    = pId;
  _altId = pAltId;

  if (!v0.isNull())
    setText(0,  v0);

  if (!v1.isNull())
    setText(1,  v1);

  if (!v2.isNull())
    setText(2,  v2);

  if (!v3.isNull())
    setText(3,  v3);

  if (!v4.isNull())
    setText(4,  v4);

  if (!v5.isNull())
    setText(5,  v5);

  if (!v6.isNull())
    setText(6,  v6);

  if (!v7.isNull())
    setText(7,  v7);

  if (!v8.isNull())
    setText(8,  v8);

  if (!v9.isNull())
    setText(9,  v9);

  if (!v10.isNull())
    setText(10, v10);

  if (treeWidget())
  {
    QTreeWidgetItem *header = treeWidget()->headerItem();
    for (int i = 0; i < header->columnCount(); i++)
      setTextAlignment(i, header->textAlignment(i));
  }
}

int XTreeWidgetItem::id(const QString p)
{
  int id = data(((XTreeWidget *)treeWidget())->column(p), Xt::IdRole).toInt();
  if (DEBUG)
    qDebug("XTreeWidgetItem::id(%s - column %d) returning %d",
           qPrintable(p), ((XTreeWidget *)treeWidget())->column(p), id);
  return id;
}

void XTreeWidgetItem::setTextColor(const QColor &pColor)
{
  for (int cursor = 0; cursor < columnCount(); cursor++)
    QTreeWidgetItem::setTextColor(cursor, pColor);
}

void XTreeWidgetItem::setText(int pColumn, const QVariant &pVariant)
{
  QTreeWidgetItem::setText(pColumn, pVariant.toString());
}

QString XTreeWidgetItem::text(const QString &pColumn) const
{
  return text(((XTreeWidget *)treeWidget())->column(pColumn));
}

QVariant XTreeWidgetItem::rawValue(const QString pName)
{
  int colIdx = ((XTreeWidget *)treeWidget())->column(pName);
  if (colIdx < 0)
    return QVariant();
  else
    return data(colIdx, Xt::RawRole);
}

void XTreeWidgetItem::setNumber(int pColumn, const QVariant pValue, const QString pNumericRole)
{
  setData(pColumn, Xt::RawRole, pValue);
  if (!setNumericRole(pColumn, pNumericRole))
  {
    // pValue is not a valid number
    setText(pColumn, pValue);
  }
}

void XTreeWidgetItem::setDate(int pColumn, const QVariant pDate)
{
  setData(pColumn, Xt::RawRole, pDate);
  setData(pColumn, Qt::DisplayRole, formatDate(pDate.toDate()));
}

bool XTreeWidgetItem::setNumericRole(int pColIdx, const QString pRole)
{
  bool canConvert = false;

  if (pColIdx >= 0)
  {
    double value = data(pColIdx, Xt::RawRole).toDouble(&canConvert);

    if (canConvert)
    {
      setData(pColIdx, Qt::DisplayRole, QLocale().toString(value, 'f', decimalPlaces(pRole)));
    }
  }
  return canConvert;
}

/* Calculate the total for a particular XTreeWidgetItem, including any children.
   pcol is the column for which we want the total.
   prole is the value of xttotalrole for which we want the total.
   See elsewhere for the meaning of xttotalrole values.
*/
double XTreeWidgetItem::totalForItem(const int pcol, const int pset) const
{
  double total = 0.0;

  if (pset == data(pcol, Xt::TotalSetRole).toInt())
    total += data(pcol, Xt::RawRole).toDouble();
  for (int i = 0; i < childCount(); i++)
    total += child(i)->totalForItem(pcol, pset);
  return total;
}

// script exposure of xtreewidgetitem /////////////////////////////////////////

QScriptValue XTreeWidgetItemToScriptValue(QScriptEngine *engine, XTreeWidgetItem *const &item)
{
  return engine->newQObject(item);
}

void XTreeWidgetItemFromScriptValue(const QScriptValue &obj, XTreeWidgetItem * &item)
{
  item = qobject_cast<XTreeWidgetItem *>(obj.toQObject());
}

QScriptValue XTreeWidgetItemListToScriptValue(QScriptEngine *engine, QList<XTreeWidgetItem *> const &cpplist)
{
  QScriptValue scriptlist = engine->newArray(cpplist.size());
  for (int i = 0; i < cpplist.size(); i++)
    scriptlist.setProperty(i, engine->newQObject(cpplist.at(i)));
  return scriptlist;
}

void XTreeWidgetItemListFromScriptValue(const QScriptValue &scriptlist, QList<XTreeWidgetItem *> &cpplist)
{
  cpplist.clear();
  int listlen = scriptlist.property("length").toInt32();
  for (int i = 0; i < listlen; i++)
  {
    XTreeWidgetItem *tmp = qobject_cast<XTreeWidgetItem *>(scriptlist.property(i).toQObject());
    cpplist.append(tmp);
  }
}

QScriptValue constructXTreeWidgetItem(QScriptContext *context,QScriptEngine  *engine)
{
  XTreeWidget     *tree  = qscriptvalue_cast<XTreeWidget *>(context->argument(0));
  XTreeWidgetItem *obj   = 0;
  int variantidx         = 0; // index in arg list of first qvariant

  if (tree && context->argument(1).isNumber())
  {
    if (DEBUG)
      qDebug("constructXTreeWidgetItem(tree, id)");
    obj        = new XTreeWidgetItem(tree, context->argument(1).toInt32());
    variantidx = 2;
  }
  else if (qscriptvalue_cast<XTreeWidgetItem *>(context->argument(0))   &&
           context->argument(1).isNumber())
  {
    if (DEBUG)
      qDebug("constructXTreeWidgetItem(item, id)");
    obj        = new XTreeWidgetItem(qscriptvalue_cast<XTreeWidgetItem *>(context->argument(0)),
                                     context->argument(1).toInt32());
    variantidx = 2;
  }
  else if (qscriptvalue_cast<XTreeWidgetItem *>(context->argument(0))   &&
           qscriptvalue_cast<XTreeWidgetItem *>(context->argument(1)) &&
           context->argument(2).isNumber())
  {
    if (DEBUG)
      qDebug("constructXTreeWidgetItem(item, item, id)");
    obj = new XTreeWidgetItem(qscriptvalue_cast<XTreeWidgetItem *>(context->argument(0)),
                              qscriptvalue_cast<XTreeWidgetItem *>(context->argument(1)),
                              context->argument(2).toInt32());
    variantidx = 3;
  }
  else if (tree  &&
                              qscriptvalue_cast<XTreeWidgetItem *>(context->argument(1)) &&
           context->argument(2).isNumber())
  {
    if (DEBUG)
      qDebug("constructXTreeWidgetItem(tree, item, id)");
    obj = new XTreeWidgetItem(tree,
                              qscriptvalue_cast<XTreeWidgetItem *>(context->argument(1)),
                              context->argument(2).toInt32());
    variantidx = 3;
  }

  if (obj)
  {
    if (context->argument(variantidx).isNumber())
    {
      if (DEBUG)
        qDebug("constructXTreeWidgetItem found altId");
      obj->setAltId(context->argument(variantidx).toInt32());
      variantidx++;
    }

    if (DEBUG)
        qDebug("constructXTreeWidgetItem variantidx %d with %d total args",
             variantidx, context->argumentCount());
    QTreeWidgetItem *header = tree ? tree->headerItem() : 0;
    for (int i = 0; variantidx < context->argumentCount(); variantidx++, i++)
    {
      QVariant var = context->argument(variantidx).toVariant();
      if (DEBUG)
        qDebug("constructXTreeWidgetItem variant %d: type %d, value %s",
               i, var.type(), qPrintable(var.toString()));
      obj->setData(i, Xt::RawRole, var);
      if (var.type() == QVariant::Int)
        obj->setData(i, Qt::DisplayRole, QLocale().toString(var.toInt()));
      else if (var.type() == QVariant::Double)
      {
        int scale = header ? header->data(i, Xt::ScaleRole).toInt() :
                    decimalPlaces("unknown");
        if (DEBUG)
          qDebug("constructXTreeWidgetItem header %p Xt::ScaleRole %d(%d) result %d",
                 header, header ? header->data(i, Xt::ScaleRole).toInt() : -1,
                 decimalPlaces("unknown"), scale);

        obj->setData(i, Qt::DisplayRole,
                     QLocale().toString(var.toDouble(), 'f', scale));
      }
      else if (var.type() == QVariant::Bool)
        obj->setData(i, Qt::DisplayRole, var.toBool() ? yesStr : noStr);
      else
        obj->setData(i, Qt::DisplayRole, var);
    }
  }

  return engine->toScriptValue(obj);
}

void setupXTreeWidgetItem(QScriptEngine *engine)
{
  if (! engine->globalObject().property("XTreeWidgetItem").isFunction())
  {
    qScriptRegisterMetaType(engine, XTreeWidgetItemToScriptValue,     XTreeWidgetItemFromScriptValue);
    qScriptRegisterMetaType(engine, XTreeWidgetItemListToScriptValue, XTreeWidgetItemListFromScriptValue);
    QScriptValue ctor = engine->newFunction(constructXTreeWidgetItem);
    QScriptValue meta = engine->newQMetaObject(&XTreeWidgetItem::staticMetaObject, ctor);

    engine->globalObject().setProperty("XTreeWidgetItem", meta,
                                       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  }
}

// xtreewidget ////////////////////////////////////////////////////////////////

QScriptValue XTreeWidgetToScriptValue(QScriptEngine *engine, XTreeWidget *const &item)
{
  return engine->newQObject(item);
}

void XTreeWidgetFromScriptValue(const QScriptValue &obj, XTreeWidget * &item)
{
  item = qobject_cast<XTreeWidget *>(obj.toQObject());
}

void setupXTreeWidget(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, XTreeWidgetToScriptValue, XTreeWidgetFromScriptValue);

  if (! engine->globalObject().property("XTreeWidget").isObject())
  {
    QScriptValue::PropertyFlags ro = QScriptValue::ReadOnly | QScriptValue::Undeletable;
    QScriptValue ctor = engine->newObject(); //engine->newFunction(scriptconstructor);
    QScriptValue meta = engine->newQMetaObject(&XTreeWidget::staticMetaObject, ctor);

    engine->globalObject().setProperty("XTreeWidget", meta, ro);

    // these aren't in an enum so need to be exposed explicitly
    meta.setProperty("itemColumn",     QScriptValue(engine, _itemColumn),     ro);
    meta.setProperty("whsColumn",      QScriptValue(engine, _whsColumn),      ro);
    meta.setProperty("userColumn",     QScriptValue(engine, _userColumn),     ro);
    meta.setProperty("dateColumn",     QScriptValue(engine, _dateColumn),     ro);
    meta.setProperty("timeDateColumn", QScriptValue(engine, _timeDateColumn), ro);
    meta.setProperty("timeColumn",     QScriptValue(engine, _timeColumn),     ro);
    meta.setProperty("qtyColumn",      QScriptValue(engine, _qtyColumn),      ro);
    meta.setProperty("priceColumn",    QScriptValue(engine, _priceColumn),    ro);
    meta.setProperty("moneyColumn",    QScriptValue(engine, _moneyColumn),    ro);
    meta.setProperty("bigMoneyColumn", QScriptValue(engine, _bigMoneyColumn), ro);
    meta.setProperty("costColumn",     QScriptValue(engine, _costColumn),     ro);
    meta.setProperty("prcntColumn",    QScriptValue(engine, _prcntColumn),    ro);
    meta.setProperty("transColumn",    QScriptValue(engine, _transColumn),    ro);
    meta.setProperty("uomColumn",      QScriptValue(engine, _uomColumn),      ro);
    meta.setProperty("orderColumn",    QScriptValue(engine, _orderColumn),    ro);
    meta.setProperty("statusColumn",   QScriptValue(engine, _statusColumn),   ro);
    meta.setProperty("seqColumn",      QScriptValue(engine, _seqColumn),      ro);
    meta.setProperty("ynColumn",       QScriptValue(engine, _ynColumn),       ro);
    meta.setProperty("docTypeColumn",  QScriptValue(engine, _docTypeColumn),  ro);
    meta.setProperty("currencyColumn", QScriptValue(engine, _currencyColumn), ro);
  }
}

void XTreeWidget::sCopyRowToClipboard()
{
  QMimeData         *mime      = new QMimeData();
  QClipboard        *clipboard = QApplication::clipboard();
  QTextDocument     *doc       = new QTextDocument();
  QTextCursor       *cursor    = new QTextCursor(doc);
  QTextTableFormat  tableFormat;
  QTextTableCell    cell;
  QTextCharFormat   format;
  QString color;
  QString font;
  XTreeWidgetItem *item  = currentItem();
  int     counter;
  int     colcnt         = 0;

  if (_x_preferences->boolean("CopyListsPlainText"))
  {
    QString line = "";
    for (int counter = 0; counter < item->columnCount(); counter++)
    {
      if (!QTreeWidget::isColumnHidden(counter))
        line = line + item->text(counter) + "\t";
    }
    clipboard->setText(line);
    return;
  }

  cursor->insertTable(1, 1,tableFormat);
  if (item)
  {
    for (counter = 0; counter < item->columnCount(); counter++)
    {
      if (!QTreeWidget::isColumnHidden(counter))
      {
        colcnt++;
        if (colcnt > 1)
        {
          cursor->currentTable()->appendColumns(1);
          cursor->movePosition(QTextCursor::NextCell);
        }
        cell   = cursor->currentTable()->cellAt(cursor->position());
        format = cell.format();
        if (item->data(counter, Qt::BackgroundRole).isValid())
          format.setBackground(item->data(counter, Qt::BackgroundRole).value<QColor>());
        if (item->data(counter, Qt::ForegroundRole).isValid())
          format.setForeground(item->data(counter, Qt::ForegroundRole).value<QColor>());

        if (item->data(counter,Qt::FontRole).isValid())
        {
          font = item->data(counter,Qt::FontRole).toString();
          if (!font.isEmpty())
            format.setFont(QFont(font));
        }

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
  QTextEdit       text;
  QMimeData       *mime      = new QMimeData();
  QTreeWidgetItem *item      = currentItem();
  QClipboard      *clipboard = QApplication::clipboard();
  QString         color;
  QString         font;
  int column = currentColumn();

  if (column > -1)
  {
    if (item->data(column, Qt::BackgroundRole).isValid())
      text.setTextBackgroundColor(item->data(column, Qt::BackgroundRole).value<QColor>());
    if (item->data(column, Qt::ForegroundRole).isValid())
      text.setTextColor(item->data(column, Qt::ForegroundRole).value<QColor>());

    if (item->data(column,Qt::FontRole).isValid())
    {
      font = item->data(column,Qt::FontRole).toString();
      if (!font.isEmpty())
        text.setFont(QFont(font));
    }

    text.setText(item->text(column));
    if (_x_preferences->boolean("CopyListsPlainText"))
      mime->setText(text.toPlainText());
    else
      mime->setHtml(text.toHtml());
    clipboard->setMimeData(mime);
  }
}

void XTreeWidget::sCopyVisibleToClipboard()
{
  QMimeData   *mime      = new QMimeData();
  QClipboard  *clipboard = QApplication::clipboard();

  if (_x_preferences->boolean("CopyListsPlainText"))
    mime->setText(toTxt());
  else
    mime->setHtml(toHtml());
  clipboard->setMimeData(mime);
}

void XTreeWidget::sCopyColumnToClipboard()
{
  QTextEdit       text;
  QMimeData       *mime      = new QMimeData();
  QClipboard      *clipboard = QApplication::clipboard();
  QString opText = "";
  XTreeWidgetItem *item = topLevelItem(0);
  if (item)
  {
    for (QModelIndex idx = indexFromItem(item); idx.isValid(); idx = indexBelow(idx))
    {
      item = (XTreeWidgetItem *)itemFromIndex(idx);
      if (item)
        opText = opText + item->text(currentColumn()) + "\t\r\n";
    }
  }
  text.setText(opText);
  mime->setText(text.toPlainText());
  mime->setHtml(text.toHtml());
  clipboard->setMimeData(mime);
}

void XTreeWidget::sSearch(const QString &pTarget)
{
  clearSelection();
  int i;
  for (i = 0; i < topLevelItemCount(); i++)
  {
    // Currently this only looks at the first column
    if (topLevelItem(i)->text(0).contains(pTarget, Qt::CaseInsensitive))
      break;
  }

  if (i < topLevelItemCount())
  {
    setCurrentItem(topLevelItem(i));
    scrollToItem(topLevelItem(i));
  }
}

QString XTreeWidget::toTxt() const
{
  QString line;
  QString opText;
  int     counter;

  QTreeWidgetItem *header = headerItem();
  for (counter = 0; counter < header->columnCount(); counter++)
  {
    if (!QTreeWidget::isColumnHidden(counter))
      line = line + header->text(counter).replace("\r\n"," ") + "\t";
  }
  opText = line + "\r\n";

  XTreeWidgetItem *item = topLevelItem(0);
  if (item)
  {
    for (QModelIndex idx = indexFromItem(item); idx.isValid(); idx = indexBelow(idx))
    {
      item = (XTreeWidgetItem *)itemFromIndex(idx);
      if (item)
      {
        line = "";
        for (counter = 0; counter < item->columnCount(); counter++)
        {
          if (!QTreeWidget::isColumnHidden(counter))
            line = line + item->text(counter) + "\t";
        }
      }
      opText = opText + line + "\r\n";
    }
  }
  return opText;
}

QString XTreeWidget::toCsv() const
{
  QString line;
  QString opText;
  int     counter;
  int     colcount         = 0;

  QTreeWidgetItem *header  = headerItem();
  for (counter = 0; counter < header->columnCount(); counter++)
  {
    if (!QTreeWidget::isColumnHidden(counter))
    {
      if (colcount)
        line = line + ",";
      line = line + header->text(counter).replace("\"","\"\"").replace("\r\n"," ").replace("\n"," ");
      colcount++;
    }
  }
  opText = line + "\r\n";

  XTreeWidgetItem *item = topLevelItem(0);
  if (item)
  {
    for (QModelIndex idx = indexFromItem(item); idx.isValid(); idx = indexBelow(idx))
    {
      colcount = 0;
      item     = (XTreeWidgetItem *)itemFromIndex(idx);
      if (item)
      {
        line = "";
        for (counter = 0; counter < item->columnCount(); counter++)
        {
          if (!QTreeWidget::isColumnHidden(counter))
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
      opText = opText + line + "\r\n";
    }
  }
  return opText;
}

QString XTreeWidget::toVcf() const
{
  XSqlQuery qry;
  qry.prepare("SELECT * FROM cntct WHERE (cntct_id=:cntct_id);");
  qry.bindValue(":cntct_id", this->selectedItems().at(0)->id());
  qry.exec();
  if (qry.first()) {
    QString name;
    QString fullName;
    QString first = qry.value("cntct_first_name").toString();
    QString middle = qry.value("cntct_middle").toString();
    QString last = qry.value("cntct_last_name").toString();
    if (!last.isEmpty()) {
      name = last;
      fullName = last;
    }
    if (!middle.isEmpty()) {
      name = name + ";" + middle;
      fullName = middle + " " + fullName;
    }
    if (!first.isEmpty()) {
      name = name + ";" + first;
      fullName = first + " " + fullName;
    }
    QString begin = "VCARD";
    QString version = "3.0";
    QString org = "";
    QString title = qry.value("cntct_title").toString();
    QString photo = "";
    QString phoneWork = qry.value("cntct_phone").toString();
    QString phoneHome = qry.value("cntct_phone2").toString();
    QString addressId = qry.value("cntct_addr_id").toString();
    XSqlQuery qry2;
    qry2.prepare("SELECT * FROM addr WHERE (addr_id=:addr_id);");
    qry2.bindValue(":addr_id", addressId);
    qry2.exec();
    QStringList address;
    QString addressWork= "";
    QString labelWork = "";
    if (qry2.first()) {
      /*
      * this is tricky, because sometimes (in the test database at least)
      * our addr_line1 is the company name, and sometimes it's really the
      * first line of the actual address.  For the former, we can use
      * the addr_line1 as the organization name.
      */
      if(qry2.value("addr_line1").toString().at(0).isDigit())
        address.append(qry2.value("addr_line1").toString());
      else
        org = qry2.value("addr_line1").toString();
      address.append(qry2.value("addr_line2").toString());
      address.append(qry2.value("addr_line3").toString());
      address.append(qry2.value("addr_city").toString());
      address.append(qry2.value("addr_state").toString());
      address.append(qry2.value("addr_postalcode").toString());
      address.append(qry2.value("addr_country").toString());
    }
    //for address, set address with semicolon delimiters
    //for label, set address with ESCAPED newline delimiters
    for (int i = 0; i < address.length(); i++) {
      if(!address.at(i).isEmpty()) {
        addressWork = addressWork + address.at(i) + ";";
        labelWork = labelWork + address.at(i) + "\\n";
      }
    }
    QString addressHome = "";
    QString labelHome = "";
    QString email = qry.value("cntct_email").toString();
    QDate date = QDate::currentDate();
    QString revision = date.toString(Qt::ISODate);
    QString end = "VCARD";

    QString stringToSave;

    stringToSave.append("BEGIN:" + begin + "\n");
    stringToSave.append("VERSION:" + version + "\n");
    stringToSave.append("N:" + name + "\n");
    stringToSave.append("FN:" + fullName + "\n");
    if (!org.isEmpty())
      stringToSave.append("ORG:" + org + "\n");
    if (!title.isEmpty())
      stringToSave.append("TITLE:" + title + "\n");
    if (!phoneWork.isEmpty())
      stringToSave.append("TEL;TYPE=WORK,VOICE:" + phoneWork + "\n");
    if (!phoneHome.isEmpty())
      stringToSave.append("TEL;TYPE=HOME,VOICE:" + phoneHome + "\n");
    if (!addressWork.isEmpty())
      stringToSave.append("ADR;TYPE=WORK:;;" + addressWork + "\n");
    if (!labelWork.isEmpty())
      stringToSave.append("LABEL;TYPE=WORK:;;" + labelWork + "\n");
    if (!email.isEmpty())
      stringToSave.append("EMAIL;TYPE=PREF,INTERNET:" + email + "\n");
    stringToSave.append("REV:" + revision + "\n");
    stringToSave.append("END:" + end + "\n");

    return stringToSave;
  }
  else
    return "failed to select contact for export";
}

QString XTreeWidget::toHtml() const
{
  QTextDocument     *doc     = new QTextDocument();
  QTextCursor       *cursor  = new QTextCursor(doc);
  QTextTableFormat  tableFormat;
  QTextTableCell    cell;
  QTextCharFormat   format;
  QString font;
  int     colcnt = 0;
  int     rowcnt = 0;
  int     row = 0;
  double  limit = 0;
  qlonglong maxDataCount = 0;
  qlonglong dataCount = 0;

  if (_x_preferences)
  {
    limit = _x_preferences->value("XTreeWidgetDataLimit").toDouble();
    if (limit > 0)
      maxDataCount = (qlonglong)(limit * 1e9);
  }

  tableFormat.setHeaderRowCount(1);

  QTreeWidgetItem *header = headerItem();
  for (int i = 0; i < header->columnCount(); i++)
    if (!QTreeWidget::isColumnHidden(i))
      colcnt++;

  XTreeWidgetItem *item = topLevelItem(0);
  if (item)
  {
    for (QModelIndex idx = indexFromItem(item); idx.isValid(); idx = indexBelow(idx))
    {
      item = (XTreeWidgetItem *)itemFromIndex(idx);
      if (item)
        rowcnt++;
    }
  }

  cursor->insertTable(rowcnt + 1, colcnt, tableFormat);

  for (int counter = 0; counter < header->columnCount(); counter++)
  {
    if (!QTreeWidget::isColumnHidden(counter))
    {
      cell   = cursor->currentTable()->cellAt(cursor->position());
      format = cell.format();
      format.setBackground(Qt::lightGray);
      cell.setFormat(format);
      cursor->insertText(header->text(counter));
      cursor->movePosition(QTextCursor::NextCell);

      dataCount += (qlonglong)(header->text(counter).size());
    }
  }

  item = topLevelItem(0);
  if (item)
  {
    for (QModelIndex idx = indexFromItem(item); idx.isValid() && (maxDataCount <= 0 || dataCount < maxDataCount); idx = indexBelow(idx), row++)
    {
      item = (XTreeWidgetItem *)itemFromIndex(idx);
      if (item)
      {
        for (int counter = 0; counter < item->columnCount(); counter++)
        {
          if (!QTreeWidget::isColumnHidden(counter))
          {
            cell   = cursor->currentTable()->cellAt(cursor->position());
            format = cell.format();
            if (item->data(counter, Qt::BackgroundRole).isValid())
              format.setBackground(item->data(counter, Qt::BackgroundRole).value<QColor>());
            if (item->data(counter, Qt::ForegroundRole).isValid())
              format.setForeground(item->data(counter, Qt::ForegroundRole).value<QColor>());

            if (item->data(counter,Qt::FontRole).isValid())
            {
              font = item->data(counter,Qt::FontRole).toString();
              if (!font.isEmpty())
                format.setFont(QFont(font));
            }

            cell.setFormat(format);
            cursor->insertText(item->text(counter));
            cursor->movePosition(QTextCursor::NextCell);

            dataCount += (qlonglong)(item->text(counter).size());
          }
        }
      }
    }

    if (maxDataCount > 0 && dataCount > maxDataCount)
    {
      QString overflowMsg = tr("Maximum data limit was encountered.  Only %1 of %2 rows could be processed.");
      QMessageBox::warning(NULL, tr("Data Limit Reached"), overflowMsg.arg(row).arg(rowcnt));
    }
  }
  return doc->toHtml();
}

QList<XTreeWidgetItem *> XTreeWidget::selectedItems() const
{
  QList<XTreeWidgetItem *>  xlist;

  foreach (QTreeWidgetItem *qitem, QTreeWidget::selectedItems())
  {
    if (dynamic_cast<XTreeWidgetItem *>(qitem))
      xlist.append(dynamic_cast<XTreeWidgetItem *>(qitem));
  }
  return xlist;
}

void XTreeWidget::addTopLevelItems(const QList<XTreeWidgetItem *> &items)
{
  QList<QTreeWidgetItem *> qlist = QTreeWidget::selectedItems();

  for (int i = 0; i < items.size(); i++)
    qlist.append(items.at(i));
  QTreeWidget::addTopLevelItems(qlist);
}

XTreeWidgetItem *XTreeWidget::itemAbove(const XTreeWidgetItem *item) const
{
  return dynamic_cast<XTreeWidgetItem *>(QTreeWidget::itemAbove(item));
}

QList<XTreeWidgetItem *> XTreeWidget::findItems(const QString &text, Qt::MatchFlags flags, int column, int role) const
{
  QModelIndexList indexes = model()->match(model()->index(0, column, QModelIndex()),
                                           role, text, -1, flags);

  QList<XTreeWidgetItem *>  *xlist = new QList<XTreeWidgetItem *>();
  for (int i = 0; i < indexes.size(); ++i)
  {
    if (dynamic_cast<XTreeWidgetItem *>(itemFromIndex(indexes.at(i))))
      xlist->append(dynamic_cast<XTreeWidgetItem *>(itemFromIndex(indexes.at(i))));
  }

  return *xlist;
}

void XTreeWidget::insertTopLevelItems(int index, const QList<XTreeWidgetItem *> &items)
{
  QList<QTreeWidgetItem *> qlist = QTreeWidget::selectedItems();

  for (int i = 0; i < items.size(); i++)
    qlist.append(items.at(i));

  QTreeWidget::insertTopLevelItems(index, qlist);
}

XTreeWidgetItem *XTreeWidget::invisibleRootItem() const
{
  return dynamic_cast<XTreeWidgetItem *>(QTreeWidget::invisibleRootItem());
}

void XTreeWidget::sCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
  if (dynamic_cast<XTreeWidgetItem *>(current) &&
      (dynamic_cast<XTreeWidgetItem *>(previous) || previous == 0))

    emit currentItemChanged(dynamic_cast<XTreeWidgetItem *>(current),
                            dynamic_cast<XTreeWidgetItem *>(previous));
}

void XTreeWidget::sItemActivated(QTreeWidgetItem *item, int column)
{
  if (dynamic_cast<XTreeWidgetItem *>(item))
    emit itemActivated(dynamic_cast<XTreeWidgetItem *>(item), column);
}

void XTreeWidget::sItemChanged(QTreeWidgetItem *item, int column)
{
  if (dynamic_cast<XTreeWidgetItem *>(item))
    emit itemChanged(dynamic_cast<XTreeWidgetItem *>(item), column);
}

void XTreeWidget::sItemClicked(QTreeWidgetItem *item, int column)
{
  if (dynamic_cast<XTreeWidgetItem *>(item))
    emit itemClicked(dynamic_cast<XTreeWidgetItem *>(item), column);
}

void XTreeWidget::sItemCollapsed(QTreeWidgetItem *item)
{
  if (dynamic_cast<XTreeWidgetItem *>(item))
    emit itemCollapsed(dynamic_cast<XTreeWidgetItem *>(item));
}

void XTreeWidget::sItemDoubleClicked(QTreeWidgetItem *item, int column)
{
  if (dynamic_cast<XTreeWidgetItem *>(item))
    emit itemDoubleClicked(dynamic_cast<XTreeWidgetItem *>(item), column);
}

void XTreeWidget::sItemEntered(QTreeWidgetItem *item, int column)
{
  if (dynamic_cast<XTreeWidgetItem *>(item))
    emit itemEntered(dynamic_cast<XTreeWidgetItem *>(item), column);
}

void XTreeWidget::sItemExpanded(QTreeWidgetItem *item)
{
  if (dynamic_cast<XTreeWidgetItem *>(item))
    emit itemExpanded(dynamic_cast<XTreeWidgetItem *>(item));
}

void XTreeWidget::sItemPressed(QTreeWidgetItem *item, int column)
{
  if (dynamic_cast<XTreeWidgetItem *>(item))
    emit itemPressed(dynamic_cast<XTreeWidgetItem *>(item), column);
}

/*!
  Returns the Raw Value of the first selected row on \a colname
*/
QVariant XTreeWidget::rawValue(const QString colname) const
{
    QList<XTreeWidgetItem *> items = selectedItems();
    if (items.count() > 0)
    {
      XTreeWidgetItem *item = items.at(0);
      return item->rawValue(colname);
    }
    return QVariant();
}

void XTreeWidget::keyPressEvent(QKeyEvent* e)
{
  if (e->key()==Qt::Key_Enter)
  {
    bool hasDefault = false;
    QList<QPushButton*> buttons = window()->findChildren<QPushButton*>();
    foreach (QPushButton* button, buttons)
      if (button->isDefault())
        hasDefault = true;
    if (window()->inherits("QDialog") && hasDefault)
      QTreeWidget::keyPressEvent(e);
    else
      sItemSelected();
  }
  else
    QTreeWidget::keyPressEvent(e);
}
