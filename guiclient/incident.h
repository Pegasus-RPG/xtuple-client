/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef INCIDENT_H
#define INCIDENT_H

#include "guiclient.h"
#include "xdialog.h"
#include <QStringList>
#include <parameter.h>

#include "ui_incident.h"

class incident : public XDialog, public Ui::incident
{
    Q_OBJECT

public:
    incident(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~incident();
    
    bool save(bool);
    int  saveContact(ContactCluster*);
    void populate();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sCancel();
    virtual void sCRMAcctChanged(const int);
    virtual void sDeleteTodoItem();
    virtual void sEditTodoItem();
    virtual void sFillHistoryList();
    virtual void sFillTodoList();
    virtual void sHandleTodoPrivs();
    virtual void sNewTodoItem();
    virtual void sPopulateTodoMenu(QMenu*);
    virtual void sSave();
    virtual void sViewTodoItem();
    virtual void sReturn();
    virtual void sViewAR();
    virtual void sContactChanged();
    virtual void sPrepareMail();
    virtual void sSendMail(ParameterList & params, ParameterList & rptParams);
    virtual void sChanged() {_updated=true;};
    virtual void sAssigned();
    virtual void sCommentAdded() {_commentAdded=true;};
    virtual void sUpdateEdiProfile();

signals:
    void prepareMail();

protected slots:
    virtual void languageChange();

private:
    int		_cntctid;
    int		_incdtid;
    int		_mode;
    int         _aropenid;
    int         _ediprofileid;
    bool	_saved;
    QString     _ardoctype;
    QStringList	_statusCodes;
    
    bool        _commentAdded;
    bool        _updated;
    int         _statusCache;
};

#endif // INCIDENT_H
