#include "mementonode.h"

#include "proxies/mementoproxy.h"
#include "proxies/rowsdeletionproxy.h"
#include "widgets/memento.h"

#include <QtCore/QDebug>
#include <QtGui/QIcon>
#include <QtCore/QAbstractListModel>
#include <QtCore/QItemSelectionModel>
#include <QtCore/QDateTime>
#include <QtCore/QJsonArray>

#if QT_VERSION < 0x050700
//Q_FOREACH is deprecated and Qt CoW containers are detached on C++11 for loops
template<typename T>
const T& qAsConst(const T& v)
{
    return const_cast<const T&>(v);
}
#endif

class MementosList : public QAbstractListModel
{
    Q_OBJECT
    friend class MementoNode;
public:
    explicit MementosList(QObject* parent);

    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual int rowCount(const QModelIndex& parent = {}) const override;
    virtual bool removeRows(int row, int count, const QModelIndex &parent = {}) override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    virtual Qt::ItemFlags flags(const QModelIndex &idx) const override;


    QVector<MementoProxy*> m_lMementos;
    QVector<QVariant> m_lTitle;

    MementoProxy* takeMemento(QAbstractItemModel* model);

    MementoNode* q_ptr;
};

class MementoNodePrivate : public QObject
{
    Q_OBJECT
public:
    MementosList *m_pMementoList {new MementosList(this)};
    Memento* m_pWidgets {new Memento()};
    RowsDeletionProxy m_RemoveRowProxy {this};
    QItemSelectionModel* m_pSelection {new QItemSelectionModel(&m_RemoveRowProxy)};

    MementoNode* q_ptr;

public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
    void slotSelectionChanged();
};

MementosList::MementosList(QObject* parent) : QAbstractListModel(parent)
{
    
}

QVariant MementosList::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid()) return {};

    switch(role) {
        case Qt::DisplayRole:
            return m_lTitle[idx.row()];
        case Qt::UserRole:
        case Qt::ToolTipRole:
            return m_lMementos[idx.row()]->rowCount();
    };

    return {};
}

bool MementosList::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if ((!index.isValid()) || !index.model())
        return false;

    if (value.toString().isEmpty())
        return false;

    switch(role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            m_lTitle[index.row()] = value;
            return true;
    }

    return false;
}

Qt::ItemFlags MementosList::flags(const QModelIndex &idx) const
{
    return (!idx.isValid()) ? Qt::NoItemFlags :
        (Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
}

int MementosList::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_lMementos.size();
}

bool MementosList::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || count < 1 || row+count > rowCount())
        return false;

    beginRemoveRows(parent, row, row+count-1);
    for (int i = row; i < row+count; i++) {
        auto s = m_lMementos[i];
        m_lMementos.remove(i);
        delete s;
    }
    endRemoveRows();

    return true;
}

MementoNode::MementoNode(AbstractSession* sess) : ProxyNode(sess), d_ptr(new MementoNodePrivate())
{
    d_ptr->q_ptr = this;
    d_ptr->m_pMementoList->q_ptr = this;

    d_ptr->m_RemoveRowProxy.setSourceModel(d_ptr->m_pMementoList);
    d_ptr->m_RemoveRowProxy.setSelectionModel(d_ptr->m_pSelection);
    d_ptr->m_RemoveRowProxy.setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));

    d_ptr->m_pWidgets->setModel(&d_ptr->m_RemoveRowProxy, d_ptr->m_pSelection);

    QObject::connect(this, &ProxyNode::modelChanged, d_ptr, &MementoNodePrivate::slotModelChanged);
    QObject::connect(d_ptr->m_pWidgets, &Memento::takeMemento, this, &MementoNode::takeMemento);

    connect(d_ptr->m_pSelection, &QItemSelectionModel::currentChanged, d_ptr, &MementoNodePrivate::slotSelectionChanged);
}

MementoNode::~MementoNode()
{
    delete d_ptr;
}

QString MementoNode::title() const
{
    return QStringLiteral("Memento");
}

QString MementoNode::id() const
{
    return QStringLiteral("memento_node");
}

void MementoNode::read(const QJsonObject &parent)
{
    const auto mementos = parent[QStringLiteral("mementos")].toArray();

    d_ptr->m_pMementoList->beginInsertRows({}, 0, mementos.size()-1);
    for (int i = 0; i < mementos.size(); ++i) {
        const QJsonObject obj = mementos[i].toObject();

        const QJsonObject memento = obj[QStringLiteral("content")].toObject();

        auto proxy = new MementoProxy(memento, this);

        d_ptr->m_pMementoList->m_lMementos << proxy;
        d_ptr->m_pMementoList->m_lTitle << obj[QStringLiteral("title")].toString();

        if (obj[QStringLiteral("selected")].toBool()) {
            d_ptr->m_pSelection->setCurrentIndex(
                d_ptr->m_pMementoList->index(i,0),
                QItemSelectionModel::ClearAndSelect
            );
        }
    }
    d_ptr->m_pMementoList->endInsertRows();

}

void MementoNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);

    QJsonArray mementos;

    int i = 0;

    for (auto m : qAsConst(d_ptr->m_pMementoList->m_lMementos)) {
        QJsonObject obj;
        obj[ QStringLiteral("title")    ] = d_ptr->m_pMementoList->m_lTitle[i++].toString();
        obj[ QStringLiteral("content")  ] = m->toJson();
        obj[ QStringLiteral("selected") ] = selectedMemento() == m;
        mementos.append(obj);
    }

    parent[QStringLiteral("mementos")] = mementos;
}

QWidget* MementoNode::widget() const
{
    return d_ptr->m_pWidgets;
}

MementoProxy* MementosList::takeMemento(QAbstractItemModel* model)
{
    auto p = new MementoProxy(this);
    p->setSourceModel(model);
    p->sync();

    beginInsertRows({}, m_lMementos.size(), m_lMementos.size());
    m_lMementos << p;
    m_lTitle << QDateTime::currentDateTime();
    endInsertRows();

    Q_EMIT q_ptr->mementoAdded(p);

    return p;
}


QAbstractItemModel* MementoNode::lastestMemento() const
{
    const int count = d_ptr->m_pMementoList->m_lMementos.size();
    return count ? d_ptr->m_pMementoList->m_lMementos[count-1] : nullptr;
}

void MementoNode::takeMemento(bool)
{
    d_ptr->m_pMementoList->takeMemento(model());
}

void MementoNodePrivate::slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old)
{
    Q_UNUSED(newModel)
    Q_UNUSED(old)
}

QAbstractItemModel* MementoNode::selectedMemento() const
{
    const auto idx = d_ptr->m_pSelection->currentIndex();

    if (idx.isValid())
        return d_ptr->m_pMementoList->m_lMementos[idx.row()];

    return d_ptr->m_pMementoList->m_lMementos.size() ?
        d_ptr->m_pMementoList->m_lMementos[0] : nullptr;
}

void MementoNodePrivate::slotSelectionChanged()
{

    Q_EMIT q_ptr->selectedMementoChanged(q_ptr->selectedMemento());
}

#include "mementonode.moc"
