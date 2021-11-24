#include "graph_view.h"
#include <QDebug>

//MyGraphicsView
MyGraphicsView::MyGraphicsView(QWidget *parent) :
    QGraphicsView(parent){
    this->setMouseTracking(true);
    this->setBackgroundBrush(Qt::transparent);
    myGraphicsScene = new QGraphicsScene();
    this->setScene(myGraphicsScene);
    this->setRenderHint(QPainter::Antialiasing);
}

void MyGraphicsView::mousePressEvent(QMouseEvent *event){
    if(event->button() == Qt::LeftButton)
        emit mouseLeftClicked(mapToScene(event->pos()));
    //else if(event->button() == Qt::RightButton)
    //    emit mouseRightClicked(mapToScene(event->pos()));
    else{
        MyGraphicsVexItem *newDot = new MyGraphicsVexItem(mapToScene(event->pos()), 10);
        this->scene()->addItem(newDot);
        newDot->estConnection(this);
        newDot->showAnimation();
    }
}

void MyGraphicsView::mouseReleaseEvent(QMouseEvent *event){
    emit mouseReleased();
}

void MyGraphicsView::mouseMoveEvent(QMouseEvent *event){
    emit mouseMoved(mapToScene(event->pos()));
}

void MyGraphicsView::vexSelected(MyGraphicsVexItem *sel){
    selVex = sel;
}

//MyGraphicsVexItem
MyGraphicsVexItem::MyGraphicsVexItem(QPointF _center, qreal _r, QGraphicsItem *parent) :
    QGraphicsEllipseItem(_center.x() - 0.5, _center.y() - 0.5, 1, 1, parent),
    center(_center),
    radius(_r){
    this->setPen(Qt::NoPen);
    this->setBrush(Qt::blue);
}

void MyGraphicsVexItem::showAnimation(){
    //stopAnimation();
    state = PREPARING;
    QTimeLine *timeLine = new QTimeLine(500, this);
    timeLine->setFrameRange(0, 200);
    QEasingCurve curve = QEasingCurve::OutBounce;
    qreal baseRadius = this->rect().width() / 2;
    qreal difRadius = radius - baseRadius;
    connect(timeLine, &QTimeLine::frameChanged, timeLine, [=](int frame){
        qreal curProgress = curve.valueForProgress(frame / 200.0);
        qreal curRadius = baseRadius + difRadius * curProgress;
        this->setRect(QRectF(center.x() - curRadius, center.y() - curRadius, curRadius * 2, curRadius * 2));
    });
    curAnimation = timeLine;
    startAnimation();
    connect(timeLine, &QTimeLine::finished, [this](){this->state = UNDEFINED;});
    //timeLine->start();
}

void MyGraphicsVexItem::hoverInAnimation(){
    stopAnimation();
    QTimeLine *timeLine = new QTimeLine(300, this);
    timeLine->setFrameRange(0, 100);
    QEasingCurve curve = QEasingCurve::OutBounce;
    qreal baseRadius = this->rect().width() / 2;
    qreal difRadius = 1.25 * radius - baseRadius;
    connect(timeLine, &QTimeLine::frameChanged, [=](int frame){
        qreal curProgress = curve.valueForProgress(frame / 100.0);
        qreal curRadius = baseRadius + difRadius * curProgress;
        this->setRect(QRectF(center.x() - curRadius, center.y() - curRadius, curRadius * 2, curRadius * 2));

    });
    curAnimation = timeLine;
    startAnimation();
}

void MyGraphicsVexItem::hoverOutAnimation(){
    stopAnimation();
    QTimeLine *timeLine = new QTimeLine(300, this);
    timeLine->setFrameRange(0, 100);
    QEasingCurve curve = QEasingCurve::OutBounce;
    qreal baseRadius = this->rect().width() / 2;
    qreal difRadius = radius - baseRadius;
    connect(timeLine, &QTimeLine::frameChanged, [=](int frame){
        qreal curProgress = curve.valueForProgress(frame / 100.0);
        qreal curRadius = baseRadius + difRadius * curProgress;
        this->setRect(QRectF(center.x() - curRadius, center.y() - curRadius, curRadius * 2, curRadius * 2));
    });
    curAnimation = timeLine;
    startAnimation();
}

void MyGraphicsVexItem::clickAnimation(){
    stopAnimation();
    //QTimeLine *timeLine = new QTimeLine(30, this);
    //timeLine->setFrameRange(0, 30);
    //QEasingCurve curve = QEasingCurve::OutBounce;
    //qreal baseRadius = this->rect().width() / 2;
    //qreal difRadius = 0.75 * radius - baseRadius;
    //connect(timeLine, &QTimeLine::frameChanged, [=](int frame){
    //    qreal curProgress = curve.valueForProgress(frame / 30.0);
    //    qreal curRadius = baseRadius + difRadius * curProgress;
    //    this->setRect(QRectF(center.x() - curRadius, center.y() - curRadius, curRadius * 2, curRadius * 2));
    //});
    //curAnimation = timeLine;
    //startAnimation();
    qreal curRadius = 0.75 * radius;
    this->setRect(QRectF(center.x() - curRadius, center.y() - curRadius, curRadius * 2, curRadius * 2));
}

void MyGraphicsVexItem::releaseAnimation(){
    stopAnimation();
    QTimeLine *timeLine = new QTimeLine(300, this);
    timeLine->setFrameRange(0, 100);
    QEasingCurve curve = QEasingCurve::OutBounce;
    qreal baseRadius = this->rect().width() / 2;
    qreal difRadius = (state & ON_HOVER) == 0 ? radius - baseRadius : radius * 1.25 - baseRadius;
    connect(timeLine, &QTimeLine::frameChanged, [=](int frame){
        qreal curProgress = curve.valueForProgress(frame / 100.0);
        qreal curRadius = baseRadius + difRadius * curProgress;
        this->setRect(QRectF(center.x() - curRadius, center.y() - curRadius, curRadius * 2, curRadius * 2));
    });
    curAnimation = timeLine;
    startAnimation();
}

void MyGraphicsVexItem::startAnimation(){
    if(curAnimation != nullptr){
        curAnimation->start();
    }
}

void MyGraphicsVexItem::stopAnimation(){
    if(curAnimation != nullptr){
        curAnimation->stop();
        curAnimation->deleteLater();
        curAnimation = nullptr;
    }
}

void MyGraphicsVexItem::estConnection(MyGraphicsView* view){
    connect(view, SIGNAL(mouseMoved(QPointF)), this, SLOT(onMouseMove(QPointF)));
    connect(view, SIGNAL(mouseLeftClicked(QPointF)), this, SLOT(onLeftClick(QPointF)));
    connect(view, SIGNAL(mouseRightClicked(QPointF)), this, SLOT(onRightClick(QPointF)));
    connect(view, SIGNAL(mouseReleased()), this, SLOT(onMouseRelease()));
    connect(this, SIGNAL(selected(MyGraphicsVexItem*)), view, SLOT(vexSelected(MyGraphicsVexItem*)));
}

void MyGraphicsVexItem::onMouseMove(QPointF position){
    if(state & PREPARING)
        return;
    if(this->contains(position)){
        emit mouseOn(this);
        if((state & ON_HOVER) == 0){
            hoverInAnimation();
            state |= ON_HOVER;
        }
    }
    else{
        if(state & ON_HOVER){
            hoverOutAnimation();
            state &= !ON_HOVER;
        }
    }
}

void MyGraphicsVexItem::onLeftClick(QPointF position){
    if(state & PREPARING)
        return;
    if(state & (ON_LEFT_CLICK | ON_RIGHT_CLICK))
        return;
    if(this->contains(position)){
        emit selected(this);
        state |= ON_LEFT_CLICK;
        clickAnimation();
    }
    else{
        state &= UNDEFINED;
    }
}

void MyGraphicsVexItem::onRightClick(QPointF position){
    if(state & PREPARING)
        return;
    if(state & (ON_LEFT_CLICK | ON_RIGHT_CLICK))
        return;
    if(this->contains(position)){
        state |= ON_RIGHT_CLICK;
        clickAnimation();
    }
    else{
        state &= UNDEFINED;
    }
}

void MyGraphicsVexItem::onMouseRelease(){
    if(state & PREPARING)
        return;
    if(state & (ON_LEFT_CLICK | ON_RIGHT_CLICK)){
        state &= ~(ON_LEFT_CLICK | ON_RIGHT_CLICK);
        releaseAnimation();
    }
}
