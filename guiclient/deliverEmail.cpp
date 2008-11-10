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

#include "deliverEmail.h"

#include <qvariant.h>
#include <qmessagebox.h>

#include <openreports.h>
#include <parameter.h>

#include <QUrl>
#include <QDesktopServices>

/*
 *  Constructs a deliverEmail as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
deliverEmail::deliverEmail(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    _captive = FALSE;

    // signals and slots connections
    connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    
    

}

/*
 *  Destroys the object and frees any allocated resources
 */
deliverEmail::~deliverEmail()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void deliverEmail::languageChange()
{
    retranslateUi(this);
}

enum SetResponse deliverEmail::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;
  
  _captive = true;

  param = pParams.value("from", &valid);
  if (valid)
    _from->setText(param.toString());

  param = pParams.value("to", &valid);
  if (valid)
  {
    _to->setText(param.toString());
    if (!_to->text().isEmpty())
      _submit->setEnabled(TRUE);
  }
  
  param = pParams.value("cc", &valid);
  if (valid)
    _cc->setText(param.toString());
  
  param = pParams.value("subject", &valid);
  if (valid)
    _subject->setText(param.toString());
    
  param = pParams.value("body", &valid);
  if (valid)
    _body->setText(param.toString());

  return NoError;
}

void deliverEmail::sSubmit()
{
  if (_to->text().isEmpty())
  {
    QMessageBox::critical( this, tr("Cannot Email for Delivery"),
                           tr("You must enter a email address to which this message is to be delivered.") );
    _to->setFocus();
    return;
  }
  
  submitEmail(this,
         _from->text(),
         _to->text(),
         _cc->text(),
         _subject->text(),
         _body->toPlainText(),
         _emailHTML->isChecked());
  
  if (_captive)
    accept();
  else
  {
    _submit->setEnabled(FALSE);
    _close->setText(tr("&Close"));
    _to->clear();
    _cc->clear();
    _body->clear();
    _subject->clear();
  }
}


bool deliverEmail::profileEmail(QWidget *parent, int profileid, ParameterList &pParams)
{
  if (!profileid)
    return false;

  QVariant param;
  bool     valid;
  
  bool    preview = false;
  
  //Token variables
  int     docid = 0;
  QString comments;
  QString descrip;
  QString docnumber;
  QString doctype;
  QString docbody;
  QString email1;
  QString email2; 
  QString email3; 
  
  //Email variables
  QString from;
  QString to;
  QString cc;
  QString subject;
  QString body;
    
  //Process parameters
  param = pParams.value("preview", &valid);
  if (valid)
    preview = param.toBool(); 
    
  param = pParams.value("description", &valid);
  if (valid)
    descrip = param.toString();
    
  param = pParams.value("docid", &valid);
  if (valid)
    docid = param.toInt();

  param = pParams.value("docbody", &valid);
  if (valid)
    docbody = param.toString();
    
  param = pParams.value("docnumber", &valid);
  if (valid)
    docnumber = param.toString();

  param = pParams.value("doctype", &valid);
  if (valid)
    doctype = param.toString();
    
  param = pParams.value("email1", &valid);
  if (valid)
    email1 = param.toString();
    
  param = pParams.value("email2", &valid);
  if (valid)
    email2 = param.toString();

  param = pParams.value("email3", &valid);
  if (valid)
    email3 = param.toString();

  //Get user email
  q.exec( "SELECT usr_email "
          "FROM usr "
          "WHERE (usr_username=CURRENT_USER);" );
  if (q.first())
    from=q.value("usr_email").toString();
    
  //Process profile
  q.prepare( "SELECT ediprofile_option1 AS emailto, "
             "  ediprofile_option4 AS emailcc, "
             "  ediprofile_option2 AS subject, "
             "  ediprofile_option3 AS body, "
             "  ediprofile_emailhtml "
             "FROM ediprofile "
             "WHERE ( (ediprofile_id=:ediprofile_id) "
             " AND (ediprofile_type='email') );" );
  q.bindValue(":ediprofile_id", profileid);
  q.exec();
  if (q.first())
  {
    to=q.value("emailto").toString();
    cc=q.value("emailcc").toString();
    body=QString(q.value("body").toString()
      .replace("</description>" , descrip)
      .replace("</docnumber>"   , docnumber)
      .replace("</doctype>"     , doctype)
      .replace("</docbody>"     , docbody));
    subject=QString(q.value("subject").toString()
      .replace("</description>" , descrip)
      .replace("</docnumber>"   , docnumber)
      .replace("</doctype>"     , doctype)
      .replace("</docbody>"     , docbody));
            
    //Handle E-mail
    //Don't send messages to myself
    if (email1 == to)
      email1 = QString::null;
    if (email2 == from)
      email2 = QString::null;
    if (email3 == from)
      email3 = QString::null;
      
    //Add token addresses to "To" list if applicable
    if ((email1.isEmpty()) || (to.count(email1,Qt::CaseInsensitive)))
      to=to.remove("</email1>");
    else
    {
      if (to.stripWhiteSpace()
            .remove("</email1>")
            .remove("</email2>")
            .remove("</email3>")
            .length())
        to=to.replace("</email1>", ", " + email1);
      else
        to=to.replace("</email1>", email1);
    }
  
    if ((email2.isEmpty()) || (to.count(email2,Qt::CaseInsensitive)))
      to=to.remove("</email2>");
    else
    {
      if (to.stripWhiteSpace()
            .remove("</email1>")
            .remove("</email2>")
            .remove("</email3>")
            .length())
        to=to.replace("</email2>", ", " + email2);
      else
        to=to.replace("</email2>", email2);
    }
        
    if ((email3.isEmpty()) || (to.count(email3,Qt::CaseInsensitive)))
      to=to.remove("</email3>");
    else
    {
      if (to.stripWhiteSpace()
            .remove("</email1>")
            .remove("</email2>")
            .remove("</email3>")
            .length())
        to=to.replace("</email3>", ", " + email3);
      else
        to=to.replace("</email3>", email3);
    }
    
    //Add token addresses to "CC" list if applicable
    if ((email1.isEmpty()) || (cc.count(email1,Qt::CaseInsensitive)))
      cc=cc.remove("</email1>");
    else
    {
      if (cc.stripWhiteSpace()
            .remove("</email1>")
            .remove("</email2>")
                        .remove("</email3>")
                        .length())
        cc=cc.replace("</email1>", ", " + email1);
      else
        cc=cc.replace("</email1>", email1);
    }
  
    if ((email2.isEmpty()) || (cc.count(email2,Qt::CaseInsensitive)))
      cc=cc.remove("</email2>");
    else
    {
      if (cc.stripWhiteSpace()
            .remove("</email1>")
            .remove("</email2>")
            .remove("</email3>")
            .length())
        cc=cc.replace("</email2>", ", " + email2);
      else
        cc=cc.replace("</email2>", email2);
    }
        
    if ((email3.isEmpty()) || (cc.count(email3,Qt::CaseInsensitive)))
      cc=cc.remove("</email3>");
    else
    {
      if (cc.stripWhiteSpace()
            .remove("</email1>")
            .remove("</email2>")
            .remove("</email3>")
            .length())
        cc=cc.replace("</email3>", ", " + email3);
      else
        cc=cc.replace("</email3>", email3);
    }
    
    //Build comment detail if applicable
    if (body.count("</comments>") &&
         !doctype.isEmpty() && docid)
    {                         
      q.prepare("SELECT comment_user, comment_date, comment_text "
                "FROM comment "
                "WHERE ( (comment_source=:doctype) "
                " AND (comment_source_id=:docid) ) "
                "ORDER BY comment_date;");
      q.bindValue(":doctype", doctype);
      q.bindValue(":docid", docid);
      q.exec();
      while (q.next())
      {
        comments += "-----------------------------------------------------\n";
        comments += q.value("comment_user").toString();
        comments += " - ";
        comments += q.value("comment_date").toString();
        comments += "\n-----------------------------------------------------\n";
        comments += q.value("comment_text").toString();
        comments += "\n\n";
      }
      
      body=body.replace("</comments>", comments);
    }
    
    if (preview)
    {
      ParameterList params;
      params.append("from", from);
      params.append("to", to);
      params.append("cc", cc);
      params.append("subject", subject);
      params.append("body", body);
      params.append("emailhtml", q.value("ediprofile_emailhtml").toBool());
       
      deliverEmail newdlg(parent, "", TRUE);
      newdlg.set(params);
      if (newdlg.exec() == XDialog::Rejected)
        return false;
      else
        return true;
    }
    else
      submitEmail(parent,from,to,cc,subject,body,q.value("ediprofile_emailhtml").toBool());
  }
  else
    return false;

  return true;
}

bool deliverEmail::submitEmail(QWidget* parent, const QString to, const QString cc, const QString subject, const QString body)
{
  QString from;
  
  //Get user email
  q.exec( "SELECT usr_email "
          "FROM usr "
          "WHERE (usr_username=CURRENT_USER);" );
  if (q.first())
    from=q.value("usr_email").toString();
  else
    return false;
    
  return submitEmail(parent,from,to,cc,subject,body);
}

bool deliverEmail::submitEmail(QWidget* parent, const QString from, const QString to, const QString cc, const QString subject, const QString body)
{
  return submitEmail(parent,from,to,cc,subject,body,false);
}

bool deliverEmail::submitEmail(QWidget* parent, const QString from, const QString to, const QString cc, const QString subject, const QString body, const bool emailHTML)
{
  if (to.isEmpty())
    return false;

  q.prepare( "SELECT submitEmailToBatch( :fromEmail, :emailAddress, :ccAddress, :subject,"
             "                            :emailBody, :fileName, CURRENT_TIMESTAMP, :emailHTML) AS batch_id;" );
  q.bindValue(":fromEmail", from);
  q.bindValue(":emailAddress", to);
  q.bindValue(":ccAddress", cc);
  q.bindValue(":subject", subject);
  q.bindValue(":emailBody", body);
  q.bindValue(":emailHTML", emailHTML);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(parent, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }
  
  return true;
}

