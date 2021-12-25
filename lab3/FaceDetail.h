
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

using namespace glm;
using namespace std;
using namespace OpenMesh;
using namespace IO;
using namespace Iterators;

typedef OpenMesh::PolyMesh_ArrayKernelT<>  MyMesh;


class FaceDetail {
public:
	
	MyMesh :: Point facepoint;
	FaceHandle fh;

	FaceDetail() {};
};
