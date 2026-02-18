#ifndef GPUI_SYSTEMD_WIDGET_H
#define GPUI_SYSTEMD_WIDGET_H

#include "common/basepreferencewidget.h"

#include <QtWidgets>

QT_BEGIN_NAMESPACE
namespace Ui
{
class SystemdWidget;
}
QT_END_NAMESPACE

namespace ModelView
{
class SessionItem;
}

namespace preferences
{

class SystemdItem;
enum class SystemdEditMode;

class SystemdWidget : public BasePreferenceWidget
{
public:
    Q_OBJECT

public:
    explicit SystemdWidget(QWidget *parent = nullptr, SystemdItem *item = nullptr);
    ~SystemdWidget() override;

    void setItem(ModelView::SessionItem *item) override;
    bool validate() override;
    QString name() const override;

private slots:
    void submit() override;
    void on_switchEditorModePushButton_clicked();
    void on_actionMoveUpButton_clicked();
    void on_actionMoveDownButton_clicked();
    void on_dependMoveUpButton_clicked();
    void on_dependMoveDownButton_clicked();

public slots:
    void on_actionAddButton_clicked();
    void on_actionsClearButton_clicked();
    void on_actionRemoveButton_clicked();

    void on_dependAddButton_clicked();
    void on_dependsClearButton_clicked();
    void on_dependRemoveButton_clicked();

private:
    SystemdWidget(const SystemdWidget &) = delete;
    SystemdWidget(SystemdWidget &&) = delete;
    SystemdWidget &operator=(const SystemdWidget &) = delete;
    SystemdWidget &operator=(SystemdWidget &&) = delete;

    void readEditTable();
    void writeEditTable();

    void readDependencyTable();
    void writeDependencyTable();

    void updateUiForUnitType();
    void updatePolicyTargetControls();
    void updateStateControls();
    void updateEditControls();
    void updateDependencyControls();
    void updateEditModeAvailability();
    void updateEditorModeUi();
    void attachStrategyComboBox(int row);
    void applyStrategyStateToRow(int row) const;
    QString buildUnitFileTextFromTable(bool with_header) const;
    void fillTableFromUnitFileText(const QString &unitFileText);
    bool validateTableMode(QString &errorText) const;
    bool isUserPolicyContext() const;
    QList<int> allowedEditModesForApplyMode() const;
    void ensureValidEditModeSelection();
    QString editModeHintForCurrentSelection() const;

    SystemdItem *m_item{nullptr};
    Ui::SystemdWidget *ui{nullptr};
    bool m_textEditorMode{false};
};

} // namespace preferences

#endif // GPUI_SYSTEMD_WIDGET_H
