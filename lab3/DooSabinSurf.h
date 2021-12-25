#ifndef _DOOSUBIN_
#define _DOOSUBIN_

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
#include "FaceData.h"
// -------------------- OpenMesh Includes ----------------
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/LoopT.hh>

#include <OpenMesh/Core/System/config.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/SubdividerT.hh>
#include <OpenMesh/Core/Utils/vector_cast.hh>


using namespace glm;
using namespace std;
using namespace OpenMesh;
using namespace IO;
using namespace Iterators;

typedef OpenMesh::PolyMesh_ArrayKernelT<>  MyMesh;

vector<pointStruct> newVertex;

struct DooSabinSurf {

	//This func returns midpoint of a edge
	MyMesh :: Point compute_midpoint(MyMesh mesh, EdgeHandle e)
	{
		HalfedgeHandle heh, opp_heh;
		VertexHandle v1,v2;
		MyMesh :: Point midPoint;
		heh = mesh.halfedge_handle( e, 0);
		opp_heh  = mesh.halfedge_handle( e, 1);
		v1 = mesh.to_vertex_handle(heh);
		v2 = mesh.to_vertex_handle(opp_heh);
		midPoint = (mesh.point(v1) + mesh.point(v2))/2;
		return midPoint;				
	}

	//This func creates the DooSabin subdivision on a given mesh and generates points
	void DooSabinGen (MyMesh mesh)
	{
		MyMesh doosubmesh;
		int counter=0;
		FaceHandle fhandle;
		MyMesh :: Point fp,fp1;		
		fp1[0]=fp1[1]=fp1[2]=0;

		int nPtFace=0;
		dooSabinCompleteFaceVertex finalFaceVertex;		

		newVertex.clear(); // empty the newVertex
		for(MyMesh :: FaceIter fe = mesh.faces_begin(); fe != mesh.faces_end(); ++fe) // for each face loop through the whole mesh
		{
			fhandle = fe.handle();				
			mesh.calc_face_centroid(fhandle,fp1);
			fp[0]=fp[1]=fp[2]=0;
			nPtFace=0;
			dooSabinFace dface;
			VertexHandle vh;
			HalfedgeHandle heh,nxtheh;				
			EdgeHandle eh1,eh2;


			for(MyMesh::FaceHalfedgeIter fh_iter = mesh.fh_iter(fhandle); fh_iter; ++fh_iter) // for each half edge in each face
			{
				dooSabinVertexMap dsvmap;
				pointStruct p;				
				heh= fh_iter.handle();
				nxtheh= mesh.next_halfedge_handle(heh);
				eh1 = mesh.edge_handle(heh);
				eh2 = mesh.edge_handle(nxtheh);
				vh = mesh.to_vertex_handle(heh);
				fp = fp + compute_midpoint(mesh,eh1);
				fp = fp + compute_midpoint(mesh,eh2);				
				fp[0]=fp[1]=fp[2]=0; // clear point fp
				nPtFace++;
				p.index=counter;
				p.point=(fp1+fp+mesh.point(vh))/4;
				counter++;
				dface.facePoints.push_back(p); // push point struct to vector
				newVertex.push_back(p);
				dface.fh = fhandle;
				doosubmesh.add_vertex((fp1+fp+mesh.point(vh))/4); // write to mesh just for checking
				dsvmap.vh=vh;
				dsvmap.pt=p;
				dsvmap.fh=fhandle;
				finalFaceVertex.finalVertexMap.push_back(dsvmap);
			}

			dface.numOfPoints=nPtFace;	
			finalFaceVertex.innerDooSabFace.push_back(dface); // create the doosabin face lists
			dface.facePoints.clear();
			dface.numOfPoints=0;
		}

		//call output function which creates the off 3D file
		createOutMesh(finalFaceVertex,mesh);
	}


	//This function creates the output mesh and off file
	void createOutMesh(dooSabinCompleteFaceVertex finalFaceVertex,MyMesh mesh)
	{
		MyMesh out;
		ofstream fout;
		fout.open("DooSub.off",ios::out);
		int numPt=0;

		fout<<"OFF"<<endl;
		fout<<newVertex.size()<<"\t"<<mesh.n_edges()+mesh.n_faces()+mesh.n_vertices()<<"\t"<<0<<endl;

		//Print All Vertices first
		for(vector<pointStruct> :: iterator vi = newVertex.begin(); vi < newVertex.end(); vi++)
		{
			fout<<vi->point<<endl;
		}

		//Now Print Face Information

		//This loop prints all the new faces generated
		for(vector<dooSabinFace> :: iterator ds = finalFaceVertex.innerDooSabFace.begin(); ds < finalFaceVertex.innerDooSabFace.end(); ds++)
		{
			fout<<ds->numOfPoints<<"\t";

			for(vector<pointStruct> :: iterator p = ds->facePoints.begin(); p < ds->facePoints.end(); p++)
			{
				fout<<p->index<<"\t";
			}

			fout<<endl;
		}


		//This loop prints all the corner points around each vertex
		for(MyMesh::VertexIter v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); ++v_it)
		{
			vector<pointStruct> temp;
			numPt=0;
			for(vector<dooSabinVertexMap> :: iterator dsv = finalFaceVertex.finalVertexMap.begin(); dsv < finalFaceVertex.finalVertexMap.end(); dsv++)
			{

				if(dsv->vh== v_it.handle())
				{
					temp.push_back(dsv->pt);
					numPt++;
				}
			}

			fout<<numPt<<"\t";
			for(vector<pointStruct> :: reverse_iterator p = temp.rbegin(); p < temp.rend(); ++p)
			{
				fout<<p->index<<"\t";
			}

			fout<<endl;

		}

		//This loop joins all adjacent edge points to form quads only
		for(MyMesh::EdgeIter e_it = mesh.edges_begin(); e_it != mesh.edges_end(); ++e_it)
		{
			EdgeHandle eh = e_it.handle();

				FaceHandle f1,f2;
				VertexHandle v1,v2;
				HalfedgeHandle heh, oppheh;
				heh = mesh.halfedge_handle(eh,0);
				oppheh = mesh.halfedge_handle(eh,1);
				v1 = mesh.to_vertex_handle(heh);
				v2 = mesh.from_vertex_handle(heh);
				f1= mesh.face_handle(heh);
				f2= mesh.face_handle(oppheh);
				vector<pointStruct> temp;
				numPt=0;
				for(vector<dooSabinVertexMap> :: iterator dsv = finalFaceVertex.finalVertexMap.begin(); dsv < finalFaceVertex.finalVertexMap.end(); dsv++)
				{								
					
					if (v1 == dsv->vh && f2 == dsv->fh)
					{
						temp.push_back(dsv->pt);
						numPt++;
					}
					if(v1 == dsv->vh && f1 == dsv->fh)
					{
						temp.push_back(dsv->pt);
						numPt++;
					}
					if (v2 == dsv->vh && f1 == dsv->fh)
					{
						temp.push_back(dsv->pt);
						numPt++;
					}
					if (v2 == dsv->vh && f2 == dsv->fh)
					{
						temp.push_back(dsv->pt);
						numPt++;
					}
				}

				fout<<numPt<<"\t";
				
				for(vector<pointStruct> :: iterator p = temp.begin(); p < temp.end(); ++p)
				{
					fout<<p->index<<"\t";

				}

				fout<<endl;


			
		}
	}


};

#endif