#include "graph_view.h"
#include <QDebug>

//MyGraphicsView
MyGraphicsView::MyGraphicsView(int _type, QWidget *parent) :
    QGraphicsView(parent),
    type(_type){
    this->setMouseTracking(true);
    this->setBackgroundBrush(Qt::transparent);
    myGraphicsScene = new QGraphicsScene();
    this->setScene(myGraphicsScene);
    this->setRenderHint(QPainter::Antialiasing);
    this->setCursor(Qt::CrossCursor);
}

void MyGraphicsView::nextAni(){
    if(aniQueue.size() > 0){
        QTimeLine *next = aniQueue.front();
        curAni = next;
        aniQueue.pop_front();
        connect(next, &QTimeLine::finished, [=](){nextAni(); next->deleteLater();});
        next->setDuration(next->duration() / speedRate);
        next->start();
    }
    else{
        onAni = false;
        curAni = nullptr;
    }
}

void MyGraphicsView::mousePressEvent(QMouseEvent *event){
    if(hasVisitedItem){
        emit visitClear();
        if(curAni){
            curAni->stop();
            curAni->deleteLater();
            curAni = nullptr;
            onAni = false;
        }
        aniQueue.clear();
        hasVisitedItem = false;
    }
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
        else if(event->button() == Qt::RightButton){
            emit mouseRightClicked(mapToScene(event->pos()));
            onRightPress = true;
        }
    }
    changeCursor();
}

void MyGraphicsView::mouseReleaseEvent(QMouseEvent *event){
    if(itemState == NON){
        if(selItem == nullptr){
            //create vex
            if(onRightPress)
                onRightPress = false;
            else
                addVex(mapToScene(event->pos()));
        }
        else{
            //deselect
            if(event->buttons() == 0)
                selItem = nullptr;
            else if(selItem != nullptr)
                itemState |= SEL;
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
    emit vexAdded(newVex);
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
    MyGraphicsLineItem *newLine = new MyGraphicsLineItem(start, end, type == DG);
    scene()->addItem(newLine);
    newLine->estConnection(this);
    newLine->refrshLine();
    newLine->setZValue(--zValue);
    start->addStartLine(newLine);
    end->addEndLine(newLine);

    emit arcAdded(newLine);
}

MyGraphicsVexItem* MyGraphicsView::selectedVex(){
    return selItem ? (selItem->type() == QGraphicsItem::UserType + 1 ? (MyGraphicsVexItem*)selItem : nullptr) : nullptr;
}

MyGraphicsLineItem* MyGraphicsView::selectedArc(){
    return selItem ? (selItem->type() == QGraphicsItem::UserType + 2 ? (MyGraphicsLineItem*)selItem : nullptr) : nullptr;
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

void MyGraphicsView::setMenu(QGraphicsItem *target, bool display){
    if(display){
        itemState |= SEL;
        selItem = target;
    }
}

void MyGraphicsView::addAnimation(QTimeLine *ani){
    aniQueue.push_back(ani);
    if(!onAni){
        onAni = true;
        nextAni();
    }
}

/*****************************************************************************/

unsigned int MyGraphicsVexItem::internalID = 0;

MyGraphicsVexItem::MyGraphicsVexItem(QPointF _center, qreal _r, QGraphicsItem *parent) :
    QGraphicsEllipseItem(_center.x() - 0.5, _center.y() - 0.5, 1, 1, parent),
    center(_center),
    radius(_r){
    id = internalID++;
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
    if(tag)
        tag->moveBy(displacement.x(), displacement.y());
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
    connect(this, SIGNAL(addAnimation(QTimeLine*)), view, SLOT(addAnimation(QTimeLine*)));
}

void MyGraphicsVexItem::select(){
    state = ON_SELECTED;
    this->setBrush(selBrush);
    if(tag)
        tag->setBrush(selBrush);
    emit selected(this);
}

void MyGraphicsVexItem::visit(bool visited){
    if(visited){
        QTimeLine *visitEffect = new QTimeLine;
        visitEffect->setDuration(1000);
        visitEffect->setFrameRange(0, 200);
        QEasingCurve curveIn = QEasingCurve::InElastic;
        QEasingCurve curveOut = QEasingCurve::OutBounce;
        connect(visitEffect, &QTimeLine::frameChanged, this, [=](int frame){
            if(frame > 100){
                this->setBrush(visitedBrush);
                if(tag)
                    tag->setBrush(visitedBrush);
            }
            if(frame < 100){
                qreal curProgress = curveIn.valueForProgress(frame / 100.0);
                qreal curRadius = radius + 0.3 * radius * curProgress;
                this->setRect(QRectF(center.x() - curRadius, center.y() - curRadius, curRadius * 2, curRadius * 2));
            }
            else{
                qreal curProgress = curveOut.valueForProgress((frame - 100.0) / 100.0);
                qreal curRadius = 1.3 * radius - 0.3 * radius * curProgress;
                this->setRect(QRectF(center.x() - curRadius, center.y() - curRadius, curRadius * 2, curRadius * 2));
            }
        });
        emit addAnimation(visitEffect);
    }
    else{
        if(state & ON_SELECTED){
            this->setBrush(selBrush);
            if(tag)
                tag->setBrush(selBrush);
        }
        else{
            this->setBrush(regBrush);
            if(tag)
                tag->setBrush(regBrush);
        }
    }
}

void MyGraphicsVexItem::access(const QString &hint, bool isAccess){
    if(isAccess){
        hintText = hint;
        if(!tag)
            tag = new QGraphicsSimpleTextItem;
        tag->setText(hintText);
        tag->setPos(center + QPointF(radius, radius) * 1.3);
        tag->setFont(hintFont);
        tag->setZValue(this->zValue());
        QTimeLine *accessEffect = new QTimeLine;
        accessEffect->setDuration(1000);
        accessEffect->setFrameRange(0, 200);
        QEasingCurve curveIn = QEasingCurve::InElastic;
        QEasingCurve curveOut = QEasingCurve::OutBounce;
        connect(accessEffect, &QTimeLine::frameChanged, this, [=](int frame){
            if(frame > 100){
                this->setBrush(accessBrush);
                this->tag->setBrush(accessBrush);
                this->scene()->addItem(tag);
            }
            if(frame < 100){
                qreal curProgress = curveIn.valueForProgress(frame / 100.0);
                qreal curRadius = radius + 0.3 * radius * curProgress;
                this->setRect(QRectF(center.x() - curRadius, center.y() - curRadius, curRadius * 2, curRadius * 2));
                this->tag->setScale(1 + curProgress * 0.2);
            }
            else{
                qreal curProgress = curveOut.valueForProgress((frame - 100.0) / 100.0);
                qreal curRadius = 1.3 * radius - 0.3 * radius * curProgress;
                this->setRect(QRectF(center.x() - curRadius, center.y() - curRadius, curRadius * 2, curRadius * 2));
                this->tag->setScale(1.2 - curProgress * 0.2);
            }
        });
        emit addAnimation(accessEffect);
    }
    else{
        hintText = "";
        if(tag)
            scene()->removeItem(tag);
        tag = nullptr;
    }
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
        if(state & (ON_MENU | ON_SELECTED)){
            if(state & ON_MENU){
                emit menuStateChanged(this, false);
                state &= ~ON_MENU;
            }
            if(state & ON_SELECTED){
                this->setBrush(regBrush);
                state &= ~ON_SELECTED;
            }
        }
        else
            state &= UNDEFINED;
    }
    visit(false);
}

void MyGraphicsVexItem::onRightClick(QPointF position){
    if(state & PREPARING)
        return;
    if(state & (ON_LEFT_CLICK | ON_RIGHT_CLICK))
        return;
    if(this->contains(position)){
        emit selected(this);
        state |= ON_RIGHT_CLICK;
        onClickEffect();
    }
    else{
        if(state & (ON_MENU | ON_SELECTED)){
            if(state & ON_MENU){
                emit menuStateChanged(this, false);
                state &= ~ON_MENU;
            }
            if(state & ON_SELECTED){
                this->setBrush(regBrush);
                state &= ~ON_SELECTED;
            }
        }
        else
            state &= UNDEFINED;
        //if(state & ON_SELECTED)
        //    this->setBrush(regBrush);
        //state &= UNDEFINED;
    }
    visit(false);
}

void MyGraphicsVexItem::onMouseRelease(){
    if(state & PREPARING)
        return;
    if(state & (ON_LEFT_CLICK | ON_RIGHT_CLICK)){
        if(state & ON_SELECTED){
            if(state & ON_LEFT_CLICK)
                emit lineFrom(this);
            else if(state & ON_RIGHT_CLICK){
                emit menuStateChanged(this, true);
                state |= (ON_MENU | ON_SELECTED);
            }
        }
        else{
            this->setBrush(selBrush);
            if(state & ON_LEFT_CLICK)
                state |= ON_SELECTED;
            if(state & ON_RIGHT_CLICK){
                emit menuStateChanged(this, true);
                state |= (ON_MENU | ON_SELECTED);
            }
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
    curPen = defaultPen;
    //textItem = new QGraphicsSimpleTextItem(text);
}

void MyGraphicsLineItem::refrshLine(){
    setLengthRate(1);
    drawLine();
}

void MyGraphicsLineItem::drawLine(){
    //draw invisible line
    this->setLine(sP.x(), sP.y(), eP.x(), eP.y());
    //this->setPen(curPen);
    QPen bgPen;
    bgPen.setColor(QColor(255, 255, 255, 0));
    bgPen.setWidth(lineWidth + 5);
    this->setPen(bgPen);

    if(line1){
        scene()->removeItem(line1);
        line1 = nullptr;
    }
    if(line2){
        scene()->removeItem(line2);
        line2 = nullptr;
    }

    center = (startVex->scenePos() + startVex->rect().center() + endVex->scenePos() + endVex->rect().center())/2;

    drawText();

    if(text != "" && (eP - center).x() * (sP - center).x() <= 0){
        qreal dx = 0;
        qreal dy = 0;
        int f1 = 1, f2 = 1;
        if(textItem->boundingRect().width() != 0){
            if(abs(textItem->boundingRect().height() / textItem->boundingRect().width()) < abs(tan(angle))){
                dx = (textItem->boundingRect().height() + 10) / (2 * tan(angle));
                dy = (textItem->boundingRect().height() + 10) / 2;
                f2 = angle > 0 ? -1 : 1;
            }
            else{
                dy = (textItem->boundingRect().width() + 10) * tan(angle) / 2;
                dx = (textItem->boundingRect().width() + 10) / 2;
                f1 = tan(angle) < 0 ? -1 : 1;
                f2 = angle >= 0 ? -1 : 1;
            }
        }
        dx *= f1 * f2;
        dy *= f1 * f2;
        QGraphicsLineItem *newLine1 = new QGraphicsLineItem(sP.x(), sP.y(), center.x() + dx, center.y() + dy);
        QGraphicsLineItem *newLine2 = new QGraphicsLineItem(center.x() - dx, center.y() - dy, eP.x(), eP.y());

        newLine1->setZValue(this->zValue() - 1);
        newLine2->setZValue(this->zValue() - 1);
        newLine1->setPen(curPen);
        newLine2->setPen(curPen);

        scene()->addItem(newLine1);
        scene()->addItem(newLine2);
        line1 = newLine1;
        line2 = newLine2;
    }
    else{
        QGraphicsLineItem *newLine = new QGraphicsLineItem(sP.x(), sP.y(), eP.x(), eP.y());
        newLine->setPen(curPen);
        newLine->setZValue(this->zValue() - 1);
        this->scene()->addItem(newLine);
        line1 = newLine;
    }

    if(hasDirection){
        delArrow();
        drawArrow();
    }
}

void MyGraphicsLineItem::drawText(){
    if(textItem){
        this->scene()->removeItem(textItem);
        textItem = nullptr;
    }
    QGraphicsSimpleTextItem *t = new QGraphicsSimpleTextItem(text);
    t->setFont(textFont);
    t->setPos(center - QPointF(t->boundingRect().width(), t->boundingRect().height()) / 2);
    QColor c = curPen.color();
    t->setBrush(c.darker(150));
    this->scene()->addItem(t);
    textItem = t;
}

void MyGraphicsLineItem::drawArrow(){
    QPointF leftEnd = QPointF(eP.x() - cos(angle - M_PI / 6) * arrowLength, eP.y() - sin(angle - M_PI / 6) * arrowLength);
    QPointF rightEnd = QPointF(eP.x() - cos(angle + M_PI / 6) * arrowLength, eP.y() - sin(angle + M_PI / 6) * arrowLength);

    QPainterPath arrowPath;
    arrowPath.moveTo(leftEnd);
    arrowPath.lineTo(eP);
    arrowPath.lineTo(rightEnd);

    QGraphicsPathItem* arrowItem = new QGraphicsPathItem(arrowPath);
    arrowItem->setPen(curPen);
    this->scene()->addItem(arrowItem);
    arrow = arrowItem;
}

void MyGraphicsLineItem::delArrow(){
    if(arrow != nullptr){
        this->scene()->removeItem(arrow);
        arrow = nullptr;
    }
}

void MyGraphicsLineItem::setLengthRate(qreal r){
    sP = startVex->scenePos() + startVex->rect().center();
    eP = endVex->scenePos() + endVex->rect().center();
    dP = eP - sP;
    angle = atan2(dP.y(), dP.x());
    eP -= QPointF(endVex->getRadius() * cos(angle), endVex->getRadius() * sin(angle));
    sP += QPointF(endVex->getRadius() * cos(angle), endVex->getRadius() * sin(angle));
    dP = (eP - sP) * r;
    eP = sP + dP;
}

void MyGraphicsLineItem::estConnection(MyGraphicsView *view){
    connect(view, SIGNAL(mouseMoved(QPointF)), this, SLOT(onMouseMove(QPointF)));
    connect(view, SIGNAL(mouseLeftClicked(QPointF)), this, SLOT(onLeftClick(QPointF)));
    connect(view, SIGNAL(mouseRightClicked(QPointF)), this, SLOT(onRightClick(QPointF)));
    connect(view, SIGNAL(mouseReleased()), this, SLOT(onMouseRelease()));
    connect(this, SIGNAL(setHover(bool)), view, SLOT(setHover(bool)));
    connect(this, SIGNAL(selected(QGraphicsItem*)), view, SLOT(setSel(QGraphicsItem*)));
    connect(this, SIGNAL(menuStateChanged(QGraphicsItem*, bool)), view, SLOT(setMenu(QGraphicsItem*, bool)));
    connect(this, SIGNAL(addAnimation(QTimeLine*)), view, SLOT(addAnimation(QTimeLine*)));
}

void MyGraphicsLineItem::reverseDirection(){
    delArrow();
    startVex->removeStartLine(this);
    endVex->removeEndLine(this);
    MyGraphicsVexItem *temp = startVex;
    startVex = endVex;
    endVex = temp;
    startVex->addStartLine(this);
    endVex->addEndLine(this);
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

void MyGraphicsLineItem::setText(const QString &_text){
    text = _text;
    refrshLine();
}

void MyGraphicsLineItem::setDirection(bool hasDir){
    hasDirection = hasDir;
    refrshLine();
}

void MyGraphicsLineItem::hoverInEffect(){
    curPen.setWidth(lineWidth + 1);
    //curPen.setColor(hoverColor);
    refrshLine();
}

void MyGraphicsLineItem::hoverOutEffect(){
    curPen.setWidth(lineWidth);
    //curPen.setColor(state & ON_VISIT ? visitColor : state & ON_SELECTED ? selColor : defaultColor);
    refrshLine();
}

void MyGraphicsLineItem::onClickEffect(){
    curPen.setWidth(lineWidth - 1);
    refrshLine();
}

void MyGraphicsLineItem::onReleaseEffect(){
    curPen.setWidth(lineWidth);
    curPen.setColor(selColor);
    refrshLine();
}

void MyGraphicsLineItem::onSelectEffect(){
    curPen.setColor(selColor);
    refrshLine();
}

void MyGraphicsLineItem::deSelectEffect(){
    curPen = defaultPen;
    refrshLine();
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
    if(state & ON_VISIT)
        visit(false);
    if(this->contains(position)){
        emit selected(this);
        onClickEffect();
        state |= ON_LEFT_CLICK;
    }
    else{
        if(state & (ON_MENU | ON_SELECTED)){
            if(state & ON_MENU){
                emit menuStateChanged(this, false);
                state &= ~ON_MENU;
            }
            if(state & ON_SELECTED){
                deSelectEffect();
                state &= ~ON_SELECTED;
            }
        }
        else
            state &= UNDEFINED;
    }
}

void MyGraphicsLineItem::onRightClick(QPointF position){
    if(state & (ON_LEFT_CLICK | ON_RIGHT_CLICK))
        return;
    if(state & ON_VISIT)
        visit(false);
    if(this->contains(position)){
        emit selected(this);
        onClickEffect();
        state |= ON_RIGHT_CLICK;
    }
    else{
        if(state & (ON_MENU | ON_SELECTED)){
            if(state & ON_MENU){
                emit menuStateChanged(this, false);
                state &= ~ON_MENU;
            }
            if(state & ON_SELECTED){
                deSelectEffect();
                state &= ~ON_SELECTED;
            }
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

void MyGraphicsLineItem::visit(bool visited){
    if(visited){
        state |= ON_VISIT;
        QTimeLine *visitEffect = new QTimeLine;
        visitEffect->setDuration(1000);
        visitEffect->setFrameRange(0, 200);
        QEasingCurve curve = QEasingCurve::InOutQuad;
        QGraphicsLineItem *newLine1 = new QGraphicsLineItem(line1->line());
        QGraphicsLineItem *newLine2 = line2 ? new QGraphicsLineItem(line2->line()) : nullptr;
        connect(visitEffect, &QTimeLine::stateChanged, this, [=](){
            if(visitEffect->state() == QTimeLine::Running){
                QPen pen;
                pen.setWidth(3);
                pen.setStyle(Qt::DashLine);
                pen.setBrush(QColor(58, 143, 192, 100));
                pen.setCapStyle(Qt::RoundCap);
                newLine1->setPen(pen);
                newLine1->setZValue(this->zValue() - 2);
                scene()->addItem(newLine1);
                if(newLine2){
                    newLine2->setPen(pen);
                    newLine2->setZValue(this->zValue() - 2);
                    scene()->addItem(newLine2);
                }
            }
            else{
                scene()->removeItem(newLine1);
                if(newLine2)
                    scene()->removeItem(newLine2);
            }
        });
        connect(visitEffect, &QTimeLine::frameChanged, this, [=](int frame){
            this->curPen.setColor(visitColor);
            qreal curProgress = curve.valueForProgress(frame / 200.0);
            setLengthRate(curProgress);
            drawLine();
        });
        emit addAnimation(visitEffect);
    }
    else{
        state &= ~ON_VISIT;
        curPen = defaultPen;
        refrshLine();
    }
}

void MyGraphicsLineItem::access(){
    QTimeLine *accessEffect = new QTimeLine;
    accessEffect->setDuration(1000);
    accessEffect->setFrameRange(0, 200);
    QEasingCurve curve = QEasingCurve::InOutQuad;
    QGraphicsLineItem *newLine1 = new QGraphicsLineItem(line1->line());
    QGraphicsLineItem *newLine2 = line2 ? new QGraphicsLineItem(line2->line()) : nullptr;
    connect(accessEffect, &QTimeLine::stateChanged, this, [=](){
        if(accessEffect->state() == QTimeLine::Running){
            QPen pen;
            pen.setWidth(3);
            pen.setStyle(Qt::DashLine);
            pen.setBrush(QColor(58, 143, 192, 100));
            pen.setCapStyle(Qt::RoundCap);
            newLine1->setPen(pen);
            newLine1->setZValue(this->zValue() - 2);
            scene()->addItem(newLine1);
            if(newLine2){
                newLine2->setPen(pen);
                newLine2->setZValue(this->zValue() - 2);
                scene()->addItem(newLine2);
            }
        }
        else{
            scene()->removeItem(newLine1);
            if(newLine2)
                scene()->removeItem(newLine2);
        }
    });
    connect(accessEffect, &QTimeLine::frameChanged, this, [=](int frame){
        this->curPen.setColor(accessColor);
        qreal curProgress = curve.valueForProgress(frame / 200.0);
        setLengthRate(curProgress);
        drawLine();
    });
    emit addAnimation(accessEffect);
    state |= ON_SELECTED;
}
