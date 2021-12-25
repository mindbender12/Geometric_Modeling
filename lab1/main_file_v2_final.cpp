/* CSE 784 Lab1*/
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
#include <glm/glm.hpp> // glm::vec3, glm::vec4, glm::ivec4, glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr
#include <ctime>
#include <vector>

using namespace glm;
using namespace std;

void display (vector<vec2>);
void myInit();
void radioChanged(int);
void reshape (int,int);
void quit(int);
void control_cb(int);
void clearScreen();
void clear (int);
void myGlui();
void drawDot(vector<vec2>);
void drawLine(vector<vec2>);
void drawCurves(vector<vec2>);
void display();
void mouseButton(int,int,int,int);
void motionFunc(int,int);
vector<vec2> oneSubdivision(vector<vec2>,vector<vec2>,vector<vec2>,float);
vector<vec2> subDivide(vector<vec2>,int,float);
vector<vec2> chaikinSubDivision(vector<vec2>,int);
void pointStatusChanged(int);
vector<vec2> cubicSpline(vector<vec2>);

#define SHOW_CTRLPOINTS_ID	100
#define SHOW_BEZIERWIRES_ID	101
#define SHOW_CTRLPOLYGON_ID 102
#define SHOW_SUBDIVISION_ID 103
#define USUBDIVI_ID 104
#define QSPLINE_ID 105

int main_window;
int showCtrlPointsFlag=1; 
int showBezierWiresFlag=1;
int showCtrlPolygonFlag =1;
int showSubdivisionFlag=0;
int subdivi=1;
int qspline=5;
int numOfPts =0;
int subdivision =1;
int j = 0;
int track=0;
int countDrag=0;
int count=0;
uvec2 dim(800,600);
GLUI *glui1;
vec2 P0(0,0);
vector<vec2> listPts;
int iterationNo=15;

//live vars
int curveType=0;
int iterations;
int mode;
int currentCurveType;
int pointStat=0;

//control ids
enum Controls {
	CLEAR
	,QUIT
	,CURVETYPE
	,ITERATIONS
	,POINTSTATUS
	,REFRESH
};

void myInit() 
{
	glClearColor(0.0,0.0,0.0,0.0);
	glColor3f(1.0,0.0,0.0);
	glPointSize(6.0);
	glEnable(GL_POINT_SMOOTH);	

    /* viewing transformation  */
    gluLookAt (0.0, 0.0, 1.0, 0,0,0, 0.0, 1.0, 0.0);	
	glOrtho(0.0,dim.x,0.0,dim.y,1,-1);

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


//-------------------------------------------------------------------------
//  This function is passed to the glutReshapeFunc and is called 
//  whenever the window is resized.
//-------------------------------------------------------------------------
void reshape (int w, int h)
{
	//  Reset viewport
	glViewport(0, 0, w,h);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();  
	glutWireCube (1.0);
	glFrustum (-1.0, 1.0, 1.0, 1.0, 1.5, 20.0);
    glMatrixMode (GL_MODELVIEW);
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

//intermediate clear function which is called before each redraw
void clearScreen()
{	
	listPts.clear();
	count=0;
	countDrag=0;
	track=0;
	j=0;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f );
	glutSwapBuffers();	
}

//CLEAR
void clear (int id)
{
	switch (id)
	{
	case CLEAR:
		clearScreen(); // button event clear comes here
		break;
	}
}

void refresh(int id)
{
	switch (id)
	{
	case REFRESH:

		count=0;
		track=0;
		countDrag=0;
		j=0;
		break;
	}
}

void myGlui()
{
	glui1 = GLUI_Master.create_glui( "Options");
	glui1->set_main_gfx_window(main_window);

	//glui1 Features
	GLUI_Panel *panel = glui1->add_panel ( "Generation Technique" );

	//curve type group
	GLUI_RadioGroup *curveType_rg =
		glui1->add_radiogroup_to_panel(panel,&curveType,CURVETYPE,radioChanged);
	glui1->add_radiobutton_to_group( curveType_rg, "Bezier" );
	glui1->add_radiobutton_to_group( curveType_rg, "Bezier with Subdivision" );
	glui1->add_radiobutton_to_group( curveType_rg, "Uniform Quadratic B-spline (Subdivision)" );
	glui1->add_radiobutton_to_group( curveType_rg, "Uniform Cubic B-spline" );

	GLUI_Panel *pstatus = glui1->add_panel ( "Add/Modify Points" );
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
	splineSpinner->set_int_limits(1,5);

	GLUI_Panel *showWhatPanel=glui1->add_panel("Select What to show");
	GLUI_Checkbox *showCtrlPointsCheckbox=glui1->add_checkbox_to_panel
		(showWhatPanel,"Show Control Points",&showCtrlPointsFlag,SHOW_CTRLPOINTS_ID,control_cb);
	GLUI_Checkbox *showBezierWiresCheckbox=glui1->add_checkbox_to_panel
		(showWhatPanel,"Show Curve",&showBezierWiresFlag,SHOW_BEZIERWIRES_ID,control_cb);
	GLUI_Checkbox *showCtrlPolygonCheckbox=glui1->add_checkbox_to_panel
		(showWhatPanel,"Show Control Polygon",&showCtrlPolygonFlag,SHOW_CTRLPOLYGON_ID,control_cb);
	GLUI_Checkbox *showSubdivisionCheckbox=glui1->add_checkbox_to_panel
		(showWhatPanel,"Show Sub-Division Polygon",&showSubdivisionFlag,SHOW_SUBDIVISION_ID,control_cb);

	glui1->add_button( "Refresh", REFRESH, refresh );
	glui1->add_button( "Clear All", CLEAR, clear );	
	glui1->add_button( "Quit", QUIT, quit );

}

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


/*template <class T>
void concat(vector<T>& v1, vector<T>& v2){
v1.insert(v1.end(), v2.begin(), v2.end());
}
v1.insert(v1.end(), v2.begin(), v2.end());
*/

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
	//intermediatePoints.push_back(listPts[listPts.size()-1]);

	return chaikinSubDivision(intermediatePoints,m-1);    

}

//This function generates points for a Cubic B-spline curve
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

//This is the display function
void display ()
{
	glClear(GL_COLOR_BUFFER_BIT);
	vector<vec2> newPts,newPts1;
	vector<vec2> finalSubdivided;
	vector<vec2> finalChaikin;

	if(listPts.empty()) // this is for deleting the last point from screen as the listPts vector is already empty
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f );
		glutSwapBuffers();
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

		if(showSubdivisionFlag)
		{
			//Use this draw function if you want to see sub-divided line segments which the algorithm has produced.
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

	countDrag=0;// bug fix for edit points
	
	glutSwapBuffers();
}

// This is to handle drag and move mouse function for editing points
void motionFunc(int x, int y)
{
	if(pointStat==1)
	{
		vector<vec2>:: iterator it2;

		if(countDrag==0)
		{
			for(int p=0;p<listPts.size();p++)
			{
				if(fabs(listPts[p].x-x) <= 2 && fabs(listPts[p].y-(dim.y-y)) <= 2)
				{
					track=p;
					countDrag++;
					break;
				}	
			}
		}
		else if(countDrag>=1)
		{
			it2 = listPts.insert(listPts.begin()+track+1,vec2(x,dim.y-y));
			it2 = listPts.erase(listPts.begin()+track);
		}
	}

}


void mouseButton(int button, int state, int x, int y) 
{
	//static int count=0;   
	//static int j = 0;

	vector<vec2>::iterator it;

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		switch (state)
		{
		case GLUT_DOWN:					

			//P0 = listPts[0];
			if(pointStat==0)// Normal Insertion through Add points
			{
				vec2 pt = vec2(x,dim.y - y);
				count=0;
				countDrag=0;
				track=0;

				//Add the points at the end of the list
				listPts.push_back(pt);	
			}

			if(pointStat==1)//To drag and edit points
			{
				// Write code here for edit points	
				if(!listPts.empty())
				{
					glutMotionFunc(motionFunc);                        
				}
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

						newPt = vec2(x,dim.y - y);

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

				}


			} //end if(pointStat==2 && count==0)

			else if(pointStat==2 && count>=1) // Insert After Selected Point
			{
				countDrag=0;
				track=0;
				it = listPts.insert(listPts.begin()+j+1,vec2(x,dim.y-y));
				j++;

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
							if(fabs(listPts[k].x-x) <=2 && fabs(listPts[k].y-(dim.y-y)) <=2)
							{
								it1 = listPts.erase(listPts.begin()+k);
								break;
							}
						}
						break;
					}
					break;
				}
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

							if(fabs(listPts[k].x-x) <=2 && fabs(listPts[k].y-(dim.y-y)) <=2)
							{
								it3 = listPts.insert(listPts.begin()+k+1,listPts[k]);
								break;
							}
						}
						break;
					}
					break;
				}
			}							
			//Calling the display function					
			glutDisplayFunc(display);	

			break;
		}				
		break;
	}
}


//Main Function
int main(int argc, char *argv[])
{
	glutInit(&argc,argv);		
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
	glutInitWindowSize(dim.x,dim.y);
	glutInitWindowPosition(350,350);
	main_window = glutCreateWindow("Soumua Dutta Lab1 CSE784 ");

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		cout << "glewInit failed, aborting." << endl;
		exit (1);
	}

	//cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << endl;
	//cout << "OpenGL version " << glGetString(GL_VERSION) << " supported" << endl<<endl;

	cout << "CSE 784 Lab1 Demo: " << endl;
	cout << "Instructor: Dr. Tamal K. Dey" << endl;
	cout << "TA: Andrew Slatton"<< endl;
	cout <<  "Author: Soumya Dutta " << endl << "email: dutta.33@osu.edu"<<endl << "1st Year Ph.D. student." << endl << "The Ohio State University" <<
	endl << "Department of Computer Science and Engineering" << endl << endl << endl; 

	cout << "This Program is written using C++ and openGL library functions."<<endl;
	cout << "It uses GLUI,GLUT and glm libraries to create the interactive GUI"<< endl << "and adds flexibility for the user." << endl;

	cout << endl<<endl<<"Implemented Curve Types: "<< endl;
	cout << "1. Bezier Curve " << endl;
	cout << "2. Bezier Curve (Using Subdivision algorithm) " << endl;
	cout << "3. Uniform Quadratic B-spline Curve ( Using Chaikin's Subdivision) " << endl;
	cout << "4. Uniform Cubic B-spline Curve " << endl;

	myGlui();
	myInit();
	glutMouseFunc(mouseButton);
	glutReshapeFunc(reshape);
	glutMainLoop();
	return 0;
}