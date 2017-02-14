#pragma once

#include <QtWidgets/QWidget>
#include <QtCore/QDateTime>

class QPushButton;
class QLabel;
class AquisitionModel;

class Aquisition : public QWidget
{
    Q_OBJECT
public:
    explicit Aquisition(bool showAquired = true, QWidget* parent = nullptr);

    void setShowAcquire(bool v);

    virtual ~Aquisition();

    void setStatus(const QString& st);
    void setCount(int count);
    void setLast(const QDateTime& ti);

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
    QLabel* m_pStatus;
    QLabel* m_pTime;
    QLabel* m_pCount;
};
