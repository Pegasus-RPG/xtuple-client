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
#include <xsqlquery.h>
#include <QDebug>

#include "parameter.h"
#include "subscribers.h"
#include "subscriber.h"

subscribers::subscribers(QWidget *parent, const char *name)
    : QWidget(parent,name)
{
    Ui_subscribers::setupUi(this);
    _subscriberTree->addColumn(tr("Type"),100,Qt::AlignLeft,true,"subscriber_type");
    _subscriberTree->addColumn(tr("Name"),100,Qt::AlignLeft,true,"subscriber_name");
    _subscriberTree->addColumn(tr("Email"),-1,Qt::AlignLeft,true,"subscriber_email");
    connect(_new,SIGNAL(clicked()),this,SLOT(add()));
    connect(_remove,SIGNAL(clicked()),this,SLOT(remove()));
}

void subscribers::add()
{
    ParameterList params;
    params.append("_incdtid",_incdtid);
    subscriber newdlg(this,"",false);
    newdlg.set(params);
    if(newdlg.exec() != QDialog::Rejected)
    {
        refresh();
    }
    else
        qDebug() << "Attempt to add subscriber to incident " << _incdtid << " failed or was cancelled";
}


void subscribers::remove()
{

    if(_subscriberTree->selectedItems().count() == 0)
    {
        qDebug() << "No items are selected or avaiable to remove, do nothing";
        return;
    }

    qDebug() << "The currently selected item returned id: " << _subscriberTree->currentItem()->id();
    QString name = _subscriberTree->currentItem()->text(1);
    QString type = _subscriberTree->currentItem()->text(0);
    XSqlQuery subs;
    subs.prepare(
        "DELETE "
                "FROM incdt_subs "
                "WHERE incdt_subs_idx=:idx;"
    );
    subs.bindValue(":idx",_subscriberTree->currentItem()->id());
    /*
    subs.prepare(
        "SELECT incdt_subs_remove_subscriber(:incdtid,:name,:type);"
    );
    subs.bindValue(":incdtid",_incdtid);
    subs.bindValue(":name",name);
    subs.bindValue(":type",type);
    */
    if(!subs.exec())
    {
        qDebug() << "Query to remove subscriber '" << name << "' of type '" << type << "' for incident '"
               << _incdtid << "' failed";
    }
    else
    {
        qDebug() << "Query to remove subscriber '" << name << "' of type '" << type << "' for incident '"
               << _incdtid << "' returned true!";
    }
    refresh();
}

void subscribers::refresh()
{
    if(_incdtid == -1)
    {
        _subscriberTree->clear();
        return;
    }

    XSqlQuery subs;
    subs.prepare(
        "SELECT "
                "incdt_subs_idx AS id, "
                "0 AS altId, "
                "subscriber_type, "
                "CASE "
                "WHEN subscriber_type='contact' THEN "
                "(SELECT cntct_first_name || ' ' || cntct_last_name "
                "FROM cntct WHERE cntct_id=subscriber_id) "
                "WHEN subscriber_type='user' THEN "
                "(SELECT usr_propername "
                "FROM usr WHERE usr_id=subscriber_id) "
                "END AS subscriber_name, "
                "subscriber_id AS subscriber_name_xtidrole, "
                "CASE "
                "WHEN subscriber_type='contact' THEN "
                "(SELECT cntct_email "
                "FROM cntct WHERE cntct_id=subscriber_id) "
                "WHEN subscriber_type='user' THEN "
                "(SELECT usr_email "
                "FROM usr WHERE usr_id=subscriber_id) "
                "END AS subscriber_email "
        "FROM incdt_subs "
        "WHERE incdt_assoc_id=:incdtid "
                 );
    subs.bindValue(":incdtid",_incdtid);
    subs.exec();
    _subscriberTree->populate(subs);
}

void subscribers::setId(int id)
{
    _incdtid = id;
    refresh();
}
