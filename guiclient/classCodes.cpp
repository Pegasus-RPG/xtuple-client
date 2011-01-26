/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "classCodes.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>

#include "classCode.h"
#include "storedProcErrorLookup.h"

#include <xtClassCode.h>
#include <xtStorableQuery.h>
#include <interfaces/xiPropertyObserver.h>

class ClassCodeWidgetItem : public XTreeWidgetItem, public xiPropertyObserver
{
  public:
    ClassCodeWidgetItem(xtClassCode * pcode, XTreeWidget * parent)
      : XTreeWidgetItem(parent, pcode->getId())
    {
      code = pcode;
      if(code)
      {
        setId(code->getId());
        setText(0, QString::fromStdString(xtAnyUtility::toString(code->getCode())));
        setText(1, QString::fromStdString(xtAnyUtility::toString(code->getDescription())));
        code->attachPropertyObserver(this);
      }
    }

   virtual void propertyChanged(xtObject * pcode, const std::string & pname, int role)
   {
     if(pcode)
     {
       if(role == xtlib::ValueRole && pname == "code")
       {
         setText(0, QString::fromStdString(xtAnyUtility::toString(pcode->getProperty("code"))));
       }
       else if(role == xtlib::ValueRole && pname == "description")
       {
         setText(1, QString::fromStdString(xtAnyUtility::toString(pcode->getProperty("description"))));
       }
     }
   }

   xtClassCode * code;
};

classCodes::classCodes(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_deleteUnused, SIGNAL(clicked()), this, SLOT(sDeleteUnused()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));

  if (_privileges->check("MaintainClassCodes"))
  {
    connect(_classcode, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_classcode, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_classcode, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    connect(_classcode, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

    _new->setEnabled(FALSE);
    _deleteUnused->setEnabled(FALSE);
  }

  _classcode->addColumn(tr("Class Code"),  70, Qt::AlignLeft, true, "classcode_code");
  _classcode->addColumn(tr("Description"), -1, Qt::AlignLeft, true, "classcode_descrip");

  sFillList(-1);
}

classCodes::~classCodes()
{
}

void classCodes::languageChange()
{
  retranslateUi(this);
}

void classCodes::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  classCode newdlg(this, "", TRUE);
  newdlg.set(params);
  
  int result = newdlg.exec();
  if (result != XDialog::Rejected)
    sFillList(result);
}

void classCodes::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("classcode_id", _classcode->id());

  classCode newdlg(this, "", TRUE);
  newdlg.set(params);
  
  int result = newdlg.exec();
  if (result != XDialog::Rejected)
    sFillList(result);
}

void classCodes::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("classcode_id", _classcode->id());

  classCode newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void classCodes::sDelete()
{
  try {
    XTreeWidgetItem *ci = _classcode->currentItem();
    if(!ci)
      return;
    ClassCodeWidgetItem * ccwi = (ClassCodeWidgetItem*)ci;
    if(!ccwi)
      return;
    ccwi->code->setDeleted(true);
    ccwi->code->save();
    delete ci;
  }
  catch (std::exception e)
  {
    systemError(this, e.what(), __FILE__, __LINE__);
  }
}

void classCodes::sPrint()
{
  orReport report("ClassCodesMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void classCodes::sDeleteUnused()
{
  if ( QMessageBox::warning( this, tr("Delete Unused Class Codes"),
                             tr("<p>Are you sure that you wish to delete all "
                                "unused Class Codes?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    q.exec("SELECT deleteUnusedClassCodes() AS result;");
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        systemError(this,
                    storedProcErrorLookup("deleteUnusedClassCodes", result),
                    __FILE__, __LINE__);
        return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    sFillList(-1);
  }
}

void classCodes::sFillList(int pId)
{
  _classcode->clear();
  try
  {
    xtClassCode ex;
    xtStorableQuery<xtClassCode> sq(&ex);
    sq.exec();
    std::set<xtClassCode*> codes = sq.result();
    if(!codes.empty())
    {
      for(std::set<xtClassCode*>::const_iterator ci = codes.begin(); ci != codes.end(); ci++)
        new ClassCodeWidgetItem((*ci), _classcode);
      if(pId != -1)
        _classcode->setId(pId);
    }
  }
  catch(std::exception &e)
  {
    QMessageBox::critical(this, "Error", QString("Error querying for Class Codes: %1").arg(e.what()));
  }
}
