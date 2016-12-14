#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

// class RemoteWidgetsReplica;

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
    void setMainLabel(float value);
    void reloadremotewidgets();
    void buttonPressed();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
