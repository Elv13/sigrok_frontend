#pragma once

#include <QtWidgets/QWidget>

class Xml : public QWidget
{
    Q_OBJECT

public:
    explicit Xml(QWidget* parent = nullptr);

    virtual ~Xml();

private Q_SLOTS:
    void slotSettings();
    void slotOpen();
};
