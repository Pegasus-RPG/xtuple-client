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
    _docid = 0;

    q.exec( "SELECT usr_email "
          "FROM usr "
          "WHERE (usr_username=CURRENT_USER);" );
    if (q.first())
      _fromEmail->setText(q.value("usr_email"));

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


enum SetResponse deliverEmail::set(ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;
    
  param = pParams.value("description", &valid);
  if (valid)
    _descrip = param.toString();
    
  param = pParams.value("docid", &valid);
  if (valid)
    _docid = param.toInt();

  param = pParams.value("docbody", &valid);
  if (valid)
    _docbody = param.toString();
    
  param = pParams.value("docnumber", &valid);
  if (valid)
    _docnumber = param.toString();

  param = pParams.value("doctype", &valid);
  if (valid)
    _doctype = param.toString();
    
  param = pParams.value("email1", &valid);
  if (valid)
    _email1 = param.toString();
    
  param = pParams.value("email2", &valid);
  if (valid)
    _email2 = param.toString();

  param = pParams.value("email3", &valid);
  if (valid)
    _email3 = param.toString();
    
  param = pParams.value("profileid", &valid);
  if (valid)
    setProfileId(param.toInt());

  return NoError;
}

void deliverEmail::sSubmit()
{
  if (_email->text().isEmpty())
  {
    QMessageBox::critical( this, tr("Cannot Email for Delivery"),
                           tr("You must enter a email address to which this message is to be delivered.") );
    _email->setFocus();
    return;
  }

  q.prepare( "SELECT submitEmailToBatch( :fromEmail, :emailAddress, :ccAddress, :subject,"
             "                            :emailBody, :fileName, CURRENT_TIMESTAMP, :emailHTML) AS batch_id;" );
  q.bindValue(":fromEmail", _fromEmail->text());
  q.bindValue(":emailAddress", _email->text());
  q.bindValue(":ccAddress", _cc->text());
  q.bindValue(":subject", _subject->text());
  q.bindValue(":fileName", _fileName->text());
  q.bindValue(":emailBody", _emailBody->toPlainText());
  q.bindValue(":emailHTML", QVariant(_emailHTML->isChecked(), 0));
  q.exec();

  if (_captive)
    accept();
  else
  {
    _submit->setEnabled(FALSE);
    _close->setText(tr("&Close"));
    _email->clear();
    _cc->clear();
    _emailBody->clear();
    _subject->clear();
    _fileName->clear();
  }
}

void deliverEmail::setProfileId(int profileid)
{
  q.prepare( "SELECT ediprofile_option1 AS emailto, "
             "  ediprofile_option4 AS emailcc, "
             "  ediprofile_option2 AS subject, "
             "  ediprofile_option3 AS body "
             "FROM ediprofile "
             "WHERE ( (ediprofile_id=:ediprofile_id) "
             " AND (ediprofile_type='email') );" );
  q.bindValue(":ediprofile_id", profileid);
  q.exec();
  if (q.first())
  {
    _submit->setEnabled(TRUE);
    _email->setText(q.value("emailto").toString());
    _cc->setText(q.value("emailcc").toString());
    _emailBody->setPlainText(q.value("body").toString()
      .replace("</description>" , _descrip)
      .replace("</docnumber>"   , _docnumber)
      .replace("</doctype>"     , _doctype)
      .replace("</docbody>"     , _docbody));
    _subject->setText(q.value("subject").toString()
      .replace("</description>" , _descrip)
      .replace("</docnumber>"   , _docnumber)
      .replace("</doctype>"     , _doctype)
      .replace("</docbody>"     , _docbody));
            
    //Handle E-mail
    //Don't send messages to myself
    if (_email1 == _fromEmail->text())
      _email1 = QString::null;
    if (_email2 == _fromEmail->text())
      _email2 = QString::null;
    if (_email3 == _fromEmail->text())
      _email3 = QString::null;
      
    //Add token addresses to "To" list if applicable
    if ((_email1.isEmpty()) || (_email->text().count(_email1,Qt::CaseInsensitive)))
      _email->setText(_email->text().remove("</email1>"));
    else
    {
      if (_email->text().stripWhiteSpace()
                        .remove("</email1>")
                        .remove("</email2>")
                        .remove("</email3>")
                        .length())
        _email->setText(_email->text().replace("</email1>", ", " + _email1));
      else
        _email->setText(_email->text().replace("</email1>", _email1));
    }
  
    if ((_email2.isEmpty()) || (_email->text().count(_email2,Qt::CaseInsensitive)))
      _email->setText(_email->text().remove("</email2>"));
    else
    {
      if (_email->text().stripWhiteSpace()
                        .remove("</email1>")
                        .remove("</email2>")
                        .remove("</email3>")
                        .length())
        _email->setText(_email->text().replace("</email2>", ", " + _email2));
      else
        _email->setText(_email->text().replace("</email2>", _email2));
    }
        
    if ((_email3.isEmpty()) || (_email->text().count(_email3,Qt::CaseInsensitive)))
      _email->setText(_email->text().remove("</email3>"));
    else
    {
      if (_email->text().stripWhiteSpace()
                        .remove("</email1>")
                        .remove("</email2>")
                        .remove("</email3>")
                        .length())
        _email->setText(_email->text().replace("</email3>", ", " + _email3));
      else
        _email->setText(_email->text().replace("</email3>", _email3));
    }
    
    //Add token addresses to "CC" list if applicable
    if ((_email1.isEmpty()) || (_cc->text().count(_email1,Qt::CaseInsensitive)))
      _cc->setText(_cc->text().remove("</email1>"));
    else
    {
      if (_cc->text().stripWhiteSpace()
                        .remove("</email1>")
                        .remove("</email2>")
                        .remove("</email3>")
                        .length())
        _cc->setText(_cc->text().replace("</email1>", ", " + _email1));
      else
        _cc->setText(_cc->text().replace("</email1>", _email1));
    }
  
    if ((_email2.isEmpty()) || (_cc->text().count(_email2,Qt::CaseInsensitive)))
      _cc->setText(_cc->text().remove("</email2>"));
    else
    {
      if (_cc->text().stripWhiteSpace()
                        .remove("</email1>")
                        .remove("</email2>")
                        .remove("</email3>")
                        .length())
        _cc->setText(_cc->text().replace("</email2>", ", " + _email2));
      else
        _cc->setText(_cc->text().replace("</email2>", _email2));
    }
        
    if ((_email3.isEmpty()) || (_cc->text().count(_email3,Qt::CaseInsensitive)))
      _cc->setText(_cc->text().remove("</email3>"));
    else
    {
      if (_cc->text().stripWhiteSpace()
                        .remove("</email1>")
                        .remove("</email2>")
                        .remove("</email3>")
                        .length())
        _cc->setText(_cc->text().replace("</email3>", ", " + _email3));
      else
        _cc->setText(_cc->text().replace("</email3>", _email3));
    }
    
    //Build comment detail if applicable
    if (_emailBody->text().count("</comments>") &&
         !_doctype.isEmpty() && _docid)
    {                         
      q.prepare("SELECT comment_user, comment_date, comment_text "
                "FROM comment "
                "WHERE ( (comment_source=:doctype) "
                " AND (comment_source_id=:docid) ) "
                "ORDER BY comment_date;");
      q.bindValue(":doctype", _doctype);
      q.bindValue(":docid", _docid);
      q.exec();
      while (q.next())
      {
        _comments += "-----------------------------------------------------\n";
        _comments += q.value("comment_user").toString();
        _comments += " - ";
        _comments += q.value("comment_date").toString();
        _comments += "\n-----------------------------------------------------\n";
        _comments += q.value("comment_text").toString();
        _comments += "\n\n";
      }
      
      _emailBody->setText(_emailBody->text().replace("</comments>", _comments));
    }
  }
  else
  {
    _submit->setEnabled(FALSE);
    _email->clear();
    _cc->clear();
    _emailBody->clear();
    _subject->clear();
    _fileName->clear();
  }
}
