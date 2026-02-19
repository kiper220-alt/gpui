#ifndef GPUI_SYSTEMD_WIDGET_TEST_H
#define GPUI_SYSTEMD_WIDGET_TEST_H

#include <QtTest>

namespace tests
{

class SystemdWidgetTest : public QObject
{
    Q_OBJECT

private slots:
    void scaffoldCreateModes();
    void noScaffoldInOverride();
    void autocompleteEditorsUseCatalog();
    void strictMandatoryKeyLocked();
    void oneOfScaffoldKeySelectableWithinGroupOnly();
    void removeMandatoryRows();
    void validationForMandatoryRows();
    void moveActionsRowsNoCrash();
    void moveDependencyRowsNoCrash();
};

} // namespace tests

#endif // GPUI_SYSTEMD_WIDGET_TEST_H
