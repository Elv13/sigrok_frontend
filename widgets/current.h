#pragma once

#include <QtWidgets/QWidget>

class QLabel;
class QAbstractItemModel;

class Current : public QWidget
{
    Q_OBJECT

public:
    explicit Current(QWidget* parent = nullptr);

    void setModel(const QAbstractItemModel* m);
    void setText(const QString& text);

    virtual ~Current();

private:
    QLabel* m_pValue;
};
