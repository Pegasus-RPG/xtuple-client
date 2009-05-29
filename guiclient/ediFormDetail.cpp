/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "ediFormDetail.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include "metasqlHighlighter.h"

/*
 *  Constructs a ediFormDetail as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
ediFormDetail::ediFormDetail(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_cancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_accept, SIGNAL(clicked()), this, SLOT(sSave()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
ediFormDetail::~ediFormDetail()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void ediFormDetail::languageChange()
{
    retranslateUi(this);
}


void ediFormDetail::init()
{
  _mode = cNew;
  _ediformid = -1;
  _ediformdetailid = -1;

  QTextDocument *document = _query->document();
  _highlighter = new MetaSQLHighlighter(document);
  document->setDefaultFont(QFont("Courier"));
}

enum SetResponse ediFormDetail::set( const ParameterList & pParams )
{
  QVariant param;
  bool     valid;

  param = pParams.value("ediform_id", &valid);
  if (valid)
    _ediformid = param.toInt();

  param = pParams.value("ediformdetail_id", &valid);
  if (valid)
  {
    _ediformdetailid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if("new" == param.toString())
      _mode = cNew;
    else if("edit" == param.toString())
      _mode = cEdit;
  }

  return NoError;
}

void ediFormDetail::sSave()
{
  if(_name->text().trimmed().isEmpty())
  {
    QMessageBox::critical( this, tr("Cannot Save EDI Form Detail"),
      tr("You must enter in a valid name for this form detail.") );
    _name->setFocus();
    return;
  }

  if(_query->toPlainText().trimmed().isEmpty())
  {
    QMessageBox::critical( this, tr("Cannot Save EDI Form Detail"),
      tr("You must enter in a valid query for this form detail.") );
    _query->setFocus();
    return;
  }

  if(cNew == _mode)
    q.prepare("INSERT INTO ediformdetail "
              "(ediformdetail_ediform_id,"
              " ediformdetail_name, ediformdetail_query,"
              " ediformdetail_order, ediformdetail_notes) "
              "VALUES(:ediform_id,"
              " :ediformdetail_name, :ediformdetail_query,"
              " :ediformdetail_order, :ediformdetail_notes);");
  else
    q.prepare("UPDATE ediformdetail"
              "   SET ediformdetail_name=:ediformdetail_name,"
              "       ediformdetail_query=:ediformdetail_query,"
              "       ediformdetail_order=:ediformdetail_order,"
              "       ediformdetail_notes=:ediformdetail_notes "
              " WHERE (ediformdetail_id=:ediformdetail_id); ");

  q.bindValue(":ediform_id", _ediformid);
  q.bindValue(":ediformdetail_id", _ediformdetailid);
  q.bindValue(":ediformdetail_name", _name->text().trimmed());
  q.bindValue(":ediformdetail_query", _query->toPlainText().trimmed());
  q.bindValue(":ediformdetail_notes", _notes->toPlainText());
  q.bindValue(":ediformdetail_order", _order->value());

  if(!q.exec())
  {
    QMessageBox::critical( this, tr("Cannot Save EDI Form Detail"),
      tr("There was a database error preventing this record from being saved.") );
    return;
  }

  accept();
}

void ediFormDetail::populate()
{
  q.prepare("SELECT ediformdetail_name, ediformdetail_order,"
            "       ediformdetail_query, ediformdetail_notes"
            "  FROM ediformdetail"
            " WHERE (ediformdetail_id=:ediformdetail_id); ");
  q.bindValue(":ediformdetail_id", _ediformdetailid);
  q.exec();
  if(q.first())
  {
    _name->setText(q.value("ediformdetail_name").toString());
    _order->setValue(q.value("ediformdetail_order").toInt());
    _query->setText(q.value("ediformdetail_query").toString());
    _notes->setText(q.value("ediformdetail_notes").toString());
  }
}

