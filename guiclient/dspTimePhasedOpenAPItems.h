/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPTIMEPHASEDOPENAPITEMS_H
#define DSPTIMEPHASEDOPENAPITEMS_H

#include "xwidget.h"
#include <QList>

#include "ui_dspTimePhasedOpenAPItems.h"

class dspTimePhasedOpenAPItems : public XWidget, public Ui::dspTimePhasedOpenAPItems
{
    Q_OBJECT

public:
    dspTimePhasedOpenAPItems(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspTimePhasedOpenAPItems();

public slots:
    virtual void sPrint();
    virtual void sSubmit();
    virtual void sViewOpenItems();
    virtual void sPopulateMenu( QMenu * menuThis, QTreeWidgetItem *, int pColumn );
    virtual void sFillList();
    virtual void sFillStd();
    virtual void sFillCustom();
    virtual void sToggleCustom();

protected slots:
    virtual void languageChange();
    virtual bool setParams(ParameterList &);

private:
    int _column;
    QList<DatePair> _columnDates;

};

#endif // DSPTIMEPHASEDOPENAPITEMS_H
