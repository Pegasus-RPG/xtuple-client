  /*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef XTREEVIEW_H
#define XTREEVIEW_H

#include <QTreeView>
#include <QMenu>

#include "xdatawidgetmapper.h"
#include "xsqltablemodel.h"
#include "widgets.h"

class XTUPLEWIDGETS_EXPORT XTreeView : public QTreeView
{
    Q_OBJECT
    Q_PROPERTY(QString         schemaName            READ schemaName           WRITE setSchemaName       )
    Q_PROPERTY(QString         tableName             READ tableName            WRITE setTableName        )
    Q_PROPERTY(int             primaryKeyCoulmns     READ primaryKeyColumns    WRITE setPrimaryKeyColumns)
    
    public:
      XTreeView(QWidget *parent = 0);
      ~XTreeView();

      Q_INVOKABLE bool    isRowHidden(int row);
                  int     primaryKeyColumns() const { return _keyColumns; };
                  QString schemaName()        const { return _schemaName; };
                  QString tableName()         const { return _tableName;  };
                  bool    throwScriptException(const QString &message);

      Q_INVOKABLE      QString columnNameFromLogicalIndex(const int logicalIndex) const;
      Q_INVOKABLE         void setColumn(const QString &label, int width, int alignment, bool visible, const QString &colname);
      Q_INVOKABLE virtual void setColumnRole(int column, int role, QVariant value);
      Q_INVOKABLE virtual void setColumnRole(const QString column, int role, QVariant value);
      Q_INVOKABLE virtual void setColumnLocked(const QString &pColname, bool pLocked);
      Q_INVOKABLE virtual void setColumnLocked(const int      pColumn, bool pLocked);
      Q_INVOKABLE virtual void setColumnVisible(int, bool);
      Q_INVOKABLE virtual void setForegroundColor(int row, int col, QString color);
      Q_INVOKABLE virtual void setFormat(const QString column, int format);
      Q_INVOKABLE virtual void setRowForegroundColor(int row, QString color);
      Q_INVOKABLE         void setTable();
      Q_INVOKABLE virtual void setTextAlignment(int column, int alignment);
      Q_INVOKABLE virtual void setTextAlignment(const QString column, int alignment);
      Q_INVOKABLE XDataWidgetMapper *mapper()  { return _mapper;};
      Q_INVOKABLE XSqlTableModel    *model()   { return _model;};
      
    public slots:
      virtual int  rowCount();
      virtual int  rowCountVisible();
      virtual QString filter();
      virtual QVariant value(int row, int column, int role = Qt::DisplayRole);
      virtual QVariant selectedValue(int column);
      virtual void insert();
      virtual void populate(int p);
      virtual void removeSelected();
      virtual void revertAll();
      virtual void save();
      virtual void select();
      virtual void selectRow(int index);
      virtual void setFilter(const QString filter);
      virtual void setDataWidgetMap(XDataWidgetMapper* mapper);
      virtual void setModel(XSqlTableModel* model=0);
      virtual void setPrimaryKeyColumns(int p)                { _keyColumns = p;            };
      virtual void setSchemaName(const QString p)             { _schemaName = p;            };
      virtual void setTableName(const QString p)              { _tableName = p;             };
      virtual void setValue(int row, int column, QVariant value);
      virtual void sShowMenu(const QPoint &);

    signals:
      void  dataChanged(int row, int col);
      void  newModel(XSqlTableModel *model);
      void  rowSelected(int);
      void  valid(bool);
      void  saved();
      void  populateMenu(QMenu *, QModelIndex);
      
    protected:
      virtual void applyColumnRole(int column, int role, QVariant value);
      virtual void applyColumnRoles();
      virtual void applyColumnRoles(int row);
      virtual void resizeEvent(QResizeEvent*);
      virtual void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);

    private slots:
      void handleDataChanged(const QModelIndex topLeft, const QModelIndex lowerRight);
      void popupMenuActionTriggered(QAction*);
      void sColumnSizeChanged(int, int, int);
      void sResetAllWidths();
      void sResetWidth();
      void sShowHeaderMenu(const QPoint &);
      void sToggleForgetfulness();
      
    private:
      bool                 _forgetful;
      QSqlRecord           _idx;
      int                  _keyColumns;
      XDataWidgetMapper   *_mapper;
      QMenu               *_menu;
      XSqlTableModel      *_model;
      int                  _resetWhichWidth;
      bool                 _resizingInProcess;
      QString              _schemaName;
      bool                 _settingsLoaded;
      QString              _tableName;
      QString              _windowName;

      struct ColumnProps { 
        QString columnName;
        int     logicalIndex;
        int     defaultWidth;
        int     savedWidth;
        bool    stretchy;
        bool    locked;
        bool    visible;
        QString label;
        int     alignment;
        bool    fromSettings;
      };
      QMap<QString, ColumnProps*> _columnByName;
      QMultiHash<int, QPair<QVariant, int> > _columnRoles;
      
};

#endif
