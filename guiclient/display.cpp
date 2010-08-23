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
#include "ui_display.h"

#include <QSqlError>
#include <QMessageBox>
#include <QPrinter>
#include <QPrintDialog>

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
    _print->hide(); // hide the print button until a reportName is set
    _preview->hide(); // hide the preview button until a reportName is set
    _autoupdate->hide(); // hide until auto update is enabled
    _parameterWidget->hide(); // hide until user shows manually
    _useAltId = false;
    _autoUpdateEnabled = false;
  }

  void print(bool);

  QString reportName;
  QString metasqlName;
  QString metasqlGroup;

  bool _useAltId;
  bool _autoUpdateEnabled;

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

  connect(_data->_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_data->_preview, SIGNAL(clicked()), this, SLOT(sPreview()));
  connect(_data->_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_data->_list, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*,int)));
  connect(_data->_autoupdate, SIGNAL(toggled(bool)), this, SLOT(sAutoUpdateToggled()));
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

QWidget * display::optionsWidget()
{
  return _data->_optionsFrame;
}

ParameterWidget * display::parameterWidget()
{
  return _data->_parameterWidget;
}

XTreeWidget * display::list()
{
  return _data->_list;
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
