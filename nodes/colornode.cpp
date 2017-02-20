#include "colornode.h"

#include "proxies/columnproxy.h"
#include "proxies/coloredrangeproxy.h"

#include <QtWidgets/QScrollBar>
#include <KColorButton>

#include "widgets/range.h"

#include "proxies/rangeproxy.h"

#include <QtCore/QDebug>
#include <QtCore/QJsonArray>

#if QT_VERSION < 0x050700
//Q_FOREACH is deprecated and Qt CoW containers are detached on C++11 for loops
template<typename T>
const T& qAsConst(const T& v)
{
    return const_cast<const T&>(v);
}
#endif

struct SerializedRange
{
    QString column;
    QString delimiter;
    QString bg;
    QString fg;
};

class ColorNodePrivate : public QObject
{
    Q_OBJECT
public:
    ColorNodePrivate(ColorNode* p) : QObject(p) {}

    Range* m_pWidget {nullptr};

    ColoredRangeProxy* m_pRangeProxy {new ColoredRangeProxy(this)};

    QHash<QString, QList<SerializedRange*>> m_hlSerializedRanges;

    ColorNode* q_ptr;

public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
    void slotRowsInserted(const QModelIndex& parent, int first, int last);
    void slotModelReset();
};

ColorNode::ColorNode(AbstractSession* sess) : ProxyNode(sess), d_ptr(new ColorNodePrivate(this))
{
    d_ptr->q_ptr = this;
    connect(this, &ProxyNode::modelChanged, d_ptr, &ColorNodePrivate::slotModelChanged);
    connect(d_ptr->m_pRangeProxy, &QAbstractItemModel::rowsInserted, d_ptr, &ColorNodePrivate::slotRowsInserted);
    connect(d_ptr->m_pRangeProxy, &QAbstractItemModel::modelReset, d_ptr, &ColorNodePrivate::slotModelReset);
}

ColorNode::~ColorNode()
{
    QList<SerializedRange*> ret;

    QHash<QString, QList<SerializedRange*>>::const_iterator i;
    for (i = d_ptr->m_hlSerializedRanges.constBegin(); i != d_ptr->m_hlSerializedRanges.constEnd(); ++i) {
        for (int j=0;j< i.value().size(); j++)
            ret << i.value()[j];
    }

    while (!ret.isEmpty())
        delete ret.takeLast();

    delete d_ptr;
}

QString ColorNode::title() const
{
    return QStringLiteral("Color");
}

QString ColorNode::id() const
{
    return QStringLiteral("color_node");
}

void ColorNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);

    QJsonArray columns;

    for (int i = 0; i< d_ptr->m_pRangeProxy->rowCount(); i++) {
        const auto idx = d_ptr->m_pRangeProxy->index(i, 0);
        const QString column = idx.data().toString();

        for (int j=0; j < d_ptr->m_pRangeProxy->rowCount(idx); j++) {
            QJsonObject col;

            col[ QStringLiteral("name")      ] = column;
            col[ QStringLiteral("bg")        ] = d_ptr->m_pRangeProxy->index(j, 1, idx)
                .data(Qt::BackgroundRole).toString();
            col[ QStringLiteral("fg")        ] = d_ptr->m_pRangeProxy->index(j, 2, idx)
                .data(Qt::ForegroundRole).toString();
            col[ QStringLiteral("delimiter") ] = d_ptr->m_pRangeProxy->index(j, 0, idx)
                .data((int)RangeProxy::Role::RANGE_DELIMITER_NAME).toString();

            Q_ASSERT(d_ptr->m_pRangeProxy->index(j, 0, idx).parent() == idx);
            Q_ASSERT(d_ptr->m_pRangeProxy->index(j, 1, idx).parent() == idx);
            Q_ASSERT(d_ptr->m_pRangeProxy->index(j, 2, idx).parent() == idx);

            columns.append(col);
        }
    }

    parent[QStringLiteral("columns")] = columns;
}

void ColorNode::read(const QJsonObject &parent)
{
    const auto arr = parent[QStringLiteral("columns")].toArray();
    for (int i=0; i < arr.size();i++) {
        const auto elem = arr[i].toObject();
        const auto name = elem[QStringLiteral("name")].toString();
        d_ptr->m_hlSerializedRanges[name] << new SerializedRange {
            name,
            elem[ "delimiter" ].toString(),
            elem[ "bg"        ].toString(),
            elem[ "fg"        ].toString(),
        };
    }
}

QWidget* ColorNode::widget() const
{
    if (!d_ptr->m_pWidget) {
        d_ptr->m_pWidget = new Range();
        d_ptr->m_pWidget->setRangeProxy(d_ptr->m_pRangeProxy);
    }
    return d_ptr->m_pWidget;
}

QAbstractItemModel* ColorNode::filteredModel() const
{
    return d_ptr->m_pRangeProxy->filteredModel();
}

void ColorNodePrivate::slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old)
{
    Q_UNUSED(old)
    Q_UNUSED(newModel)

    m_pRangeProxy->setSourceModel(newModel);

    slotModelReset();
}

void ColorNodePrivate::slotRowsInserted(const QModelIndex& parent, int first, int last)
{
    if (parent.isValid() || !last)
        return;

    for (int i = first; i <= last; i++) {
        const auto idx  = m_pRangeProxy->index(i, 0, parent);
        const auto name = idx.data().toString();
        if (m_hlSerializedRanges.contains(name)) {
            for (auto row : qAsConst(m_hlSerializedRanges[name])) {
                auto idx2 = m_pRangeProxy->addFilter(idx, row->delimiter);
                Q_ASSERT(idx2.isValid());
                m_pRangeProxy->setData(idx2, row->bg, Qt::BackgroundRole);
                m_pRangeProxy->setData(idx2, row->fg, Qt::ForegroundRole);
            }
        }
    }
}

void ColorNodePrivate::slotModelReset()
{
    slotRowsInserted({}, 0, m_pRangeProxy->rowCount());
}

QStringList ColorNode::searchTags() const
{
    static QStringList l {
        QStringLiteral("range"),
        QStringLiteral("alerts"),
    };

    return l;
}

#include <colornode.moc>
