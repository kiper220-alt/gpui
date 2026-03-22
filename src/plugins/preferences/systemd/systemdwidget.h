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
enum class SystemdConflictStrategy;
enum class SystemdUnitType;

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
    void updateActionColumnsMinimumWidths();
    void updateForcedOptions();
    void attachStrategyComboBox(int row);
    void attachDependencyTypeComboBox(int row, int typeIndex = 0);
    void applyStrategyStateToRow(int row) const;
    QString buildUnitFileTextFromTable(bool with_header) const;
    void fillTableFromUnitFileText(const QString &unitFileText);
    bool validateTableMode(QString &errorText) const;
    bool isUserPolicyContext() const;
    QList<int> allowedEditModesForApplyMode() const;
    void ensureValidEditModeSelection();
    QString editModeHintForCurrentSelection() const;
    SystemdUnitType currentUnitType() const;
    SystemdEditMode currentEditMode() const;
    bool isMachineGlobalUserTarget() const;
    bool isEditForcedByApplyMode() const;
    QString currentDropInName() const;
    void setDropInName(const QString &dropInName);
    bool mandatoryRulesActive() const;
    void ensureActionRowItems(int row);
    int appendActionRow(const QString &section,
                        const QString &key,
                        const QString &value,
                        SystemdConflictStrategy strategy,
                        bool scaffold = false,
                        bool strictMandatory = false,
                        const QString &groupId = QString());
    void setActionRowMetadata(int row, bool scaffold, bool strictMandatory, const QString &groupId);
    bool isScaffoldRow(int row) const;
    bool isStrictMandatoryRow(int row) const;
    QString scaffoldGroupId(int row) const;
    QString sectionTextAtRow(int row) const;
    QString keyTextAtRow(int row) const;
    QString valueTextAtRow(int row) const;
    QStringList oneOfGroupKeys(const QString &groupId) const;
    QStringList keySuggestionsForRow(int row) const;
    bool normalizeKeyForRow(int row, const QString &candidateKey, QString &normalizedKey) const;
    void ensureScaffoldRows();
    void refreshStrategyForAllRows();
    bool canRemoveActionRows(const QSet<int> &rowsToRemove, QString &errorText) const;
    bool isTruthySystemdValue(const QString &value) const;
    void applyActionTableDelegates();
    void moveActionRows(bool moveUp);
    void moveDependencyRows(bool moveUp);

    SystemdItem *m_item{nullptr};
    Ui::SystemdWidget *ui{nullptr};
    bool m_textEditorMode{false};
    int m_preferredFlexibleEditMode{0};
};

} // namespace preferences

#endif // GPUI_SYSTEMD_WIDGET_H
