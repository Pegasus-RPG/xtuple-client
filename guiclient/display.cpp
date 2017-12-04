/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "characteristic.h"
#include "display.h"
#include "ui_display.h"

#include <QSqlError>
#include <QMessageBox>
#include <QPrinter>
#include <QPrintDialog>
#include <QShortcut>
#include <QToolButton>

#include <metasql.h>
#include <mqlutil.h>
#include <orprerender.h>
#include <orprintrender.h>
#include <renderobjects.h>
#include <parameter.h>
#include <previewdialog.h>

#include "parameterlistsetup.h"
#include "errorReporter.h"
#include "displayprivate.h"

displayPrivate::displayPrivate(::display *parent)
    : QObject(parent),
      _useAltId(false),
      _queryOnStartEnabled(false),
      _autoUpdateEnabled(false),
      _filterChanged(false),
      _parent(parent)
{
  setupUi(_parent);

  _parameterWidget->setVisible(false);
  _queryonstart->hide(); // hide until query on start enabled
  _autoupdate->hide(); // hide until auto update is enabled
  _search->hide();
  _searchLit->hide();
  _listLabelFrame->setVisible(false);

  // Build Toolbar even if we hide it so we get actions
  _newBtn = new QToolButton(_toolBar);
  _newBtn->setObjectName("_newBtn");
  _newBtn->setFocusPolicy(Qt::NoFocus);
  _newAct = _toolBar->addWidget(_newBtn);
  _newAct->setVisible(false);

  _closeBtn = new QToolButton(_toolBar);
  _closeBtn->setObjectName("_closeBtn");
  _closeBtn->setFocusPolicy(Qt::NoFocus);
  _closeAct = _toolBar->addWidget(_closeBtn);

  _sep1 = _toolBar->addSeparator();

  // Move parameter widget controls into toolbar
  _moreBtn = new QToolButton(_toolBar);
  _moreBtn->setObjectName("_moreBtn");
  _moreBtn->setFocusPolicy(Qt::NoFocus);
  _moreBtn->setCheckable(true);
  _moreAct = _toolBar->addWidget(_moreBtn);
  _moreAct->setVisible(false);

  QLabel* filterListLit = _parent->findChild<QLabel*>("_filterListLit");
  filterListLit->setContentsMargins(3,0,3,0);
  XComboBox* filterList = _parent->findChild<XComboBox*>("_filterList");

  _filterLitAct = _toolBar->insertWidget(_moreAct, filterListLit);
  _filterLitAct->setVisible(false);

  _filterAct = _toolBar->insertWidget(_moreAct, filterList);
  _filterAct->setVisible(false);

  _sep2 = _toolBar->addSeparator();
  _sep2->setVisible(false);

  _expandBtn = new QToolButton(_toolBar);
  _expandBtn->setObjectName("_expandBtn");
  _expandBtn->setFocusPolicy(Qt::NoFocus);
  _expandAct = _toolBar->addWidget(_expandBtn);
  _expandAct->setVisible(false);

  _collapseBtn = new QToolButton(_toolBar);
  _collapseBtn->setObjectName("_collapseBtn");
  _collapseBtn->setFocusPolicy(Qt::NoFocus);
  _collapseAct = _toolBar->addWidget(_collapseBtn);
  _collapseAct->setVisible(false);

  // Print buttons
  _printBtn = new QToolButton(_toolBar);
  _printBtn->setObjectName("_printBtn");
  _printBtn->setFocusPolicy(Qt::NoFocus);
  _printAct = _toolBar->addWidget(_printBtn);
  _printAct->setVisible(false); // hide the print button until a reportName is set

  _previewBtn = new QToolButton(_toolBar);
  _previewBtn->setObjectName("_previewBtn");
  _printBtn->setFocusPolicy(Qt::NoFocus);
  _previewAct = _toolBar->addWidget(_previewBtn);
  _previewAct->setVisible(false); // hide the preview button until a reportName is set
  _sep3 = _toolBar->addSeparator();
  _sep3->setVisible(false);

  // Optional search widget in toolbar
  _search->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  _searchAct = _toolBar->addWidget(_search);
  _searchAct->setVisible(false);

  // Remaining buttons in toolbar
  _queryBtn = new QToolButton(_toolBar);
  _queryBtn->setObjectName("_queryBtn");
  _queryBtn->setFocusPolicy(Qt::NoFocus);
  _queryAct = _toolBar->addWidget(_queryBtn);

  // Menu actions for query options
  _queryMenu = new QMenu(_queryBtn);
  _queryOnStartAct = new QAction(_queryMenu);
  _queryOnStartAct->setCheckable(true);
  _queryOnStartAct->setVisible(false);
  _queryMenu->addAction(_queryOnStartAct);

  _autoUpdateAct = new QAction(_queryMenu);
  _autoUpdateAct->setCheckable(true);
  _autoUpdateAct->setVisible(false);
  _queryMenu->addAction(_autoUpdateAct);

  _parent->layout()->setContentsMargins(0,0,0,0);
  _parent->layout()->setSpacing(0);
}

void displayPrivate::sFilterChanged()
{
  _filterChanged = true;
}

void displayPrivate::print(ParameterList pParams, bool showPreview, bool forceSetParams)
{
  int numCopies = 1;
  ParameterList params = pParams;

  if (forceSetParams || !params.count())
  {
    if(!_parent->setParams(params))
      return;
  }
  params.append("isReport", true);

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
    ReportPrinter printer(QPrinter::HighResolution);
    printer.setNumCopies( numCopies );

    ORPrintRender render;
    render.setupPrinter(doc, &printer);

    if(showPreview)
    {
      QPrinter * tPrinter = &printer;
      QPrinter pdfPrinter(QPrinter::HighResolution);
      if(!printer.isValid())
      {
        render.setupPrinter(doc, &pdfPrinter);
        pdfPrinter.setOutputFormat(QPrinter::PdfFormat);
        tPrinter = &pdfPrinter;
      }
      PreviewDialog preview (doc, tPrinter, _parent);
      if (preview.exec() == QDialog::Rejected)
        return;
    }

    QPrintDialog pd(&printer);
    pd.setMinMax(1, doc->pages());
    if(pd.exec() == QDialog::Accepted)
    {
      render.render(doc, &printer);
    }
    delete doc;
  }
}

void displayPrivate::setupCharacteristics(QStringList uses)
{
  QString column;
  QString name;
  QString sql = QString("SELECT char_id, char_name, char_type "
                        "FROM char "
                        "  JOIN charuse ON char_id = charuse_char_id"
                        " WHERE char_search "
                        "   AND charuse_target_type IN ('%1')"
                        " ORDER BY char_name;").arg(uses.join("','"));
  XSqlQuery chars;
  chars.exec(sql);
  if (chars.size() <= 0)
  {
    qWarning() << "Could not find any characteristics matching" << uses;
  }
  while (chars.next())
  {
    characteristic::Type chartype = (characteristic::Type)chars.value("char_type").toInt();
    column = QString("char%1").arg(chars.value("char_id").toString());
    name = chars.value("char_name").toString();
    _list->addColumn(name, -1, Qt::AlignLeft , false, column );
    if (chartype == characteristic::Text || chartype == characteristic::Number)
    {
      _charidstext.append(chars.value("char_id").toInt());
      _parameterWidget->append(name, column, ParameterWidget::Text);
    }
    else if (chartype == characteristic::List)
    {
      _charidslist.append(chars.value("char_id").toInt());
      QString sql = QString("SELECT charopt_value, charopt_value"
                            "  FROM charopt"
                            " WHERE charopt_char_id = %1"
                            " ORDER BY charopt_order, charopt_value;")
          .arg(chars.value("char_id").toInt());
      _parameterWidget->append(name, column, ParameterWidget::Multiselect, QVariant(), false, sql);
    }
    else if (chartype == characteristic::Date)
    {
      _charidsdate.append(chars.value("char_id").toInt());
      QString start = QApplication::translate("display", "Start Date", 0);
      QString end = QApplication::translate("display", "End Date", 0);
      _parameterWidget->append(name + " " + start, column + "startDate", ParameterWidget::Date);
      _parameterWidget->append(name + " " + end, column + "endDate", ParameterWidget::Date);
    }
  }
}

bool displayPrivate::setParams(ParameterList &params)
{
  QString filter = _parameterWidget->filter();
  _parameterWidget->appendValue(params);
  if (!_search->isNull())
  {
    params.append("search_pattern", _search->text());
    QString searchon =  QApplication::translate("display", "Search On:", 0);
    filter.prepend(searchon + " " + _search->text() + "\n");
  }
  params.append("filter", filter);

  // Handle characteristics
  QVariant param;
  bool valid;
  QString column;
  QStringList clauses;

  // Put together the list of text and date based charids used to build joins
  params.append("char_id_text_list", _charidstext);
  params.append("char_id_list_list", _charidslist);
  params.append("char_id_date_list", _charidsdate);

  // Handle text based sections of clause
  foreach (QVariant columnid, _charidstext)
  {
    column = QString("char%1").arg(columnid.toString());
    param = params.value(column, &valid);
    if (valid)
      clauses.append(QString("charass_alias%1.charass_value ~* :char%1").arg(columnid.toString()));
  }
  // Handle text based sections of clause
  foreach (QVariant columnid, _charidslist)
  {
    column = QString("char%1").arg(columnid.toString());
    param = params.value(column, &valid);
    if (valid)
    {
      QStringList list;
      for (int j = 0; j < param.toStringList().count(); j++)
        list << QString(":char%1_%2").arg(columnid.toString()).arg(j);

      clauses.append(QString("charass_alias%1.charass_value IN (%2) ").arg(columnid.toString()).arg(list.join(",")));
    }
  }
  // Handle date based sections of clause
  foreach (QVariant columnid, _charidsdate)
  {
    // Look for start date
    column = QString("char%1startDate").arg(columnid.toString());
    param = params.value(column, &valid);
    if (valid)
      clauses.append(QString("charass_alias%1.charass_value::date >= :char%1startDate").arg(columnid.toString()));

    // Look for end date
    column = QString("char%1endDate").arg(columnid.toString());
    param = params.value(column, &valid);
    if (valid)
      clauses.append(QString("charass_alias%1.charass_value::date <= :char%1endDate").arg(columnid.toString()));
  }
  if (clauses.count())
    params.append("charClause", clauses.join(" AND ").prepend(" AND "));

  return true;
}

display::display(QWidget* parent, const char* name, Qt::WindowFlags flags)
    : XWidget(parent, name, flags)
{
  _data = new displayPrivate(this);

  QPushButton* filterButton = findChild<QPushButton*>("_filterButton");

  // Set text
  _data->_newBtn->setText(tr("New"));
  _data->_closeBtn->setText(tr("Close"));
  _data->_moreBtn->setText(tr("More"));
  _data->_expandBtn->setText(tr("Expand All"));
  _data->_collapseBtn->setText(tr("Collapse All"));
  _data->_printBtn->setText(tr("Print"));
  _data->_previewBtn->setText(tr("Preview"));
  _data->_queryBtn->setText(tr("Query"));
  _data->_queryOnStartAct->setText(tr("Query on start"));
  _data->_autoUpdateAct->setText(tr("Automatically Update"));

  // Set shortcuts
  _data->_newAct->setShortcut(QKeySequence::New);
  _data->_closeAct->setShortcut(QKeySequence::Close);
  _data->_queryAct->setShortcut(QKeySequence::Refresh);
  _data->_printAct->setShortcut(QKeySequence::Print);

  _data->_search->setNullStr(tr("search"));
  QList<QKeySequence> shortcuts;
  shortcuts << QKeySequence(Qt::Key_Enter) << QKeySequence(Qt::Key_Return);
  _data->_searchAct->setShortcuts(shortcuts);
  _data->_searchAct->setShortcutContext(Qt::WidgetWithChildrenShortcut);

  // Set tooltips
  _data->_newBtn->setToolTip(_data->_newBtn->text() + " " + _data->_newAct->shortcut().toString(QKeySequence::NativeText));
  _data->_closeBtn->setToolTip(_data->_closeBtn->text() + " " + _data->_closeAct->shortcut().toString(QKeySequence::NativeText));
  _data->_queryBtn->setToolTip(_data->_queryBtn->text() + " " + _data->_queryAct->shortcut().toString(QKeySequence::NativeText));
  _data->_printBtn->setToolTip(_data->_printBtn->text() + " " + _data->_printAct->shortcut().toString(QKeySequence::NativeText));
  _data->_expandBtn->setToolTip(_data->_expandBtn->text());
  _data->_collapseBtn->setToolTip(_data->_collapseBtn->text());

  connect(_data->_newBtn, SIGNAL(clicked()), _data->_newAct, SLOT(trigger()));
  connect(_data->_closeBtn, SIGNAL(clicked()), _data->_closeAct, SLOT(trigger()));
  connect(_data->_moreBtn, SIGNAL(clicked(bool)), filterButton, SLOT(setChecked(bool)));
  connect(_data->_expandBtn, SIGNAL(clicked()), _data->_expandAct, SLOT(trigger()));
  connect(_data->_collapseBtn, SIGNAL(clicked()), _data->_collapseAct, SLOT(trigger()));
  connect(_data->_printBtn, SIGNAL(clicked()), _data->_printAct, SLOT(trigger()));
  connect(_data->_previewBtn, SIGNAL(clicked()), _data->_previewAct, SLOT(trigger()));
  connect(_data->_queryBtn, SIGNAL(clicked()), _data->_queryAct, SLOT(trigger()));
  // Connect these two simply so checkbox takes care of pref. memory.  Could separate out later.
  connect(_data->_autoupdate, SIGNAL(toggled(bool)), _data->_autoUpdateAct, SLOT(setChecked(bool)));
  connect(_data->_autoUpdateAct, SIGNAL(triggered(bool)), _data->_autoupdate, SLOT(setChecked(bool)));
  connect(_data->_queryonstart, SIGNAL(toggled(bool)), _data->_queryOnStartAct, SLOT(setChecked(bool)));
  connect(_data->_queryOnStartAct, SIGNAL(triggered(bool)), _data->_queryonstart, SLOT(setChecked(bool)));

  // Connect Actions
  connect(_data->_newAct, SIGNAL(triggered()), this, SLOT(sNew()));
  connect(_data->_closeAct, SIGNAL(triggered()), this, SLOT(close()));
  connect(_data->_queryAct, SIGNAL(triggered()), this, SLOT(sFillList()));
  connect(_data->_expandAct, SIGNAL(triggered()), this, SLOT(sExpand()));
  connect(_data->_collapseAct, SIGNAL(triggered()), this, SLOT(sCollapse()));
  connect(_data->_printAct, SIGNAL(triggered()), this, SLOT(sPrint()));
  connect(_data->_previewAct, SIGNAL(triggered()), this, SLOT(sPreview()));
  connect(_data->_searchAct, SIGNAL(triggered()), this, SLOT(sFillList()));
  connect(this, SIGNAL(fillList()), this, SLOT(sFillList()));
  connect(_data->_list, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*,int)));
  connect(_data->_autoupdate, SIGNAL(toggled(bool)), this, SLOT(sAutoUpdateToggled()));
  connect(filterButton, SIGNAL(toggled(bool)), _data->_moreBtn, SLOT(setChecked(bool)));
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
  loadScriptEngine();
  XWidget::showEvent(e);

  // don't overwrite the user's filter when the window is minimized
  if (! _data->_filterChanged)
  {
    parameterWidget()->applyDefaultFilterSet();
    connect(parameterWidget(), SIGNAL(filterChanged()), _data, SLOT(sFilterChanged()), Qt::UniqueConnection);
  }

  if (_data->_queryOnStartEnabled &&
      _data->_queryonstart->isChecked())
    emit fillList();
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
  return _data->_newAct;
}

QAction * display::closeAction()
{
  return _data->_closeAct;
}

QAction * display::expandAction()
{
  return _data->_expandAct;
}

QAction * display::collapseAction()
{
  return _data->_collapseAct;
}

QAction * display::filterSeparator()
{
  return _data->_sep1;
}

QAction * display::printSeparator()
{
  return _data->_sep2;
}

QAction * display::printAction()
{
  return _data->_printAct;
}

QAction * display::previewAction()
{
  return _data->_previewAct;
}

QAction * display::querySeparator()
{
  return _data->_sep3;
}

QAction * display::searchAction()
{
  return _data->_searchAct;
}

QAction * display::queryAction()
{
  return _data->_queryAct;
}

QString display::searchText()
{
  if (!_data->_search->isNull())
    return _data->_search->text().trimmed();
  return QString("");
}

bool display::setParams(ParameterList & params)
{
  bool ret = _data->setParams(params);

  if (ret)
  {
    ret = setScriptableParams(params);
  }

  return ret;
}

void display::setupCharacteristics(QStringList uses)
{
  _data->setupCharacteristics(uses);
}

void display::setupCharacteristics(QString uses)
{
  QStringList ulist;
  ulist << uses;
  _data->setupCharacteristics(ulist);
}

void display::setReportName(const QString & reportName)
{
  _data->reportName = reportName;

  _data->_printAct->setVisible(!reportName.isEmpty());
  _data->_previewAct->setVisible(!reportName.isEmpty());
  _data->_sep3->setVisible(!reportName.isEmpty());
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
  _data->_listLabelFrame->setHidden(pText.isEmpty());
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
  _data->_newAct->setVisible(show);
  _data->_sep1->setVisible(show || _data->_closeAct->isVisible());
}

bool display::newVisible() const
{
  return _data->_newAct->isVisible();
}

void display::setCloseVisible(bool show)
{
  _data->_closeAct->setVisible(show);
  _data->_sep1->setVisible(show || _data->_newAct->isVisible());
}

bool display::closeVisible() const
{
  return _data->_closeAct->isVisible();
}

void display::setExpandVisible(bool show)
{
  _data->_expandAct->setVisible(show);
}

bool display::expandVisible() const
{
  return _data->_expandAct->isVisible();
}

void display::setCollapseVisible(bool show)
{
  _data->_collapseAct->setVisible(show);
}

bool display::collapseVisible() const
{
  return _data->_collapseAct->isVisible();
}

void display::setParameterWidgetVisible(bool show)
{
  _data->_parameterWidget->setVisible(show);
  _data->_parameterWidget->_filterButton->hide(); // _moreBtn is what you see here
  _data->_moreAct->setVisible(show);
  _data->_filterLitAct->setVisible(show);
  _data->_filterAct->setVisible(show);
  _data->_sep2->setVisible(show);
}

bool display::parameterWidgetVisible() const
{
  return _data->_parameterWidget->_filterGroup->isVisible();
}

bool display::searchVisible() const
{
  return _data->_search->isVisible();
}

void display::setSearchVisible(bool show)
{
  _data->_search->setVisible(show);
  _data->_searchAct->setVisible(show);
}

void display::setQueryOnStartEnabled(bool on)
{
  _data->_queryOnStartEnabled = on;
  _data->_queryOnStartAct->setVisible(on);

  if (_data->_queryOnStartEnabled ||
      _data->_autoUpdateEnabled)
  {
    _data->_queryBtn->setPopupMode(QToolButton::MenuButtonPopup);
    _data->_queryBtn->setMenu(_data->_queryMenu);
  }
  else
  {
    _data->_queryBtn->setPopupMode(QToolButton::DelayedPopup);
    _data->_queryBtn->setMenu(0);
  }

  _data->_queryonstart->setForgetful(!on);

  // Ensure query on start is checked by default
  if (on)
  {
    QString prefname = window()->objectName() + "/" +
                       _data->_queryonstart->objectName() + "/checked";
    XSqlQuery qry;
    qry.prepare("SELECT usrpref_id "
                "FROM usrpref "
                "WHERE ((usrpref_username=getEffectiveXtUser()) "
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
  _data->_autoUpdateAct->setVisible(on);

  if (_data->_queryOnStartEnabled ||
      _data->_autoUpdateEnabled)
  {
    _data->_queryBtn->setPopupMode(QToolButton::MenuButtonPopup);
    _data->_queryBtn->setMenu(_data->_queryMenu);
  }
  else
  {
    _data->_queryBtn->setPopupMode(QToolButton::DelayedPopup);
    _data->_queryBtn->setMenu(0);
  }

  sAutoUpdateToggled(); 
}

bool display::autoUpdateEnabled() const
{
  return _data->_autoUpdateEnabled;
}

void display::sNew()
{
}

void display::sExpand()
{
    if (_data->_list)
        _data->_list->expandAll();
}

void display::sCollapse()
{
    if (_data->_list)
        _data->_list->collapseAll();
}


void display::sPrint()
{
  sPrint(ParameterList());
}

void display::sPrint(ParameterList pParams, bool forceSetParams)
{
  _data->print(pParams, false, forceSetParams);
}

void display::sPreview()
{
  sPreview(ParameterList());
}

void display::sPreview(ParameterList pParams, bool forceSetParams)
{
  _data->print(pParams, true, forceSetParams);
}

void display::sFillList()
{
  sFillList(ParameterList());
}

void display::sFillList(ParameterList pParams, bool forceSetParams)
{
  emit fillListBefore();
  if (forceSetParams || !pParams.count())
  {
    if (!setParams(pParams))
      return;
  }
  int itemid = _data->_list->id();
  bool ok = true;
  QString errorString;
  MetaSQLQuery mql = MQLUtil::mqlLoad(_data->metasqlGroup, _data->metasqlName, errorString, &ok);
  if(!ok)
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Information"),
                         errorString, __FILE__, __LINE__);
    return;
  }
  XSqlQuery xq = mql.toQuery(pParams, QSqlDatabase(), false);

  QString column;
  QVariant param;
  bool valid;

  foreach (QVariant columnid, _data->_charidstext)
  {
    column = QString("char%1").arg(columnid.toString());
    param = pParams.value(column, &valid);
    if (valid)
      xq.bindValue(QString(":%1").arg(column), param.toString());
  }

  foreach (QVariant columnid, _data->_charidslist)
  {
    column = QString("char%1").arg(columnid.toString());
    param = pParams.value(column, &valid);
    if (valid)
    {
      QStringList list = param.toStringList();
      for (int j = 0; j < list.count(); j++)
        xq.bindValue(QString(":%1_%2").arg(column).arg(j), list.at(j));
    }
  }

  foreach (QVariant columnid, _data->_charidsdate)
  {
    // Look for start date
    column = QString("char%1startDate").arg(columnid.toString());
    param = pParams.value(column, &valid);
    if (valid)
      xq.bindValue(QString(":%1").arg(column), param.toString());

    // Look for end date
    column = QString("char%1endDate").arg(columnid.toString());
    param = pParams.value(column, &valid);
    if (valid)
      xq.bindValue(QString(":%1").arg(column), param.toString());
  }

  xq.exec();

  _data->_list->populate(xq, itemid, _data->_useAltId);
  if (xq.lastError().type() != QSqlError::NoError)
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Information"),
                           xq, __FILE__, __LINE__);
    return;
  }
  emit fillListAfter();
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
