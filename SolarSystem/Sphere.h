#pragma once
#ifndef SPHERE_H
#define SPHERE_H


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <numbers>
#include <iostream>


class Sphere {

private:
    // Sphere Parameters
    const int LATITUDE_DIVISIONS = 40;
    const int LONGITUDE_DIVISIONS = 40;
    const float RADIUS = 1.0f;
    unsigned int textureID;
	unsigned int VAO, VBO, EBO;
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
public:
    Sphere() {
        generateSphereData();

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // Position attribute (layout = 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Texture Coordinates (layout = 1)
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void generateSphereData() {
        for (int lat = 0; lat <= LATITUDE_DIVISIONS; ++lat) {
            for (int lon = 0; lon <= LONGITUDE_DIVISIONS; ++lon) {
                float theta = lat * 3.14 / LATITUDE_DIVISIONS;
                float phi = lon * 2.0f * 3.14 / LONGITUDE_DIVISIONS;

                float x = RADIUS * sin(theta) * cos(phi);
                float y = RADIUS * cos(theta);
                float z = RADIUS * sin(theta) * sin(phi);
                float u = (float)lon / LONGITUDE_DIVISIONS;
                float v = (float)lat / LATITUDE_DIVISIONS;

                // Push vertex data (position + UV)
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);
                vertices.push_back(u);
                vertices.push_back(v);
            }
        }
        for (int lat = 0; lat < LATITUDE_DIVISIONS; ++lat) {
            for (int lon = 0; lon < LONGITUDE_DIVISIONS; ++lon) {
                int current = lat * (LONGITUDE_DIVISIONS + 1) + lon;
                int next = current + LONGITUDE_DIVISIONS + 1;

                // Triangle 1
                indices.push_back(current);
                indices.push_back(next);
                indices.push_back(current + 1);

                // Triangle 2
                indices.push_back(current + 1);
                indices.push_back(next);
                indices.push_back(next + 1);
            }
        }
    }

    void renderSphere() {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    void DeleteBuffers() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }


};
#endif // !SPHERE_H
