/*
 *This file is part of the xTuple ERP: PostBooks Edition, a free and
 *open source Enterprise Resource Planning software suite,
 *Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 *It is licensed to you under the Common Public Attribution License
 *version 1.0, the full text of which(including xTuple-specific Exhibits)
 *is available at www.xtuple.com/CPAL.  By using this software, you agree
 *to be bound by its terms.
 */

#include "xwebsync.h"
#include "xwebsync_p.h"

#include <QObject>

/*!
  Constructs a XWebSync with the specified \a parent.
 */
XWebSync::XWebSync(QObject *parent) :
  QObject(parent), d_ptr(new XWebSyncPrivate)
{
}

/*!
  Destroys the XWebSync.
*/
XWebSync::~XWebSync()
{
}

/*!
  \property XWebSync::data
  \brief the data of the XWebSync
 */
void XWebSync::setData(const QString &data)
{
  Q_D(XWebSync);
  if (d->data != data) {
      d->data = data;
      emit dataChanged(d->data);
  }
}

QString XWebSync::data() const
{
  Q_D(const XWebSync);
  return d->data;
}

/*!
  \fn void XWebSync::dataChanged(const QString &data)

  This signal is emitted whenever the data of the web sync changes.
  The \a data string specifies the new data.

  \sa XWebSync::data()
*/

/*!
  \property XWebSync::query
  \brief the title of the XWebSync
 */
void XWebSync::setQuery(const QString &query)
{
  Q_D(XWebSync);
  if (d->query != query) {
      d->query = query;
      emit queryChanged(d->query);
  }
}

QString XWebSync::query() const
{
  Q_D(const XWebSync);
  return d->query;
}

/*!
  \fn void XWebSync::queryChanged(const QString &query)

  This signal is emitted whenever the query of the web sync changes.
  The \a query string specifies the new query.

  \sa XWebSync::query()
*/

/*!
  \property XWebSync::title
  \brief the title of the XWebSync
 */
void XWebSync::setTitle(const QString &title)
{
  Q_D(XWebSync);
  if (d->title != title) {
      d->title = title;
      emit titleChanged(d->title);
  }
}

QString XWebSync::title() const
{
  Q_D(const XWebSync);
  return d->title;
}

/*!
  \fn void XWebSync::titleChanged(const QString &title)

  This signal is emitted whenever the title of the web sync changes.
  The \a title string specifies the new title.

  \sa XWebSync::title()
*/

/*!
  \fn void XWebSync::testChanged(QJsonObject &test)

  This signal is emitted whenever the test of the web sync changes.
  The \a test string specifies the new test.

  \sa XWebSync::test()
*/

/*!
  \fn void XWebSync::executeRequest()

  This signal is emitted whenever the execute() function is called.
*/
