/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <parameter.h>
#include <xsqlquery.h>

#include <QtScript>
#include <QMessageBox>

#include "parameterwidget.h"
#include "widgets.h"
#include "xcombobox.h"
#include "usernamecluster.h"
#include "datecluster.h"
#include "crmacctcluster.h"
#include "filterManager.h"
#include "contactcluster.h"
#include "filtersave.h"

/* TODO: - add Flag to the set of types to handle parameters that are either
           passed or not (this is different from boolean parameters, that can
           be either true or false).
         - add XComboBoxCode to the set of types to handle parameters that
           require the code value from an xcombobox instead of the id.
         - store the text value of the enum in the filter definition, not the
           integer. otherwise the db contents will prevent reorganizing the
           order of the symbolic values of the type enumeration.
 */

ParameterWidget::ParameterWidget(QWidget *pParent, const char *pName)  :
    QWidget(pParent)
{
  setupUi(this);


  if(pName)
    setObjectName(pName);

  _initialized = false;
  _shared = false;
  setSavedFilters(-1);
  _filterSignalMapper = new QSignalMapper(this);
  _saveButton->setDisabled(true);

  connect(_addFilterRow, SIGNAL(clicked()), this, SLOT( addParam() ) );
  connect(_filterButton, SIGNAL(clicked()), this, SLOT( setFiltersVisabiltyPreference() ) );
  connect(_filterSignalMapper, SIGNAL(mapped(int)), this, SLOT( removeParam(int) ));
  connect(_saveButton, SIGNAL(clicked()), this, SLOT( save() ) );
  connect(_manageButton, SIGNAL(clicked()), this, SLOT( sManageFilters() ) );
  connect(_filterList, SIGNAL(currentIndexChanged(int)), this, SLOT( applySaved(int) ) );
  connect(this, SIGNAL(updated()), this, SLOT( toggleSave() ) );
}

void ParameterWidget::showEvent(QShowEvent * event)
{
  if(_initialized)
    return;

  QString pname;
  if(window())
    pname = window()->objectName() + "/";
  _settingsName = pname + objectName();

  if(_x_preferences)
  {
    if (_x_preferences->value(_settingsName + "/checked") == "f")
    {
      _filterGroup->setVisible(false);
      _filterButton->setChecked(false);
    }
    else
    {
      _filterGroup->setVisible(true);
      _filterButton->setChecked(true);
    }
  }
  _initialized = true;
  QWidget::showEvent(event);
}

void ParameterWidget::appendValue(ParameterList &pParams)
{
  QMapIterator<int, QPair<QString, QVariant> > i(_filterValues);
  while (i.hasNext())
  {
    i.next();
    QPair<QString, QVariant> tempPair = i.value();
    if (pParams.inList(tempPair.first))
    {
      pParams.remove(tempPair.first);
    }

    pParams.append(tempPair.first, tempPair.second);
  }
}


void ParameterWidget::applyDefaultFilterSet()
{
  XSqlQuery qry;
  const QMetaObject *metaobject;
  QString classname;
  QString filter_name;
  int filter_id;

  //hides parameterwidget when it's embedded within another widget with a parent
  if (this->parent() && this->parent()->parent())
  {
    clearFilters();
    this->hide();

    return;
  }

  QString query = "SELECT filter_id, filter_name "
                  "FROM filter "
                  "WHERE filter_screen=:screen "
                  " AND filter_username=current_user "
                  " AND filter_selected=TRUE";

  if (this->parent())
  {
    metaobject = this->parent()->metaObject();
    classname = metaobject->className();
    qry.prepare(query);
    qry.bindValue(":screen", classname);

    qry.exec();

    if (qry.first())
    {
      filter_id = qry.value("filter_id").toInt();
      filter_name = qry.value("filter_name").toString();
      setSavedFiltersIndex(filter_name);
      applySaved(0, filter_id);
    }
    else
    {
      addParam();
    }

  }

}

void ParameterWidget::addParam()
{
  XComboBox *xcomboBox = new XComboBox(_filterGroup);
  QToolButton *toolButton = new QToolButton(_filterGroup);
  QLineEdit *lineEdit = new QLineEdit(_filterGroup);
  QGridLayout *gridLayout = new QGridLayout();
  QVBoxLayout *xcomboLayout = new QVBoxLayout();
  QHBoxLayout *widgetLayout1 = new QHBoxLayout();
  QVBoxLayout *widgetLayout2 = new QVBoxLayout();
  QVBoxLayout *buttonLayout = new QVBoxLayout();

  int nextRow = _filtersLayout->rowCount();
  QString currRow = QString().setNum(nextRow);

  // Set up objects
  gridLayout->setObjectName("topLayout" + currRow);

  xcomboLayout->setObjectName("xcomboLayout" + currRow);
  xcomboLayout->setContentsMargins(0, 0, 0, 0);

  xcomboBox->setObjectName("xcomboBox" + currRow);
  xcomboBox->addItem("", currRow + ":" + "2");

  widgetLayout1->setObjectName("widgetLayout1" + currRow);

  widgetLayout2->setObjectName("widgetLayout2" + currRow);
  widgetLayout2->setContentsMargins(0, 0, 0, 0);

  lineEdit->setObjectName("widget" + currRow);
  lineEdit->setDisabled(true);

  buttonLayout->setObjectName("buttonLayout" + currRow);
  buttonLayout->setContentsMargins(0, 0, 0, 0);

  toolButton->setObjectName("button" + currRow);
  toolButton->setText(tr("-"));

  //grab the items provided by other widgets to populate xcombobox with
  QMapIterator<QString, QPair<QString, ParameterWidgetTypes> > i(_types);
  while (i.hasNext())
  {
    i.next();
    QPair<QString, ParameterWidgetTypes> tempPair = i.value();
    QString value = QString().setNum(nextRow) + ":" + QString().setNum(tempPair.second);
    if ( _usedTypes.isEmpty() || !containsUsedType(i.key()) )
      xcomboBox->addItem(i.key(), value );
  }

  xcomboLayout->addWidget(xcomboBox);
  xcomboLayout->addItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

  // Place the default line edit/button combo
  widgetLayout1->addWidget(lineEdit);
  widgetLayout1->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Fixed));

  widgetLayout2->addLayout(widgetLayout1);
  widgetLayout2->addItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

  gridLayout->addLayout(widgetLayout2, 0, 0, 1, 1);

  // Place Button
  buttonLayout->addWidget(toolButton);
  buttonLayout->addItem(new QSpacerItem(10, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

  gridLayout->addLayout(buttonLayout, 0, 1, 1, 1);

  _filtersLayout->addLayout(gridLayout, nextRow, 1, 1, 1);
  _filtersLayout->addLayout(xcomboLayout, nextRow, 0, 1, 1);

  // Hook up connections
  connect(toolButton, SIGNAL(clicked()), _filterSignalMapper, SLOT(map()));
  connect(toolButton, SIGNAL(clicked()), gridLayout, SLOT( deleteLater() ) );
  connect(toolButton, SIGNAL(clicked()), xcomboBox, SLOT( deleteLater() ) );
  connect(toolButton, SIGNAL(clicked()), lineEdit, SLOT( deleteLater() ) );
  connect(toolButton, SIGNAL(clicked()), toolButton, SLOT( deleteLater() ) );
  connect(xcomboBox, SIGNAL(currentIndexChanged(int)), this, SLOT( changeFilterObject(int)) );
  connect(lineEdit, SIGNAL(editingFinished()), this, SLOT( storeFilterValue() ) );

  _filterSignalMapper->setMapping(toolButton, nextRow);

  _addFilterRow->setDisabled(true);
}

void ParameterWidget::applySaved(int pId, int filter_id)
{
  qDebug() << "in applySaved, pid is: " << pId;
  QGridLayout *container;
  QLayoutItem *child;
  QLayoutItem *child2;
  QHBoxLayout *layout2;
  QWidget *found;
  QDate tempdate;
  XSqlQuery qry;
  QString query;
  QString filterValue;
  int xid;

  QMapIterator<int, QPair<QString, QVariant> > j(_filterValues);

  clearFilters();

  if (!parent())
    return;

  //if (pId == 0)
  //addParam();

  if (_filterList->id() == -1)
  {
    setSelectedFilter(-1);
    emit updated();
    return;
  }

  if (filter_id == 0 && _filterList->id() != -1)
    filter_id = _filterList->id(_filterList->currentIndex());

  const QMetaObject *metaobject = this->parent()->metaObject();
  QString classname(metaobject->className());

  //look up filter from database
  query = " SELECT filter_value, "
          "  CASE WHEN (filter_username IS NULL) THEN true "
          "  ELSE false END AS shared "
          " FROM filter "
          " WHERE filter_id=:id ";

  qry.prepare(query);
  qry.bindValue(":id", filter_id );

  qry.exec();

  if (qry.first())
  {
    filterValue = qry.value("filter_value").toString();
    _shared = qry.value("shared").toBool();
  }

  QStringList filterRows = filterValue.split("|");
  QString tempFilter = QString();

  int windowIdx = _filtersLayout->rowCount();

  for (int i = 0; i < filterRows.size(); ++i)
  {
    tempFilter = filterRows[i];
    if ( !(tempFilter.isEmpty()) )
    {
      //0 is filterType, 1 is filterValue, 2 is parameterwidgettype
      QStringList tempFilterList = tempFilter.split(":");
      this->addParam();

      QLayoutItem *test = _filtersLayout->itemAtPosition(windowIdx, 0)->layout()->itemAt(0);
      XComboBox *mybox = (XComboBox*)test->widget();

      QString key = this->getParameterTypeKey(tempFilterList[0]);
      int idx = mybox->findText(key);

      mybox->setCurrentIndex(idx);

      QString row;
      row = row.setNum(windowIdx);

      container = _filtersLayout->findChild<QGridLayout *>("topLayout" + row);
      child = container->itemAtPosition(0, 0)->layout()->itemAt(0);
      layout2 = (QHBoxLayout *)child->layout();
      child2 = layout2->itemAt(0);
      found = child2->widget();

      int widgetType = tempFilterList[2].toInt();

      //grab pointer to newly created filter object
      switch (widgetType)
      {
      case Date:
        DLineEdit *dLineEdit;
        dLineEdit = (DLineEdit*)found;
        dLineEdit->setDate(QDate::fromString(tempFilterList[1], "yyyy-MM-dd"), true);
        break;
      case User:
        UsernameCluster *usernameCluster;
        usernameCluster = (UsernameCluster*)found;
        usernameCluster->setUsername(tempFilterList[1]);
        break;
      case Crmacct:
        CRMAcctCluster *crmacctCluster;
        crmacctCluster = (CRMAcctCluster*)found;
        crmacctCluster->setId(tempFilterList[1].toInt());
        break;
      case Contact:
        ContactCluster *contactCluster;
        contactCluster = (ContactCluster*)found;
        contactCluster->setId(tempFilterList[1].toInt());
        break;
      case XComBox:
        XComboBox *xBox;
        xBox = (XComboBox*)found;

        //fix for setid not emitting id signal if id found for filter is first in list
        //set to any other valid id first to fix it
        xBox->setId(2);

        xid = tempFilterList[1].toInt();
        qDebug() << "xid is: " << xid;
        xBox->setId(xid);
        while (j.hasNext()) {
          j.next();
          QPair<QString, QVariant> tempPair = j.value();
          qDebug() << j.key() << ": " << tempPair.second << endl;
        }
        break;
      default:
        QLineEdit *lineEdit;
        lineEdit = (QLineEdit*)found;
        lineEdit->setText(tempFilterList[1]);
        storeFilterValue(-1, lineEdit);
        break;
      }
    }//end of if
    windowIdx++;
  }//end of for

  setSelectedFilter(filter_id);
  emit updated();
}

void ParameterWidget::changeFilterObject(int index)
{
  XComboBox *mybox = (XComboBox *)sender();
  QStringList split = mybox->itemData(index).toString().split(":");
  QString row = split.at(0);
  int type = split.at(1).toInt();
  XSqlQuery qry;

  QWidget *widget = _filterGroup->findChild<QWidget *>("widget" + row);
  QWidget *button = _filterGroup->findChild<QToolButton *>("button" + row);
  QHBoxLayout *layout = _filterGroup->findChild<QHBoxLayout *>("widgetLayout1" + row);;

  QPair<QString, ParameterWidgetTypes> tempPair;


  if (widget && layout && button)
    delete widget;
  else
    return;

  QWidget *newWidget = 0;
  switch (type)
  {
  case Date:
    {
      DLineEdit *dLineEdit= new DLineEdit(_filterGroup);
      newWidget = dLineEdit;
      connect(dLineEdit, SIGNAL(newDate(QDate)), this, SLOT( storeFilterValue(QDate) ) );
    }
    break;
  case User:
    {
      UsernameCluster *usernameCluster = new UsernameCluster(_filterGroup);
      newWidget = usernameCluster;
      usernameCluster->setNameVisible(false);
      usernameCluster->setDescriptionVisible(false);
      usernameCluster->setLabel("");

      connect(button, SIGNAL(clicked()), usernameCluster, SLOT( deleteLater() ) );
      connect(usernameCluster, SIGNAL(newId(int)), this, SLOT( storeFilterValue(int) ) );
    }
    break;
  case Crmacct:
    {
      CRMAcctCluster *crmacctCluster = new CRMAcctCluster(_filterGroup);
      newWidget = crmacctCluster;
      crmacctCluster->setNameVisible(false);
      crmacctCluster->setDescriptionVisible(false);
      crmacctCluster->setLabel("");

      connect(button, SIGNAL(clicked()), crmacctCluster, SLOT( deleteLater() ) );
      connect(crmacctCluster, SIGNAL(newId(int)), this, SLOT( storeFilterValue(int) ) );
    }
    break;
  case Contact:
    {
      ContactCluster *contactCluster = new ContactCluster(_filterGroup);
      newWidget = contactCluster;
      contactCluster->setDescriptionVisible(false);
      contactCluster->setLabel("");

      connect(button, SIGNAL(clicked()), contactCluster, SLOT( deleteLater() ) );
      connect(contactCluster, SIGNAL(newId(int)), this, SLOT( storeFilterValue(int) ) );
    }
    break;
  case XComBox:
    {
      XComboBox *xBox = new XComboBox(_filterGroup);
      newWidget = xBox;

      xBox->setType(_comboTypes[mybox->currentText()]);
      if (_comboTypes[mybox->currentText()] == XComboBox::Adhoc)
      {
        qry.prepare( _comboQuery[mybox->currentText()] );

        qry.exec();
        xBox->populate(qry);
      }
      connect(button, SIGNAL(clicked()), xBox, SLOT( deleteLater() ) );
      connect(xBox, SIGNAL(newID(int)), this, SLOT( storeFilterValue(int) ) );
    }
    break;

  default:
    {
      QLineEdit *lineEdit = new QLineEdit(_filterGroup);
      newWidget = lineEdit;

      connect(button, SIGNAL(clicked()), lineEdit, SLOT( deleteLater() ) );
      connect(lineEdit, SIGNAL(editingFinished()), this, SLOT( storeFilterValue() ) );
    }
    break;
  }

  if (newWidget)
  {
    connect(button, SIGNAL(clicked()), newWidget, SLOT( deleteLater() ) );
    newWidget->setObjectName("widget" + row);
    layout->insertWidget(0, newWidget);
  }
}

void ParameterWidget::clearFilters()
{
  QStringList del;
  QList<QLayout *> llist = _filterGroup->findChildren<QLayout*>();
  QList<QWidget *> wlist = _filterGroup->findChildren<QWidget*>();

  // Find layout deletion candidates first
  for (int i = 0; i < llist.count(); i++)
  {
    if (llist.at(i)->objectName().startsWith("topLayout") ||
        llist.at(i)->objectName().startsWith("widgetLayout2") ||
        llist.at(i)->objectName().startsWith("buttonLayout"))
      del.append(llist.at(i)->objectName());
  }

  // Then find widget deletion candidates
  for (int i = 0; i < wlist.count(); i++)
  {
    if (wlist.at(i)->objectName().startsWith("xcombo") ||
        wlist.at(i)->objectName().startsWith("widget") ||
        wlist.at(i)->objectName().startsWith("button"))
      del.append(wlist.at(i)->objectName());
  }

  // Perform deletions
  while (del.count())
  {
    QObject *obj = _filterGroup->findChild<QObject*>(del.at(0));
    delete obj;
    del.takeFirst();
  }

  _filterValues.clear();
  _usedTypes.clear();
  _addFilterRow->setDisabled(false);
}


ParameterList ParameterWidget::parameters()
{
  ParameterList params;

  appendValue(params);

  return params;
}

void ParameterWidget::repopulateComboboxes()
{
  QRegExp rx("^xcomboBox");

  QList<XComboBox *> xlist = _filterGroup->findChildren<XComboBox*>(rx);
  QMapIterator<QString, QPair<QString, ParameterWidgetTypes> > j(_types);
  QString current;
  QString value;
  int idx;
  for (int i = 0; i < xlist.count(); i++)
  {
    current = xlist.at(i)->currentText();
    QStringList split = xlist.at(i)->itemData(xlist.at(i)->currentIndex()).toString().split(":");
    disconnect(xlist.at(i), 0, this, 0);
    xlist.at(i)->clear();
    while (j.hasNext())
    {
      j.next();
      QPair<QString, ParameterWidgetTypes> tempPair = j.value();
      value = split[0] + ":" + QString().setNum(tempPair.second);
      if ( !containsUsedType(j.key()) || current == j.key() )
      {
        xlist.at(i)->addItem(j.key(), value );
      }
    }
    j.toFront();

    idx = xlist.at(i)->findText(current);
    xlist.at(i)->setCurrentIndex(idx);
    connect(xlist.at(i), SIGNAL(currentIndexChanged(int)), this, SLOT( changeFilterObject(int)) );   
  }
}


void ParameterWidget::removeParam(int pRow)
{
  QLayoutItem *test;
  QLayoutItem *test2;

  test = _filtersLayout->itemAtPosition(pRow, 0)->layout()->itemAt(0);
  XComboBox *mybox = (XComboBox*)test->widget();

  QVariant filterVar(mybox->itemData(mybox->currentIndex()));
  QString filterType = filterVar.toString();
  QStringList split = filterType.split(":");

  QPair<QString, QVariant> tempPair = _filterValues.value(split[0].toInt());

  _filterValues.remove(split[0].toInt());


  test2 = _filtersLayout->itemAtPosition(pRow, 0)->layout()->takeAt(1);
  delete test2;
  test2 = 0;
  _filtersLayout->update();
  _addFilterRow->setDisabled(false);
  repopulateComboboxes();
  if (!mybox->currentText().isEmpty())
    _usedTypes.remove(pRow);
  emit updated();
}

void ParameterWidget::save()
{
  QString filter;
  QString variantString;
  QString username;
  QString query;
  QVariant tempVar;
  int filter_id;

  QMapIterator<int, QPair<QString, QVariant> > i(_filterValues);
  while (i.hasNext())
  {
    i.next();
    QPair<QString, QVariant> tempPair = i.value();

    tempVar = tempPair.second;

    QLayoutItem *test = _filtersLayout->itemAtPosition(i.key(), 0)->layout()->itemAt(0);
    XComboBox* mybox = (XComboBox*)test->widget();
    QStringList split = mybox->itemData(mybox->currentIndex()).toString().split(":");

    if ( tempVar.canConvert<QString>() )
    {
      variantString = tempVar.toString();
      filter = filter + tempPair.first + ":" + variantString + ":" + split[1] + "|";
    }
  }

  const QMetaObject *metaobject = this->parent()->metaObject();
  QString classname(metaobject->className());

  ParameterList params;
  params.append("filter", filter);
  if (_filterList->id())
    params.append("filtersetname", _filterList->currentText());
  params.append("classname", classname);
  if (_shared)
    params.append("shared", true);

  filterSave newdlg(this);
  newdlg.set(params);
  filter_id = newdlg.exec();
  if (!filter_id)
    return;
  emit filterSetSaved();

  setSavedFilters();
  _filterList->setId(filter_id);
}

void ParameterWidget::setFiltersVisabiltyPreference()
{
  QString pname;
  if(window())
    pname = window()->objectName() + "/";
  _settingsName = pname + this->objectName();

  if (!_settingsName.isEmpty() && _x_preferences)
  {
    _x_preferences->set(_settingsName + "/checked", _filterGroup->isVisible());
  }
}
void ParameterWidget::setSavedFilters(int defaultId)
{
  QString query;
  XSqlQuery qry;

  if(this->parent())
  {
    const QMetaObject *metaobject = this->parent()->metaObject();
    QString classname(metaobject->className());

    query = " SELECT 0 AS filter_id, :none AS filter_name, 1 AS seq "
            " UNION "
            " SELECT filter_id, filter_name, 2 AS seq "
            " FROM filter "
            " WHERE COALESCE(filter_username,current_user)=current_user "
            " AND filter_screen=:screen "
            " ORDER BY seq, filter_name ";

    qry.prepare(query);

    qry.bindValue(":screen", classname);
    qry.bindValue(":none", tr("None"));
    qry.exec();
    if (defaultId)
      _filterList->populate(qry, defaultId);
    else
      _filterList->populate(qry, 0);
  }
}

void ParameterWidget::setSavedFiltersIndex(QString filterSetName)
{
  _filterList->findText(filterSetName);
  _filterList->setText(filterSetName);
}

void ParameterWidget::setType(QString pName, QString pParam, ParameterWidgetTypes type)
{
  _types[pName] = qMakePair(pParam, type);
}

void ParameterWidget::setXComboBoxType(QString pName, QString pParam, XComboBox::XComboBoxTypes xType)
{
  _comboTypes[pName] = xType;
  _types[pName] = qMakePair(pParam, XComBox);
}

void ParameterWidget::setXComboBoxType(QString pName, QString pParam, QString pQry)
{
  _comboTypes[pName] = XComboBox::Adhoc;
  _comboQuery[pName] = pQry;
  _types[pName] = qMakePair(pParam, XComBox);

}
void ParameterWidget::sManageFilters()
{
  ParameterList params;
  params.append("screen", parent()->metaObject()->className());

  filterManager *newdlg = new filterManager(this, "");
  newdlg->set(params);
  newdlg->exec();
}

void ParameterWidget::toggleSave()
{
  if (!_filterValues.isEmpty())
  {
    _saveButton->setDisabled(false);
  }
  else
  {
    _saveButton->setDisabled(true);
  }
}
void ParameterWidget::storeFilterValue(QDate date)
{
  QObject *filter = (QObject *)sender();
  QLayoutItem *test;
  QLayoutItem *test2;
  QLayoutItem *child;
  QLayoutItem *child2;
  QGridLayout *layout;
  QHBoxLayout *layout2;
  QWidget *found;
  XComboBox *mybox;
  int foundRow = 0;

  for (int i = 1; i < _filtersLayout->rowCount(); i++)
  {
    test = _filtersLayout->itemAtPosition(i, 1);
    if (test)
    {
      layout = (QGridLayout *)test->layout();
      child =layout->itemAtPosition(0, 0);
      layout2 = (QHBoxLayout *)child->layout()->itemAt(0);
      child2 = layout2->itemAt(0);
      found = child2->widget();

      if (found == filter )
        foundRow = i;
    }
  }

  test2 = _filtersLayout->itemAtPosition(foundRow, 0)->layout()->itemAt(0);
  mybox = (XComboBox*)test2->widget();
  QString currText = mybox->currentText();
  QPair<QString, ParameterWidgetTypes> tempPair = _types[currText];

  _filterValues[foundRow] = qMakePair(tempPair.first, QVariant(date));
  //if (!mybox->currentText().isEmpty())
  //{
  //_usedTypes->removeAll(mybox->currentText());
  
  if (!_usedTypes.isEmpty())
    _usedTypes.remove(foundRow);

  _usedTypes[foundRow] = mybox->currentText();
  _addFilterRow->setDisabled(false);
  repopulateComboboxes();

  emit updated();
}

//stores the value of a filter object into the filtervalues map
void ParameterWidget::storeFilterValue(int pId, QObject* filter)
{
  if (!filter)
  {
    filter = (QObject *)sender();
  }

  QLayoutItem *test;
  QLayoutItem *test2;
  QLayoutItem *child;
  QLayoutItem *child2;
  QGridLayout *layout;
  QHBoxLayout *layout2;
  QWidget *found;
  XComboBox *mybox;
  int foundRow = 0;

  for (int i = 1; i < _filtersLayout->rowCount(); i++)
  {

    test = _filtersLayout->itemAtPosition(i, 1);
    if (test)
    {
      layout = (QGridLayout *)test->layout();
      child = layout->itemAtPosition(0, 0);
      layout2 = (QHBoxLayout *)child->layout()->itemAt(0);
      child2 = layout2->itemAt(0);

      found = child2->widget();

      if (found == filter )
      {
        foundRow = i;
      }
    }
  }


  test2 = _filtersLayout->itemAtPosition(foundRow, 0)->layout()->itemAt(0);
  mybox = (XComboBox*)test2->widget();
  QString _currText = mybox->currentText();
  QPair<QString, ParameterWidgetTypes> tempPair = _types[_currText];

  const QMetaObject *metaobject = filter->metaObject();
  QString classname(metaobject->className());

  if (pId == -1)
  {
    if (classname == "QLineEdit")
    {
      QLineEdit *lineEdit = (QLineEdit *)filter;
      _filterValues[foundRow] = qMakePair(tempPair.first, QVariant(lineEdit->text()));
      emit updated();
    }
  }
  else
  {
    if (classname == "UsernameCluster")
    {
      UsernameCluster *usernameCluster = (UsernameCluster *)filter;
      QString username = usernameCluster->username();
      _filterValues[foundRow] = qMakePair(tempPair.first, QVariant(username));
      emit updated();
    }
    else
    {
      _filterValues[foundRow] = qMakePair(tempPair.first, QVariant(pId));
      emit updated();
    }
  }


  if (!_usedTypes.isEmpty())
    _usedTypes.remove(foundRow);
  _usedTypes[foundRow] = mybox->currentText();

  _addFilterRow->setDisabled(false);
  repopulateComboboxes();

}

QString ParameterWidget::getParameterTypeKey(QString pValue)
{
  QMapIterator<QString, QPair<QString, ParameterWidgetTypes> > i(_types);
  while (i.hasNext())
  {
    i.next();
    QPair<QString, ParameterWidgetTypes> tempPair = i.value();

    if (pValue == tempPair.first)
      return i.key();
  }

  return QString();
}


//updates selected filter set to be the default upon loading of the current screen
//for the current user
void ParameterWidget::setSelectedFilter(int filter_id)
{
  XSqlQuery qry;
  const QMetaObject *metaobject = this->parent()->metaObject();
  QString classname(metaobject->className());

  QString query = "UPDATE filter SET filter_selected=false "
                  "WHERE filter_screen=:screen AND filter_username=current_user";
  QString query2 = "UPDATE filter SET filter_selected=true "
                   "WHERE filter_screen=:screen AND filter_id=:id AND filter_username=current_user";
  qry.prepare(query);
  qry.bindValue(":screen", classname);

  qry.exec();

  qry.prepare(query2);
  qry.bindValue(":screen", classname);
  qry.bindValue(":id", filter_id);

  qry.exec();
}

bool ParameterWidget::containsUsedType(QString value)
{
  QMapIterator<int, QString > i(_usedTypes);
  while (i.hasNext()) {
    i.next();
    if (i.value() == value)
      return true;
  }

  return false;
}

// script exposure ///////////////////////////////////////////////////////////

void ParameterWidgetfromScriptValue(const QScriptValue &obj, ParameterWidget* &item)
{
  item = qobject_cast<ParameterWidget*>(obj.toQObject());
}

QScriptValue ParameterWidgettoScriptValue(QScriptEngine *engine, ParameterWidget* const &item)
{
  return engine->newQObject(item);
}

QScriptValue constructParameterWidget(QScriptContext *context,
                                      QScriptEngine  *engine)
{
  ParameterWidget *obj = 0;
  if (context->argumentCount() == 1 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)))
    obj = new ParameterWidget(qscriptvalue_cast<QWidget*>(context->argument(0)));
  else if (context->argumentCount() >= 2 &&
           qscriptvalue_cast<QWidget*>(context->argument(0)))
    obj = new ParameterWidget(qscriptvalue_cast<QWidget*>(context->argument(0)),
                              context->argument(1).toString());

  return engine->toScriptValue(obj);
}

void setupParameterWidget(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, ParameterWidgettoScriptValue, ParameterWidgetfromScriptValue);

  QScriptValue widget = engine->newFunction(constructParameterWidget);

  widget.setProperty("Crmacct", QScriptValue(engine, ParameterWidget::Crmacct), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("User", QScriptValue(engine, ParameterWidget::User), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Text", QScriptValue(engine, ParameterWidget::Text), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Date", QScriptValue(engine, ParameterWidget::Date), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("XComBox", QScriptValue(engine, ParameterWidget::XComBox), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Contact", QScriptValue(engine, ParameterWidget::Contact), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("ParameterWidget", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
