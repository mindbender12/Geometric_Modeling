#ifndef _LOOP_
#define _LOOP_

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


// -------------------- OpenMesh
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

typedef OpenMesh::PolyMesh_ArrayKernelT<>  MyMesh;

struct loop {
	public:
		
		loop()  {} // no argument constructor

		void createLoopSubdivision(MyMesh mesh, int it_no)
		{
			OpenMesh::Subdivider::Uniform::LoopT <MyMesh> Tt;
	        Tt.attach(mesh);
	        bool a = Tt(it_no);
	        Tt.detach();
			write_mesh(mesh,"loop.off");
		}		
};


#endif

