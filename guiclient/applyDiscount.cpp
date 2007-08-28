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
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
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

#include "applyDiscount.h"

#include <QVariant>
#include <QMessageBox>
#include <QVariant>
#include <QSqlError>

/*
 *  Constructs a applyDiscount as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
applyDiscount::applyDiscount(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_cancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_apply, SIGNAL(clicked()), this, SLOT(sApply()));
  connect(_amount, SIGNAL(idChanged(int)), _owed, SLOT(setId(int)));
  connect(_amount, SIGNAL(effectiveChanged(const QDate&)), _owed, SLOT(setEffective(const QDate&)));
  connect(_amount, SIGNAL(idChanged(int)), _applieddiscounts, SLOT(setId(int)));
  connect(_amount, SIGNAL(effectiveChanged(const QDate&)), _applieddiscounts, SLOT(setEffective(const QDate&)));

  _apopenid = -1;
}

/*
 *  Destroys the object and frees any allocated resources
 */
applyDiscount::~applyDiscount()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void applyDiscount::languageChange()
{
  retranslateUi(this);
}

enum SetResponse applyDiscount::set( const ParameterList & pParams )
{
  QVariant param;
  bool     valid;

  param = pParams.value("apopen_id", &valid);
  if (valid)
  {
    _apopenid = param.toInt();
    populate();
  }

  param = pParams.value("curr_id", &valid);
  if (valid)
    _amount->setId(param.toInt());

  param = pParams.value("amount", &valid);
  if (valid)
    _amount->setLocalValue(param.toDouble());

  
  return NoError;
}

void applyDiscount::sApply()
{
  accept();
}

void applyDiscount::populate()
{
  q.prepare("SELECT (vend_number|| '-' || vend_name) as f_vend,"
            "       CASE WHEN (apopen_doctype='V') THEN 'Voucher'"
            "            WHEN (apopen_doctype='D') THEN 'Debit Memo'"
            "            ELSE apopen_doctype"
            "       END AS f_doctype,"
            "       apopen_docnumber,"
            "       formatDate(apopen_docdate) AS f_docdate,"
	    "       apopen_docdate, "
            "       (terms_code|| '-' || terms_descrip) AS f_terms,"
            "       formatDate(apopen_docdate + terms_discdays) AS f_discdate,"
            "       formatPrcnt(terms_discprcnt) AS f_discprcnt,"
            "       apopen_amount, apopen_curr_id, applied, "
            "       noNeg(apopen_amount * CASE WHEN (CURRENT_DATE <= (apopen_docdate + terms_discdays)) THEN terms_discprcnt ELSE 0.0 END - applied) AS f_amount,"
            "       ((apopen_docdate + terms_discdays) < CURRENT_DATE) AS past"
            "  FROM apopen LEFT OUTER JOIN terms ON (apopen_terms_id=terms_id),"
	    "       vend, "
            "       (SELECT COALESCE(SUM(apapply_amount),0) AS applied"
            "          FROM apapply, apopen"
            "         WHERE ((apapply_target_apopen_id=:apopen_id)"
            "           AND  (apapply_source_apopen_id=apopen_id)"
            "           AND  (apopen_discount)) ) AS data"
            " WHERE ((apopen_vend_id=vend_id)"
            "   AND  (apopen_id=:apopen_id)); ");
  q.bindValue(":apopen_id", _apopenid);
  q.exec();

  if(q.first())
  {
    _vend->setText(q.value("f_vend").toString());

    _doctype->setText(q.value("f_doctype").toString());
    _docnum->setText(q.value("apopen_docnumber").toString());
    _docdate->setText(q.value("f_docdate").toString());

    _terms->setText(q.value("f_terms").toString());
    _discdate->setText(q.value("f_discdate").toString());
    if(q.value("past").toBool())
      _discdate->setPaletteForegroundColor(QColor("red"));
    _discprcnt->setText(q.value("f_discprcnt").toString());

    _owed->setLocalValue(q.value("apopen_amount").toDouble());
    _applieddiscounts->setLocalValue(q.value("applied").toDouble());

    _amount->set(q.value("f_amount").toDouble(),
		 q.value("apopen_curr_id").toInt(), 
		 q.value("apopen_docdate").toDate(), false);
  }
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
}

