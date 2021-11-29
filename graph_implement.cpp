#include "graph_implement.h"
#include <QDebug>

void ALVex::visit(){
    if(visited)
        return;
    info->gVex->visit(true);
    visited = true;
}

void ALVex::access(const QString &hint){
    info->gVex->access(hint);
}

void ALArc::visit(){
    gArc->visit(true);
}

void ALArc::access(){
    gArc->access();
}

void ALGraph::AddVex(MyGraphicsVexItem *gvex){
    ALVex newVex(gvex);
    vexList.push_back(newVex);
}

void ALGraph::AddArc(MyGraphicsLineItem *garc){
    int strtVex = GetIdOf(garc->stVex());
    int endVex = GetIdOf(garc->edVex());

    ALArc *temp = vexList[strtVex].firstArc;
    ALArc *newArc = new ALArc(garc, endVex, temp);
    vexList[strtVex].firstArc = newArc;

    if(type == UDG){
        temp = vexList[endVex].firstArc;
        newArc = new ALArc(garc, strtVex, temp);
        vexList[endVex].firstArc = newArc;
    }
}

void ALGraph::DelVex(MyGraphicsVexItem *gvex){
    int vexID = GetIdOf(gvex);
    DelVex(vexID);
}

void ALGraph::DelVex(int vexID){
    //Delete out arc
    ALArc *curArc = vexList[vexID].firstArc;
    while(curArc != nullptr){
        ALArc *next = curArc->nextArc;
        delete curArc;
        curArc = next;
    }
    //Delete in arc and adjust arcs
    for(int i = 0; i < vexList.size(); i++){
        if(i == vexID)  continue;
        ALArc *dummyHead = new ALArc(nullptr, 0, vexList[i].firstArc);
        ALArc *preArc = dummyHead;
        while(preArc->nextArc != nullptr){
            if(preArc->nextArc->eVexID == vexID){
                ALArc *next = preArc->nextArc;
                preArc->nextArc = next->nextArc;
                delete next;
                continue;
            }
            if(preArc->nextArc->eVexID > vexID)
                preArc->nextArc->eVexID--;
            preArc = preArc->nextArc;
        }
        vexList[i].firstArc = dummyHead->nextArc;
        delete dummyHead;
    }
    vexList.erase(vexList.begin() + vexID);
}

void ALGraph::DelArc(MyGraphicsLineItem *garc){
    int sVex = GetIdOf(garc->stVex());
    int eVex = GetIdOf(garc->edVex());
    DelArc(sVex, eVex);
}

void ALGraph::DelArc(int sVexID, int eVexID){
    //Delete sVex -> eVex
    if(vexList[sVexID].firstArc != nullptr){
        if(vexList[sVexID].firstArc->eVexID == eVexID){
            ALArc *awaitDel = vexList[sVexID].firstArc;
            vexList[sVexID].firstArc = awaitDel->nextArc;
            delete awaitDel;
        }
        else{
            ALArc *preArc = vexList[sVexID].firstArc;
            while(preArc->nextArc != nullptr && preArc->nextArc->eVexID != eVexID)
                preArc = preArc->nextArc;
            if(preArc->nextArc != nullptr){
                ALArc *awaitDel = preArc->nextArc;
                preArc->nextArc = awaitDel->nextArc;
                delete awaitDel;
            }
        }
    }
    //Delete eVex -> sVex
    if(type == UDG && vexList[eVexID].firstArc != nullptr){
        if(vexList[eVexID].firstArc->eVexID == sVexID){
            ALArc *awaitDel = vexList[eVexID].firstArc;
            vexList[eVexID].firstArc = awaitDel->nextArc;
            delete awaitDel;
        }
        else{
            ALArc *preArc = vexList[eVexID].firstArc;
            while(preArc->nextArc != nullptr && preArc->nextArc->eVexID != sVexID)
                preArc = preArc->nextArc;
            if(preArc->nextArc != nullptr){
                ALArc *awaitDel = preArc->nextArc;
                preArc->nextArc = awaitDel->nextArc;
                delete awaitDel;
            }
        }
    }
}

int ALGraph::GetIdOf(MyGraphicsVexItem *gvex){
    int i = 0;
    while(i < vexList.size() && !vexList[i].info->gVex->equalTo(gvex))
        i++;
    return i == vexList.size() ? -1 : i;
}

ALArc* ALGraph::FindArc(int sID, int eID){
    if(sID < 0 || sID >= vexList.size())
        return nullptr;
    ALArc *p = vexList[sID].firstArc;
    while(p != nullptr){
        if(p->eVexID == eID)
            return p;
        p = p->nextArc;
    }
    return nullptr;
}

void ALGraph::SetWeight(MyGraphicsLineItem *garc, int weight){
    int strtVex = GetIdOf(garc->stVex());
    int endVex = GetIdOf(garc->edVex());
    ALArc *p = vexList[strtVex].firstArc;
    while(p != nullptr){
        if(p->eVexID == endVex){
            p->weight = weight;
            p->gArc->setText(QString::asprintf("%d", weight));
        }
        p = p->nextArc;
    }
    if(type == UDG){
        p = vexList[endVex].firstArc;
        while(p != nullptr){
            if(p->eVexID == strtVex){
                p->weight = weight;
            }
            p = p->nextArc;
        }
    }
}

void ALGraph::ClearVisit(){
    for(int i = 0; i < vexList.size(); i++){
        vexList[i].visited = false;
        vexList[i].info->gVex->visit(false);
    }
}

void ALGraph::ResetDistance(){
    for(int i = 0; i < vexList.size(); i++){
        vexList[i].info->strtVexInfo = nullptr;
        vexList[i].info->distance = VexInfo::INF;
        vexList[i].info->preVexID = -1;
        vexList[i].info->gVex->access("", false);
    }
}

void ALGraph::DFS(int strtID){
    if(strtID == -1)
        return;
    vector<int> awaitVexList;
    vector<ALArc*> awaitArcList;
    awaitVexList.push_back(strtID);
    while(awaitVexList.size() > 0){
        int nextVex = awaitVexList.back();
        ALArc *nextArc = awaitArcList.size() > 0 ? awaitArcList.back() : nullptr;
        awaitVexList.pop_back();
        if(nextArc)
            awaitArcList.pop_back();
        for(ALArc *p = vexList[nextVex].firstArc; p != nullptr; p = p->nextArc){
            if(vexList[p->eVexID].visited == false){
                awaitVexList.push_back(p->eVexID);
                awaitArcList.push_back(p);
                if(type == UDG && GetIdOf(p->gArc->edVex()) != p->eVexID)
                    p->gArc->reverseDirection();
            }
        }
        if(nextArc && !vexList[nextArc->eVexID].visited)
            nextArc->visit();
        vexList[nextVex].visit();
    }
}

void ALGraph::BFS(int strtID){
    if(strtID == -1)
        return;
    vector<int> awaitVexList;
    vector<ALArc*> awaitArcList;
    awaitVexList.push_back(strtID);
    while(awaitVexList.size() > 0){
        int nextVex = awaitVexList[0];
        ALArc *nextArc = awaitArcList.size() > 0 ? awaitArcList[0] : nullptr;
        awaitVexList.erase(awaitVexList.begin());
        if(nextArc)
            awaitArcList.erase(awaitArcList.begin());
        for(ALArc *p = vexList[nextVex].firstArc; p != nullptr; p = p->nextArc){
            if(vexList[p->eVexID].visited == false){
                awaitVexList.push_back(p->eVexID);
                awaitArcList.push_back(p);
                if(type == UDG && GetIdOf(p->gArc->edVex()) != p->eVexID)
                    p->gArc->reverseDirection();
            }
        }
        if(nextArc && !vexList[nextArc->eVexID].visited)
            nextArc->visit();
        vexList[nextVex].visit();
    }
}

void ALGraph::Dijkstra(int strtID){
    //Clear previous result
    ClearVisit();
    ResetDistance();
    qDebug() << "strt :" << strtID;
    //Set start vex info to all vertexes
    for(int i = 0; i < vexList.size(); i++)
        vexList[i].info->strtVexInfo = vexList[strtID].info;
    //Start dijkstra
    vexList[strtID].info->distance = 0;
    vexList[strtID].access("strt");
    while(true){
        //Find next
        int minVexID = -1;
        for(int i = 0; i < vexList.size(); i++){
            qDebug() << "i = " << i << "dis:" << vexList[i].info->distance;
            if(vexList[i].visited || vexList[i].info->distance == VexInfo::INF)
                continue;
            if(minVexID == -1)
                minVexID = i;
            else if(vexList[i].info->distance < vexList[minVexID].info->distance)
                minVexID = i;
        }
        if(minVexID == -1)
            break;
        qDebug() << "minVex:" << minVexID;
        //Set visit to edge and vex
        ALArc *edge = FindArc(vexList[minVexID].info->preVexID, minVexID);
        if(edge){
            if(type == UDG && GetIdOf(edge->gArc->edVex()) != minVexID)
                edge->gArc->reverseDirection();
            edge->visit();
            qDebug() << "visited edge";
        }
        vexList[minVexID].visit();
        qDebug() << "visited vex";
        //Find adjacent
        for(ALArc *p = vexList[minVexID].firstArc; p != nullptr; p = p->nextArc){
            if(!vexList[p->eVexID].visited){
                if(GetIdOf(p->gArc->edVex()) != p->eVexID)
                    p->gArc->reverseDirection();
                p->access();
                if(vexList[p->eVexID].info->distance == VexInfo::INF ||
                        vexList[p->eVexID].info->distance > vexList[minVexID].info->distance + p->weight){
                    vexList[p->eVexID].info->preVexID = minVexID;
                    vexList[p->eVexID].info->distance = vexList[minVexID].info->distance + p->weight;
                    vexList[p->eVexID].access(QString::asprintf("%d", vexList[p->eVexID].info->distance));
                }
            }
        }
    }
}

/**************************************************************************************/

void AMLGraph::AddVex(MyGraphicsVexItem *gvex){
    AMLVex newVex(gvex);
    outVexList.push_back(newVex);

    if(type == DG){
        inVexList.push_back(newVex);
    }
}

void AMLGraph::AddArc(MyGraphicsLineItem *garc){
    int strtVex = GetIdOf(garc->stVex());
    int endVex = GetIdOf(garc->edVex());

    AMLArc *nextOutArc = outVexList[strtVex].firstArc;
    AMLArc *nextInArc = type == DG ? inVexList[endVex].firstArc : outVexList[endVex].firstArc;
    AMLArc *newArc = new AMLArc(garc, strtVex, endVex, nextOutArc, nextInArc);
    outVexList[strtVex].firstArc = newArc;
    if(type == DG)
        inVexList[endVex].firstArc = newArc;
    else
        outVexList[endVex].firstArc = newArc;
}

void AMLGraph::DelVex(MyGraphicsVexItem *gvex){
    int vexID = GetIdOf(gvex);
    DelVex(vexID);
}

void AMLGraph::DelVex(int vexID){
    if(type == DG){
        //Delete out arc
        AMLArc *outArc = outVexList[vexID].firstArc;
        while(outArc != nullptr){
            AMLArc *dummyHead = new AMLArc(nullptr, 0, 0, nullptr, inVexList[outArc->inVexID].firstArc);
            AMLArc *preInArc = dummyHead;
            while(preInArc->nextInArc != nullptr){
                if(preInArc->nextInArc->outVexID == vexID){
                    AMLArc *next = preInArc->nextInArc;
                    preInArc->nextInArc = next->nextInArc;
                    delete next;
                }
                else
                    preInArc = preInArc->nextInArc;
            }
            inVexList[outArc->inVexID].firstArc = dummyHead->nextInArc;
            delete dummyHead;
            outArc = outArc->nextOutArc;
        }
        //Delete in arc and adjust ID
        for(int i = 0; i < outVexList.size(); i++){
            if(i == vexID)  continue;
            AMLArc *dummyHead = new AMLArc(nullptr, 0, 0, outVexList[i].firstArc, nullptr);
            AMLArc *preOutArc = dummyHead;
            while(preOutArc->nextOutArc != nullptr){
                if(preOutArc->nextOutArc->inVexID == vexID){
                    AMLArc *next = preOutArc->nextOutArc;
                    preOutArc->nextOutArc = next->nextOutArc;
                    delete next;
                    continue;
                }
                if(preOutArc->nextOutArc->inVexID > vexID)
                    preOutArc->nextOutArc->inVexID--;
                if(preOutArc->nextOutArc->outVexID > vexID)
                    preOutArc->nextOutArc->outVexID--;
                preOutArc = preOutArc->nextOutArc;
            }
            outVexList[i].firstArc = dummyHead->nextOutArc;
            delete dummyHead;
        }
        outVexList.erase(outVexList.begin() + vexID);
        inVexList.erase(inVexList.begin() + vexID);
    }
    else{
        //Traverse all and adjust ID
        for(int i = 0; i < outVexList.size(); i++){
            AMLArc *dummyHead = new AMLArc(nullptr, i, -1, outVexList[i].firstArc, nullptr);
            AMLArc *preArc = dummyHead;
            AMLArc *nextArc = preArc->nextOutArc;
            while(nextArc != nullptr){
                if(nextArc->inVexID == vexID || nextArc->outVexID == vexID){
                    if(preArc->outVexID == i){
                        if(nextArc->outVexID == i){
                            preArc->nextOutArc = nextArc->nextOutArc;
                            nextArc->nextOutArc = nullptr;
                        }
                        else{
                            preArc->nextOutArc = nextArc->nextInArc;
                            nextArc->nextInArc = nullptr;
                        }
                    }
                    else{
                        if(nextArc->outVexID == i){
                            preArc->nextInArc = nextArc->nextOutArc;
                            nextArc->nextOutArc = nullptr;
                        }
                        else{
                            preArc->nextInArc = nextArc->nextInArc;
                            nextArc->nextInArc = nullptr;
                        }
                    }
                    if(nextArc->nextOutArc == nullptr && nextArc->nextInArc == nullptr)
                        delete nextArc;
                    nextArc = preArc->outVexID == i ? preArc->nextOutArc : preArc->nextInArc;
                    continue;
                }
                preArc = nextArc;
                nextArc = preArc->outVexID == i ? preArc->nextOutArc : preArc->nextInArc;
                if(preArc->inVexID > vexID)
                    preArc->inVexID++;
                if(preArc->outVexID > vexID)
                    preArc->outVexID++;
            }
            delete dummyHead;
        }
        //Delete vexID
        outVexList.erase(outVexList.begin() + vexID);
    }
}

void AMLGraph::DelArc(MyGraphicsLineItem *garc){
    int sVex = GetIdOf(garc->stVex());
    int eVex = GetIdOf(garc->edVex());
    DelArc(sVex, eVex);
}

void AMLGraph::DelArc(int sVexID, int eVexID){
    if(type == DG){
        AMLArc *dummyHead;
        AMLArc *preArc;
        //Delete sVex -> eVex
        dummyHead = new AMLArc(nullptr, sVexID, -1, outVexList[sVexID].firstArc, nullptr);
        preArc = dummyHead;
        while(preArc->nextOutArc != nullptr){
            if(preArc->nextOutArc->inVexID == eVexID){
                preArc->nextOutArc = preArc->nextOutArc->nextOutArc;
                continue;
            }
            else
                preArc = preArc->nextOutArc;
        }
        delete dummyHead;
        //Delete eVex -> sVex
        dummyHead = new AMLArc(nullptr, -1, eVexID, nullptr, inVexList[eVexID].firstArc);
        preArc = dummyHead;
        while(preArc->nextInArc != nullptr){
            if(preArc->nextInArc->outVexID == sVexID){
                AMLArc *awaitDel = preArc->nextInArc;
                preArc->nextInArc = preArc->nextInArc->nextInArc;
                delete awaitDel;
                continue;
            }
            else
                preArc = preArc->nextInArc;
        }
        delete dummyHead;
    }
    else{
        AMLArc *dummyHead;
        AMLArc *preArc;
        AMLArc *nextArc;
        //Delete sVex -> eVex
        dummyHead = new AMLArc(nullptr, sVexID, -1, outVexList[sVexID].firstArc, nullptr);
        preArc = dummyHead;
        nextArc = outVexList[sVexID].firstArc;
        while(nextArc != nullptr){
            if(nextArc->inVexID == eVexID || nextArc->outVexID == eVexID){
                if(preArc->outVexID == sVexID){
                    if(nextArc->outVexID == sVexID){
                        preArc->nextOutArc = nextArc->nextOutArc;
                        nextArc->nextOutArc = nullptr;
                    }
                    else{
                        preArc->nextOutArc = nextArc->nextInArc;
                        nextArc->nextInArc = nullptr;
                    }
                }
                else{
                    if(nextArc->outVexID == sVexID){
                        preArc->nextInArc = nextArc->nextOutArc;
                        nextArc->nextOutArc = nullptr;
                    }
                    else{
                        preArc->nextInArc = nextArc->nextInArc;
                        nextArc->nextInArc = nullptr;
                    }
                }
                nextArc = preArc->outVexID == sVexID ? preArc->nextOutArc : preArc->nextInArc;
                continue;
            }
            else{
                preArc = nextArc;
                nextArc = nextArc = preArc->outVexID == sVexID ? preArc->nextOutArc : preArc->nextInArc;
            }
        }
        delete dummyHead;
        //Delete eVex -> sVex and release arc
        dummyHead = new AMLArc(nullptr, -1, eVexID, outVexList[eVexID].firstArc, nullptr);
        preArc = dummyHead;
        nextArc = outVexList[eVexID].firstArc;
        while(nextArc != nullptr){
            if(nextArc->inVexID == sVexID || nextArc->outVexID == sVexID){
                if(preArc->outVexID == eVexID){
                    if(nextArc->outVexID == eVexID){
                        preArc->nextOutArc = nextArc->nextOutArc;
                        delete nextArc;
                    }
                    else{
                        preArc->nextOutArc = nextArc->nextInArc;
                        delete nextArc;
                    }
                }
                else{
                    if(nextArc->outVexID == eVexID){
                        preArc->nextInArc = nextArc->nextOutArc;
                        delete nextArc;
                    }
                    else{
                        preArc->nextInArc = nextArc->nextInArc;
                        delete nextArc;
                    }
                }
                nextArc = preArc->outVexID == eVexID ? preArc->nextOutArc : preArc->nextInArc;
                continue;
            }
            else{
                preArc = nextArc;
                nextArc = nextArc = preArc->outVexID == eVexID ? preArc->nextOutArc : preArc->nextInArc;
            }
        }
        delete dummyHead;
    }
}

int AMLGraph::GetIdOf(MyGraphicsVexItem *gvex){
    int i = 0;
    while(i < outVexList.size() && outVexList[i].info->gVex != gvex)
        i++;
    return i == outVexList.size() ? -1 : i;
}
