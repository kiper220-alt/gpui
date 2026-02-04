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

public slots:
    void on_actionAddButton_clicked() const;
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
    void updateStateControls();
    void updateEditControls();
    void updateDependencyControls();

    SystemdItem *m_item{nullptr};
    Ui::SystemdWidget *ui{nullptr};
};

} // namespace preferences

#endif // GPUI_SYSTEMD_WIDGET_H
