/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xcheckbox.h"

XCheckBox::XCheckBox(QWidget *pParent) :
  QCheckBox(pParent)
{
  constructor();
}

XCheckBox::XCheckBox(const QString &pText, QWidget *pParent) :
  QCheckBox(pText, pParent)
{
  constructor();
}

// can't make a static QPixmap 'cause Qt complains:
// Must construct a QApplication before a QPaintDevice
QPixmap *XCheckBox::_checkedIcon = 0;

void XCheckBox::constructor()
{
  _default=false;
  setForgetful(false);
    
  _mapper = new XDataWidgetMapper(this);
}

void XCheckBox::setData()
{
  if (_mapper->model() &&
    _mapper->model()->data(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this))).toBool() != isChecked())
  _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this)), isChecked());
}

void XCheckBox::setForgetful(bool p)
{
  if (_x_preferences && !p)
    _forgetful = _x_preferences->value("XCheckBox/forgetful").startsWith("t", Qt::CaseInsensitive);
  else
    _forgetful = p;
    
  if (! _forgetful)
  {
    Q_INIT_RESOURCE(widgets);
    if (! _checkedIcon)
      _checkedIcon = new QPixmap(":/widgets/images/xcheckbox.png");
      
    setIcon(*_checkedIcon);
    setIconSize(_checkedIcon->size());
  }
  else
    setIcon(QPixmap());    
}

void XCheckBox::setObjectName(const QString & pName)
{
  QCheckBox::setObjectName(pName);

  QString pname;
  if(window())
    pname = window()->objectName() + "/";
  _settingsName = pname + objectName();

  if(_x_preferences)
  {
    if (!_forgetful)
      setCheckState((Qt::CheckState)(_x_preferences->value(_settingsName + "/checked").toInt()));
  }
}

XCheckBox::~XCheckBox()
{
  if (!_settingsName.isEmpty() && _x_preferences)
  {
    if (_forgetful)
      _x_preferences->remove(_settingsName + "/checked");
    else
      _x_preferences->set(_settingsName + "/checked", (int)checkState());
  }
}

void XCheckBox::setDataWidgetMap(XDataWidgetMapper* m)
{
  m->addMapping(this, _fieldName, "checked", "defaultChecked");
  _mapper=m;
  connect(this, SIGNAL(stateChanged(int)), this, SLOT(setData())); 
}

