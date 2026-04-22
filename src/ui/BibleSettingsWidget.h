#ifndef BIBLE_SETTINGS_WIDGET_H
#define BIBLE_SETTINGS_WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QSpinBox>
#include <QFontComboBox>
#include <QCheckBox>
#include <QFormLayout>

class BiblePluginManager;

class BibleSettingsWidget : public QWidget {
    Q_OBJECT
public:
    explicit BibleSettingsWidget(BiblePluginManager &manager, QWidget *parent = nullptr);
    ~BibleSettingsWidget() override = default;

private slots:
    void onFontSizeChanged(int size);
    void onFontFamilyChanged(const QString &family);
    void onAutoScrollChanged(bool checked);

private:
    BiblePluginManager &pluginManager;
    QSpinBox *fontSizeSpinBox;
    QFontComboBox *fontFamilyComboBox;
    QCheckBox *autoScrollCheckBox;
};

#endif // BIBLE_SETTINGS_WIDGET_H