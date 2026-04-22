#pragma once
#include <QWidget>

class QLineEdit;
class QPushButton;
class QListView;
class QStandardItemModel;
class QModelIndex;

class BibleWidget : public QWidget {
    Q_OBJECT
public:
    explicit BibleWidget(QWidget *parent = nullptr);
    ~BibleWidget() override;

private slots:
    void onSearchTriggered();
    void onBookClicked(const QModelIndex &index);
    void onVerseCurrentChanged(const QModelIndex &current, const QModelIndex &previous);
    void onToggleVisibleClicked();

    // manager → UI
    void onListChanged();
    void onVerseChanged(QString reference, QString body, bool visible);

private:
    void populateBookList();
    void refreshVerseList();
    void syncSelectionFromManager();

    // 좌측
    QLineEdit          *searchEdit;
    QPushButton        *searchBtn;
    QListView          *verseView;
    QStandardItemModel *verseModel;
    QPushButton        *toggleBtn;

    // 우측
    QListView          *bookView;
    QStandardItemModel *bookModel;

    bool suppress_selection_feedback = false;
};
