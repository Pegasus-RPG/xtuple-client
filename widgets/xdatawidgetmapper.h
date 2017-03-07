/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef XDATAWIDGETMAPPER_H
#define XDATAWIDGETMAPPER_H

#include <QDataWidgetMapper>
#include <QPointer>
#include <QSqlTableModel>
#include <QWidget>

#include "widgets.h"

class QScriptEngine;

class XTUPLEWIDGETS_EXPORT XDataWidgetMapper : public QDataWidgetMapper
{
  Q_OBJECT

  public:
    XDataWidgetMapper(QObject *parent = 0);
    ~XDataWidgetMapper();
    
    Q_INVOKABLE virtual QByteArray mappedDefaultName(QWidget *widget);
    Q_INVOKABLE virtual void       addMapping(QWidget *widget, QString fieldName);
    Q_INVOKABLE virtual void       addMapping(QWidget *widget, QString fieldName, const QByteArray &propertyName);
    Q_INVOKABLE virtual void       addMapping(QWidget *widget, QString fieldName, const QByteArray &propertyName, const QByteArray &defaultName);
    Q_INVOKABLE virtual void       removeDefault(QWidget *widget);
    Q_INVOKABLE virtual void       clearMapping();
    Q_INVOKABLE virtual int        currentIndex() const;

    Q_INVOKABLE virtual QAbstractItemDelegate *itemDelegate() const;
    Q_INVOKABLE virtual QByteArray             mappedPropertyName(QWidget *widget) const;
    Q_INVOKABLE virtual int                    mappedSection(QWidget *widget) const;
    Q_INVOKABLE virtual QWidget               *mappedWidgetAt(int section) const;
    Q_INVOKABLE virtual QAbstractItemModel    *model() const;
    Q_INVOKABLE virtual int         orientation() const;
    Q_INVOKABLE virtual void        removeMapping(QWidget *widget);
    Q_INVOKABLE virtual QModelIndex rootIndex() const;
    Q_INVOKABLE virtual void        setItemDelegate(QAbstractItemDelegate *delegate);
    Q_INVOKABLE virtual void        setModel(QAbstractItemModel *model);
    Q_INVOKABLE virtual void        setOrientation(Qt::Orientation orientation);
    Q_INVOKABLE virtual void        setRootIndex(const QModelIndex &index);
    Q_INVOKABLE virtual void        setSubmitPolicy(QDataWidgetMapper::SubmitPolicy policy);
    Q_INVOKABLE virtual int         submitPolicy() const;
    
  public slots:
    virtual void    clear();
    virtual QString toString() const;
    
  signals:
    bool newMapping(QWidget *widget);

  private:
    struct WidgetMapper
    {
        inline WidgetMapper(QWidget *w, int c, const QByteArray &p)
            : widget(w),  section(c), property(p) {}

        QPointer<QWidget> widget;
        int section;
        QPersistentModelIndex currentIndex;
        QByteArray property;
    };

    QList<WidgetMapper> widgetMap;
    virtual void clear(WidgetMapper &m);

};

void setupXDataWidgetMapper(QScriptEngine *engine);

#endif
