/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "unpostedGlSeries.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
//#include <QStatusBar>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>

#include "failedPostList.h"
#include "getGLDistDate.h"
#include "glSeries.h"
#include "storedProcErrorLookup.h"

unpostedGlSeries::unpostedGlSeries(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_delete,   SIGNAL(clicked()),		this, SLOT(sDelete()));
  connect(_edit,     SIGNAL(clicked()),		this, SLOT(sEdit()));
  connect(_glseries, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_new,	     SIGNAL(clicked()),		this, SLOT(sNew()));
  connect(_post,     SIGNAL(clicked()),		this, SLOT(sPost()));
  connect(_print,    SIGNAL(clicked()),		this, SLOT(sPrint()));
  connect(_view,     SIGNAL(clicked()),		this, SLOT(sView()));
  connect(omfgThis,  SIGNAL(glSeriesUpdated()),	this, SLOT(sFillList()));

  _glseries->addColumn(tr("Date"),          _dateColumn,     Qt::AlignCenter, true,  "glseries_distdate" );
  _glseries->addColumn(tr("Source"),        _orderColumn,    Qt::AlignCenter, true,  "glseries_source" );
  _glseries->addColumn(tr("Doc. Type"),     _docTypeColumn,  Qt::AlignCenter, true,  "glseries_doctype" );
  _glseries->addColumn(tr("Doc. #"),        _orderColumn,    Qt::AlignCenter, true,  "glseries_docnumber" );
  _glseries->addColumn(tr("Reference"),     -1,              Qt::AlignLeft,   true,  "glseries_notes"   );
  _glseries->addColumn(tr("Account"),       -1,              Qt::AlignLeft,   true,  "account"   );
  _glseries->addColumn(tr("Debit"),         _moneyColumn,    Qt::AlignRight,  true,  "debit"  );
  _glseries->addColumn(tr("Credit"),        _moneyColumn,    Qt::AlignRight,  true,  "credit"  );

  sFillList();
}

unpostedGlSeries::~unpostedGlSeries()
{
    // no need to delete child widgets, Qt does it all for us
}

void unpostedGlSeries::languageChange()
{
    retranslateUi(this);
}

void unpostedGlSeries::sPrint()
{
  orReport report("UnpostedGlSeries");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void unpostedGlSeries::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  glSeries *newdlg = new glSeries();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void unpostedGlSeries::sEdit()
{
  QList<QTreeWidgetItem*>selected = _glseries->selectedItems();
  removeDupAltIds(selected);
  for (int i = 0; i < selected.size(); i++)
  {
    ParameterList params;
    params.append("mode",	"edit");
    params.append("glSequence",	((XTreeWidgetItem*)(selected[i]))->altId() );

    glSeries *newdlg = new glSeries();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void unpostedGlSeries::sDelete()
{
  if (QMessageBox::question(this, tr("Cancel G/L Transactions?"),
			    tr("<p>Are you sure you want to delete these "
			       "unposted G/L Transactions?"),
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    q.prepare( "DELETE FROM glseries "
	       "WHERE (glseries_sequence=:id);");
    QList<QTreeWidgetItem*>selected = _glseries->selectedItems();
    removeDupAltIds(selected);
    for (int i = 0; i < selected.size(); i++)
    {
      q.bindValue(":id", ((XTreeWidgetItem*)(selected[i]))->altId() );
      q.exec();
      if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
    omfgThis->sGlSeriesUpdated();
  }
}

void unpostedGlSeries::sView()
{
  QList<QTreeWidgetItem*>selected = _glseries->selectedItems();
  removeDupAltIds(selected);
  for (int i = 0; i < selected.size(); i++)
  {
    ParameterList params;
    params.append("mode",	"view");
    params.append("glSequence",	((XTreeWidgetItem*)(selected[i]))->altId() );

    glSeries *newdlg = new glSeries();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void unpostedGlSeries::sPost()
{
  QDate newDate = QDate::currentDate();

  QList<QTreeWidgetItem*>selected = _glseries->selectedItems();
  removeDupAltIds(selected);
  QList<QTreeWidgetItem*>triedToClosed;

  XSqlQuery post;
  post.prepare("SELECT postGLSeriesNoSumm(:sequence) AS result;");

  bool tryagain = false;
  do {
    for (int i = 0; i < selected.size(); i++)
    {
      int id = ((XTreeWidgetItem*)(selected[i]))->altId();

      post.bindValue(":sequence", id);
      post.exec();
      if (post.first())
      {
	int result = post.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("postGLSeriesNoSumm", result),
		      __FILE__, __LINE__);
	  continue;
	}
      }
      // contains() string is hard-coded in stored procedure
      else if (post.lastError().databaseText().contains("post to closed period"))
      {
	triedToClosed.append(selected[i]);
      }
      else if (post.lastError().type() != QSqlError::None)
      {
	systemError(this, post.lastError().databaseText(), __FILE__, __LINE__);
      }
    } // for each selected line

    if (triedToClosed.size() > 0)
    {
      failedPostList newdlg(this, "", true);
      newdlg.sSetList(triedToClosed, _glseries->headerItem(), _glseries->header());
      tryagain = (newdlg.exec() == XDialog::Accepted);
      selected = triedToClosed;
      triedToClosed.clear();
    }
  } while (tryagain);

  omfgThis->sGlSeriesUpdated();
}

void unpostedGlSeries::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit G/L Series..."),  this, SLOT(sEdit()));
  menuItem = pMenu->insertItem(tr("View G/L Series..."),  this, SLOT(sView()));
  menuItem = pMenu->insertItem(tr("Delete G/L Series..."),this, SLOT(sDelete()));

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Post G/L Series..."),  this, SLOT(sPost()));

  pMenu->insertSeparator();
}

void unpostedGlSeries::sFillList()
{
  XSqlQuery fillq;
  fillq.prepare("SELECT *, "
                "       (formatGLAccount(glseries_accnt_id) || ' - ' || accnt_descrip) AS account,"
                "       CASE WHEN (glseries_amount < 0) THEN (glseries_amount * -1)"
                "            ELSE 0 END AS debit,"
                "       CASE WHEN (glseries_amount >= 0) THEN (glseries_amount)"
                "            ELSE 0 END AS credit,"
                "       'curr' AS debit_xtnumericrole,"
                "       'curr' AS credit_xtnumericrole,"
                "       CASE WHEN (glseries_amount < 0) THEN '' END AS credit_qtdisplayrole,"
                "       CASE WHEN (glseries_amount >= 0) THEN '' END AS debit_qtdisplayrole "
                "FROM glseries, accnt "
                "WHERE (glseries_accnt_id=accnt_id) "
                "ORDER BY glseries_distdate, glseries_sequence, glseries_amount;");
  fillq.exec();
  _glseries->populate(fillq, true);
  if (fillq.lastError().type() != QSqlError::None)
  {
    systemError(this, fillq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void unpostedGlSeries::removeDupAltIds(QList<QTreeWidgetItem*> & list)
{
  for (int i = 0; i < list.size(); i++)
  {
    for (int j = i + 1; j < list.size(); j++)
    {
      if (((XTreeWidgetItem*)(list[i]))->altId() ==
	  ((XTreeWidgetItem*)(list[j]))->altId())
	list.removeAt(j);
    }
  }
}
