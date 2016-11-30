#pragma once

#include <QtCore/QStringListModel>

/*
 * The D_PTR are shared so classes inheriting the RangeProxy can store
 * arbitrary data in the nodes. Otherwise, they all need to create an extra
 * structure for each node and keep track of it.
 * 
 * The m_hExtraValues is a pointer to slim down the Node size (for the cache
 * performance).
 */

struct Node
{
    enum class Mode {
        ROOT,
        CHILD
    };

    int                   m_Index       { 0                           };
    Mode                  m_Mode        { Mode::ROOT                  };
    Node*                 m_pParent     { nullptr                     };
    RangeProxy::Delimiter m_Delim       { RangeProxy::Delimiter::ANY  };
    QVariant              m_RangeValue  { QString()                   };
    QHash<int, QVariant>* m_hExtraValues{ new QHash<int, QVariant>()  };
    QVector<Node*>        m_lChildren   {                             };
};

class RangeProxyPrivate : public QObject
{
public:
    int m_ExtraColumnCount {0};
    QVector<Node*> m_lRows;
    bool m_MatchAllColumns{false};

    static QStringListModel* s_pDelimiterModel;
    static QStringList DELIMITERNAMES;

    RangeProxy* q_ptr;

    //Helpers
    QModelIndex matchSourceIndex(const QModelIndex& srcIdx) const;

public Q_SLOTS:
    void slotLayoutChanged();
    void slotAutoAddRows(const QModelIndex& parent);
    void slotRowsAboutToBeInserted(const QModelIndex &parent, int first, int last);
    void slotRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
};
