#include "colornode.h"

#include "../proxies/columnproxy.h"
#include "../proxies/coloredrangeproxy.h"

#include <QtWidgets/QScrollBar>
#include <QtWidgets/QTableView>

#include <QDebug>

#include "../mainwindow.h"

class ColorNodePrivate : public QObject
{
public:
    QTableView* m_pTableView {nullptr};

    ColoredRangeProxy* m_pRangeProxy {new ColoredRangeProxy()};


public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
    void slotDataChanged();
};

ColorNode::ColorNode(QObject* parent) : ProxyNode(parent), d_ptr(new ColorNodePrivate())
{
    d_ptr->m_pTableView  = new QTableView(nullptr);

    d_ptr->m_pRangeProxy->setWidget(d_ptr->m_pTableView);

    d_ptr->m_pTableView->setModel(d_ptr->m_pRangeProxy);

    QObject::connect(this, &ProxyNode::modelChanged, d_ptr, &ColorNodePrivate::slotModelChanged);
//     QObject::connect(d_ptr->m_pCheckable, &QAbstractItemModel::dataChanged, d_ptr, &ColorNodePrivate::slotDataChanged);

    if (d_ptr->m_pTableView->horizontalHeader()) {
        d_ptr->m_pTableView->verticalHeader()->setHidden(true);
        d_ptr->m_pTableView->horizontalHeader()->setHidden(true);
        d_ptr->m_pTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        d_ptr->m_pTableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        d_ptr->m_pTableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    }
}

ColorNode::~ColorNode()
{
    
}

QString ColorNode::title() const
{
    return "Color";
}

QString ColorNode::id() const
{
    return QStringLiteral("color_node");
}

void ColorNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);

    
}

QWidget* ColorNode::widget() const
{
    return d_ptr->m_pTableView;
}

QAbstractItemModel* ColorNode::filteredModel() const
{
    return d_ptr->m_pRangeProxy->filteredModel();
}

void ColorNodePrivate::slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old)
{
    m_pRangeProxy->setSourceModel(newModel);
}
/*
void ColorNodePrivate::slotDataChanged()
{
}*/
