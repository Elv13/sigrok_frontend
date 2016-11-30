#pragma once

#include <QtWidgets/QWidget>

class QPushButton;
class AquisitionModel;

class Aquisition : public QWidget
{
    Q_OBJECT
public:
    explicit Aquisition(bool showAquired = true, QWidget* parent = nullptr);

    void setShowAcquire(bool v);

    virtual ~Aquisition();

public Q_SLOTS:
    void slotStarted();
    void slotStopped();
    void slotCleared();
    void slotAquire();

Q_SIGNALS:
    void started();
    void stopped();
    void cleared();
    void aquired();
    void showAquire(bool);

private:
    QPushButton* m_pAcqB;
};
