/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */
#ifndef __XWIDGET_H__
#define __XWIDGET_H__

#include <QWidget>
#include <QtScript>

#include <parameter.h>
#include <guiclient.h>

class XWidgetPrivate;

class XWidget : public QWidget
{
  Q_OBJECT

  public:
    XWidget(QWidget * parent = 0, Qt::WindowFlags flags = 0);
    XWidget(QWidget * parent, const char * name, Qt::WindowFlags flags = 0);
    ~XWidget();

  public slots:
    virtual SetResponse set( const ParameterList & pParams );

  protected:
    void closeEvent(QCloseEvent * event);
    void showEvent(QShowEvent * event);

  private:
    friend class XWidgetPrivate;
    XWidgetPrivate *_private;
};

#endif // __XWIDGET_H__

