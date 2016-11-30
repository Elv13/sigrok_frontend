#pragma once

#include <KConfigDialog>

class ConfigDialog : public KConfigDialog
{
    Q_OBJECT
public:
    explicit ConfigDialog(QWidget* parent);
    virtual ~ConfigDialog();
};
