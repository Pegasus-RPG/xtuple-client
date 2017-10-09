/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "glSeries.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "glSeriesItem.h"
#include "storedProcErrorLookup.h"
#include "submitAction.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"

#define cPostStandardJournal 0x10

#define DEBUG false

glSeries::glSeries(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _post = _buttonBox->addButton(tr("&Post"), QDialogButtonBox::ActionRole);

  connect(_delete,	SIGNAL(clicked()),	this, SLOT(sDelete()));
  connect(_edit,	SIGNAL(clicked()),	this, SLOT(sEdit()));
  connect(_new,		SIGNAL(clicked()),	this, SLOT(sNew()));
  connect(_post,	SIGNAL(clicked()),	this, SLOT(sPost()));
  connect(_buttonBox,	SIGNAL(accepted()),	this, SLOT(sSave()));

  _glseries->addColumn(tr("Account"), -1,           Qt::AlignLeft,  true,  "account"  );
  _glseries->addColumn(tr("Debit"),   _moneyColumn, Qt::AlignRight, true,  "debit" );
  _glseries->addColumn(tr("Credit"),  _moneyColumn, Qt::AlignRight, true,  "credit" );

  _credits->setPrecision(omfgThis->moneyVal());
  _debits->setPrecision(omfgThis->moneyVal());
  _diff->setPrecision(omfgThis->moneyVal());

  _source->setText("G/L");
  _source->setEnabled(false);

  _doctype->addItem("AD");
  _doctype->addItem("CD");
  _doctype->addItem("CK");
  _doctype->addItem("CM");
  _doctype->addItem("CR");
  _doctype->addItem("CT");
  _doctype->addItem("DM");
  _doctype->addItem("DS");
  _doctype->addItem("IN");
  _doctype->addItem("JE");
  _doctype->addItem("MM");
  _doctype->addItem("PO");
  _doctype->addItem("SO");
  _doctype->addItem("ST");
  _doctype->addItem("VO");
  _doctype->addItem("WO");
  _doctype->setCurrentIndex(_doctype->findText("JE"));
  _documents->setType("JE");

  _submit = false;
  _journal = 0;
}

glSeries::~glSeries()
{
  // no need to delete child widgets, Qt does it all for us
}

void glSeries::languageChange()
{
  retranslateUi(this);
}

enum SetResponse glSeries::set(const ParameterList &pParams)
{
  XSqlQuery glet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  _submit = pParams.inList("submit");
  if((_submit)  && (_metrics->boolean("EnableBatchManager")))
    _post->setText(tr("Schedule"));
  else
    _submit = false;

  param = pParams.value("glSequence", &valid);
  if (valid)
  {
    _glsequence = param.toInt();
    _documents->setId(_glsequence);
    glet.prepare("SELECT DISTINCT glseries_distdate, glseries_source,"
	      "                glseries_doctype,  glseries_docnumber,"
	      "                glseries_notes"
              "  FROM glseries"
              " WHERE (glseries_sequence=:glseries_sequence);" );
    glet.bindValue(":glseries_sequence", _glsequence);
    glet.exec();
    if(glet.first())
    {
      _date->setDate(glet.value("glseries_distdate").toDate());
      _source->setText(glet.value("glseries_source").toString());
      int idx = _doctype->findText(glet.value("glseries_doctype").toString());
      if(idx < 0)
        _doctype->addItem(glet.value("glseries_doctype").toString());
      _doctype->setCurrentIndex(_doctype->findText(glet.value("glseries_doctype").toString()));
      _docnumber->setText(glet.value("glseries_docnumber").toString());
      _notes->setText(glet.value("glseries_notes").toString());
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving G/L Series Information"),
                                  glet, __FILE__, __LINE__))
    {
      return UndefinedError;
    }
    sFillList();
  }

  param = pParams.value("journalnumber", &valid);
  if (valid)
  {
    _journal = param.toInt();
    _buttonBox->removeButton(_buttonBox->button(QDialogButtonBox::Save));
    _mode = cEdit;
    _doctype->setCurrentIndex(_doctype->findText("JE"));
    _doctype->setEnabled(false);
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      glet.exec("SELECT fetchGLSequence() AS glsequence;");
      if (glet.first())
      {
        _glsequence = glet.value("glsequence").toInt();
        _documents->setId(_glsequence);
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving G/L Series Information"),
                                    glet, __FILE__, __LINE__))
      {
        return UndefinedError;
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
    else if (param.toString() == "postStandardJournal")
    {
      _mode = cPostStandardJournal;
      
      _doctype->setCurrentIndex(_doctype->findText("ST"));
      _doctype->setEnabled(false);
      _date->setEnabled(false);
      _notes->setEnabled(false);

      glet.prepare( "SELECT DISTINCT glseries_docnumber, stdjrnl_notes "
                 "  FROM glseries, stdjrnl "
                 " WHERE ( (stdjrnl_name=glseries_docnumber) "
                 "   AND   (stdjrnl_notes IS NOT NULL) "
                 "   AND   (stdjrnl_notes != '') "
                 "   AND   (glseries_sequence=:glsequence) ); ");
      glet.bindValue(":glsequence", _glsequence);
      glet.exec();
      if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving G/L Series Information"),
                                    glet, __FILE__, __LINE__))
      {
        return UndefinedError;
      }
      while(glet.next())
        _notes->append(glet.value("glseries_docnumber").toString() + ": " + glet.value("stdjrnl_notes").toString() + "\n\n");
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _new->setEnabled(false);
      disconnect(_glseries, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      disconnect(_glseries, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      _source->setEnabled(false);
      _doctype->setEnabled(false);
      _docnumber->setEnabled(false);
      _date->setEnabled(false);
      _notes->setEnabled(false);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void glSeries::sNew()
{

  QList<GuiErrorCheck> errors;
    errors<< GuiErrorCheck(!_date->isValid(), _date,
                           tr("You must enter a Distribution Date for this Series."))
    ;
    if (GuiErrorCheck::reportErrors(this, tr("Cannot Maintain G/L Series"), errors))
      return;

  ParameterList params;
  params.append("mode", "new");
  params.append("doctype", _doctype->currentText());
  params.append("docnumber", _docnumber->text());
  params.append("glSequence", _glsequence);
  params.append("distDate", _date->date());

  if (_mode == cPostStandardJournal)
    params.append("postStandardJournal");

  glSeriesItem newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
  {
    sFillList();
    sNew();
  }
}

void glSeries::sEdit()
{

  QList<GuiErrorCheck> errors;
    errors<< GuiErrorCheck(!_date->isValid(), _date,
                           tr("You must enter a Distribution Date for this Series."))
    ;
    if (GuiErrorCheck::reportErrors(this, tr("Cannot Maintain G/L Series"), errors))
      return;

  ParameterList params;
  params.append("mode", "edit");
  params.append("glseries_id", _glseries->id());
  params.append("glSequence", _glsequence);
  params.append("distDate", _date->date());

  if (_mode == cPostStandardJournal)
    params.append("postStandardJournal");

  glSeriesItem newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void glSeries::sDelete()
{
  XSqlQuery glDelete;
  glDelete.prepare( "DELETE FROM glseries "
             "WHERE (glseries_id=:glseries_id);" );
  glDelete.bindValue(":glseries_id", _glseries->id());
  glDelete.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Selected G/L Series"),
                                glDelete, __FILE__, __LINE__))
  {
    return;
  }

  sFillList();
}

bool glSeries::update()
{
  XSqlQuery glupdate;

  QList<GuiErrorCheck> errors;
    errors<< GuiErrorCheck(!_date->isValid(), _date,
                           tr("You must enter a Distribution Date for this Series."))
          << GuiErrorCheck(_metrics->boolean("MandatoryGLEntryNotes") && _notes->toPlainText().trimmed().isEmpty(), _notes,
                           tr("You must enter some Notes to describe this transaction."))
    ;
    if (GuiErrorCheck::reportErrors(this, tr("Cannot Post G/L Series"), errors))
      return false;

// Do not save notes when posting std journal
  if (_mode == cPostStandardJournal)
    glupdate.prepare( "UPDATE glseries "
               "SET glseries_source=:source,"
               "    glseries_doctype=:doctype,"
               "    glseries_docnumber=:docnumber,"
               "    glseries_distdate=:glseries_distdate "
               "WHERE (glseries_sequence=:glseries_sequence);" );
  else
    glupdate.prepare( "UPDATE glseries "
               "SET glseries_notes=:glseries_notes, "
               "    glseries_source=:source,"
               "    glseries_doctype=:doctype,"
               "    glseries_docnumber=:docnumber,"
               "    glseries_distdate=:glseries_distdate "
               "WHERE (glseries_sequence=:glseries_sequence);" );
  glupdate.bindValue(":glseries_notes", _notes->toPlainText());
  glupdate.bindValue(":source",	_source->text());
  glupdate.bindValue(":doctype",	_doctype->currentText());
  glupdate.bindValue(":docnumber",	_docnumber->text());
  glupdate.bindValue(":glseries_sequence", _glsequence);
  glupdate.bindValue(":glseries_distdate", _date->date());
  glupdate.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Selected G/L Series"),
                                glupdate, __FILE__, __LINE__))
  {
    return false;
  }

  glupdate.prepare("SELECT SUM(glseries_amount) AS result "
            "  FROM glseries "
            " WHERE (glseries_sequence=:glseries_sequence); ");
  glupdate.bindValue(":glseries_sequence", _glsequence);
  glupdate.exec();
  if(glupdate.first())
  {
    double result = glupdate.value("result").toDouble();
    if(result != 0)
    {
      QMessageBox::critical( this, tr("Cannot Post G/L Series"),
			     tr("<p>The G/L Series information is unbalanced and cannot be posted. Please correct this before continuing.") );
      return false;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Selected G/L Series"),
                                glupdate, __FILE__, __LINE__))
  {
    return false;
  }

  omfgThis->sGlSeriesUpdated();
  return true;
}

void glSeries::sSave()
{
  if (! update())
    return;
  accept();
}

void glSeries::sPost()
{
  XSqlQuery glPost;
  if (! update())
    return;

  if(_submit)
  {
    ParameterList params;

    params.append("action_name", "PostGLSeries");
    params.append("glseries_sequence", _glsequence); 

    submitAction newdlg(this, "", true);
    newdlg.set(params);

    if(newdlg.exec() == XDialog::Accepted)
    {
      // TODO: do something?
    }
  }
  else
  {
    glPost.prepare("SELECT postGLSeriesNoSumm(:glseries_sequence,COALESCE(:journal,fetchJournalNumber('G/L'))) AS return;");
    glPost.bindValue(":glseries_sequence", _glsequence);
    if (_journal)
      glPost.bindValue(":journal", _journal);
    glPost.exec();
    if (glPost.first())
    {
      int returnVal = glPost.value("return").toInt();
      if (returnVal < 0)
      {
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Selected G/L Series"),
                               storedProcErrorLookup("postGLSeriesNoSumm", returnVal),
                               __FILE__, __LINE__);
        return;
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Selected G/L Series"),
                                  glPost, __FILE__, __LINE__))
    {
      return;
    }

    omfgThis->sGlSeriesUpdated();
  }

  if(cPostStandardJournal == _mode || _journal)
  {
    accept();
    return;
  }

  _date->clear();
  _notes->clear();
  _debits->clear();
  _credits->clear();
  _diff->clear();
  _glseries->clear();
  _docnumber->clear();
  
  ParameterList params;
  params.append("mode", "new");
  set(params);
}

void glSeries::reject()
{
  XSqlQuery glreject;
  if (DEBUG)
    qDebug("glSeries::reject() entered with _mode %d, topLevelItemCount %d",
           _mode, _glseries->topLevelItemCount());
  if (cNew == _mode &&
      _glseries->topLevelItemCount() > 0)
  {
    if (QMessageBox::question(this, tr("Delete G/L Series?"),
                              tr("<p>Are you sure you want to delete this G/L "
                                 "Series Entry?"),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) == QMessageBox::Yes)
    {
      glreject.prepare("SELECT deleteGLSeries(:glsequence);");
      glreject.bindValue(":glsequence", _glsequence);
      glreject.exec();
      if (glreject.lastError().type() != QSqlError::NoError)
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Removing G/L Series"),
                           glreject, __FILE__, __LINE__);
    }
    else
      return;
  }

  omfgThis->sGlSeriesUpdated();

  XDialog::reject();
}

void glSeries::sFillList()
{
  XSqlQuery glFillList;
  glFillList.prepare( "SELECT glseries_id, (formatGLAccount(accnt_id) || '-' || accnt_descrip) AS account,"
             "       CASE WHEN (glseries_amount < 0) THEN (glseries_amount * -1)"
             "            ELSE 0"
             "       END AS debit,"
             "       CASE WHEN (glseries_amount > 0) THEN glseries_amount"
             "            ELSE 0"
             "       END AS credit,"
             "       'curr' AS debit_xtnumericrole,"
             "       'curr' AS credit_xtnumericrole "
             "FROM glseries, accnt "
             "WHERE ( (glseries_accnt_id=accnt_id)"
             " AND (glseries_sequence=:glseries_sequence) );" );
  glFillList.bindValue(":glseries_sequence", _glsequence);
  glFillList.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving G/L Series Information"),
                                glFillList, __FILE__, __LINE__))
  {
    return;
  }
  _glseries->populate(glFillList);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving G/L Series Information"),
                                glFillList, __FILE__, __LINE__))
  {
    return;
  }

  glFillList.prepare("SELECT SUM(CASE WHEN (glseries_amount < 0) THEN (glseries_amount * -1)"
            "                              ELSE 0"
            "                         END) AS debit,"
            "       SUM(CASE WHEN (glseries_amount > 0) THEN glseries_amount"
            "                              ELSE 0"
            "                         END ) AS credit,"
            "       SUM(glseries_amount) AS diff,"
            "       (SUM(glseries_amount) <> 0) AS oob "
            "FROM glseries "
            "WHERE (glseries_sequence=:glseries_sequence);" );
  glFillList.bindValue(":glseries_sequence", _glsequence);
  glFillList.exec();
  if (glFillList.first())
  {
    _debits->setDouble(glFillList.value("debit").toDouble());
    _credits->setDouble(glFillList.value("credit").toDouble());
    _diff->setDouble(glFillList.value("diff").toDouble());

    QString stylesheet;
    if (glFillList.value("oob").toBool())
        stylesheet = QString("* { color: %1; }").arg(namedColor("error").name());

    _debits->setStyleSheet(stylesheet);
    _credits->setStyleSheet(stylesheet);
    _diff->setStyleSheet(stylesheet);
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving G/L Series Information"),
                                glFillList, __FILE__, __LINE__))
  {
    return;
  }
}
