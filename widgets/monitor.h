#pragma once

#include <QtWidgets/QWidget>

class QLabel;

class Monitor : public QWidget
{
    Q_OBJECT

public:
    explicit Monitor(QWidget* parent = nullptr);

    virtual ~Monitor();

    void setName(const QString& s);
    void setType(const QString& s);
    void setTime(const QString& s);
    void setContent(const QString& s);

private:
    QLabel* m_pName;
    QLabel* m_pType;
    QLabel* m_pTime;
    QLabel* m_pContent;
};
