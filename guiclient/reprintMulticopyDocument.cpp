/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "reprintMulticopyDocument.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "errorReporter.h"
#include "storedProcErrorLookup.h"

class reprintMulticopyDocumentPrivate : public Ui::reprintMulticopyDocument
{
  public:
    reprintMulticopyDocumentPrivate(::reprintMulticopyDocument *parent) :
      _parent(parent),
      _printer(0)
    {
      setupUi(_parent);
      _printer = new QPrinter(QPrinter::HighResolution);

      _print->setFocus();
    }

    ~reprintMulticopyDocumentPrivate()
    {
      if (_printer)
      {
        delete _printer;
        _printer = 0;
      }
    }

    ::reprintMulticopyDocument *_parent;
    QPrinter                   *_printer;
};

reprintMulticopyDocument::reprintMulticopyDocument(QWidget    *parent,
                                                   const char *name,
                                                   bool        modal,
                                                   Qt::WFlags  fl)
  : XDialog(parent, name, modal, fl)
{
  _data = new reprintMulticopyDocumentPrivate(this);

  connect(_data->_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_data->_query, SIGNAL(clicked()), this, SLOT(sPopulate()));
}

reprintMulticopyDocument::reprintMulticopyDocument(QString numCopiesMetric,
                                                   QString watermarkMetric,
                                                   QString showPriceMetric,
                                                   QWidget    *parent,
                                                   const char *name,
                                                   bool        modal,
                                                   Qt::WFlags  fl)
  : XDialog(parent, name, modal, fl)
{
  _data = new reprintMulticopyDocumentPrivate(this);

  _data->_copies->setWatermarkMetric(watermarkMetric);
  _data->_copies->setShowPriceMetric(showPriceMetric);
  _data->_copies->setNumCopiesMetric(numCopiesMetric);

  connect(_data->_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_data->_query, SIGNAL(clicked()), this, SLOT(sPopulate()));
}

reprintMulticopyDocument::~reprintMulticopyDocument()
{
  if (_data)
  {
    delete _data;
    _data = 0;
  }
}

void reprintMulticopyDocument::languageChange()
{
  _data->retranslateUi(this);
}

enum SetResponse reprintMulticopyDocument::set(const ParameterList &pParams)
{
  XDialog::set(pParams);

  return NoError;
}

void reprintMulticopyDocument::sPrint()
{
  bool mpIsInitialized = false;
  QList<QVariant> printedDocs;

  bool userCanceled = false;
  if (orReport::beginMultiPrint(_data->_printer, userCanceled) == false)
  {
    if(!userCanceled)
      systemError(this, tr("Could not initialize printing system for multiple reports."));
    return;
  }

  foreach (XTreeWidgetItem *item, list()->selectedItems())
  {
    emit aboutToStart(item);

    message(tr("Printing %1 #%2").arg(_doctypefull, item->text("docnumber")));

    QString  reportname = item->text("reportname");
    QString  docnumber  = item->text("docnumber");
    bool     printedOK  = false;

    orReport report(reportname);
    if (! report.isValid())
      QMessageBox::critical(this, tr("Cannot Find Form"),
                            tr("<p>Cannot find form '%1' for %2 %3."
                               "It cannot be printed until the Form"
                               "Assignment is updated to remove references "
                               "to this Form or the Form is created.")
                             .arg(reportname, _doctypefull, docnumber));
    else
    {
      for (int i = 0; i < _data->_copies->numCopies(); i++)
      {
        report.setParamList(getParamsOneCopy(i, item));
        if (! report.isValid())
        {
          ErrorReporter::error(QtCriticalMsg, this, tr("Invalid Parameters"),
                               tr("<p>Report '%1' cannot be run. Parameters "
                                   "are missing.").arg(reportname),
                               __FILE__, __LINE__);
          continue;
        }
        else if (report.print(_data->_printer, ! mpIsInitialized))
        {
          mpIsInitialized = true;
          printedOK = true;
        }
        else
        {
          report.reportError(this);
          continue;
        }
      }
    }

    if (printedOK)
    {
      emit finishedPrinting(item->id());
      printedDocs.append(item->id());
    }

    message("");
  }

  orReport::endMultiPrint(_data->_printer);

  if (printedDocs.size() == 0)
    QMessageBox::information(this, tr("No Documents to Print"),
                             tr("There aren't any documents to print."));

  emit finishedWithAll();

  list()->clearSelection();
}

void reprintMulticopyDocument::sPopulate()
{
  ParameterList getp = getParamsDocList();

  MetaSQLQuery getm(_docListQueryString);
  XSqlQuery    getq = getm.toQuery(getp);
  list()->populate(getq);
  ErrorReporter::error(QtCriticalMsg, this, tr("Getting Documents"),
                       getq, __FILE__, __LINE__);
}

XDocCopySetter *reprintMulticopyDocument::copies()
{
  return _data->_copies;
}

ParameterList reprintMulticopyDocument::getParamsDocList()
{
  ParameterList params;

  params.append(_reportKey, list()->id());

  return params;
}

ParameterList reprintMulticopyDocument::getParamsOneCopy(int row,
                                                         XTreeWidgetItem *item)
{
  ParameterList params;
  params.append(_reportKey,  item->id());
  params.append("showcosts", (_data->_copies->showCosts(row) ? "TRUE" : "FALSE"));
  params.append("watermark", _data->_copies->watermark(row));

  return params;
}

XTreeWidget *reprintMulticopyDocument::list()
{
  return _data->_list;
}

QWidget *reprintMulticopyDocument::optionsWidget()
{
  return _data->_optionsFrame;
}

void reprintMulticopyDocument::setNumCopiesMetric(QString metric)
{
  _data->_copies->setNumCopiesMetric(metric);
}

void reprintMulticopyDocument::setShowPriceMetric(QString metric)
{
  _data->_copies->setShowPriceMetric(metric);
}

void reprintMulticopyDocument::setWatermarkMetric(QString metric)
{
  _data->_copies->setWatermarkMetric(metric);
}
