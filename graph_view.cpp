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
    this->setCursor(Qt::CrossCursor);
}

void MyGraphicsView::mousePressEvent(QMouseEvent *event){
    if(itemState & ADD){
        if(event->button() == Qt::LeftButton){
            selItem = nullptr;
            emit mouseLeftClicked(mapToScene(event->pos()));
        }
        else{
            clearSketch();
            itemState = selItem == nullptr ? NON : SEL | VEX;
        }
    }
    else{
        itemState = NON;
        if(event->button() == Qt::LeftButton)
            emit mouseLeftClicked(mapToScene(event->pos()));
        else if(event->button() == Qt::RightButton)
            emit mouseRightClicked(mapToScene(event->pos()));
    }
    changeCursor();
}

void MyGraphicsView::mouseReleaseEvent(QMouseEvent *event){
    if(itemState == NON){
        if(selItem == nullptr){
            //create vex
            addVex(mapToScene(event->pos()));
        }
        else{
            //deselect
            selItem = nullptr;
        }
    }
    else if(itemState & ADD){
        clearSketch();
        itemState = NON;
        if(selItem == nullptr){
            selItem = addVex(mapToScene(event->pos()));
            ((MyGraphicsVexItem*)selItem)->select();
            addLine(strtVex, (MyGraphicsVexItem*)selItem);
        }
        else if(selItem->type() == QGraphicsItem::UserType + 1){
            addLine(strtVex, (MyGraphicsVexItem*)selItem);
        }
        else{
            itemState = SEL | selItem->type();
        }
    }
    emit mouseReleased();
    changeCursor();
}

void MyGraphicsView::mouseMoveEvent(QMouseEvent *event){
    changeCursor();
    emit mouseMoved(mapToScene(event->pos()));
    if(itemState & ADD){
        clearSketch();
        sketchLine(strtVex->scenePos() + strtVex->rect().center(), mapToScene(event->pos()));
    }
}

void MyGraphicsView::changeCursor(){

}

MyGraphicsVexItem* MyGraphicsView::addVex(QPointF center, qreal radius){
    MyGraphicsVexItem *newVex = new MyGraphicsVexItem(center, radius);
    this->scene()->addItem(newVex);
    newVex->estConnection(this);
    newVex->showAnimation();
    return newVex;
}

void MyGraphicsView::clearSketch(){
    if(sketchItem != nullptr){
        scene()->removeItem(sketchItem);
        sketchItem = nullptr;
    }
}

void MyGraphicsView::sketchLine(QPointF start, QPointF end){
    QGraphicsLineItem *newLine = new QGraphicsLineItem(start.x(), start.y(), end.x(), end.y());
    QPen pen;
    pen.setWidth(3);
    pen.setStyle(Qt::DashLine);
    pen.setBrush(QColor(58, 143, 192, 100));
    pen.setCapStyle(Qt::RoundCap);
    newLine->setPen(pen);
    scene()->addItem(newLine);
    newLine->stackBefore(selItem);
    sketchItem = newLine;
}

void MyGraphicsView::addLine(MyGraphicsVexItem *start, MyGraphicsVexItem *end){
    MyGraphicsLineItem *newLine = new MyGraphicsLineItem(start, end, true);
    scene()->addItem(newLine);
    newLine->estConnection(this);
    newLine->refrshLine();
    newLine->setZValue(--zValue);
    start->addStartLine(newLine);
    end->addEndLine(newLine);
}

void MyGraphicsView::setHover(bool in){
    if(in)
        mouseState |= ON_HOVER;
    else
        mouseState &= ~ON_HOVER;
}

void MyGraphicsView::setSel(QGraphicsItem *sel){
    int state = SEL | (sel->type() - QGraphicsItem::UserType);
    if(itemState == NON){
        itemState = state;
        selItem = sel;
    }
    else if(itemState & SEL){
        if(itemState > state){
            itemState = state;
            selItem = sel;
        }
    }
    else if(itemState & ADD){
        if(selItem == nullptr || selItem->type() > sel->type())
            selItem = sel;
    }
}

void MyGraphicsView::startLine(MyGraphicsVexItem *startVex){
    itemState = ADD | LINE;
    strtVex = startVex;
}

//MyGraphicsVexItem
MyGraphicsVexItem::MyGraphicsVexItem(QPointF _center, qreal _r, QGraphicsItem *parent) :
    QGraphicsEllipseItem(_center.x() - 0.5, _center.y() - 0.5, 1, 1, parent),
    center(_center),
    radius(_r){
    this->setPen(Qt::NoPen);
    this->setBrush(regBrush);
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
    connect(timeLine, &QTimeLine::finished, [this](){this->state &= ~PREPARING;});
    //timeLine->start();
}

void MyGraphicsVexItem::hoverInEffect(){
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

void MyGraphicsVexItem::hoverOutEffect(){
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

void MyGraphicsVexItem::onClickEffect(){
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

void MyGraphicsVexItem::onReleaseEffect(){
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

void MyGraphicsVexItem::move(QPointF position){
    QPointF displacement = position - (this->scenePos() + this->rect().center());
    this->setRect(QRectF(this->rect().x() + displacement.x(), this->rect().y() + displacement.y(), this->rect().width(), this->rect().height()));
    center = center + displacement;
    for(int i = 0; i < linesStartWith.size(); i++)
        linesStartWith[i]->moveStart(this);
    for(int i = 0; i < linesEndWith.size(); i++)
        linesEndWith[i]->moveEnd(this);
}

void MyGraphicsVexItem::estConnection(MyGraphicsView* view){
    connect(view, SIGNAL(mouseMoved(QPointF)), this, SLOT(onMouseMove(QPointF)));
    connect(view, SIGNAL(mouseLeftClicked(QPointF)), this, SLOT(onLeftClick(QPointF)));
    connect(view, SIGNAL(mouseRightClicked(QPointF)), this, SLOT(onRightClick(QPointF)));
    connect(view, SIGNAL(mouseReleased()), this, SLOT(onMouseRelease()));
    connect(this, SIGNAL(setHover(bool)), view, SLOT(setHover(bool)));
    connect(this, SIGNAL(selected(QGraphicsItem*)), view, SLOT(setSel(QGraphicsItem*)));
    connect(this, SIGNAL(lineFrom(MyGraphicsVexItem*)), view, SLOT(startLine(MyGraphicsVexItem*)));
    connect(this, SIGNAL(menuStateChanged(QGraphicsItem*, bool)), view, SLOT(setMenu(QGraphicsItem*, bool)));
}

void MyGraphicsVexItem::select(){
    state = ON_SELECTED;
    this->setBrush(selBrush);
    emit selected(this);
}

void MyGraphicsVexItem::onMouseMove(QPointF position){
    if(state & PREPARING)
        return;
    if((state & ON_LEFT_CLICK) == 0){
        if(this->contains(position)){
            if((state & ON_HOVER) == 0){
                emit setHover(true);
                hoverInEffect();
                state |= ON_HOVER;
            }
        }
        else{
            if(state & ON_HOVER){
                emit setHover(false);
                hoverOutEffect();
                state &= ~ON_HOVER;
            }
        }
    }
    else{
        move(position);
        state &= ~ON_SELECTED;
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
        onClickEffect();
    }
    else{
        if(state & ON_SELECTED)
            this->setBrush(regBrush);
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
        onClickEffect();
    }
    else{
        if(state & ON_SELECTED)
            this->setBrush(regBrush);
        state &= UNDEFINED;
    }
}

void MyGraphicsVexItem::onMouseRelease(){
    if(state & PREPARING)
        return;
    if(state & (ON_LEFT_CLICK | ON_RIGHT_CLICK)){
        if(state & ON_SELECTED){
            if(state & ON_LEFT_CLICK)
                emit lineFrom(this);
        }
        else{
            this->setBrush(selBrush);
            if(state & ON_LEFT_CLICK)
                state |= ON_SELECTED;
        }
        state &= ~(ON_LEFT_CLICK | ON_RIGHT_CLICK);
        onReleaseEffect();
    }
}

/********************************************************/

MyGraphicsLineItem::MyGraphicsLineItem(MyGraphicsVexItem *start, MyGraphicsVexItem *end, bool hasDir, QGraphicsItem *parent) :
    QGraphicsLineItem(parent),
    hasDirection(hasDir),
    startVex(start),
    endVex(end){
    //Set display effect
    defaultPen.setWidth(lineWidth);
    defaultPen.setStyle(lineStyle);
    defaultPen.setCapStyle(capStyle);
    defaultPen.setColor(defaultColor);
    this->setPen(defaultPen);
}

void MyGraphicsLineItem::refrshLine(){
    delArrow();
    QPointF startPos = startVex->scenePos() + startVex->rect().center();
    QPointF endPos = endVex->scenePos() + endVex->rect().center();
    this->setLine(startPos.x(), startPos.y(), endPos.x(), endPos.y());
    this->setPen(defaultPen);
    if(hasDirection)
        drawArrow();
}

void MyGraphicsLineItem::drawArrow(){
    QPointF sP = startVex->scenePos() + startVex->rect().center();
    QPointF eP = endVex->scenePos() + endVex->rect().center();
    QPointF dP = eP - sP;

    qreal angle = atan2(dP.y(), dP.x());
    eP -= QPointF(endVex->getRadius() * cos(angle), endVex->getRadius() * sin(angle));
    QPointF leftEnd = QPointF(eP.x() - cos(angle - M_PI / 6) * arrowLength, eP.y() - sin(angle - M_PI / 6) * arrowLength);
    QPointF rightEnd = QPointF(eP.x() - cos(angle + M_PI / 6) * arrowLength, eP.y() - sin(angle + M_PI / 6) * arrowLength);

    QPainterPath arrowPath;
    arrowPath.moveTo(leftEnd);
    arrowPath.lineTo(eP);
    arrowPath.lineTo(rightEnd);

    QGraphicsPathItem* arrowItem = new QGraphicsPathItem(arrowPath);
    arrowItem->setPen(defaultPen);
    this->scene()->addItem(arrowItem);
    arrow = arrowItem;
}

void MyGraphicsLineItem::delArrow(){
    if(arrow != nullptr){
        this->scene()->removeItem(arrow);
        arrow = nullptr;
    }
}

void MyGraphicsLineItem::estConnection(MyGraphicsView *view){
    connect(view, SIGNAL(mouseMoved(QPointF)), this, SLOT(onMouseMove(QPointF)));
    connect(view, SIGNAL(mouseLeftClicked(QPointF)), this, SLOT(onLeftClick(QPointF)));
    connect(view, SIGNAL(mouseRightClicked(QPointF)), this, SLOT(onRightClick(QPointF)));
    connect(view, SIGNAL(mouseReleased()), this, SLOT(onMouseRelease()));
    connect(this, SIGNAL(setHover(bool)), view, SLOT(setHover(bool)));
    connect(this, SIGNAL(selected(QGraphicsItem*)), view, SLOT(setSel(QGraphicsItem*)));
    connect(this, SIGNAL(menuStateChanged(QGraphicsItem*, bool)), view, SLOT(setMenu(QGraphicsItem*, bool)));
}

void MyGraphicsLineItem::reverseDirection(){
    delArrow();
    MyGraphicsVexItem *temp = startVex;
    startVex = endVex;
    endVex = temp;
    refrshLine();
}

void MyGraphicsLineItem::moveStart(MyGraphicsVexItem *start){
    delArrow();
    startVex = start;
    refrshLine();
}

void MyGraphicsLineItem::moveEnd(MyGraphicsVexItem *end){
    delArrow();
    endVex = end;
    refrshLine();
}

void MyGraphicsLineItem::setDirection(bool hasDir){
    hasDirection = hasDir;
    refrshLine();
}

void MyGraphicsLineItem::hoverInEffect(){
    QPen pen = this->pen();
    pen.setWidth(lineWidth + 1);
    pen.setColor(hoverColor);
    this->setPen(pen);
    if(arrow != nullptr)
        arrow->setPen(pen);
}

void MyGraphicsLineItem::hoverOutEffect(){
    this->setPen(defaultPen);
    if(arrow != nullptr)
        arrow->setPen(defaultPen);
}

void MyGraphicsLineItem::onClickEffect(){
    QPen pen = this->pen();
    pen.setColor(selColor);
    pen.setWidth(lineWidth - 0.5);
    this->setPen(pen);
    if(arrow != nullptr)
        arrow->setPen(pen);
}

void MyGraphicsLineItem::onReleaseEffect(){
    QPen pen = this->pen();
    pen.setWidth(lineWidth);
    this->setPen(pen);
    if(arrow != nullptr)
        arrow->setPen(pen);
}

void MyGraphicsLineItem::onMouseMove(QPointF position){
    if(this->contains(position)){
        emit setHover(true);
        hoverInEffect();
        state |= ON_HOVER;
    }
    else{
        if(state & ON_HOVER){
            emit setHover(false);
            hoverOutEffect();
            state &= ~ON_HOVER;
        }
    }
}

void MyGraphicsLineItem::onLeftClick(QPointF position){
    if(state & (ON_LEFT_CLICK | ON_RIGHT_CLICK))
        return;
    if(this->contains(position)){
        emit selected(this);
        onClickEffect();
        state |= ON_LEFT_CLICK;
    }
    else{
        if(state & ON_SELECTED){
            deSelectEffect();
        }
        state &= UNDEFINED;
    }
}

void MyGraphicsLineItem::onRightClick(QPointF position){
    if(state & (ON_LEFT_CLICK | ON_RIGHT_CLICK))
        return;
    if(this->contains(position)){
        emit selected(this);
        onClickEffect();
        state |= ON_RIGHT_CLICK;
    }
    else{
        if(state & ON_MENU){
            emit menuStateChanged(this, false);
            state &= ~ON_MENU;
        }
        else if(state & ON_SELECTED){
            deSelectEffect();
            state &= ~ON_SELECTED;
        }
        else
            state &= UNDEFINED;
    }
}

void MyGraphicsLineItem::onMouseRelease(){
    if(state & (ON_LEFT_CLICK | ON_RIGHT_CLICK)){
        onReleaseEffect();
        if((state & ON_SELECTED) == 0)
            onSelectEffect();
        if(state & ON_RIGHT_CLICK){
            emit menuStateChanged(this, true);
            state |= ON_MENU;
        }
        state |= ON_SELECTED;
        state &= ~(ON_LEFT_CLICK | ON_RIGHT_CLICK);
    }
}
