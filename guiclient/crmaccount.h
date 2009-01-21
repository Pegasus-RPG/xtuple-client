/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CRMACCOUNT_H
#define CRMACCOUNT_H

#include "guiclient.h"
#include "xwidget.h"
#include <QStandardItemModel>
#include "ui_crmaccount.h"

class crmaccount : public XWidget, public Ui::crmaccount
{
    Q_OBJECT

public:
    crmaccount(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~crmaccount();
    static void doDialog(QWidget *, const ParameterList &);
    int	getIncidentId();

public slots:
    virtual enum SetResponse set(const ParameterList&);
    virtual void sPopulate();
    virtual void sPopulateContacts();
    virtual void sPopulateTodo();
    virtual void sPopulateOplist();
    virtual void sPopulateRegistrations();

protected slots:
    virtual void languageChange();

    virtual void sAttach();
    virtual int  saveContact(ContactCluster*);
    virtual void sClose();
    virtual void sCompetitor();
    virtual void sCustomer();
    virtual void sDeleteCharacteristic();
    virtual void sDeleteIncdt();
    virtual void sDeleteReg();
    virtual void sDeleteTodo();
    virtual void sDeleteTodoIncdt();
    virtual void sDetach();
    virtual void sEdit();
    virtual void sEditCharacteristic();
    virtual void sEditIncdt();
    virtual void sEditReg();
    virtual void sEditTodo();
    virtual void sEditTodoIncdt();
    virtual void sGetCharacteristics();
    virtual void sHandleAutoUpdate();
    virtual void sHandleTodoPrivs();
    virtual void sNew();
    virtual void sNewCharacteristic();
    virtual void sNewIncdt();
    virtual void sNewReg();
    virtual void sNewTodo();
    virtual void sPartner();
    virtual void sPopulateMenu(QMenu *);
    virtual void sPopulateTodoMenu(QMenu *);
    virtual void sPopulateOplistMenu(QMenu *);
    virtual void sProspect();
    virtual void sSave();
    virtual void sTaxAuth();
    virtual void sUpdateRelationships();
    virtual void sVendor();
    virtual void sView();
    virtual void sViewIncdt();
    virtual void sViewTodo();
    virtual void sViewTodoIncdt();
    virtual void sCustomerInfo();
    virtual void sCustomerToggled();
    virtual void sProspectToggled();
    virtual void sOplistNew();
    virtual void sOplistDelete();
    virtual void sOplistView();
    virtual void sOplistEdit();
    virtual void sCheckNumber();
    virtual void sHandleButtons();
    virtual void sVendorInfo();

protected:
    virtual void closeEvent(QCloseEvent*);

private:
    bool	_modal;
    int		_mode;
    int		_crmacctId;
    int		_competitorId;
    int		_custId;
    int		_partnerId;
    int		_prospectId;
    int		_taxauthId;
    int		_vendId;
    int		_cntct1Id;
    int		_cntct2Id;
    int		_myUsrId;
    int         _NumberGen;

    int	saveNoErrorCheck();

};

#endif // CRMACCOUNT_H
