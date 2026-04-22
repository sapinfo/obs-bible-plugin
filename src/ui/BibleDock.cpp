#include "BibleDock.h"
#include "BibleWidget.h"
#include <obs-module.h>

BibleDock::BibleDock(QWidget *parent)
    : QDockWidget(obs_module_text("BibleDock"), parent),
      bibleWidget(new BibleWidget(this)) {
    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    setObjectName("bible_dock");
    setWidget(bibleWidget);
}

BibleDock::~BibleDock() = default;
