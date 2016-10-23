#pragma once

#include <KXmlGuiWindow>

#include <QtCore/QAbstractItemModel>

#include "ui_mainwindow.h"

class StatusBar2;
class ProxyNodeFactoryAdapter;

class KTextEdit;
class QDockWidget;
class KJob;

class MainWindow : public KXmlGuiWindow, public Ui_MainWindow
{
    Q_OBJECT

  public:
    MainWindow(QWidget *parent=0);

    static QDockWidget* addDock(QWidget* w, const QString& title);

  private:
    KTextEdit* textArea;
    StatusBar2* m_pStatusBar;
    ProxyNodeFactoryAdapter* m_pSession;
    void setupActions();

    QString fileName;

  private Q_SLOTS:
    void newFile();
    void openFile();
    void saveFile();
    void saveFileAs();
    void saveFileAs(const QString &outputFileName);

    void downloadFinished(KJob* job);
};
