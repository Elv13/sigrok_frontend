#pragma once

#include <QtWidgets/QStatusBar>

class StatusBar2Private;

class StatusBar2 : public QStatusBar
{
    Q_OBJECT

public:
    explicit StatusBar2(QWidget* parent = nullptr);
    virtual ~StatusBar2();

private:
    StatusBar2Private* d_ptr;
    Q_DECLARE_PRIVATE(StatusBar2)

};
