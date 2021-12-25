#ifndef _BSPLINE_
#define _BSPLINE_

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

int totFace1=0;
vector<pointStruct> allVertices1;

struct cubicBsplineSurf {

	//map<FaceHandle,FaceData> completeFaceData; // It will hold all the face info present in the mesh and returned
	map<FaceHandle,FaceData> completeFaceData;


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

	//Given a mesh this func creates all the three type of points
	void  cubicBSplineFunc(MyMesh mesh)
	{
		//Calculate Face points separately for each face
		FaceHandle fh;
		MyMesh::Point fp;
		int numOfPoints;
		int counter=0;
		fp[0]=fp[1]=fp[2]=numOfPoints=0;
		for (MyMesh::FaceIter ff_it= mesh.faces_begin(); ff_it!=mesh.faces_end(); ++ff_it) 
		{
			fh = ff_it.handle();
			mesh.calc_face_centroid(fh,fp);
			FaceData fdata;
			fdata.f.facepoint.point = fp;
			fdata.f.facepoint.index = counter++;
			fdata.f.fh=fh;
			allVertices1.push_back(fdata.f.facepoint);
			//cout<<"face pt"<<endl;
			completeFaceData[fh] = fdata;
		}


		//calculate edge points separately for each edge here
		EdgeHandle eh;
		HalfedgeHandle heh,oppheh;
		VertexHandle v1,v2;
		FaceHandle f1,f2;
		MyMesh :: Point center1,center2,edgecorner1,edgecorner2,newPoint;
		for (MyMesh::EdgeIter e_it= mesh.edges_begin(); e_it!=mesh.edges_end(); ++e_it) 
		{

			eh = e_it.handle();
			if(!mesh.is_boundary(eh))
			{

				heh = mesh.halfedge_handle(eh,0);
				oppheh = mesh.halfedge_handle(eh,1);
				f1 = mesh.face_handle(heh);
				f2 = mesh.face_handle(oppheh);
				mesh.calc_face_centroid(f1,center1);	
				mesh.calc_face_centroid(f2,center2);
				v1 = mesh.to_vertex_handle(heh); 
				v2 = mesh.to_vertex_handle(oppheh);
				newPoint = (center1 + center2 + mesh.point(v1) + mesh.point(v2) )/4;


				FaceData fdata = completeFaceData[f1];
				pointStruct p;
				p.point=newPoint;
				p.index = counter;
				fdata.edgePointHash[eh] = p;
				completeFaceData[f1] = fdata;

				fdata = completeFaceData[f2];				
				fdata.edgePointHash[eh] = p;

				completeFaceData[f2] = fdata;

				counter++;
				allVertices1.push_back(p);
				//cout<<"edge pt"<<endl;


			}
		}

		//calculate vertex points separately for each vertex here
		MyMesh :: Point Q,R;
		VertexHandle vh;

		for (MyMesh::VertexIter v_it=mesh.vertices_begin(); v_it!=mesh.vertices_end(); ++v_it) 
		{
			vh = v_it.handle();
			Q[0] = Q[1] = Q[2] = 0;
			R[0] = R[1] = R[2] = 0;
			MyMesh :: Point newVertex;

			if(!mesh.is_boundary(vh))
			{

				//Compute Q
				FaceHandle fh;
				MyMesh :: Point fp1,fp2;
				fp1[0] = fp1[1] = fp1[2] = 0;
				fp2[0] = fp2[1] = fp2[2] = 0;
				numOfPoints=0;
				for (MyMesh::VertexFaceIter vf_iter = mesh.vf_iter(vh); vf_iter; ++vf_iter) // calculates sum of centroids of all adjacent face points
				{
					fh =vf_iter.handle();
					//cout<<fh<<endl;
					mesh.calc_face_centroid(fh,fp2);
					fp1 = fp1 + fp2;
					numOfPoints++;				
				}

				Q = fp1/numOfPoints;

				//Compute R
				EdgeHandle eh;
				numOfPoints=0;
				fp[0] = fp[1] = fp[2] = 0;
				for (MyMesh::VertexEdgeIter ve_iter=mesh.ve_iter(vh); ve_iter; ++ve_iter)
				{
					eh = ve_iter.handle();
					fp = fp + compute_midpoint(mesh,eh);
					numOfPoints++;
				}

				R = fp/numOfPoints;

				newVertex = Q/numOfPoints + (R+R)/numOfPoints + (mesh.point(v_it)/numOfPoints);			
				pointStruct p;
				p.point = newVertex;
				p.index = counter;

				allVertices1.push_back(p);
				//cout<<"vertex pt"<<endl;

				for (MyMesh::VertexFaceIter vf_iter = mesh.vf_iter(v_it); vf_iter; ++vf_iter)
				{
					fh = vf_iter.handle();
					FaceData fdata = completeFaceData[fh];
					fdata.vertexPointHash[vh] = p;
					completeFaceData[fh] = fdata;
					//cout<<"vertex"<<vh<<"P   "<<p.index<<"f   "<<fh<<endl;

				}

				counter++;
			}
		}

		createOutMesh(completeFaceData,mesh);
	}
	//cubicBSplineFunc func ends here


	//This function creates the output mesh and off file
	void createOutMesh(map<FaceHandle,FaceData> completeFaceData,MyMesh mesh)
	{
		MyMesh out;
		ofstream fout;
		fout.open("bSpline.off",ios::out);		

		for(map<FaceHandle,FaceData> :: iterator mapit = completeFaceData.begin(); mapit != completeFaceData.end(); mapit++)
		{
			totFace1 = totFace1 + (*mapit).second.edgePointHash.size();
		}

		fout<<"OFF"<<endl;
		fout<<allVertices1.size()<<"\t"<<totFace1<<"\t"<<0<<endl;

		for(vector<pointStruct> :: iterator vi = allVertices1.begin(); vi < allVertices1.end(); vi++)
		{
			fout<<vi->point<<endl;
			out.add_vertex(vi->point);
		}


		for(map<FaceHandle,FaceData> :: iterator mapit = completeFaceData.begin(); mapit != completeFaceData.end(); mapit++)
		{

			FaceData fdata = (*mapit).second;
			FaceHandle fh = (*mapit).first;


			for (MyMesh::FaceHalfedgeIter fe_iter ( mesh, fh); fe_iter; ++fe_iter) // calculates sum of centroids of all adjacent face points
			{
				HalfedgeHandle heh = fe_iter.handle();
				EdgeHandle eh = mesh.edge_handle(heh);

				fout<<4;
				fout<<"\t"<<fdata.edgePointHash[eh].index;

				VertexHandle vt = mesh.to_vertex_handle(heh);
				fout<<"\t"<<fdata.vertexPointHash[vt].index;

				HalfedgeHandle nextheh = mesh.next_halfedge_handle(heh);
				eh = mesh.edge_handle(nextheh);
				fout<<"\t"<<fdata.edgePointHash[eh].index<<"\t"<<fdata.f.facepoint.index<<endl;
			}


		}

		write_mesh(out,"splinevertex.off");

	}

}; 

#endif
