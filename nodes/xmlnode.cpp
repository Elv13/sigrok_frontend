#include "xmlnode.h"

#include <QtCore/QDebug>

#include "widgets/xml.h"

#include <QtXml/QDomElement>
#include <QtXml/QDomDocument>

#include <QtCore/QFile>
#include <QtCore/QDebug>
#include <QtCore/QVector>
#include <QtCore/QString>

#include "/tmp/output"

class TabModel : public QAbstractTableModel
{
public:

    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual int rowCount(const QModelIndex& parent = {}) const override;
    virtual int columnCount(const QModelIndex& parent = {}) const override;

    QVector<Meas*> m_lRows;
};

QVariant TabModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid())
        return {};

    switch(role) {
        case Qt::DisplayRole:
            switch(idx.column()) {
                case 0:
                    return idx.row();
                case 1:
                    return m_lRows[idx.row()]->m_Val;
                case 2:
                    return m_lRows[idx.row()]->m_lID;
                case 3:
                    return m_lRows[idx.row()]->m_mID;
            }
    }

    return {};
}

int TabModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_lRows.size();
}

int TabModel::columnCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : 4;
}

class XmlNodePrivate final
{
public:
    TabModel* m_pModel {new TabModel};
    Xml* m_pWidget{nullptr};
    XmlNode* q_ptr;
};

XmlNode::XmlNode(AbstractSession* sess) : AbstractNode(sess), d_ptr(new XmlNodePrivate())
{
    QFile device("/home/lepagee/Downloads/mn-aej-01p6.Scan.xml");

    QDomDocument domDocument;

    QString errorStr;
    int errorLine;
    int errorColumn;

    if (!domDocument.setContent(&device, true, &errorStr, &errorLine,
                                &errorColumn)) {
        return;
    }

    QDomElement root = domDocument.documentElement();
    SunriseDocument* d = new SunriseDocument;
    d->readElements(root);

    auto l = d->m_lTestData.first()->m_lsubTest.first()->m_lSCAN.first()->m_lData.first()->m_lCH;

    for (auto l2 : l) {
        qDebug() << "asdasdl";
        d_ptr->m_pModel->m_lRows << l2->m_lmList.first()->m_lMeas.first();
    }

}

XmlNode::~XmlNode()
{
    delete d_ptr;
}

void XmlNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);
}

QWidget* XmlNode::widget() const
{
    if (!d_ptr->m_pWidget)
        d_ptr->m_pWidget = new Xml();
    return d_ptr->m_pWidget;
}

QAbstractItemModel* XmlNode::model() const
{
    return d_ptr->m_pModel;
}
