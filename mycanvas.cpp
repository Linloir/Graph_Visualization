#include "mycanvas.h"

MyCanvas::MyCanvas(int _type, QWidget *parent) : QWidget(parent), type(_type)
{
    QGridLayout *mainLayout = new QGridLayout(this);
    this->setLayout(mainLayout);
    view = new MyGraphicsView(type == UDG ? MyGraphicsView::UDG : MyGraphicsView::DG);
    view->setSceneRect(view->rect());
    view->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
    mainLayout->addWidget(view);
    g = new ALGraph(type);
    connect(view, SIGNAL(vexAdded(MyGraphicsVexItem*)), this, SLOT(addVex(MyGraphicsVexItem*)));
    connect(view, SIGNAL(arcAdded(MyGraphicsLineItem*)), this, SLOT(addArc(MyGraphicsLineItem*)));
    connect(view, &MyGraphicsView::visitClear, this, [=](){g->ClearVisit();});
}

void MyCanvas::addVex(MyGraphicsVexItem *vex){
    g->AddVex(vex);
}

void MyCanvas::addArc(MyGraphicsLineItem *arc){
    g->AddArc(arc);
}

void MyCanvas::dfs(){
    MyGraphicsVexItem *strtVex = view->selectedVex();
    if(strtVex != nullptr){
        g->DFS(strtVex);
        view->hasVisitedItem = true;
    }
}

void MyCanvas::bfs(){
    MyGraphicsVexItem *strtVex = view->selectedVex();
    if(strtVex != nullptr){
        g->BFS(strtVex);
        view->hasVisitedItem = true;
    }
}
