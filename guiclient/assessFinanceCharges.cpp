/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "assessFinanceCharges.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
//#include <QStatusBar>
#include <parameter.h>
#include <openreports.h>

/*
 *
 *
 */
assessFinanceCharges::assessFinanceCharges(QWidget* parent, const char* name, Qt::WFlags fl)
{
    setupUi(this);
    _assessmentDate->setDate(QDate::currentDate());
    _invoiceList->setColumnCount(4);

    //setup filtering variables
    QString statementCycle;
    QDate assessmentDate;
    QString customerIdRange;

    XSqlQuery query;
    query.exec("SELECT * FROM fincharg LIMIT 1;");
    if(query.first())
    {
        _assessCycleFrom->setText(rstrip(query.value(8).toString()));
        _customerIdFrom->setText(rstrip(query.value(9).toString()));
        _customerIdTo->setText(rstrip(query.value(10).toString()));
        _assessCycleTo->setText(rstrip(query.value(11).toString()));
    }

    //setup table
    tableHeader<<"Assess Charge"<<"Customer"<<"Overdue Balance"<<"Finance Charge";
    _invoiceList->setHorizontalHeaderLabels(tableHeader);
    _invoiceList->verticalHeader()->setVisible(false);
    _invoiceList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _invoiceList->setSelectionBehavior(QAbstractItemView::SelectRows);
    _invoiceList->setSelectionMode(QAbstractItemView::SingleSelection);

    //setup slots
    connect(_invoiceList, SIGNAL( cellDoubleClicked (int, int) ),
             this, SLOT( cellSelected( int, int ) ) );
    connect(_markAllInvoices,SIGNAL(clicked()), this,SLOT(markAllInvoices_clicked()));
    connect(_unmarkAllInvoices,SIGNAL(clicked()), this,SLOT(unmarkAllInvoice_clicked()));
    connect(_assessCharges,SIGNAL(clicked()), this,SLOT(assessCharges_clicked()));
    connect(_filterCharges,SIGNAL(clicked()), this,SLOT(filterCharges_clicked()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
assessFinanceCharges::~assessFinanceCharges()
{
  // no need to delete child widgets, Qt does it all for us
}

void assessFinanceCharges::filterCharges_clicked()
{
    _invoiceList->clearContents();

    //set last fc in db
    XSqlQuery query;
    QString command = "UPDATE fincharg SET fincharg_lastfc_statementcyclefrom='" + _assessCycleFrom->text() + "' WHERE fincharg_id='1';";
    query.exec(command);
    command = "UPDATE fincharg SET fincharg_lastfc_statementcycleto='" + _assessCycleTo->text() + "' WHERE fincharg_id='1';";
    query.exec(command);
    command = "UPDATE fincharg SET fincharg_lastfc_custidfrom='" + _customerIdFrom->text() + "' WHERE fincharg_id='1';";
    query.exec(command);
    command = "UPDATE fincharg SET fincharg_lastfc_custidto='" + _customerIdTo->text() + "' WHERE fincharg_id='1';";
    query.exec(command);

    /*
      GET CONFIGURATION SETTINGS FOR FINANCE CHARGES
    */
    double minimumCharge;
    QString gracePeriod;
    bool assessOnOverdue;
    int calcFromId;
    QString markOnInvoice;
    double annualInterestRate;

    query.exec("SELECT * FROM fincharg LIMIT 1;");
    if(query.first())
    {
        minimumCharge = query.value(1).toDouble();
        gracePeriod = query.value(2).toString();
        assessOnOverdue = query.value(3).toBool();
        calcFromId = query.value(4).toInt();
        markOnInvoice = query.value(5).toString();
        annualInterestRate = query.value(6).toDouble();
    }
    else
    {
        QMessageBox::information(this,"WARNING", "You may experience unintended results, if you do not configure finance charge settings under Accounting -> Setup -> Finance Charges");
    }
    /*
      END GET CONFIGURATION SETTINGS FOR FINANCE CHARGES
    */

    QString customerIdFrom = _customerIdFrom->text();
    QString customerIdTo = _customerIdTo->text();
    QString statementCycleFrom = _assessCycleFrom->text();
    QString statementCycleTo = _assessCycleTo->text();

    command = QString("SELECT * FROM aropen ") +
            "INNER JOIN custinfo ON custinfo.cust_id=aropen.aropen_cust_id " +
            "WHERE (aropen_duedate + interval '" + gracePeriod + "' day) < NOW() " +
            "AND (('" + customerIdFrom + "' = '') OR (cust_number >= '" + customerIdFrom + "')) " +
            "AND (('" + customerIdTo + "' = '') OR (cust_number <= '" + customerIdTo + "')) " +
            "AND (('" + statementCycleFrom + "' = '') OR (cust_statementcycle >= '" + statementCycleFrom + "')) " +
            "AND (('" + statementCycleTo + "' = '') OR (cust_statementcycle <= '" + statementCycleTo + "'));";

    //QMessageBox::information(this,"SQL", command);

    query.exec(command);
    if(query.first())
    {
        _invoiceList->setRowCount(query.size());
        int i = 0;
        while(query.next())
        {
            QTableWidgetItem *item2 = new QTableWidgetItem();
            item2->setCheckState(Qt::Unchecked);

            //calculate finance charge divide interest rate by 100 to get percentage value
            double charge = (query.value(9).toDouble() * (annualInterestRate/100));
            char buffer[256];  // make sure this is big enough!!!
            snprintf(buffer, sizeof(buffer), "%.2f", charge);

            _invoiceList->setItem(i,0,item2);
            _invoiceList->setItem(i,1,new QTableWidgetItem(query.value(34).toString()));
            _invoiceList->setItem(i,2,new QTableWidgetItem(query.value(9).toString()));
            _invoiceList->setItem(i,3,new QTableWidgetItem(buffer));
            i++;
        }
    }
    else
    {
        _invoiceList->setRowCount(0);
        QMessageBox::information(this,tr("Information"),tr("No records were found matching that criteria!"));
    }
}

void assessFinanceCharges::cellSelected(int nRow, int nCol)
{
    QTableWidgetItem *twi = _invoiceList->item(nRow, 0);
    if(twi->checkState() == Qt::Checked)
    {
        twi->setCheckState(Qt::Unchecked);
    }
    else
    {
        twi->setCheckState(Qt::Checked);
    }

    /*
        QMessageBox::information(this, "",
                            "Cell at row "+QString::number(nRow)+
                             " column "+QString::number(nCol)+
                             " was double clicked.");
                             */
}

void assessFinanceCharges::markAllInvoices_clicked()
{
    for(int i = 0; i < _invoiceList->rowCount()-1; i++)
    {
        QTableWidgetItem *twi = _invoiceList->item(i,0);
        twi->setCheckState(Qt::Checked);
    }
}

void assessFinanceCharges::unmarkAllInvoice_clicked()
{
    for(int i = 0; i < _invoiceList->rowCount()-1; i++)
    {
        QTableWidgetItem *twi = _invoiceList->item(i,0);
        twi->setCheckState(Qt::Unchecked);
    }
}

void assessFinanceCharges::assessCharges_clicked()
{
    for(int i = 0; i < _invoiceList->rowCount()-1; i++)
    {
        QTableWidgetItem *twi = _invoiceList->item(i,0);
        QTableWidgetItem *custName = _invoiceList->item(i,1);
        QTableWidgetItem *amount = _invoiceList->item(i,3);

        if(twi->checkState() == Qt::Checked)
        {
            postFinanceCharge(custName->text(),amount->text());
        }
    }
    QMessageBox::information(this, "MESSAGE",
                        "ASSESSING FINANCE CHARGES!");
}

void assessFinanceCharges::postFinanceCharge(QString customerName, QString payment)
{
    XSqlQuery query;
    query.exec("SELECT fincharg_glaccnt FROM fincharg LIMIT 1;");
    int glaccnt;
    if(query.first())
    {
        glaccnt = query.value(0).toInt();
    }

    //Crmacct.crmacct_id -> Cntct.cntct_addr_id -> addr.addr_id
    query.exec("SELECT custinfo.cust_id,crmacct.crmacct_cntct_id_1 FROM custinfo INNER JOIN crmacct ON crmacct.crmacct_cust_id=custinfo.cust_id WHERE custinfo.cust_name = '" + customerName + "';");
    int customerId;
    QString contactId;
    if(query.first())
    {
        customerId = query.value("cust_id").toInt();
        contactId = query.value("crmacct_cntct_id_1").toString();
    }

    query.exec("SELECT * FROM cntct INNER JOIN addr ON cntct.cntct_addr_id=addr.addr_id WHERE cntct.cntct_addr_id='" + contactId + "';");

    QString billToname, billToAddr1, billToAddr2, billToAddr3, billToCity, billToState, billToZip, billToCountry, billToPhone;
    if(query.first())
    {
        billToname = query.value("cntct_first_name").toString() + " " + query.value("cntct_last_name").toString();
        billToAddr1 = query.value("addr_line1").toString();
        billToAddr2 = query.value("addr_line2").toString();
        billToAddr3 = query.value("addr_line3").toString();
        billToCity = query.value("addr_city").toString();
        billToState = query.value("addr_state").toString();
        billToZip = query.value("addr_postalcode").toString();
        billToCountry = query.value("addr_country").toString();
        billToPhone = query.value("cntct_phone").toString();
    }

    query.exec("SELECT NEXTVAL('invchead_invchead_id_seq') AS invchead_id;");


    int invoiceId;
    if (query.first())
    {
      invoiceId = query.value("invchead_id").toInt();
    }
    QString invoiceNo = QString("FINCHARG") + invoiceId;

    query.prepare("INSERT INTO invchead ("
              "    invchead_id, invchead_invcnumber, invchead_orderdate,"
              "    invchead_invcdate, invchead_cust_id, invchead_posted,"
              "    invchead_printed, invchead_commission, invchead_freight,"
              "    invchead_misc_amount, invchead_shipchrg_id "
              ") VALUES ("
              "    :invchead_id, :invchead_invcnumber, :invchead_orderdate, "
              "    :invchead_invcdate, -1, false,"
              "    false, 0, 0,"
              "    0, -1"
              ");");
    query.bindValue(":invchead_id",	 invoiceId);
    query.bindValue(":invchead_invcnumber",invoiceNo);
    query.bindValue(":invchead_orderdate", _assessmentDate->date());
    query.bindValue(":invchead_invcdate",	 _assessmentDate->date());
    query.exec();
    if (query.lastError().type() != QSqlError::NoError)
    {
      systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    XSqlQuery invoiceave;

    XSqlQuery rollbackq;
    rollbackq.prepare("ROLLBACK;");

    XSqlQuery begin("BEGIN;");

    invoiceave.prepare( "UPDATE invchead "
           "SET invchead_cust_id=:invchead_cust_id,"
           "    invchead_invcdate=:invchead_invcdate,"
           "    invchead_invcnumber=:invchead_invcnumber,"
           "    invchead_ordernumber=:invchead_ordernumber,"
           "    invchead_orderdate=:invchead_orderdate,"
           "    invchead_billto_name=:invchead_billto_name, invchead_billto_address1=:invchead_billto_address1,"
           "    invchead_billto_address2=:invchead_billto_address2, invchead_billto_address3=:invchead_billto_address3,"
           "    invchead_billto_city=:invchead_billto_city, invchead_billto_state=:invchead_billto_state,"
           "    invchead_billto_zipcode=:invchead_billto_zipcode, invchead_billto_phone=:invchead_billto_phone,"
           "    invchead_billto_country=:invchead_billto_country,"
           "    invchead_misc_amount=:invchead_payment,"
           "    invchead_misc_accnt_id=:invchead_glaccnt "
           "WHERE (invchead_id=:invchead_id);" );

    invoiceave.bindValue(":invchead_id",			invoiceId);
    invoiceave.bindValue(":invchead_invcnumber",		invoiceNo);
    invoiceave.bindValue(":invchead_cust_id",		customerId);
    invoiceave.bindValue(":invchead_invcdate",		_assessmentDate->date());
    invoiceave.bindValue(":invchead_billto_name",		billToname);
    invoiceave.bindValue(":invchead_billto_address1",	billToAddr1);
    invoiceave.bindValue(":invchead_billto_address2",	billToAddr2);
    invoiceave.bindValue(":invchead_billto_address3",	billToAddr3);
    invoiceave.bindValue(":invchead_billto_city",		billToCity);
    invoiceave.bindValue(":invchead_billto_state",		billToState);
    invoiceave.bindValue(":invchead_billto_zipcode",	billToZip);
    invoiceave.bindValue(":invchead_billto_country",	billToCountry);
    invoiceave.bindValue(":invchead_billto_phone",		billToPhone);
    invoiceave.bindValue(":invchead_payment",	payment);
    invoiceave.bindValue(":invchead_glaccnt",	glaccnt);

    invoiceave.exec();
    if (invoiceave.lastError().type() != QSqlError::NoError)
    {
      rollbackq.exec();
      systemError(this, invoiceave.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    XSqlQuery commitq("COMMIT;");
    this->close();
}

QString assessFinanceCharges::rstrip(const QString& str) {
    int n = str.size() - 1;
    for (; n >= 0; --n) {
      if (!str.at(n).isSpace()) {
        return str.left(n + 1);
      }
    }
    return "";
}
