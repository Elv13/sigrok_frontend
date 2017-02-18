#pragma once

#include <QtWidgets/QWidget>
#include <QtCore/QDateTime>

class QPushButton;
class QLabel;
class AcquisitionModel;

class Acquisition : public QWidget
{
    Q_OBJECT
public:
    explicit Acquisition(bool showAcquired = true, QWidget* parent = nullptr);

    void setShowAcquire(bool v);

    virtual ~Acquisition();

    void setStatus(const QString& st);
    void setCount(int count);
    void setLast(const QDateTime& ti);

public Q_SLOTS:
    void slotStarted();
    void slotStopped();
    void slotCleared();
    void slotAcquire();

Q_SIGNALS:
    void started();
    void stopped();
    void cleared();
    void acquired();
    void showAcquire(bool);

private:
    QPushButton* m_pAcqB;
    QLabel* m_pStatus;
    QLabel* m_pTime;
    QLabel* m_pCount;
};
