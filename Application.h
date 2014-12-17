#ifndef APPLICATION_H
#define APPLICATION_H

#include <GL/freeglut.h>
#include "Camera.h"
#include "Scene.h"

#define WINDOW_WIDTH 700
#define WINDOW_HEIGHT 700
#define MOUSE_SENSITIVITY 0.2

#define MAX_ITERATIONS 10

class Application {
public:
    Application();

    void init();

    void keyboardFunc(unsigned char key, int x, int y);
    void keyboardUpFunc(unsigned char key, int x, int y);
    void mouseMotionFunc(int x,int y);
    void mouseFunc(int button, int state, int x, int y);
    void reshapeFunc(int w, int h);
    void displayFunc();
    void idleFunc();

    void greetingMessage ();

protected:
    Camera camera;
    bool perspective;
    int width, height;

    Scene scene;

    bool isSimulating;
    int numIterations;
};

#endif // APPLICATION_H
