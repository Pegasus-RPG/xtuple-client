/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "crmaccountMergeResultPage.h"

#include "crmaccount.h"
#include "errorReporter.h"

class CrmaccountMergeResultPagePrivate
{
  public:
    CrmaccountMergeResultPagePrivate(QWizardPage *parent)
      : _parent(parent)
    {
      _crmaccount = new crmaccount();
      _crmaccount->_save->setVisible(false);
      _crmaccount->_close->setVisible(false);

      ParameterList params;
      params.append("mode", "view");
      _crmaccount->set(params);

      QWidget *sa = _parent->findChild<QWidget*>("_resultScrollAreaContents");
      QLayout *lyt = sa ? sa->layout() : 0;
      if (lyt)
        lyt->addWidget(_crmaccount);
      else
        ErrorReporter::error(QtWarningMsg, _parent,
                             QT_TRANSLATE_NOOP("CrmaccountMergeResultPage",
                                               "Could not draw CRM Account"),
                             QT_TRANSLATE_NOOP("CrmaccountMergeResultPage",
                                "Could not find the portion of the window "
                                "in which to draw the target CRM Account."));
    };

    crmaccount *_crmaccount;
    QWidget    *_parent;
};

CrmaccountMergeResultPage::CrmaccountMergeResultPage(QWidget *parent)
  : QWizardPage(parent),
    _data(0)
{
  setupUi(this);

  _data = new CrmaccountMergeResultPagePrivate(this);
}

CrmaccountMergeResultPage::~CrmaccountMergeResultPage()
{
  if (_data)
    delete _data;
}

void CrmaccountMergeResultPage::initializePage()
{
  XSqlQuery idq;
  idq.prepare("SELECT crmacct_id"
              "  FROM crmacct"
              " WHERE (crmacct_number=:number);");
  idq.bindValue(":number", field("_target"));
  idq.exec();
  if (idq.first())
    _data->_crmaccount->setId(idq.value("crmacct_id").toInt());
  else if (ErrorReporter::error(QtCriticalMsg, this,
                                tr("Error Getting CRM Account"),
                                idq, __FILE__, __LINE__))
    return;
  else
    _data->_crmaccount->setId(-1);
}
