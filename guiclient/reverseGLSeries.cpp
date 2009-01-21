/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "reverseGLSeries.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a reverseGLSeries as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
reverseGLSeries::reverseGLSeries(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_cancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
reverseGLSeries::~reverseGLSeries()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void reverseGLSeries::languageChange()
{
    retranslateUi(this);
}

void reverseGLSeries::init()
{
  _glseries = -1;
}

enum SetResponse reverseGLSeries::set( const ParameterList & pParams )
{
  QVariant param;
  bool valid = false;
  
  param = pParams.value("glseries", &valid);
  if(valid)
  {
    _glseries = param.toInt();
    
    q.prepare("SELECT gltrans_journalnumber, gltrans_date "
              "FROM gltrans "
              "WHERE (gltrans_sequence=:glseries); " );
    q.bindValue(":glseries", _glseries);
    q.exec();
    if(q.first())
    {
      _journalNum->setText(q.value("gltrans_journalnumber").toString());
      _distDate->setDate(q.value("gltrans_date").toDate());
    }
    else
    {
      systemError( this, tr("A System Error occurred at reverseGLSeries::%1.")
                       .arg(__LINE__) ); 
      return UndefinedError;
    }
  }
  
  return NoError;
}

void reverseGLSeries::sPost()
{
  if(!_distDate->isValid())
  {
    QMessageBox::warning(this, tr("Cannot Reverse Series"),
                         tr("A valid distribution date must be entered before the G/L Series can be reversed.") );
    _distDate->setFocus();
    return;
  }

  if(_metrics->boolean("MandatoryGLEntryNotes") && _notes->toPlainText().trimmed().isEmpty())
  {
    QMessageBox::information( this, tr("Cannot Post G/L Series"),
                                    tr("You must enter some Notes to describe this transaction.") );
    _notes->setFocus();
    return;
  }

  
  q.prepare("SELECT reverseGLSeries(:glseries, :distdate, :notes) AS result;");
  q.bindValue(":glseries", _glseries);
  q.bindValue(":distdate", _distDate->date());
  q.bindValue(":notes", _notes->toPlainText());
  q.exec();
  if(q.first())
  {
    int result = q.value("result").toInt();
    if(result < 0)
    {
      switch(result)
      {
      case -1:
      default:
        QMessageBox::warning( this, tr("Error Reversing G/L Series"),
                              tr("An Unknown Error was encountered while reversing the G/L Series.") );
      }
      return;
    }
  }
  else
    systemError( this, tr("A System Error occurred at reverseGLSeries::%1.")
                       .arg(__LINE__) ); 
  
  accept();
}

