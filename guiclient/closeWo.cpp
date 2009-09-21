/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "closeWo.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "inputManager.h"
#include "dspWoEffortByWorkOrder.h"

closeWo::closeWo(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    connect(_closeWo, SIGNAL(clicked()), this, SLOT(sCloseWo()));

    _captive = FALSE;

    omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

    _wo->setType(cWoOpen | cWoExploded | cWoReleased | cWoIssued);
    _cmnttype->setType(XComboBox::AllCommentTypes);
    _commentGroup->setEnabled(_postComment->isChecked());
    
    if (!_metrics->boolean("Routings"))
    {
      _postLaborVariance->setChecked(FALSE);
      _postLaborVariance->hide();
    }
    else
      _postLaborVariance->setChecked(_metrics->boolean("PostLaborVariances"));
    _postMaterialVariance->setChecked(_metrics->boolean("PostMaterialVariances"));
}

closeWo::~closeWo()
{
    // no need to delete child widgets, Qt does it all for us
}

void closeWo::languageChange()
{
    retranslateUi(this);
}

enum SetResponse closeWo::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _wo->setId(param.toInt());
    _wo->setReadOnly(TRUE);
    _closeWo->setFocus();
  }

  return NoError;
}

void closeWo::sCloseWo()
{
  XSqlQuery type;
  type.prepare( "SELECT item_type "
	            "FROM item,itemsite,wo "
			    "WHERE ((wo_id=:wo_id) "
			    "AND (wo_itemsite_id=itemsite_id) "
			    "AND (itemsite_item_id=item_id)); ");
  type.bindValue(":wo_id", _wo->id());
  type.exec();
  if (type.first())
  {
    if (type.value("item_type").toString() == "J")
	{
      QMessageBox::critical( this, tr("Invalid Work Order"),
                           tr("Work Orders of Item Type Job are posted when shipping \n"
						      "the Sales Order they are associated with.") );
	  clear();
	  return;
	}
	else
	{
	  q.prepare( "SELECT wo_qtyrcv, womatl_issuemethod, womatl_qtyiss "
				 "FROM wo, womatl "
				 "WHERE ( (womatl_wo_id=wo_id)"
				 " AND (wo_id=:wo_id) );" );
	  q.bindValue(":wo_id", _wo->id());
	  q.exec();
	  if (q.first())
	  {
		if (q.value("wo_qtyrcv").toDouble() == 0.0)
		  QMessageBox::warning( this, tr("No Production Posted"),
								tr( "<p>There has not been any Production received from this Work Order.\n"
									"This probably means Production Postings for this Work Order have been overlooked.\n" ) );

		bool unissuedMaterial = FALSE;
		bool unpushedMaterial = FALSE;
		do
		{
		  if ( (!unissuedMaterial) &&
			   (q.value("womatl_issuemethod") == "S") &&
			   (q.value("womatl_qtyiss").toDouble() == 0.0) )
		  {
			QMessageBox::warning( this, tr("Unissued Push Items"),
								  tr( "<p>The selected Work Order has Material Requirements that are Push-Issued\n"
									  "but have not had any material issued to them.  This probably means that\n"
									  "required manual material issues have been overlooked." ) );
			unissuedMaterial = TRUE;
		  }
		  else if ( (!unpushedMaterial) &&
					( (q.value("womatl_issuemethod") == "L") ||
					  (q.value("womatl_issuemethod") == "M") ) &&
					(q.value("womatl_qtyiss").toDouble() == 0.0) )
		  {
			QMessageBox::warning( this, tr("Unissued Pull Items"),
								  tr( "<p>The selected Work Order has Material Requirements that are Pull-Issued\n"
									  "but have not had any material issued to them.  This probably means that Production\n"
									  "was posted for this Work Order through posting Operations.  The BOM for this\n"
									  "Item should be modified to list Used At selections for each BOM Item." ) );
			unpushedMaterial = TRUE;
		  }
		}
		while (q.next());
	  }
	  else if (q.lastError().type() != QSqlError::NoError)
	  {
		systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
		return;
	  }

	  if (_metrics->boolean("Routings"))
	  {
		q.prepare("SELECT count(*) AS countClockins "
  			"FROM wotc "
			"WHERE ((wotc_timein IS NOT NULL)"
			"  AND  (wotc_timeout IS NULL)"
			"  AND  (wotc_wo_id=:wo_id));");
		q.bindValue(":wo_id", _wo->id());
		if (q.exec() && q.first() && q.value("countClockins").toInt() > 0)
		{
		  if (_privileges->check("ViewWoTimeClock") || _privileges->check("MaintainWoTimeClock"))
		  {
			if (QMessageBox::question(this, tr("Users Still Clocked In"),
		  			tr("<p>This Work Order still has %1 user(s) clocked in.\n"
					   "Have those users clock out before closing this "
					   "Work Order.\nWould you like to see the time clock "
					   "data for this Work Order?")
					  .arg(q.value("countClockins").toString()),
					QMessageBox::Yes | QMessageBox::Default,
					QMessageBox::No) == QMessageBox::Yes)
			{
			  ParameterList p;
			  p.append("wo_id", _wo->id());

			 dspWoEffortByWorkOrder* newdlg = new dspWoEffortByWorkOrder();
			 newdlg->set(p);
			 omfgThis->handleNewWindow(newdlg);
			 close();
			}
		  }
		  else
			QMessageBox::critical(this, tr("Users Still Clocked In"),
					tr("<p>This Work Order still has %1 user(s) clocked in.\n"
					   "Have those users clock out before closing this "
					   "Work Order.")
					  .arg(q.value("countClockins").toString()));

		return;
		}
		else if (q.lastError().type() != QSqlError::NoError)
		{
		 systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
		 return;
		}
	  }

	  if (QMessageBox::information( this, tr("Close Work Order"),
									tr("<p>Are you sure you want to close this Work Order?"),
									tr("Close &Work Order"), tr("&Cancel"), 0, 0, 1 ) == 0)
          {
            q.prepare("SELECT closeWo(:wo_id, :postMatVar, :postLaborVar);");
            q.bindValue(":wo_id", _wo->id());
            q.bindValue(":postMatVar",   QVariant(_postMaterialVariance->isChecked()));
            q.bindValue(":postLaborVar", QVariant(_postLaborVariance->isChecked()));
            q.exec();
            if (q.lastError().type() != QSqlError::NoError)
            {
              systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
              return;
            }

            if (_postComment->isChecked())
            {
              q.prepare("SELECT postComment(:cmnttype_id, 'W', :wo_id, :comment) AS _result");
              q.bindValue(":cmnttype_id", _cmnttype->id());
              q.bindValue(":wo_id", _wo->id());
              q.bindValue(":comment", _comment->toPlainText());
              q.exec();
              if (q.lastError().type() != QSqlError::NoError)
              {
                systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
                return;
              }
            }

            omfgThis->sWorkOrdersUpdated(_wo->id(), TRUE);

            if (_captive)
              close();
            else
            {
              clear();
            }
          }
	}
  }
  else
  {
	systemError( this, tr("A System Error occurred at closeWo::%1, Work Order ID #%2.")
		 .arg(__LINE__)
		 .arg(_wo->id()) );
	return;
  }
}

void closeWo::clear()
{
  _wo->setId(-1);
  _close->setText(tr("&Close"));
  _wo->setFocus();
}
