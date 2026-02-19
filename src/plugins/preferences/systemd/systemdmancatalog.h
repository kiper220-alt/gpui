#ifndef GPUI_SYSTEMD_MAN_CATALOG_H
#define GPUI_SYSTEMD_MAN_CATALOG_H

#include <QtCore>

#include "systemditem.h"

namespace preferences
{

struct MandatoryGroup
{
    QString id;
    QString section;
    QStringList keys;
    QString defaultKey;
};

struct MandatoryProfile
{
    QList<QPair<QString, QString>> strictKeys;
    QList<MandatoryGroup> oneOfGroups;
    bool requireServiceRemainAfterExitWithExecStopOnly{false};
};

struct MandatoryScaffoldEntry
{
    QString section;
    QString key;
    bool strict{false};
    QString groupId;
};

class SystemdManCatalog
{
public:
    static QStringList sectionsForUnitType(SystemdUnitType type);
    static QStringList keysFor(SystemdUnitType type, const QString &section);
    static MandatoryProfile mandatoryProfile(SystemdUnitType type);
    static QList<MandatoryScaffoldEntry> mandatoryScaffold(SystemdUnitType type);
    static QPair<QString, QString> defaultSectionKey(SystemdUnitType type);
};

} // namespace preferences

#endif // GPUI_SYSTEMD_MAN_CATALOG_H
