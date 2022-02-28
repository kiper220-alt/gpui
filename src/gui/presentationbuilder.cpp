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

#include "presentationbuilder.h"

#include "../model/presentation/presentationwidget.h"
#include "../model/presentation/presentationwidgetvisitor.h"
#include "../model/presentation/presentation.h"

#include "../model/presentation/checkbox.h"
#include "../model/presentation/combobox.h"
#include "../model/presentation/decimaltextbox.h"
#include "../model/presentation/dropdownlist.h"
#include "../model/presentation/listbox.h"
#include "../model/presentation/longdecimaltextbox.h"
#include "../model/presentation/multitextbox.h"
#include "../model/presentation/text.h"
#include "../model/presentation/textbox.h"

#include "../model/admx/policy.h"
#include "../model/admx/policyelement.h"
#include "../model/admx/policyenumelement.h"

#include "../model/commands/command.h"
#include "../model/commands/commandgroup.h"

#include "../model/registry/abstractregistrysource.h"

#include "listboxdialog.h"

#include <QVBoxLayout>

#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QListWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpinBox>
#include <QPlainTextEdit>
#include <QTableWidget>
#include <QHeaderView>

#include <QDebug>

#include <iostream>

using namespace model::presentation;
using namespace model::admx;
using namespace model::registry;
using namespace model::command;

namespace gui
{
    template <typename TLayoutItem>
    QLayoutItem* createAndAttachLabel(QWidget* buddy, const QString& text)
    {
        QLabel* label = new QLabel(text.trimmed());
        label->setBuddy(buddy);
        label->setWordWrap(true);

        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(1);

        label->setSizePolicy(sizePolicy);
        buddy->setSizePolicy(sizePolicy);

        TLayoutItem* container = new TLayoutItem();
        container->addWidget(label);
        container->addWidget(buddy);

        return container;
    }

    QHBoxLayout* createCaptions()
    {
        QHBoxLayout* horizontalLayout = new QHBoxLayout();
        QLabel* descriptionLabel = new QLabel(QObject::tr("Description:"));
        QLabel* optionLabel = new QLabel(QObject::tr("Options:"));
        horizontalLayout->addWidget(descriptionLabel);
        horizontalLayout->addWidget(optionLabel);
        return horizontalLayout;
    }

    bool* m_dataChanged = nullptr;
    bool* m_stateEnabled = nullptr;

    class PresentationBuilderPrivate : public PresentationWidgetVisitor {
    public:
        struct ElementInfo
        {
            std::string key;
            std::string value;
            RegistryEntryType type;
            PolicyElement* element;
        };

    public:
        virtual void visit(CheckBox &widget) const override
        {
            QCheckBox* checkBox = new QCheckBox();

            checkBox->setChecked(widget.defaultChecked);
            QLayoutItem* container = createAndAttachLabel<QHBoxLayout>(checkBox, QString::fromStdString(widget.label));

            if (m_policy && m_source)
            {
                const ElementInfo elementInfo = findElementInfo();

                if (m_source->isValuePresent(elementInfo.key, elementInfo.value))
                {
                    checkBox->setChecked(m_source->getValue(elementInfo.key, elementInfo.value).value<bool>());
                }

                checkBox->connect(checkBox, &QCheckBox::toggled, []()
                {
                    *m_dataChanged = true;
                });

                // TODO: Implement correct type on save.
                m_saveDialog->connect(m_saveDialog, &QDialogButtonBox::accepted, [elementInfo, checkBox, this ]() {
                    if (!(*m_stateEnabled))
                    {
                        return;
                    }
                    qWarning() << "Presentation builder::save: " << elementInfo.key.c_str()
                               << " " << elementInfo.value.c_str();
                    int checked = checkBox->checkState() == Qt::Checked ? 1 : 0;
                    m_source->setValue(elementInfo.key, elementInfo.value, RegistryEntryType::REG_DWORD, checked);
                });
            }

            addToLayout(container);
        }

        virtual void visit(ComboBox &widget) const override
        {
            QComboBox* comboBox = new QComboBox();
            comboBox->setCurrentText(QString::fromStdString(widget.defaultValue));
            for (const auto& item: widget.suggestions)
            {
                comboBox->addItem(QString::fromStdString(item));
            }

            if (m_policy && m_source)
            {
                const ElementInfo elementInfo = findElementInfo();

                if (m_source->isValuePresent(elementInfo.key, elementInfo.value))
                {
                    int index = getComboData(m_source->getValue(elementInfo.key, elementInfo.value), elementInfo.element);
                    comboBox->setCurrentIndex(index);
                }

                comboBox->connect(comboBox,  QOverload<int>::of(&QComboBox::currentIndexChanged), [=]()
                {
                    *m_dataChanged = true;
                });

                // TODO: Implement correct type on save.
                m_saveDialog->connect(m_saveDialog, &QDialogButtonBox::accepted, [elementInfo, comboBox, this]()
                {
                    if (!(*m_stateEnabled))
                    {
                        return;
                    }
                    qWarning() << "Presentation builder::save: " << elementInfo.key.c_str()
                               << " " << elementInfo.value.c_str();
                    setComboData(elementInfo.key, elementInfo.value, elementInfo.type, comboBox, elementInfo.element);
                });
            }

            QLayoutItem* container = createAndAttachLabel<QHBoxLayout>(comboBox, QString::fromStdString(widget.label));

            addToLayout(container);
        }

        virtual void visit(DecimalTextBox &widget) const override
        {
            QWidget* textBox = createAnyDecimalTextBox(widget.spin, widget.defaultValue, widget.spinStep);

            QLayoutItem* container = createAndAttachLabel<QHBoxLayout>(textBox, QString::fromStdString(widget.label));

            addToLayout(container);
        }

        virtual void visit(DropdownList &widget) const override
        {
            QComboBox* comboBox = new QComboBox();
            comboBox->setCurrentIndex(widget.defaultItem);

            QLayoutItem* container = createAndAttachLabel<QHBoxLayout>(comboBox, QString::fromStdString(widget.label));

            if (widget.values.size() > 0) {
                for (auto& value : widget.values)
                {
                    comboBox->addItem(QString::fromStdString(value));
                }
                if (widget.defaultItem < widget.values.size())
                {
                    comboBox->setCurrentIndex(widget.defaultItem);
                }
            }

            if (m_policy && m_source)
            {
                const ElementInfo elementInfo = findElementInfo();

                if (m_source->isValuePresent(elementInfo.key, elementInfo.value))
                {
                    int index = getComboData(m_source->getValue(elementInfo.key, elementInfo.value), elementInfo.element);
                    comboBox->setCurrentIndex(index);
                }

                comboBox->connect(comboBox,  QOverload<int>::of(&QComboBox::currentIndexChanged), [=]()
                {
                    *m_dataChanged = true;
                });

                // TODO: Implement correct type on save.
                m_saveDialog->connect(m_saveDialog, &QDialogButtonBox::accepted, [elementInfo, comboBox, this]() {
                    if (!(*m_stateEnabled))
                    {
                        return;
                    }
                    qWarning() << "Presentation builder::save: " << elementInfo.key.c_str()
                               << " " << elementInfo.value.c_str();
                    setComboData(elementInfo.key, elementInfo.value, elementInfo.type, comboBox, elementInfo.element);
                });
            }

            addToLayout(container);
        }

        virtual void visit(ListBox &widget) const override
        {
            QPushButton* button = new QPushButton(QObject::tr("Edit"));

            QLayoutItem* container = createAndAttachLabel<QHBoxLayout>(button, QString::fromStdString(widget.label));

            auto onClicked = [&]()
                {
                gpui::ListBoxDialog* listBox = new gpui::ListBoxDialog(QString::fromStdString(widget.label));
                listBox->setAttribute(Qt::WA_DeleteOnClose);

                if (m_policy && m_source)
                {
                    const ElementInfo elementInfo = findElementInfo();

                    if (m_source->isValuePresent(elementInfo.key, elementInfo.value))
                    {
                        QStringList items = m_source->getValue(elementInfo.key, elementInfo.value).value<QStringList>();
                        qWarning() << "Items debug: " << items;
                        listBox->setItems(items);
                    }

                    listBox->connect(listBox, &gpui::ListBoxDialog::itemsEditingFinished, [=](QStringList items) {
                        if (!(*m_stateEnabled))
                        {
                            return;
                        }
                        qWarning() << "Items debug: " << items;
                        QStringList itemsToAdd;
                        for (const auto& item : items)
                        {
                            if (!item.trimmed().isEmpty())
                            {
                                itemsToAdd.push_back(item);
                            }
                        }

                        m_source->setValue(elementInfo.key, elementInfo.value, RegistryEntryType::REG_MULTI_SZ, itemsToAdd);
                        *m_dataChanged = true;
                    });
                }

                listBox->show();
            };

            QObject::connect(button, &QPushButton::clicked, onClicked);

            addToLayout(container);
        }

        virtual void visit(LongDecimalTextBox &widget) const override
        {
            QWidget* textBox = createAnyDecimalTextBox(widget.spin, widget.defaultValue, widget.spinStep);

            QLayoutItem* container = createAndAttachLabel<QHBoxLayout>(textBox, QString::fromStdString(widget.label));

            addToLayout(container);
        }

        virtual void visit(MultiTextBox &widget) const override
        {
            QTextEdit* textEdit = new QTextEdit();
            textEdit->setMaximumHeight(widget.defaultHeight * textEdit->fontMetrics().height());

            if (m_policy && m_source)
            {
                const ElementInfo elementInfo = findElementInfo();

                if (m_source->isValuePresent(elementInfo.key, elementInfo.value))
                {
                    auto value = m_source->getValue(elementInfo.key, elementInfo.value).value<QString>();
                    textEdit->setPlainText(value);
                }

                textEdit->connect(textEdit, &QTextEdit::textChanged, [=]()
                {
                    *m_dataChanged = true;
                });

                // TODO: Implement correct type on save.
                m_saveDialog->connect(m_saveDialog, &QDialogButtonBox::accepted, [elementInfo, textEdit, this]() {
                    if (!(*m_stateEnabled))
                    {
                        return;
                    }
                    qWarning() << "Presentation builder::save: " << elementInfo.key.c_str()
                               << " " << elementInfo.value.c_str();
                    QStringList data(textEdit->toPlainText());
                    m_source->setValue(elementInfo.key, elementInfo.value, RegistryEntryType::REG_MULTI_SZ, data);
                });
            }

            addToLayout(textEdit);
        }

        virtual void visit(Text &widget) const override
        {
            QLabel* label = new QLabel();
            label->setText(QString::fromStdString(widget.content));
            label->setWordWrap(true);
            label->setAlignment(Qt::AlignHCenter);
            addToLayout(label);
        }

        virtual void visit(TextBox &widget) const override
        {
            QLineEdit* lineEdit = new QLineEdit();
            lineEdit->setText(QString::fromStdString(widget.defaultValue));

            if (m_policy && m_source)
            {
                const ElementInfo elementInfo = findElementInfo();

                if (m_source->isValuePresent(elementInfo.key, elementInfo.value))
                {
                    auto value = m_source->getValue(elementInfo.key, elementInfo.value).value<QString>();
                    lineEdit->setText(value);
                }

                lineEdit->connect(lineEdit, &QLineEdit::textChanged, [=]()
                {
                    *m_dataChanged = true;
                });

                // TODO: Implement correct type on save.
                m_saveDialog->connect(m_saveDialog, &QDialogButtonBox::accepted, [elementInfo, lineEdit, this]() {
                    if (!(*m_stateEnabled))
                    {
                        return;
                    }
                    qWarning() << "Presentation builder::save: " << elementInfo.key.c_str()
                               << " " << elementInfo.value.c_str();
                    QString data(lineEdit->text());
                    m_source->setValue(elementInfo.key, elementInfo.value, RegistryEntryType::REG_SZ,
                                       QVariant::fromValue(data));
                });
            }

            QLayoutItem* container = createAndAttachLabel<QHBoxLayout>(lineEdit, QString::fromStdString(widget.label));

            addToLayout(container);
        }

        void setLayout(QLayout* layout) {
            m_layout = layout;
        }

        void setPolicy(const Policy& policy)
        {
            m_policy = &policy;
        }

        void setRegistrySource(AbstractRegistrySource& source)
        {
            m_source = &source;
        }

        void setCurrentElementName(std::string elementName)
        {
            m_elementName = elementName;
        }

        void setSaveDialog(QDialogButtonBox& saveButton)
        {
            m_saveDialog = &saveButton;
        }

        void setDataChanged(bool& dataChanged)
        {
            m_dataChanged = &dataChanged;
        }

        void setStateEnabled(bool& stateEnabled)
        {
            m_stateEnabled = &stateEnabled;
        }


    private:
        QLayout* m_layout = nullptr;
        const Policy* m_policy = nullptr;
        AbstractRegistrySource* m_source = nullptr;
        QDialogButtonBox* m_saveDialog = nullptr;
        std::string m_elementName = "";

        void addToLayout(QWidget* widget) const {
            if (m_layout) {
                m_layout->addWidget(widget);
            }
        }

        void addToLayout(QLayoutItem* container) const
        {
            if (container)
            {
                m_layout->addItem(container);
            }
        }

        template<typename Number>
        QWidget* createAnyDecimalTextBox(bool spin, Number value, Number step) const {
            if (spin)
            {
                QSpinBox* spinBox = new QSpinBox();
                spinBox->setMinimum(0);
                spinBox->setMaximum(std::numeric_limits<int>::max());
                spinBox->setSingleStep(step);
                spinBox->setValue(value);

                if (m_policy && m_source)
                {
                    const ElementInfo elementInfo = findElementInfo();

                    if (m_source->isValuePresent(elementInfo.key, elementInfo.value))
                    {
                        spinBox->setValue(m_source->getValue(elementInfo.key, elementInfo.value).value<Number>());
                    }

                    spinBox->connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), [=]()
                    {
                        *m_dataChanged = true;
                    });

                    // TODO: Implement correct type on save.
                    m_saveDialog->connect(m_saveDialog, &QDialogButtonBox::accepted, [elementInfo, spinBox, this]() {
                        if (!(*m_stateEnabled))
                        {
                            return;
                        }
                        qWarning() << "Presentation builder::save: " << elementInfo.key.c_str()
                                   << " " << elementInfo.value.c_str();
                        m_source->setValue(elementInfo.key, elementInfo.value, RegistryEntryType::REG_DWORD,
                                           spinBox->value());
                    });
                }

                return spinBox;
            }

            QLineEdit* edit = new QLineEdit();
            edit->setText(QString::number(value));
            edit->setValidator(new QIntValidator(0, std::numeric_limits<int>::max()));

            if (m_policy && m_source)
            {
                const ElementInfo elementInfo = findElementInfo();

                if (m_source->isValuePresent(elementInfo.key, elementInfo.value))
                {
                    edit->setText(QString(m_source->getValue(elementInfo.key, elementInfo.value).value<Number>()));
                }

                edit->connect(edit, &QLineEdit::textChanged, [=]()
                {
                    *m_dataChanged = true;
                });

                // TODO: Implement correct type on save.
                m_saveDialog->connect(m_saveDialog, &QDialogButtonBox::accepted, [elementInfo, edit, this]() {
                    if (!(*m_stateEnabled))
                    {
                        return;
                    }
                    qWarning() << "Presentation builder::save: " << elementInfo.key.c_str()
                               << " " << elementInfo.value.c_str();
                    m_source->setValue(elementInfo.key, elementInfo.value, RegistryEntryType::REG_DWORD,
                                       edit->text().toUInt());
                });
            }

            return edit;
        }

        ElementInfo findElementInfo() const
        {
            if (m_policy && m_source)
            {
                for (const auto& element : m_policy->elements)
                {
                    if (element->id.compare(m_elementName) == 0)
                    {
                        if (element->key.size() > 0)
                        {
                            return { element->key, element->valueName, element->getRegistryEntryType(), element.get() };
                        } else {
                            return { m_policy->key, element->valueName, element->getRegistryEntryType(), element.get() };
                        }
                    }
                }
            }

            qWarning() << "Key and value not found!" << m_elementName.c_str();

            return { "", "", static_cast<RegistryEntryType>(0), nullptr };
        }

        int getComboData(QVariant value, PolicyElement* element) const
        {
            PolicyEnumElement* enumElement = dynamic_cast<PolicyEnumElement*>(element);
            if (enumElement)
            {
                int index = 0;
                for (const auto& it : enumElement->items)
                {
                    auto sv = dynamic_cast<StringValue*>(it.second.get());
                    if (sv)
                    {
                        if (sv->value.compare(value.toString().toStdString()) == 0)
                        {
                            return index;
                        }
                    }
                    auto dv = dynamic_cast<DecimalValue*>(it.second.get());
                    if (dv)
                    {
                        if (dv->value == value.toUInt())
                        {
                            return index;
                        }
                    }
                    auto qv = dynamic_cast<LongDecimalValue*>(it.second.get());
                    if (qv)
                    {
                        if (qv->value == value.toULongLong())
                        {
                            return index;
                        }
                    }
                    index++;
                }
            }
            return 0;
        }

        void setComboData(const std::string& key, const std::string& valueName, RegistryEntryType type,
                          QComboBox const* comboBox, PolicyElement* element) const
        {
            std::string stringValue;
            uint32_t dwordValue = 0;
            uint64_t qwordValue = 0;
            PolicyEnumElement* enumElement = dynamic_cast<PolicyEnumElement*>(element);
            if (enumElement)
            {
                auto begin = enumElement->items.begin();
                auto it = std::next(begin, comboBox->currentIndex());
                if (it != enumElement->items.end())
                {
                    auto sv = dynamic_cast<StringValue*>(it->second.get());
                    if (sv)
                    {
                        stringValue = sv->value;
                    }
                    auto dv = dynamic_cast<DecimalValue*>(it->second.get());
                    if (dv)
                    {
                        dwordValue = dv->value;
                    }
                    auto qv = dynamic_cast<LongDecimalValue*>(it->second.get());
                    if (qv)
                    {
                        qwordValue = qv->value;
                    }
                    qWarning() << "Element: "
                               << it->first.c_str()
                               << " : " << stringValue.c_str()
                               << " : " << dwordValue << " : " << qwordValue;
                }
            }

            switch (type) {
            case REG_SZ:
            case REG_BINARY:
            case REG_EXPAND_SZ:
                m_source->setValue(key, valueName, type, QString::fromStdString(stringValue));
                break;
            case REG_DWORD:
            case REG_DWORD_BIG_ENDIAN:
                m_source->setValue(key, valueName, type, dwordValue);
                break;
            case REG_QWORD:
                m_source->setValue(key, valueName, type, QVariant::fromValue(qwordValue));
                break;
            case REG_MULTI_SZ:
            {
                m_source->setValue(key, valueName, type, QStringList(QString::fromStdString(stringValue)));
            }   break;
            default:
                qWarning() << "Unable to detect value type for element with key: "
                           << key.c_str()
                           << " value: "
                           << valueName.c_str();
                break;
            }
        }
    };

    PresentationBuilderPrivate* PresentationBuilder::d = new PresentationBuilderPrivate();

    QVBoxLayout* PresentationBuilder::build(const PresentationBuilderParams& params)
    {
        QVBoxLayout* layout = new QVBoxLayout();
        d->setLayout(layout);
        d->setPolicy(params.policy);
        d->setRegistrySource(params.source);
        d->setSaveDialog(params.saveButton);
        d->setDataChanged(params.dataChanged);
        d->setStateEnabled(params.stateEnabled);

        QHBoxLayout* captions = createCaptions();
        layout->addLayout(captions);

        for (const auto& widget : params.presentation.widgets) {
            QWidget* policyWidget = nullptr;
            d->setCurrentElementName(widget.first);
            widget.second->accept(*d);

            if (policyWidget) {
                layout->addWidget(policyWidget);
            }
        }
        layout->addStretch();

        return layout;
    }
}
