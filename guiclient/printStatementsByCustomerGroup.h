#ifndef PRINTSTATEMENTSBYCUSTOMERGROUP
#define PRINTSTATEMENTSBYCUSTOMERGROUP

#include "printSinglecopyDocument.h"
#include "ui_printStatementsByCustomerGroup.h"


class printStatementsByCustomerGroup : public printSinglecopyDocument,
                                       public Ui::statementByCustGroup
{
  Q_OBJECT

  public:
    printStatementsByCustomerGroup(QWidget *parent = 0, const char *name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~printStatementsByCustomerGroup();

    Q_INVOKABLE virtual ParameterList getParams(XSqlQuery *docq);
    Q_INVOKABLE virtual ParameterList getParams();

  public slots:
    virtual void clear();
    virtual void sPopulate(XSqlQuery *docq);

  protected slots:
    virtual void languageChange();
};
#endif // PRINTSTATEMENTSBYCUSTOMERGROUP

