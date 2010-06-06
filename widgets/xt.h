#ifndef XT_H
#define XT_H

#include <QScriptEngine>

void setupXt(QScriptEngine *engine);

class Xt
{

  public:

  enum ItemDataRole {
    QtDisplayRole = (Qt::DisplayRole),
    QtDecorationRole = (Qt::DecorationRole),
    QtEditRole = (Qt::EditRole),
    QtToolTipRole = (Qt::ToolTipRole),
    QtStatusTipRole = (Qt::StatusTipRole),
    QtWhatsThisRole = (Qt::WhatsThisRole),
    QtSizeHintRole = (Qt::SizeHintRole),
    RawRole = (Qt::UserRole + 1),
    ScaleRole,
    IdRole,
    RunningSetRole,
    RunningInitRole,
    TotalSetRole,
    TotalInitRole,
    IndentRole
  };

};

#endif // XT_H
