#pragma once

#include <QtWidgets/QWidget>

class QListView;
class QAbstractItemModel;

class ChronoNode;

class Chrono : public QWidget
{
    Q_OBJECT
public:
    explicit Chrono(ChronoNode* n, QWidget* parent = nullptr);
    virtual ~Chrono();
};
