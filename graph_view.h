#ifndef GRAPH_VIEW_H
#define GRAPH_VIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMouseEvent>

//Header for MyVex class
#include <QGraphicsEllipseItem>

//Headers for lines
#include <QPainter>
//#include <QPainterPath>
#include <QGraphicsPathItem>

#include <QTimeLine>
#include <QEasingCurve>

enum ItemState{
    PREPARING       = 0b10000000,
    UNDEFINED       = 0b00000000,
    ON_HOVER        = 0b00000001,
    ON_LEFT_CLICK   = 0b00000010,
    ON_RIGHT_CLICK  = 0b00000100,
    ON_SELECTED     = 0b00001000,
    ON_LINING       = 0b00010000
};

enum MouseState{
    REGULAR         = 0b00000000,
    ADDING_VEX      = 0b00000001,
    ADDING_LINE     = 0b00000010,
    DRAGGING        = 0b00000100
};

class MyGraphicsView;
class MyGraphicsVexItem;
class MyGraphicsLineItem;


// Summary:
//  MyGraphicsView class is customized for visualizing graph
//
// Functions:
//  -Left click to add vex
//  -Left click on vex to add arc
//  -Right click on vex or arc to view information
//  -Drag vex to adjust place (adjust connected arcs either)

class MyGraphicsView : public QGraphicsView{
    Q_OBJECT

private:
    QGraphicsScene* myGraphicsScene;
    MyGraphicsVexItem *selVex;

    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

public:
    MyGraphicsView(QWidget *parent = nullptr);

signals:
    void mouseMoved(QPointF position);
    void mouseLeftClicked(QPointF position);
    void mouseRightClicked(QPointF position);
    void mouseReleased();

public slots:
    void vexSelected(MyGraphicsVexItem* sel);

};


// Summary:
//  MyGraphicsVexItem realize the interactions with vex

class MyGraphicsVexItem : public QObject, public QGraphicsEllipseItem{
    Q_OBJECT

private:
    QPointF center;
    double radius;
    int state = UNDEFINED;
    QTimeLine* curAnimation = nullptr;

    void hoverInAnimation();
    void hoverOutAnimation();
    void clickAnimation();
    void releaseAnimation();

    void startAnimation();
    void stopAnimation();

public:
    MyGraphicsVexItem(QPointF _center, qreal _r, QGraphicsItem *parent = nullptr);

    void estConnection(MyGraphicsView* view);
    void showAnimation();
    void visit(bool visited = true);

signals:
    void mouseOn(MyGraphicsVexItem* item);
    void selected(MyGraphicsVexItem* sel);

public slots:
    void onMouseMove(QPointF position);
    void onLeftClick(QPointF position);
    void onRightClick(QPointF position);
    void onMouseRelease();

};

class MyGraphicsLineItem : public QGraphicsLineItem, public QObject{

private:

public:

signals:

private slots:
};

#endif // GRAPH_VIEW_H
