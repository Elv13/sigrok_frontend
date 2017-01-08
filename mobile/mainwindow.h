#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QtCore/QAbstractItemModel>

// class RemoteWidgetsReplica;
class QRemoteObjectNode;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

//     RemoteWidgetsReplica* rw;

private Q_SLOTS:
    void addDock(QWidget* w, const QString& title);
    void slotPageInserted(const QModelIndex&, int start, int end);
    void createSection(const QPersistentModelIndex& idx);
//     void setMainLabel(float value);
//     void reloadremotewidgets();
//     void buttonPressed();

private:
    Ui::MainWindow *ui;
    QAbstractItemModel* m_pPageModelReplica;
    QHash<QPersistentModelIndex, bool> m_hLoaded;
    QRemoteObjectNode* registry {nullptr};
    QHash<QString, QWidget*> m_lDocks;

    void addWidget(const QString& type, QAbstractItemModel* model, const QString& uid);
};

#endif // MAINWINDOW_H
