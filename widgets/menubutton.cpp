/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "menubutton.h"

#include <parameter.h>
#include <quuencode.h>
#include <xsqlquery.h>

#include <QImage>
#include <QMessageBox>
#include <QSqlError>
#include <QtScript>
#include <QVBoxLayout>

GuiClientInterface* MenuButton::_guiClientInterface = 0;

MenuButton::MenuButton(QWidget *pParent) :
  QWidget(pParent)
{
  setupUi(this);

  _button->setIconSize(QSize(64,64));
  _button->setIcon(QIcon(QPixmap(":/widgets/images/folder_zoom_64.png")));

  connect(_button, SIGNAL(clicked()), this, SLOT(openWindow()));
}

MenuButton::~MenuButton()
{
}

QString MenuButton::label()
{
  return _label->text();
}

void MenuButton::setLabel(QString text)
{
  _label->setText(text);
}

void MenuButton::setEditPriv(QString priv)
{
  if (_editPriv == priv)
    return;

  _editPriv = priv;
  if (_x_privileges)
  {
     setEnabled((_x_privileges->check(_editPriv) ||
                _x_privileges->check(_viewPriv)) ||
                (_editPriv.isEmpty() &&
                _viewPriv.isEmpty()));
  }
}

void MenuButton::setViewPriv(QString priv)
{
  if (_viewPriv == priv)
    return;

  _viewPriv = priv;
  if (_x_privileges)
  {
     setEnabled((_x_privileges->check(_editPriv) ||
                _x_privileges->check(_viewPriv)) ||
                (_editPriv.isEmpty() &&
                _viewPriv.isEmpty()));
  }
}

void MenuButton::setImage(QString image)
{
  if (_image == image)
    return;

  _image = image;
  XSqlQuery qry;
  qry.prepare("SELECT image_data "
              "FROM image "
              "WHERE (image_name=:image);");
  qry.bindValue(":image", _image);
  qry.exec();
  if (qry.first())
  {
    QImage img;
    img.loadFromData(QUUDecode(qry.value("image_data").toString()));
    _button->setIcon(QIcon(QPixmap::fromImage(img)));
    return;
  }
  else if (qry.lastError().type() != QSqlError::NoError)
    QMessageBox::critical(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__),
                          qry.lastError().databaseText());
  _button->setIcon(QIcon(QPixmap(":/widgets/images/folder_zoom_64.png")));
}

void MenuButton::openWindow()
{
  if (_ui.isEmpty() ||
      !_x_privileges)
    return;

  ParameterList params;
  if (!_editPriv.isEmpty() ||
      !_viewPriv.isEmpty())
  {
    if (_x_privileges->check(_editPriv))
      params.append("mode", "edit");
    else if (_x_privileges->check(_viewPriv))
      params.append("mode", "view");
    else
      return;
  }

  QWidget* w = 0;
  if (parentWidget()->window())
  {
    if (parentWidget()->window()->isModal())
      w = _guiClientInterface->openWindow(_ui, params, parentWidget()->window() , Qt::WindowModal, Qt::Dialog);
    else
      w = _guiClientInterface->openWindow(_ui, params, parentWidget()->window() , Qt::NonModal, Qt::Window);

    if (w)
    {
      if (w->inherits("QDialog"))
      {
        QDialog* newdlg = qobject_cast<QDialog*>(w);
        newdlg->exec();
      }
    }
  }
}

// scripting exposure /////////////////////////////////////////////////////////

QScriptValue MenuButtontoScriptValue(QScriptEngine *engine, MenuButton* const &item)
{
  return engine->newQObject(item);
}

void MenuButtonfromScriptValue(const QScriptValue &obj, MenuButton* &item)
{
  item = qobject_cast<MenuButton*>(obj.toQObject());
}

QScriptValue constructMenuButton(QScriptContext *context,
                                QScriptEngine  *engine)
{
  MenuButton *button = 0;

  if (context->argumentCount() == 0)
    button = new MenuButton();

  else if (context->argumentCount() == 1 &&
           qscriptvalue_cast<QWidget*>(context->argument(0)))
    button = new MenuButton(qscriptvalue_cast<QWidget*>(context->argument(0)));

  else
    context->throwError(QScriptContext::UnknownError,
                        QString("Could not find an appropriate MenuButton constructor"));

  return engine->toScriptValue(button);
}

void setupMenuButton(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, MenuButtontoScriptValue, MenuButtonfromScriptValue);
  QScriptValue widget = engine->newFunction(constructMenuButton);
  engine->globalObject().setProperty("MenuButton", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
