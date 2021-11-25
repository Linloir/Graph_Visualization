#ifndef GRAPH_VIEW_H
#define GRAPH_VIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMouseEvent>

//Header for MyVex class
#include <QGraphicsEllipseItem>
#include <QVector>

//Headers for lines
#include <QPainter>
#include <QColor>
//#include <QPainterPath>
#include <QGraphicsPathItem>

#include <QTimeLine>
#include <QEasingCurve>

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
    enum mouseStates{
        NORMAL      = 0b00000000,
        ON_HOVER    = 0b00010000,
        ON_SELECTED = 0b00100000,
        ON_MOVING   = 0b01000000
    };

    enum itemStates{
        NON         = 0b00000000,
        SEL         = 0b00010000,
        ADD         = 0b00100000,
        VEX         = 0b00000001,
        LINE        = 0b00000010
    };

    QGraphicsScene* myGraphicsScene;

    int mouseState = NORMAL;
    int itemState = NON;

    QGraphicsItem *selItem = nullptr;

    MyGraphicsVexItem *strtVex = nullptr;
    QGraphicsItem *sketchItem = nullptr;
    qreal zValue = -1;

    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

    void changeCursor();

    MyGraphicsVexItem* addVex(QPointF center, qreal radius = 10);
    void clearSketch();
    void sketchLine(QPointF start, QPointF end);
    void addLine(MyGraphicsVexItem* start, MyGraphicsVexItem* end);

public:
    MyGraphicsView(QWidget *parent = nullptr);

signals:
    void mouseMoved(QPointF position);
    void mouseLeftClicked(QPointF position);
    void mouseRightClicked(QPointF position);
    void mouseReleased();

public slots:
    void setHover(bool in = true);
    void setSel(QGraphicsItem *sel);
    void startLine(MyGraphicsVexItem* startVex);
    void setMenu(QGraphicsItem *target, bool display = true){}

};


// Summary:
//  MyGraphicsVexItem realize the interactions with vex

class MyGraphicsVexItem : public QObject, public QGraphicsEllipseItem{
    Q_OBJECT

private:
    enum { Type = UserType + 1 };
    enum {
        PREPARING       = 0b10000000,
        UNDEFINED       = 0b00000000,
        ON_HOVER        = 0b00000001,
        ON_LEFT_CLICK   = 0b00000010,
        ON_RIGHT_CLICK  = 0b00000100,
        ON_SELECTED     = 0b00001000,
        ON_LINING       = 0b00010000,
        ON_MENU         = 0b00100000
    };

    QBrush regBrush = QBrush(QColor(58, 143, 192));
    QBrush selBrush = QBrush(QColor(208, 90, 110));

    QPointF center;
    qreal radius;
    QString text;
    int state = UNDEFINED;
    QTimeLine* curAnimation = nullptr;

    QVector<MyGraphicsLineItem*> linesStartWith;
    QVector<MyGraphicsLineItem*> linesEndWith;

    void displayText();

    void hoverInEffect();
    void hoverOutEffect();
    void onClickEffect();
    void onReleaseEffect();
    void startAnimation();
    void stopAnimation();

    void move(QPointF position);

public:
    MyGraphicsVexItem(QPointF _center, qreal _r, QGraphicsItem *parent = nullptr);

    /* initializing */
    void estConnection(MyGraphicsView* view);
    void showAnimation();

    void select();
    //void visit(bool visited = true);
    //void setText(const QString & text);
    void addStartLine(MyGraphicsLineItem *line){linesStartWith.push_back(line);}
    void addEndLine(MyGraphicsLineItem *line){linesEndWith.push_back(line);}

    int type() const override {return Type;}
    qreal getRadius() {return radius;}

signals:
    void setHover(bool in = true);
    void selected(QGraphicsItem *sel);
    void lineFrom(MyGraphicsVexItem *start);
    void menuStateChanged(QGraphicsItem *item, bool display = true);

public slots:
    void onMouseMove(QPointF position);
    void onLeftClick(QPointF position);
    void onRightClick(QPointF position);
    void onMouseRelease();

};

class MyGraphicsLineItem : public QObject, public QGraphicsLineItem{
    Q_OBJECT

private:
    enum { Type = UserType + 2 };
    enum {
        UNDEFINED       = 0b00000000,
        ON_HOVER        = 0b00000001,
        ON_LEFT_CLICK   = 0b00000010,
        ON_RIGHT_CLICK  = 0b00000100,
        ON_SELECTED     = 0b00001000,
        ON_MENU         = 0b00100000
    };

    bool hasDirection;
    MyGraphicsVexItem *startVex;
    MyGraphicsVexItem *endVex;
    QGraphicsPathItem *arrow = nullptr;
    QString text;

    int state = UNDEFINED;

    /* about animation */
    QTimeLine *curAnimation;

    /* detail of the line */
    qreal lineWidth = 3;
    qreal arrowLength = 10;
    Qt::PenStyle lineStyle = Qt::SolidLine;
    Qt::PenCapStyle capStyle = Qt::RoundCap;
    QColor defaultColor = QColor(125, 185, 222);
    QColor hoverColor = QColor(0, 98, 132);
    QColor selColor = QColor(208, 90, 110);
    QPen defaultPen;
    QBrush defaultBrush;

    void drawArrow();
    void delArrow();

public:
    MyGraphicsLineItem(MyGraphicsVexItem *start, MyGraphicsVexItem *end, bool hasDir = false, QGraphicsItem *parent = nullptr);

    /* initialize functions */
    void estConnection(MyGraphicsView *view);
    void refrshLine();

    /* adjust functions */
    void reverseDirection();
    void moveStart(MyGraphicsVexItem *start);
    void moveEnd(MyGraphicsVexItem *end);
    //void setText(const QString & text);
    void setDirection(bool hasDir = true);

    /* effects */
    void hoverInEffect();
    void hoverOutEffect();
    void onClickEffect();
    void onReleaseEffect();
    void onSelectEffect(){}
    void deSelectEffect(){}
    void startAnimation(){}
    void stopAnimation(){}

    int type() const override {return Type;}

signals:
    void setHover(bool in = true);
    void selected(QGraphicsItem *sel);
    void menuStateChanged(QGraphicsItem *item, bool display = true);

private slots:
    void onMouseMove(QPointF position);
    void onLeftClick(QPointF position);
    void onRightClick(QPointF position);
    void onMouseRelease();

};

#endif // GRAPH_VIEW_H
