#include "configdialog.h"

#include "tutorial4Settings.h"

#include <KLocalizedString>

#include "ui_generalconfig.h"

ConfigDialog::ConfigDialog(QWidget* parent) :
    KConfigDialog(parent, QStringLiteral("settings"), Settings::self())
{
//     if (KConfigDialog::showDialog(QStringLiteral("settings"))) {
//         return;
//     }

    QWidget *generalSettingsDialog = new QWidget;

    auto ui = new Ui_GeneralConfig;

    ui->setupUi(generalSettingsDialog);

    addPage(generalSettingsDialog, i18n("General"), QStringLiteral("package_setting"));
//     connect(dialog, SIGNAL(settingsChanged(QString)), this, SLOT(slotSettingsChanged()));
    setAttribute(Qt::WA_DeleteOnClose);
}

ConfigDialog::~ConfigDialog()
{
    
}
