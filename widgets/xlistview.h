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

//  xlistview.h
//  Created 03/16/2002 JSL
//  Copyright (c) 2002-2007, OpenMFG, LLC

#ifndef xlistview_h
#define xlistview_h

#include <Q3ListView>
#include <QVariant>
#include <Q3PopupMenu>

#include "OpenMFGWidgets.h"

class XSqlQuery;

//  Table Column Widths
#define _itemColumn        100
#define _whsColumn         35
#define _userColumn        80
#define _dateColumn        80
#define _timeDateColumn    160
#define _timeColumn        80
#define _qtyColumn         80
#define _priceColumn       60
#define _moneyColumn       60
#define _bigMoneyColumn    100
#define _costColumn        70
#define _prcntColumn       55
#define _transColumn       35
#define _uomColumn         45
#define _orderColumn       60
#define _statusColumn      45
#define _seqColumn         40
#define _ynColumn          45
#define _docTypeColumn     80
#define _currencyColumn    80

typedef QPair<int,QString> ColumnInfo;

class XListView;

class OPENMFGWIDGETS_EXPORT XListViewItem : public Q3ListViewItem
{
  friend class XListView;

  public:
    XListViewItem();

    XListViewItem( XListViewItem *, int, QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant() );

    XListViewItem( XListViewItem *, int, int, QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant() );

    XListViewItem( XListView *parent, int, QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant() );

    XListViewItem( XListView *parent, int, int, QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant() );

    XListViewItem( XListView *parent, XListViewItem *, int, QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant() );

    XListViewItem( XListView *parent, XListViewItem *, int, int, QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant() );

    XListViewItem( XListViewItem *parent, XListViewItem *, int, QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant() );

    XListViewItem( XListViewItem *parent, XListViewItem *, int, int, QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant(),
                   QVariant = QVariant(), QVariant = QVariant() );

    void setColor(const QString &);
    void setColor(int, const QString &);
    void closeAll();
    void openAll();
    void setText(int, QVariant);

    inline int id() const         { return _id;    }
    inline int altId() const      { return _altId; }
    inline void setId(int pId)    { _id = pId;     }
    inline void setAltId(int pId) { _altId = pId;  }

    inline XListViewItem *firstChild() const { Q3ListViewItem *item = Q3ListViewItem::firstChild();
                                               return ((XListViewItem *)item);                     };

    inline XListViewItem *nextSibling() const { Q3ListViewItem *item = Q3ListViewItem::nextSibling();
                                                return ((XListViewItem *)item);                     };
  
    inline XListViewItem *itemBelow()   { Q3ListViewItem *item = Q3ListViewItem::itemBelow();
                                          return ((XListViewItem *)item);                     };

  private:
    void constructor( int, QVariant,
                      QVariant = QVariant(), QVariant = QVariant(),
                      QVariant = QVariant(), QVariant = QVariant(),
                      QVariant = QVariant(), QVariant = QVariant(),
                      QVariant = QVariant(), QVariant = QVariant(),
                      QVariant = QVariant(), QVariant = QVariant(),
                      int = -1 );
    void paintCell(QPainter *, const QColorGroup &, int, int, int);

  QMap<int, QString> _colors;
  int                _id;
  int                _altId;
};


class OPENMFGWIDGETS_EXPORT XListView : public Q3ListView
{
  Q_OBJECT

  public:
    XListView(QWidget *, const char * = 0);

    void populate(XSqlQuery &, bool = FALSE);
    void populate(XSqlQuery &, int, bool = FALSE);
    void populate(const QString &, bool = FALSE);
    void populate(const QString &, int, bool = FALSE);
    int  addColumn(const QString &, int, int);
    void removeColumn(int pIndex);
    void hideColumn(int column);
    void showColumn(int column);
    void setColumnText(int, const QString&);

    void setDragString(QString);
    void setAltDragString(QString);

    void clear();
    void closeAll();
    void openAll();

    int  id() const;
    int  altId() const;
    void setId(int);

    inline XListViewItem *firstChild() const { return ((XListViewItem *)Q3ListView::firstChild()); };
    inline XListViewItem *lastItem() const { return ((XListViewItem *)Q3ListView::lastItem()); };
    inline XListViewItem *selectedItem() const { return ((XListViewItem *)Q3ListView::selectedItem()); };

  signals:
    void  valid(bool);
    void  newId(int);
    void  itemSelected(int);
    void  populateMenu(Q3PopupMenu *, Q3ListViewItem *, int);

  protected:
    QPoint dragStartPosition;

    virtual void mousePressEvent(QMouseEvent*);
    virtual void mouseMoveEvent(QMouseEvent*);

  private:
    QString    _dragString;
    QString    _altDragString;
    bool       _dragEnabled;
    Q3PopupMenu *_menu;
    QMap<int, ColumnInfo > _hiddenColumns;	// old width and label

  private slots:
    void sSelectionChanged();
    void sItemSelected(Q3ListViewItem *);
    void sStartDrag(int, Q3ListViewItem *);
    void sShowMenu(Q3ListViewItem *, const QPoint &, int);
    void sExport();
    void sColumnSizeChanged(int, int, int);
};

#endif

