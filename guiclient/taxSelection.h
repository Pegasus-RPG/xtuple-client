/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TAXSELECTION_H
#define TAXSELECTION_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_taxSelection.h"

class taxSelection : public XDialog, public Ui::taxSelection
{
  Q_OBJECT

  public:
    taxSelection(QWidget* = 0, const char* = 0, bool = false, Qt::WFlags = 0);
    ~taxSelection();

  public slots:
    virtual SetResponse	set(const ParameterList &);
    virtual void	sPopulate();
    virtual void	sSave();

  protected slots:
    virtual void	languageChange();

  private:
    bool	_modal;
    int		_mode;
    int		_taxselId;
};

#endif // TAXSELECTION_H
