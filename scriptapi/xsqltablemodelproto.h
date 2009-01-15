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
 * The Original Code is xTuple ERP: PostBooks Edition 
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
 * Powered by xTuple ERP: PostBooks Edition
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

#ifndef __XSQLTABLEMODELPROTO_H__
#define __XSQLTABLEMODELPROTO_H__

#include <QMap>
#include <QModelIndex>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlIndex>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlRelation>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QtScript>

#include "xsqltablemodel.h"

class QMimeData;

Q_DECLARE_METATYPE(XSqlTableModel*)
//Q_DECLARE_METATYPE(XSqlTableModel)

void setupXSqlTableModelProto(QScriptEngine *engine);
QScriptValue constructXSqlTableModel(QScriptContext *context, QScriptEngine *engine);

class XSqlTableModelProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    XSqlTableModelProto(QObject *parent = 0);

    Q_INVOKABLE QModelIndex     buddy(const QModelIndex &index) const;
    Q_INVOKABLE bool            canFetchMore(const QModelIndex &parent = QModelIndex()) const;
    Q_INVOKABLE void            clear();
    Q_INVOKABLE int             columnCount(const QModelIndex &parent = QModelIndex());
    Q_INVOKABLE QVariant        data(const QModelIndex &item, int role = Qt::DisplayRole) const;
    Q_INVOKABLE QSqlDatabase    database()                          const;
    Q_INVOKABLE bool            dropMimeData(const QMimeData *data, int action, int row, int column, const QModelIndex &parent);
    Q_INVOKABLE int             editStrategy()                      const;
    Q_INVOKABLE void            fetchMore(const QModelIndex &parent = QModelIndex());
    Q_INVOKABLE int             fieldIndex(const QString &fieldName) const;
    Q_INVOKABLE QString         filter()                             const;
    Q_INVOKABLE int             flags(const QModelIndex &index)      const;
    Q_INVOKABLE bool            hasIndex(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    Q_INVOKABLE QVariant        headerData(int section, int orientation, int role = Qt::DisplayRole) const;
    Q_INVOKABLE QModelIndex     index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    Q_INVOKABLE bool            insertColumn(int column, const QModelIndex &parent = QModelIndex());
    Q_INVOKABLE bool            insertColumns(int column, int count, const QModelIndex &parent = QModelIndex());
    Q_INVOKABLE bool            insertRecord(int row, const QSqlRecord &record);
    Q_INVOKABLE bool            insertRow(int row, const QModelIndex &parent = QModelIndex());
    Q_INVOKABLE bool            insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    Q_INVOKABLE bool            isDirty(const QModelIndex &index)      const;
    Q_INVOKABLE QMap<int, QVariant> itemData(const QModelIndex &index) const;
    Q_INVOKABLE QSqlError       lastError()                            const;
    Q_INVOKABLE QModelIndexList match(const QModelIndex &start, int role, const QVariant &value, int hits = 1, int flags = Qt::MatchFlags(Qt::MatchStartsWith | Qt::MatchWrap)) const;
    Q_INVOKABLE QMimeData      *mimeData(const QModelIndexList &indexes) const;
    Q_INVOKABLE QStringList     mimeTypes()                              const;
    Q_INVOKABLE QSqlIndex       primaryKey()                             const;
    Q_INVOKABLE QSqlQuery       query()                                  const;
    Q_INVOKABLE QSqlRecord      record()                                 const;
    Q_INVOKABLE QSqlRecord      record(int row)                          const;
    Q_INVOKABLE QSqlRelation    relation(int column)                     const;
    Q_INVOKABLE QSqlTableModel *relationModel(int column)                const;
    Q_INVOKABLE bool            removeColumn(int column, const QModelIndex &parent = QModelIndex());
    Q_INVOKABLE bool            removeColumns(int column, int count, const QModelIndex &parent = QModelIndex());
    Q_INVOKABLE bool            removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    Q_INVOKABLE void            revertRow(int row);
    Q_INVOKABLE int             rowCount(const QModelIndex &parent = QModelIndex()) const;
    Q_INVOKABLE bool            select();
    Q_INVOKABLE bool            setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    Q_INVOKABLE void            setEditStrategy(int strategy);
    Q_INVOKABLE void            setFilter(const QString &filter);
    Q_INVOKABLE bool            setHeaderData(int section, int orientation, const QVariant &value, int role = Qt::EditRole);
    Q_INVOKABLE bool            setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);
    Q_INVOKABLE void            setKeys(int keyColumns);
    Q_INVOKABLE bool            setRecord(int row, const QSqlRecord &record);
    Q_INVOKABLE void            setRelation(int column, const QSqlRelation &relation);
    Q_INVOKABLE void            setSort(int column, int order);
    Q_INVOKABLE void            setSupportedDragActions(int actions);
    Q_INVOKABLE void            setTable(const QString &tableName, int keyColumns);
    Q_INVOKABLE QModelIndex     sibling(int row, int column, const QModelIndex &index) const;
    Q_INVOKABLE void            sort(int column, int order = Qt::AscendingOrder);
    Q_INVOKABLE QSize           span(const QModelIndex &index) const;
    Q_INVOKABLE int             supportedDragActions()         const;
    Q_INVOKABLE int             supportedDropActions()         const;
    Q_INVOKABLE QString         tableName()                    const;
    Q_INVOKABLE QString         toString()                     const;
};

#endif
