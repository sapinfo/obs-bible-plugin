#pragma once
#include <QDockWidget>

class BibleWidget;

class BibleDock : public QDockWidget {
    Q_OBJECT
public:
    explicit BibleDock(QWidget *parent = nullptr);
    ~BibleDock() override;
private:
    BibleWidget *bibleWidget;
};
