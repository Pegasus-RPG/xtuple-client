/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __MENUBUTTON_H__
#define __MENUBUTTON_H__

#include <QLabel>
#include <QPushButton>

#include "widgets.h"
#include "guiclientinterface.h"
#include "ui_menubutton.h"

class QScriptEngine;

void setupMenuButton(QScriptEngine *engine);

class XTUPLEWIDGETS_EXPORT MenuButton : public QWidget, public Ui::MenuButton
{
  Q_OBJECT
  Q_PROPERTY(QString image            READ image         WRITE setImage)
  Q_PROPERTY(QString label            READ label         WRITE setLabel)
  Q_PROPERTY(QString ui               READ ui            WRITE setUi)
  Q_PROPERTY(QString editPrivilege    READ editPriv      WRITE setEditPriv)
  Q_PROPERTY(QString viewPrivilege    READ viewPriv      WRITE setViewPriv)

  public:
    MenuButton(QWidget * = 0);
    ~MenuButton();

    static GuiClientInterface *_guiClientInterface;

    QString image()     const { return _image; }
    QString label();
    QString ui()        const { return _ui; }
    QString editPriv()  const { return _editPriv; }
    QString viewPriv()  const { return _viewPriv; }

  public slots:
    void setImage(QString image);
    void setLabel(QString text);
    void setUi(QString ui)         { _ui = ui; }
    void setEditPriv(QString priv);
    void setViewPriv(QString priv);
    void openWindow();
    
  private:
    QString _image;
    QString _ui;
    QString _editPriv;
    QString _viewPriv;
};

Q_DECLARE_METATYPE(MenuButton*)

#endif
