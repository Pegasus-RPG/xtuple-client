/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QWidget>
#include <QDialog>
#include <QDebug>
#include "parameter.h"
#include "subscriber.h"

subscriber::subscriber(QWidget *parent,const char* name,bool modal,Qt::WindowFlags fl)
    : QDialog(parent,fl)
{
    _incdtid = -1;
    _whichRButton = false;
    if(name)
        setObjectName(name);
    setWindowTitle(tr("Subscriber"));
    if(modal)
        setWindowModality(Qt::WindowModal);
    Ui_subscriber::setupUi(this);
    _subsSelUserRButton->setChecked(true);
    _subsSelCntctRButton->setChecked(false);
    _subsSelUserCluster->setReadOnly(false);
    _subsSelCntctCluster->setReadOnly(true);
    connect(_cancel,SIGNAL(clicked()),this,SLOT(reject()));
    connect(_save,SIGNAL(clicked()),this,SLOT(addSelected()));
    connect(_subsSelUserRButton,SIGNAL(clicked()),this,SLOT(switchSelection()));
    connect(_subsSelCntctRButton,SIGNAL(clicked()),this,SLOT(switchSelection()));
}

void subscriber::switchSelection()
{
    switch(_whichRButton)
    {
        case false:
            //case for switching to contact selection
            _whichRButton = true;
            _subsSelCntctRButton->setChecked(true);
            _subsSelCntctCluster->setReadOnly(false);
            _subsSelUserRButton->setCheckable(true);
            _subsSelUserCluster->setReadOnly(true);
            break;
        case true:
            //case for switching back to user selection
            _whichRButton = false;
            _subsSelCntctRButton->setCheckable(true);
            _subsSelCntctCluster->setReadOnly(true);
            _subsSelUserRButton->setChecked(true);
            _subsSelUserCluster->setReadOnly(false);
            break;
    }
}

QString subscriber::whichSelection()
{
    switch(_whichRButton)
    {
        case false:
            return "user";
            break;
        case true:
            return "contact";
            break;
    }
}

void subscriber::set(ParameterList& params)
{
    bool valid;
    QVariant param;

    param = params.value("_incdtid",&valid);
    if(valid)
    {
        _incdtid = param.toInt();
    }
    else
        emit reject();
}

void subscriber::addSelected()
{
    QString type = whichSelection();
    QString usr;
    int id;
    if(type == "user")
    {
        id  = _subsSelUserCluster->id();
        qDebug() << "Selected a user returned id: " << id;
        usr = _subsSelUserCluster->username();
        if(usr == "" || usr == " ")
        {
            qDebug() << "No user selected, do nothing";
            return;
        }
        else
            qDebug() << "The targeted user is: " << usr;
    }
    else
    {
        id = _subsSelCntctCluster->id();
        qDebug() << "Selected a contact returned id: " << id;
        usr = _subsSelCntctCluster->first() + " " + _subsSelCntctCluster->last();
        if(usr == "" || usr == " ")
        {
            qDebug() << "No contact selected, do nothing";
            return;
        }
        else
            qDebug() << "The targeted contact is: " << usr;
    }
    XSqlQuery subs;
    subs.prepare(
        "INSERT "
        "INTO incdt_subs("
                "incdt_assoc_id,subscriber_id,subscriber_type"
        ") VALUES("
        ":incdtid,:subscriberid,:subscribertype"
        ");"
    );
    subs.bindValue(":incdtid",_incdtid);
    subs.bindValue(":subscriberid",id);
    subs.bindValue(":subscribertype",type);
    if(!subs.exec())
        emit reject();
    else
        emit done(1);
}
