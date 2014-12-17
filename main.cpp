/** Sabin Timalsena, Radiosity
 *
 * This program implements radiosity using the hemicube method to calculate the
 * form factors. To project facets to the sides of the hemicube, I have used
 * OpenGL's rasterizing functionality. All of this functionality is in the Scene
 * class, which holds the geometry of the scene and contains several functions.
 * The most important functions are loadFromObjFile(), which loads the geometry
 * from an external file; calculateFormFactors(), which calculates the form
 * factors for every face pair, and calculateRadiosities() which calculates the
 * radiosities based on form factors and face properties (one iteration). Also,
 * there are two rendering functions: render(), which renders the scene based on
 * the radiosities computed, and renderColorCoded(), which renders the scene
 * with a unique color for each face. Faces can be identified by the color drawn
 * into the framebuffer.
 *
 * The geometry consists entirely of triangles. They are read from a standard
 * .obj file, which lists vertices, faces, and also specifies which object a
 * group of faces belong to. The material properties are assigned properly to
 * each face based on the object name in the file. (Scene::loadFromObjFile())
 *
 * Application.h/cpp files contain all of the code for keyboard/mouse/event
 * handling. main.cpp sets up glut and register the handlers defined in the
 * Application class.
 *
 * Features:
 * -----------------------------------------------------------------------------
 * Press 'S' to start the simulation
 * Press 'X' to quit application
 * Pressing 'P' switches between orthographic/perspective renderings
 * Left mouse click + drag rotates the camera view
 */

#include <GL/freeglut.h>
#include "Application.h"

static Application* application = 0;

static void keyboardFunc(unsigned char key, int x, int y) {
    application->keyboardFunc(key,x,y);
}

static void reshapeFunc(int w, int h) {
    application->reshapeFunc(w,h);
}

static void idleFunc() {
    application->idleFunc();
}

static void displayFunc(void) {
    application->displayFunc();
}

static void mouseMotionFunc (int x, int y) {
    application->mouseMotionFunc(x, y);
}

static void mouseFunc (int button, int state, int x, int y) {
    application->mouseFunc(button, state, x, y);
}

/* Program entry point */
int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(10,10);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE | GLUT_DEPTH);

    glutCreateWindow("Radiosity");

    application = new Application();
    application->init();

    glutKeyboardFunc(keyboardFunc);
    glutReshapeFunc(reshapeFunc);
    glutMouseFunc(mouseFunc);
    glutMotionFunc(mouseMotionFunc);
    glutIdleFunc(idleFunc);
    glutDisplayFunc(displayFunc);

    displayFunc();
    glutMainLoop();

    delete application;
    return 0;
}
