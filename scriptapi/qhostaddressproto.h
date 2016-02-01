#ifndef __QHOSTADDRESSPROTO_H__
#define __QHOSTADDRESSPROTO_H__

#include <QObject>
#include <QtScript>
#include <QAbstractSocket>
#include <QString>
#include <QHostAddress>

void setupQHostAddressProto(QScriptEngine *engine);
QScriptValue constructQHostAddress(QScriptContext *context, QScriptEngine *engine);

Q_DECLARE_METATYPE(enum QHostAddress::SpecialAddress)
Q_DECLARE_METATYPE(QHostAddress*)

class QHostAddressProto : public QObject, public QScriptable
{
    Q_OBJECT
public:
    QHostAddressProto(QObject *parent);

    Q_INVOKABLE void                                  clear();
    Q_INVOKABLE bool                                  isInSubnet(const QHostAddress & subnet, int netmask) const;
    Q_INVOKABLE bool                                  isInSubnet(const QPair<QHostAddress, int> & subnet) const;
    Q_INVOKABLE bool                                  isLoopback() const;
    Q_INVOKABLE bool                                  isNull() const;
    Q_INVOKABLE QAbstractSocket::NetworkLayerProtocol protocol() const;
    Q_INVOKABLE QString	                              scopeId() const;
    Q_INVOKABLE void                                  setAddress(quint32 ip4Addr);
    Q_INVOKABLE void                                  setAddress(quint8 * ip6Addr);
    Q_INVOKABLE void                                  setAddress(const quint8 * ip6Addr);
    Q_INVOKABLE void                                  setAddress(const sockaddr * sockaddr);
    Q_INVOKABLE bool                                  setAddress(const QString & address);
    Q_INVOKABLE void                                  setScopeId(const QString & id);
    Q_INVOKABLE quint32                               toIPv4Address() const;
    Q_INVOKABLE quint32                               toIPv4Address(bool * ok) const;
    Q_INVOKABLE Q_IPV6ADDR                            toIPv6Address() const;
    Q_INVOKABLE QString	                              toString() const;
};

#endif // QHOSTADDRESSPROTO_H
