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

#include "sysLocale.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a sysLocale as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
sysLocale::sysLocale(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_dateDBFormat, SIGNAL(lostFocus()), this, SLOT(sBuildDate()));
    connect(_timeDBFormat, SIGNAL(lostFocus()), this, SLOT(sBuildTime()));
    connect(_timestampDBFormat, SIGNAL(lostFocus()), this, SLOT(sBuildTimestamp()));
    connect(_intervalDBFormat, SIGNAL(lostFocus()), this, SLOT(sBuildInterval()));
    connect(_currencyDBFormat, SIGNAL(lostFocus()), this, SLOT(sBuildCurrency()));
    connect(_salesPriceDBFormat, SIGNAL(lostFocus()), this, SLOT(sBuildSalesPrice()));
    connect(_extPriceDBFormat, SIGNAL(lostFocus()), this, SLOT(sBuildExtPrice()));
    connect(_qtyDBFormat, SIGNAL(lostFocus()), this, SLOT(sBuildQty()));
    connect(_qtyPerDBFormat, SIGNAL(lostFocus()), this, SLOT(sBuildQtyPer()));
    connect(_costDBFormat, SIGNAL(lostFocus()), this, SLOT(sBuildCost()));
    connect(_purchPriceDBFormat, SIGNAL(lostFocus()), this, SLOT(sBuildPurchPrice()));
    connect(_uomRatioDBFormat, SIGNAL(lostFocus()), this, SLOT(sBuildUOMRatio()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
sysLocale::~sysLocale()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void sysLocale::languageChange()
{
    retranslateUi(this);
}


void sysLocale::init()
{
}

enum SetResponse sysLocale::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("locale_id", &valid);
  if (valid)
  {
    _localeid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _code->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _description->setFocus();
    }
    else if (param.toString() == "copy")
    {
      _mode = cCopy;

      q.prepare("SELECT copyLocale(:locale_id) AS _locale_id;");
      q.bindValue(":locale_id", _localeid);
      q.exec();
      if (q.first())
      {
        _localeid = q.value("_locale_id").toInt();
	populate();
      }
      else
      {
        systemError(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__) );
        return UndefinedError;
      }

      _code->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _code->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _language->setEnabled(FALSE);
      _dateDBFormat->setEnabled(FALSE);
      _timeDBFormat->setEnabled(FALSE);
      _timestampDBFormat->setEnabled(FALSE);
      _intervalDBFormat->setEnabled(FALSE);
      _currencyDBFormat->setEnabled(FALSE);
      _salesPriceDBFormat->setEnabled(FALSE);
      _purchPriceDBFormat->setEnabled(FALSE);
      _extPriceDBFormat->setEnabled(FALSE);
      _costDBFormat->setEnabled(FALSE);
      _qtyDBFormat->setEnabled(FALSE);
      _qtyPerDBFormat->setEnabled(FALSE);
      _uomRatioDBFormat->setEnabled(FALSE);
      _comments->setReadOnly(TRUE);
      _close->setText(tr("&Close"));
      _save->hide();
      
      _close->setFocus();
    }
  }

  return NoError;
}

void sysLocale::sSave()
{
  if (_code->text().stripWhiteSpace().length() == 0)
  {
    QMessageBox::critical( this, tr("Cannot Save Locale"),
                           tr("You must enter a Code for this Locale before you may save it.") );
    _code->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('locale_locale_id_seq') AS _locale_id");
    if (q.first())
      _localeid = q.value("_locale_id").toInt();

    q.prepare( "INSERT INTO locale "
               "( locale_id, locale_code, locale_descrip, locale_lang,"
               "  locale_dateformat, locale_timeformat,"
               "  locale_timestampformat, locale_intervalformat,"
	       "  locale_currformat,"
               "  locale_salespriceformat, locale_purchpriceformat,"
               "  locale_extpriceformat, locale_costformat,"
               "  locale_qtyformat, locale_qtyperformat, locale_uomratioformat,"
               "  locale_comments ) "
               "VALUES "
               "( :locale_id, :locale_code, :locale_descrip, :locale_lang,"
               "  :locale_dateformat, :locale_timeformat,"
               "  :locale_timestampformat, :locale_intervalformat,"
	       "  :locale_currformat,"
               "  :locale_salespriceformat, :locale_purchpriceformat,"
               "  :locale_extpriceformat, :locale_costformat,"
               "  :locale_qtyformat, :locale_qtyperformat, :locale_uomratioformat,"
               "  :locale_comments );" );
  }
  else if ( (_mode == cEdit) || (_mode == cCopy) )
    q.prepare( "UPDATE locale "
                "SET locale_code=:locale_code, locale_descrip=:locale_descrip, locale_lang=:locale_lang,"
                "    locale_dateformat=:locale_dateformat, locale_timeformat=:locale_timeformat,"
                "    locale_timestampformat=:locale_timestampformat, locale_intervalformat=:locale_intervalformat,"
		"    locale_currformat=:locale_currformat,"
                "    locale_salespriceformat=:locale_salespriceformat, locale_purchpriceformat=:locale_purchpriceformat,"
                "    locale_extpriceformat=:locale_extpriceformat, locale_costformat=:locale_costformat,"
                "    locale_qtyformat=:locale_qtyformat, locale_qtyperformat=:locale_qtyperformat,"
                "    locale_uomratioformat=:locale_uomratioformat,"
                "    locale_comments=:locale_comments "
                "WHERE (locale_id=:locale_id);" );

  q.bindValue(":locale_id", _localeid);
  q.bindValue(":locale_code", _code->text());
  q.bindValue(":locale_descrip", _description->text());
  q.bindValue(":locale_lang", _language->text());
  q.bindValue(":locale_dateformat", _dateDBFormat->text());
  q.bindValue(":locale_timeformat", _timeDBFormat->text());
  q.bindValue(":locale_timestampformat", _timestampDBFormat->text());
  q.bindValue(":locale_intervalformat", _intervalDBFormat->text());
  q.bindValue(":locale_currformat", _currencyDBFormat->text());
  q.bindValue(":locale_salespriceformat", _salesPriceDBFormat->text());
  q.bindValue(":locale_purchpriceformat", _purchPriceDBFormat->text());
  q.bindValue(":locale_extpriceformat", _extPriceDBFormat->text());
  q.bindValue(":locale_costformat", _costDBFormat->text());
  q.bindValue(":locale_qtyformat", _qtyDBFormat->text());
  q.bindValue(":locale_qtyperformat", _qtyPerDBFormat->text());
  q.bindValue(":locale_uomratioformat", _uomRatioDBFormat->text());
  q.bindValue(":locale_comments", _comments->text());
  q.exec();

  done(_localeid);
}

void sysLocale::sClose()
{
  if (_mode == cCopy)
  {
    q.prepare( "DELETE FROM locale "
               "WHERE (locale_id=:locale_id);" );
    q.bindValue(":locale_id", _localeid);
    q.exec();
  }

  reject();
}
 
void sysLocale::sBuildDate()
{
  q.prepare("SELECT LTRIM(TO_CHAR(CURRENT_DATE, :format)) AS result;");
  q.bindValue(":format", _dateDBFormat->text());
  q.exec();
  if (q.first())
    _dateSample->setText(q.value("result").toString());
  else
    _dateSample->setText(tr("Error"));
}

void sysLocale::sBuildTime()
{
  q.exec( QString("SELECT LTRIM(TO_CHAR(CURRENT_TIME::TIME, '%1')) AS result;")
          .arg(_timeDBFormat->text()) );
  if (q.first())
    _timeSample->setText(q.value("result").toString());
  else
    _timeSample->setText(tr("Error"));
}

void sysLocale::sBuildTimestamp()
{
  q.prepare("SELECT LTRIM(TO_CHAR(CURRENT_TIMESTAMP, :format)) AS result;");
  q.bindValue(":format", _timestampDBFormat->text());
  q.exec();
  if (q.first())
    _timestampSample->setText(q.value("result").toString());
  else
    _timestampSample->setText(tr("Error"));
}

void sysLocale::sBuildInterval()
{
  q.prepare("SELECT LTRIM(TO_CHAR(CURRENT_TIMESTAMP - CURRENT_DATE, :format)) AS result;");
  q.bindValue(":format", _intervalDBFormat->text());
  q.exec();
  if (q.first())
    _intervalSample->setText(q.value("result").toString());
  else
    _intervalSample->setText(tr("Error"));
}

void sysLocale::sBuildCurrency()
{
  q.prepare("SELECT LTRIM(TO_CHAR(123456789.123456789, :format)) AS result;");
  q.bindValue(":format", _currencyDBFormat->text());
  q.exec();
  if (q.first())
    _currencySample->setText(q.value("result").toString());
  else
    _currencySample->setText(tr("Error"));
}

void sysLocale::sBuildSalesPrice()
{
  q.prepare("SELECT LTRIM(TO_CHAR(123456789.123456789, :format)) AS result;");
  q.bindValue(":format", _salesPriceDBFormat->text());
  q.exec();
  if (q.first())
    _salesPriceSample->setText(q.value("result").toString());
  else
    _salesPriceSample->setText(tr("Error"));
}

void sysLocale::sBuildPurchPrice()
{
  q.prepare("SELECT LTRIM(TO_CHAR(123456789.123456789, :format)) AS result;");
  q.bindValue(":format", _purchPriceDBFormat->text());
  q.exec();
  if (q.first())
    _purchPriceSample->setText(q.value("result").toString());
  else
    _purchPriceSample->setText(tr("Error"));
}

void sysLocale::sBuildExtPrice()
{
  q.prepare("SELECT LTRIM(TO_CHAR(123456789.123456789, :format)) AS result;");
  q.bindValue(":format", _extPriceDBFormat->text());
  q.exec();
  if (q.first())
    _extPriceSample->setText(q.value("result").toString());
  else
    _extPriceSample->setText(tr("Error"));
}

void sysLocale::sBuildCost()
{
  q.prepare("SELECT LTRIM(TO_CHAR(123456789.123456789, :format)) AS result;");
  q.bindValue(":format", _costDBFormat->text());
  q.exec();
  if (q.first())
    _costSample->setText(q.value("result").toString());
  else
    _costSample->setText(tr("Error"));
}

void sysLocale::sBuildQty()
{
  q.prepare("SELECT LTRIM(TO_CHAR(123456789.123456789, :format)) AS result;");
  q.bindValue(":format", _qtyDBFormat->text());
  q.exec();
  if (q.first())
    _qtySample->setText(q.value("result").toString());
  else
    _qtySample->setText(tr("Error"));
}

void sysLocale::sBuildQtyPer()
{
  q.prepare("SELECT LTRIM(TO_CHAR(123456789.123456789, :format)) AS result;");
  q.bindValue(":format", _qtyPerDBFormat->text());
  q.exec();
  if (q.first())
    _qtyPerSample->setText(q.value("result").toString());
  else
    _qtyPerSample->setText(tr("Error"));
}

void sysLocale::sBuildUOMRatio()
{
  q.prepare("SELECT LTRIM(TO_CHAR(123456789.123456789, :format)) AS result;");
  q.bindValue(":format", _uomRatioDBFormat->text());
  q.exec();
  if (q.first())
    _uomRatioSample->setText(q.value("result").toString());
  else
    _uomRatioSample->setText(tr("Error"));
}

void sysLocale::populate()
{
  q.prepare( "SELECT * "
             "FROM locale "
             "WHERE (locale_id=:locale_id);" );
  q.bindValue(":locale_id", _localeid);
  q.exec();
  if (q.first())
  {
    _code->setText(q.value("locale_code").toString());
    _description->setText(q.value("locale_descrip").toString());
    _language->setText(q.value("locale_lang").toString());
    _dateDBFormat->setText(q.value("locale_dateformat").toString());
    _timeDBFormat->setText(q.value("locale_timeformat").toString());
    _timestampDBFormat->setText(q.value("locale_timestampformat").toString());
    _intervalDBFormat->setText(q.value("locale_intervalformat").toString());
    _currencyDBFormat->setText(q.value("locale_currformat").toString());
    _salesPriceDBFormat->setText(q.value("locale_salespriceformat").toString());
    _purchPriceDBFormat->setText(q.value("locale_purchpriceformat").toString());
    _extPriceDBFormat->setText(q.value("locale_extpriceformat").toString());
    _costDBFormat->setText(q.value("locale_costformat").toString());
    _qtyDBFormat->setText(q.value("locale_qtyformat").toString());
    _qtyPerDBFormat->setText(q.value("locale_qtyperformat").toString());
    _uomRatioDBFormat->setText(q.value("locale_uomratioformat").toString());
    _comments->setText(q.value("locale_comments").toString());

    sBuildDate();
    sBuildTime();
    sBuildTimestamp();
    sBuildInterval();
    sBuildCurrency();
    sBuildSalesPrice();
    sBuildPurchPrice();
    sBuildExtPrice();
    sBuildCost();
    sBuildQty();
    sBuildQtyPer();
    sBuildUOMRatio();
  }
}
