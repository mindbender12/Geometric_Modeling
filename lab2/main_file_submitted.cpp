/* CSE 784 Lab2*/
/* Author: Soumya Dutta */
/* 1st Year PhD Student */
/* dutta.33@osu.edu */

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <cfloat>
#include <GL/glew.h>
#include <GL/glui.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp> 
#include <ctime>
#include <vector>
#include <glm/gtx/perpendicular.hpp>
#include "Viewport.h"

using namespace glm;
using namespace std;

#define SHOW_CTRLPOINTS_ID	100
#define SHOW_BEZIERWIRES_ID	101
#define SHOW_CTRLPOLYGON_ID 102
#define SHOW_SUBDIVISION_ID 103
#define USUBDIVI_ID 104
#define QSPLINE_ID 105
#define GENFREQ_ID 106
#define EXTRUDE_ID 107

//live vars
extern int curveType=0;
extern int iterations;
extern int mode;
extern int currentCurveType;
extern int pointStat=0;
extern int technique=0;

//Global Vars
int showCtrlPointsFlag=1; 
int showBezierWiresFlag=1;
int showCtrlPolygonFlag =1;
int showSubdivisionFlag=0;
int subdivi=1;
int genFreq=6;
int qspline=6;
uvec2 dim(800,800);
int main_window;
GLUI *glui1;
extern double PI = 3.14;
int extrusionLength=100;
int extrudeDepth=70;
bool isFirstDown = false;

void computePoints();
void clear (int);

// Class Declaration of Curve
class Curve : public Viewport 
{
public:

	int numOfPts;
	int subdivision;
	int j;
	int track;
	int countDrag;
	int count;	
	vector<vec2> listPts;
	int iterationNo;
	vec3 backgroundColor;
	vector<vec3> newList;
	vector<vec2> subDivNew;
	vector<vec2> subDivSpline;

	Curve(float x, float y, float w, float h, vec3 backgroundColor): Viewport(x,y,w,h), backgroundColor(backgroundColor)
	{
		numOfPts =0;
		subdivision =1;
		j = 0;
		track=0;
		countDrag=0;
		count=0;
		iterationNo=15;
	}	

	void mouseMove(int x, int y) 
	{
		if(pointStat==1)
		{
			vector<vec2>:: iterator it2;

			if(countDrag==0)
			{
				for(int p=0;p<listPts.size();p++)
				{
					if(fabs(listPts[p].x-x) <= 10 && fabs(listPts[p].y-y) <= 10)
					{
						track=p;
						countDrag++;
						break;
					}	
				}
			}
			else if(countDrag>=1)
			{
				it2 = listPts.insert(listPts.begin()+track+1,vec2(x,y));
				it2 = listPts.erase(listPts.begin()+track);
			}
		}
	}

	void passiveMouseMove(int x, int y) {};
	void keyDown (unsigned char key, int x, int y){};
	void keyUp(unsigned char key, int x, int y) {};

	void draw() {
		display();	
	};

	// This function takes a list of points and draws the dots in the screen
	void drawDot(vector<vec2> listPts)
	{
		glBegin(GL_POINTS);
		glColor3f(1.0,1.0,1.0);

		if(j==0) // J=0 means points are being added using add option, no insert after is currently being used
		{
			for(int i=0;i<listPts.size();i++)
			{
				glVertex2i(listPts[i].x,listPts[i].y);
			}
		}
		else if(j!=0)// this else part is for making selected points look red, as for this j!=0 
		{
			for(int i=0;i<listPts.size();i++)
			{
				if(i==j ) // make only the selected point red
				{
					glColor3f(1.0,0.0,0.0);
					glVertex2i(listPts[i].x,listPts[i].y);
				}
				else // rest all the points are same as before i.e. white
				{
					glColor3f(1.0,1.0,1.0);
					glVertex2i(listPts[i].x,listPts[i].y);
				}
			}
		}

		glEnd();
		glFlush();
	}


	//This function takes a list of points and draws lines between every two consecutive points 
	void drawLine(vector<vec2> listPts)
	{
		glBegin(GL_LINES);
		glColor3f(0.0,1.0,0.0);
		for(int i=0;i<listPts.size()-1;i++)
		{
			glVertex2f(listPts[i].x,listPts[i].y);
			glVertex2f(listPts[i+1].x,listPts[i+1].y);
		}	
		glEnd();
		glFlush();

	}

	//This function takes a list of points and draws Bezier curve with those set of points
	void drawCurves(vector<vec2> listPts)
	{

		glBegin(GL_LINES);
		glColor3f(0.0,0.0,1.0);
		if(showBezierWiresFlag)
		{
			for(int i=0;i<listPts.size()-1;i++)
			{
				glVertex2f(listPts[i].x,listPts[i].y);
				glVertex2f(listPts[i+1].x,listPts[i+1].y);
			}
		}	
		glEnd();
		glFlush();

	}

	//DeCasteljau's Recursive Algorithm
	vec2 deCasteljau(vector<vec2> newPts,float t)
	{
		if(newPts.size()==1)
		{
			return (vec2(newPts[0].x,newPts[0].y));
		}

		else
		{
			vector<vec2> temp;
			for(int i=0;i<newPts.size()-1;i++)
			{
				temp.push_back(newPts[i] + t*(newPts[i+1] - newPts[i]));
			}

			return(deCasteljau(temp,t));
		}	    
	}

	//OneSubdivision Function
	vector<vec2> oneSubdivision(vector<vec2> listPts,vector<vec2> poly1,vector<vec2> poly2,float u)
	{
		vector<vec2> returnVector;
		vector<vec2>::iterator it;

		if(listPts.size()==1)
		{	
			returnVector = poly1;
			returnVector.push_back(listPts[0]);
			returnVector.insert(returnVector.end(),poly2.begin(),poly2.end());
			return (returnVector);
		}

		else
		{
			vector<vec2> temp;
			poly1.push_back(listPts[0]);
			it = poly2.begin();
			it = poly2.insert(it,listPts[listPts.size()-1]);

			for(int i=0;i<listPts.size()-1;i++)
			{
				temp.push_back(listPts[i] + u*(listPts[i+1] - listPts[i]));
			}

			return (oneSubdivision(temp,poly1,poly2,u));

		}
	}

	//subDivide Function
	vector<vec2> subDivide(vector<vec2> listPts,int m, float u)
	{
		vector<vec2> poly1, poly2;
		if(m==1)
		{		
			return oneSubdivision(listPts,poly1,poly2,u);
		}
		else
		{
			vector<vec2> firstHalf, secondHalf,total,finalTotal1,finalTotal2,finalTotalReturn;
			vector<vec2>::iterator it;		
			total = oneSubdivision(listPts,poly1,poly2,u);

			firstHalf.insert(firstHalf.begin(),total.begin(),total.begin()+1+total.size()/2);
			secondHalf.insert(secondHalf.begin(),total.begin()+(1+total.size())/2-1,total.end());
			finalTotal1 = subDivide(firstHalf,m-1,u);
			finalTotal2 = subDivide(secondHalf,m-1,u);
			finalTotalReturn.insert(finalTotalReturn.begin(),finalTotal1.begin(),finalTotal1.end());
			finalTotalReturn.insert(finalTotalReturn.end(),finalTotal2.begin(),finalTotal2.end());
			return finalTotalReturn;

		}
	}


	//chaikinSubDivision Algorithm for Quadratic B-splines
	vector<vec2> chaikinSubDivision(vector<vec2> listPts, int m)
	{
		vector<vec2> intermediatePoints;
		if(m==0 || listPts.size()==1)
			return listPts;		   
		else
		{
			//intermediatePoints.push_back(listPts[0]);		
			for(int i=0;i<(listPts.size()-1);i++) 
			{

				// get 2 original points
				vec2 p0 = listPts[i];
				vec2 p1 = listPts[i+1];
				vec2 R,Q;

				// Interpolate the 2 original points to create 2 new points.
				Q.x = 0.75*p0.x + 0.25*p1.x;
				Q.y = 0.75*p0.y + 0.25*p1.y;		

				R.x = 0.25*p0.x + 0.75*p1.x;
				R.y = 0.25*p0.y + 0.75*p1.y;		

				intermediatePoints.push_back(Q);
				intermediatePoints.push_back(R);
			}
		}
		// keep the last point		

		return chaikinSubDivision(intermediatePoints,m-1);    
	}

	vector<vec2> cubicSpline(vector<vec2> listPts)
	{
		vector<vec2> returnVector;
		// use the parametric time value 0 to 1

		for(int m=0;m<listPts.size()-3;m++)
		{
			for(int i=0;i!=iterationNo;i++) 
			{
				float t = (float)i/(iterationNo-1);

				// the t value inverted
				float it = 1.0f-t;

				// calculate blending functions
				float b0 = it*it*it/6.0f;
				float b1 = (3*t*t*t - 6*t*t +4)/6.0f;
				float b2 = (-3*t*t*t +3*t*t + 3*t + 1)/6.0f;
				float b3 =  t*t*t/6.0f;

				float Bmat[4] = {b0,b1,b2,b3};

				float x = b0*listPts[m].x+ b1*listPts[m+1].x + b2*listPts[m+2].x + b3*listPts[m+3].x;
				float y = b0*listPts[m].y+ b1*listPts[m+1].y + b2*listPts[m+2].y + b3*listPts[m+3].y;		

				returnVector.push_back(vec2(x,y));	
			}
		}

		return returnVector;
	}

	void display()
	{
		vector<vec2> newPts,newPts1;
		vector<vec2> finalSubdivided;
		vector<vec2> finalChaikin;
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0.0,w,0.0,h);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);			

		if(listPts.empty()) // this is for deleting the last point from screen as the listPts vector is already empty
		{
			return;
		}

		//Normal DeCasteljau function for drawing Bezier Curve
		if(curveType==0)
		{
			if(showCtrlPointsFlag)
			{
				// Draw the dot
				drawDot(listPts);
			}

			if(showCtrlPolygonFlag)
			{
				// Draw the line segments
				drawLine(listPts);
			}

			//call De Casteljau function
			for(float t=0;t<=1;t=t+0.001)
			{
				newPts.push_back(deCasteljau(listPts,t));
			}

			/* This function draws line segments between 
			two consecutive control points to generate the bezier curve */
			drawCurves(newPts);
		}

		// Subdivision Algorithm + DeCasteljau function
		if(curveType==1)
		{
			if(showCtrlPointsFlag)
			{
				// Draw the dot
				drawDot(listPts);
			}

			if(showCtrlPolygonFlag)
			{
				// Draw the line segments
				drawLine(listPts); 
			}

			//finalSubdivided = oneSubdivision(listPts,poly1,poly2,0.5);
			finalSubdivided = subDivide(listPts,subdivi,0.5);
			subDivNew = finalSubdivided;

			if(showSubdivisionFlag)
			{
				//Use this draw function if you want to see sub-divided line segments which the algorithm has produced.
				//subDivNew = finalSubdivided;
				drawLine(finalSubdivided);
			}


			//call DeCasteljau function for generating Bezier Curve
			for(float t=0;t<=1;t=t+0.01)
			{
				newPts1.push_back(deCasteljau(finalSubdivided,t));
			}

			drawCurves(newPts1);
		}	

		//Subdivision Algorithm for Quadratic B-spline (Chaikin's Algorithm) 
		if(curveType==2)
		{		
			finalChaikin = chaikinSubDivision(listPts,qspline);
			subDivSpline = finalChaikin; 

			if(showCtrlPointsFlag)
			{
				// Draw the dot
				drawDot(listPts);
			}

			if(showCtrlPolygonFlag)
			{
				// Draw the line segments
				drawLine(listPts); 
			}

			drawCurves(finalChaikin);
		}

		//Cubic B-spline curve generation without subdivision
		if(curveType==3)
		{
			vector<vec2> finalCubicSpline;
			if(listPts.size()>=4)
			{
				finalCubicSpline = cubicSpline(listPts);
			}

			if(showCtrlPointsFlag)
			{
				// Draw the dot
				drawDot(listPts);
			}

			if(showCtrlPolygonFlag)
			{
				// Draw the line segments
				drawLine(listPts); 
			}

			if(listPts.size()<4) // If number of control points < 4 then cubic spline = quadratic spline
			{
				finalChaikin = chaikinSubDivision(listPts,qspline);
				drawCurves(finalChaikin);
			}
			else if(listPts.size()>=4)
			{
				drawCurves(finalCubicSpline);
			}

		}

		//countDrag=0; // bug fix for edit points not required it now coz doing it at mouse_up event. 

	}

	// MouseButton function for Curve Class
	void mouseButton(int button, int state, int x, int y)
	{
		vector<vec2>::iterator it;

		switch (button)
		{
		case GLUT_LEFT_BUTTON:
			switch (state)
			{
			case GLUT_DOWN:	

				if(pointStat==0)// Normal Insertion through Add points
				{
					vec2 pt = vec2(x,y);
					count=0;
					countDrag=0;
					track=0;

					//cout<<"clk at: "<<pt.x<<","<<pt.y<<endl;

					//cout<<"size"<<listPts.size()<<endl;
					//Add the points at the end of the list
					listPts.push_back(pt);	

					draw();

				}

				if(pointStat==1)//To drag and edit points
				{
					// Write code here for edit points	
					if(!listPts.empty())
					{
						//glutMotionFunc(motionFunc); find a way						
						mouseMove(x,y);
					}
					draw();
				}


				if(pointStat==2 && count==0)// Insert After Selected Point
				{	

					vec2 newPt;
					countDrag=0;
					track=0;
					switch (button)
					{							 
					case GLUT_LEFT_BUTTON:
						switch (state)
						{
						case GLUT_DOWN:

							newPt = vec2(x,y);

							//Match the clicked points with existing list and highlight that									
							for(int i=0;i<listPts.size();i++)
							{
								if(fabs(listPts[i].x-newPt.x) <=2 && fabs(listPts[i].y-newPt.y) <=2)
								{	
									newPt = listPts[i];
									count++;
									j=i;
									break;
								}
							}
							break; 
						}

						break;

						draw();

					}


				} //end if(pointStat==2 && count==0)

				else if(pointStat==2 && count>=1) // Insert After Selected Point
				{
					countDrag=0;
					track=0;
					it = listPts.insert(listPts.begin()+j+1,vec2(x,y));
					j++;

					draw();
				}

				if(pointStat==3) // Delete Selected Point
				{
					count=0; j=0;
					track=0;
					countDrag=0;
					vector<vec2>:: iterator it1;
					switch (button)
					{
					case GLUT_LEFT_BUTTON:
						switch (state)
						{
						case GLUT_DOWN:

							//Match the clicked points with existing list and highlight that
							for(int k=0;k<listPts.size();k++)
							{

								if(fabs(listPts[k].x-x) <=2 && fabs(listPts[k].y-(y)) <=2)
								{
									it1 = listPts.erase(listPts.begin()+k);
									break;
								}


							}
							break;
						}
						break;
					}

					draw();
				}

				if(pointStat==4) // Duplicate selected Point
				{
					count=0; j= 0;
					track=0;
					countDrag=0;
					vector<vec2>:: iterator it3;
					switch (button)
					{
					case GLUT_LEFT_BUTTON:
						switch (state)
						{
						case GLUT_DOWN:

							//Match the clicked points with existing list and highlight that
							for(int k=0;k<listPts.size();k++)
							{

								if(fabs(listPts[k].x-x) <=2 && fabs(listPts[k].y-(y)) <=2)
								{
									it3 = listPts.insert(listPts.begin()+k+1,listPts[k]);
									break;
								}
							}
							break;
						}
						break;
					}

					draw();
				}				

				break;

			case GLUT_UP:
				countDrag=0; // for working the drag and edit points option
				//computePoints();
			}				
			break;
		}

		draw();	
	}

};


//control ids
enum Controls 
{
	CLEAR
	,QUIT
	,CURVETYPE
	,ITERATIONS
	,POINTSTATUS
	,REFRESH
	,SELTECH
	,MESH
};

void myInit() 
{
	glClearColor(0.0,0.0,0.0,0.0);
	glColor3f(1.0,0.0,0.0);
	glPointSize(5.0);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_SCISSOR_TEST);
	glEnable(GL_DEPTH_TEST);
	
}

//void radioChanged(GLUI_Control* radio){
void radioChanged(int id)
{
	switch (id)
	{
	case CURVETYPE:			
		break;
	}
}

void pointStatusChanged(int id)
{
	switch (id)
	{
	case POINTSTATUS:			
		break;
	}
}

void techStatusChanged(int id)
{
	switch (id)
	{
	case SELTECH:			
		break;
	}
}

//Quit Function
void quit(int id){
	exit(0);
}

void control_cb(int control)
{
	switch(control)
	{
		//write cases here
	}

}


void myGlui()
{
	glui1 = GLUI_Master.create_glui( "Options");
	glui1->set_main_gfx_window(main_window);

	glui1->add_button( "Clear All", CLEAR, clear );

	//glui1 Features
	GLUI_Panel *panel = glui1->add_panel ( "Curve Type" );

	//curve type group
	GLUI_RadioGroup *curveType_rg =
		glui1->add_radiogroup_to_panel(panel,&curveType,CURVETYPE,radioChanged);
	glui1->add_radiobutton_to_group( curveType_rg, "Bezier" );
	glui1->add_radiobutton_to_group( curveType_rg, "Subdivision for Bezier" );
	glui1->add_radiobutton_to_group( curveType_rg, "Uniform Quadratic B-spline (Subdivision)" );
	glui1->add_radiobutton_to_group( curveType_rg, "Uniform Cubic B-spline" );

	GLUI_Panel *pstatus = glui1->add_panel ( "Add/Edit/Insert Points" );
	GLUI_RadioGroup *pointStatus =
		glui1->add_radiogroup_to_panel(pstatus,&pointStat,POINTSTATUS,pointStatusChanged);
	glui1->add_radiobutton_to_group( pointStatus, "Add Points" );
	glui1->add_radiobutton_to_group( pointStatus, "Edit Points" );
	glui1->add_radiobutton_to_group( pointStatus, "Insert After Selected" );
	glui1->add_radiobutton_to_group( pointStatus, "Delete Selected Point" );
	glui1->add_radiobutton_to_group( pointStatus, "Duplicate Selected Point" );

	GLUI_Spinner *subdiviSpinner=glui1->add_spinner_to_panel
		(panel,"Iteration for Bezier",GLUI_SPINNER_INT,&subdivi,USUBDIVI_ID,control_cb);
	subdiviSpinner->set_int_limits(1, 5);

	GLUI_Spinner *splineSpinner=glui1->add_spinner_to_panel
		(panel,"Iteration for Quadratic B-Spline",GLUI_SPINNER_INT,&qspline,QSPLINE_ID,control_cb);
	splineSpinner->set_int_limits(1,10);

	GLUI_Spinner *genFreqSpinner=glui1->add_spinner_to_panel
		(panel,"Generation Frequency",GLUI_SPINNER_INT,&genFreq,GENFREQ_ID,control_cb);
	genFreqSpinner->set_int_limits(5,100);

	GLUI_Spinner *extrusionSpinner=glui1->add_spinner_to_panel
		(panel,"Extrusion Depth",GLUI_SPINNER_INT,&extrudeDepth,EXTRUDE_ID,control_cb);
	extrusionSpinner->set_int_limits(20,400);

	GLUI_Panel *showWhatPanel=glui1->add_panel("Select What to show");
	GLUI_Checkbox *showCtrlPointsCheckbox=glui1->add_checkbox_to_panel
		(showWhatPanel,"Show Control Points",&showCtrlPointsFlag,SHOW_CTRLPOINTS_ID,control_cb);
	GLUI_Checkbox *showBezierWiresCheckbox=glui1->add_checkbox_to_panel
		(showWhatPanel,"Show Curve",&showBezierWiresFlag,SHOW_BEZIERWIRES_ID,control_cb);
	GLUI_Checkbox *showCtrlPolygonCheckbox=glui1->add_checkbox_to_panel
		(showWhatPanel,"Show Control Polygon",&showCtrlPolygonFlag,SHOW_CTRLPOLYGON_ID,control_cb);
	GLUI_Checkbox *showSubdivisionCheckbox=glui1->add_checkbox_to_panel
		(showWhatPanel,"Show Sub-Division Polygon",&showSubdivisionFlag,SHOW_SUBDIVISION_ID,control_cb);

	GLUI_Panel *selTechnique=glui1->add_panel("Select Technique");
	GLUI_RadioGroup *seltech =
		glui1->add_radiogroup_to_panel(selTechnique,&technique,SELTECH,techStatusChanged);
	glui1->add_radiobutton_to_group( seltech, "Surface of Revolution Control Points" );
	glui1->add_radiobutton_to_group( seltech, "Surface of Revolution with Subdivision" );
	glui1->add_radiobutton_to_group( seltech, "Surface of Revolution with Quadratic Spline" );
	glui1->add_radiobutton_to_group( seltech, "Extrusion with Bezier Subdivision" );
	glui1->add_radiobutton_to_group( seltech, "Extrusion with Quadric Spline Subdivision" );
	glui1->add_radiobutton_to_group( seltech, "Sweep with Quadric Spline Subdivision" );

	glui1->add_button( "Quit", QUIT, quit );
}

ViewportManager window;
Curve curve1(0,0,0.4,0.5,vec3 (0.0,0.0,0.0)); //bottom left corner
Curve curve2(0,0.5,0.4,0.5,vec3 (0.2,0.2,0.2)); // top left corner


// Class Declaration of Surface
class Surface : public Viewport 
{
public:
	vec3 position;
	vector<vec3> newPoints;
	vector<vec3> extrudedPoints;

	Surface(float x, float y, float w, float h): Viewport(x,y,w,h)
		,position(0,0,100){}


	void mouseMove(int x, int y) {};	

	void passiveMouseMove(int x, int y) {};

	void keyUp(unsigned char key, int x, int y){};	


	void keyDown (unsigned char key, int x, int y)
	{
		switch (key)
		{
		case 'w':
			position.z += -10;
			break;
		case 's':
			position.z += 10;
			break;
		case 'a':
			position.x += 10;
			break;
		case 'd':
			position.x += -10;
			break;
		case 'q':
			position.y += -10;
			break;
		case 'e':
			position.y += 10;
			break;

		}

	}

	void computePoints(vector<vec2> listPts)
	{
		newPoints.clear();

		glBegin(GL_POINTS);

		for (float theta = 0.0; theta <= 2.0 * PI ; theta += PI/genFreq)
		{
			for (int point=0; point<listPts.size(); point++) 
			{
				float x = listPts[point].x*cos(theta);
				float z = listPts[point].x*sin(theta);
				//cout<<"x="<<x<<","<<"y="<<y<<","<<"z="<<z<<endl; // to check the range of x,y,z after rotation

				newPoints.push_back(vec3(x,listPts[point].y,z));
				//glVertex3f(x,curve1.listPts[point].y,z);
			}		

		}
		glEnd();
	}


	// Function to do extrusion
	void extrudeFunc(vector<vec2> list)
	{
		newPoints.clear();
		glBegin(GL_POINTS);

		for(int i = 0; i<2; i++)
		{
			for(int j=0;j<list.size(); j++)

			{
				newPoints.push_back(vec3(list[j].x,i*extrudeDepth,list[j].y));
			}
		}

		glEnd();
	}	


	void drawSweep(vector<vec2> c1, vector<vec2> c2)
	{
		newPoints.clear();

		if(!c1.size() || !c2.size())
			return;
		else 
		{	

			for (int i=0; i < c1.size()-1; i++)
			{
				vec3 fv = vec3((c1[i+1] - c1[i]),0);

				vec3 x = normalize(perp(vec3(0,1,0), fv));

				if(dot(vec3(1,0,0), fv) < 0)
					x *= -1;
				vec3 y = normalize(cross(fv, x));

				for(int j=0; j < c2.size(); j++)
				{
					newPoints.push_back(vec3(c1[i], 0) + x*c2[j].x + y*c2[j].y);
				}
			}

			drawQuads(c2);
		}
	}

	void printfile(vector<vec3> newPoints,vector<vec2> list)
	{
		FILE *fp;

		fp = fopen("Vertices.txt","w");
		fprintf(fp,"%d\t",newPoints.size()); // newPoints.size() = number of vertices
		fprintf(fp,"%d\n",newPoints.size()-list.size());
		for(int i=0;i<newPoints.size();i++)
		{
			
			fprintf(fp,"%.2f\t%.2f\t%.2f\t",newPoints[i].x,newPoints[i].y,newPoints[i].z);
			fprintf(fp,"\n");
		}

        for(int i=0;i<newPoints.size()-list.size();i++)
		{
			//if(i % list.size() == list.size()-1) continue;
			fprintf(fp,"4\t");
			//fprintf(fp,"\n");
			fprintf(fp,"%d\t",i);
			fprintf(fp,"%d\t",i+1);
			fprintf(fp,"%d\t",i+1+list.size());
			fprintf(fp,"%d\t",i+list.size());
			fprintf(fp,"\n");
		}

		fclose(fp);
	}

	void drawQuads(vector<vec2> c2)
	{
		glColor3f(0.5,0.2,0);

		glBegin(GL_QUADS);

		for(int i=0;i<newPoints.size()-c2.size();i++)
		{
			if(i % c2.size() == c2.size()-1) continue;
			vec3 point1= newPoints[i];
			vec3 point2 = newPoints[i+1];
			vec3 point3 = newPoints[i+c2.size()+1];
			vec3 point4 = newPoints[i+c2.size()];

			vec3 normal = cross(point2-point1,point3-point1);
			normal = normalize(normal);

			glNormal3f(normal.x,normal.y,normal.z);

			glVertex3f(newPoints[i].x,newPoints[i].y,newPoints[i].z);
			glVertex3f(newPoints[i+1].x,newPoints[i+1].y,newPoints[i+1].z);
			glVertex3f(newPoints[i+c2.size()+1].x,newPoints[i+c2.size()+1].y,newPoints[i+c2.size()+1].z);
			glVertex3f(newPoints[i+c2.size()].x,newPoints[i+c2.size()].y,newPoints[i+c2.size()].z);
		}
		printfile(newPoints,c2); // printfile called
		glEnd();

	}     


	void drawMesh(vector<vec2> list)
	{
		glColor3f(0.5,0.2,0);		
		glBegin(GL_QUADS);

		for(int i=0;i<newPoints.size()-list.size();i++)
		{			
			if(i % list.size() == list.size()- 1)	
				continue;		
			
				vec3 point1= newPoints[i];
				vec3 point2 = newPoints[i+1];
				vec3 point3 = newPoints[i+list.size()+1];
				vec3 point4 = newPoints[i+list.size()];

				vec3 normal = cross(point2-point1,point3-point1);
				normal = normalize(normal);

				glNormal3f(normal.x,normal.y,normal.z);

				glVertex3f(newPoints[i].x,newPoints[i].y,newPoints[i].z);
				glVertex3f(newPoints[i+1].x,newPoints[i+1].y,newPoints[i+1].z);
				glVertex3f(newPoints[i+list.size()+1].x,newPoints[i+list.size()+1].y,newPoints[i+list.size()+1].z);
				glVertex3f(newPoints[i+list.size()].x,newPoints[i+list.size()].y,newPoints[i+list.size()].z);
			
			
		}

		printfile(newPoints,list); // printfile called
		glEnd();
	}

	

	void draw() 
	{
		glClearColor(0.5,0.5,0.3,1);		
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glMatrixMode (GL_PROJECTION);
		glLoadIdentity ();
		gluPerspective(70,(float)w/h,1,1200); 
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity ();			

		gluLookAt(position.x,position.y,position.z, position.x,position.y,position.y-1, 0,1,0);
		glTranslatef(-50,-150,-800); // cruicial statement to see the object.		

		GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
		GLfloat mat_shininess[] = { 50.0 };
		GLfloat light_position[] = { 0.0, 0.0, 200.0, 0.0 };
		GLfloat light_position1[] = {-200,0,100,0 };
		glShadeModel (GL_SMOOTH);

		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
		glLightfv(GL_LIGHT0, GL_POSITION, light_position);
		glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHT1);

		GLfloat material_diffuse[] = { 0.4, 0.2, 0.4, 1 };
		GLfloat material_specular[] = { 1, 1, 1, 1 };
		GLfloat material_shininess[] = { 100 };
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_specular);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, material_shininess);


		if(technique==0)// means surface of revolution
		{
			computePoints(curve1.listPts);
			drawMesh(curve1.listPts);
		}

		else if(technique==1) // surface with subdivision bezier
		{
			computePoints(curve1.subDivNew);			
			drawMesh(curve1.subDivNew);
		}

		else if (technique==2) // surface with subdivision quadratic bspline 
		{
			computePoints(curve1.subDivSpline);			
			drawMesh(curve1.subDivSpline);

		}

		else if (technique==3) // Extrusion 
		{
			extrudeFunc(curve1.subDivNew);
			drawMesh(curve1.subDivNew);

		}

		else if (technique==4) // Extrusion with spline subdivision
		{
			extrudeFunc(curve1.subDivSpline);
			drawMesh(curve1.subDivSpline);
		}

		else if (technique==5) // Sweep 
		{			
			drawSweep(curve1.subDivSpline,curve2.subDivSpline);

		}

	}

	void mouseButton(int button, int state, int x, int y)
	{ 

		switch (button)
		{
		case GLUT_LEFT_BUTTON:
			switch (state)
			{
			case GLUT_DOWN:	

				if(technique==0) // means surface of revolution
				{
					//cout<<"clk at: "<<x<<","<<y<<endl;
				}

				break;
			}
			break;
		}
	}
};



//creating object for surface class
Surface surface1(0.4,0,0.6,1); // right side 

void mouseButton(int button, int state, int x, int y)
{		
	window.mouseButton(button,state,x,y);

}

void motionFunc(int x, int y)
{
	window.mouseMove(x,y);
}

void keyDown(unsigned char key, int x, int y){
	window.keyDown(key,x,y);
}

void reshape(int x, int y)
{
	int tx,ty,tw,th;
	GLUI_Master.get_viewport_area(&tx,&ty,&tw,&th);
	window.resize(tw,th);
	glutPostRedisplay();
}


void idle()
{
	glutPostRedisplay();

};

void display()
{
	window.draw();
	glutSwapBuffers();
};

//CLEAR
void clear (int id)
{
	switch (id)
	{
	case CLEAR:  // button event clear comes here and clears the screen
		{
			curve1.listPts.clear();
			curve2.listPts.clear();
			curve1.subDivNew.clear();
			curve2.subDivNew.clear();	
			surface1.newPoints.clear();
			curve1.count=0;
			curve2.count=0;
			curve1.countDrag=0;
			curve2.countDrag=0;
			curve1.track=0;
			curve2.track=0;
			curve1.subDivSpline.clear();
			curve2.subDivSpline.clear();
			curve1.j=0;
			curve2.j=0;
			glutPostRedisplay();
		}
		break;
	}
}

//Main Function
int main(int argc, char *argv[])
{
	glutInit(&argc,argv);		
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
	glutInitWindowSize(dim.x,dim.y);
	glutInitWindowPosition(50,50);
	main_window = glutCreateWindow("Soumya Dutta Lab2 CSE784");
	cout << "Instructor: Dr. Tamal K. Dey" << endl;
	cout << "TA: Andrew Slatton"<< endl;
	cout <<  "Author: Soumya Dutta " << endl << "email: dutta.33@osu.edu"<<endl << "1st Year Ph.D. student." << endl << "The Ohio State University" <<
		endl << "Department of Computer Science and Engineering" << endl << endl << endl; 

	cout << "This Program is written using C++ and openGL library functions."<<endl; 
	cout << "It uses GLUI,GLUT and glm libraries to create the interactive GUI"<< endl << "and adds flexibility for the user." << endl;
	cout << "The interactive screen has three parts, identified by different colors. The two regins in left are interactive regions and user can click "<<endl
		  << "in that region to  input control points for the curves. Bottom left region is used for getting the curve for extrusion and surface of revolution" << endl;
	cout << "For sweep user have to draw two crves. The the curve from the top section will be sweeped into curve in the bottom section and the figuer will be"<<endl;
	cout<< "shown at the right part of the window."<<endl<<endl<<endl;

	cout << "Implemented Curve Types: "<< endl;
	cout << "1. Bezier Curve " << endl;
	cout << "2. Bezier Curve (Using Subdivision algorithm) " << endl;
	cout << "3. Uniform Quadratic B-spline Curve ( Using Chaikin's Subdivision) " << endl;
	cout << "4. Uniform Cubic B-spline Curve " << endl<<endl<<endl;

	cout<< "Different Implemented methods: "<<endl;
	cout<< "1. Surface of Revolution"<<endl;
	cout<< "2. Extrusion"<<endl;
	cout<< "3. Sweep Operator"<<endl;

	window.add(&curve1);
	window.add(&curve2);
	window.add(&surface1);

	myGlui();
	myInit();

	GLUI_Master.set_glutMouseFunc(mouseButton);
	glutMotionFunc(motionFunc);
	glutDisplayFunc(display);	
	glutIdleFunc(idle); 
	glutKeyboardFunc(keyDown);	
	GLUI_Master.set_glutReshapeFunc(reshape);
	glutMainLoop();
	return 0;
}