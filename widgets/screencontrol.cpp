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

#include "screencontrol.h"
#include <QMessageBox>

ScreenControl::ScreenControl(QWidget *parent) : 
  QWidget(parent)
{
  setupUi(this);
  _autoSave=false;
  _searchType=Query;
  _shown=false;
  
  connect (_new,	SIGNAL(clicked()),	this,	SLOT(newRow()));
  connect (_save,       SIGNAL(clicked()),      this,   SIGNAL(saveClicked()));
  connect (_save,	SIGNAL(clicked()),	this,	SLOT(save()));
  connect (_print,	SIGNAL(clicked()),	this,	SLOT(print()));
  connect (_prev,	SIGNAL(clicked()),	this,	SLOT(toPrevious()));
  connect (_next,	SIGNAL(clicked()),	this,	SLOT(toNext()));
  connect (_search,	SIGNAL(clicked()),	this,	SLOT(search()));
  
  //Hiding future functionality that has been commented out for now
  _print->setVisible(FALSE);
  _view->setVisible(FALSE);

}

ScreenControl::Modes ScreenControl::mode()
{
  return _mode;
}

ScreenControl::SearchTypes ScreenControl::searchType()
{
  return _searchType;
}

void ScreenControl::languageChange()
{
    retranslateUi(this);
}

/* When the widget is first shown, set up table mappings if they exist*/
void ScreenControl::showEvent(QShowEvent *event)
{
  if(!_shown)
  {
    _shown = true;
    setTable(_schemaName, _tableName);
  }
  QWidget::showEvent(event);
}

void ScreenControl::toNext()
{
  _mapper.toNext();
  _prev->setEnabled(true);
  _next->setEnabled(_mapper.currentIndex() < _model.rowCount()-1);
}

void ScreenControl::toPrevious()
{
  _mapper.toPrevious();
  _next->setEnabled(true);
  _prev->setEnabled(_mapper.currentIndex());
}

void ScreenControl::newRow()
{
  _new->setEnabled(true);
  _save->setEnabled(true);
  _view->setEnabled(true);
  _print->setEnabled(true);
  _model.insertRows(_model.rowCount(),1);
  _mapper.toLast();
}
/* Future functionality
void ScreenControl::previewForm()
{
}

void ScreenControl::previewList()
{
}

void ScreenControl::print()
{
}

void ScreenControl::printForm()
{
}

void ScreenControl::printList()
{
}
*/
void ScreenControl::save()
{
  _mapper.submit();
  _model.submitAll();
  if (_mode==New)
    newRow();
}

void ScreenControl::search()
{
  if (_searchText->text().stripWhiteSpace().length())
  {
    QString filter="((true) ";
    for (int i = 0; i < _model.columnCount(); i++)
    { 
      if (_model.headerData(i,Qt::Horizontal,Qt::DisplayRole).toString().length())
      {
	filter += "AND (";
	filter += _model.headerData(i,Qt::Horizontal,Qt::DisplayRole).toString();
	filter += " ~ '";
	filter += _searchText->text();
	filter += "')";
      }	
    }
    filter += ")";
    setFilter(filter);
  }
  select();
}

void ScreenControl::select()
{
  _model.select();
  if (_model.rowCount())
  {
    _mapper.toFirst();
    _new->setEnabled(true);
    _save->setEnabled(true);
    _view->setEnabled(true);
    _next->setEnabled(_model.rowCount() > 1);
  }
  else
  {
    _new->setEnabled(false);
    _view->setEnabled(false);
    _next->setEnabled(false);
    _prev->setEnabled(false);
    _next->setEnabled(false);
  }
}

/* Future functionality
void ScreenControl::setAutoSave(bool p)
{
  _autoSave=p;
  _save->setVisible(p);
}
*/ 

void ScreenControl::setMode(Modes p)
{
  if (p==New)
  {
    _mode=New;
    newRow();
  }
  else if (p==Edit)
  {
    _mode=Edit;
  }
  else
  {
    _mode=View;
    //to do: loop through each field, see if the is a widget     _mapper.mappedWidgetAt(int)  and disable
  }
    
}

void ScreenControl::setSearchType(SearchTypes p)
{
  _searchType=p;
  //to do: set button to change label between list and query
}

/* Pass in a schema name and a table name.  If schema name is blank, it will be ignored.
     If there is a table name to set, data widget mappings will be set as well. */
void ScreenControl::setTable(QString s, QString t)
{
  if (t.length())
  {
    QString tableName="";
    if (s.length())
      tableName=s + ".";
    tableName+=t;
    if (_model.tableName() != tableName)
    {
      _model.setTable(tableName);
      setDataWidgetMapper(&_model);
      _search->setEnabled(true);
      _searchText->setEnabled(true);
    }
  }
  else
  {
    _search->setEnabled(false);
    _searchText->setEnabled(false);
  }
}

void ScreenControl::setDataWidgetMapper(QSqlTableModel *p)
{
  _mapper.setSqlTableModel(&_model);
  emit newDataWidgetMapper(&_mapper);
}





