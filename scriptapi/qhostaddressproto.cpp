#include "scriptapi_internal.h"
#include "qhostaddressproto.h"

QScriptValue SpecialAddresstoScriptValue(QScriptEngine *engine, const enum QHostAddress::SpecialAddress &p)
{
  return QScriptValue(engine, (int)p);
}

void SpecialAddressfromScriptValue(const QScriptValue &obj, enum QHostAddress::SpecialAddress &p)
{
  p = (enum QHostAddress::SpecialAddress)obj.toInt32();
}

void setupQHostAddressProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QHostAddressProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QHostAddress*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQHostAddress, proto);
  engine->globalObject().setProperty("QHostAddress",  constructor);

  // enum QHostAddress::SpecialAddress
  qScriptRegisterMetaType(engine,          SpecialAddresstoScriptValue, SpecialAddressfromScriptValue);
  constructor.setProperty("Null",          QScriptValue(engine,         QHostAddress::Null),            ENUMPROPFLAGS);
  constructor.setProperty("LocalHost",     QScriptValue(engine,         QHostAddress::LocalHost),       ENUMPROPFLAGS);
  constructor.setProperty("LocalHostIPv6", QScriptValue(engine,         QHostAddress::LocalHostIPv6),   ENUMPROPFLAGS);
  constructor.setProperty("Broadcast",     QScriptValue(engine,         QHostAddress::Broadcast),       ENUMPROPFLAGS);
  constructor.setProperty("AnyIPv4",       QScriptValue(engine,         QHostAddress::AnyIPv4),         ENUMPROPFLAGS);
  constructor.setProperty("AnyIPv4",       QScriptValue(engine,         QHostAddress::AnyIPv4),         ENUMPROPFLAGS);
  constructor.setProperty("Any",           QScriptValue(engine,         QHostAddress::Any),             ENUMPROPFLAGS);

}

QScriptValue constructQHostAddress(QScriptContext *context, QScriptEngine  *engine)
{
  QHostAddress *obj = 0;
  if (context->argumentCount() == 1 && context->argument(0).isString())
    obj = new QHostAddress(context->argument(0).toString());
  else
    obj = new QHostAddress();
  return engine->toScriptValue(obj);
}

QHostAddressProto::QHostAddressProto(QObject *parent = 0)
    : QObject(parent)
{
}

void QHostAddressProto::clear()
{
  QHostAddress *item = qscriptvalue_cast<QHostAddress*>(thisObject());
  if (item)
    item->clear();
}

bool QHostAddressProto::isInSubnet(const QHostAddress & subnet, int netmask) const
{
  QHostAddress *item = qscriptvalue_cast<QHostAddress*>(thisObject());
  if (item)
    return item->isInSubnet(subnet, netmask);
  return false;
}

bool QHostAddressProto::isInSubnet(const QPair<QHostAddress, int> & subnet) const
{
  QHostAddress *item = qscriptvalue_cast<QHostAddress*>(thisObject());
  if (item)
    return item->isInSubnet(subnet);
  return false;
}

bool QHostAddressProto::isLoopback() const
{
  QHostAddress *item = qscriptvalue_cast<QHostAddress*>(thisObject());
  if (item)
    return item->isLoopback();
  return false;
}

bool QHostAddressProto::isNull() const
{
  QHostAddress *item = qscriptvalue_cast<QHostAddress*>(thisObject());
  if (item)
    return item->isNull();
  return false;
}

QAbstractSocket::NetworkLayerProtocol QHostAddressProto::protocol() const
{
  QHostAddress *item = qscriptvalue_cast<QHostAddress*>(thisObject());
  if (item)
    return item->protocol();
  return QAbstractSocket::IPv4Protocol;
}

QString QHostAddressProto::scopeId() const
{
  QHostAddress *item = qscriptvalue_cast<QHostAddress*>(thisObject());
  if (item)
    return item->scopeId();
  return QString();
}

void QHostAddressProto::setAddress(quint32 ip4Addr)
{
  QHostAddress *item = qscriptvalue_cast<QHostAddress*>(thisObject());
  if (item)
    item->setAddress(ip4Addr);
}

void QHostAddressProto::setAddress(quint8 * ip6Addr)
{
  QHostAddress *item = qscriptvalue_cast<QHostAddress*>(thisObject());
  if (item)
    item->setAddress(ip6Addr);
}

void QHostAddressProto::setAddress(const quint8 * ip6Addr)
{
  QHostAddress *item = qscriptvalue_cast<QHostAddress*>(thisObject());
  if (item)
    item->setAddress(ip6Addr);
}

void QHostAddressProto::setAddress(const sockaddr * sockaddr)
{
  QHostAddress *item = qscriptvalue_cast<QHostAddress*>(thisObject());
  if (item)
    item->setAddress(sockaddr);
}

bool QHostAddressProto::setAddress(const QString & address)
{
  QHostAddress *item = qscriptvalue_cast<QHostAddress*>(thisObject());
  if (item)
    return item->setAddress(address);
  return false;
}

void QHostAddressProto::setScopeId(const QString & id)
{
  QHostAddress *item = qscriptvalue_cast<QHostAddress*>(thisObject());
  if (item)
    item->setScopeId(id);
}

quint32 QHostAddressProto::toIPv4Address() const
{
  QHostAddress *item = qscriptvalue_cast<QHostAddress*>(thisObject());
  if (item)
    return item->toIPv4Address();
  return quint32();
}

quint32 QHostAddressProto::toIPv4Address(bool * ok) const
{
  QHostAddress *item = qscriptvalue_cast<QHostAddress*>(thisObject());
  if (item)
    return item->toIPv4Address(ok);
  return quint32();
}

Q_IPV6ADDR QHostAddressProto::toIPv6Address() const
{
  QHostAddress *item = qscriptvalue_cast<QHostAddress*>(thisObject());
  if (item)
    return item->toIPv6Address();
  return Q_IPV6ADDR();
}

QString QHostAddressProto::toString() const
{
  QHostAddress *item = qscriptvalue_cast<QHostAddress*>(thisObject());
  if (item)
    return item->toString();
  return QString();
}

