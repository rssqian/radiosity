#include "Scene.h"
#include "vec3.h"
#include "uvec3.h"

#include <GL/freeglut.h>
#include <fstream>
#include <sstream>

using namespace std;

void Scene::loadFromOBJFile(const char* filename) {
    vertices.clear();
    faces.clear();
    faceNormals.clear();
    faceEmissions.clear();
    faceReflectivities.clear();
    formFactors.clear();

    VtoF.clear();

    ifstream fs(filename);

    char c;
    vec3 vertex;
    uvec3 face;
    int index[3];
    vec3 currentEmission;
    vec3 currentReflectivity; /*  this is the material "color" */
    long faceIndex = 0;

    std::string line;
    while (std::getline(fs, line)) {
        std::istringstream iss(line);
        iss >> c;

        string name;
        switch (c) {
        case 'o':
            /*  this marks the start a new object (i.e. a new material) */
            iss >> name;
            /*  assign proper colors according to the object */
            if (name.find("wall") != string::npos) {
                currentEmission = vec3(0,0,0);
                currentReflectivity = 0.7 * vec3(1, 0.25, 0.000000); /* red */
            } else if (name.find("door") != string::npos) {
                currentEmission = vec3(0,0,0);
                currentReflectivity = 0.5 * vec3(1,1,1); /* gray */
            } else if (name.find("window") != string::npos) {
                currentEmission = vec3(1,1,1);
                currentReflectivity = 0.6 * vec3(1, 1, 1);
            } else if (name.find("frame") != string::npos) {
                currentEmission = vec3(0,0,0);
                currentReflectivity = 0.1 * vec3(0.06, 0.06, 0.06); /* very dark */
            } else if (name.find("floor") != string::npos) {
                currentEmission = vec3(0,0,0);
                currentReflectivity = 0.3 * vec3(1,1,1);
            } else if (name.find("pole") != string::npos) {
                currentEmission = vec3(0,0,0);
                currentReflectivity = 0.5 * vec3(0.5, 0.5, 0.5);
            } else if (name.find("lamp") != string::npos) {
                currentEmission = 2 * vec3(1, 1, 1);
                currentReflectivity = vec3(1, 1, 1);
            } else if (name.find("Cube") != string::npos) {
                currentEmission = vec3(0,0,0);
                currentReflectivity = 0.7 * vec3(1,1,1);
            }
            break;
        case 'v':
            /* read a vertex */
            iss >> vertex.x;
            iss >> vertex.y;
            iss >> vertex.z;
            vertices.push_back(vertex);
            /* initial vertex colors for debugging */
            vertexColors.push_back(currentReflectivity);
            break;
        case 'f':
            /* read a triangle's vertex indices */
            iss >> index[0];
            iss >> index[1];
            iss >> index[2];
            /* NOTE: index in obj files starts from 1 */
            face = uvec3(index[0] - 1, index[1] -1, index[2] -1);
            faces.push_back(face);
            faceEmissions.push_back(currentEmission );
            faceReflectivities.push_back(currentReflectivity);
            {
                uvec3 code = encodeColor(faceIndex);
                faceColorCodes.push_back(code);
            }
            faceIndex++;
            break;
        default:
            /* skip the line */
            break;
        }
    }

    fs.close();

    buildVertexToFaceMap();
    autoCalculateNormals();
    vec3 r(0.5, 0.5, 0.5);  /* initialize radiosities to 0.5 */
    radiosities = vector<vec3> (faces.size(), r);
}

void Scene::buildVertexToFaceMap() {
    VtoF = vector< vector<int> >(vertices.size());

    for (unsigned int i = 0; i < faces.size(); i++) {
        uvec3& face = faces[i];
        /* add this face to the list of faces for all three vertices */
        VtoF[face.x].push_back(i);
        VtoF[face.y].push_back(i);
        VtoF[face.z].push_back(i);
    }
}

void Scene::render() {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        for (unsigned int fi = 0; fi < faces.size(); fi++) {
        uvec3& face =  faces[fi];
        vec3& v1 = vertices[face.x];
        vec3& v2 = vertices[face.y];
        vec3& v3 = vertices[face.z];

        //vec3& c1 = radiosities[fi];
        //vec3& c2 = c1;
        //vec3& c3 = c1;
        vec3& c1 = vertexColors[face.x];
        vec3& c2 = vertexColors[face.y];
        vec3& c3 = vertexColors[face.z];

        glBegin(GL_TRIANGLES);
            glColor3f(c1.x, c1.y, c1.z);
            glVertex3f(v1.x, v1.y, v1.z);
            glColor3f(c2.x, c2.y, c2.z);
            glVertex3f(v2.x, v2.y, v2.z);
            glColor3f(c3.x, c3.y, c3.z);
            glVertex3f(v3.x, v3.y, v3.z);
        glEnd();
    }
}

void Scene::autoCalculateNormals () {
    faceNormals.clear();

    for (unsigned int i = 0; i < faces.size(); i++) {
        uvec3& face =  faces[i];
        vec3& v1 = vertices[face.x];
        vec3& v2 = vertices[face.y];
        vec3& v3 = vertices[face.z];

        vec3 normal = cross(v2 - v1, v3 - v2);
        normal.normalize();
        faceNormals.push_back(normal);
    }
}

void Scene::calculateFormFactors() {
    formFactors.clear();
    formFactors = vector<vector<float>> (faces.size(), vector<float>(faces.size(), 0.0f));

    for (unsigned int f = 0; f < faces.size(); f++) {
        int pixelCount[MAX_FACES] = {0};

        uvec3& face = faces[f];
        vec3 centroid = (vertices[face.x] + vertices[face.y] + vertices[face.z]) * 0.333333f;

        /* the new basis for the coordinate system around the centroid */
        /* n is the normal; d1, d2 are orthogonal unit vectors on the plane of the face */
        vec3& n = faceNormals[f];
        vec3 d1, d2;

        /* find vertex farthest from centroid */
        vec3 a = vertices[face.x] - centroid;
        vec3 b =  vertices[face.y] - centroid;
        vec3 c = vertices[face.z] - centroid;
        d1 = longestVector(a,b,c);
        d1.normalize();
        d2 =  cross(n, d1);

        /* half-length of the unit hemicube */
        float r = 0.5;

        float left, right, bottom, top;
        vec3& eye = centroid;
        vec3 lookat, up;

        for (int i = 1; i <= 5; i++) {
            if (i == 1) { /* setup view for face 1 (top) */
                left = -r; right = r; bottom = -r; top = r;
                lookat = centroid + n; up = d2;
            }
            else if (i == 2) { /* setup view for face 2 (left) */
                left = -r; right = r; bottom = 0; top = r;
                lookat = centroid + d1; up = n;
            }
            else if (i == 3) { /* setup view for face 3 (right) */
                left = -r; right = r; bottom = 0; top = r;
                lookat = centroid - d1; up = n;
            }
            else if (i == 4) { /* setup view for face 4 (back) */
                left = -r; right = r; bottom = 0; top = r;
                lookat = centroid - d2; up = n;
            }
            else if (i == 5) { /* setup view for face 5 (front) */
                left = -r; right = r; bottom = 0; top = r;
                lookat = centroid + d2; up = n;
            }

            /* setup viewport */
            unsigned int viewWidth = (right - left) / (2 * r) * HEMICUBE_RESOLUTION;
            unsigned int viewHeight = (top - bottom) / (2 * r) * HEMICUBE_RESOLUTION;
            glViewport(0, 0, viewWidth, viewHeight);

            /* setup the hemicube's side as a viewplane */
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glFrustum(left, right, bottom, top, r, 1000);
            gluLookAt(eye.x, eye.y, eye.z, lookat.x, lookat.y, lookat.z, up.x, up.y, up.z);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            /* rasterize entire scene onto this side of the hemicube */
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            renderColorCoded();

            GLubyte* buffer = new GLubyte[viewWidth * viewHeight * 3];
            /* get the contents from frame buffer */
            glReadPixels(0, 0, viewWidth, viewHeight, GL_RGB, GL_UNSIGNED_BYTE, buffer);

            /* count pixels for each face */
            for (int y = 0; y < viewHeight; y++) {
                for (int x = 0; x < viewWidth; x++) {
                    uvec3 color(buffer[(y*viewWidth+x)*3], buffer[(y*viewWidth+x)*3+1], buffer[(y*viewWidth+x)*3+2]);
                    unsigned int face = decodeColor(color);
                    if (face == -1) {
                        continue; /* empty pixel */
                    }
                    pixelCount[face]++;
                }
            }

            delete buffer;
        }
        /* end of for loop, pixels now counted on all 5 sides of hemicube */

        int totalPixels = 3 * HEMICUBE_RESOLUTION * HEMICUBE_RESOLUTION;

        /* compute form factors */
        for (unsigned int k = 0; k < faces.size(); k++) {
            if (k == f) {
                formFactors[f][k] = 0;
                continue;
            }
            float factor = pixelCount[k] / (float) totalPixels;
            formFactors[f][k] = factor;
        }
    }
}

void Scene::renderColorCoded() {
    for (unsigned int fi = 0; fi < faces.size(); fi++) {
        uvec3& face =  faces[fi];
        vec3& v1 = vertices[face.x];
        vec3& v2 = vertices[face.y];
        vec3& v3 = vertices[face.z];

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        uvec3& color = faceColorCodes[fi];
        glColor3ub(color.x, color.y, color.z);
        glBegin(GL_TRIANGLES);
            glVertex3f(v1.x, v1.y, v1.z);
            glVertex3f(v2.x, v2.y, v2.z);
            glVertex3f(v3.x, v3.y, v3.z);
        glEnd();
    }
}

uvec3 Scene::encodeColor (unsigned int f) {
    uvec3 code;
    f = f + 1; /* to take care of 0 based indexing of faces */
    code.x = f % 256;
    code.y = (f >> 8) % 256;
    code.z = (f >> 16) % 256;
    return code;
}

unsigned int Scene::decodeColor (uvec3 color) {
    return color.x + ((unsigned int)color.y << 8) + ((unsigned int)color.z << 16) - 1;
}

void Scene::calculateRadiosities() {
    for (int i = 0; i < faces.size(); i++) {
        radiosities[i] = faceEmissions[i];
        for (int j = 0; j < faces.size(); j++) {
            radiosities[i].add(faceReflectivities[i] * formFactors[j][i] * radiosities[j]);
        }
    }
}

void Scene::interpolateColors() {
    for (int vi = 0; vi < vertices.size(); vi++) {
        vector<int>& nearFaces = VtoF[vi];
        vec3 color(0,0,0);
        for (int fi = 0; fi < nearFaces.size(); fi++) {
            color.add(radiosities[nearFaces[fi]]);
        }
        color = color / (float) nearFaces.size();
        vertexColors[vi] = color;
    }
}
