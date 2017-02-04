#include "statusbar.h"

#include "ui_statusbar.h"

class StatusBar2Private : public QObject
{
    Q_OBJECT
public:
    Ui_StatusBar* ui {new Ui_StatusBar};
    StatusBar2* q_ptr;

public Q_SLOTS:
    void slotForwardZoom(int level);
    void slotReset();
};

StatusBar2::StatusBar2(QWidget* parent) : QStatusBar(parent),
    d_ptr(new StatusBar2Private())
{
    d_ptr->q_ptr = this;

    auto w = new QWidget(this);

    d_ptr->ui->setupUi(w);

    addPermanentWidget(w);

    connect(d_ptr->ui->m_pZoomS, &QSlider::valueChanged,
        d_ptr, &StatusBar2Private::slotForwardZoom);
    connect(d_ptr->ui->m_pResetPB, &QToolButton::clicked,
        d_ptr, &StatusBar2Private::slotReset);
}

StatusBar2::~StatusBar2()
{
    
}

void StatusBar2Private::slotForwardZoom(int level)
{
    Q_EMIT q_ptr->zoomLevel(((qreal) level) / 10.0);
}

void StatusBar2::setZoomLevel(qreal level)
{
    d_ptr->ui->m_pZoomS->blockSignals(true);
    d_ptr->ui->m_pZoomS->setValue(level*10);
    d_ptr->ui->m_pZoomL->setText(QString::number(int(level*100))+"%");
    d_ptr->ui->m_pZoomS->blockSignals(false);
}

void StatusBar2Private::slotReset()
{
    ui->m_pZoomS->setValue(10);
}

#include <statusbar.moc>
