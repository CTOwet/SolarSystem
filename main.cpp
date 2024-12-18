#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

float zoomLevel = -25.0f; // Level zoom, lebih negatif = lebih jauh

// Callback untuk menangani input scroll mouse
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    // yoffset menunjukkan arah scroll: positif untuk zoom in, negatif untuk zoom out
    if (yoffset > 0) {
        zoomLevel += 1.0f; // Zoom in
    } else if (yoffset < 0) {
        zoomLevel -= 1.0f; // Zoom out
    }
}

// Fungsi untuk memuat teksturi
GLuint loadTexture(const char* filename) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cerr << "Failed to load texture: " << filename << std::endl;
    }
    stbi_image_free(data);
    return textureID;
}

// Fungsi untuk menggambar bola (sphere) dengan tekstur
void drawSphere(float radius, int longitude, int latitude, GLuint texture) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);

    for (int i = 0; i <= latitude; i++) {
        float theta1 = (float(i) / latitude) * M_PI;
        float theta2 = (float(i + 1) / latitude) * M_PI;

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= longitude; j++) {
            float phi = (float(j) / longitude) * 2.0f * M_PI;

            float x1 = radius * sin(theta1) * cos(phi);
            float y1 = radius * cos(theta1);
            float z1 = radius * sin(theta1) * sin(phi);
            float u1 = (float(j) / longitude);
            float v1 = (float(i) / latitude);

            float x2 = radius * sin(theta2) * cos(phi);
            float y2 = radius * cos(theta2);
            float z2 = radius * sin(theta2) * sin(phi);
            float u2 = (float(j) / longitude);
            float v2 = (float(i + 1) / latitude);

            glTexCoord2f(u1, v1); glVertex3f(x1, y1, z1);
            glTexCoord2f(u2, v2); glVertex3f(x2, y2, z2);
        }
        glEnd();
    }

    glDisable(GL_TEXTURE_2D);
}

// Fungsi untuk menggambar orbit sebagai lingkaran
void drawOrbit(float radius) {
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i <= 360; i++) {
        float theta = i * M_PI / 180.0f;
        float x = radius * cos(theta);
        float z = radius * sin(theta);
        glVertex3f(x, 0.0f, z);
    }
    glEnd();
}

// Variabel untuk menyimpan ID tekstur
GLuint mercuryTexture, earthTexture, marsTexture, venusTexture, jupiterTexture, saturnTexture, uranusTexture, neptuneTexture, sunTexture;

// Fungsi untuk inisialisasi semua tekstur planet
void initTextures() {
    mercuryTexture = loadTexture("resources/planets/2k_mercury.jpg");
    earthTexture = loadTexture("resources/planets/earth2k.jpg");
    marsTexture = loadTexture("resources/planets/2k_mars.jpg");
    venusTexture = loadTexture("resources/planets/2k_venus.jpg");
    jupiterTexture = loadTexture("resources/planets/2k_jupiter.jpg");
    saturnTexture = loadTexture("resources/planets/2k_saturn.jpg");
    uranusTexture = loadTexture("resources/planets/2k_uranus.jpg");
    neptuneTexture = loadTexture("resources/planets/2k_neptune.jpg");
    sunTexture = loadTexture("resources/planets/8k_sun.jpg");
}
	
// Fungsi untuk menggambar tata surya
void drawSolarSystem(float planetRotation, float orbitRotation) {
    glPushMatrix();

    // Matahari (statis)
    glColor3f(1.0f, 1.0f, 0.0f); // Kuning untuk matahari
    drawSphere(1.0f, 40, 40, sunTexture); // Matahari

    // Garis orbit dan planet-planet
    float orbitRadii[] = {2.0f, 3.0f, 4.0f, 5.0f, 6.5f, 8.0f, 9.5f, 11.0f};
    GLuint planetTextures[] = {mercuryTexture, venusTexture, earthTexture, marsTexture, jupiterTexture, saturnTexture, uranusTexture, neptuneTexture};
    float planetSizes[] = {0.2f, 0.3f, 0.4f, 0.3f, 0.7f, 0.6f, 0.5f, 0.5f};

    // Periode orbit dalam tahun
    float orbitPeriod[] = {0.24f, 0.62f, 1.0f, 1.88f, 11.86f, 29.46f, 84.01f, 164.8f};

    // Faktor untuk rotasi planet pada porosnya (kecepatan rotasi poros fiktif)
    float rotationSpeed[] = {58.6f, -243.0f, 1.0f, 1.03f, 0.41f, 0.45f, -0.72f, 0.67f}; // Dalam hari relatif ke bumi

    for (int i = 0; i < 8; i++) {
        glColor3f(1.0f, 1.0f, 1.0f); // Warna putih untuk garis orbit
        drawOrbit(orbitRadii[i]); // Gambar orbit

        glPushMatrix();
        // Rotasi orbit berdasarkan periode orbit dalam tahun, planet yang lebih jauh bergerak lebih lambat
        glRotatef(orbitRotation / orbitPeriod[i], 0.0f, 1.0f, 0.0f); 
        glTranslatef(orbitRadii[i], 0.0f, 0.0f); // Jarak dari matahari
        // Rotasi planet di porosnya sendiri berdasarkan kecepatan rotasi (hari)
        glRotatef(planetRotation * rotationSpeed[i], 0.0f, 1.0f, 0.0f); 
        drawSphere(planetSizes[i], 40, 40, planetTextures[i]); // Planet dengan tekstur
        glPopMatrix();
    }

    glPopMatrix();
}

// Variabel untuk mengatur zoom dan pergeseran kamera

float cameraX = 0.0f;      // Pergeseran kamera ke kiri atau kanan

// Fungsi untuk menangani input dari keyboard
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        zoomLevel += 0.5f; // Zoom in (kamera mendekat)
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        zoomLevel -= 0.5f; // Zoom out (kamera menjauh)
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        cameraX -= 0.5f; // Geser kamera ke kiri
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        cameraX += 0.5f; // Geser kamera ke kanan
    }
}

int main() {
    // Inisialisasi GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Buat window
    GLFWwindow* window = glfwCreateWindow(1910, 1200, "Tata Surya 3D", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Set callback untuk scroll mouse
    glfwSetScrollCallback(window, scrollCallback);

    // Inisialisasi GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Enable depth test and set perspective
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, 800.0f / 600.0f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    // Inisialisasi tekstur
    initTextures();

    float planetRotation = 0.0f;
    float orbitRotation = 0.0f;

    // Loop utama
    while (!glfwWindowShouldClose(window)) {
        // Bersihkan layar
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        // Proses input dari keyboard
        processInput(window);

        // Posisikan kamera berdasarkan zoomLevel dan pergeseran ke kiri-kanan (cameraX)
        glTranslatef(cameraX, 0.0f, zoomLevel); // Zoom dan geser kamera

        // Rotasi kamera agar kita bisa melihat seluruh tata surya dari sudut pandang yang sedikit miring
        glRotatef(20.0f, 1.0f, 0.0f, 0.0f); // Rotasi ke bawah untuk memberi perspektif atas
        glRotatef(orbitRotation * 0.1f, 0.0f, 1.0f, 0.0f); // Rotasi seluruh tata surya pelan-pelan untuk efek rotasi global

        // Gambar tata surya
        drawSolarSystem(planetRotation, orbitRotation);

        // Update rotasi
        planetRotation += 0.5f;
        orbitRotation += 0.1f;

        // Swap buffers dan cek event
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}