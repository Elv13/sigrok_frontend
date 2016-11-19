#pragma once

#include <QtWidgets/QWidget>


class AquisitionModel;

class Aquisition : public QWidget
{
    Q_OBJECT
public:
    explicit Aquisition(QWidget* parent = nullptr);

    void setModel(const AquisitionModel* m);

    virtual ~Aquisition();

public Q_SLOT:
    void slotStarted();
    void slotStopped();
    void slotCleared();

Q_SIGNALS:
    void started();
    void stopped();
    void cleared();
};
