// main.cpp
#include <iostream>
#include <cmath>
#include <cstdlib> // Para rand()
#include <ctime>   // Para srand()

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
#include "stb_image.h"

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp> // Para glm::linearRand

// Load Models / Utils
#include "SOIL2/SOIL2.h"
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

// Para el sonido
#include <windows.h>
#pragma comment(lib, "winmm.lib")

// Function prototypes
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Camera
Camera camera(glm::vec3(0.0f, 3.0f, 10.0f));
GLfloat lastX = WIDTH / 2.0f;
GLfloat lastY = HEIGHT / 2.0f;
bool keys[1024];
bool firstMouse = true;

// Deltatime
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;


// Variables de estado para cortina
bool cortinaCerrada = false; // false = abierta, true = cerrada
float factorCierre = 0.0f; // 0.0 = abiertas, 1.0 = cerradas
bool puertaAbierta = false;

// Variables para el péndulo
bool pendulumActive = false;
float phaseTime = 0.0f;
int currentPhase = 0;
const float maxAngle = 30.0f;
#define M_PI 3.14159265358979323846
const float period = 1.2; //para la velociad del pendulo, mientras mas pequeño mas rapido
const float omega = 2.0f * (float)M_PI / period;
const float quarterPeriodTime = (float)M_PI / (2.0f * omega);
const float halfPeriodTime = (float)M_PI / omega;
const float middleBallMaxAngle = 6.0f;  // Movimiento  de las esferas 2 y 3

// NUBES EN MOVIMIENTO
bool cloudsMoving = false;
float cloudsTime = 0.0f;
const float cloudsSpeed = 0.095f;
const float cloudsTile = 1.0f;

// VARIABLES DE ANIMACIÓN DEL PEZ 
bool fishSwimming = false;
const float FISH_FORWARD_SPEED = 0.09f;
float fishXOffset = 0.0f;

// Variables de la cola 
const float TAIL_AMPLITUDE = 10.0f; // 10 grados máximo
float tailAngle = 0.0f;
float tailTime = 0.0f;
const float TAIL_FREQ = 3.5f;
glm::vec3 tailPivotLocal(-2.985f, 0.603f, -2.57f);

float fishSwimTime = 0.0f;
bool fishFlipped = false;
const float FLIP_TIME = 5.0f; // 10 segundos
float flipRotation = 0.0f;
bool isFlipping = false;
float flipProgress = 0.0f;
const float FLIP_DURATION = 0.001f; // Duración del flip en segundos
glm::vec3 bodyPivotLocal(-3.7f, 6.03f, -2.57f);

// VARIABLES PARA EL SOL Y CICLO DÍA/NOCHE
bool sunAnimationActive = false;
float sunAngle = 0.0f; // 0 a 360 grados
const float SUN_ROTATION_TIME_SEC = 60.0f; // 1 minuto para 360 grados
const float SUN_SPEED_DEGREES_PER_SEC = 360.0f / SUN_ROTATION_TIME_SEC; // 6 grados/seg
const float SUN_DISTANCE = 56.144f;
glm::vec3 sunPos(0.0f, 0.0f, -SUN_DISTANCE); // Posición inicial y calculada

GLuint hdrTexture = 0;


int main()
{
    // Inicializar srand para números aleatorios
    srand(static_cast<unsigned int>(time(NULL)));

    // Init GLFW
    glfwInit();

    // Create window
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "320018277_Proyecto_final", nullptr, nullptr);
    if (nullptr == window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

    // Callbacks
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);

    // Init GLEW
    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    // Viewport and GL options
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_DEPTH_TEST);
    // Ensure opaque rendering by default
    glDisable(GL_BLEND);

    // Shaders
    Shader lightingShader("Shaders/lighting.vs", "Shaders/lighting.frag");
    Shader lampShader("Shaders/lamp.vs", "Shaders/lamp.frag");
    Shader cloudsShader("Shaders/clouds.vs", "Shaders/clouds.frag");
    Shader skydomeShader("Shaders/skydome.vs", "Shaders/skydome.frag");

    // Load models
    Model mesa((char*)"Models/mesa_dibujo/mesa_dibujo.obj");
    Model barril((char*)"Models/barril/barril.obj");
    Model pecera1((char*)"Models/pecera/pecera_1_opaco.obj");
    Model pecera2((char*)"Models/pecera/pecera_2_traslucido.obj");
    Model pez_cuerpo((char*)"Models/pecera/pez_cuerpo.obj");
    Model pez_cola((char*)"Models/pecera/pez_cola.obj");
    Model camas((char*)"Models/camas/camas_ff.obj");
    Model pendulo((char*)"Models/pendulo/pendulo_estructura_opaco.obj");
    Model escritorio((char*)"Models/escritorio_phineas/escritorio_phineas.obj");
    Model ball1((char*)"Models/pendulo/ball_1_traslucido.obj");
    Model ball2((char*)"Models/pendulo/ball_2_traslucido.obj");
    Model ball3((char*)"Models/pendulo/ball_3_traslucido.obj");
    Model ball4((char*)"Models/pendulo/ball_4_traslucido.obj");
    Model cabina((char*)"Models/cabina/cabina_opaco.obj");
    Model cabina2((char*)"Models/cabina/cabina_vidrio.obj");
    Model casa((char*)"Models/casa/casa.obj");
    Model ventanas((char*)"Models/casa/ventanas_casa.obj");
    Model cuarto_phineas_paredes((char*)"Models/casa/cuarto_phineas_paredes.obj");
    Model cuarto_phineas_techo((char*)"Models/casa/cuarto_phineas_techo.obj");
    Model puerta_cuarto_phineas((char*)"Models/casa/puerta_cuarto_phineas.obj");
    Model puerta_redonda((char*)"Models/casa/puerta_redonda.obj");
    Model puerta_cuadrada((char*)"Models/casa/puerta_cuadrada.obj");

    // PARES DE CORTINAS 
    // Par 1 (Z)
    Model cortina_pa_z_neg((char*)"Models/casa/cortinas_pa_z_neg.obj");
    Model cortina_pa_z_pos((char*)"Models/casa/cortinas_pa_z_pos.obj");
    // Par 2 (Z)
    Model cortina_pb_z_neg((char*)"Models/casa/cortina_pb_z_neg.obj");
    Model cortina_pb_z_pos((char*)"Models/casa/cortina_pb_z_pos.obj");
    // Par 3 (X)
    Model cortina_pb_2_x_neg((char*)"Models/casa/cortina_pb_2_x_neg.obj");
    Model cortina_pb_2_x_pos((char*)"Models/casa/cortina_pb_2_x_pos.obj");
    // Par 4 (X)
    Model cortina_pa_2_x_neg((char*)"Models/casa/cortinas_pa_2_x_neg.obj");
    Model cortina_pa_2_x_pos((char*)"Models/casa/cortinas_pa_2_x_pos.obj");
    // Par 5 (X)
    Model cortina_pa_x_neg((char*)"Models/casa/cortinas_pa_x_neg.obj");
    Model cortina_pa_x_pos((char*)"Models/casa/cortinas_pa_x_pos.obj");
    // Par 6 (X)
    Model cortina_pb_x_neg((char*)"Models/casa/cortina_pb_x_neg.obj");
    Model cortina_pb_x_posi((char*)"Models/casa/cortina_pb_x_posi.obj");

    // Sol y luces
    Model sol((char*)"Models/casa/sol.obj");
    Model lampara_phineas((char*)"Models/casa/lampara_phineas.obj");
    Model pasto((char*)"Models/casa/piso.obj");

    Model skydome((char*)"Models/skybox/skydome.obj");

    // --- NUEVOS MODELOS: CUARTO DE CANDACE ---
    Model cama_candace((char*)"Models/candace_cama/candace_cama.obj");
    Model puerta_candace((char*)"Models/cuarto_candace/puerta_candace.obj");
    Model microfono_candace((char*)"Models/cuarto_candace/microfono.obj");
    Model baul1((char*)"Models/cuarto_candace/baul1.obj");
    Model baul2((char*)"Models/cuarto_candace/baul2_tapa.obj");
    Model cuarto((char*)"Models/cuarto_candace/cuarto_candace.obj");


    GLuint whiteSpecTex = 0;
    GLuint blackSpecTex = 0;
    {

        unsigned char whitePixel[3] = { 255, 255, 255 };
        glGenTextures(1, &whiteSpecTex);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, whiteSpecTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, whitePixel);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        unsigned char blackPixel[3] = { 0, 0, 0 };
        glGenTextures(1, &blackSpecTex);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, blackSpecTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, blackPixel);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // CARGAR TEXTURA HDR (SKYDOME)
    stbi_set_flip_vertically_on_load(true); // El cielo a veces viene invertido
    int hdrWidth, hdrHeight, hdrNrChannels;
    float* hdrData = stbi_loadf("skybox/cielo.hdr", &hdrWidth, &hdrHeight, &hdrNrChannels, 0);

    if (hdrData)
    {
        glGenTextures(1, &hdrTexture);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, hdrWidth, hdrHeight, 0, GL_RGB, GL_FLOAT, hdrData);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        std::cout << "Textura HDR cargada." << std::endl;
        stbi_image_free(hdrData);
    }
    else
    {
        std::cout << "Error al cargar la textura HDR." << std::endl;
    }
    stbi_set_flip_vertically_on_load(false); // Regresa al estado normal
    // --- FIN CARGA HDR ---


    // Projection
    glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.01f, 100.0f);

    // MAIN LOOP
    while (!glfwWindowShouldClose(window))
    {
        // Timing
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Actualizar tiempo de animación si el péndulo está activo
        if (pendulumActive) {
            phaseTime += deltaTime;
        }

        // Actualizar tiempo de nubes si están activas
        if (cloudsMoving) {
            cloudsTime += deltaTime;
        }

        if (fishSwimming) {
            fishSwimTime += deltaTime;
            tailTime += deltaTime;

            if (fishSwimTime >= FLIP_TIME && !isFlipping) {
                isFlipping = true;
                flipProgress = 0.0f;
                fishSwimTime = 0.0f; // Reiniciar el contador para el próximo flip
                std::cout << "Pez iniciando flip!" << std::endl;
            }

            // Animación del flip (CÍCLICA)
            if (isFlipping) {
                flipProgress += deltaTime;
                float flipPercent = glm::clamp(flipProgress / FLIP_DURATION, 0.0f, 1.0f);

                if (fishFlipped) {
                    flipRotation = 180.0f + (180.0f * flipPercent);
                }
                else {
                    flipRotation = 180.0f * flipPercent;
                }

                if (flipProgress >= FLIP_DURATION) {
                    isFlipping = false;
                    fishFlipped = !fishFlipped;

                    if (fishFlipped) {
                        flipRotation = 180.0f;
                    }
                    else {
                        flipRotation = 0.0f;
                    }
                }
            }

            if (!fishFlipped) {
                fishXOffset -= FISH_FORWARD_SPEED * deltaTime;
            }
            else {
                fishXOffset += FISH_FORWARD_SPEED * deltaTime;
            }

            tailAngle = TAIL_AMPLITUDE * sin(tailTime * TAIL_FREQ);
        }

        if (sunAnimationActive) {
            // 1. Avanzar el ángulo
            sunAngle += SUN_SPEED_DEGREES_PER_SEC * deltaTime;
            if (sunAngle >= 360.0f) {
                sunAngle -= 360.0f; // Resetear el ángulo
            }

            glm::mat4 sunRotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(sunAngle), glm::vec3(1.0f, 0.0f, 0.0f));

            glm::vec4 initialPos = glm::vec4(0.0f, 0.0f, -SUN_DISTANCE, 1.0f);

            sunPos = glm::vec3(sunRotationMatrix * initialPos);
        }

        // Input
        glfwPollEvents();
        DoMovement();

        glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.01f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glDepthFunc(GL_LEQUAL);
        skydomeShader.Use();

        // Prueba cambiando este 1.0 (ej. 0.5 es más oscuro, 2.0 es más brillante)
        glUniform1f(glGetUniformLocation(skydomeShader.Program, "exposure"), 1.0f);

        glm::mat4 skyView = glm::mat4(glm::mat3(view));

        glUniformMatrix4fv(glGetUniformLocation(skydomeShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(skyView));
        glUniformMatrix4fv(glGetUniformLocation(skydomeShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Enlazar la textura HDR
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(skydomeShader.Program, "skydomeTexture"), 0);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);

        skydome.Draw(skydomeShader);

        glDepthFunc(GL_LESS);

        // Use lighting shader
        lightingShader.Use();
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        GLint diffLoc = glGetUniformLocation(lightingShader.Program, "material.diffuse");
        GLint specLoc = glGetUniformLocation(lightingShader.Program, "material.specular");
        if (diffLoc != -1) glUniform1i(diffLoc, 0); // diffuse -> texture unit 0
        if (specLoc != -1) glUniform1i(specLoc, 1); // specular -> texture unit 1



        if (sunAnimationActive)
        {

            // 0.0 = noche, 1.0 = mediodía
            float dayFactor = 0.0f;
            if (sunAngle >= 0.0f && sunAngle <= 180.0f) {
                dayFactor = sin(glm::radians(sunAngle));
            }

            glm::vec3 nightAmbient(0.03f, 0.03f, 0.03f);
            glm::vec3 nightDiffuse(0.0f, 0.0f, 0.0f);
            glm::vec3 nightSpecular(0.0f, 0.0f, 0.0f);
            glm::vec3 dayDirAmbient(0.3f, 0.3f, 0.3f);
            glm::vec3 dayDirDiffuse(0.7f, 0.7f, 0.7f);
            glm::vec3 dayDirSpecular(1.0f, 1.0f, 1.0f);
            float maxPointIntensity = 5.0f;
            glm::vec3 dayPointDiffuse = glm::vec3(1.0f, 1.0f, 0.9f) * maxPointIntensity;
            glm::vec3 dayPointSpecular = glm::vec3(1.0f, 1.0f, 1.0f) * maxPointIntensity;

            glm::vec3 currentDirAmbient = glm::mix(nightAmbient, dayDirAmbient, dayFactor);
            glm::vec3 currentDirDiffuse = glm::mix(nightDiffuse, dayDirDiffuse, dayFactor);
            glm::vec3 currentDirSpecular = glm::mix(nightSpecular, dayDirSpecular, dayFactor);
            glm::vec3 currentPointDiffuse = glm::mix(nightDiffuse, dayPointDiffuse, dayFactor);
            glm::vec3 currentPointSpecular = glm::mix(nightSpecular, dayPointSpecular, dayFactor);

            glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), currentDirAmbient.x, currentDirAmbient.y, currentDirAmbient.z);
            glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), currentDirDiffuse.x, currentDirDiffuse.y, currentDirDiffuse.z);
            glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), currentDirSpecular.x, currentDirSpecular.y, currentDirSpecular.z);

            std::string base = "pointLights[0]";
            glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".position").c_str()), sunPos.x, sunPos.y, sunPos.z);
            glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".ambient").c_str()), 0.0f, 0.0f, 0.0f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".diffuse").c_str()), currentPointDiffuse.x, currentPointDiffuse.y, currentPointDiffuse.z);
            glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".specular").c_str()), currentPointSpecular.x, currentPointSpecular.y, currentPointSpecular.z);
            glUniform1f(glGetUniformLocation(lightingShader.Program, (base + ".constant").c_str()), 1.0f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, (base + ".linear").c_str()), 0.007f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, (base + ".quadratic").c_str()), 0.0002f);

            // Lámpara Phineas
            base = "pointLights[1]";
            glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".position").c_str()), -1.45779f, 8.159f, -1.136f);
            // Atenuación para una luz de cuarto
            glUniform1f(glGetUniformLocation(lightingShader.Program, (base + ".constant").c_str()), 1.0f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, (base + ".linear").c_str()), 0.35f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, (base + ".quadratic").c_str()), 0.44f);

            if (dayFactor > 0.0f) {
                // DÍA: Lámpara APAGADA
                glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".ambient").c_str()), 0.0f, 0.0f, 0.0f);
                glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".diffuse").c_str()), 0.0f, 0.0f, 0.0f);
                glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".specular").c_str()), 0.0f, 0.0f, 0.0f);
            }
            else {
                // NOCHE: Lámpara ENCENDIDA (luz cálida)
                glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".ambient").c_str()), 0.05f, 0.05f, 0.0f);
                glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".diffuse").c_str()), 2.5f, 2.5f, 2.5f);
                glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".specular").c_str()), 2.0f, 2.0f, 2.0f);
            }

            for (int i = 2; i < 4; ++i)
            {
                base = "pointLights[" + std::to_string(i) + "]";
                glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".position").c_str()), 0.0f, 0.0f, 0.0f);
                glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".ambient").c_str()), 0.0f, 0.0f, 0.0f);
                glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".diffuse").c_str()), 0.0f, 0.0f, 0.0f);
                glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".specular").c_str()), 0.0f, 0.0f, 0.0f);
                glUniform1f(glGetUniformLocation(lightingShader.Program, (base + ".constant").c_str()), 1.0f);
                glUniform1f(glGetUniformLocation(lightingShader.Program, (base + ".linear").c_str()), 0.0f);
                glUniform1f(glGetUniformLocation(lightingShader.Program, (base + ".quadratic").c_str()), 0.0f);
            }
        }
        else
        {


            glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), 0.3f, 0.3f, 0.3f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 0.7f, 0.7f, 0.7f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), 1.0f, 1.0f, 1.0f);

            for (int i = 0; i < 4; ++i)
            {
                std::string base = "pointLights[" + std::to_string(i) + "]";
                glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".position").c_str()), 0.0f, 0.0f, 0.0f);
                glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".ambient").c_str()), 0.0f, 0.0f, 0.0f);
                glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".diffuse").c_str()), 0.0f, 0.0f, 0.0f);
                glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".specular").c_str()), 0.0f, 0.0f, 0.0f);
                glUniform1f(glGetUniformLocation(lightingShader.Program, (base + ".constant").c_str()), 1.0f);
                glUniform1f(glGetUniformLocation(lightingShader.Program, (base + ".linear").c_str()), 0.0f);
                glUniform1f(glGetUniformLocation(lightingShader.Program, (base + ".quadratic").c_str()), 0.0f);
            }
        }

        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.position"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.direction"), camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.ambient"), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.diffuse"), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.specular"), 0.0f, 0.0f, 0.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.linear"), 0.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.quadratic"), 0.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.cutOff"), glm::cos(glm::radians(12.0f)));
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(15.0f)));

        // Camera position uniform
        GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
        glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);



        // MODEL: identity (no translate/rotate/scale)
        glm::mat4 model = glm::mat4(1.0f);
        // Guardar la ubicación del uniform del modelo
        GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));


        // Ensure default opaque uniforms (in case shader expects them)
        GLint trasLoc = glGetUniformLocation(lightingShader.Program, "tras");
        if (trasLoc != -1) glUniform4f(trasLoc, 1.0f, 1.0f, 1.0f, 1.0f);
        GLint activaTransLoc = glGetUniformLocation(lightingShader.Program, "activaTrans");
        if (activaTransLoc != -1) glUniform1f(activaTransLoc, 0.0f);

        // Important: ensure blending disabled and depth mask enabled (opaque) by default
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);


        // DIBUJAR 


        // CASA (opaca - mate)
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 1.0f); // Muy bajo - mate
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, blackSpecTex);
        model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        casa.Draw(lightingShader);

        // PAREDES PHIENS
        cuarto_phineas_paredes.Draw(lightingShader);

        // techo con animacion de nubes
        cloudsShader.Use(); // CAMBIAR SHADER

        glUniformMatrix4fv(glGetUniformLocation(cloudsShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model)); // model sigue siendo identity
        glUniformMatrix4fv(glGetUniformLocation(cloudsShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(cloudsShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1f(glGetUniformLocation(cloudsShader.Program, "time"), cloudsTime);
        glUniform1i(glGetUniformLocation(cloudsShader.Program, "cloudsMoving"), cloudsMoving);
        glUniform1f(glGetUniformLocation(cloudsShader.Program, "speed"), cloudsSpeed);
        glUniform1f(glGetUniformLocation(cloudsShader.Program, "tileFactor"), cloudsTile);
        glUniform1i(glGetUniformLocation(cloudsShader.Program, "texture1"), 0);

        cuarto_phineas_techo.Draw(cloudsShader);

        // volver al shader
        lightingShader.Use();
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
        glUniform1i(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0);
        glUniform1i(glGetUniformLocation(lightingShader.Program, "material.specular"), 1);

        //  DIBUJAR PUERTA PHINEAS (mate, opaca) 
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 1.0f);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, blackSpecTex);

        glm::vec3 pivotPuerta(-3.975f, 5.921f, 0.4826f);
        float rotPuerta = puertaAbierta ? 90.0f : 0.0f;

        glm::mat4 modelPuerta = glm::mat4(1.0f);
        modelPuerta = glm::translate(modelPuerta, pivotPuerta);
        modelPuerta = glm::rotate(modelPuerta, glm::radians(rotPuerta), glm::vec3(0.0f, 1.0f, 0.0f));
        modelPuerta = glm::translate(modelPuerta, -pivotPuerta);

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPuerta));
        puerta_cuarto_phineas.Draw(lightingShader);

        // --- DIBUJAR PUERTA REDONDA ---
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 1.0f);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, blackSpecTex);

        glm::vec3 pivotPuertaRedonda(-11.8534f, 2.21575f, 7.0159f);

        glm::mat4 modelPuertaRedonda = glm::mat4(1.0f);

        if (puertaAbierta) {
            glm::mat4 translateToPivot = glm::translate(glm::mat4(1.0f), pivotPuertaRedonda);
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 translateFromPivot = glm::translate(glm::mat4(1.0f), -pivotPuertaRedonda);
            modelPuertaRedonda = translateToPivot * rotation * translateFromPivot;
        }

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPuertaRedonda));
        puerta_redonda.Draw(lightingShader);

        // DIBUJAR PUERTA CUADRADA 
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 1.0f);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, blackSpecTex);

        glm::vec3 pivotPuertaCuadrada(0.809f, 1.987f, -5.4f);

        glm::mat4 modelPuertaCuadrada = glm::mat4(1.0f);

        if (puertaAbierta) {
            glm::mat4 translateToPivot = glm::translate(glm::mat4(1.0f), pivotPuertaCuadrada);
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 translateFromPivot = glm::translate(glm::mat4(1.0f), -pivotPuertaCuadrada);
            modelPuertaCuadrada = translateToPivot * rotation * translateFromPivot;
        }

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPuertaCuadrada));
        puerta_cuadrada.Draw(lightingShader);


        // CORTINAS 
        float distancia_pa_z = 0.9f;
        float distancia_pb_z = 0.8f;
        float distancia_pb_z_2 = 1.2;
        float distancia_pb_2_x = 1.1f;
        float distancia_pa_2_x = 0.32f;
        float distancia_pa_x = 0.45f;
        float distancia_pb_x = 1.42f;

        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 1.0f);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, blackSpecTex);

        glm::mat4 modelCortina = glm::mat4(1.0f);

        // Par 1 (Z)
        modelCortina = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, factorCierre * -distancia_pa_z));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCortina));
        cortina_pa_z_neg.Draw(lightingShader);

        modelCortina = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, factorCierre * distancia_pa_z));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCortina));
        cortina_pa_z_pos.Draw(lightingShader);

        // Par 2 (Z)
        modelCortina = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, factorCierre * -distancia_pb_z_2));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCortina));
        cortina_pb_z_neg.Draw(lightingShader);

        modelCortina = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, factorCierre * distancia_pb_z));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCortina));
        cortina_pb_z_pos.Draw(lightingShader);

        // Par 3 (X)
        modelCortina = glm::translate(glm::mat4(1.0f), glm::vec3(factorCierre * -distancia_pb_2_x, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCortina));
        cortina_pb_2_x_neg.Draw(lightingShader);

        modelCortina = glm::translate(glm::mat4(1.0f), glm::vec3(factorCierre * distancia_pb_2_x, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCortina));
        cortina_pb_2_x_pos.Draw(lightingShader);

        // Par 4 (X)
        modelCortina = glm::translate(glm::mat4(1.0f), glm::vec3(factorCierre * -distancia_pa_2_x, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCortina));
        cortina_pa_2_x_neg.Draw(lightingShader);

        modelCortina = glm::translate(glm::mat4(1.0f), glm::vec3(factorCierre * distancia_pa_2_x, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCortina));
        cortina_pa_2_x_pos.Draw(lightingShader);

        // Par 5 (X)
        modelCortina = glm::translate(glm::mat4(1.0f), glm::vec3(factorCierre * -distancia_pa_x, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCortina));
        cortina_pa_x_neg.Draw(lightingShader);

        modelCortina = glm::translate(glm::mat4(1.0f), glm::vec3(factorCierre * distancia_pa_x, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCortina));
        cortina_pa_x_pos.Draw(lightingShader);

        // Par 6 (X)
        modelCortina = glm::translate(glm::mat4(1.0f), glm::vec3(factorCierre * -distancia_pb_x, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCortina));
        cortina_pb_x_neg.Draw(lightingShader);

        modelCortina = glm::translate(glm::mat4(1.0f), glm::vec3(factorCierre * distancia_pb_x, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCortina));
        cortina_pb_x_posi.Draw(lightingShader);


        float windowsOpacity = 0.20f;

        // Enable blending para objetos translúcidos
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, whiteSpecTex);
        if (activaTransLoc != -1) glUniform1f(activaTransLoc, 1.0f);
        if (trasLoc != -1) glUniform4f(trasLoc, 1.0f, 1.0f, 1.0f, windowsOpacity);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);
        model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        ventanas.Draw(lightingShader);

        // Restaurar estado opaco
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        if (activaTransLoc != -1) glUniform1f(activaTransLoc, 0.0f);
        if (trasLoc != -1) glUniform4f(trasLoc, 1.0f, 1.0f, 1.0f, 1.0f);


        // DIBUJAR OTROS OBJETOS OPACOS

        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        if (activaTransLoc != -1) glUniform1f(activaTransLoc, 0.0f);
        if (trasLoc != -1) glUniform4f(trasLoc, 1.0f, 1.0f, 1.0f, 1.0f);

        // MESA (completamente mate)
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 1.0f);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, blackSpecTex);
        model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        mesa.Draw(lightingShader);

        // ESCRITORIO (opaco - mate)
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 1.0f);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, blackSpecTex);
        model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        escritorio.Draw(lightingShader);

        // BARRIL (metal - brillante)
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 256.0f);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, whiteSpecTex);
        model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        barril.Draw(lightingShader);

        // PECERA 1 (opaca - mate)
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 2.0f);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, blackSpecTex);
        model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        pecera1.Draw(lightingShader);

        // PEZ
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 4.0f);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, blackSpecTex);

        glm::mat4 bodyModel = glm::mat4(1.0f);

        if (isFlipping || fishFlipped) {
            bodyModel = glm::translate(bodyModel, bodyPivotLocal);
            bodyModel = glm::rotate(bodyModel, glm::radians(flipRotation), glm::vec3(0.0f, 1.0f, 0.0f));
            bodyModel = glm::translate(bodyModel, -bodyPivotLocal);
        }


        if (!fishFlipped) {
            bodyModel = glm::translate(bodyModel, glm::vec3(fishXOffset, 0.0f, 0.0f));
        }
        else {
            bodyModel = glm::translate(bodyModel, glm::vec3(-1.2, 0.0f, 0.0f));

            bodyModel = glm::translate(bodyModel, glm::vec3(-fishXOffset, 0.0f, 0.0f));
        }

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(bodyModel));
        pez_cuerpo.Draw(lightingShader);

        float currentTailAngle = fishSwimming ? tailAngle : 0.0f;

        glm::mat4 tailModel = bodyModel; // Comenzar con la transformación del cuerpo

        //  rotación pivote  cola
        tailModel = glm::translate(tailModel, tailPivotLocal);
        tailModel = glm::rotate(tailModel, glm::radians(currentTailAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        tailModel = glm::translate(tailModel, -tailPivotLocal);

        // 4. Dibujar cola
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(tailModel));
        pez_cola.Draw(lightingShader);

        // DRAW CAMAS (opacas - mate)
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 1.0f);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, blackSpecTex);
        model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        camas.Draw(lightingShader);

        // PASTO
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 1.0f); // Mate
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, whiteSpecTex);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, blackSpecTex);


        model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        pasto.Draw(lightingShader);

        // DIBUJAR LAMPARA PHINEAS
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 1.0f);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, blackSpecTex);
        model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        lampara_phineas.Draw(lightingShader);

        // --- DIBUJAR CUARTO DE CANDACE (Opacos y Mate) ---
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 1.0f);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, blackSpecTex); // Sin brillo especular

        // Resetear textura difusa a blanco por si los modelos no tienen textura propia
        // Esto evita que se "pinten" con la textura del objeto anterior (pasto/lampara)
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, whiteSpecTex);

        model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        cama_candace.Draw(lightingShader);

        // --- PUERTA CANDACE (ANIMADA) ---
       // Definir el pivote de la puerta de Candace
        glm::vec3 pivotPuertaCandace(6.0632f, 5.92179f, 0.9839f);

        // Usar la misma variable de estado 'puertaAbierta' para sincronizar
        // NOTA: Ajusta el ángulo (-90.0f o 90.0f) dependiendo de hacia dónde debe abrir
        float rotPuertaCandace = puertaAbierta ? 90.0f : 0.0f;

        glm::mat4 modelPuertaC = glm::mat4(1.0f);
        // 1. Mover al pivote
        modelPuertaC = glm::translate(modelPuertaC, pivotPuertaCandace);
        // 2. Rotar
        modelPuertaC = glm::rotate(modelPuertaC, glm::radians(rotPuertaCandace), glm::vec3(0.0f, 1.0f, 0.0f));
        // 3. Mover de regreso desde el pivote
        modelPuertaC = glm::translate(modelPuertaC, -pivotPuertaCandace);

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPuertaC));
        puerta_candace.Draw(lightingShader);

        // --- Restaurar matriz modelo a identidad para los siguientes objetos ---
        model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        microfono_candace.Draw(lightingShader);
        baul1.Draw(lightingShader);
        baul2.Draw(lightingShader);
        cuarto.Draw(lightingShader);


        // DRAW CABINA (opaco - metal brillante)
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 256.0f); // Valor alto para metal
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, whiteSpecTex); //  textura blanca para reflejos
        model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        cabina.Draw(lightingShader);

        // DRAW PÉNDULO ESTRUCTURA (plástico opaco)
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 16.0f);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, whiteSpecTex);
        model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        pendulo.Draw(lightingShader);

        // DRAW ESFERAS DEL PÉNDULO (metal opaco - brillante)
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 256.0f);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, whiteSpecTex);

        // Calcular ángulos para la animación
        float angle1 = 0.0f;
        float angle4 = 0.0f;
        float angleMiddle = 0.0f;  // Para ball2 y ball3
        bool advancePhase = false;

        if (pendulumActive) {
            if (currentPhase == 0) {
                // Fase inicial: esfera1 de -maxAngle a 0
                angle1 = -maxAngle * cos(omega * phaseTime);
                if (phaseTime >= quarterPeriodTime) {
                    advancePhase = true;
                }
            }
            else if (currentPhase % 2 == 1) {
                // Fases impares: esfera4 de 0 a +maxAngle y de regreso
                angle4 = maxAngle * sin(omega * phaseTime);
                // Movimiento  en esferas 2 y 3 (hacia la derecha)
                angleMiddle = middleBallMaxAngle * sin(omega * phaseTime);
                if (phaseTime >= halfPeriodTime) {
                    advancePhase = true;
                }
            }
            else {
                // Fases pares: esfera1 de 0 a -maxAngle y de regreso
                angle1 = -maxAngle * sin(omega * phaseTime);
                // Movimiento  en esferas 2 y 3 (hacia la izquierda)
                angleMiddle = -middleBallMaxAngle * sin(omega * phaseTime);
                if (phaseTime >= halfPeriodTime) {
                    advancePhase = true;
                }
            }

            if (advancePhase) {
                currentPhase++;
                phaseTime = 0.0f;
                PlaySound(TEXT("sonido/golpe_esferas.wav"), NULL, SND_FILENAME | SND_ASYNC);
            }
        }

        // Pivotes
        glm::vec3 pivot1(-0.8331f, 6.6857f, -2.7448f);
        glm::vec3 pivot2(-0.8331f, 6.6857f, -2.7448f);  // ball2
        glm::vec3 pivot3(-0.7983f, 6.6857f, -2.7448f);  // ball3
        glm::vec3 pivot4(-0.7636f, 6.6857f, -2.7448f);

        // Dibujar ball1
        model = glm::mat4(1.0f);
        model = glm::translate(model, pivot1);
        model = glm::rotate(model, glm::radians(angle1), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, -pivot1);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        ball1.Draw(lightingShader);

        // Dibujar ball2 (con movimiento sutil)
        model = glm::mat4(1.0f);
        model = glm::translate(model, pivot2);
        model = glm::rotate(model, glm::radians(angleMiddle), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, -pivot2);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        ball2.Draw(lightingShader);

        // Dibujar ball3 (con movimiento sutil)
        model = glm::mat4(1.0f);
        model = glm::translate(model, pivot3);
        model = glm::rotate(model, glm::radians(angleMiddle), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, -pivot3);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        ball3.Draw(lightingShader);

        // Dibujar ball4
        model = glm::mat4(1.0f);
        model = glm::translate(model, pivot4);
        model = glm::rotate(model, glm::radians(angle4), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, -pivot4);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        ball4.Draw(lightingShader);

        // DIBUJAR OBJETOS TRANSLÚCIDOS

        // --- Configuración para TODOS los translúcidos ---
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE); // No escribir en Z-buffer

        // Usar textura specular blanca para todos los vidrios
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, whiteSpecTex);
        if (activaTransLoc != -1) glUniform1f(activaTransLoc, 1.0f); // Activar modo transparencia en shader

        // DRAW PECERA 2 (vidrio translúcido con brillo medio)
        float peceraOpacity = 0.15f; // 75% opaco (25% transparente)
        if (trasLoc != -1) glUniform4f(trasLoc, 1.0f, 1.0f, 1.0f, peceraOpacity);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f); // Medio - vidrio
        model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        pecera2.Draw(lightingShader);

        // DRAW CABINA 2 (vidrio 90% translúcido)

        float cabinaOpacity = 0.40f; // 40% opaco (60% transparente)
        if (trasLoc != -1) glUniform4f(trasLoc, 1.0f, 1.0f, 1.0f, cabinaOpacity);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f); // Brillo medio (igual que pecera)
        model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        cabina2.Draw(lightingShader);


        // SOL
        float sunOpacity = 0.75f;
        if (trasLoc != -1) glUniform4f(trasLoc, 1.0f, 1.0f, 1.0f, sunOpacity);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 16.0f);

        model = glm::mat4(1.0f);

        model = glm::rotate(model, glm::radians(sunAngle), glm::vec3(1.0f, 0.0f, 0.0f));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        sol.Draw(lightingShader);


        // --- Restaurar estado opaco ---
        glDepthMask(GL_TRUE); // Reactivar escritura de profundidad
        glDisable(GL_BLEND);
        if (activaTransLoc != -1) glUniform1f(activaTransLoc, 0.0f);
        if (trasLoc != -1) glUniform4f(trasLoc, 1.0f, 1.0f, 1.0f, 1.0f);


        // Unbind specular texture
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Swap buffers
        glfwSwapBuffers(window);
    }


    // cleanup created textures
    if (whiteSpecTex != 0) glDeleteTextures(1, &whiteSpecTex);
    if (blackSpecTex != 0) glDeleteTextures(1, &blackSpecTex);
    if (hdrTexture != 0) glDeleteTextures(1, &hdrTexture);

    glfwTerminate();
    return 0;
}


// Moves/alters the camera positions based on user input
void DoMovement()
{
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP]) camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN]) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT]) camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) camera.ProcessKeyboard(RIGHT, deltaTime);
}

// Key callback
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
    {
        glfwSetWindowShouldClose(window, GL_TRUE); // <-- Corregido
    }

    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS) keys[key] = true;
        else if (action == GLFW_RELEASE) keys[key] = false;
    }

    // (Lógica para teclas 1 y 2 eliminada)

    // **Control de CORTINAS y PUERTA con tecla 5**
    if (key == GLFW_KEY_5 && action == GLFW_PRESS)
    {
        cortinaCerrada = !cortinaCerrada;
        puertaAbierta = !puertaAbierta;

        if (cortinaCerrada)
        {
            // Cerrar (Mover)
            factorCierre = 1.0f; // 1.0 significa "cerradas"
            std::cout << "Cortinas y puertas: CERRADAS" << std::endl;
        }
        else
        {
            // Abrir (volver a la posición 0)
            factorCierre = 0.0f; // 0.0 significa "abiertas"
            std::cout << "Cortinas y puertas: ABIERTAS" << std::endl;
        }
    }

    // Control del péndulo con tecla 4
    if (key == GLFW_KEY_4 && action == GLFW_PRESS)
    {
        pendulumActive = !pendulumActive;
        if (pendulumActive) {
            phaseTime = 0.0f;
            currentPhase = 0;
            std::cout << "Péndulo activado" << std::endl;
        }
        else {
            phaseTime = 0.0f;
            currentPhase = 0;
            std::cout << "Péndulo desactivado" << std::endl;
        }
    }

    // Control de nubes con tecla 3
    if (key == GLFW_KEY_3 && action == GLFW_PRESS)
    {
        cloudsMoving = !cloudsMoving;
        if (cloudsMoving) {
            std::cout << "Nubes: ACTIVADAS (presiona 3 para detener)" << std::endl;
        }
        else {
            std::cout << "Nubes: DETENIDAS" << std::endl;
        }
    }

    // --- En KeyCallback, resetear las variables cuando se pare el pez ---
    if (key == GLFW_KEY_2 && action == GLFW_PRESS)
    {
        fishSwimming = !fishSwimming;
        // Reiniciar timers cuando se inicia
        if (fishSwimming) {
            fishSwimTime = 0.0f;
            isFlipping = false;
            flipProgress = 0.0f;
            // no tocar fishXOffset aquí; el movimiento ocurre en main loop
            std::cout << "Pez nadando: ON" << std::endl;
        }
        else {
            std::cout << "El pez no está nadando" << std::endl;
        }
    }

    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        sunAnimationActive = !sunAnimationActive;
        if (sunAnimationActive) {
            std::cout << "Animacion Sol: ACTIVADA" << std::endl;
        }
        else {
            std::cout << "Animacion Sol: DESACTIVADA" << std::endl;
            // Opcional: resetear el sol a su posición inicial
            // sunAngle = 0.0f;
            // sunPos = glm::vec3(0.0f, 0.0f, -SUN_DISTANCE);
        }
    }
}

// Mouse callback
void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMouse)
    {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }

    GLfloat xOffset = xPos - lastX;
    GLfloat yOffset = lastY - yPos; // reversed

    lastX = xPos;
    lastY = yPos;

    camera.ProcessMouseMovement(xOffset, yOffset);
}