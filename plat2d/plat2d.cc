#include "plat2d.hpp"
#include "../utils/utils.hpp"
#include "../structs/binheap.hpp"

const unsigned int Plat2d::Ops[] = {
	Player::Left,
	Player::Right,
	Player::Jump,
	Player::Left | Player::Jump,
	Player::Right | Player::Jump,
};

const unsigned int Plat2d::Nops = sizeof(Plat2d::Ops) / sizeof(Plat2d::Ops[0]);

Plat2d::Plat2d(FILE *in) : lvl(in) {
	gx = lvl.width();
	gy = lvl.height();
	for (unsigned int x = 0; x < lvl.width(); x++) {
	for (unsigned int y = 0; y < lvl.height(); y++) {
		if (!(lvl.at(x, y).tile.flags & Tile::Down))
			continue;
		if (gx < lvl.width() || gy < lvl.height())
			fatal("There are multiple goal locations in the level");
		gx = x;
		gy = y;
	}
	}
	if (gx >= lvl.width() || gy >= lvl.height())
		fatal("No goal location in the level");

	gleft = gx * W;
	gright = (gx + 1) * W;
	gtop = gy * H;
	gbottom = (gy + 1) * H;

	initvg();
}

Plat2d::~Plat2d(void) {
	delete vg;
}

Plat2d::State Plat2d::initialstate(void) {
	return State(2 * Tile::Width + Player::Offx, 2 * Tile::Height + Player::Offy,
		0, Player::Width, Player::Height);
}

void Plat2d::initvg(void) {
	double strt = walltime();
	bool *blkd = new bool[lvl.width() * lvl.height()];
	for (unsigned int i = 0; i < lvl.width() * lvl.height(); i++)
		blkd[i] = false;
	for (unsigned int i = 0; i < lvl.width(); i++) {
	for (unsigned int j = 0; j < lvl.height(); j++)
		blkd[i * lvl.height() + j] = lvl.blocked(i, j);
	}

	vg = new VisGraph(PolyMap(blkd, lvl.width(), lvl.height()));
	delete[]blkd;

	for (unsigned int i = 0; i < lvl.width() * lvl.height(); i++) {
		unsigned int x = i / lvl.height();
		unsigned int y = i % lvl.height();
		geom2d::Pt pt(x + 0.5, y + 0.5);
		if (!vg->map.obstructed(pt))
			centers.push_back(vg->add(pt));
		else
			centers.push_back(-1);
	}

	vg->scale(W, H);

	togoal.resize(vg->verts.size());
	for (unsigned int i = 0; i < vg->verts.size(); i++) {
		togoal[i].d = geom2d::Infinity;
		togoal[i].i = -1;
		togoal[i].v = i;
		togoal[i].prev = -1;
	}

	gcenter = centers[gx * lvl.height() + gy];
	assert (gcenter >= 0);
	togoal[gcenter].d = 0;

	BinHeap<Node, Node*> open;
	open.push(&togoal[gcenter]);

	while (!open.empty()) {
		const Node *n = *open.pop();
		const std::vector<VisGraph::Edge> &es = vg->verts[n->v].edges;
		for (unsigned int i = 0; i < es.size(); i++) {
			const VisGraph::Edge &e = es[i];
			double d = n->d + e.dist;
			if (togoal[e.dst].d <= d)
				continue;
			togoal[e.dst].d = d;
			togoal[e.dst].prev = n->v;
			if (togoal[e.dst].i < 0)
				open.push(&togoal[e.dst]);
			else
				open.update(togoal[e.dst].i);
		}
	}
	drawmap("vismap.eps");
	dfpair(stdout, "visibility graph time", "%g", walltime() - strt);
}

void Plat2d::drawmap(const char *file) const {
	static const unsigned int Width = 400, Height = 400;

	VisGraph graph(*vg);
	geom2d::Pt min = graph.map.min(), max = graph.map.max();
	graph.translate(-min.x, -min.y);
	double w = max.x - min.x, h = max.y - min.y;
	double s = Height / h;
	if (Width / w < s)
		s = Width / w;
	graph.scale(s, s);

	Image img(w * s, h * s);
	graph.map.draw(img, 1);

	int i = centers[2 * lvl.height() + 2];
	i = togoal[i].prev;
	geom2d::Pt p0 = graph.verts[i].pt;
	while (i >= 0) {
		const geom2d::Pt &p1 = graph.verts[i].pt;
		img.add(new Image::Line(p0, p1, Image::red, 1));
		p0 = p1;
		i = togoal[i].prev;
	}

	img.saveeps(file);
}

std::string controlstr(const std::vector<unsigned int> &controls) {
	std::string bytes;
	for (unsigned int i = 0; i < controls.size(); i++)
		bytes.push_back(controls[i] & 0xFF);
	return base64enc(runlenenc(bytes));
}

std::vector<unsigned int> controlvec(const std::string &enc) {
	std::string bytes = runlendec(base64dec(enc));
	std::vector<unsigned int> v;
	for (unsigned int i = 0; i < bytes.size(); i++)
		v.push_back(bytes[i]);
	return v;
}