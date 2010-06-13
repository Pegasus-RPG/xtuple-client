#ifndef XT_H
#define XT_H

#include <QScriptEngine>

void setupXt(QScriptEngine *engine);

class Xt
{

  public:

  enum ItemDataRole {
    RawRole = (Qt::UserRole + 1),
    ScaleRole,
    IdRole,
    RunningSetRole,
    RunningInitRole,
    TotalSetRole,
    TotalInitRole,
    IndentRole,
    DeletedRole
  };

};

#endif // XT_H
