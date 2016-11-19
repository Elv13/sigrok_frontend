#include "columnnode.h"

#include "proxies/columnproxy.h"

#include <QtWidgets/QScrollBar>

#include "widgets/column.h"

#include <QtCore/QItemSelectionModel>

#include <KRearrangeColumnsProxyModel>
#include <KCheckableProxyModel>

#include <QDebug>

class ColumnNodePrivate : public QObject
{
public:
    Column* m_pColumnW {new Column()};

    KRearrangeColumnsProxyModel* m_pFilteredModel {
        new KRearrangeColumnsProxyModel(this)
    };

    KCheckableProxyModel* m_pCheckable {
        new KCheckableProxyModel(this)
    };

    ColumnProxy* m_pColumnProxy {new ColumnProxy()};

public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
    void slotDataChanged();
};

ColumnNode::ColumnNode(QObject* parent) : ProxyNode(parent), d_ptr(new ColumnNodePrivate())
{

    d_ptr->m_pCheckable->setSourceModel(d_ptr->m_pColumnProxy);

    d_ptr->m_pColumnW->setModel(d_ptr->m_pCheckable);

    d_ptr->m_pFilteredModel->setObjectName("FOOPROXY");

    QObject::connect(this, &ProxyNode::modelChanged, d_ptr, &ColumnNodePrivate::slotModelChanged);
    QObject::connect(d_ptr->m_pCheckable, &QAbstractItemModel::dataChanged, d_ptr, &ColumnNodePrivate::slotDataChanged);
}

ColumnNode::~ColumnNode()
{
    
}

QString ColumnNode::title() const
{
    return "Column";
}

void ColumnNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);

    
}

QString ColumnNode::id() const
{
    return QStringLiteral("column_node");
}

QWidget* ColumnNode::widget() const
{
    return d_ptr->m_pColumnW;
}

QAbstractItemModel* ColumnNode::filteredModel() const
{
    return d_ptr->m_pFilteredModel;
}

void ColumnNodePrivate::slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old)
{
    m_pColumnProxy->setSourceModel(newModel);

    m_pFilteredModel->setSourceModel(newModel);

    QItemSelectionModel *checkModel = new QItemSelectionModel(newModel, this); //FIXME leak
    m_pCheckable->setSelectionModel(checkModel);

}

void ColumnNodePrivate::slotDataChanged()
{
    QVector<int> ret;

    auto m = m_pCheckable;

    for (auto i = m->index(0,0); i.isValid(); i = m->index(i.row()+1,0)) {
        if (i.data(Qt::CheckStateRole) == Qt::Checked) {
            ret << i.row();
        }
    }

    m_pFilteredModel->setSourceColumns(ret);
}

bool ColumnNode::test2() const {return true;}
    void ColumnNode::setTest2(bool) {}
