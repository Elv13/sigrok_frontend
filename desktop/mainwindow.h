#pragma once

#include <KXmlGuiWindow>

#include <QtCore/QAbstractItemModel>
#include <QtCore/QUrl>

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

    static MainWindow* instance();

  private:
    StatusBar2* m_pStatusBar;
    ProxyNodeFactoryAdapter* m_pSession;
    void setupActions();

    QUrl fileName;

  private Q_SLOTS:
    QDockWidget* addDock(QWidget* w, const QString& title);
    void settingsConfigure();
    void newFile();
    void openFile();
    void openFile(const QUrl &name);
    void saveFile();
    void saveFileAs();
    void saveFileAs(const QUrl &outputFileName);

    void downloadFinished(KJob* job);
};