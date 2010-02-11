/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <parameter.h>
#include <xsqlquery.h>
#include "parameterwidget.h"
#include "widgets.h"
#include "xcombobox.h"
#include "usernamecluster.h"
#include "datecluster.h"
#include "crmacctcluster.h"
#include "filterManager.h"

ParameterWidget::ParameterWidget(QWidget *pParent, const char *pName)  :
  QWidget(pParent)
{
	
	if(pName)
	{
		setObjectName(pName);
	}

	_filterButton = new QPushButton("Hide Filters", this);

	_filterList = new XComboBox(this);
	
	QSpacerItem *horizontalSpacer =  new QSpacerItem(379, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

	this->setSavedFilters(-1);

	_hboxLayout = new QHBoxLayout();
	_hboxLayout->addWidget(_filterButton);
	_hboxLayout->addWidget(_filterList);
	_hboxLayout->addItem(horizontalSpacer);

	_filterSignalMapper = new QSignalMapper(this);

	this->addFilters();

	connect(_filterButton, SIGNAL(clicked()), this, SLOT(changeFilterButton()));
	connect(addFilterRow, SIGNAL(clicked()), this, SLOT( addParam() ) );
	connect(_filterSignalMapper, SIGNAL(mapped(int)), this, SLOT( removeParam(int) ));
	connect(_saveButton, SIGNAL(clicked()), this, SLOT( save() ) );
	connect(_manageButton, SIGNAL(clicked()), this, SLOT( sManageFilters() ) );
	connect(_filterList, SIGNAL(currentIndexChanged(int)), this, SLOT( applySaved(int) ) );
}

void ParameterWidget::addFilters()
{
    QGridLayout *gridLayout;
    QGridLayout *gridLayout_5;
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QVBoxLayout *verticalLayout_3;
    QLabel *label_3;
    QGridLayout *gridLayout_3;

	int nextRow;
	QString currRow;

	nextRow = 1;
	currRow = currRow.setNum(nextRow);

	 QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
     sizePolicy.setHorizontalStretch(0);
     sizePolicy.setVerticalStretch(0);
     sizePolicy.setHeightForWidth(this->sizePolicy().hasHeightForWidth());
     this->setSizePolicy(sizePolicy);
	 _groupBox = new QGroupBox(this);
     gridLayout = new QGridLayout(_groupBox);
     gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
     gridLayout_5 = new QGridLayout();
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        _window = new QGridLayout();
        _window->setObjectName(QString::fromUtf8("_window"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label = new QLabel(_groupBox);
        label->setObjectName(QString::fromUtf8("label"));
        label->setMaximumSize(QSize(16777215, 18));

        verticalLayout->addWidget(label);

        _window->addLayout(verticalLayout, 0, 0, 1, 1);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        label_3 = new QLabel(_groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        verticalLayout_3->addWidget(label_3);

        _window->addLayout(verticalLayout_3, 0, 1, 1, 1);


        gridLayout_5->addLayout(_window, 0, 0, 1, 1);

        gridLayout_3 = new QGridLayout();
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        addFilterRow = new QPushButton(_groupBox);
        addFilterRow->setObjectName(QString::fromUtf8("addFilterRow"));
        addFilterRow->setMaximumSize(QSize(16777215, 23));

        gridLayout_3->addWidget(addFilterRow, 0, 0, 1, 1);

        _filterSetName = new QLineEdit(_groupBox);
        _filterSetName->setObjectName(QString::fromUtf8("_filterSetName"));
        _filterSetName->setMaximumSize(QSize(16777215, 23));

        gridLayout_3->addWidget(_filterSetName, 0, 1, 1, 1);

        _saveButton = new QPushButton(_groupBox);
        _saveButton->setObjectName(QString::fromUtf8("_saveButton"));
        _saveButton->setMaximumSize(QSize(16777215, 23));

        gridLayout_3->addWidget(_saveButton, 0, 2, 1, 1);

        _manageButton = new QPushButton(_groupBox);
        _manageButton->setObjectName(QString::fromUtf8("_manageButton"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(_manageButton->sizePolicy().hasHeightForWidth());
        _manageButton->setSizePolicy(sizePolicy2);
        _manageButton->setMaximumSize(QSize(134, 23));

        gridLayout_3->addWidget(_manageButton, 0, 3, 1, 1);

        gridLayout_5->addLayout(gridLayout_3, 1, 0, 1, 1);
        gridLayout->addLayout(gridLayout_5, 1, 0, 1, 1);

		_groupBox->setTitle(QApplication::translate("ParameterWidget", "Filters", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("ParameterWidget", "Filter By", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("ParameterWidget", "Value", 0, QApplication::UnicodeUTF8));
        addFilterRow->setText(QApplication::translate("ParameterWidget", "Add Filter", 0, QApplication::UnicodeUTF8));
        _saveButton->setText(QApplication::translate("ParameterWidget", "Save", 0, QApplication::UnicodeUTF8));
        _manageButton->setText(QApplication::translate("ParameterWidget", "Manage Filter Sets", 0, QApplication::UnicodeUTF8));

	_groupBox->setLayout(gridLayout);

	vbox = new QVBoxLayout(this);
    vbox->addLayout(_hboxLayout);
	vbox->addWidget(_groupBox);
}


void ParameterWidget::appendValue(ParameterList &pParams)
{
	QMapIterator<int, QPair<QString, QVariant>> i(_filterValues);
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

	QString query = "SELECT filter_id, filter_name FROM filter WHERE filter_screen=:screen AND filter_username=current_user AND filter_selected=TRUE";

	if (this->parent() != NULL)
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
			this->setSavedFiltersIndex(filter_name);
			this->applySaved(0, filter_id);
		}

	}

}

void ParameterWidget::addParam()
{
	XComboBox *xcomboBox;
	QPushButton *pushButton_2;
	QLineEdit *lineEdit;
	QGridLayout *gridLayout_4;
	QVBoxLayout *verticalLayout;
	QVBoxLayout *verticalLayout2;
	QVBoxLayout *verticalLayout3;
	QSpacerItem *verticalSpacer;
	QSpacerItem *verticalSpacer2;
	QSpacerItem *verticalSpacer3;

	int nextRow;
	QString currRow;

	nextRow = _window->rowCount();
	currRow = currRow.setNum(nextRow);

	verticalLayout = new QVBoxLayout();
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
	verticalLayout->setContentsMargins(0, 0, 0, 0);
	verticalLayout2 = new QVBoxLayout();
    verticalLayout2->setObjectName(QString::fromUtf8("verticalLayout2"));
	verticalLayout2->setContentsMargins(0, 0, 0, 0);
	
	verticalLayout3 = new QVBoxLayout();
    verticalLayout3->setObjectName(QString::fromUtf8("verticalLayout3"));
	verticalLayout3->setContentsMargins(0, 0, 0, 0);

	xcomboBox = new XComboBox(_groupBox);
    xcomboBox->setObjectName(QString::fromUtf8("xcomboBox" + currRow));
    QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Fixed);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(xcomboBox->sizePolicy().hasHeightForWidth());
    xcomboBox->setSizePolicy(sizePolicy1);
    xcomboBox->setMaximumSize(QSize(16777215, 20));

	xcomboBox->addItem("", currRow + ":" + "2");

	//grab the items provided by other widgets to populate xcombobox with
	QMapIterator<QString, QPair<QString, ParameterWidgetTypes>> i(_types);
	while (i.hasNext())
	{
		i.next();
		QPair<QString, ParameterWidgetTypes> tempPair = i.value();
		QString _value;
		_value = _value.setNum(nextRow) + ":" + _value.setNum(tempPair.second);
		xcomboBox->addItem(i.key(), _value );
	}

	 verticalLayout->addWidget(xcomboBox);
     verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
     verticalLayout->addItem(verticalSpacer);

	
	//then create the default line edit/button combo
	gridLayout_4 = new QGridLayout();
	gridLayout_4->setObjectName(QString::fromUtf8("gridLayout" + currRow));
    lineEdit = new QLineEdit(_groupBox);
    lineEdit->setObjectName(QString::fromUtf8("lineEdit" + currRow));

	QHBoxLayout *horizontalLayout = new QHBoxLayout();
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
	QSpacerItem *horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

	horizontalLayout->addWidget(lineEdit);
	horizontalLayout->addItem(horizontalSpacer);

	verticalSpacer2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
	verticalLayout2->addLayout(horizontalLayout);
	verticalLayout2->addItem(verticalSpacer2);

	gridLayout_4->addLayout(verticalLayout2, 0, 0, 1, 1);

    pushButton_2 = new QPushButton(_groupBox);
    pushButton_2->setObjectName(QString::fromUtf8("pushButton" + currRow));
    pushButton_2->setMaximumSize(QSize(31, 20));
	pushButton_2->setText(QApplication::translate("ParameterWidget", "-", 0, QApplication::UnicodeUTF8));
    
	verticalSpacer3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
	verticalLayout3->addWidget(pushButton_2);
	verticalLayout3->addItem(verticalSpacer3);

	gridLayout_4->addLayout(verticalLayout3, 0, 1, 1, 1);	

	_window->addLayout(gridLayout_4, nextRow, 1, 1, 1);
	_window->addLayout(verticalLayout, nextRow, 0, 1, 1);
	
	connect(pushButton_2, SIGNAL(clicked()), gridLayout_4, SLOT( deleteLater() ) );
	connect(pushButton_2, SIGNAL(clicked()), xcomboBox, SLOT( deleteLater() ) );
	connect(pushButton_2, SIGNAL(clicked()), lineEdit, SLOT( deleteLater() ) );
	connect(pushButton_2, SIGNAL(clicked()), pushButton_2, SLOT( deleteLater() ) );
	connect(xcomboBox, SIGNAL(currentIndexChanged(int)), this, SLOT( changeFilterObject(int)) );
	connect(pushButton_2, SIGNAL(clicked()), _filterSignalMapper, SLOT(map()));	
	connect(lineEdit, SIGNAL(textChanged(QString)), this, SLOT( storeFilterValue() ) );

	_filterSignalMapper->setMapping(pushButton_2, nextRow);
}


void ParameterWidget::applySaved(int pId)
{
	this->applySaved(pId, NULL);
}

void ParameterWidget::applySaved(int pId, int filter_id)
{
	QGridLayout *container; 
	QLayoutItem *_child;		
	QLayoutItem *_child2;
	QHBoxLayout *_layout2;
	QWidget *_found;
	QDate tempdate;
	DLineEdit *dLineEdit;
	XSqlQuery qry;
	QString query;
	QString filterValue;
	QLineEdit *lineEdit;
	UsernameCluster *usernameCluster;
	CRMAcctCluster *crmacctCluster;

	this->clearFilters();

	if (this->parent() == NULL)
		return;

	if (_filterList->id() == NULL)
	{
		_filterSetName->clear();
		setSelectedFilter(-1);
		emit updated();
		return;	
	}
	if (filter_id == NULL && _filterList->id() != NULL)
	{
		filter_id = _filterList->id(_filterList->currentIndex());
	}
	const QMetaObject *metaobject = this->parent()->metaObject();     
	QString classname(metaobject->className());
	
	//look up filter from database
	query = " SELECT filter_value "
		" FROM filter "
		" WHERE filter_username=current_user "
		" AND filter_id=:id "
		" AND filter_screen=:screen ";

	qry.prepare(query);
	qry.bindValue(":screen", classname);
	qry.bindValue(":id", filter_id );

	qry.exec();

	if (qry.first())
	{
		filterValue = qry.value("filter_value").toString();
	}
	
	QStringList filterRows = filterValue.split("|");
	QString tempFilter = NULL;

	int windowIdx = _window->rowCount();

	for (int i = 0; i < filterRows.size(); ++i)
	{
		tempFilter = filterRows[i];
		if ( !(tempFilter.isEmpty()) || !(tempFilter.isNull()) )
		{
			//0 is filterType, 1 is filterValue, 2 is parameterwidgettype
			QStringList tempFilterList = tempFilter.split(":");
			this->addParam();
			
			QLayoutItem *test = _window->itemAtPosition(windowIdx, 0)->layout()->itemAt(0);
			XComboBox *_mybox = (XComboBox*)test->widget();

			QString key = this->getParameterTypeKey(tempFilterList[0]);
			int idx = _mybox->findText(key);
			
			_mybox->setCurrentIndex(idx);		
			
			QString row;
			row = row.setNum(windowIdx);

			container = _window->findChild<QGridLayout *>("gridLayout" + row);
			_child = container->itemAtPosition(0, 0)->layout()->itemAt(0);
			_layout2 = (QHBoxLayout *)_child->layout();
			_child2 = _layout2->itemAt(0);
			_found = _child2->widget();

			int widgetType = tempFilterList[2].toInt();

			//grab pointer to newly created filter object
			switch (widgetType)
			{
				case 3:
					dLineEdit = (DLineEdit*)_found;
					dLineEdit->setDate(QDate::fromString(tempFilterList[1], "yyyy-MM-dd"), true);
					break;
				case 2:
					lineEdit = (QLineEdit*)_found;
					lineEdit->setText(tempFilterList[1]);
					break;
				case 1:
					usernameCluster = (UsernameCluster*)_found;
					usernameCluster->setUsername(tempFilterList[1]);
					break;
				case 0:
					crmacctCluster = (CRMAcctCluster*)_found;
					crmacctCluster->setId(tempFilterList[1].toInt());
					break;
				default:
					lineEdit = (QLineEdit*)_found;
					lineEdit->setText(tempFilterList[1]);
					break;
			}
			
		
		}//end of if
		windowIdx++;
	}//end of for

	_filterSetName->setText( _filterList->currentText() );
	setSelectedFilter(filter_id);
	emit updated();
}


void ParameterWidget::changeFilterButton()
{
	if (_filterButton->text() == "More Filters")
	{
		_filterButton->setText("Hide Filters");
		_groupBox->show();
	}
	else
	{
		_filterButton->setText("More Filters");
		_groupBox->hide();
	}
}


void ParameterWidget::changeFilterObject(int _index)
{
	
	QHBoxLayout *horizontalLayout;
	QSpacerItem *horizontalSpacer;
	QVBoxLayout *verticalLayout;
	QSpacerItem *verticalSpacer;
	QLayoutItem *_child;
	QLayoutItem *_child2;
	QHBoxLayout *_layout2;
	QLayoutItem *_childSpacer;
	QLayoutItem *_button;
	QWidget *_buttonFound;
	QWidget *_found;

	XComboBox *_mybox = (XComboBox *)sender();
	QVariant _filterVar(_mybox->itemData(_index));
	QString _filterType = _filterVar.toString();
	QStringList split = _filterType.split(":");

	QList<QObject *>_list = _window->findChildren<QObject *>();
	QGridLayout *container = _window->findChild<QGridLayout *>("gridLayout" + split[0]);
	if (container != NULL)
	{
		_child = container->itemAtPosition(0, 0);
		_layout2 = (QHBoxLayout *)_child->layout()->itemAt(0);
		_child2 = _layout2->itemAt(0);
		_childSpacer = _layout2->itemAt(1);
		_button = container->itemAtPosition(0, 1)->layout()->itemAt(0);
		_buttonFound = _button->widget();
		_found = _child2->widget();
	}

	QLineEdit *lineEdit;
	DLineEdit *dLineEdit;
	UsernameCluster *usernameCluster;
	CRMAcctCluster *crmacctCluster;

	if (_found != NULL  && _child != 0 && _child2 != NULL)
	{
		container->removeItem(_child);
		delete _found;
		_found = NULL;
	}
	
	int mytype = split[1].toInt();

	switch (mytype)
	{
		case 3:
			horizontalLayout = new QHBoxLayout();
			horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
			horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

			dLineEdit = new DLineEdit(_groupBox);
			dLineEdit->setObjectName(QString::fromUtf8("lineEdit" + split[0]));
			
			horizontalLayout->addWidget(dLineEdit);
			horizontalLayout->addItem(horizontalSpacer);

			verticalLayout = new QVBoxLayout();
			verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
			verticalLayout->setContentsMargins(0, 0, 0, 0);
			verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
			verticalLayout->addLayout(horizontalLayout);
			verticalLayout->addItem(verticalSpacer);

			container->addLayout(verticalLayout, 0, 0, 1, 1);

			connect(_buttonFound, SIGNAL(clicked()), dLineEdit, SLOT( deleteLater() ) );
			connect(dLineEdit, SIGNAL(newDate(QDate)), this, SLOT( storeFilterValue(QDate) ) );
			break;
		case 2:
			horizontalLayout = new QHBoxLayout();
			horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
			horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
			lineEdit = new QLineEdit(_groupBox);
			lineEdit->setObjectName(QString::fromUtf8("lineEdit" + split[0]));

			horizontalLayout->addWidget(lineEdit);
			horizontalLayout->addItem(horizontalSpacer);

			verticalLayout = new QVBoxLayout();
			verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
			verticalLayout->setContentsMargins(0, 0, 0, 0);
			verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
			verticalLayout->addLayout(horizontalLayout);
			verticalLayout->addItem(verticalSpacer);

			container->addLayout(verticalLayout, 0, 0, 1, 1);

			connect(_buttonFound, SIGNAL(clicked()), lineEdit, SLOT( deleteLater() ) );
			connect(lineEdit, SIGNAL(textChanged(QString)), this, SLOT( storeFilterValue() ) );
			break;
		case 1:
			horizontalLayout = new QHBoxLayout();
			horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
			horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
			usernameCluster = new UsernameCluster(_groupBox);
			usernameCluster->setObjectName(QString::fromUtf8("lineEdit" + split[0]));

			horizontalLayout->addWidget(usernameCluster);
			horizontalLayout->addItem(horizontalSpacer);

			verticalLayout = new QVBoxLayout();
			verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
			verticalLayout->setContentsMargins(0, 0, 0, 0);
			verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
			verticalLayout->addLayout(horizontalLayout);
			verticalLayout->addItem(verticalSpacer);

			container->addLayout(verticalLayout, 0, 0, 1, 1);
			connect(_buttonFound, SIGNAL(clicked()), usernameCluster, SLOT( deleteLater() ) );
			connect(usernameCluster, SIGNAL(newId(int)), this, SLOT( storeFilterValue(int) ) );
			break;
		case 0:
			horizontalLayout = new QHBoxLayout();
			horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
			horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
			crmacctCluster = new CRMAcctCluster(this);
			crmacctCluster->setObjectName(QString::fromUtf8("lineEdit" + split[0]));

			horizontalLayout->addWidget(crmacctCluster);
			horizontalLayout->addItem(horizontalSpacer);

			verticalLayout = new QVBoxLayout();
			verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
			verticalLayout->setContentsMargins(0, 0, 0, 0);
			verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
			verticalLayout->addLayout(horizontalLayout);
			verticalLayout->addItem(verticalSpacer);

			container->addLayout(verticalLayout, 0, 0, 1, 1);
			connect(_buttonFound, SIGNAL(clicked()), crmacctCluster, SLOT( deleteLater() ) );
			connect(crmacctCluster, SIGNAL(newId(int)), this, SLOT( storeFilterValue(int) ) );
			break;
		default:
			horizontalLayout = new QHBoxLayout();
			horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
			horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
			lineEdit = new QLineEdit(_groupBox);
			lineEdit->setObjectName(QString::fromUtf8("lineEdit" + split[0]));
	
			horizontalLayout->addWidget(lineEdit);
			horizontalLayout->addItem(horizontalSpacer);
			
			verticalLayout = new QVBoxLayout();
			verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
			verticalLayout->setContentsMargins(0, 0, 0, 0);
			verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
			verticalLayout->addLayout(horizontalLayout);
			verticalLayout->addItem(verticalSpacer);

			container->addLayout(verticalLayout, 0, 0, 1, 1);
			connect(_buttonFound, SIGNAL(clicked()), lineEdit, SLOT( deleteLater() ) );
			connect(lineEdit, SIGNAL(textChanged(QString)), this, SLOT( storeFilterValue() ) );
			break;
	}
}





void ParameterWidget::clearFilters()
{
	bool _hide = NULL;

	if (_groupBox->isHidden())
	{
		_hide = true;
	}
	else
	{
		_hide = false;
	}

	delete _groupBox;
	_groupBox=NULL;

	addGroupBox();
	if (_hide)
	{
		_groupBox->hide();
	}
	else
	{
		_groupBox->show();
	}
	_filterValues.clear();
}









void ParameterWidget::removeParam(int pRow)
{
	QLayoutItem *test;
	QLayoutItem *test2;
	
	test = _window->itemAtPosition(pRow, 0)->layout()->itemAt(0);
	XComboBox *_mybox = (XComboBox*)test->widget();
	
	QVariant _filterVar(_mybox->itemData(_mybox->currentIndex()));
	QString _filterType = _filterVar.toString();
	QStringList split = _filterType.split(":");

	QPair<QString, QVariant> tempPair = _filterValues.value(split[0].toInt());

	_filterValues.remove(split[0].toInt());
	

	test2 = _window->itemAtPosition(pRow, 0)->layout()->takeAt(1);
	delete test2;
	test2=NULL;
	_window->update();
	emit updated();
}



void ParameterWidget::save()
{
	QString filter;
	QString filtersetname;
	QString variantString;
	QString username;
	QString query;
	QVariant tempVar;
	int filter_id;
	QMessageBox msgBox;

	filtersetname = _filterSetName->text();

	if ( filtersetname.isEmpty() )
	{
		 msgBox.setText("Please enter a name for this filter set before saving.");
		 msgBox.exec();
		 return;
	}
	else
	{

		QMapIterator<int, QPair<QString, QVariant>> i(_filterValues);
		while (i.hasNext())
		{
			i.next();
			QPair<QString, QVariant> tempPair = i.value();

			tempVar = tempPair.second;

			QLayoutItem *test = _window->itemAtPosition(i.key(), 0)->layout()->itemAt(0);
			XComboBox *_mybox = (XComboBox*)test->widget();
			QVariant _filterVar(_mybox->itemData(_mybox->currentIndex()));
			QString _filterType = _filterVar.toString();
			QStringList split = _filterType.split(":");
	
			if ( tempVar.canConvert<QString>() )
			{
				variantString = tempVar.toString();
				filter = filter + tempPair.first + ":" + variantString + ":" + split[1] + "|";
			}			
		}

		const QMetaObject *metaobject = this->parent()->metaObject();     
		QString classname(metaobject->className());

		XSqlQuery qry, qry2, qry3;

		qry.exec("SELECT current_user;");
		if (qry.first())
		username = qry.value("current_user").toString();

		//check to see if filter name exists for this screen
		QString filter_query = "select filter_name from filter where filter_name=:name and filter_username=:username and filter_screen=:screen";
		qry2.prepare(filter_query);
		qry2.bindValue(":name", filtersetname);
		qry2.bindValue(":username" , username);
		qry2.bindValue(":screen", classname);

		qry2.exec();
		

		//if the filter name is found, update it
		if (qry2.first() && !qry2.isNull(0))
		{
			query = "update filter set filter_value=:value where filter_screen=:screen and filter_name=:name and filter_username=:username";

		}
		else
		{
			
			query = "insert into filter (filter_screen, filter_name, filter_value, filter_username) "
				" values (:screen, :name, :value, :username) ";
		}

		qry.prepare(query);
		qry.bindValue(":screen", classname);
		qry.bindValue(":value", filter);
		qry.bindValue(":username" , username);
 		qry.bindValue(":name", filtersetname );

			if (qry.exec())
			{
				query = "select filter_id from filter where filter_name=:name and filter_username=:username and filter_screen=:screen";
				qry3.prepare(query);
				qry3.bindValue(":screen", classname);
				qry3.bindValue(":username" , username);
 				qry3.bindValue(":name", filtersetname );
				qry3.exec();

				if (qry3.first())
				{
					filter_id = qry.value("filter_id").toInt();
				}
				emit filterSetSaved();
			}
	}
	int foundIndex = _filterList->findText(filtersetname);
	setSavedFilters(foundIndex);
}

void ParameterWidget::setSavedFilters()
{
	this->setSavedFilters(-1);
}
void ParameterWidget::setSavedFilters(int defaultId = NULL)
{
	QString query;
	XSqlQuery qry;

	if(this->parent() != NULL)
	{
		const QMetaObject *metaobject = this->parent()->metaObject();     
		QString classname(metaobject->className());

		query = " SELECT 0 AS filter_id, 'Select Saved Filters' AS filter_name "
		" UNION " 
		" SELECT filter_id, filter_name "
		" FROM filter " 
		" WHERE filter_username=current_user "
		" AND filter_screen=:screen "
		" ORDER BY filter_name ";

		qry.prepare(query);
		
		qry.bindValue(":screen", classname);
		qry.exec();
		if (defaultId != NULL && defaultId > 0)
		{
			_filterList->populate(qry, defaultId);
		}
		else
		{
			_filterList->populate(qry, 0);
		}
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

void ParameterWidget::sManageFilters()
{
	filterManager *newdlg = new filterManager(this, "");

	newdlg->exec();
}


void ParameterWidget::storeFilterValue()
{
	this->storeFilterValue(NULL);
}

void ParameterWidget::storeFilterValue(QDate _date)
{
	QObject *filter = (QObject *)sender();
	QLayoutItem *test;
	QLayoutItem *test2;
	QLayoutItem *_child;
	QLayoutItem *_child2;
	QGridLayout *_layout;
	QHBoxLayout *_layout2;
	QWidget *_found;
	XComboBox *_mybox;
	int foundRow = NULL;

	for (int i = 1; i < _window->rowCount(); i++)
	{
		test = _window->itemAtPosition(i, 1);
		if (test != NULL)
		{
			_layout = (QGridLayout *)test->layout();
			_child =_layout->itemAtPosition(0, 0);
			_layout2 = (QHBoxLayout *)_child->layout()->itemAt(0);
			_child2 = _layout2->itemAt(0);
			_found = _child2->widget();

			if (_found == filter )
			{
				foundRow = i;
			}
		}
	}
	
	test2 = _window->itemAtPosition(foundRow, 0)->layout()->itemAt(0);
	_mybox = (XComboBox*)test2->widget();
	QString _currText = _mybox->currentText();
	QPair<QString, ParameterWidgetTypes> tempPair = _types[_currText];

	_filterValues[foundRow] = qMakePair(tempPair.first, QVariant(_date));
	emit updated();
	
}


//stores the value of a filter object into the filtervalues map
void ParameterWidget::storeFilterValue(int pId = NULL)
{
	qDebug() << "in storefiltervalue, pId passed is: " << pId;

	QObject *filter = (QObject *)sender();
	QLayoutItem *test;
	QLayoutItem *test2;
	QLayoutItem *_child;
	QLayoutItem *_child2;
	QGridLayout *_layout;
	QHBoxLayout *_layout2;
	QWidget *_found;
	XComboBox *_mybox;
	int foundRow = NULL;

	for (int i = 1; i < _window->rowCount(); i++)
	{
		
		test = _window->itemAtPosition(i, 1);
		if (test != NULL)
		{
			_layout = (QGridLayout *)test->layout();
			_child =_layout->itemAtPosition(0, 0);
			_layout2 = (QHBoxLayout *)_child->layout()->itemAt(0);
			_child2 = _layout2->itemAt(0);

			_found = _child2->widget();
		
			if (_found == filter )
			{
				foundRow = i;
			}
		}
	}


	test2 = _window->itemAtPosition(foundRow, 0)->layout()->itemAt(0);
	_mybox = (XComboBox*)test2->widget();
	QString _currText = _mybox->currentText();
	QPair<QString, ParameterWidgetTypes> tempPair = _types[_currText];

	const QMetaObject *metaobject = filter->metaObject();     
	QString classname(metaobject->className());

	if (pId == NULL)
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
			QString _username = usernameCluster->username();
			_filterValues[foundRow] = qMakePair(tempPair.first, QVariant(_username));
			emit updated();
		}
		else
		{
			_filterValues[foundRow] = qMakePair(tempPair.first, QVariant(pId));
			emit updated();
		}
	}
}





void ParameterWidget::addGroupBox()
{
	QGridLayout *gridLayout;
    QGridLayout *gridLayout_5;
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QVBoxLayout *verticalLayout_3;
    QLabel *label_3;
    QGridLayout *gridLayout_3;

	int nextRow;
	QString currRow;

	nextRow = 1;
	currRow = currRow.setNum(nextRow);

	 QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
     sizePolicy.setHorizontalStretch(0);
     sizePolicy.setVerticalStretch(0);
     sizePolicy.setHeightForWidth(this->sizePolicy().hasHeightForWidth());
     this->setSizePolicy(sizePolicy);
	 _groupBox = new QGroupBox(this);
     gridLayout = new QGridLayout(_groupBox);
     gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
     gridLayout_5 = new QGridLayout();
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        _window = new QGridLayout();
        _window->setObjectName(QString::fromUtf8("_window"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label = new QLabel(_groupBox);
        label->setObjectName(QString::fromUtf8("label"));
        label->setMaximumSize(QSize(16777215, 18));

        verticalLayout->addWidget(label);

        _window->addLayout(verticalLayout, 0, 0, 1, 1);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        label_3 = new QLabel(_groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        verticalLayout_3->addWidget(label_3);

        _window->addLayout(verticalLayout_3, 0, 1, 1, 1);


        gridLayout_5->addLayout(_window, 0, 0, 1, 1);

        gridLayout_3 = new QGridLayout();
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        addFilterRow = new QPushButton(_groupBox);
        addFilterRow->setObjectName(QString::fromUtf8("addFilterRow"));
        addFilterRow->setMaximumSize(QSize(16777215, 23));

        gridLayout_3->addWidget(addFilterRow, 0, 0, 1, 1);

        _filterSetName = new QLineEdit(_groupBox);
        _filterSetName->setObjectName(QString::fromUtf8("_filterSetName"));
        _filterSetName->setMaximumSize(QSize(16777215, 23));

        gridLayout_3->addWidget(_filterSetName, 0, 1, 1, 1);

        _saveButton = new QPushButton(_groupBox);
        _saveButton->setObjectName(QString::fromUtf8("_saveButton"));
        _saveButton->setMaximumSize(QSize(16777215, 23));

        gridLayout_3->addWidget(_saveButton, 0, 2, 1, 1);

        _manageButton = new QPushButton(_groupBox);
        _manageButton->setObjectName(QString::fromUtf8("_manageButton"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(_manageButton->sizePolicy().hasHeightForWidth());
        _manageButton->setSizePolicy(sizePolicy2);
        _manageButton->setMaximumSize(QSize(134, 23));

        gridLayout_3->addWidget(_manageButton, 0, 3, 1, 1);

        gridLayout_5->addLayout(gridLayout_3, 1, 0, 1, 1);
        gridLayout->addLayout(gridLayout_5, 1, 0, 1, 1);

		_groupBox->setTitle(QApplication::translate("ParameterWidget", "Filters", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("ParameterWidget", "Filter By", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("ParameterWidget", "Value", 0, QApplication::UnicodeUTF8));
        addFilterRow->setText(QApplication::translate("ParameterWidget", "Add Filter", 0, QApplication::UnicodeUTF8));
        _saveButton->setText(QApplication::translate("ParameterWidget", "Save", 0, QApplication::UnicodeUTF8));
        _manageButton->setText(QApplication::translate("ParameterWidget", "Manage Filter Sets", 0, QApplication::UnicodeUTF8));

	connect(addFilterRow, SIGNAL(clicked()), this, SLOT( addParam() ) );
	connect(_saveButton, SIGNAL(clicked()), this, SLOT( save() ) );
	connect(_manageButton, SIGNAL(clicked()), this, SLOT( sManageFilters() ) );

	_groupBox->setLayout(gridLayout);

	vbox->addWidget(_groupBox);
}



QString ParameterWidget::getParameterTypeKey(QString pValue)
{
	QMapIterator<QString, QPair<QString, ParameterWidgetTypes>> i(_types);
	while (i.hasNext())
	{
		i.next();
		QPair<QString, ParameterWidgetTypes> tempPair = i.value();
		
		if (pValue == tempPair.first)
		{
			return i.key();
		}
	}

	return NULL;
}


//updates selected filter set to be the default upon loading of the current screen
//for the current user
void ParameterWidget::setSelectedFilter(int filter_id)
{
	XSqlQuery qry;
	const QMetaObject *metaobject = this->parent()->metaObject();     
	QString classname(metaobject->className());

	QString query = "UPDATE filter SET filter_selected=false WHERE filter_screen=:screen AND filter_username=current_user";
	QString query2 = "UPDATE filter SET filter_selected=true WHERE filter_screen=:screen AND filter_id=:id AND filter_username=current_user";
	qry.prepare(query);
	qry.bindValue(":screen", classname);

	qry.exec();

	qry.prepare(query2);
	qry.bindValue(":screen", classname);
	qry.bindValue(":id", filter_id);

	qry.exec();	
}

