#ifndef GPUI_SYSTEMD_WIDGET_H
#define GPUI_SYSTEMD_WIDGET_H

#include "common/basepreferencewidget.h"

#include <QtWidgets>

QT_BEGIN_NAMESPACE
namespace Ui { class SystemdWidget; }
QT_END_NAMESPACE

class QDataWidgetMapper;

namespace ModelView
{
    class ViewModel;
    class ViewModelDelegate;
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
    explicit SystemdWidget(QWidget* parent = nullptr, SystemdItem* item = nullptr);
    ~SystemdWidget() override;

    void setItem(ModelView::SessionItem *item) override;

    bool validate() override;

    QString name() const override;

private slots:
    void submit() override;

public slots:
    void on_actionComboBox_currentIndexChanged(int index);
    void on_actionAddButton_clicked() const;
    void on_actionsClearButton_clicked();
    void on_actionRemoveButton_clicked();

    void on_dependAddButton_clicked();
    void on_dependsClearButton_clicked();
    void on_dependRemoveButton_clicked();

private:
    SystemdWidget(const SystemdWidget&)            = delete;   // copy ctor
    SystemdWidget(SystemdWidget&&)                 = delete;   // move ctor
    SystemdWidget& operator=(const SystemdWidget&) = delete;   // copy assignment
    SystemdWidget& operator=(SystemdWidget&&)      = delete;   // move assignment

private:
    void readEditTable();
    void writeEditTable();

    void readDependencyTable();
    void writeDependencyTable();

private:
    //!< Underlying item of this view.
    SystemdItem* m_item {nullptr};

    std::unique_ptr<ModelView::ViewModel> view_model;
    std::unique_ptr<ModelView::ViewModelDelegate> delegate;

private:
    Ui::SystemdWidget *ui {nullptr};
};

}

#endif // GPUI_SYSTEMD_WIDGET_H
