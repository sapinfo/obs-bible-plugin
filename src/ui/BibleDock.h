#ifndef BIBLE_DOCK_H
#define BIBLE_DOCK_H

#include <QDockWidget>
#include <QString>

class BiblePluginManager;
class BibleWidget;

class BibleDock : public QDockWidget {
    Q_OBJECT
public:
    explicit BibleDock(const QString &title, BiblePluginManager &manager, QWidget *parent = nullptr);
    ~BibleDock() override;

private:
    BiblePluginManager &pluginManager;
    BibleWidget *bibleWidget;
};

#endif // BIBLE_DOCK_H