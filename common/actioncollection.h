#pragma once

#include <QtCore/QObject>

#ifdef WITH_XMLGUI
 class KXmlGuiWindow;
#else
 #include <QtWidgets/QMainWindow>
 typedef QMainWindow KXmlGuiWindow;
#endif

class ActionCollectionPrivate;

class QAction;

class ActionCollection : public  QObject
{
    Q_OBJECT
public:
    explicit ActionCollection(KXmlGuiWindow* parent = nullptr);
    virtual ~ActionCollection();

    QAction* zoonInAction() const;
    QAction* zoonOutAction() const;
    QAction* zoonFitAction() const;
    QAction* zoonResetAction() const;
    QAction* pasteAction() const;

Q_SIGNALS:
    void zoomIn();
    void zoomOut();
    void zoomFit();
    void zoomReset();
    void paste();

private:
    ActionCollectionPrivate* d_ptr;
    Q_DECLARE_PRIVATE(ActionCollection)
};
