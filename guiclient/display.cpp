/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "display.h"
#include "xlineedit.h"
#include "ui_display.h"

#include <QSqlError>
#include <QMessageBox>
#include <QPrinter>
#include <QPrintDialog>
#include <QShortcut>

#include <metasql.h>
#include <mqlutil.h>
#include <orprerender.h>
#include <orprintrender.h>
#include <renderobjects.h>
#include <parameter.h>
#include <previewdialog.h>

class displayPrivate : public Ui::display
{
public:
  displayPrivate(::display * parent) : _parent(parent)
  {
    setupUi(_parent);
    _new->setVisible(false);
    _print->setVisible(false); // hide the print button until a reportName is set
    _preview->setVisible(false); // hide the preview button until a reportName is set
    _queryonstart->hide(); // hide until query on start enabled
    _autoupdate->hide(); // hide until auto update is enabled
    _parameterWidget->hide(); // hide until user shows manually
    _more->setVisible(false); // hide until user shows manually
    _useAltId = false;
    _queryOnStartEnabled = false;
    _autoUpdateEnabled = false;

    // Move parameter widget controls into toolbar
    QLabel* filterListLit = _parent->findChild<QLabel*>("_filterListLit");
    XComboBox* filterList = _parent->findChild<XComboBox*>("_filterList");

    _filterLitAct = _toolBar->insertWidget(_more, filterListLit);
    _filterAct = _toolBar->insertWidget(_more, filterList);
    _filterSep = _toolBar->insertSeparator(_query);

    _filterLitAct->setVisible(false);
    _filterAct->setVisible(false);
    _filterSep->setVisible(false);

    // Add search int toolbar
    _search = new XLineEdit(_toolBar, "_search");
    _search->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    _searchAct = _toolBar->insertWidget(_query, _search);
    _searchAct->setVisible(false);
  }

  void print(bool);

  QString reportName;
  QString metasqlName;
  QString metasqlGroup;

  bool _useAltId;
  bool _queryOnStartEnabled;
  bool _autoUpdateEnabled;

  QAction* _filterLitAct;
  QAction* _filterAct;
  QAction* _filterSep;
  QAction* _searchAct;

  XLineEdit* _search;

private:
  ::display * _parent;
};

void displayPrivate::print(bool showPreview)
{
  int numCopies = 1;

  ParameterList params;
  if(!_parent->setParams(params))
    return;

  XSqlQuery report;
  report.prepare("SELECT report_grade, report_source "
                 "  FROM report "
                 " WHERE (report_name=:report_name)"
                 " ORDER BY report_grade DESC LIMIT 1");
  report.bindValue(":report_name", reportName);
  report.exec();
  QDomDocument _doc;
  if (report.first())
  {
    QString errorMessage;
    int     errorLine;

    if (!_doc.setContent(report.value("report_source").toString(), &errorMessage, &errorLine))
    {
      QMessageBox::critical(_parent, ::display::tr("Error Parsing Report"),
        ::display::tr("There was an error Parsing the report definition. %1 %2").arg(errorMessage).arg(errorLine));
      return;
    }
  }
  else
  {
    QMessageBox::critical(_parent, ::display::tr("Report Not Found"),
      ::display::tr("The report %1 does not exist.").arg(reportName));
    return;
  }

  ORPreRender pre;
  pre.setDom(_doc);
  pre.setParamList(params);
  ORODocument * doc = pre.generate();

  if(doc)
  {
    QPrinter printer(QPrinter::HighResolution);
    printer.setNumCopies( numCopies );

    if(showPreview)
    {
      PreviewDialog preview (doc, &printer, _parent);
      if (preview.exec() == QDialog::Rejected)
        return;
    }

    ORPrintRender render;
    render.setupPrinter(doc, &printer);

    QPrintDialog pd(&printer);
    pd.setMinMax(1, doc->pages());
    if(pd.exec() == QDialog::Accepted)
    {
      render.setPrinter(&printer);
      render.render(doc);
    }
    delete doc;
  }
}

display::display(QWidget* parent, const char* name, Qt::WindowFlags flags)
    : XWidget(parent, name, flags)
{
  _data = new displayPrivate(this);

  QPushButton* filterButton = findChild<QPushButton*>("_filterButton");

  // Set shortcuts
  _data->_new->setShortcut(QKeySequence::New);
  _data->_close->setShortcut(QKeySequence::Close);
  _data->_query->setShortcut(QKeySequence::Refresh);
  _data->_print->setShortcut(QKeySequence::Print);
  _data->_searchAct->setShortcut(QKeySequence::InsertParagraphSeparator);
  _data->_searchAct->setShortcutContext(Qt::WidgetWithChildrenShortcut);

  // Set tooltips
  _data->_new->setToolTip(_data->_new->text() + " " + _data->_new->shortcut().toString(QKeySequence::NativeText));
  _data->_close->setToolTip(_data->_close->text() + " " + _data->_close->shortcut().toString(QKeySequence::NativeText));
  _data->_query->setToolTip(_data->_query->text() + " " + _data->_query->shortcut().toString(QKeySequence::NativeText));
  _data->_print->setToolTip(_data->_print->text() + " " + _data->_print->shortcut().toString(QKeySequence::NativeText));
  _data->_search->setNullStr(tr("Search"));

  connect(_data->_new, SIGNAL(triggered()), this, SLOT(sNew()));
  connect(_data->_print, SIGNAL(triggered()), this, SLOT(sPrint()));
  connect(_data->_preview, SIGNAL(triggered()), this, SLOT(sPreview()));
  connect(_data->_searchAct, SIGNAL(triggered()), this, SLOT(sFillList()));
  connect(_data->_query, SIGNAL(triggered()), this, SLOT(sFillList()));
  connect(_data->_list, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*,int)));
  connect(_data->_autoupdate, SIGNAL(toggled(bool)), this, SLOT(sAutoUpdateToggled()));
  connect(filterButton, SIGNAL(toggled(bool)), _data->_more, SLOT(setChecked(bool)));
  connect(_data->_more, SIGNAL(triggered(bool)), filterButton, SLOT(setChecked(bool)));
  connect(_data->_close, SIGNAL(triggered()), this, SLOT(close()));
}

display::~display()
{
  delete _data;
  _data = 0;
}

void display::languageChange()
{
  _data->retranslateUi(this);
}

void display::showEvent(QShowEvent * e)
{
  XWidget::showEvent(e);

  if (_data->_queryOnStartEnabled &&
      _data->_queryonstart->isChecked())
    sFillList();
}

QWidget * display::optionsWidget()
{
  return _data->_optionsFrame;
}

XTreeWidget * display::list()
{
  return _data->_list;
}

ParameterWidget * display::parameterWidget()
{
  return _data->_parameterWidget;
}

QToolBar * display::toolBar()
{
  return _data->_toolBar;
}

QAction * display::newAction()
{
  return _data->_new;
}

QAction * display::closeAction()
{
  return _data->_close;
}

QAction * display::queryAction()
{
  return _data->_query;
}

QAction * display::printAction()
{
  return _data->_print;
}

QAction * display::previewAction()
{
  return _data->_preview;
}

QAction * display::searchAction()
{
  return _data->_searchAct;
}

QString display::searchText()
{
  if (!_data->_search->isNull())
    return _data->_search->text().trimmed();
  return QString("");
}

bool display::setParams(ParameterList & params)
{
  parameterWidget()->appendValue(params);
  if (!_data->_search->isNull())
    params.append("pattern", _data->_search->text());

  return true;
}

void display::setReportName(const QString & reportName)
{
  _data->reportName = reportName;
  _data->_print->setVisible(!reportName.isEmpty());
  _data->_preview->setVisible(!reportName.isEmpty());
}

QString display::reportName() const
{
  return _data->reportName;
}

void display::setMetaSQLOptions(const QString & group, const QString & name)
{
  _data->metasqlName = name;
  _data->metasqlGroup = group;
}

void display::setListLabel(const QString & pText)
{
  _data->_listLabel->setText(pText);
}

void display::setUseAltId(bool on)
{
  _data->_useAltId = on;
}

bool display::useAltId() const
{
  return _data->_useAltId;
}

void display::setNewVisible(bool show)
{
  _data->_new->setVisible(show);
}

bool display::newVisible() const
{
  return _data->_new->isVisible();
}

void display::setCloseVisible(bool show)
{
  _data->_close->setVisible(show);
}

bool display::closeVisible() const
{
  return _data->_close->isVisible();
}

void display::setParameterWidgetVisible(bool show)
{
  _data->_parameterWidget->setVisible(show);
  _data->_parameterWidget->_filterButton->hide(); // _more action is what you see here
  _data->_more->setVisible(show);
  _data->_filterLitAct->setVisible(show);
  _data->_filterAct->setVisible(show);
  _data->_filterSep->setVisible(show);
}

bool display::parameterWidgetVisible() const
{
  return _data->_parameterWidget->isVisible();
}

void display::setSearchVisible(bool show)
{
  _data->_searchAct->setVisible(show);
}

bool display::searchVisible() const
{
  return _data->_searchAct->isVisible();
}

void display::setQueryOnStartEnabled(bool on)
{
  _data->_queryOnStartEnabled = on;
  _data->_queryonstart->setVisible(on);

  // Ensure query on start is checked by default
  if (on)
  {
    QString prefname = window()->objectName() + "/" +
                       _data->_queryonstart->objectName() + "/checked";
    XSqlQuery qry;
    qry.prepare("SELECT usrpref_id "
                "FROM usrpref "
                "WHERE ((usrpref_username=current_user) "
                " AND (usrpref_name=:prefname));");
    qry.bindValue(":prefname", prefname);
    qry.exec();
    if (!qry.first())
      _preferences->set(prefname, 2);
  }
}

bool display::queryOnStartEnabled() const
{
  return _data->_queryonstart->isVisible();
}

void display::setAutoUpdateEnabled(bool on)
{
  _data->_autoUpdateEnabled = on;
  _data->_autoupdate->setVisible(on);
  sAutoUpdateToggled(); 
}

bool display::autoUpdateEnabled() const
{
  return _data->_autoUpdateEnabled;
}

void display::sNew()
{
}

void display::sPrint()
{
  _data->print(false);
}

void display::sPreview()
{
  _data->print(true);
}

void display::sFillList()
{
  ParameterList params;
  if (!setParams(params))
    return;
  int itemid = _data->_list->id();
  bool ok = true;
  QString errorString;
  MetaSQLQuery mql = MQLUtil::mqlLoad(_data->metasqlGroup, _data->metasqlName, errorString, &ok);
  if(!ok)
  {
    systemError(this, errorString, __FILE__, __LINE__);
    return;
  }
  XSqlQuery xq = mql.toQuery(params);
  _data->_list->populate(xq, itemid, _data->_useAltId);
  if (xq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, xq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void display::sPopulateMenu(QMenu *, QTreeWidgetItem *, int)
{
}

void display::sAutoUpdateToggled()
{
  bool update = _data->_autoUpdateEnabled && _data->_autoupdate->isChecked();
  if (update)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
}

ParameterList display::getParams()
{
  ParameterList params;
  bool ret = setParams(params);
  params.append("checkParamsReturn", ret);
    
  return params;
}
