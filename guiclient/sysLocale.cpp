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

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

sysLocale::sysLocale(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_country,     SIGNAL(newID(int)), this, SLOT(sUpdateSamples()));
  connect(_language,    SIGNAL(newID(int)), this, SLOT(sUpdateSamples()));
  connect(_error,       SIGNAL(editingFinished()), this, SLOT(sUpdateColors()));
  connect(_warning,     SIGNAL(editingFinished()), this, SLOT(sUpdateColors()));
  connect(_alternate,   SIGNAL(editingFinished()), this, SLOT(sUpdateColors()));
  connect(_emphasis,    SIGNAL(editingFinished()), this, SLOT(sUpdateColors()));
  connect(_future,      SIGNAL(editingFinished()), this, SLOT(sUpdateColors()));
  connect(_expired,     SIGNAL(editingFinished()), this, SLOT(sUpdateColors()));
  connect(_save,        SIGNAL(clicked()),  this, SLOT(sSave()));

  connect(_currencyDBFormat, SIGNAL(lostFocus()), this, SLOT(sBuildCurrency()));
  connect(_salesPriceDBFormat, SIGNAL(lostFocus()), this, SLOT(sBuildSalesPrice()));
  connect(_extPriceDBFormat, SIGNAL(lostFocus()), this, SLOT(sBuildExtPrice()));
  connect(_qtyDBFormat, SIGNAL(lostFocus()), this, SLOT(sBuildQty()));
  connect(_qtyPerDBFormat, SIGNAL(lostFocus()), this, SLOT(sBuildQtyPer()));
  connect(_costDBFormat, SIGNAL(lostFocus()), this, SLOT(sBuildCost()));
  connect(_purchPriceDBFormat, SIGNAL(lostFocus()), this, SLOT(sBuildPurchPrice()));
  connect(_uomRatioDBFormat, SIGNAL(lostFocus()), this, SLOT(sBuildUOMRatio()));

  _localeid = -1;
}

sysLocale::~sysLocale()
{
  // no need to delete child widgets, Qt does it all for us
}

void sysLocale::languageChange()
{
  retranslateUi(this);
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
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
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
      _country->setEnabled(FALSE);
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
                           tr("<p>You must enter a Code for this Locale before "
                              "you may save it.") );
    _code->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('locale_locale_id_seq') AS _locale_id");
    if (q.first())
      _localeid = q.value("_locale_id").toInt();

    q.prepare( "INSERT INTO locale "
               "( locale_id, locale_code, locale_descrip,"
               "  locale_lang_id, locale_country_id, "
	       "  locale_currformat,"
               "  locale_salespriceformat, locale_purchpriceformat,"
               "  locale_extpriceformat, locale_costformat,"
               "  locale_qtyformat, locale_qtyperformat, locale_uomratioformat,"
               "  locale_comments, "
               "  locale_error_color, locale_warning_color,"
               "  locale_emphasis_color, locale_altemphasis_color,"
               "  locale_expired_color, locale_future_color) "
               "VALUES "
               "( :locale_id, :locale_code, :locale_descrip,"
               "  :locale_lang_id, :locale_country_id,"
	       "  :locale_currformat,"
               "  :locale_salespriceformat, :locale_purchpriceformat,"
               "  :locale_extpriceformat, :locale_costformat,"
               "  :locale_qtyformat, :locale_qtyperformat, :locale_uomratioformat,"
               "  :locale_comments,"
               "  :locale_error_color, :locale_warning_color,"
               "  :locale_emphasis_color, :locale_altemphasis_color,"
               "  :locale_expired_color, :locale_future_color);" );
  }
  else if ( (_mode == cEdit) || (_mode == cCopy) )
    q.prepare( "UPDATE locale "
                "SET locale_code=:locale_code, locale_descrip=:locale_descrip,"
                "    locale_lang_id=:locale_lang_id,"
                "    locale_country_id=:locale_country_id,"
		"    locale_currformat=:locale_currformat,"
                "    locale_salespriceformat=:locale_salespriceformat, locale_purchpriceformat=:locale_purchpriceformat,"
                "    locale_extpriceformat=:locale_extpriceformat, locale_costformat=:locale_costformat,"
                "    locale_qtyformat=:locale_qtyformat, locale_qtyperformat=:locale_qtyperformat,"
                "    locale_uomratioformat=:locale_uomratioformat,"
                "    locale_comments=:locale_comments,"
                "    locale_error_color=:locale_error_color,"
                "    locale_warning_color=:locale_warning_color,"
                "    locale_emphasis_color=:locale_emphasis_color,"
                "    locale_altemphasis_color=:locale_altemphasis_color,"
                "    locale_expired_color=:locale_expired_color,"
                "    locale_future_color=:locale_future_color "
                "WHERE (locale_id=:locale_id);" );

  q.bindValue(":locale_id", _localeid);
  q.bindValue(":locale_code", _code->text());
  q.bindValue(":locale_descrip", _description->text());
  q.bindValue(":locale_lang_id",        _language->id());
  q.bindValue(":locale_country_id",     _country->id());
  q.bindValue(":locale_currformat", _currencyDBFormat->text());
  q.bindValue(":locale_salespriceformat", _salesPriceDBFormat->text());
  q.bindValue(":locale_purchpriceformat", _purchPriceDBFormat->text());
  q.bindValue(":locale_extpriceformat", _extPriceDBFormat->text());
  q.bindValue(":locale_costformat", _costDBFormat->text());
  q.bindValue(":locale_qtyformat", _qtyDBFormat->text());
  q.bindValue(":locale_qtyperformat", _qtyPerDBFormat->text());
  q.bindValue(":locale_uomratioformat", _uomRatioDBFormat->text());
  q.bindValue(":locale_comments", _comments->text());
  q.bindValue(":locale_error_color",       _error->text());
  q.bindValue(":locale_warning_color",     _warning->text());
  q.bindValue(":locale_emphasis_color",    _emphasis->text());
  q.bindValue(":locale_altemphasis_color", _alternate->text());
  q.bindValue(":locale_expired_color",     _expired->text());
  q.bindValue(":locale_future_color",      _future->text());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_localeid);
}

void sysLocale::close()
{
  qDebug("sysLocale::close()");
  if (_mode == cCopy && _localeid > 0)
  {
    q.prepare( "DELETE FROM locale "
               "WHERE (locale_id=:locale_id);" );
    q.bindValue(":locale_id", _localeid);
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  reject();
}
 

void sysLocale::sUpdateSamples()
{
  QLocale::Language localeLang = QLocale::C;
  QLocale::Country  localeCountry = QLocale::AnyCountry;

  if (_language->id() > 0)
  {
    q.prepare("SELECT lang_qt_number FROM lang WHERE (lang_id=:langid);");
    q.bindValue(":langid", _language->id());
    q.exec();
    if (q.first())
    {
      localeLang = QLocale::Language(q.value("lang_qt_number").toInt());
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  if (_country->id() > 0)
  {
    q.prepare("SELECT country_qt_number "
              "FROM country "
              "WHERE (country_id=:countryid);");
    q.bindValue(":countryid", _country->id());
    q.exec();
    if (q.first())
    {
      localeCountry = QLocale::Country(q.value("country_qt_number").toInt());
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  q.prepare("SELECT CURRENT_DATE AS dateSample, CURRENT_TIME AS timeSample,"
            "       CURRENT_TIMESTAMP AS timestampSample,"
            "       CURRENT_TIMESTAMP - CURRENT_DATE AS intervalSample;");
  q.exec();
  if (q.first())
  {
    QLocale sampleLocale = QLocale(localeLang, localeCountry);
    _dateSample->setText(sampleLocale.toString(q.value("dateSample").toDate()));
    _timeSample->setText(sampleLocale.toString(q.value("timeSample").toTime()));
    _timestampSample->setText(sampleLocale.toString(q.value("timestampSample").toDate()) +
                              " " +
                              sampleLocale.toString(q.value("timestampSample").toTime()));
    _intervalSample->setText(sampleLocale.toString(q.value("intervalSample").toTime()));
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void sysLocale::sUpdateColors()
{
  QList<QLineEdit*> colorwidgets;
  colorwidgets << _error        << _warning << _emphasis
               << _alternate    << _expired << _future;
  for (int i = 0; i < colorwidgets.size(); i++)
  {
    QLineEdit *widget = colorwidgets.at(i);
    QPalette colors = widget->palette();
    colors.setBrush(QPalette::Text, QBrush(QColor(widget->text())));
    widget->setPalette(colors);
  }
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
  XSqlQuery popq;
  popq.prepare( "SELECT * "
             "FROM locale "
             "WHERE (locale_id=:locale_id);" );
  popq.bindValue(":locale_id", _localeid);
  popq.exec();
  if (popq.first())
  {
    _code->setText(popq.value("locale_code").toString());
    _description->setText(popq.value("locale_descrip").toString());
    _language->setId(popq.value("locale_lang_id").toInt());
    _country->setId(popq.value("locale_country_id").toInt());
    _currencyDBFormat->setText(popq.value("locale_currformat").toString());
    _salesPriceDBFormat->setText(popq.value("locale_salespriceformat").toString());
    _purchPriceDBFormat->setText(popq.value("locale_purchpriceformat").toString());
    _extPriceDBFormat->setText(popq.value("locale_extpriceformat").toString());
    _costDBFormat->setText(popq.value("locale_costformat").toString());
    _qtyDBFormat->setText(popq.value("locale_qtyformat").toString());
    _qtyPerDBFormat->setText(popq.value("locale_qtyperformat").toString());
    _uomRatioDBFormat->setText(popq.value("locale_uomratioformat").toString());
    _comments->setText(popq.value("locale_comments").toString());
    _error->setText(popq.value("locale_error_color").toString());
    _warning->setText(popq.value("locale_warning_color").toString());
    _emphasis->setText(popq.value("locale_emphasis_color").toString());
    _alternate->setText(popq.value("locale_altemphasis_color").toString());
    _expired->setText(popq.value("locale_expired_color").toString());
    _future->setText(popq.value("locale_future_color").toString());

    sUpdateSamples();
    sUpdateColors();

    sBuildCurrency();
    sBuildSalesPrice();
    sBuildPurchPrice();
    sBuildExtPrice();
    sBuildCost();
    sBuildQty();
    sBuildQtyPer();
    sBuildUOMRatio();
  }
  if (popq.lastError().type() != QSqlError::None)
  {
    systemError(this, popq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
