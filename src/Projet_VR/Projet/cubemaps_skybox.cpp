#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <opengl/filesystem.h>
#include <opengl/shader_m.h>
#include <opengl/camera.h>
#include <opengl/model.h>
#include <vector>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
unsigned int loadCubemap(vector<std::string> faces);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 10.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//lighting
glm::vec3 lightPos(1.2f, 1.0f, -8.0f);//  (1.2f, 1.0f, 2.0f);

// Structure pour représenter une particule
// Déclaration des structures pour les particules
struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec4 color;
    float life;

    Particle()
        : position(-0.19f, 0.90f, 0.0f), velocity(2.0f), color(1.0f), life(2.0f) {}
};
const int maxParticles = 10000;
std::vector<Particle> particles;

// Fonction utilitaire pour mettre à jour les particules
void updateParticles(std::vector<Particle>& particles, float deltaTime) {
    for (auto& particle : particles) {
        // Mettre à jour la position en fonction de la vitesse
        particle.position += particle.velocity * deltaTime;

        // Mettre à jour la durée de vie
        particle.life -= deltaTime;

        // Remettre la particule à sa position initiale si sa durée de vie est écoulée
        if (particle.life <= 0.0f) {
            particle.position = glm::vec3(-0.19f, 0.90f, 0.0f);
            particle.velocity = glm::vec3(
                static_cast<float>(rand()) / RAND_MAX - 0.5f,
                static_cast<float>(rand()) / RAND_MAX - 0.5f,
                static_cast<float>(rand()) / RAND_MAX - 0.5f
            ) * 0.5f;
            particle.life = static_cast<float>(rand()) / RAND_MAX;
        }
    }
}

// Fonction utilitaire pour initialiser les particules
void initializeParticles(std::vector<Particle>& particles, int numParticles) {
    for (int i = 0; i < numParticles; ++i) {
        Particle particle;
        particle.position = glm::vec3(1.2f, 1.0f, -1.0f); // Position initiale des particules
        particle.velocity = glm::vec3(
            static_cast<float>(rand()) / RAND_MAX - 0.5f,
            static_cast<float>(rand()) / RAND_MAX - 0.5f,
            static_cast<float>(rand()) / RAND_MAX - 0.5f
        ) * 0.5f; // Vitesse initiale des particules
        particle.life = static_cast<float>(rand()) / RAND_MAX; // Durée de vie aléatoire

        particles.push_back(particle);
    }
}

//**********************

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Projet VR", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configurer l'état global d'opengl
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // construction et compilation les shaders
    // -------------------------
    Shader sunshader("sun.vs", "sun.fs");
    Shader shader("cubemaps.vs", "cubemaps.fs");
    Shader particuleshader("particule.vs", "particule.fs");
    Shader skyboxShader("skybox.vs", "skybox.fs");
    //light
    Shader earthshader("basic_lighting.vs", "basic_lighting.fs");
    Shader lightCubeShader("light_cube.vs", "light_cube.fs");

    // mettre en place les données de vertex (et le(s) tampon(s)) et configurer les attributs de vertex.
    // ------------------------------------------------------------------
    //texture
    // Définir les sommets et les indices du cube
    float cubeVertices[] = {
        // Positions          // Coordonnées de texture
         1.0f,  1.0f,  1.0f,   1.0f, 1.0f,
        -1.0f,  1.0f,  1.0f,   0.0f, 1.0f,
        -1.0f, -1.0f,  1.0f,   0.0f, 0.0f,
         1.0f, -1.0f,  1.0f,   1.0f, 0.0f,

         1.0f,  1.0f, -1.0f,   1.0f, 1.0f,
        -1.0f,  1.0f, -1.0f,   0.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,   0.0f, 0.0f,
         1.0f, -1.0f, -1.0f,   1.0f, 0.0f,
    };


    float vertices[] = {
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
             0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
             0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
             0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };

    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    // load textures
    // ... Créer des VAO, VBO, EBO ...
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindVertexArray(cubeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // configurer le VAO de la lumière (le VBO reste le même ; les sommets sont les mêmes pour l'objet lumière qui est aussi un cube 3D).
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    // Notez que nous mettons à jour le pas de l'attribut de position de la lampe pour refléter les données mises à jour de la mémoire tampon.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // -------------

   //---------  Particule
    unsigned int particlesVAO, particlesVBO;

    // Fonction pour initialiser le VAO et le VBO pour les particules
   // void initParticleBuffers() {
    glGenVertexArrays(1, &particlesVAO);
    glGenBuffers(1, &particlesVBO);
    glBindVertexArray(particlesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, particlesVBO);
    glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(Particle), particles.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, color));

    glBindVertexArray(0);


    // Initialisation des particules
    const int numParticles = 10000;
    //std::vector<Particle> particles;
    initializeParticles(particles, numParticles);

    //----------------


    Model planet(FileSystem::getPath("resources/objects/planet/planet.obj"));
    Model sun(FileSystem::getPath("resources/objects/planet/planet.obj"));
    unsigned int texture = loadTexture("resources/textures/container2.png");
    char paths[] = "resources/objects/planet/planet.obj";

    vector<std::string> faces
    {
        FileSystem::getPath("resources/textures/skybox/right.jpg"),
        FileSystem::getPath("resources/textures/skybox/left.jpg"),
        FileSystem::getPath("resources/textures/skybox/top.jpg"),
        FileSystem::getPath("resources/textures/skybox/bottom.jpg"),
        FileSystem::getPath("resources/textures/skybox/front.jpg"),
        FileSystem::getPath("resources/textures/skybox/back.jpg")
    };
    unsigned int cubemapTexture = loadCubemap(faces);


    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    // Configuration GLFW
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;


        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //---------------------------------------

        // Veillez à activer le shader lorsque vous définissez des uniformes ou dessinez des objets.
        earthshader.use();
        earthshader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
        earthshader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
        earthshader.setVec3("lightPos", lightPos);
        earthshader.setVec3("viewPos", camera.Position);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

          // world transformation
        glm::mat4 modelLight = glm::mat4(1.0f);
        // earthshader.setMat4("model", modelLight);

         // render the cube
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

     //   modelLight = glm::scale(modelLight, glm::vec3(0.2f)); // a smaller cube
      //  lightCubeShader.setMat4("model", modelLight);

     //   glBindVertexArray(lightCubeVAO);
     //   glDrawArrays(GL_TRIANGLES, 0, 36);

        //--------------------------------
                //----Particule
        //glBindVertexArray(particlesVAO);
        //glDrawArrays(GL_POINTS, 0, static_cast<int>(particles.size()));
        //glm::mat4 particulesview = camera.GetViewMatrix(); // obtenir la matrice de vue ;
        //glm::mat4 particulesprojection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        updateParticles(particles, deltaTime);
        particuleshader.use();// Utilisez le shader de particules
        glBindVertexArray(particlesVAO);

        for (const auto& particle : particles) {
            // Matrice modèle
            

            glm::mat4 particulesmodel = glm::mat4(1.0f);
            //glm::mat4 particulesprojection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            //glm::mat4 particulesview = camera.GetViewMatrix();
            
            //glm::mat4 particulesview = glm::mat4(glm::mat3(camera.GetViewMatrix()));

            particulesmodel = glm::translate(particulesmodel, particle.position); // Translation par la position de la particule
            particulesmodel = glm::scale(particulesmodel, glm::vec3(5.0f)); // Échelle de la particule 
            glm::mat4 particulesprojection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            glm::mat4 particulesview = glm::lookAt(glm::vec3(3.0f, -3.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            // Appliquer la transformation pour chaque particule
            //glm::mat4 particulesview = camera.GetViewMatrix();
            //glm::mat4 particulesprojection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

            particuleshader.setMat4("view", particulesview);
            particuleshader.setMat4("projection", particulesprojection);
            particuleshader.setMat4("model", particulesmodel);


            // Rendu de la particule (simple point)
            glDrawArrays(GL_POINTS, 0, 1); 
            //glDrawArrays(GL_POINTS, 0, static_cast<int>(particles.size()));
        }
        glBindVertexArray(0);


        // draw scene as normal
        sunshader.use();
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view1 = camera.GetViewMatrix();
        glm::mat4 projection1 = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        sunshader.setMat4("model", model);
        sunshader.setMat4("view", view1);
        sunshader.setMat4("projection", projection1);

        // dessiner la terre
        earthshader.use();
        // Activez la texture 

        glm::mat4 earthmodel = glm::mat4(1.0f);
        glm::mat4 earthview = camera.GetViewMatrix();
        glm::mat4 earthprojection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        earthmodel = glm::rotate(earthmodel, (float)glfwGetTime() * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotation autour de l'axe Y
        earthmodel = glm::translate(earthmodel, glm::vec3(0.01f, 3.5f, -2.f));
        earthmodel = glm::scale(earthmodel, glm::vec3(0.1f, 0.1f, 0.1f));
        earthshader.setMat4("model", earthmodel);
        earthshader.setMat4("view", earthview);
        earthshader.setMat4("projection", earthprojection);
        earthshader.setVec3("objectColor", glm::vec3(1.0f, 1.0f, 1.0f));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        earthshader.setInt("texture1", 0);
        //glBindTexture(GL_TEXTURE_2D, textureID);
        planet.Draw(earthshader);

        // dessiner le soleil
        sunshader.use();
        sunshader.setVec3("objectColor", 1.0f, 1.1f, 1.0f);
        sunshader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
        glm::mat4 sunmodel = glm::mat4(1.0f);
        //sunmodel = glm::rotate(sunmodel, (float)glfwGetTime() * 0.1f, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotation autour de l'axe Y
        sunmodel = glm::translate(sunmodel, glm::vec3(0.0f, 3.5f, -1.0f));
        sunmodel = glm::scale(sunmodel, glm::vec3(0.2f, 0.2f, 0.2f));
        sunshader.setMat4("model", sunmodel);
        sun.Draw(sunshader);

        // dessiner la boîte à ciel en dernier
        glDepthFunc(GL_LEQUAL);  // modifie la fonction de profondeur afin que le test de profondeur soit réussi lorsque les valeurs sont égales au contenu du tampon de profondeur.
        skyboxShader.use();
        view1 = glm::mat4(glm::mat3(camera.GetViewMatrix())); //supprimer la translation de la matrice de vue. 
        skyboxShader.setMat4("view", view1);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default  



        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteVertexArrays(1, &particlesVAO);
    glDeleteVertexArrays(1, &particlesVBO);
    glDeleteTextures(1, &texture);

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
