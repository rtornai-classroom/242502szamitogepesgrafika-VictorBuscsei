#include <GL/glew.h>           
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>


const GLint   WINDOW_WIDTH = 600;   /** Ablak szélessége pixelben */
const GLint   WINDOW_HEIGHT = 600;   /** Ablak magassága pixelben */
const GLfloat RADIUS = 50.0f; /** A kör sugara pixelben */
const GLfloat LINE_LENGTH = WINDOW_WIDTH / 3.0f; /** Vízszintes szakasz hossza ablak egyharmada */
const GLfloat LINE_THICKNESS = 3.0f; /** A szakasz vastagsága pixelben */

GLfloat circleX = WINDOW_WIDTH / 2.0f;  /** Kör kezdeti X koordinátája középen */
GLfloat circleY = WINDOW_HEIGHT / 2.0f; /** Kör Y koordinátája középen */
GLfloat dir = 5.0f;                /** Mozgás lépéshossza pixel/frame */

GLuint  program;                       /** Shader azonosító */
GLint   uCenterLoc, uRadiusLoc;        /** Kör középpont és sugár uniform-helyei */
GLint   uHalfSegLoc, uLineThickLoc;    /** Szakasz félhossz és vastagság uniform-helyei */

/// Beolvassa egy fájl tartalmát stringbe
std::string readFile(const char* path) {
    std::ifstream file(path);                          /** Fájl megnyitása olvasásra */
    std::stringstream ss;                              /** Stringstream a tartalomnak létrehozása*/
    ss << file.rdbuf();                                /** Fájltartalom beolvasása a stream-be */
    file.close();                                      /** Fájl lezárása */
    return ss.str();                                   /** Visszatérünk a tartalommal */
}

/// Shader létrehozása és fordítása
GLuint compileShader(const char* path, GLenum type) {
    GLuint shader = glCreateShader(type);              /** Shader objektum létrehozása */
    std::string src = readFile(path);                  /** Forrás beolvasása fájlból */
    const char* csrc = src.c_str();                    /** C-stílusú karaktertömb */
    glShaderSource(shader, 1, &csrc, nullptr);         /** Forrás hozzárendelése */
    glCompileShader(shader);                           /** Shader fordítása */
    GLint ok = GL_FALSE;                               /** Ellenőrizzük a fordítást */
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (ok != GL_TRUE) {                               /** Error esetén, hibakiírása */
        GLint len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');                              /** Hibalog tárhely, null-terminált */
        glGetShaderInfoLog(shader, len, nullptr, &log[0]);      /** Írás a string belső bufferébe */
        std::cerr << "Shader compile error:\n" << log << std::endl;
    }
    return shader;                                     /** Shader ID visszaadása */
}

/// Program létrehozása vertex és fragment shaderekből
GLuint createProgram(const char* vertPath, const char* fragPath) {
    GLuint vs = compileShader(vertPath, GL_VERTEX_SHADER);   /** Compile vertex shader */
    GLuint fs = compileShader(fragPath, GL_FRAGMENT_SHADER);   /** Compile fragment shader */
    GLuint prog = glCreateProgram();                         /** Program object létrehozása*/
    glAttachShader(prog, vs);                                /** Vertex csatolása */
    glAttachShader(prog, fs);                                /** Fragment csatolása */
    glLinkProgram(prog);                                     /** Program linkelése */
    GLint ok = GL_FALSE;                                     /** Link check */
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (ok != GL_TRUE) {                                     /** Error esetén, hibakiírása */
        GLint len = 0;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');                              /** Link-log tárhely, null-terminált */
        glGetProgramInfoLog(prog, len, nullptr, &log[0]);       /** Írás a string belső bufferébe */
        std::cerr << "Program link error:\n" << log << std::endl;
    }
    glDeleteShader(vs);                                       /** VertexShader törlése */
    glDeleteShader(fs);                                       /** FragmentShader törlése */
    return prog;                                              /** Program ID visszaadása */
}

/// Pozíció frissítése, ütközéskezeléssel.
void update() {
    GLfloat nextX=circleX+dir;                           /** Következő X pozíció */
    if (nextX + RADIUS > WINDOW_WIDTH) {                     /** Ha jobb fal ütközés */
        GLfloat over = nextX+RADIUS-WINDOW_WIDTH;        /** Túllövés kiszámolása */
        dir = -dir;                                          /** Irány megfordítása */
        nextX = WINDOW_WIDTH-RADIUS-over;                /** Pozíció korrekció */
    }
    else if (nextX - RADIUS < 0.0f) {                         /** Ha bal fal ütközés */
        GLfloat over = RADIUS - nextX;                       /** Túllövés kiszámolása */
        dir = -dir;                                          /** Irány megfordítása */
        nextX = RADIUS + over;                               /** Pozíció korrekció */
    }
    circleX = nextX;                                         /** Pozíció frissítése */
}

/// Kirajzolás: quad + circle+line a fragment shaderben.
void render() {
    glClear(GL_COLOR_BUFFER_BIT);                            /** Háttér törlése */
    glUseProgram(program);                                   /** Shader bekapcsolása */
    // uniforms átadása
    glUniform2f(uCenterLoc, circleX, circleY);               /** Kör középpontja */
    glUniform1f(uRadiusLoc, RADIUS);                         /** Kör sugara */
    glUniform1f(uHalfSegLoc, LINE_LENGTH/WINDOW_WIDTH);    /** Szakasz fél-hossza NDC-ben */
    glUniform1f(uLineThickLoc, LINE_THICKNESS);              /** Szakasz vastagsága pixelben */

    // teljes képernyős quad glBegin-módban
    glBegin(GL_QUADS);                                       /** Quad kirajzolása */
    glVertex2f(-1.0f, -1.0f);                              /** Bal alsó sarok */
    glVertex2f(1.0f, -1.0f);                              /** Jobb alsó sarok */
    glVertex2f(1.0f, 1.0f);                              /** Jobb felső sarok */
    glVertex2f(-1.0f, 1.0f);                              /** Bal felső sarok */
    glEnd();                                                 /** Quad vége */
}

int main() {
    glfwInit();                                              /** GLFW inicializálása */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);           /** OpenGL verzió major */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);           /** OpenGL verzió minor */
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

#ifdef __APPLE__ // To make macOS happy; should not be needed.
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Elso beadando", nullptr, nullptr);
    /** Ablak létrehozása */
    glfwMakeContextCurrent(window);                          /** Kontext kiválasztása */
    glewInit();                                              /** GLEW inicializálása */
    glfwSwapInterval(1);                                     /** V-sync bekapcsolása */

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);           /** Viewport beállítása */
    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);                    /** Sárga háttérszín */

    // shader készítése és uniform-helyek lekérése
    program = createProgram("vertexShader.glsl", "fragmentShader.glsl");
    uCenterLoc = glGetUniformLocation(program, "u_center"); /** u_center helyének lekérése */
    uRadiusLoc = glGetUniformLocation(program, "u_radius"); /** u_radius helyének lekérése */
    uHalfSegLoc = glGetUniformLocation(program, "u_halfSeg");   /** u_halfSeg helyének lekérése */
    uLineThickLoc = glGetUniformLocation(program, "u_lineThick");   /** u_lineThick helyének lekérése */


    // fő ciklus
    while (!glfwWindowShouldClose(window)) {
        update();                                            /** Pozíció frissítése */
        render();                                            /** Render frame */
        glfwSwapBuffers(window);                             /** Buffer csere */
        glfwPollEvents();                                    /** Események kezelése */
    }
    glfwDestroyWindow(window);                              /** Ablak törlése */
    glfwTerminate();                                        /** GLFW leállítása */

    return 0;
}
