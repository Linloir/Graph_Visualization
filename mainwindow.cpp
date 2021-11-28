#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "graph_implement.h"
#include "mycanvas.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->del->setDisabled(true);
    connect(ui->UDG, &QPushButton::clicked, this, [=](){
        ui->UDG->hide();
        ui->DG->hide();
        MyCanvas *newCanvas = new MyCanvas(MyCanvas::UDG);
        connect(ui->pushButton, SIGNAL(clicked()), newCanvas, SLOT(dfs()));
        connect(ui->pushButton_2, SIGNAL(clicked()), newCanvas, SLOT(bfs()));
        connect(ui->spinBox, SIGNAL(valueChanged(int)), newCanvas, SLOT(setWeight(int)));
        connect(ui->dijkstra, SIGNAL(clicked()), newCanvas, SLOT(dijkstra()));
        ui->canvasLayout->addWidget(newCanvas);
        ui->del->setEnabled(true);
        connect(ui->del, &QPushButton::clicked, newCanvas, [=](){
            ui->canvasLayout->removeWidget(newCanvas);
            ui->UDG->show();
            ui->DG->show();
            newCanvas->deleteLater();
        });
    });
    connect(ui->DG, &QPushButton::clicked, this, [=](){
        ui->UDG->hide();
        ui->DG->hide();
        MyCanvas *newCanvas = new MyCanvas(MyCanvas::DG);
        connect(ui->pushButton, SIGNAL(clicked()), newCanvas, SLOT(dfs()));
        connect(ui->pushButton_2, SIGNAL(clicked()), newCanvas, SLOT(bfs()));
        connect(ui->spinBox, SIGNAL(valueChanged(int)), newCanvas, SLOT(setWeight(int)));
        connect(ui->dijkstra, SIGNAL(clicked()), newCanvas, SLOT(dijkstra()));
        ui->canvasLayout->addWidget(newCanvas);
        ui->del->setEnabled(true);
        connect(ui->del, &QPushButton::clicked, newCanvas, [=](){
            ui->canvasLayout->removeWidget(newCanvas);
            ui->UDG->show();
            ui->DG->show();
            newCanvas->deleteLater();
        });
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
