#pragma once
#include <vector>
#include <QString>

class W_Edge;//forward

class Vertex {
public:
	double x, y, z;
	W_Edge* edge; //vychadzajuca hrana z tohto bodu

	Vertex(double xx, double yy, double zz, W_Edge* aedge) : x(xx), y(yy), z(zz), edge(aedge) {}
};

class Face {
public:
	W_Edge* edge; // jedna hrana plochy
	Face(W_Edge* aedge) :edge(aedge){}
};

class W_Edge {
public:
	Vertex* vert_origin, * vert_destination;
	Face* face_left, *face_right;
	W_Edge* edge_left_prev,* edge_left_next;
	W_Edge* edge_right_prev, * edge_right_next;

	W_Edge(Vertex* origin, Vertex* destination) : vert_origin(origin), vert_destination(destination), face_left(nullptr), face_right(nullptr),
		edge_left_prev(nullptr), edge_left_next(nullptr), edge_right_prev(nullptr), edge_right_next(nullptr) {}

};

class WingedEdge
{
private:
	void connect();

public:
	std::vector<Vertex*> vertices;
	std::vector<Face*> faces;
	std::vector<W_Edge*> edges;

	~WingedEdge();

	void createCube(double size);
	void clearCube();
	void saveToVTK(const QString& filename);
	void loadFromVTK(const QString& filename);
	void draw();



};

