#include "WingedEdge.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>

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
    clearCube();

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

    //funkciu pushback som si nasla na internete, je z vector, prida prvok nakoniec a resizne vektor

    //rozdelenie na trojuholníky

    int tris[12][3] = {
        {0,1,2},{0,2,3},{1,5,6},{1,6,2},{5,4,7},{5,7,6},{4,0,3},{4,3,7},{0,1,5},{0,5,4},{3,2,6},{3,6,7}
    };

    for (int i = 0; i < 12; i++) {
        addTriangle(tris[i][0], tris[i][1], tris[i][2]);
    }

    connect();
}

void WingedEdge::clearCube()
{
    for (Vertex* v : vertices) delete v;
    for (Face* f : faces) delete f;
    for (W_Edge* e : edges) delete e;
    vertices.clear();
    faces.clear();
    edges.clear();
}

void WingedEdge::saveToVTK(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    QTextStream out(&file);

    out << "# vtk DataFile Version 3.0\n";
    out << "WingedEdge Cube\n";
    out << "ASCII\n";
    out << "DATASET POLYDATA\n";

    //vrvholy
    out << "POINTS " << vertices.size() << " float\n";
    for (Vertex* v : vertices) {
        out << v->x << " " << v->y << " " << v->z << "\n";
    }

    //trojuholniky
    out << "POLYGONS " << faces.size() << " " << faces.size() * 4 << "\n"; //kolko je troj., kolko cisel tam je (1 troj su 4 cisla)

    for (Face* f : faces) {
        W_Edge* start = f->edge; //start=e1
        W_Edge* e = start; // aktuálna hrana (teraz e)
        int indices[3];
        int idx = 0;

        do {
            for (int i = 0; i < vertices.size(); i++) {
                if (vertices[i] == e->vert_origin) { //hladame suradnice zaciatocneho bodu e
                    indices[idx] = i;
                    idx++;
                    break;
                }
            }

            e = e->edge_left_next; //presun na dalsiu hranu
        } while (e != start);

        out << "3 " << indices[0] << " " << indices[1] << " " << indices[2] << "\n";
    }
    file.close();
}


void WingedEdge::loadFromVTK(const QString& filename)
{
    clearCube();

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Nepodarilo sa otvorit subor";
        return;
    }

    QTextStream in(&file);

    
    for (int i = 0; i < 4; i++) {
        in.readLine(); //skip hlavicku
    }

   //vrcholy
    QString word;
    int numPoints;
    in >> word;  // "POINTS"
    in >> numPoints;  // 8
    in >> word;  // "float"

    //qDebug() << "Pocet vrcholov:" << numPoints;

    for (int i = 0; i < numPoints; i++) {
        double x, y, z;
        in >> x >> y >> z;
        vertices.push_back(new Vertex(x, y, z, nullptr));
       // qDebug() << "  Vertex" << i << ":" << x << y << z;
    }

    // trojuholniky
    in >> word;  // "POLYGONS"
    int numTriangles, totalNumbers;
    in >> numTriangles >> totalNumbers;

    //qDebug() << "Pocet trojuholnikov:" << numTriangles;

    for (int i = 0; i < numTriangles; i++) {
        int count, v1, v2, v3;
        in >> count >> v1 >> v2 >> v3;

        // qDebug() << "  Trojuholnik" << i << ":" << v1 << v2 << v3;

        if (count != 3) continue;

        addTriangle(v1, v2, v3);
    }

    file.close();
    connect();

   // qDebug() << "Nacitanie dokoncene. Vrcholy:" << vertices.size() << "Plochy:" << faces.size() << "Hrany:" << edges.size();

}

void WingedEdge::addTriangle(int i1, int i2, int i3)
{
    Vertex* v1 = vertices[i1];
    Vertex* v2 = vertices[i2];
    Vertex* v3 = vertices[i3];
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

void WingedEdge::createUVSphere(float radius, int parallels, int meridians)
{
    clearCube();

    //vrcholy
    vertices.push_back(new Vertex(0, radius, 0, nullptr)); //severny pol

    //rovnobezky
    for (int i = 0; i < parallels; i++) {
        double theta = M_PI * (i + 1) / (parallels + 1); 
        double y = radius * cos(theta);
        double r = radius * sin(theta); //polomer kruznice danej rovnobezky

        for (int j = 0; j < meridians; j++) { //prechadzam poludnikmi na kazdej rovnobezke
            double phi = 2 * M_PI * j / meridians; 
            double x = r * cos(phi);
            double z = r * sin(phi);
            vertices.push_back(new Vertex(x, y, z, nullptr)); //vytvorenie vrcholu 
        }
    }

    //juzny pol
    int southPoleIndex = vertices.size();
    vertices.push_back(new Vertex(0, -radius, 0, nullptr));

    //trojuholniky
    //medzi severnym polom a prvu rovnobezkou
    int firstRingStart = 1;
    for (int j = 0; j < meridians; j++) {
        addTriangle(0, firstRingStart + j, firstRingStart + (j + 1) % meridians); 
    }

    //stred
    for (int i = 0; i < parallels - 1; i++) {
        int ringStart = 1 + i * meridians;
        int nextRingStart = 1 + (i + 1) * meridians;
        for (int j = 0; j < meridians; j++) {
            int j1 = ringStart + j; //hore vlavo
            int j2 = ringStart + (j + 1) % meridians; //hore vpravo
            int k1 = nextRingStart + j; //dple vlavo
            int k2 = nextRingStart + (j + 1) % meridians; //dole vpravo
            addTriangle(j1, j2, k2);
            addTriangle(j1, k2, k1);
        }
    }

    // medzi juznym a poslednou rovnpb.
    int lastRingStart = 1 + (parallels - 1) * meridians;
    for (int j = 0; j < meridians; j++) {
        addTriangle(lastRingStart + j, lastRingStart + (j + 1) % meridians, southPoleIndex);
    }

    connect();

}



