#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QtCore/QAbstractItemModel>

// class RemoteWidgetsReplica;
class QRemoteObjectNode;

struct RemoteWidget
{
    QString m_Type;
    QString m_Name;
    QString m_Uid;
    QAbstractItemModel* m_pModel;
};

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
    void slotPageInserted(const QModelIndex&, int start, int end);
    void createSection(const QPersistentModelIndex& idx);
    void enableEditMode(bool edit);
//     void setMainLabel(float value);
//     void reloadremotewidgets();
//     void buttonPressed();

private:
//     Ui::MainWindow *ui;
    QAbstractItemModel* m_pPageModelReplica;
    QHash<QPersistentModelIndex, bool> m_hLoaded;
    QRemoteObjectNode* registry {nullptr};
    QHash<QString, QDockWidget*> m_lDocks;

    void addWidget(RemoteWidget wdg);
    void initStyle();
    QWidget* createTitlebar(QWidget* parent);
};

#endif // MAINWINDOW_H
