#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp> 
#include <vector>
#include <cmath>
#include <iostream>

const int WINDOW_WIDTH = 600;     /** Ablak szélessége pixelben */
const int WINDOW_HEIGHT = 600;     /** Ablak magassága pixelben */
const float     M_PI = asin(1.0) * 2.0;

std::vector<glm::vec2> controlPoints;  /** A képernyőn lévő kontrollpontok vektora */
int selectedPoint = -1;                /** Épp áthúzás alatt álló pont indexe (-1 = nincs) */
bool dragging = false;             /** Drag and drop állapot jelzője */

float pointDiameter = 7.0f;            /** Kontrollpontok átmérője pixelben (3–9 között) */
float pointRadius = pointDiameter / 2.0f; /** Sugár pixelben */

/// De Casteljau algoritmus: Tetszőleges pontszámú Bézier-görbe értékszámítása
glm::vec2 deCasteljau(const std::vector<glm::vec2>& pts, float t) {
    /** Lokális másolat, amin iterálunk */
    std::vector<glm::vec2> tmp = pts;
    int n = (int)tmp.size();
    for (int k = 1; k < n; ++k) {            /** Rétegek száma = pontszám – 1 */
        for (int i = 0; i < n - k; ++i) {
            /** Lineáris interpoláció az alsó rétegen */
            tmp[i] = tmp[i] * (1.0f - t) + tmp[i + 1] * t;
        }
    }
    return tmp[0];                          /** Visszatérünk a végső ponttal */

}

/// Bézier-görbe kirajzolása törött vonallal (line strip)
void drawBezier(const std::vector<glm::vec2>& pts) {
    if (pts.size() < 2) return;            /** Legalább 2 pont kell a vonalhoz */
    glBegin(GL_LINE_STRIP);                /** Vonal kirajzolása */
    for (float t = 0.0f; t <= 1.0f; t += 0.01f) {
        /** Pontszámítás t szerint */
        glm::vec2 p = deCasteljau(pts, t);
        glVertex2f(p.x, p.y);              /** Pont kirajzolása */
    }
    glEnd();                               /** Vonal vége */
}

/// Minden kontrollpont kirajzolása kört formázva
void drawControlPoints() {
    for (auto& p : controlPoints) {
        glBegin(GL_TRIANGLE_FAN);           /** Kör approx */
        glVertex2f(p.x, p.y);              /** Kör közepe */
        int segments = 20;                 /** Szegmensek száma */
        for (int i = 0; i <= segments; ++i) {
            float theta = 2.0f * M_PI * i / segments; /** Szög */
            float x = p.x + pointRadius * cosf(theta);
            float y = p.y + pointRadius * sinf(theta);
            glVertex2f(x, y);              /** Kör kerületi pontja */
        }
        glEnd();                           /** Kör vége */
    }
}

/// Egér eseménykezelő: pont hozzáadás, eltávolítás, kiválasztás
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    my = WINDOW_HEIGHT - my;               /** Y tengely invertálása */

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            /** Először megnézzük, kiválasztottunk e pontot? */
            for (int i = 0; i < (int)controlPoints.size(); ++i) {
                glm::vec2 cp = controlPoints[i];
                float dx = (float)mx - cp.x, dy = (float)my - cp.y;
                if (dx * dx + dy * dy <= pointRadius * pointRadius) {
                    selectedPoint = i;     /** Pont index mentése */
                    dragging = true;       /** Drag kezdete */
                    return;
                }
            }
            /** Ha nem választottunk pontot, új pontot adunk hozzá. */
            controlPoints.emplace_back((float)mx, (float)my);
        }
        else if (action == GLFW_RELEASE) {
            dragging = false;            /** Drag vége. */
            selectedPoint = -1;          /** Nincs kiválasztva. */
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        /** Jobb klikk: pont törlése, ha az egér közelében van */
        for (int i = 0; i < (int)controlPoints.size(); ++i) {
            glm::vec2 cp = controlPoints[i];
            float dx = (float)mx - cp.x, dy = (float)my - cp.y;
            if (dx * dx + dy * dy <= pointRadius * pointRadius) {
                controlPoints.erase(controlPoints.begin() + i);
                return;
            }
        }
    }
}

/// Egér mozgatása: kiválasztott pont mozgatása
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    if (dragging && selectedPoint >= 0) {
        float x = (float)xpos;
        float y = WINDOW_HEIGHT - (float)ypos; /** Y tengely invert */
        controlPoints[selectedPoint] = glm::vec2(x, y);
    }
}

int main() {
    glfwInit();                            /** GLFW init */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    /** Compatibility profil, immediate mode-hoz */
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Bezier Gorbe", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glewInit();                            /** GLEW init */

/** Viewport és ortho projekció beállítása pixel-koordinátákhoz */
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);   /** Fekete háttér */

/** Eseménykezelők regisztrálása */
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);      /** Képernyő törlése */

        glColor3f(1.0f, 1.0f, 1.0f);       /** Fehér kontrollpontok */
        drawControlPoints();               /** Kontrollpontok kirajzolása */

        glLineWidth(2.0f);                 /** Görbe vonalvastagság */
        glColor3f(1.0f, 0.0f, 0.0f);       /** Piros görbe */
        drawBezier(controlPoints);         /** Bézier-görbe kirajzolása */

        glfwSwapBuffers(window);           /** Buffer csere */
        glfwPollEvents();                  /** Események kezelése */
    }

    glfwDestroyWindow(window);            /** Ablak törlése */
    glfwTerminate();                      /** GLFW leállítása */

    return 0;                             /** Normál kilépés */

}
