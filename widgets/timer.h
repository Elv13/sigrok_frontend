#pragma once

#include <QtWidgets/QWidget>

class QCheckBox;

class Q_DECL_EXPORT Timer : public QWidget
{
    Q_OBJECT

public:
    explicit Timer(QWidget* parent = nullptr);

    virtual ~Timer();

    void setActive(bool a);

public Q_SLOTS:
    void slotSeconds(int v);
    void slotsMs(int v);
    void slotActivated(bool);

Q_SIGNALS:
    void secondChanged(int);
    void millisecondChanged(int);
    void activated(bool);

private:
    QCheckBox* m_pCheck;
};
