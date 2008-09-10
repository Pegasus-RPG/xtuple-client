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

//  xlabel.cpp
//  Created 08/05/2008 GJM from xlineedit.cpp
//  Copyright (c) 2003-2008, OpenMFG, LLC

#include "xlabel.h"

#include <QLocale>
#include <QValidator>

#include "format.h"

#define DEBUG false

XLabel::XLabel(QWidget *parent, const char *name) :
  QLabel(parent, name)
{
  _fieldName = "";
  _precision = 0;
}

void XLabel::setDataWidgetMap(XDataWidgetMapper* m)
{
  m->addMapping(this, _fieldName, QByteArray("text"), QByteArray("defaultText"));
}

void XLabel::setPrecision(const int pPrec)
{
  _precision = pPrec;
}

void XLabel::setPrecision(const QDoubleValidator *pVal)
{
  _precision = pVal->decimals();
}

void XLabel::setPrecision(const QIntValidator * /*pVal*/)
{
  _precision = 0;
}

void XLabel::setDouble(const double pDouble, const int pPrec)
{
  QLabel::setText(formatNumber(pDouble, (pPrec < 0) ? _precision : pPrec));
}

void XLabel::setText(const QVariant &pVariant)
{
  if (DEBUG)
    qDebug("XLabel::setText(const QVariant & = %s)",
           qPrintable(pVariant.toString()));
  if (pVariant.type() == QVariant::Double ||
      pVariant.type() == QVariant::Int)
    QLabel::setText(formatNumber(pVariant.toDouble(), _precision));
  else
    QLabel::setText(pVariant.toString());
}

void XLabel::setText(const char *pText)
{
  if (DEBUG) qDebug("XLabel::setText(const char * = %s)", pText);
  setText(QString(pText));
}

void XLabel::setText(const QString &pText)
{
  if (DEBUG)
    qDebug("XLabel::setText(const QString & = %s)", qPrintable(pText));
  if (_precision == 0)
    QLabel::setText(pText);
  else
  {
    bool ok;
    double val = QLocale().toDouble(pText, &ok);
    if (DEBUG) qDebug("XLabel::setText() %f %d", val, ok);
    if (ok)
      setDouble(val);
    else
      QLabel::setText(pText);
  }
}

double XLabel::toDouble(bool *pIsValid)
{
  return QLocale().toDouble(text(), pIsValid);
}
