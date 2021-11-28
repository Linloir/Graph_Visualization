#ifndef GRAPH_IMPLEMENT_H
#define GRAPH_IMPLEMENT_H

#include "graph_view.h"

using namespace std;

class VexInfo{
public:
    enum { INF = 2147483647 };
    MyGraphicsVexItem *gVex;
    string text;
    VexInfo *strtVexInfo = nullptr;
    int preVexID = -1;
    int distance = INF;

    VexInfo(MyGraphicsVexItem *vex, QString t = "") : gVex(vex), text(t.toStdString()){}
};

class AbstractGraph{
protected:
    int type;

public:
    enum { UDG = 1, DG = 2 };

    AbstractGraph(int _type = DG) : type(_type){}
    /* Insert */
    virtual void AddVex(MyGraphicsVexItem *gvex) = 0;
    virtual void AddArc(MyGraphicsLineItem *garc) = 0;

    /* Delete */
    virtual void DelVex(MyGraphicsVexItem *gvex) = 0;
    virtual void DelVex(int vexID) = 0;
    virtual void DelArc(MyGraphicsLineItem *garc) = 0;
    virtual void DelArc(int sVexID, int eVexID) = 0;

    /* Find */
    virtual int GetIdOf(MyGraphicsVexItem *gvex) = 0;

    /* Modify */
    //virtual void SetText(MyGraphicsVexItem *gvex);
    virtual void SetWeight(MyGraphicsLineItem *garc, int weight) = 0;

    /* Other Function */
    virtual void ClearVisit() = 0;
    virtual void ResetDistance() = 0;
    virtual void DFS(int strtID) = 0;
    virtual void DFS(MyGraphicsVexItem *strtVex) = 0;
    virtual void BFS(int strtID) = 0;
    virtual void BFS(MyGraphicsVexItem *strtVex) = 0;
    virtual void Dijkstra(int strtID) = 0;
    virtual void Dijkstra(MyGraphicsVexItem *strtVex) = 0;

    virtual int Type() const = 0;
};

/* Classes of ALGraph */

class ALVex;
class ALArc;
class ALGraph;

class ALVex{
public:
    bool visited = false;

    VexInfo *info = nullptr;
    ALArc *firstArc = nullptr;

    ALVex(MyGraphicsVexItem *vex){info = new VexInfo(vex);}
    bool equalTo(const ALVex &v){return info == v.info;}
    void visit();
    void access(const QString &hint = "");
};

class ALArc{
public:
    MyGraphicsLineItem *gArc;
    int weight = 1;
    int eVexID;
    ALArc *nextArc = nullptr;

    ALArc(MyGraphicsLineItem *garc, int eVex, ALArc *next = nullptr) : gArc(garc), eVexID(eVex), nextArc(next){}
    void visit();
    void access();
};

class ALGraph : public AbstractGraph{
private:
    vector<ALVex> vexList;

public:
    ALGraph(int _type = DG) : AbstractGraph(_type){}
    /* Insert */
    void AddVex(MyGraphicsVexItem *gvex);
    void AddArc(MyGraphicsLineItem *garc);

    /* Delete */
    void DelVex(MyGraphicsVexItem *gvex);
    void DelVex(int vexID);
    void DelArc(MyGraphicsLineItem *garc);
    void DelArc(int sVexID, int eVexID);

    /* Find */
    int GetIdOf(MyGraphicsVexItem *gvex);
    ALArc* FindArc(int sID, int eID);

    /* Modify */
    void SetWeight(MyGraphicsLineItem *garc, int weight);

    /* Other Function */
    void ClearVisit();
    void ResetDistance();
    void DFS(int strtID);
    void DFS(MyGraphicsVexItem *strtVex){DFS(GetIdOf(strtVex));}
    void BFS(int strtID);
    void BFS(MyGraphicsVexItem *strtVex){BFS(GetIdOf(strtVex));}
    void Dijkstra(int strtID);
    void Dijkstra(MyGraphicsVexItem *strtVex){Dijkstra(GetIdOf(strtVex));}

    int Type() const { return type; }
};

/* Classes of AMLGraph */

class AMLVex;
class AMLArc;
class AMLGraph;

class AMLVex{
public:
    VexInfo *info = nullptr;
    AMLArc *firstArc = nullptr;

    AMLVex(MyGraphicsVexItem *gvex){info = new VexInfo(gvex);}
    bool equalTo(const AMLVex &v){return info == v.info;}
    void visit();
};

class AMLArc{
public:
    MyGraphicsLineItem *gArc;
    int weight = 1;
    int outVexID;
    AMLArc *nextOutArc = nullptr;
    int inVexID;
    AMLArc *nextInArc = nullptr;

    AMLArc(MyGraphicsLineItem *garc, int sVex, int eVex) : gArc(garc), outVexID(sVex), inVexID(eVex){}
    AMLArc(MyGraphicsLineItem *garc, int sVex, int eVex, AMLArc *nextOut, AMLArc *nextIn) :
        gArc(garc), outVexID(sVex), nextOutArc(nextOut), inVexID(eVex), nextInArc(nextIn){}
};

class AMLGraph : public AbstractGraph{
private:
    vector<AMLVex> outVexList;
    vector<AMLVex> inVexList;

public:
    AMLGraph(int _type = DG) : AbstractGraph(_type){}
    /* Insert */
    void AddVex(MyGraphicsVexItem *gvex);
    void AddArc(MyGraphicsLineItem *garc);

    /* Delete */
    void DelVex(MyGraphicsVexItem *gvex);
    void DelVex(int vexID);
    void DelArc(MyGraphicsLineItem *garc);
    void DelArc(int sVexID, int eVexID);

    /* Find */
    int GetIdOf(MyGraphicsVexItem *gvex);

    void DFS(int strtID){}
    void DFS(MyGraphicsVexItem *strtVex){DFS(GetIdOf(strtVex));}

    int Type() const { return type; }
};

#endif // GRAPH_IMPLEMENT_H
