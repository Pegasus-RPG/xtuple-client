/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printCreditMemos.h"

#include <QVariant>
#include <QValidator>
#include <QMessageBox>
#include <QApplication>
#include <parameter.h>
#include <openreports.h>
#include "guiclient.h"
#include "editICMWatermark.h"

/*
 *  Constructs a printCreditMemos as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
printCreditMemos::printCreditMemos(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_creditMemoNumOfCopies, SIGNAL(valueChanged(int)), this, SLOT(sHandleCopies(int)));
    connect(_creditMemoWatermarks, SIGNAL(itemSelected(int)), this, SLOT(sEditWatermark()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
printCreditMemos::~printCreditMemos()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void printCreditMemos::languageChange()
{
    retranslateUi(this);
}


void printCreditMemos::init()
{
  _creditMemoWatermarks->addColumn( tr("Copy #"),      _dateColumn, Qt::AlignCenter );
  _creditMemoWatermarks->addColumn( tr("Watermark"),   -1,          Qt::AlignLeft   );
  _creditMemoWatermarks->addColumn( tr("Show Prices"), _dateColumn, Qt::AlignCenter );
  
  _creditMemoNumOfCopies->setValue(_metrics->value("CreditMemoCopies").toInt());
  if (_creditMemoNumOfCopies->value())
  {
    for (int i = 0; i < _creditMemoWatermarks->topLevelItemCount(); i++)
    {
      _creditMemoWatermarks->topLevelItem(i)->setText(1, _metrics->value(QString("CreditMemoWatermark%1").arg(i)));
      _creditMemoWatermarks->topLevelItem(i)->setText(2, ((_metrics->value(QString("CreditMemoShowPrices%1").arg(i)) == "t") ? tr("Yes") : tr("No")));
    }
  }

  if(!_privileges->check("PostARDocuments"))
  {
    _post->setChecked(false);
    _post->setEnabled(false);
  }
  
  _print->setFocus();
}

void printCreditMemos::sPrint()
{
  XSqlQuery cmhead( "SELECT cmhead_id, cmhead_number,"
                    "       findCustomerForm(cmhead_cust_id, 'C') AS _reportname "
                    "FROM ( SELECT cmhead_id, cmhead_number, cmhead_cust_id "
                    "       FROM cmhead "
                    "       WHERE ( (NOT cmhead_hold)"
                    "         AND   (NOT cmhead_printed) ) ) AS data "
                    "WHERE (checkCreditMemoSitePrivs(cmhead_id));" );
  if (cmhead.first())
  {
    QPrinter printer(QPrinter::HighResolution);
    bool     setupPrinter  = TRUE;
    int      itemlocSeries = 0;
    bool userCanceled = false;
    if (orReport::beginMultiPrint(&printer, userCanceled) == false)
    {
      if(!userCanceled)
        systemError(this, tr("Could not initialize printing system for multiple reports."));
      return;
    }

    do
    {
      for (int i = 0; i < _creditMemoWatermarks->topLevelItemCount(); i++ )
      {
        ParameterList params;

        params.append("cmhead_id", cmhead.value("cmhead_id").toInt());
        params.append("showcosts", ((_creditMemoWatermarks->topLevelItem(i)->text(2) == tr("Yes")) ? "TRUE" : "FALSE"));
        params.append("watermark", _creditMemoWatermarks->topLevelItem(i)->text(1));

        orReport report(cmhead.value("_reportname").toString(), params);
        if (!report.isValid())
          QMessageBox::critical( this, tr("Cannot Find Credit Memo Form"),
                                 tr( "The Credit Memo Form '%1' for Credit Memo #%2 cannot be found.\n"
                                     "This Credit Memo cannot be printed until Customer Form Assignments are updated to remove\n"
                                     "any references to this Credit Memo Form or this Credit Memo Form is created." )
                                 .arg(cmhead.value("_reportname").toString())
                                 .arg(cmhead.value("cmhead_number").toString()) );
        else
        {
          if (report.print(&printer, setupPrinter))
            setupPrinter = FALSE;
          else
          {
            systemError( this, tr("A Printing Error occurred at printCreditMemos::%1.")
                               .arg(__LINE__) );
	    orReport::endMultiPrint(&printer);
            return;
          }
        }

        if (_post->isChecked())
        {
          XSqlQuery postCreditMemo;
          postCreditMemo.prepare("SELECT postCreditMemo(:cmhead_id, :itemlocSeries) AS result;");
          postCreditMemo.bindValue(":cmhead_id", cmhead.value("cmhead_id").toInt());
          postCreditMemo.bindValue(":itemlocSeries", itemlocSeries);
          postCreditMemo.exec();
          if (postCreditMemo.first())
            itemlocSeries = postCreditMemo.value("result").toInt();
          else
            systemError(this, tr("A System Error occurred at %1::%2.")
                              .arg(__FILE__)
                              .arg(__LINE__) );
        }
      }
    }
    while (cmhead.next());
    orReport::endMultiPrint(&printer);

    if ( QMessageBox::information( this, tr("Mark Credit Memos as Printed?"),
                                   tr("Did all of the Credit Memos print correctly?"),
                                   tr("&Yes"), tr("&No"), QString::null, 0, 1) == 0)
      XSqlQuery().exec( "UPDATE cmhead "
                        "SET cmhead_printed=TRUE "
                        "WHERE (NOT cmhead_printed);" );

    omfgThis->sCreditMemosUpdated();
  }
  else
    QMessageBox::information( this, tr("No Credit Memos to Print"),
                              tr("There aren't any Credit Memos to print.") );

  accept();
}

void printCreditMemos::sHandleCopies(int pValue)
{
  if (_creditMemoWatermarks->topLevelItemCount() > pValue)
    _creditMemoWatermarks->takeTopLevelItem(_creditMemoWatermarks->topLevelItemCount() - 1);
  else
  {
    for (int i = (_creditMemoWatermarks->topLevelItemCount() + 1); i <= pValue; i++)
      new XTreeWidgetItem(_creditMemoWatermarks,
			  _creditMemoWatermarks->topLevelItem(_creditMemoWatermarks->topLevelItemCount() - 1),
			  i, i,
			  tr("Copy #%1").arg(i), "", tr("Yes"));
  }
}

void printCreditMemos::sEditWatermark()
{
  QTreeWidgetItem *cursor = _creditMemoWatermarks->currentItem();
  ParameterList params;
  params.append("watermark", cursor->text(1));
  params.append("showPrices", (cursor->text(2) == tr("Yes")));

  editICMWatermark newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted)
  {
    cursor->setText(1, newdlg.watermark());
    cursor->setText(2, ((newdlg.showPrices()) ? tr("Yes") : tr("No")));
  }
}

