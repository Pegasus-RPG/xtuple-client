/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */
#ifndef __XDIALOG_H__
#define __XDIALOG_H__

#include <QDialog>
#include <QtScript>

#include <parameter.h>
#include <guiclient.h>

class XDialogPrivate;

class XDialog : public QDialog
{
  Q_OBJECT

  public:
    XDialog(QWidget * parent = 0, Qt::WindowFlags flags = 0);
    XDialog(QWidget * parent, const char * name, bool modal = false, Qt::WindowFlags flags = 0);
    virtual ~XDialog();

  public slots:
    virtual SetResponse set(const ParameterList &);
    virtual void done(int);
    void setRememberPos(bool);
    void setRememberSize(bool);

  protected:
    virtual void showEvent ( QShowEvent * event );

  private:
    friend class XDialogPrivate;
    XDialogPrivate *_private;
};

#endif // __XDIALOG_H__

