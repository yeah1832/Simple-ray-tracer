#include "raytrace.h"
#include <math.h>

using namespace raytraceData;

raytrace::raytrace(int w, int h)
{
	/* low-level graphics setup */
	lowlevel.initCanvas(w, h);

	/* raytracer setup */
	initCamera(w, h);
	initScene();
}

raytrace::~raytrace()
{
	if (s1 != NULL) delete s1;
	if (cy != NULL) delete cy;
	if (viewpoint != NULL) delete viewpoint;
}

GLubyte* raytrace::display(void)
{	
	drawScene();  /* draws the picture in the canvas */

	return lowlevel.flushCanvas();  /* draw the canvas to the OpenGL window */	
}

void raytrace::initScene()
{
	//��ü ���� �� �� ����
	s1 = makeSphere(0.0, 0.0, -3.0, 0.25);
	s1->m = shader.makeMaterial(0.8, 0.1, 0.15, 0.3, 0.7,0.7);
	cy = makeCylinder(0.30, -0.25, -1.8, 0.0, 0.0, 0.0, 0.08, 0.35);
	cy->m = shader.makeMaterial(0.3, 0.8, 0.15, 0.3, 0.7,1.0);

	tracer.s1 = s1;
	tracer.cy = cy;
}

void raytrace::initCamera(int w, int h)
{
	viewpoint = makePoint(0.0, 0.0, 0.0, 1.0);
	pnear = -1.0;
	fovx = M_PI / 6;
	/* window dimensions */
	width = w;
	height = h;
}

void raytrace::drawScene(void)
{
	int i, j;
	GLfloat imageWidth;

	/* declare data structures on stack to avoid malloc & free */
	point worldPix;  /* current pixel in world coordinates */
	point direction;
	ray r;
	color c;

	/* z and w are constant for all pixels */
	worldPix.w = 1.0;
	worldPix.z = pnear;

	r.start = &worldPix;
	r.end = &direction;

	imageWidth = 2 * pnear * tan(fovx / 2);

	/* trace a ray for every pixel */
	for (i = 0; i < width; i++) {
		for (j = 0; j < height; j++) {

			/* find position of pixel in world coordinates */
			/* y position = (pixel height/middle) scaled to world coords */
			worldPix.y = (j - (height / 2)) * imageWidth / width;
			/* x position = (pixel width/middle) scaled to world coords */
			worldPix.x = (i - (width / 2)) * imageWidth / width;

			/* find direction */
			/* note: direction vector is NOT NORMALIZED */
			calculateDirection(viewpoint, &worldPix, &direction);

			/* Find color for pixel */
			rayColor(&r, &c);
			/* write the pixel! */
			lowlevel.drawPixel(i, j, c.r, c.g, c.b);
		}
	}
}

point* raytrace::makePoint(GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
	point* p;
	/* allocate memory */	
	p = new point();
	/* put stuff in it */
	p->x = x; p->y = y; p->z = z; p->w = w;
	return (p);
}

cylinder* raytrace::makeCylinder(GLfloat x, GLfloat y, GLfloat z, GLfloat xr, GLfloat yr, GLfloat zr, GLfloat daR, GLfloat h) {
	cylinder* co;
	co = new cylinder();
	co->c = makePoint(x, y, z, 1);
	co->n = new vector;
	co->n->x = xr;
	co->n->y = yr;
	co->n->z = zr;
	co->n->w = 0;
	float length = co->n->x * co->n->x + co->n->y * co->n->y + co->n->z * co->n->z;
	length = sqrt(length);
	co->n->x /= length;
	co->n->y /= length;
	co->n->z /= length;
	co->r = daR;
	co->h = h;

	return(co);
}

sphere* raytrace::makeSphere(GLfloat x, GLfloat y, GLfloat z, GLfloat r) {
	sphere* s;
	/* allocate memory */
	s = new sphere();

	/* put stuff in it */
	s->c = makePoint(x, y, z, 1.0);   /* center */
	s->r = r;   /* radius */
	s->m = NULL;   /* material */
	return(s);
}

/* returns the color seen by ray r in parameter c */
void raytrace::rayColor(ray* r, color* c) {
	
	point p;  /* first intersection point */
	vector n;
	material* m;
	double t;
	point v;


	p.w = 0.0;  /* inialize to "no intersection" */
	tracer.trace(r, &p, &n, &m);
	if (p.w != 0.0) {
		shader.shade(&p, &n, m, c, r->end);  /* do the lighting calculations */
	}
	else {             /* nothing was hit */
		c->r = 0.0;
		c->g = 0.0;
		c->b = 0.0;
	}
}

/* vector from point p to point q is returned in v */
void raytrace::calculateDirection(point* p, point* q, point* v) {
	float length;

	v->x = q->x - p->x;
	v->y = q->y - p->y;
	v->z = q->z - p->z;
	/* a direction is a point at infinity */
	v->w = 0.0;
}
