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
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
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

//  xlistview.cpp
//  Created 03/16/2003 JSL
//  Copyright (c) 2003-2007, OpenMFG, LLC

#include <Q3PopupMenu>
#include <QDrag>
#include <QMenu>
#include <QCursor>
#include <QApplication>
#include <QFile>
#include <Q3FileDialog>
#include <Q3Header>
#include <QMouseEvent>

#include <xsqlquery.h>
#include "xlistview.h"

#define cMinColumnWidth 30

XListView::XListView(QWidget *pParent, const char *name) :
  Q3ListView(pParent, name)
{
  _dragString    = "";
  _altDragString = "";
  _dragEnabled   = FALSE;
  _menu          = new Q3PopupMenu(this);

  setAllColumnsShowFocus(TRUE);
  setSorting(99);
  setMultiSelection(FALSE);

  connect(this, SIGNAL(selectionChanged()), this, SLOT(sSelectionChanged()));
  connect(this, SIGNAL(doubleClicked(Q3ListViewItem *)), this, SLOT(sItemSelected(Q3ListViewItem *)));
  connect(this, SIGNAL(mouseButtonPressed(int, Q3ListViewItem *, const QPoint &, int)), this, SLOT(sStartDrag(int, Q3ListViewItem *)));
  connect(this, SIGNAL(contextMenuRequested(Q3ListViewItem *, const QPoint &, int)), SLOT(sShowMenu(Q3ListViewItem *, const QPoint &, int)));
  connect(header(), SIGNAL(sizeChange(int, int, int)),
	  this,     SLOT(sColumnSizeChanged(int, int, int)));

  emit valid(FALSE);

#ifdef Q_WS_MAC
  QFont f = font();
  f.setPointSize(f.pointSize() - 2);
  setFont(f);
#endif
}

void XListView::populate(const QString &pSql, bool pUseAltId)
{
  qApp->setOverrideCursor(Qt::waitCursor);

  XSqlQuery query(pSql);
  populate(query, pUseAltId);

  qApp->restoreOverrideCursor();
}

void XListView::populate(const QString &pSql, int pIndex, bool pUseAltId)
{
  qApp->setOverrideCursor(Qt::waitCursor);

  XSqlQuery query(pSql);
  populate(query, pIndex, pUseAltId);

  qApp->restoreOverrideCursor();
}

void XListView::populate(XSqlQuery &pQuery, bool pUseAltId)
{
  populate(pQuery, id(), pUseAltId);
}

void XListView::populate(XSqlQuery &pQuery, int pIndex, bool pUseAltId)
{
  qApp->setOverrideCursor(Qt::waitCursor);

  int position = -1;
  if ((selectionMode() == Single) && (selectedItem()))
  {
//  Determine the position of the currently selected item
    int counter           = 0;
    Q3ListViewItem *cursor = firstChild();
    while (cursor)
    {
      if (cursor == selectedItem())
      {
        position = (counter - 1);
        break;
      }
      else
      {
        cursor = cursor->nextSibling();
        counter++;
      }
    }
  }

  clear();

  if (pQuery.first())
  {
    int fieldCount = pQuery.count();
    if (fieldCount > 1)
    {
      XListViewItem *last     = NULL;
      XListViewItem *selected = NULL;

      do
      {
        if (pUseAltId)
          last = new XListViewItem(this, last, pQuery.value(0).toInt(), pQuery.value(1).toInt(), pQuery.value(2).toString());
        else
          last = new XListViewItem(this, last, pQuery.value(0).toInt(), pQuery.value(1));

        if (fieldCount > ((pUseAltId) ? 3 : 2))
          for (int counter = ((pUseAltId) ? 3 : 2); counter < fieldCount; counter++)
            last->setText((counter - ((pUseAltId) ? 2 : 1)), pQuery.value(counter).toString());

        if (last->_id == pIndex)
          selected = last;
      }
      while (pQuery.next());

      if (selected != NULL)
      {
        setSelected(selected, TRUE);
        ensureItemVisible(selected);
        emit valid(TRUE);
      }
      else if (position != -1)
      {
        Q3ListViewItem *cursor = firstChild();
        int counter;
        for (counter = 0; ((counter < position) && (cursor)); counter++, cursor = cursor->nextSibling());
        if (cursor)
        {
          setSelected(cursor, TRUE);
          ensureItemVisible(cursor);
          emit valid(TRUE);
        }
      }
      else
        emit valid(FALSE);
    }
  }
  else
    emit valid(FALSE);

  qApp->restoreOverrideCursor();
}

int XListView::addColumn(const QString &pLabel, int pWidth, int pAlignment)
{
  int result;

  if (pWidth == -1)
  {
    result = Q3ListView::addColumn(pLabel, cMinColumnWidth);
    header()->setStretchEnabled(true, result);
  }
  else
    result = Q3ListView::addColumn(pLabel, pWidth);

  setColumnAlignment(result, pAlignment);

  header()->adjustHeaderSize();

  return result;
}

void XListView::removeColumn(int pIndex)
{
  Q3ListView::removeColumn(pIndex);

  header()->adjustHeaderSize();
}

/*
   QListView::hideColumn(int) sets the column width to 0, but
   resizing columns can automatically reveal the 'hidden' column.
   Hence in XListView we rehide the column whenever the XListView
   or one of its columns is resized (see XListView::XListView()'s
   connect() calls).  If we leave the columnText unchanged then
   there's flicker as the title gets redrawn/erased/redrawn/erased,
   so set it to "" until the column is reshown.
*/

void XListView::hideColumn(int column)
{
    if (! _hiddenColumns.contains(column))
    {
      ColumnInfo tmpColInfo = qMakePair(columnWidth(column), columnText(column));
      _hiddenColumns.insert(column, tmpColInfo);
      Q3ListView::setColumnText(column, "");
      //setColumnWidthMode(column, QListView::Manual);
      //header()->setResizeEnabled(false, column);
    }
    Q3ListView::hideColumn(column);
}

void XListView::sColumnSizeChanged(int section,
				   int /*oldSize*/,
				   int /*newSize*/)
{
  if (_hiddenColumns.contains(section))
    Q3ListView::hideColumn(section);
}

void XListView::showColumn(int column)
{
  QMap<int,ColumnInfo>::iterator mapIterator = _hiddenColumns.find(column);
  if (mapIterator != _hiddenColumns.end())
  {
    int tmpWidth = mapIterator.data().first;
    QString tmpText = mapIterator.data().second;
    //header()->setResizeEnabled(true, column);
    //setColumnWidthMode(column, QListView::Maximum);
    _hiddenColumns.erase(column);

    setColumnWidth(column, tmpWidth);
    setColumnText(column, tmpText);
  }
}
// end {hide,show}Column()

void XListView::setColumnText(int column, const QString& label)
{
  QMap<int,ColumnInfo>::iterator mapIterator = _hiddenColumns.find(column);
  if (mapIterator != _hiddenColumns.end())
  {
    ColumnInfo tmpColInfo = qMakePair(mapIterator.data().first, label);
    _hiddenColumns.insert(column, tmpColInfo);
  }
  else
    Q3ListView::setColumnText(column, label);
}

int XListView::id() const
{
  if (selectionMode() == Single)
  {
    if (selectedItem() == 0)
      return -1;
    else
      return (((XListViewItem *)selectedItem())->_id);
  }
  else
  {
    for (XListViewItem *cursor = firstChild(); cursor; cursor = cursor->itemBelow())
    {
      if (cursor->isSelected())
        return cursor->id();
    }

    return -1;
  }
}

int XListView::altId() const
{
  if (selectionMode() == Single)
  {
    if (selectedItem() == 0)
      return -1;
    else
      return (((XListViewItem *)selectedItem())->_altId);
  }
  else
  {
    for (XListViewItem *cursor = firstChild(); cursor; cursor = cursor->itemBelow())
    {
      if (cursor->isSelected())
        return cursor->altId();
    }

    return -1;
  }
}

void XListView::setDragString(QString pDragString)
{
  _dragEnabled = TRUE;
  _dragString = pDragString;
}

void XListView::setAltDragString(QString pAltDragString)
{
  _dragEnabled = TRUE;
  _altDragString = pAltDragString;
}

void XListView::setId(int pId)
{
  for (XListViewItem *cursor = firstChild(); cursor != 0; cursor = cursor->nextSibling())
  {
    if (cursor->_id == pId)
    {
      setSelected(cursor, TRUE);
      return;
    }
  }
}

void XListView::sSelectionChanged()
{
  if (selectedItem() != 0)
  {
    emit valid(TRUE);
    emit newId(((XListViewItem *)selectedItem())->_id);
  }
  else if (selectionMode() == Single)
  {
    emit valid(FALSE);
    emit newId(-1);
  }
  else if (selectionMode() != NoSelection)
  {
    XListViewItem *cursor = firstChild();
    while (cursor != 0)
    {
      if (isSelected(cursor))
      {
        emit valid(TRUE);
        emit newId(cursor->_id);
        return;
      }

      cursor = cursor->nextSibling();
    }

    emit valid(FALSE);
    emit newId(-1);
  }
  else
  {
    emit valid(FALSE);
    emit newId(-1);
  }
}

void XListView::sItemSelected(Q3ListViewItem *pSelected)
{
  emit itemSelected(((XListViewItem *)pSelected)->_id);
}

void XListView::sShowMenu(Q3ListViewItem *pSelected, const QPoint &pntThis, int pColumn)
{
  if (pSelected)
  {
    _menu->clear();
    emit populateMenu(_menu, pSelected, pColumn);

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
      _menu->popup(pntThis);
  }
}

void XListView::sExport()
{
  QString filename = Q3FileDialog::getSaveFileName(QString::null, "*.txt", this);
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
      for (counter = 0; counter < columns(); counter++)
      {
        line = columnText(counter) + "\t";
        fileExport.writeBlock(line, line.length());
      }
      fileExport.writeBlock("\n", 1);

      for (XListViewItem *cursor = firstChild(); cursor != NULL; cursor = cursor->itemBelow())
      {
        for (counter = 0; counter < columns(); counter++)
        {
          line = cursor->text(counter) + "\t";
          fileExport.writeBlock(line, line.length());
        }
        fileExport.writeBlock("\n", 1);
      }
    }
    fileExport.close();
  }
}

void XListView::sStartDrag(int pButton, Q3ListViewItem *item)
{
  if ( (pButton == Qt::LeftButton) && (item != 0) && (_dragEnabled) )
  {
    QString dragDescription;

    if (_dragString.length())
      dragDescription = _dragString + QString("%1").arg(id());

    if (_altDragString.length())
    {
      if (dragDescription.length())
        dragDescription += ",";

      dragDescription += _altDragString + QString("%1").arg(altId());
    }

    //Q3DragObject *drag = new Q3TextDrag(dragDescription, this);
    //drag->dragCopy();
  }
}

void XListView::clear()
{
  emit valid(FALSE);

  Q3ListView::clear();
}

void XListView::closeAll()
{
  XListViewItem *cursor = (XListViewItem *)firstChild();

  while (cursor != NULL)
  {
    cursor->closeAll();
    cursor = (XListViewItem *)cursor->nextSibling();
  }
}
    
void XListView::openAll()
{
  XListViewItem *cursor = (XListViewItem *)firstChild();

  while (cursor != NULL)
  {
    cursor->openAll();
    cursor = (XListViewItem *)cursor->nextSibling();
  }
}

void XListView::mousePressEvent(QMouseEvent * event)
{
  if (event->button() == Qt::LeftButton)
    dragStartPosition = event->pos();
  Q3ListView::mousePressEvent(event);
}

void XListView::mouseMoveEvent(QMouseEvent * event)
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

  Q3ListView::mouseMoveEvent(event);
}


XListViewItem::XListViewItem( XListViewItem *itm, int pId, QVariant v0,
                              QVariant v1, QVariant v2,
                              QVariant v3, QVariant v4,
                              QVariant v5, QVariant v6,
                              QVariant v7, QVariant v8,
                              QVariant v9, QVariant v10 ) :
  Q3ListViewItem(itm)
{
  constructor(pId, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
}

XListViewItem::XListViewItem( XListViewItem *itm, int pId, int pAltId, QVariant v0,
                              QVariant v1, QVariant v2,
                              QVariant v3, QVariant v4,
                              QVariant v5, QVariant v6,
                              QVariant v7, QVariant v8,
                              QVariant v9, QVariant v10 ) :
  Q3ListViewItem(itm)
{
  constructor(pId, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, pAltId);
}

XListViewItem::XListViewItem( XListView *pParent, int pId, QVariant v0,
                              QVariant v1, QVariant v2,
                              QVariant v3, QVariant v4,
                              QVariant v5, QVariant v6,
                              QVariant v7, QVariant v8,
                              QVariant v9, QVariant v10 ) :
  Q3ListViewItem(pParent)
{
  constructor(pId, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
}

XListViewItem::XListViewItem( XListView *pParent, int pId, int pAltId, QVariant v0,
                              QVariant v1, QVariant v2,
                              QVariant v3, QVariant v4,
                              QVariant v5, QVariant v6,
                              QVariant v7, QVariant v8,
                              QVariant v9, QVariant v10 ) :
  Q3ListViewItem(pParent)
{
  constructor(pId, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, pAltId);
}

XListViewItem::XListViewItem( XListView *pParent, XListViewItem *itm, int pId, QVariant v0,
                              QVariant v1, QVariant v2,
                              QVariant v3, QVariant v4,
                              QVariant v5, QVariant v6,
                              QVariant v7, QVariant v8,
                              QVariant v9, QVariant v10 ) :
  Q3ListViewItem(pParent, itm)
{
  constructor(pId, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
}

XListViewItem::XListViewItem( XListView *pParent, XListViewItem *itm, int pId, int pAltId, QVariant v0,
                              QVariant v1, QVariant v2,
                              QVariant v3, QVariant v4,
                              QVariant v5, QVariant v6,
                              QVariant v7, QVariant v8,
                              QVariant v9, QVariant v10 ) :
  Q3ListViewItem(pParent, itm)
{
  constructor(pId, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, pAltId);
}

XListViewItem::XListViewItem( XListViewItem *pParent, XListViewItem *itm, int pId, QVariant v0,
                              QVariant v1, QVariant v2,
                              QVariant v3, QVariant v4,
                              QVariant v5, QVariant v6,
                              QVariant v7, QVariant v8,
                              QVariant v9, QVariant v10 ) :
  Q3ListViewItem(pParent, itm)
{
  constructor(pId, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
}

XListViewItem::XListViewItem( XListViewItem *pParent, XListViewItem *itm, int pId, int pAltId, QVariant v0,
                              QVariant v1, QVariant v2,
                              QVariant v3, QVariant v4,
                              QVariant v5, QVariant v6,
                              QVariant v7, QVariant v8,
                              QVariant v9, QVariant v10 ) :
  Q3ListViewItem(pParent, itm)
{
  constructor(pId, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, pAltId);
}

void XListViewItem::constructor(int pId, QVariant v0, 
                                QVariant v1, QVariant v2,
                                QVariant v3, QVariant v4,
                                QVariant v5, QVariant v6,
                                QVariant v7, QVariant v8,
                                QVariant v9, QVariant v10,
                                int pAltId )
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
}

void XListViewItem::setColor(const QString &pColor)
{
  for (int cursor = 0; cursor < listView()->columns(); cursor++)
    _colors.insert(cursor, pColor);
}

void XListViewItem::setColor(int pColumn, const QString &pColor)
{
  _colors.insert(pColumn, pColor, TRUE);
}

void XListViewItem::paintCell(QPainter *pPoint, const QColorGroup &pDefaultColor, int pColumn, int pWidth, int pAlign)
{
  QMap<int, QString>::Iterator color = _colors.find(pColumn);

  if (color == _colors.end())
    Q3ListViewItem::paintCell(pPoint, pDefaultColor, pColumn, pWidth, pAlign);
  else
  {
    QColorGroup group(pDefaultColor);

    group.setColor(QColorGroup::Text, color.data());
    Q3ListViewItem::paintCell(pPoint, group, pColumn, pWidth, pAlign);
  }
}

void XListViewItem::closeAll()
{
  XListViewItem *cursor = (XListViewItem *)firstChild();

  while (cursor != NULL)
  {
    if (cursor->childCount())
      cursor->closeAll();

    if (cursor->isOpen())
      cursor->setOpen(FALSE);

    cursor = (XListViewItem *)cursor->nextSibling();
  }

  if (isOpen())
    setOpen(FALSE);
}

void XListViewItem::openAll()
{
  XListViewItem *cursor = (XListViewItem *)firstChild();

  while (cursor != NULL)
  {
    if (cursor->childCount())
      cursor->openAll();

    if (!cursor->isOpen())
      cursor->setOpen(TRUE);

    cursor = (XListViewItem *)cursor->nextSibling();
  }

  if (!isOpen())
    setOpen(TRUE);
}

void XListViewItem::setText(int pColumn, QVariant pVariant)
{
  Q3ListViewItem::setText(pColumn, pVariant.toString());
}


