/***********************************************************************************************************************
**
** Copyright (C) 2026 BaseALT Ltd. <org@basealt.ru>
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
***********************************************************************************************************************/

#include "filematchwidget.h"
#include "ui_filematchwidget.h"

namespace preferences
{

namespace
{
const QString kType   = QStringLiteral("type");
const QString kPath   = QStringLiteral("path");
const QString kFolder = QStringLiteral("folder");
const QString kMin    = QStringLiteral("min");
const QString kMax    = QStringLiteral("max");
const QString kGte    = QStringLiteral("gte");
const QString kLte    = QStringLiteral("lte");

const QStringList fileTypeOrder = {
    QStringLiteral("EXISTS"),
    QStringLiteral("VERSION"),
};
} // namespace

FileMatchFilterWidget::FileMatchFilterWidget(QWidget *parent)
    : TargetingFilterWidget(parent)
    , ui(new Ui::FileMatchWidget)
{
    ui->setupUi(this);

    ui->pathToolButton_1->setEnabled(false);
    ui->pathToolButton_1->setToolTip(tr("Selection dialog is not implemented yet"));
    ui->pathToolButton_2->setEnabled(false);
    ui->pathToolButton_2->setToolTip(tr("Selection dialog is not implemented yet"));

    connect(ui->typeComboBox_1, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FileMatchFilterWidget::onMatchTypeChanged);
    connect(ui->typeComboBox_2, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FileMatchFilterWidget::onMatchTypeChanged);

    ui->stackedWidget->setCurrentIndex(0);
}

FileMatchFilterWidget::~FileMatchFilterWidget()
{
    delete ui;
}

QStringList FileMatchFilterWidget::knownKeys() const
{
    return {kType, kPath, kFolder, kMin, kMax, kGte, kLte};
}

void FileMatchFilterWidget::onMatchTypeChanged(int index)
{
    const int page = (index == 2) ? 1 : 0; // "Match file version" = index 2 in combo -> page_2
    ui->stackedWidget->setCurrentIndex(page);

    ui->typeComboBox_1->setCurrentIndex(index);
    ui->typeComboBox_2->setCurrentIndex(index);
}

void FileMatchFilterWidget::readFromExtras(const QMap<QString, QString> &extras)
{
    const QString type = extras.value(kType, QStringLiteral("EXISTS"));
    const bool isVersion = (type == QLatin1String("VERSION"));

    if (isVersion)
    {
        ui->stackedWidget->setCurrentIndex(1);
        ui->typeComboBox_2->setCurrentIndex(2);
        ui->pathLineEdit_2->setText(extras.value(kPath));

        ui->firstLineEdit_2->setText(extras.value(kMin, QStringLiteral("0.0.0.0")));
        ui->secondLineEdit_2->setText(extras.value(kMax, QStringLiteral("0.0.0.0")));

        const bool gte = extras.value(kGte, QStringLiteral("0")) != QLatin1String("0");
        ui->firstComboBox_2->setCurrentIndex(gte ? 0 : 1);

        const bool lte = extras.value(kLte, QStringLiteral("0")) != QLatin1String("0");
        ui->secondComboBox_2->setCurrentIndex(lte ? 1 : 0);
    }
    else
    {
        ui->stackedWidget->setCurrentIndex(0);
        const bool isFolder = extras.value(kFolder, QStringLiteral("0")) != QLatin1String("0");
        ui->typeComboBox_1->setCurrentIndex(isFolder ? 1 : 0);
        ui->pathLineEdit_1->setText(extras.value(kPath));
    }
}

QMap<QString, QString> FileMatchFilterWidget::writeToExtras() const
{
    QMap<QString, QString> result;

    const int page = ui->stackedWidget->currentIndex();

    if (page == 1)
    {
        result[kType] = QStringLiteral("VERSION");
        result[kPath] = ui->pathLineEdit_2->text();
        result[kMin]  = ui->firstLineEdit_2->text();
        result[kMax]  = ui->secondLineEdit_2->text();
        result[kGte]  = (ui->firstComboBox_2->currentIndex() == 0)
                            ? QStringLiteral("1") : QStringLiteral("0");
        result[kLte]  = (ui->secondComboBox_2->currentIndex() == 1)
                            ? QStringLiteral("1") : QStringLiteral("0");
    }
    else
    {
        const int typeIdx = ui->typeComboBox_1->currentIndex();
        if (typeIdx == 1)
        {
            result[kType]   = QStringLiteral("EXISTS");
            result[kFolder] = QStringLiteral("1");
        }
        else
        {
            result[kType] = QStringLiteral("EXISTS");
        }
        result[kPath] = ui->pathLineEdit_1->text();
    }

    return result;
}

} // namespace preferences
