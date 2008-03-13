/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "xtreewidget.h"

#include <QMenu>
#include <QFileDialog>
#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QHeaderView>
#include <QSettings>
#include <QAction>

#include "xsqlquery.h"

XTreeWidget::XTreeWidget(QWidget *pParent) :
  QTreeWidget(pParent)
{
  _resizingInProcess = false;
  _forgetful = false;
  _settingsLoaded = false;
  _menu = new QMenu(this);

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

  setRootIsDecorated(false);
  header()->setContextMenuPolicy(Qt::CustomContextMenu);

#ifdef Q_WS_MAC
  QFont f = font();
  f.setPointSize(f.pointSize() - 2);
  setFont(f);
#endif
}

XTreeWidget::~XTreeWidget()
{
  QSettings settings(QSettings::UserScope, "OpenMFG.com", "OpenMFG");
  settings.setValue(_settingsName + "/isForgetful", _forgetful);
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
    settings.setValue(_settingsName + "/columnWidths", savedString);
  }
  if(_x_preferences)
  {
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

void XTreeWidget::populate(XSqlQuery &pQuery, bool pUseAltId)
{
  populate(pQuery, id(), pUseAltId);
}

void XTreeWidget::populate(XSqlQuery &pQuery, int pIndex, bool pUseAltId)
{
  qApp->setOverrideCursor(Qt::waitCursor);

  //QModelIndex position = currentIndex();
  
  clear();

  if (pQuery.first())
  {
    int fieldCount = pQuery.count();
    if (fieldCount > 1)
    {
      XTreeWidgetItem *last     = NULL;
      XTreeWidgetItem *selected = NULL;

      do
      {
        if (pUseAltId)
          last = new XTreeWidgetItem(this, last, pQuery.value(0).toInt(), pQuery.value(1).toInt(), pQuery.value(2).toString());
        else
          last = new XTreeWidgetItem(this, last, pQuery.value(0).toInt(), pQuery.value(1));

        if (fieldCount > ((pUseAltId) ? 3 : 2))
          for (int counter = ((pUseAltId) ? 3 : 2); counter < fieldCount; counter++)
            last->setText((counter - ((pUseAltId) ? 2 : 1)), pQuery.value(counter).toString());

        if (last->_id == pIndex)
          selected = last;
      }
      while (pQuery.next());

      if (selected != NULL)
      {
        setItemSelected(selected, TRUE);
        scrollToItem(selected);
        emit valid(TRUE);
      }
      else
      {
/*
// This was removed as it was causing a crash problem in some situations.
        setCurrentIndex(position);
        if(currentItem())
        {
          scrollToItem(currentItem());
          emit valid(TRUE);
        }
        else
*/
          emit valid(FALSE);
      }
    }
  }
  else
    emit valid(FALSE);

  qApp->restoreOverrideCursor();
}

void XTreeWidget::addColumn(const QString & pString, int pWidth, int pAlignment, bool pVisible)
{
  if(!_settingsLoaded)
  {
    _settingsLoaded = true;

    QString pname;
    if(window())
      pname = window()->objectName() + "/";
    _settingsName = pname + objectName();

    // Load any previously saved information about column widths
    QSettings settings(QSettings::UserScope, "OpenMFG.com", "OpenMFG");
    _forgetful = settings.value(_settingsName + "/isForgetful").toBool();

    QString savedString;
    QStringList savedParts;
    QString part, key, val;
    bool b1 = false, b2 = false;
    if(!_forgetful)
    {
      savedString = settings.value(_settingsName + "/columnWidths").toString();
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
  hitem->setText(column, pString);
  hitem->setTextAlignment(column, pAlignment);
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
  XTreeWidgetItem * item = topLevelItem(0);
  if(item)
  {
    QModelIndex idx = indexFromItem(item);
    while(idx.isValid())
    {
      item = (XTreeWidgetItem*)itemFromIndex(idx);
      if(item && item->id() == pId)
      {
        setCurrentIndex(idx);
        return;
      }
      idx = indexBelow(idx);
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
  QString filename = QFileDialog::getSaveFileName(this, tr("Export Save Filename"), QString::null, "*.txt");
  if (!filename.isEmpty())
  {
    QFileInfo fi(filename);
    if(fi.suffix().isEmpty())
      filename += ".txt";

    QString line;
    QFile   fileExport(filename);
    int     counter;

    if (fileExport.open(QIODevice::WriteOnly))
    {
      QTreeWidgetItem * header = headerItem();
      for (counter = 0; counter < header->columnCount(); counter++)
      {
        line = header->text(counter) + "\t";
        fileExport.writeBlock(line, line.length());
      }
      fileExport.writeBlock("\n", 1);

      XTreeWidgetItem * item = topLevelItem(0);
      if(item)
      {
        QModelIndex idx = indexFromItem(item);
        while(idx.isValid())
        {
          item = (XTreeWidgetItem*)itemFromIndex(idx);
          if(item)
          {
            for (counter = 0; counter < item->columnCount(); counter++)
            {
              line = item->text(counter) + "\t";
              fileExport.writeBlock(line, line.length());
            }
            fileExport.writeBlock("\n", 1);
          }
          idx = indexBelow(idx);
        }
      }
    }
    fileExport.close();
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

void XTreeWidgetItem::setTextColor(const QColor &pColor)
{
  for (int cursor = 0; cursor < columnCount(); cursor++)
    QTreeWidgetItem::setTextColor(cursor, pColor);
}

void XTreeWidgetItem::setText(int pColumn, const QVariant & pVariant)
{
  QTreeWidgetItem::setText(pColumn, pVariant.toString());
}

