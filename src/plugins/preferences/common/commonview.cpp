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

#include "commonview.h"
#include "ui_commonview.h"

#include "commonitem.h"
#include "item_level_targeting/targetingdialog.h"

#include <mvvm/factories/viewmodelfactory.h>
#include <mvvm/viewmodel/viewmodeldelegate.h>

namespace preferences
{

CommonView::CommonView(QWidget *parent)
    : BasePreferenceWidget(parent)
    , ui(new Ui::CommonView())
    , view_model(nullptr)
    , delegate(std::make_unique<ModelView::ViewModelDelegate>())
{
    ui->setupUi(this);

    connect(ui->targetingToolButton, &QToolButton::clicked, this, &CommonView::onTargetingClicked);
}

CommonView::~CommonView()
{
    delete ui;
}

void CommonView::setItem(ModelView::SessionItem *item)
{
    m_item = dynamic_cast<CommonItem*>(item);

    view_model = ModelView::Factory::CreatePropertyFlatViewModel(item->model());
    view_model->setRootSessionItem(item);

    mapper = std::make_unique<QDataWidgetMapper>();

    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    mapper->setOrientation(Qt::Vertical);

    mapper->setModel(view_model.get());
    mapper->setItemDelegate(delegate.get());
    mapper->setRootIndex(QModelIndex());

    mapper->addMapping(ui->descriptionPlainTextEdit, CommonItem::propertyToInt(CommonItem::DESC));
    mapper->addMapping(ui->stopOnErrorCheckBox, CommonItem::propertyToInt(CommonItem::BYPASS_ERRORS));
    mapper->addMapping(ui->userContextCheckBox, CommonItem::propertyToInt(CommonItem::USER_CONTEXT));
    mapper->addMapping(ui->removeThisCheckBox,  CommonItem::propertyToInt(CommonItem::REMOVE_POLICY));
    mapper->addMapping(ui->applyOnceCheckBox,   CommonItem::propertyToInt(CommonItem::APPLY_ONCE));
    mapper->addMapping(ui->itemLevelCheckBox,   CommonItem::propertyToInt(CommonItem::ITEM_LEVEL_TARGETING));

    mapper->setCurrentModelIndex(view_model->index(0, 1));
}

QString CommonView::name() const
{
    return tr("Common");
}

void CommonView::onTargetingClicked()
{
    if (!m_item)
        return;

    mapper->submit();

    TargetingDialog dialog(this);
    dialog.setContainer(m_item->filters());

    if (dialog.exec() == QDialog::Accepted)
    {
        m_item->setFilters(dialog.container());
    }
}

}
