/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "postGLTransactionsToExternal.h"

#include <QVariant>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <stdlib.h>

postGLTransactionsToExternal::postGLTransactionsToExternal(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_cancel, SIGNAL(clicked()), this, SLOT(reject()));

  _dates->setStartNull(tr("Always"), omfgThis->startOfTime(), TRUE);
  _dates->setStartCaption(tr("Effective"));
  _dates->setEndNull(tr("Never"), omfgThis->endOfTime(), TRUE);
  _dates->setEndCaption(tr("Expires"));
}

postGLTransactionsToExternal::~postGLTransactionsToExternal()
{
  // no need to delete child widgets, Qt does it all for us
}

void postGLTransactionsToExternal::languageChange()
{
  retranslateUi(this);
}

void postGLTransactionsToExternal::sPost()
{
  if ( QMessageBox::critical( this, tr("Create New ICDIST File?"),
                              tr( "Creating a new Export File will delete the previous Export File.\n"
                                  "You should make sure that the previous Export Files have been imported into RW2000 before Proceeding.\n\n"
                                  "Are you sure that you want to Create a New Export File?" ),
                                  "&Yes", "&No", QString::null, 0, 1  ) != 0 )
    return;

  QFile distFile(_metrics->value("AccountingSystemExportPath") + "/ICDIST00.EXP");
  if (distFile.open(QIODevice::WriteOnly))
  {
    QString sql( "SELECT accnt_profit, accnt_number, accnt_sub,"
                 "       LPAD(SUBSTR(TEXT(gltrans_journalnumber), 1, 4), 4, '0') AS journalnumber,"
                 "       TO_CHAR(gltrans_date, 'YYYYMMDD') AS distdate,"
                 "       usr_initials,"
                 "       TRUNC((SUM(gltrans_amount) * 100), 0) AS amount "
                 "FROM accnt, usr, gltrans "
                 "WHERE ( (gltrans_accnt_id=accnt_id)"
                 " AND (NOT gltrans_exported)"
                 " AND (usr_username=CURRENT_USER)"
                 " AND (gltrans_date BETWEEN :startDate AND :endDate)" );

    if (_sourceModule->currentText() != tr("All"))
      sql += " AND (gltrans_source=:source)";

    sql += ") "
           "GROUP BY accnt_profit, accnt_number, accnt_sub, journalnumber,"
           "         distdate, usr_initials "
           "HAVING (SUM(gltrans_amount) <> 0);";

    q.prepare(sql);
    q.bindValue(":startDate", _dates->startDate());
    q.bindValue(":endDate", _dates->endDate());
    q.bindValue(":source", _sourceModule->currentText());
    q.exec();
    if (q.first())
    {
      QTextStream textStream(&distFile);
      QString     format;

      do
      {
        textStream << "8";                                                                                           //  IC-DIST-TYP (MISC-COSTS-APPLIED)
        if (_metrics->value("AccountingSystem") == "RealWorld91")
        {
          if (_metrics->value("GLProfitSize").toInt() == 0)
            textStream << "00000000";                                                                                //  IC-DIST-PFT-CTR-1
          else
          {
            format = "%0" + QString("%1").arg(_metrics->value("GLProfitSize").toInt()) + "d";
      
            textStream << QString().sprintf(format, q.value("accnt_profit").toInt());                               //  IC-DIST-PFT-CTR-1
            textStream << QString().fill('0', (8 - _metrics->value("GLProfitSize").toInt()));                        //  Filler '0's
          }

          textStream << "00000000";                                                                                  //  IC-DIST-PFT-CTR-2
        }

        format = "%0" + QString("%1").arg(_metrics->value("GLMainSize").toInt()) + "d";
        
        textStream << QString().sprintf(format, q.value("accnt_number").toInt());                                   //  IC-DIST-MAIN-ACCT-NO
        textStream << QString().fill('0', (8 - _metrics->value("GLMainSize").toInt()));                              //  Filler '0's

        if (_metrics->value("GLSubaccountSize").toInt() == 0)
          textStream << "00000000";                                                                                  //  IC-DIST-SUB-ACCT-NO
        else
        {
          format = "%0" + QString("%1").arg(_metrics->value("GLSubaccountSize").toInt()) + "d";
        
          textStream << QString().sprintf(format, q.value("accnt_sub").toInt());                                    //  IC-DIST-SUB-ACCT-NO
          textStream << QString().fill('0', (8 - _metrics->value("GLSubaccountSize").toInt()));                      //  Filler '0's
        }

        textStream << q.value("distdate").toString();                                                               //  IC-DIST-TRX-DAT
        textStream << "IC";                                                                                          //  Journal Code
        textStream << q.value("journalnumber").toString();                                                          //  IC-DIST-JRNL-NO

        if (q.value("usr_initials").toString().length())
          textStream << q.value("usr_initials").toString().leftJustify(3, ' ', TRUE);                               //  IC-DIST-USER-ID
        else
          textStream << "OMG";                                                                                       //  IC-DIST-USER-ID

        textStream << "N";                                                                                           //  IC-DIST-CORR-FLG

        if (q.value("amount").toInt() < 0)
          textStream << QString().sprintf("%014d+", abs(q.value("amount").toInt()));                                //  IC-DIST-AMT
        else
          textStream << QString().sprintf("%014d-", q.value("amount").toInt());                                     //  IC-DIST-AMT

        textStream << "N";                                                                                           //  IC-DIST-POST-TO-GL-FLG

        textStream << "\n";                                                                                          //  EOL
      }
      while (q.next());
    }

    distFile.close();
  }
  else
  {
    QMessageBox::critical( this, tr("Cannot Open Distribution File"),
                           tr( "The G/L Distribution Export file cannot be opened.  Please contact your Systems Adminstrator\n\n"
                               "G/L Transactions were NOT exported." ) );
    return;
  }

  if ( QMessageBox::information( this, tr("Mark Distributions as Posted"),
                                 tr( "A new ICDIST file has been generated in the RealWorld directory.\n"
                                     "You should now use the RealWorld icfu/icutil tool to import this file.\n"
                                     "After you have successfully imported the ICDIST file click the 'Post' button\n"
                                     "to mark these transactions as distributed.\n"
                                     "If, for any reason, you were unable to post the ICDIST file click on the\n"
                                     "'Do Not Post' button and re-export G/L Transactions to re-create the IDDIST file.\n" ),
                                     tr("&Post"), tr("Do &Not Post"), QString::null, 0, 1) == 0)
  {
    q.prepare( "UPDATE gltrans "
               "SET gltrans_exported=TRUE "
               "WHERE ( (NOT gltrans_exported)"
               " AND (gltrans_date BETWEEN :startDate AND :endDate) );" );
    q.bindValue(":startDate", _dates->startDate());
    q.bindValue(":endDate", _dates->endDate());
    q.exec();
  }

  accept();
}

