/* See LICENSE file for copyright and license details. */

#ifndef __GRAPHICSNODEVIEW_HPP__59C6610F_3283_42A1_9102_38A7065DB718
#define __GRAPHICSNODEVIEW_HPP__59C6610F_3283_42A1_9102_38A7065DB718

#include <QGraphicsView>
#include <QPoint>

class QMimeData;
class QTimeLine;
class QResizeEvent;
class QItemSelectionModel;
class GraphicsNode;
class GraphicsDirectedEdge;
class GraphicsNodeSocket;

//HACK this is totally *not* fine to put the architecture on its head
class QNodeEditorEdgeModel; 
class QNodeEditorSocketModel;

struct EdgeDragEvent
{
	// encode what the user is actually doing.
	enum drag_mode {
		// connect a new edge to a source or sink
		to_source,
		to_sink,
	};

	GraphicsDirectedEdge *e;
	QMimeData* mimeData;
	drag_mode mode;
};


struct NodeResizeEvent
{
	GraphicsNode *node;
	qreal orig_width, orig_height;
	QPoint pos;
};


class GraphicsNodeView : public QGraphicsView
{
	Q_OBJECT
public:
	explicit GraphicsNodeView(QWidget *parent = nullptr);
	GraphicsNodeView(QGraphicsScene *scene, QWidget *parent = nullptr);

	void setZoomLevel(qreal);
	qreal zoomLevel() const;
protected:
	virtual void wheelEvent(QWheelEvent *event) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void mouseReleaseEvent(QMouseEvent *event) override;
	virtual void resizeEvent(QResizeEvent *event) override;
	virtual void dragEnterEvent(QDragEnterEvent *event) override;
	virtual void dragMoveEvent(QDragMoveEvent *event) override;
	virtual void dropEvent(QDropEvent *event) override;

    QNodeEditorSocketModel* m_pModel {nullptr}; //HACK evil workaround until the QAbstractItemView is added
	QItemSelectionModel* m_pSelectionModel {nullptr}; //idem
	QList<QModelIndex> selectedNodeIndexes() const;

    GraphicsNode* nodeAtAbs(const QPoint &point) const;

private:
	void middleMouseButtonPress(QMouseEvent *event);
	void leftMouseButtonPress(QMouseEvent *event);

	void middleMouseButtonRelease(QMouseEvent *event);
	void leftMouseButtonRelease(QMouseEvent *event);

	bool can_accept_edge(GraphicsNodeSocket *sock);
	GraphicsNodeSocket* socket_at(QPoint pos);

private:
	// No need for a d_ptr here. Eventually this class will become private
	EdgeDragEvent *_drag_event = nullptr;
	NodeResizeEvent *_resize_event = nullptr;
	QTimeLine *_time_line = nullptr;
	qreal _scheduled_zoom_steps = 1;
	qreal _zoom_level = 1;

private Q_SLOTS:
	void slotZoomStep(qreal target);
	void slotFinishZoom();

Q_SIGNALS:
	void zoomLevelChanged(qreal level);
};

#endif /* __GRAPHICSNODEVIEW_HPP__59C6610F_3283_42A1_9102_38A7065DB718 */
//kate: space-indent off; indent-width tabs; replace-tabs off;

