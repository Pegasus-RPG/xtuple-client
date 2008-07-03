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

#ifndef SCREEN_H
#define SCREEN_H

#include "OpenMFGWidgets.h"
#include "xdatawidgetmapper.h"
#include "xsqltablemodel.h"

#include <QWidget>
#include <QSqlIndex>
#include <QList>

//#include <QtGui/qwidget.h>

//QT_BEGIN_HEADER

//QT_MODULE(Gui)

class OPENMFGWIDGETS_EXPORT Screen : public QWidget
{
	Q_OBJECT
		
	Q_ENUMS(Modes)
	Q_ENUMS(SearchTypes)
	
	Q_PROPERTY (QString		schemaName            READ schemaName		WRITE setSchemaName)
	Q_PROPERTY (QString 		tableName             READ tableName 		WRITE setTableName)
        Q_PROPERTY (int                 primaryKeyColumns     READ primaryKeyColumns    WRITE setPrimaryKeyColumns)
	Q_PROPERTY (Modes		mode                  READ mode                 WRITE setMode)
	Q_PROPERTY (SearchTypes 	searchType            READ searchType		WRITE setSearchType)
        Q_PROPERTY (QString             sortColumn            READ sortColumn           WRITE setSortColumn)

	public:
		Screen(QWidget * = 0);
                ~Screen();
		
		enum Modes { New, Edit, View };
		enum SearchTypes { Query, List };
		Modes mode();
		SearchTypes searchType();
	
                bool    isDirty();
                int     primaryKeyColumns() const { return _keyColumns;       };
	        QString schemaName()        const { return _schemaName;       };
	        QString sortColumn()        const { return _sortColumn;       };
		QString tableName()         const { return _tableName;        };
       	
	public slots:
		void toNext()                           { _mapper.toNext();          };
		void toPrevious()                       { _mapper.toPrevious();       };
		void insert();
		void save();
		void search(QString criteria);
		void select();
                void setCurrentIndex(int index)         { _mapper.setCurrentIndex(index);};
		void setFilter(QString filter)          { _model->setFilter(filter);      };
		void setMode(Modes p);
                void setPrimaryKeyColumns(int count)    { _keyColumns = count;           };
		void setSearchType(SearchTypes p);
		void setSchemaName(QString schema)      { _schemaName = schema;           };
		void setSortColumn(QString p);
 
		void setTableName(QString table)	{ _tableName = table;            };
		void setTable(QString schema, QString table);
		void setDataWidgetMapper(QSqlTableModel *model);
	
	signals:
		void newDataWidgetMapper(XDataWidgetMapper *mapper);
                void newModel(XSqlTableModel *model);
		void saved(bool);

	private:
		enum  Modes		_mode;
		enum  SearchTypes	_searchType;
                int                     _keyColumns;
		QString			_schemaName;
                QString                 _sortColumn;
		QString			_tableName;
		XDataWidgetMapper	_mapper;
                XSqlTableModel*		_model;
};

#endif // SCREEN_H
