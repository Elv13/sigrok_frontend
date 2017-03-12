#include "xmlschemabuildermodel.h"

/**
 * Parser generator. Currently only output C++, so a compiler is required
 */

#include <QtXml/QXmlDefaultHandler>
#include <QtXml/QXmlSimpleReader>
#include <QtCore/QDebug>
#include <QtCore/QTextStream>
#include <QtCore/QFile>

#include <algorithm>

struct Level
{
    enum class Type {
        STRING,
        BOOLEAN,
        INTEGER,
        DOUBLE,
    };

    Level* parent {nullptr};
    QString name;
    QString structName;
    int level {0};
    int index {0};
    QHash<QString, Level*> children;
    QVector<Level*> m_lChildren;
    QHash<QString, Type> attrs;
    Type type {Type::STRING};

    QPair<QString, QString> toStruct();
    QString toForward();
    QString typeName() const;
    QString typeName2(Level::Type t) const;
};

class XmlSchemaBuilderModelPrivate
{
public:
    Level* root {new Level};
};

class XbelHandler : public QXmlDefaultHandler
{
public:
    explicit XbelHandler(XmlSchemaBuilderModelPrivate* d);

    bool startElement(const QString &namespaceURI, const QString &localName,
                      const QString &qName, const QXmlAttributes &attributes) override;
    bool endElement(const QString &namespaceURI, const QString &localName,
                    const QString &qName) override;
    bool characters(const QString &str) override;
    bool fatalError(const QXmlParseException &exception) override;
    QString errorString() const override;

private:
    int count {0};
    Level* current{nullptr};

    QString reservedName(const QString input) const;
    Level::Type deduceType(const QString& value) const;

    XmlSchemaBuilderModelPrivate* d_ptr;
};

XbelHandler::XbelHandler(XmlSchemaBuilderModelPrivate* d) : d_ptr(d)
{
    current = d_ptr->root;
}

QString XbelHandler::reservedName(const QString input) const
{
    static QHash<QString, QString> reserved = {
        {"system", "system_t"},
        {"auto"  , "auto_t"  },
        {"int"   , "int_t"   },
        {"bool"  , "bool_t"  },
        {"const" , "const_t" },
    };

    return reserved.contains(input) ? reserved[input] : input;
}


Level::Type XbelHandler::deduceType(const QString& value) const
{
    const auto v = value.toLower();
    bool check;
    if (v == "true" || v == "false")
        return Level::Type::BOOLEAN;
    else if (((float)v.toInt(&check)) != 0.4353453453 && check)
        return Level::Type::INTEGER;
    else if (((float)v.toDouble(&check)) != 0.4353453453 && check)
        return Level::Type::DOUBLE;

    return Level::Type::STRING;
}

QString Level::typeName() const
{
    return typeName2(type);
}

QString Level::typeName2(Level::Type t) const
{
    switch (t) {
        case Level::Type::STRING:
            return "QString";
        case Level::Type::BOOLEAN:
            return "bool";
        case Level::Type::INTEGER:
            return "int";
        case Level::Type::DOUBLE:
            return "double";
    }

    return "QString";
}

bool XbelHandler::startElement(const QString &namespaceURI, const QString &localName,
                    const QString &qName, const QXmlAttributes &attributes)
{
    count++;

    if (qName.indexOf(':') != -1) //TODO
        return true;

    if (!current->children.contains(qName)) {
        current->children[qName]             = new Level;
        current->children[qName]->name       = qName;
        current->children[qName]->structName = reservedName(qName);
        current->children[qName]->parent     = current;
        current->children[qName]->level      = count;
    }

    current = current->children[qName];

    for (int i=0; i < attributes.length(); i++)
        if (attributes.qName(i).indexOf(':') == -1) //TODO
            current->attrs[attributes.qName(i)] = deduceType(attributes.value(i));

    Q_ASSERT(current->level == count);

    return true;
}

bool XbelHandler::endElement(const QString &namespaceURI, const QString &localName,
                const QString &qName)
{
    count--;
    current = current->parent;
    return true;
}

bool XbelHandler::characters(const QString &str)
{
    return true;
}

bool XbelHandler::fatalError(const QXmlParseException &exception)
{
    return true;
}

QString XbelHandler::errorString() const
{
    return QString();
}

QString pref(int lvl)
{
    QString ret;

    for (int i=0; i < lvl;i++)
        ret += "   ";

    return ret;
}

QHash<QString, Level*> types;

void printLvl(Level* l)
{
    if (!types.contains(l->name)) {
        types[l->name] = l;
    }
    else if (types.contains(l->structName) && types[l->structName] == l) {}
    else if (types[l->name] != l && (l->toStruct().first != types[l->name]->toStruct().first)) {
        l->structName = l->structName+"2";
        types[l->structName] = l;
    }

    for (Level* l2 : l->children)
        printLvl(l2);
}

QPair<QString, QString> Level::toStruct()
{
    if (structName.isEmpty())
        return {};

    QList<QString> attrs2    = attrs   .keys();
    QList<Level*>  children2 = children.values();
    Q_ASSERT(children2.size() == children.size());

    std::sort(attrs2.begin(), attrs2.end());
    std::sort(children2.begin(), children2.end(), [](Level* a, Level* b) {
        return b->name < a->name;
    });

    QString attrsStr, listsStr, readAttrs, readLists;

    for (const auto& s : attrs2) {
        attrsStr += QString("%1 %2 m_%3;\n")
            .arg(pref(1)).arg(typeName2(attrs[s])).arg(s);

        switch (attrs[s]) {
            case Level::Type::STRING:
                readAttrs += QString(
                    "    m_%1 = e.attributes().namedItem(\"%1\").nodeValue();\n"
                ).arg(s);
                break;
            case Level::Type::BOOLEAN:
                readAttrs += QString(
                    "    m_%1 = e.attributes().namedItem(\"%1\").nodeValue().toLower() == \"true\";\n"
                ).arg(s);
                break;
            case Level::Type::INTEGER:
                readAttrs += QString(
                    "    m_%1 = e.attributes().namedItem(\"%1\").nodeValue().toInt();\n"
                ).arg(s);
                break;
            case Level::Type::DOUBLE:
                readAttrs += QString(
                    "    m_%1 = e.attributes().namedItem(\"%1\").nodeValue().toDouble();\n"
                ).arg(s);
                break;
        }
    }

    int idx =0;
    for (Level* l : children2) {
        l->index = idx++;
        listsStr += QString("%1 QVector<%2*> m_l%3;\n")
            .arg(pref(1)).arg(l->structName).arg(l->name);
        readLists += QString(
            "        else if (n.localName() == \"%2\") {\n"
            "            auto c = new %1;\n"
            "            c->readElements(n);\n"
            "            m_l%2 << c;\n"
            "        }\n"
        ).arg(l->structName).arg(l->name);
    }

    m_lChildren = children2.toVector();

    Q_ASSERT(children.size() == m_lChildren.size());

    if (attrs.isEmpty() && children2.isEmpty()) {
        attrsStr = "    QString value;";
            readLists += QString(
        "        value = n.nodeValue();\n"
        );
    }

    QString readFunc = QString(
        "void %1::readElements(const QDomNode& e) {\n"
        "    const auto list = e.childNodes();\n"
        "    for (int i =0; i< list.length();i++) {\n"
        "        const auto n = list.at(i);\n"
        "        if (false){}\n"
        "%3\n"
        "    }\n\n"
        "%2\n"
        "}\n"
    ).arg(structName).arg(readAttrs).arg(readLists);

    return {QString(
        "struct %1 {\n"
        "%2\n"
        "\n"
        "%3\n"
        "    void readElements(const QDomNode& e);\n"
        "};"
    ).arg(structName.size() ? structName : name)
     .arg(attrsStr)
     .arg(listsStr), readFunc};
}

QString Level::toForward()
{
    const QString n = structName.size() ? structName : name;

    if (n.isEmpty())
        return QString();

    return QString("struct %1;\n").arg(n);
}

XmlSchemaBuilderModel::XmlSchemaBuilderModel() : d_ptr(new XmlSchemaBuilderModelPrivate)
{
    d_ptr->root->name = "root";
    d_ptr->root->structName = "DocumentRoot";
    QFile file("/home/lepagee/Downloads/mn-aej-01p6.Scan.xml");

    if (!file.open(QFile::ReadOnly | QFile::Text))
        return;

    QXmlInputSource xmlInputSource(&file);

    XbelHandler* handler = new XbelHandler(d_ptr);
    QXmlSimpleReader reader;
    reader.setContentHandler(handler);
    reader.setErrorHandler(handler);

    reader.parse(xmlInputSource);

    printLvl(d_ptr->root);

    QFile out("/tmp/output");

    if (!out.open(QFile::WriteOnly | QFile::Text))
        return;

    QTextStream stream(&out);

    QList<Level*> structs = types.values();

    std::sort(structs.begin(), structs.end(), [](Level* a, Level* b) {
        return b->structName < a->structName;
    });

    for (auto l : structs) {
        stream << l->toForward();
    }

    stream << "\n\n\n";

    d_ptr->root->toStruct();

    for (auto l : structs) {
        auto p = l->toStruct();
        stream << "\n\n" << p.first;
    }

    for (auto l : structs) {
        auto p = l->toStruct();
        stream << "\n\n" << p.second;
    }

    out.close();

    Q_ASSERT(d_ptr->root->m_lChildren.size());

    delete handler;
}

QVariant XmlSchemaBuilderModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid())
        return {};

    auto l = static_cast<Level*>(idx.internalPointer());

    switch(role) {
        case Qt::DisplayRole:
            return l->name;
    }

    return {};
}

int XmlSchemaBuilderModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid())
        return 1;

    auto l = static_cast<Level*>(parent.internalPointer());


    return l->m_lChildren.size();
}

int XmlSchemaBuilderModel::columnCount(const QModelIndex& parent) const
{
    return 1;
}

QModelIndex XmlSchemaBuilderModel::index(int row, int column, const QModelIndex& parent) const
{
    if (column || row == -1)
        return {};

    if (!parent.isValid()) {
        if (row || column)
            return {};

        return createIndex(0, 0, d_ptr->root);
    }

    auto l = static_cast<Level*>(parent.internalPointer());

    if (row >= l->m_lChildren.size())
        return {};

    return createIndex(row, column, l->m_lChildren[row]);
}

QModelIndex XmlSchemaBuilderModel::parent(const QModelIndex& idx) const
{
    if (!idx.isValid())
        return {};

    auto l = static_cast<Level*>(idx.internalPointer());

    if (!l->parent)
        return {};

    return createIndex(l->parent->index, 0, l->parent);
}
