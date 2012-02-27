/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printMulticopyDocument.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "distributeInventory.h"
#include "errorReporter.h"
#include "storedProcErrorLookup.h"

class printMulticopyDocumentPrivate : public Ui::printMulticopyDocument
{
  public:
    printMulticopyDocumentPrivate(::printMulticopyDocument *parent,
                                  QString postPrivilege   = QString()) :
      _alert(true),
      _captive(false),
      _docid(-1),
      _parent(parent),
      _postPrivilege(postPrivilege),
      _printer(0),
      _mpIsInitialized(false)
    {
      setupUi(_parent);
      _printer = new QPrinter(QPrinter::HighResolution);

      _print->setFocus();
    }

    ~printMulticopyDocumentPrivate()
    {
      if (_printer)
      {
        delete _printer;
        _printer = 0;
      }
    }

    bool                    _alert;
    bool                    _captive;
    int                     _docid;
    ::printMulticopyDocument *_parent;
    QString                 _postPrivilege;
    QPrinter               *_printer;
    bool                    _mpIsInitialized;
};

printMulticopyDocument::printMulticopyDocument(QWidget *parent,
                                               const char *name,
                                               bool        modal,
                                               Qt::WFlags  fl)
  : XDialog(parent, name, modal, fl)
{
  _data = new printMulticopyDocumentPrivate(this);

  connect(_data->_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _distributeInventory = false;
}

printMulticopyDocument::printMulticopyDocument(QString numCopiesMetric,
                                               QString watermarkMetric,
                                               QString showPriceMetric,
                                               QString postPrivilege,
                                               QWidget    *parent,
                                               const char *name,
                                               bool        modal,
                                               Qt::WFlags  fl)
  : XDialog(parent, name, modal, fl)
{
  _data = new printMulticopyDocumentPrivate(this, postPrivilege);

  _data->_copies->setWatermarkMetric(watermarkMetric);
  _data->_copies->setShowPriceMetric(showPriceMetric);
  _data->_copies->setNumCopiesMetric(numCopiesMetric);

  connect(_data->_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _distributeInventory = false;

  setPostPrivilege(postPrivilege);
}

printMulticopyDocument::~printMulticopyDocument()
{
  if (_data)
  {
    delete _data;
    _data = 0;
  }
}

void printMulticopyDocument::languageChange()
{
  _data->retranslateUi(this);
}

enum SetResponse printMulticopyDocument::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _data->_captive = true;

  QVariant param;
  bool     valid;

  param = pParams.value("docid", &valid);
  if (valid)
    setId(param.toInt());

  if (pParams.inList("print"))
  {
    sPrint();
    return NoError_Print;
  }

  if (pParams.inList("persistentPrint"))
  {
    _data->_alert = FALSE;

    if (_data->_mpIsInitialized)
    {
      sPrint();
      return NoError_Print;
    }
    else
      return Error_NoSetup;
  }

  return NoError;
}

void printMulticopyDocument::sPrint()
{
  if (! isOkToPrint())
    return;

  ParameterList docinfop;
  docinfop.append("docid", _data->_docid);

  MetaSQLQuery docinfom(_docinfoQueryString);
  MetaSQLQuery markPrintedm(_markPrintedQry);
  MetaSQLQuery askm(_askBeforePostingQry);
  MetaSQLQuery errm(_errCheckBeforePostingQry);
  MetaSQLQuery postm(_postQuery);

  bool mpStartedInitialized = _data->_mpIsInitialized;

  if (! _data->_mpIsInitialized)
  {
    bool userCanceled = false;
    if (orReport::beginMultiPrint(_data->_printer, userCanceled) == false)
    {
      if(!userCanceled)
        systemError(this, tr("Could not initialize printing system for multiple reports."));
      return;
    }
  }

  XSqlQuery docinfoq = docinfom.toQuery(docinfop);
  while (docinfoq.next())
  {
    QString reportname = docinfoq.value("reportname").toString();
    QString docnumber  = docinfoq.value("docnumber").toString();
    bool    printedOK  = false;

    for (int i = 0; i < _data->_copies->numCopies(); i++)
    {
      ParameterList params = getParams(i);

      orReport report(reportname, params);
      if (!report.isValid())
        QMessageBox::critical(this, tr("Cannot Find Form"),
                              tr("<p>Cannot find form '%1' for %2 %3."
                                 "It cannot be printed until the Form"
                                 "Assignment is updated to remove references "
                                 "to this Form or the Form is created.")
                               .arg(reportname, _doctypefull, docnumber));
          
      else if (report.print(_data->_printer, ! _data->_mpIsInitialized))
      {
        _data->_mpIsInitialized = true;
        printedOK = true;
      }
      else
      {
        ErrorReporter::error(QtCriticalMsg, this,
                             tr("A Printing Error occurred"),
                             tr("A Printing Error occurred"),
                             __FILE__, __LINE__);
        continue;
      }
    }

    if (printedOK)
      emit finishedPrinting(_data->_docid);

    if (! _markPrintedQry.isEmpty())
    {
      XSqlQuery markPrintedq = markPrintedm.toQuery(docinfop);
      ErrorReporter::error(QtCriticalMsg, this, tr("Database Error"),
                           markPrintedq, __FILE__, __LINE__); // don't return

      if (_data->_alert)
        emit docUpdated(_data->_docid);
    }

    if (! _postQuery.isEmpty() && _data->_post->isChecked())
    {
      if (! _askBeforePostingQry.isEmpty())
      {
        XSqlQuery askq = askm.toQuery(docinfop);
        if (ErrorReporter::error(QtCriticalMsg, this,
                                 tr("Cannot Post %1").arg(docnumber),
                                 askq, __FILE__, __LINE__))
          continue;
        else if (askq.value("ask").toBool() &&
                 QMessageBox::question(this, tr("Post Anyway?"),
                                       _askBeforePostingMsg.arg(docnumber),
                                        QMessageBox::Yes,
                                        QMessageBox::No | QMessageBox::Default)
                    == QMessageBox::No)
          continue;
      }

      if (! _errCheckBeforePostingQry.isEmpty())
      {
        XSqlQuery errq = errm.toQuery(docinfop);
        if (ErrorReporter::error(QtCriticalMsg, this,
                                 tr("Cannot Post %1").arg(docnumber),
                                 errq, __FILE__, __LINE__))
          continue;
        else if (! errq.first() || ! errq.value("ok").toBool())
        {
          ErrorReporter::error(QtCriticalMsg, this,
                               tr("Cannot Post %1").arg(docnumber),
                               _errCheckBeforePostingMsg, __FILE__, __LINE__);
          continue;
        }
      }

      //TODO: find a way to do this without holding locks during user input
      XSqlQuery("BEGIN;");

      XSqlQuery rollback;
      rollback.prepare("ROLLBACK;");

      XSqlQuery postq = postm.toQuery(docinfop);
      if (postq.first())
      {
        int result = postq.value("result").toInt();
        if (result < 0)
        {
          rollback.exec();
          ErrorReporter::error(QtCriticalMsg, this,
                               tr("Cannot Post %1").arg(docnumber),
                               storedProcErrorLookup(_postFunction, result),
                               __FILE__, __LINE__);
          continue;
        }
        if (_distributeInventory &&
            (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected))
        {
          rollback.exec();
          QMessageBox::information(this, tr("Posting Canceled"),
                                   tr("Transaction Canceled") );
          continue;
        }
      }
      else if (postq.lastError().type() != QSqlError::NoError)
      {
        rollback.exec();
        ErrorReporter::error(QtCriticalMsg, this,
                             tr("Cannot Post %1").arg(docnumber),
                             postq, __FILE__, __LINE__);
        continue;
      }

      XSqlQuery("COMMIT;");

      emit posted(_data->_docid);
    }
  }

  if (! mpStartedInitialized)
  {
    orReport::endMultiPrint(_data->_printer);
    _data->_mpIsInitialized = false;
  }

  if (_data->_captive)
    accept();
  else
    clear();

  if (ErrorReporter::error(QtCriticalMsg, this, tr("Cannot Print"),
                           docinfoq, __FILE__, __LINE__))
    return;
}

void printMulticopyDocument::populate()
{
  ParameterList getp;
  getp.append("docid", _data->_docid);

  MetaSQLQuery getm(_docinfoQueryString);
  XSqlQuery getq = getm.toQuery(getp);
  if (getq.first())
  {
    if (getq.value("posted").toBool())
    {
      _data->_post->setChecked(false);
      _data->_post->hide();
    }
    else
      _data->_post->setVisible(! _data->_postPrivilege.isEmpty());
    emit gotDocInfo(&getq.record());
  }
}

XDocCopySetter *printMulticopyDocument::copies()
{
  return _data->_copies;
}

void printMulticopyDocument::clear()
{
}

ParameterList printMulticopyDocument::getParams(int row)
{
  ParameterList params;
  params.append(_reportKey, _data->_docid);
  params.append("showcosts", (_data->_copies->showCosts(row) ? "TRUE" : "FALSE"));
  params.append("watermark", _data->_copies->watermark(row));

  return params;
}

int printMulticopyDocument::id()
{
  return _data->_docid;
}

bool printMulticopyDocument::isOkToPrint()
{
  return true;
}

bool printMulticopyDocument::isSetup()
{
  return _data->_mpIsInitialized;
}

QWidget *printMulticopyDocument::optionsWidget()
{
  return _data->_optionsFrame;
}

void printMulticopyDocument::setId(int docid)
{
  _data->_docid = docid;
  populate();
}

void printMulticopyDocument::setNumCopiesMetric(QString metric)
{
  _data->_copies->setNumCopiesMetric(metric);
}

void printMulticopyDocument::setPostPrivilege(QString priv)
{
  _data->_postPrivilege = priv;

  _data->_post->setVisible(! priv.isEmpty());
  _data->_post->setChecked(_privileges->check(priv));
  _data->_post->setEnabled(_privileges->check(priv));
}

void printMulticopyDocument::setSetup(bool pSetup)
{
  _data->_mpIsInitialized = pSetup;
}

void printMulticopyDocument::setShowPriceMetric(QString metric)
{
  _data->_copies->setShowPriceMetric(metric);
}

void printMulticopyDocument::setWatermarkMetric(QString metric)
{
  _data->_copies->setWatermarkMetric(metric);
}
