#ifndef MYCANVAS_H
#define MYCANVAS_H

#include <QWidget>
#include <QGridLayout>
#include "graph_view.h"
#include "graph_implement.h"

class MyCanvas : public QWidget
{
    Q_OBJECT
private:
    MyGraphicsView *view;
    AbstractGraph *g;
    int type;

public:
    enum { UDG = AbstractGraph::UDG, DG = AbstractGraph::DG };
    explicit MyCanvas(int _type = UDG, QWidget *parent = nullptr);

signals:

private slots:
    void addVex(MyGraphicsVexItem*);
    void addArc(MyGraphicsLineItem*);
    //void delVex(MyGraphicsVexItem*);
    //void delArc(MyGraphicsLineItem*);
    void dfs();
    void bfs();

};

#endif // MYCANVAS_H
