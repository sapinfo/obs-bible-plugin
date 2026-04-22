#include "BibleDock.h"
#include "BibleWidget.h"
#include "BiblePluginManager.h"
#include <QVBoxLayout>

BibleDock::BibleDock(const QString &title, BiblePluginManager &manager, QWidget *parent)
    : QDockWidget(title, parent), pluginManager(manager), bibleWidget(nullptr)
{
    // Set widget features
    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    // Create the main widget (which contains the UI)
    bibleWidget = new BibleWidget(pluginManager, this);
    setWidget(bibleWidget);
}

BibleDock::~BibleDock() {
    // bibleWidget will be deleted by Qt's parent-child mechanism
}