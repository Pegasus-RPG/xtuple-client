/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "package.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#define DEBUG false

// TODO: XDialog should have a default implementation that returns FALSE
bool package::userHasPriv(const int pMode)
{
  bool retval = false;
  switch (pMode)
  {
    case cView:
      retval = _privileges->check("ViewPackages") ||
               _privileges->check("MaintainPackages");
      break;
    case cNew:
      retval = _privileges->check("MaintainPackages");
      break;
    case cEdit: // initial spec says that nobody has permission to edit packages
    default:
      retval = false;
      break;
  }
  return retval;
}

// TODO: this code really belongs in XDialog
void package::setVisible(bool visible)
{
  if (! visible)
    XDialog::setVisible(false);

  else if (! userHasPriv(_mode))
  {
    systemError(this,
		tr("You do not have sufficient privilege to view this window"),
		__FILE__, __LINE__);
    reject();
  }
  else
    XDialog::setVisible(true);
}

package::package(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _rec->addColumn(tr("Type"),        -1, Qt::AlignLeft, true, "type");
  _rec->addColumn(tr("Name"),        -1, Qt::AlignLeft, true, "pkgitem_name");
  _rec->addColumn(tr("Description"), -1, Qt::AlignLeft, true, "pkgitem_descrip");

  _req->addColumn(tr("Package"),     -1, Qt::AlignLeft,  true, "pkghead_name");
  _req->addColumn(tr("Description"), -1, Qt::AlignLeft,  true, "pkghead_descrip");
  _req->addColumn(tr("Version"),     -1, Qt::AlignRight, true, "pkghead_version");

  _dep->addColumn(tr("Package"),     -1, Qt::AlignLeft,  true, "pkghead_name");
  _dep->addColumn(tr("Description"), -1, Qt::AlignLeft,  true, "pkghead_descrip");
  _dep->addColumn(tr("Version"),     -1, Qt::AlignRight, true, "pkghead_version");
}

package::~package()
{
  // no need to delete child widgets, Qt does it all for us
}

void package::languageChange()
{
  retranslateUi(this);
}

enum SetResponse package::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("pkghead_id", &valid);
  if (valid)
  {
    _pkgheadid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _name->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _version->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(false);
      _version->setEnabled(false);
      _description->setEnabled(false);
      _developer->setEnabled(false);
      _notes->setEnabled(false);

      _save->hide();
      _close->setText(tr("&Close"));
      _close->setFocus();
    }
  }

  return NoError;
}

void package::sCheck()
{
  _name->setText(_name->text().stripWhiteSpace());
  if ( (_mode == cNew) && (_name->text().length()) )
  {
    q.prepare( "SELECT pkghead_id "
               "FROM pkghead "
               "WHERE (UPPER(pkghead_name)=UPPER(:pkghead_name));" );
    q.bindValue(":pkghead_code", _name->text());
    q.exec();
    if (q.first())
    {
      _pkgheadid = q.value("pkghead_id").toInt();
      // _mode = cEdit;    // initial spec says that nobody has permission to edit packages
      _mode = cView;
      populate();
    }
  }
}

void package::sSave()
{
  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('pkghead_pkghead_id_seq') AS _pkghead_id");
    if (q.first())
      _pkgheadid = q.value("_pkghead_id").toInt();
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO pkghead ("
               "  pkghead_id, pkghead_name, pkghead_descrip,"
               "  pkghead_version, pkghead_developer, pkghead_notes "
               ") VALUES ("
               "  :pkghead_id, :pkghead_name, :pkghead_descrip, "
               "  :pkghead_version, :pkghead_developer, :pkghead_notes );");
  }
  else if (_mode == cEdit)
  {
    q.prepare( "SELECT pkghead_id "
               "FROM pkghead "
               "WHERE ( (UPPER(pkghead_name)=UPPER(:pkghead_name))"
               " AND (pkghead_id<>:pkghead_id) );" );
    q.bindValue(":pkghead_id",   _pkgheadid);
    q.bindValue(":pkghead_name", _name->text());
    q.exec();
    if (q.first())
    {
      QMessageBox::warning( this, tr("Cannot Save Package"),
                            tr("<p>You may not rename this Package to %1 as "
                               "this value is used by a different Package.") );
      return;
    }

    q.prepare( "UPDATE pkghead "
               "SET pkghead_name=:pkghead_name, pkghead_descrip=:pkghead_descrip,"
               "    pkghead_version=:pkghead_version,"
               "    pkghead_developer=:pkghead_developer,"
               "    pkghead_notes=:pkghead_notes "
               "WHERE (pkghead_id=:pkghead_id);" );
  }

  q.bindValue(":pkghead_id",       _pkgheadid);
  q.bindValue(":pkghead_name",     _name->text());
  q.bindValue(":pkghead_descrip",  _description->text());
  q.bindValue(":pkghead_version",  _version->text());
  q.bindValue(":pkghead_developer",_developer->text());
  q.bindValue(":pkghead_notes",    _notes->text());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_pkgheadid);
}

void package::populate()
{
  if (DEBUG)    qDebug("package::populate() entered");

  q.prepare("SELECT * FROM pkghead WHERE (pkghead_id=:pkghead_id);");
  q.bindValue(":pkghead_id", _pkgheadid);
  q.exec();
  if (q.first())
  {
    if (DEBUG)    qDebug("package::populate() select pkghead succeeded");
    _name->setText(q.value("pkghead_name").toString());
    _description->setText(q.value("pkghead_descrip").toString());
    if (DEBUG)    qDebug("package::populate() select pkghead half done");
    _version->setText(q.value("pkghead_version").toString());
    _developer->setText(q.value("pkghead_developer").toString());
    _notes->setText(q.value("pkghead_notes").toString());
    if (DEBUG)    qDebug("package::populate() select pkghead complete");
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare("SELECT *,"
            "       CASE WHEN pkgitem_type='C' THEN :script"
            "            WHEN pkgitem_type='D' THEN :cmd"
            "            WHEN pkgitem_type='F' THEN :function"
            "            WHEN pkgitem_type='G' THEN :trigger"
            "            WHEN pkgitem_type='I' THEN :image"
            "            WHEN pkgitem_type='M' THEN :metasql"
            "            WHEN pkgitem_type='P' THEN :priv"
            "            WHEN pkgitem_type='R' THEN :report"
            "            WHEN pkgitem_type='S' THEN :schema"
            "            WHEN pkgitem_type='T' THEN :table"
            "            WHEN pkgitem_type='U' THEN :uiform"
            "            WHEN pkgitem_type='V' THEN :view"
            "       ELSE pkgitem_type END AS type "
            "FROM pkgitem "
            "WHERE (pkgitem_pkghead_id=:pkghead_id) "
            "ORDER BY type, pkgitem_name;");
  q.bindValue(":pkghead_id", _pkgheadid);
  q.bindValue(":script",     tr("Script"));
  q.bindValue(":cmd",        tr("Custom Command"));
  q.bindValue(":function",   tr("Stored Procedure"));
  q.bindValue(":trigger",    tr("Trigger"));
  q.bindValue(":image",      tr("Image"));
  q.bindValue(":metasql",    tr("MetaSQL"));
  q.bindValue(":priv",       tr("Privilege"));
  q.bindValue(":report",     tr("Report"));
  q.bindValue(":schema",     tr("Schema"));
  q.bindValue(":table",      tr("Table"));
  q.bindValue(":uiform",     tr("Screen"));
  q.bindValue(":view",       tr("View"));

  q.exec();
  if (DEBUG)    qDebug("package::populate() select pkgitem exec'ed");
  _rec->populate(q);
  if (DEBUG)    qDebug("package::populate() populate pkgitem done");
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  // TODO: make this recursive
  q.prepare("SELECT * "
            "FROM pkgdep, pkghead "
            "WHERE ((pkgdep_pkghead_id=pkghead_id)"
            "  AND  (pkgdep_parent_pkghead_id=:pkghead_id));");
  q.bindValue(":pkghead_id", _pkgheadid);
  q.exec();
  if (DEBUG)    qDebug("package::populate() select pkgdep exec'ed");
  _dep->populate(q);
  if (DEBUG)    qDebug("package::populate() populate pkgdep done");
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  // TODO: make this recursive
  q.prepare("SELECT * "
            "FROM pkgdep, pkghead "
            "WHERE ((pkgdep_parent_pkghead_id=pkghead_id)"
            "  AND  (pkgdep_pkghead_id=:pkghead_id));");
  q.bindValue(":pkghead_id", _pkgheadid);
  q.exec();
  if (DEBUG)    qDebug("package::populate() select pkgdep exec'ed");
  _req->populate(q);
  if (DEBUG)    qDebug("package::populate() populate pkgdep done");
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

}
