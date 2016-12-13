#include "chrono.h"

// #include "ratesink.h"
#include "ui_chrono.h"

#include "nodes/chrononode.h"

#include <QtWidgets/QPushButton>

#include <QtCore/QDebug>

Chrono::Chrono(ChronoNode* n, QWidget* parent) : QWidget(parent)
{
    Ui_Chrono ui;
    ui.setupUi(this);

    ui.m_pMsDelta ->setChecked(n->hasDeltaColumn    ());
    ui.m_pEpoch   ->setChecked(n->hasMsEpochColumn  ());
    ui.m_pTime    ->setChecked(n->hasTimestampColumn());
    ui.m_pDateTime->setChecked(n->hasDateTimeColumn ());

    connect(ui.m_pMsDelta , &QCheckBox::toggled, n, &ChronoNode::enableDelta    );
    connect(ui.m_pEpoch   , &QCheckBox::toggled, n, &ChronoNode::enableMsEpoch  );
    connect(ui.m_pTime    , &QCheckBox::toggled, n, &ChronoNode::enableTimestamp);
    connect(ui.m_pDateTime, &QCheckBox::toggled, n, &ChronoNode::enableDateTime );
}

Chrono::~Chrono()
{

}

