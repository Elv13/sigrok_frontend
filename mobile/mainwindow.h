#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QtCore/QAbstractItemModel>

// class RemoteWidgetsReplica;
class QRemoteObjectNode;
class QQuickWidget;

struct RemoteWidget
{
    QString m_Type;
    QString m_ModelName;
    QString m_ObjectName;
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
//     QQuickView* m_pPaneView{nullptr};
    QQuickWidget* m_pPaneWidget{nullptr};

private Q_SLOTS:
    void slotPageInserted(const QModelIndex&, int start, int end);
    void createSection(const QPersistentModelIndex& idx);
    void enableEditMode(bool edit);
//     void setMainLabel(float value);
//     void reloadremotewidgets();
//     void buttonPressed();
    void beginLeftPaneDrag();

private:
//     Ui::MainWindow *ui;
    QAbstractItemModel* m_pPageModelReplica;
    QHash<QPersistentModelIndex, bool> m_hLoaded;
    QRemoteObjectNode* registry {nullptr};
    QHash<QString, QDockWidget*> m_lDocks;

    void addWidget(RemoteWidget wdg);
    void initStyle();
    QWidget* createTitlebar(QWidget* parent, MainWindow*);
};

#endif // MAINWINDOW_H
