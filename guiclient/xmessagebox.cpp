/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#include "xmessagebox.h"

#include <qapplication.h>

int XMessageBox::message( QWidget * parent, QMessageBox::Icon severity,
                          const QString & caption, const QString & text,
                          const QString & button0Text,
                          const QString & button1Text,
                          bool snooze, int defaultButtonNumber, int escapeButtonNumber )
{
  int snoozeButtonNumber = -1;

  int b[3];
  b[0] = 1;
  b[1] = button1Text.isEmpty() ? 0 : 2;
  b[2] = 0;

  if(snooze)
  {
    if(b[1] == 0)
      snoozeButtonNumber = 1;
    else
      snoozeButtonNumber = 2;
    defaultButtonNumber = snoozeButtonNumber;
    escapeButtonNumber = snoozeButtonNumber;
    b[snoozeButtonNumber] = 3;
  }

  int i;
  for( i=0; i<3; i++ ) {
    if ( b[i] && defaultButtonNumber == i )
      b[i] += QMessageBox::Default;
    if ( b[i] && escapeButtonNumber == i )
      b[i] += QMessageBox::Escape;
  }

  QMessageBox *mb = new QMessageBox( caption, text, severity,
                                     b[0], b[1], b[2],
                                     parent, "xtuple_msgbox_snooze", TRUE);
  Q_CHECK_PTR( mb );
  if ( !button0Text.isEmpty() )
    mb->setButtonText( 1, button0Text );
  if ( !button1Text.isEmpty() )
    mb->setButtonText( 2, button1Text );
  if ( snooze )
    mb->setButtonText( 3, QObject::tr("Snooze") );

#ifndef QT_NO_CURSOR
  mb->setCursor( Qt::arrowCursor );
#endif
  int result = -1;
  do {
    QApplication::beep();
    result = mb->exec();
  } while(result == 3);

  delete mb;

  return result - 1;
}

