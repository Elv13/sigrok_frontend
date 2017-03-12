#pragma once

#include <QtWidgets/QDialog>

class XmlImport : public QDialog
{
    Q_OBJECT

public:
    explicit XmlImport(QWidget* parent = nullptr);

    virtual ~XmlImport();

private:
};
