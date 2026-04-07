#include "WingedEdge.h"

void WingedEdge::connect() 
{
    //prepojenie trojuholnikov (right)
    for (W_Edge* e : edges) {
        for (W_Edge* other : edges) {
            if (e != other && e->vert_origin == other->vert_destination && e->vert_destination == other->vert_origin) {
                e->face_right = other->face_left;
                other->face_right = e->face_left;

                e->edge_right_next = other->edge_left_next;
                e->edge_right_prev = other->edge_left_prev;
                other->edge_right_next = e->edge_left_next;
                other->edge_right_prev = e->edge_left_prev;
                break;
            }
        }
    }

    //pre kazdy vrch hrana, kt. z neho vychadza
    for (Vertex* v : vertices) {
        for (W_Edge* e : edges) {
            if (e->vert_origin == v) {
                v->edge = e;
                break;
            }
        }
    }
}

WingedEdge::~WingedEdge()
{
    for (Vertex* v : vertices) delete v;
    for (Face* f : faces) delete f;
    for (W_Edge* e : edges) delete e;
}

void WingedEdge::createCube(double size)
{
    //vycistenie
    for (Vertex* v : vertices) delete v;
    for (Face* f : faces) delete f;
    for (W_Edge* e : edges) delete e;
    vertices.clear();
    faces.clear();
    edges.clear();

    double s = size / 2.0; //kocka centrovana v (0,0,0) -> polka nalavo polka napravo

    //vrcholy
    vertices.push_back(new Vertex(-s, -s, -s, nullptr));
    vertices.push_back(new Vertex(s, -s, -s, nullptr));
    vertices.push_back(new Vertex(s, -s, s, nullptr));
    vertices.push_back(new Vertex(-s, -s, s, nullptr));
    vertices.push_back(new Vertex(-s, s, -s, nullptr));
    vertices.push_back(new Vertex(s, s, -s, nullptr));
    vertices.push_back(new Vertex(s, s, s, nullptr));
    vertices.push_back(new Vertex(-s, s, s, nullptr));

    //funkciu pudhback som si nasla na internete, je z vector, prida prvok nakoniec a resizne vektor

    //rozdelenie na trojuholníky

    int tris[12][3] = {
        {0,1,2},{0,2,3},{1,5,6},{1,6,2},{5,4,7},{5,7,6},{4,0,3},{4,3,7},{0,1,5},{0,5,4},{3,2,6},{3,6,7}
    };

    for (int i = 0; i < 12; i++) {
        Vertex* v1 = vertices[tris[i][0]];
        Vertex* v2 = vertices[tris[i][1]];
        Vertex* v3 = vertices[tris[i][2]];
        //uloží vrchy trijuholníka

        W_Edge* e1 = new W_Edge(v1, v2);
        W_Edge* e2 = new W_Edge(v2, v3);
        W_Edge* e3 = new W_Edge(v3, v1);
        //hrany medzi vrchmi

        edges.push_back(e1);
        edges.push_back(e2);
        edges.push_back(e3);

        Face* face = new Face(e1);
        faces.push_back(face);
        //jedna ploska na hrane e1 ktora ma vrchy v1 a v2

        e1->face_left = face;
        e2->face_left = face;
        e3->face_left = face;
        //vsetky hrany maju ako lavu (vnutornu) plosku face

        e1->edge_left_next = e2;
        e1->edge_left_prev = e3;
        e2->edge_left_next = e3;
        e2->edge_left_prev = e1;
        e3->edge_left_next = e1;
        e3->edge_left_prev = e2;


    }

    connect();
}
