/***********************************************************************************************************************
**
** Copyright (C) 2021 BaseALT Ltd. <org@basealt.ru>
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
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

#include "policydialog.h"
#include "ui_policydialog.h"

#include "presentationbuilder.h"

namespace gpui {

PolicyDialog::PolicyDialog(QWidget *parent, const QStandardItem& item)
    : QDialog(parent)
    , ui(new Ui::PolicyDialog())
{
    ui->setupUi(this);

    ui->helpTextEdit->setText(item.data(Qt::UserRole + 2).value<QString>());

    this->setWindowTitle(item.text());

    ui->policyNameLabel->setText(item.text());
    ui->policyNameLabel->setWordWrap(true);

    auto presentation = item.data(Qt::UserRole +5).value<std::shared_ptr<model::presentation::Presentation>>();

    if (presentation)
    {
        auto widgets = ::gui::PresentationBuilder::build(*presentation);

        ui->optionsFrame->setLayout(widgets);
    }

    ui->notConfiguredRadioButton->setChecked(true);

    ui->supportedOnTextEdit->setText(item.data(Qt::UserRole + 4).value<QString>());
}

PolicyDialog::~PolicyDialog()
{
    delete ui;
}

}

Q_DECLARE_METATYPE(std::shared_ptr<::model::presentation::Presentation>)
