/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xlineedit.h"

#include <QFocusEvent>
#include <QKeyEvent>
#include <QLocale>
#include <QMouseEvent>
#include <QValidator>

#include "format.h"

XLineEdit::XLineEdit(QWidget *parent, const char *name) :
  QLineEdit(parent, name)
{
  setAcceptDrops(FALSE);

#ifdef Q_WS_MAC
  QFont f = font();
  f.setPointSize(f.pointSize() - 2);
  setFont(f);
#endif

  _parsed = TRUE;
  _valid = FALSE;

  _id = -1;
  connect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));
  
  _mapper = new XDataWidgetMapper(this);
}

bool XLineEdit::isValid()
{
  sParse();
  return _valid;
}

int XLineEdit::id()
{
  sParse();
  return _id;
}

void XLineEdit::sParse()
{
  if (validator() && validator()->inherits("QDoubleValidator"))
  {
    QRegExp zeroRegex(QString("^[0") + QLocale().groupSeparator() + "]*" +
                      QLocale().decimalPoint() + "0*$");
    if (! text().isEmpty() && toDouble() == 0 && ! text().contains(zeroRegex))
      setText("");
  }
}

double XLineEdit::toDouble(bool *pIsValid)
{
  QString strippedText = text().remove(QLocale().groupSeparator());
  return QLocale().toDouble(strippedText, pIsValid);
}

void XLineEdit::setDataWidgetMap(XDataWidgetMapper* m)
{
  _mapper=m;
  m->addMapping(this, _fieldName, QByteArray("text"), QByteArray("defaultText"));
  connect(this, SIGNAL(textChanged(QString)), this, SLOT(setData(QString)));
}

void XLineEdit::setText(const QVariant &pVariant)
{
  if (pVariant.type() == QVariant::Double)
  {
    const QValidator *v = validator();
    int prec = 0;

    if (v && v->inherits("QDoubleValidator"))
      prec = ((QDoubleValidator*)v)->decimals();
    else if (v && v->inherits("QIntValidator"))
      prec = 0;

    QLineEdit::setText(formatNumber(pVariant.toDouble(), prec));
  }
  else
    QLineEdit::setText(pVariant.toString());
}


void XLineEdit::setDouble(const double pDouble, const int pPrec)
{
  const QValidator *v = validator();
  int prec = pPrec;

  if (pPrec < 0 && v && v->inherits("QDoubleValidator"))
    prec = ((QDoubleValidator*)v)->decimals();

  QLineEdit::setText(formatNumber(pDouble, prec));
}

void XLineEdit::mouseDoubleClickEvent(QMouseEvent *event)
{
  emit doubleClicked();

  QLineEdit::mouseDoubleClickEvent(event);
}

void XLineEdit::mousePressEvent(QMouseEvent *event)
{
  emit clicked();

  QLineEdit::mousePressEvent(event);
}

void XLineEdit::keyPressEvent(QKeyEvent *event)
{
  if (event->state() == Qt::ControlModifier)
  {
    if (event->key() == Qt::Key_L)
    {
      _parsed = TRUE;
      emit (requestList());
    }

    else if (event->key() == Qt::Key_S)
    {
      _parsed = TRUE;
      emit (requestSearch());
    }

    else if (event->key() == Qt::Key_A)
    {
      _parsed = TRUE;
      emit (requestAlias());
    }
  }
  else
    _parsed = FALSE;

  if (_parsed)
    event->accept();
  else
    QLineEdit::keyPressEvent(event);
}

void XLineEdit::focusInEvent(QFocusEvent *pEvent)
{
  if(!text().isEmpty())
    setSelection(0, text().length());

  QLineEdit::focusInEvent(pEvent);
}

void XLineEdit::setData(const QString &text)
{
  if (_mapper->model())
    _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this)), text);
}

