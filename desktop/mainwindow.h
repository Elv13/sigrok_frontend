#pragma once

#include <KXmlGuiWindow>

#include <QtCore/QAbstractItemModel>
#include <QtCore/QUrl>

#include "ui_mainwindow.h"

class StatusBar2;
class Session;

class KTextEdit;
class QDockWidget;
class KJob;
class DesktopSerializer;
class QTreeView;

class MainWindow : public KXmlGuiWindow, public Ui_MainWindow
{
    friend class DesktopSerializer;
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent=0);
    virtual ~MainWindow();

    static MainWindow* instance();

  private:
    Session* addSession(const QString& name);
    Session* currentSession() const;

    StatusBar2* m_pStatusBar;
    QVector<Session*> m_lSessions;
    void setupActions();
    QHash<QString, QDockWidget*> m_lDocks;
    DesktopSerializer* m_pInterfaceSerializer;
    QTreeView* m_pToolBox;

    QUrl fileName;

  private Q_SLOTS:
    QDockWidget* addDock(QWidget* w, const QString& title, const QString& uid);
    void settingsConfigure();
    void newFile();
    void openFile();
    void openFile(const QUrl &name);
    void saveFile();
    void saveFileAs();
    void saveFileAs(const QUrl &outputFileName);

  public Q_SLOTS:
    void slotTabCloseRequested(int index);
    void slotTabSelected(int index);

    void downloadFinished(KJob* job);
};
