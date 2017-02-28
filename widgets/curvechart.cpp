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

    auto addBgRect = [](QPainter& p, const QRect& r, int start, int end, const QColor& col) {
        p.save();
        p.setPen({});
        p.setBrush(col);
        p.drawRect(
            r.x()+start,
            r.y(),
            end-start,
            r.height()
        );
        p.restore();
    };

    static int dpiX = physicalDpiX();

    static int lineSize = 2*(dpiX/96);

    painter.setPen(QPen(QColor(Qt::yellow), lineSize, Qt::SolidLine,
                        Qt::FlatCap, Qt::MiterJoin));

    const int   dx    = rect.width()/rc;
    const float range = std::max(0.001f, m_Max - m_Min);

    const int height = rect.height() - 2*lineSize;

    float newMin(99999999999999999), newMax(0);

    // Handle the background color
    int sectionStart = -1;
    QColor sectionColor;

    //TODO cache the curve/ranges for append only models
    for (int j = 1; j < m_pModel->columnCount();j++) {
        QPainterPath path;

        path.moveTo(0, height+lineSize);

        for (qreal i = 0; i < rc; i++) {
            const auto  idx = m_pModel->index(i, j);
            const float val = idx.data().toFloat();

            // update the range
            newMin = std::min(newMin, val);
            newMax = std::max(newMax, val);

            // Detect buggy models
            if (val > m_Max || val < m_Min) {
                slotRowsInserted({}, 0, rc);
                update();
                return;
            }

            path.quadTo(
                {i*dx-dx/2, height - (val*height)/range},
                {i*dx     , height - (val*height)/range}
            );

            const auto bg = idx.data(Qt::BackgroundRole);
            if (bg.canConvert<QColor>()) {
                const auto col = qvariant_cast<QColor>(bg);
                if (sectionStart == -1) {
                    sectionStart = i*dx;
                    sectionColor = col;
                }
                else if (col != sectionColor) {
                    addBgRect(painter, rect, sectionStart, (i*dx), sectionColor);

                    sectionStart = i*dx;
                    sectionColor = col;
                }
            }
            else {
                addBgRect(painter, rect, sectionStart, (i*dx), sectionColor);

                sectionStart = -1;
                sectionColor = QColor();
            }
        }

        if (sectionStart >= 0)
            addBgRect(
                painter, rect, sectionStart, rect.width()+rect.x(), sectionColor
            );

        painter.drawPath(path);
    }

    if (newMax != m_Max || newMin != newMin) {
        m_Max = newMax;
        m_Min = newMin;
        update();
    }
}

QSize CurveChart::sizeHint() const
{
    return {0, 50};
}
