#pragma once

#include <QtWidgets/QWidget>


class Table : public QWidget
{
    Q_OBJECT

public:
    explicit Table(QWidget* parent = nullptr);

    virtual ~Table();
};
