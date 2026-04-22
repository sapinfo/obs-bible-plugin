#include "BibleSettingsWidget.h"
#include <QLabel>
#include <QSpinBox>
#include <QFontComboBox>
#include <QCheckBox>
#include <QFormLayout>

BibleSettingsWidget::BibleSettingsWidget(BiblePluginManager &manager, QWidget *parent)
    : QWidget(parent), pluginManager(manager),
      fontSizeSpinBox(new QSpinBox),
      fontFamilyComboBox(new QFontComboBox),
      autoScrollCheckBox(new QCheckBox("자동 스크롤"))
{
    auto *layout = new QFormLayout(this);

    fontSizeSpinBox->setRange(8, 72);
    fontSizeSpinBox->setValue(pluginManager.get_settings().fontSize);
    layout->addRow(new QLabel("글자 크기:"), fontSizeSpinBox);

    fontFamilyComboBox->setCurrentFont(QFont(pluginManager.get_settings().fontFamily));
    layout->addRow(new QLabel("글꼴:"), fontFamilyComboBox);

    autoScrollCheckBox->setChecked(pluginManager.get_settings().autoScroll);
    layout->addRow(autoScrollCheckBox);

    connect(fontSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &BibleSettingsWidget::onFontSizeChanged);
    connect(fontFamilyComboBox, &QFontComboBox::currentFontChanged,
            this, [this](const QFont &font) { this->onFontFamilyChanged(font.family()); });
    connect(autoScrollCheckBox, &QCheckBox::toggled,
            this, &BibleSettingsWidget::onAutoScrollChanged);
}

void BibleSettingsWidget::onFontSizeChanged(int size)
{
    auto settings = pluginManager.get_settings();
    settings.fontSize = size;
    pluginManager.update_settings(settings);
}

void BibleSettingsWidget::onFontFamilyChanged(const QString &family)
{
    auto settings = pluginManager.get_settings();
    settings.fontFamily = family;
    pluginManager.update_settings(settings);
}

void BibleSettingsWidget::onAutoScrollChanged(bool checked)
{
    auto settings = pluginManager.get_settings();
    settings.autoScroll = checked;
    pluginManager.update_settings(settings);
}