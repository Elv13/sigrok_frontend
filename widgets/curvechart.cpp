#include "curvechart.h"

#include <QtCore/QAbstractItemModel>
#include <QtCore/QDebug>
#include <QtCore/QVector>

#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QPainterPath>

CurveChart::CurveChart(QWidget* parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(0, 99);
    setStyleSheet("background-color:red;");
}

CurveChart::~CurveChart()
{
    
}

void CurveChart::setModel(QAbstractItemModel* m)
{
    if (m_pModel) {
        disconnect(m_pModel, &QAbstractItemModel::rowsInserted,
            this, &CurveChart::slotRowsInserted);
    }

    m_pModel = m;

    connect(m_pModel, &QAbstractItemModel::rowsInserted,
        this, &CurveChart::slotRowsInserted);

    if (auto rc = m->rowCount())
        slotRowsInserted({}, 0, rc);

    connect(m_pModel, &QAbstractItemModel::modelReset, [this]() {
        m_Min = 99999999999999999;
        m_Max = 0;

        if (auto rc = m_pModel->rowCount())
            slotRowsInserted({}, 0, rc);
    });

    connect(m_pModel, &QAbstractItemModel::dataChanged, [this](const QModelIndex&, const QModelIndex& br) {
        const auto rc = m_pModel->rowCount();

        if (m_Max == 99999999999999999)
            slotRowsInserted({}, 0, rc);

        if (br.row() == rc -1)
            update();
    });
}

void CurveChart::slotRowsInserted(const QModelIndex& parent, int first, int last)
{
    Q_UNUSED(parent)

    // Force a reset
    if (first == 0) {
        m_Min = 99999999999999999;
        m_Max = 0;
    }

    for (int i=first; i <= last;i++) {
        const float v = m_pModel->index(i, 1).data().toFloat();
        m_Min = std::min(m_Min, v);
        m_Max = std::max(m_Max, v);
    }

    update();
}

void CurveChart::paintEvent(QPaintEvent *event)
{
    if (!m_pModel)
        return;

    const auto rect = event->rect();
    const int  rc   = m_pModel->rowCount();

    if (!rc)
        return;

    QPainter painter(this);
    painter.setRenderHints(
        QPainter::Antialiasing     | QPainter::HighQualityAntialiasing |
        QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform
    );

    painter.setPen(QPen(QColor(Qt::yellow), 2, Qt::SolidLine,
                        Qt::FlatCap, Qt::MiterJoin));

    const int   dx    = rect.width()/rc;
    const float range = std::max(0.001f, m_Max - m_Min);

    for (int j = 1; j < m_pModel->columnCount();j++) {
        QPainterPath path;

        path.moveTo(0, rect.height());

        for (qreal i = 0; i < rc; i++) {
            const float val = m_pModel->index(i, j).data().toFloat();

            // Detect buggy models
            if (val > m_Max || val < m_Min) {
                slotRowsInserted({}, 0, rc);
                update();
                return;
            }

            path.quadTo(
                {i*dx-dx/2, rect.height() - (val*rect.height())/range},
                {i*dx     , rect.height() - (val*rect.height())/range}
            );
        }
        painter.drawPath(path);
    }
}

QSize CurveChart::sizeHint() const
{
    return {0, 50};
}
