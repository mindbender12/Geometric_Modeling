#pragma once

using namespace std;

struct Viewport {
	float xp, yp, wp, hp;
	GLint x,y,w,h;

	Viewport(float xp, float yp, float wp, float hp){
		operator()(xp,yp,wp,hp);
	}

	void operator()(float xp, float yp, float wp, float hp){
		this->xp = xp; this->yp = yp; this->wp = wp; this->hp = hp;
	}

	void enable(){
		push(x,y,w,h);
	}

	void disable(){
		pop();
	}

	//called by the ViewportManager
	virtual void resize(int _w, int _h){
		x = _w * xp;
		y = _h * yp;
		w = _w * wp;
		h = _h * hp;
	}

	virtual void draw()=0;
	virtual void mouseButton(int button, int state, int x, int y)=0;
	virtual void mouseMove(int x, int y)=0;
	virtual void passiveMouseMove(int x, int y)=0;
	virtual void keyDown (unsigned char key, int x, int y)=0;
	virtual void keyUp(unsigned char key, int x, int y)=0;

	static void push(GLint x, GLint y, GLsizei width, GLsizei height){

		glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_VIEWPORT_BIT | GL_SCISSOR_BIT | GL_POLYGON_BIT);

		glViewport(x,y,width,height);
		glScissor(x,y,width,height);
	}

	static void pop(){
		//printGLErrors("pop start");
		glPopAttrib();
		//printGLErrors("pop end");
	}
};

struct ViewportManager {
	vector<Viewport*> viewports;
	int w, h;

	void operator()(){
		glEnable(GL_SCISSOR_TEST);
	}

	void add(Viewport* viewport){
		viewports.push_back(viewport);
		viewport->resize(w, h);
	}

	void resize(int width, int height){
		w = width;
		h = height;

		for (size_t i=0; i < viewports.size(); i++)
			viewports[i]->resize(w,h);
	}

	void draw(){
		for (size_t i=0; i < viewports.size(); i++){
			Viewport* v = viewports[i];
			v->enable();
			v->draw();
			v->disable();
		}
	}

	bool inRegion(int x, int y, const Viewport& v, int& tx, int& ty){
		if (x >= v.x && x < (v.x+v.w) && y >= v.y && y < (v.y+v.h)){
			tx = x - v.x;
			ty = y - v.y;
			return true;
		}
		return false;
	}

	void mouseButton(int button, int state, int x, int y){
		y = h - y;
		for (size_t i=0; i < viewports.size(); i++){
			Viewport* v = viewports[i];
			int tx,ty;
			if (inRegion(x, y, *v, tx, ty))
				v->mouseButton(button, state, tx, ty);
		}
	}

	void mouseMove(int x, int y){
		y = h - y;
		for (size_t i=0; i < viewports.size(); i++){
			Viewport* v = viewports[i];
			int tx,ty;
			if (inRegion(x, y, *v, tx, ty)){
				v->enable();
				v->mouseMove(tx, ty);
				v->disable();
			}
		}
	}
	void passiveMouseMove(int x, int y){
		y = h - y;
		for (size_t i=0; i < viewports.size(); i++){
			Viewport* v = viewports[i];
			int tx,ty;
			if (inRegion(x, y, *v, tx, ty)){
				v->enable();
				v->passiveMouseMove(tx, ty);
				v->disable();
			}
		}
	}
	void keyDown (unsigned char key, int x, int y){
		y = h - y;
		for (size_t i=0; i < viewports.size(); i++){
			Viewport* v = viewports[i];
			int tx,ty;
			if (inRegion(x, y, *v, tx, ty)){
				v->enable();
				v->keyDown(key, tx, ty);
				v->disable();
			}
		}
	}
	void keyUp(unsigned char key, int x, int y){
		y = h - y;
		for (size_t i=0; i < viewports.size(); i++){
			Viewport* v = viewports[i];
			int tx,ty;
			if (inRegion(x, y, *v, tx, ty)){
				v->enable();
				v->keyUp(key, tx, ty);
				v->disable();
			}
		}
	}
};