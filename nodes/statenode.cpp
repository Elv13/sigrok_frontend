#include "statenode.h"

#include <QtCore/QDebug>
#include <QtCore/QStringListModel>
#include <QtCore/QTimer>

#include <widgets/statemachine.h>

#if QT_VERSION < 0x050700
//Q_FOREACH is deprecated and Qt CoW containers are detached on C++11 for loops
template<typename T>
const T& qAsConst(const T& v)
{
    return const_cast<const T&>(v);
}
#endif

struct CloneHolder
{
    bool                  init     { false                    };
    QString               name     { QStringLiteral("Output") };
    QPersistentModelIndex m_rIndex {                          };
};

class InputActionModel : public QStringListModel
{
    Q_OBJECT
public:
    explicit InputActionModel(QObject* parent) : QStringListModel(parent) {}

    QStringList m_lActions;
};

class StateListModel : public QStringListModel
{
    Q_OBJECT
public:
    explicit StateListModel(QObject* parent) : QStringListModel(parent) {}
    QStringList m_lStates;
};

class StateMachineModel final : public QAbstractTableModel
{
    Q_OBJECT
    friend class StateNode;
public:
    explicit StateMachineModel(QObject* parent = nullptr) :
        QAbstractTableModel(parent) {}
    ~StateMachineModel();

    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual int rowCount(const QModelIndex& parent = {}) const override;
    virtual int columnCount(const QModelIndex& parent = {}) const override;
    virtual bool setData(const QModelIndex& idx, const QVariant& value, int role) override;
    virtual Qt::ItemFlags flags(const QModelIndex &idx) const override;
    virtual QVariant headerData(int sec, Qt::Orientation ori, int role) const;

public Q_SLOTS:
    void slotActionsRowsInserted(const QModelIndex&, int first, int last);
    void slotActionsRowsRemoved(const QModelIndex&, int first, int last);
    void slotStatesRowsInserted(const QModelIndex&, int first, int last);
    void slotStatesRowsRemoved(const QModelIndex&, int first, int last);

private:
    int                   m_MaxId {0};
    QVector<CloneHolder*> m_lRows {new CloneHolder};
    QVariant              m_Var;
    QString               m_SinkName {QStringLiteral("Input")};

    QVector<QVector<QString>> m_lCells;
    StateListModel*    m_pStateModel  {nullptr};
    InputActionModel*  m_pActionModel {nullptr};

};

class StateNodePrivate final
{
public:
    StateMachine*      m_pWidget      {nullptr};
    StateMachineModel* m_pModel       {nullptr};
};

QVariant StateMachineModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid()) return {};

    switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            return m_lCells[idx.row()][idx.column()];
    }

    return {};
}

bool StateMachineModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
    if (!idx.isValid())
        return false;

    if (role == Qt::EditRole || role == Qt::DisplayRole) {
        qDebug() << "\n\n\nDSFDSF" << value.toString();
        m_lCells[idx.row()][idx.column()] = value.toString();
        return true;
    }

    return false;
}

int StateMachineModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_lCells.size();
}

int StateMachineModel::columnCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : (m_lCells.isEmpty() ? 0 : m_lCells.first().count());
}

Qt::ItemFlags StateMachineModel::flags(const QModelIndex &idx) const
{
    Q_UNUSED(idx)
    return Qt::ItemIsEnabled    |
           Qt::ItemIsEditable   |
           Qt::ItemIsSelectable |
           Qt::ItemIsDropEnabled;
}

QVariant StateMachineModel::headerData(int sec, Qt::Orientation ori, int role) const
{
    if (m_lCells.isEmpty() || role != Qt::DisplayRole)
        return {};

    switch(ori) {
        case Qt::Horizontal:
            return m_pActionModel->index(sec, 0).data();
        case Qt::Vertical:
            return m_pStateModel->index(sec, 0).data();
    }

    return {};
}

void StateMachineModel::slotActionsRowsInserted(const QModelIndex&, int first, int last)
{
    beginInsertColumns({}, first, last);
    for(auto& v : m_lCells) {
        v.resize(v.count() + (last - first) + 1);
    }
    endInsertColumns();
}

void StateMachineModel::slotActionsRowsRemoved(const QModelIndex&, int first, int last)
{
    beginRemoveColumns({}, first, last);
    endRemoveColumns();
}

void StateMachineModel::slotStatesRowsInserted(const QModelIndex&, int first, int last)
{
    beginInsertRows({}, first, last);
    for (int i = first; i <= last; i++) {
        QVector<QString> row;
        for (int j=0; j < m_pActionModel->rowCount(); j++) {
            row << m_pActionModel->index(j, 0).data().toString();
            m_lCells.insert(i, row);
        }
        m_lCells.insert(i, row);
    }
    endInsertRows();
}

void StateMachineModel::slotStatesRowsRemoved(const QModelIndex&, int first, int last)
{
    beginRemoveRows({}, first, last);
    endRemoveRows();
}

StateNode::StateNode(AbstractSession* sess) : AbstractNode(sess), d_ptr(new StateNodePrivate())
{
    d_ptr->m_pModel       = new StateMachineModel(this);
    d_ptr->m_pModel->m_pStateModel  = new StateListModel   (this);
    d_ptr->m_pModel->m_pActionModel = new InputActionModel (this);

    connect(d_ptr->m_pModel->m_pStateModel, &QAbstractItemModel::rowsInserted,
        d_ptr->m_pModel, &StateMachineModel::slotStatesRowsInserted);
    connect(d_ptr->m_pModel->m_pStateModel, &QAbstractItemModel::rowsRemoved,
        d_ptr->m_pModel, &StateMachineModel::slotStatesRowsRemoved);

    connect(d_ptr->m_pModel->m_pActionModel, &QAbstractItemModel::rowsInserted,
        d_ptr->m_pModel, &StateMachineModel::slotActionsRowsInserted);
    connect(d_ptr->m_pModel->m_pActionModel, &QAbstractItemModel::rowsRemoved,
        d_ptr->m_pModel, &StateMachineModel::slotActionsRowsRemoved);
}

StateMachineModel::~StateMachineModel()
{
    while (!m_lRows.isEmpty())
        delete m_lRows.takeLast();
}

StateNode::~StateNode()
{
    delete d_ptr;
}

void StateNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);
}

QWidget* StateNode::widget() const
{
    if (!d_ptr->m_pWidget) {
        d_ptr->m_pWidget = new StateMachine();
        d_ptr->m_pWidget->setActionsModel (d_ptr->m_pModel->m_pActionModel);
        d_ptr->m_pWidget->setStateModel   (d_ptr->m_pModel->m_pStateModel );
        d_ptr->m_pWidget->setMatrixModel  (d_ptr->m_pModel                );
    }

    return d_ptr->m_pWidget;
}

QAbstractItemModel* StateNode::sourceModel() const
{
    return d_ptr->m_pModel;
}

bool StateNode::createSinkSocket(const QString& name)
{
    // The name isn't relevant

    d_ptr->m_pModel->m_SinkName = name;

    return true;
}

bool StateNode::createSourceSocket(const QString& name)
{
    // The name isn't relevant

    auto cl  = new CloneHolder();
    cl->name = name;
    cl->init = true;

    d_ptr->m_pModel->beginInsertRows({}, 1, 1);
    d_ptr->m_pModel->m_lRows.insert(0, cl);
    d_ptr->m_pModel->endInsertRows();

    return true;
}

#include <statenode.moc>
