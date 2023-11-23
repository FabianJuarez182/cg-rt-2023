#include <SDL2/SDL.h>
#include <SDL_events.h>
#include <SDL_render.h>
#include <cstdlib>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/geometric.hpp>
#include <string>
#include <glm/glm.hpp>
#include <vector>
#include <print.h>

#include "cube.h"
#include "color.h"
#include "intersect.h"
#include "object.h"
#include "sphere.h"
#include "light.h"
#include "camera.h"
#include "skybox.h"

Skybox skybox("src/skybox.jpg");
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const float ASPECT_RATIO = static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT);
const int MAX_RECURSION = 3;
const float BIAS = 0.0001f;

SDL_Renderer* renderer;
std::vector<Object*> objects;
Light light(glm::vec3(-1.0, 0, 10), 1.5f, Color(255, 255, 255));
Camera camera(glm::vec3(0.0, 0.0, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 10.0f);


void point(glm::vec2 position, Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawPoint(renderer, position.x, position.y);
}

float castShadow(const glm::vec3& shadowOrigin, const glm::vec3& lightDir, Object* hitObject) {
    for (auto& obj : objects) {
        if (obj != hitObject) {
            Intersect shadowIntersect = obj->rayIntersect(shadowOrigin, lightDir);
            if (shadowIntersect.isIntersecting && shadowIntersect.dist > 0) {
                float shadowRatio = shadowIntersect.dist / glm::length(light.position - shadowOrigin);
                shadowRatio = glm::min(1.0f, shadowRatio);
                return 1.0f - shadowRatio;
            }
        }
    }
    return 1.0f;
}

Color castRay(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const short recursion = 0) {
    float zBuffer = 99999;
    Object* hitObject = nullptr;
    Intersect intersect;

    for (const auto& object : objects) {
        Intersect i = object->rayIntersect(rayOrigin, rayDirection);
        if (i.isIntersecting && i.dist < zBuffer) {
            zBuffer = i.dist;
            hitObject = object;
            intersect = i;
        }
    }

    if (!intersect.isIntersecting || recursion == MAX_RECURSION) {
        glm::vec3 skyboxColor = skybox.getColor(rayDirection);
        return Color(skyboxColor.r, skyboxColor.g, skyboxColor.b);
    }


    glm::vec3 lightDir = glm::normalize(light.position - intersect.point);
    glm::vec3 viewDir = glm::normalize(rayOrigin - intersect.point);
    glm::vec3 reflectDir = glm::reflect(-lightDir, intersect.normal); 

    float shadowIntensity = castShadow(intersect.point, lightDir, hitObject);

    float diffuseLightIntensity = std::max(0.0f, glm::dot(intersect.normal, lightDir));
    float specReflection = glm::dot(viewDir, reflectDir);
    
    Material mat = hitObject->material;

    float specLightIntensity = std::pow(std::max(0.0f, glm::dot(viewDir, reflectDir)), mat.specularCoefficient);


    Color reflectedColor(0.0f, 0.0f, 0.0f);
    if (mat.reflectivity > 0) {
        glm::vec3 origin = intersect.point + intersect.normal * BIAS;
        reflectedColor = castRay(origin, reflectDir, recursion + 1); 
    }

    Color refractedColor(0.0f, 0.0f, 0.0f);
    if (mat.transparency > 0) {
        glm::vec3 origin = intersect.point - intersect.normal * BIAS;
        glm::vec3 refractDir = glm::refract(rayDirection, intersect.normal, mat.refractionIndex);
        refractedColor = castRay(origin, refractDir, recursion + 1); 
    }



    Color diffuseLight = mat.diffuse * light.intensity * diffuseLightIntensity * mat.albedo * shadowIntensity;
    Color specularLight = light.color * light.intensity * specLightIntensity * mat.specularAlbedo * shadowIntensity;
    Color color = (diffuseLight + specularLight) * (1.0f - mat.reflectivity - mat.transparency) + reflectedColor * mat.reflectivity + refractedColor * mat.transparency;
    return color;
}

void setUpPokeball() {
    // Parte roja de la Pokébola
    Material red = {
        Color(255, 0, 0),
        1.0,
        0.0,
        9.0f,
        0.0f,
        0.0f
    };

 // Añadir cubos para la parte blanca
        //primer circulo
    objects.push_back(new Cube(glm::vec3(0.0f, 0.2f, -0.1f), 0.1f, red));//esquina frente
    objects.push_back(new Cube(glm::vec3(0.1f, 0.2f, -0.1f), 0.1f, red));//esquina frente
    objects.push_back(new Cube(glm::vec3(-0.1f, 0.2f, -0.1f), 0.1f, red));//esquina frente
    objects.push_back(new Cube(glm::vec3(-0.2f, 0.1f, -0.1f), 0.1f, red));
    objects.push_back(new Cube(glm::vec3(-0.3f, 0.1f, -0.2f), 0.1f, red));
    objects.push_back(new Cube(glm::vec3(-0.4f, 0.1f, -0.3f), 0.1f, red));
    objects.push_back(new Cube(glm::vec3(-0.5f, 0.1f, -0.4f), 0.1f, red));
    objects.push_back(new Cube(glm::vec3(-0.6f, 0.1f, -0.5f), 0.1f, red));//esquina izquierda
    objects.push_back(new Cube(glm::vec3(-0.6f, 0.1f, -0.6f), 0.1f, red));//esquina izquierda
    objects.push_back(new Cube(glm::vec3(-0.5f, 0.1f, -0.7f), 0.1f, red));
    objects.push_back(new Cube(glm::vec3(-0.4f, 0.1f, -0.8f), 0.1f, red));
    objects.push_back(new Cube(glm::vec3(-0.3f, 0.1f, -0.9f), 0.1f, red));
    objects.push_back(new Cube(glm::vec3(-0.2f, 0.1f, -1.0f), 0.1f, red));
    objects.push_back(new Cube(glm::vec3(-0.1f, 0.1f, -1.1f), 0.1f, red));
    objects.push_back(new Cube(glm::vec3(0.0f, 0.1f, -1.2f), 0.1f, red));//esquina trasera
    objects.push_back(new Cube(glm::vec3(0.1f, 0.1f, -1.1f), 0.1f, red));
    objects.push_back(new Cube(glm::vec3(0.2f, 0.1f, -1.0f), 0.1f, red));
    objects.push_back(new Cube(glm::vec3(0.3f, 0.1f, -0.9f), 0.1f, red));
    objects.push_back(new Cube(glm::vec3(0.4f, 0.1f, -0.8f), 0.1f, red));
    objects.push_back(new Cube(glm::vec3(0.5f, 0.1f, -0.7f), 0.1f, red));
    objects.push_back(new Cube(glm::vec3(0.6f, 0.1f, -0.6f), 0.1f, red));//esquina derecha
    objects.push_back(new Cube(glm::vec3(0.6f, 0.1f, -0.5f), 0.1f, red));//esquina derecha
    objects.push_back(new Cube(glm::vec3(0.5f, 0.1f, -0.4f), 0.1f, red));
    objects.push_back(new Cube(glm::vec3(0.4f, 0.1f, -0.3f), 0.1f, red));
    objects.push_back(new Cube(glm::vec3(0.3f, 0.1f, -0.2f), 0.1f, red));
    objects.push_back(new Cube(glm::vec3(0.2f, 0.1f, -0.1f), 0.1f, red));
    objects.push_back(new Cube(glm::vec3(0.0f, 0.3f, -0.2f), 0.1f, red));//esquina frente
    objects.push_back(new Cube(glm::vec3(0.1f, 0.3f, -0.2f), 0.1f, red));//esquina frente
    objects.push_back(new Cube(glm::vec3(-0.1f, 0.3f, -0.2f), 0.1f, red));//esquina frente
    objects.push_back(new Cube(glm::vec3(-0.2f, 0.3f, -0.3f), 0.1f, red));//esquina
    objects.push_back(new Cube(glm::vec3(0.2f, 0.3f, -0.3f), 0.1f, red)); //esquina
    objects.push_back(new Cube(glm::vec3(0.0f, 0.4f, -0.3f), 0.1f, red));//esquina frente
    objects.push_back(new Cube(glm::vec3(0.1f, 0.4f, -0.3f), 0.1f, red));//esquina frente
    objects.push_back(new Cube(glm::vec3(-0.1f, 0.4f, -0.3f), 0.1f, red));//esquina frente
        // final de abajo
    objects.push_back(new Cube(glm::vec3(0.0f, 0.6f, -0.6f), 0.1f, red));//esquina frente
    objects.push_back(new Cube(glm::vec3(0.1f, 0.6f, -0.6f), 0.1f, red));//esquina frente
    objects.push_back(new Cube(glm::vec3(-0.1f, 0.6f, -0.6f), 0.1f, red));//esquina frente
        // penultimo
    objects.push_back(new Cube(glm::vec3(0.0f, 0.6f, -0.5f), 0.1f, red));//esquina frente
    objects.push_back(new Cube(glm::vec3(0.1f, 0.6f, -0.5f), 0.1f, red));//esquina frente
    objects.push_back(new Cube(glm::vec3(-0.1f, 0.6f, -0.5f), 0.1f, red));//esquina frente
    objects.push_back(new Cube(glm::vec3(0.0f, 0.5f, -0.7f), 0.1f, red));//esquina frente
    objects.push_back(new Cube(glm::vec3(0.1f, 0.5f, -0.7f), 0.1f, red));//esquina frente
    objects.push_back(new Cube(glm::vec3(-0.1f, 0.5f, -0.7f), 0.1f, red));//esquina frente
    objects.push_back(new Cube(glm::vec3(0.2f, 0.5f, -0.6f), 0.1f, red));//esquina derecha
    objects.push_back(new Cube(glm::vec3(0.2f, 0.5f, -0.5f), 0.1f, red));//esquina derecha
    objects.push_back(new Cube(glm::vec3(-0.2f, 0.5f, -0.5f), 0.1f, red));//esquina izquierda
    objects.push_back(new Cube(glm::vec3(-0.2f, 0.5f, -0.6f), 0.1f, red));//esquina izquierda
        //antepenultimo

    objects.push_back(new Cube(glm::vec3(0.2f,0.4f, -0.4f), 0.1f, red));//esquina cruzada
    objects.push_back(new Cube(glm::vec3(0.0f, 0.5f, -0.4f), 0.1f, red));//esquina frente
    objects.push_back(new Cube(glm::vec3(0.1f, 0.5f, -0.4f), 0.1f, red));//esquina frente
    objects.push_back(new Cube(glm::vec3(-0.1f, 0.5f, -0.4f), 0.1f, red));//esquina frente
    objects.push_back(new Cube(glm::vec3(0.2f, 0.4f, -0.7f), 0.1f, red));//esquina cruzada
    objects.push_back(new Cube(glm::vec3(0.0f, 0.4f, -0.8f), 0.1f, red));//trasero
    objects.push_back(new Cube(glm::vec3(0.1f, 0.4f, -0.8f), 0.1f, red));//trasero
    objects.push_back(new Cube(glm::vec3(-0.1f, 0.4f, -0.8f), 0.1f, red));//trasero
    objects.push_back(new Cube(glm::vec3(-0.2f, 0.4f, -0.7f), 0.1f, red));//esquina cruzada
    objects.push_back(new Cube(glm::vec3(0.3f, 0.4f, -0.6f), 0.1f, red));//esquina derecha
    objects.push_back(new Cube(glm::vec3(0.3f, 0.4f, -0.5f), 0.1f, red));//esquina derecha
    objects.push_back(new Cube(glm::vec3(-0.2f, 0.4f, -0.4f), 0.1f, red));//esquina cruzada
    objects.push_back(new Cube(glm::vec3(-0.3f, 0.4f, -0.5f), 0.1f, red));//esquina izquierda
    objects.push_back(new Cube(glm::vec3(-0.3f, 0.4f, -0.6f), 0.1f, red));//esquina izquierda

        //anteantepenultimo
    objects.push_back(new Cube(glm::vec3(0.0f, 0.6f, -0.6f), 0.1f, red));//esquina frente
    objects.push_back(new Cube(glm::vec3(0.1f, 0.6f, -0.6f), 0.1f, red));//esquina frente
    objects.push_back(new Cube(glm::vec3(-0.1f, 0.6f, -0.6f), 0.1f, red));//esquina frente
    objects.push_back(new Cube(glm::vec3(-0.2f, 0.4f, -0.3f), 0.1f, red));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(0.2f, 0.4f, -0.3f), 0.1f, red));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(-0.3f, 0.4f, -0.4f), 0.1f, red));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(0.3f, 0.4f, -0.4f), 0.1f, red));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(0.4f, 0.3f, -0.6f), 0.1f, red));//esquina derecha
    objects.push_back(new Cube(glm::vec3(0.4f, 0.3f, -0.5f), 0.1f, red));//esquina derecha
    objects.push_back(new Cube(glm::vec3(0.0f, 0.3f, -0.9f), 0.1f, red));//esquina atras
    objects.push_back(new Cube(glm::vec3(0.1f, 0.3f, -0.9f), 0.1f, red));//esquina atras
    objects.push_back(new Cube(glm::vec3(-0.1f, 0.3f, -0.9f), 0.1f, red));//esquina atras
    objects.push_back(new Cube(glm::vec3(-0.2f, 0.4f, -0.4f), 0.1f, red));//esquina cruzada
    objects.push_back(new Cube(glm::vec3(-0.4f, 0.3f, -0.5f), 0.1f, red));//esquina izquierda
    objects.push_back(new Cube(glm::vec3(-0.4f, 0.3f, -0.6f), 0.1f, red));//esquina izquierda
    objects.push_back(new Cube(glm::vec3(-0.2f, 0.4f, -0.8f), 0.1f, red));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(0.2f, 0.4f, -0.8f), 0.1f, red));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(-0.3f, 0.4f, -0.7f), 0.1f, red));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(0.3f, 0.4f, -0.7f), 0.1f, red));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(0.2f, 0.3f, -0.3f), 0.1f, red));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(0.4f, 0.3f, -0.4f), 0.1f, red));//esquina cruzada adelante
        //primer circulo
    objects.push_back(new Cube(glm::vec3(0.0f, 0.5f, -0.4f), 0.1f, red));//esquina frente
    objects.push_back(new Cube(glm::vec3(0.1f, 0.5f, -0.4f), 0.1f, red));//esquina frente
    objects.push_back(new Cube(glm::vec3(-0.1f, 0.5f, -0.4f), 0.1f, red));//esquina frente
    objects.push_back(new Cube(glm::vec3(0.5f, 0.2f, -0.6f), 0.1f, red));//esquina derecha
    objects.push_back(new Cube(glm::vec3(0.5f, 0.2f, -0.5f), 0.1f, red));//esquina derecha
    objects.push_back(new Cube(glm::vec3(0.0f, 0.2f, -1.0f), 0.1f, red));//esquina atras
    objects.push_back(new Cube(glm::vec3(0.1f, 0.2f, -1.0f), 0.1f, red));//esquina atras
    objects.push_back(new Cube(glm::vec3(-0.1f, 0.2f, -1.0f), 0.1f, red));//esquina atras
    objects.push_back(new Cube(glm::vec3(-0.5f, 0.2f, -0.5f), 0.1f, red));//esquina izquierda
    objects.push_back(new Cube(glm::vec3(-0.5f, 0.2f, -0.6f), 0.1f, red));//esquina izquierda
    objects.push_back(new Cube(glm::vec3(-0.4f, 0.2f, -0.4f), 0.1f, red));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(0.4f, 0.2f, -0.4f), 0.1f, red));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(-0.3f, 0.2f, -0.3f), 0.1f, red));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(0.3f, 0.2f, -0.3f), 0.1f, red));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(-0.2f, 0.2f, -0.2f), 0.1f, red));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(0.2f, 0.2f, -0.2f), 0.1f, red));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(-0.3f, 0.3f, -0.3f), 0.1f, red));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(0.3f, 0.3f, -0.3f), 0.1f, red));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(-0.3f, 0.3f, -0.4f), 0.1f, red));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(0.3f, 0.3f, -0.4f), 0.1f, red));//esquina cruzada adelante

    // Parte negra de la Pokébola
    Material black = {
        Color(0, 0, 0),
        1.0,
        0.0,
        9.0f,
        0.0f,
        0.0f
    };

    // Añadir cubos para la parte negra
        //circulo central
    objects.push_back(new Cube(glm::vec3(-0.1f, 0.0f, 0.0f), 0.1f, black));
    objects.push_back(new Cube(glm::vec3(0.1f, 0.0f, 0.0f), 0.1f, black));
    objects.push_back(new Cube(glm::vec3(0.0f, -0.1f, 0.0f), 0.1f, black));
    objects.push_back(new Cube(glm::vec3(0.0f, 0.1f, 0.0f), 0.1f, black));
    objects.push_back(new Cube(glm::vec3(0.1f, 0.1f, 0.0f), 0.1f, black));
    objects.push_back(new Cube(glm::vec3(-0.1f, -0.1f, 0.0f), 0.1f, black));
    objects.push_back(new Cube(glm::vec3(0.1f, -0.1f, 0.0f), 0.1f, black));
    objects.push_back(new Cube(glm::vec3(-0.1f, 0.1f, 0.0f), 0.1f, black));
    //contorno pokebola
    objects.push_back(new Cube(glm::vec3(0.2f, 0.0f, -0.1f), 0.1f, black)); 
    objects.push_back(new Cube(glm::vec3(0.3f, 0.0f, -0.2f), 0.1f, black));
    objects.push_back(new Cube(glm::vec3(0.4f, 0.0f, -0.3f), 0.1f, black));
    objects.push_back(new Cube(glm::vec3(0.5f, 0.0f, -0.4f), 0.1f, black));
    objects.push_back(new Cube(glm::vec3(0.6f, 0.0f, -0.5f), 0.1f, black));
    objects.push_back(new Cube(glm::vec3(0.6f, 0.0f, -0.6f), 0.1f, black));
    objects.push_back(new Cube(glm::vec3(0.5f, 0.0f, -0.7f), 0.1f, black));
    objects.push_back(new Cube(glm::vec3(0.4f, 0.0f, -0.8f), 0.1f, black));
    objects.push_back(new Cube(glm::vec3(0.3f, 0.0f, -0.9f), 0.1f, black));
    objects.push_back(new Cube(glm::vec3(0.2f, 0.0f, -1.0f), 0.1f, black));
    objects.push_back(new Cube(glm::vec3(0.1f, 0.0f, -1.1f), 0.1f, black));
    objects.push_back(new Cube(glm::vec3(0.0f, 0.0f, -1.2f), 0.1f, black)); //esquina trasera
    objects.push_back(new Cube(glm::vec3(-0.1f, 0.0f, -1.1f), 0.1f, black));
    objects.push_back(new Cube(glm::vec3(-0.2f, 0.0f, -1.0f), 0.1f, black));
    objects.push_back(new Cube(glm::vec3(-0.3f, 0.0f, -0.9f), 0.1f, black));
    objects.push_back(new Cube(glm::vec3(-0.4f, 0.0f, -0.8f), 0.1f, black));
    objects.push_back(new Cube(glm::vec3(-0.5f, 0.0f, -0.7f), 0.1f, black));
    objects.push_back(new Cube(glm::vec3(-0.6f, 0.0f, -0.6f), 0.1f, black));//esquina izquierda
    objects.push_back(new Cube(glm::vec3(-0.6f, 0.0f, -0.5f), 0.1f, black));//esquina izquierda
    objects.push_back(new Cube(glm::vec3(-0.5f, 0.0f, -0.4f), 0.1f, black));
    objects.push_back(new Cube(glm::vec3(-0.4f, 0.0f, -0.3f), 0.1f, black));
    objects.push_back(new Cube(glm::vec3(-0.3f, 0.0f, -0.2f), 0.1f, black));
    objects.push_back(new Cube(glm::vec3(-0.2f, 0.0f, -0.1f), 0.1f, black));


    Material white = {
    Color(255, 255, 255),  // Color blanco
    1.0,                    // Coeficiente de reflexión difusa
    0.0,                    // Coeficiente de reflexión especular
    9.0f,                   // Exponente especular
    0.0f,                   // Transmitancia
    0.0f                    // Índice de refracción
    };

    // Añadir cubos para la parte blanca
        //primer circulo
    objects.push_back(new Cube(glm::vec3(0.0f, -0.2f, -0.1f), 0.1f, white));//esquina frente
    objects.push_back(new Cube(glm::vec3(0.1f, -0.2f, -0.1f), 0.1f, white));//esquina frente
    objects.push_back(new Cube(glm::vec3(-0.1f, -0.2f, -0.1f), 0.1f, white));//esquina frente
    objects.push_back(new Cube(glm::vec3(-0.2f, -0.1f, -0.1f), 0.1f, white));
    objects.push_back(new Cube(glm::vec3(-0.3f, -0.1f, -0.2f), 0.1f, white));
    objects.push_back(new Cube(glm::vec3(-0.4f, -0.1f, -0.3f), 0.1f, white));
    objects.push_back(new Cube(glm::vec3(-0.5f, -0.1f, -0.4f), 0.1f, white));
    objects.push_back(new Cube(glm::vec3(-0.6f, -0.1f, -0.5f), 0.1f, white));//esquina izquierda
    objects.push_back(new Cube(glm::vec3(-0.6f, -0.1f, -0.6f), 0.1f, white));//esquina izquierda
    objects.push_back(new Cube(glm::vec3(-0.5f, -0.1f, -0.7f), 0.1f, white));
    objects.push_back(new Cube(glm::vec3(-0.4f, -0.1f, -0.8f), 0.1f, white));
    objects.push_back(new Cube(glm::vec3(-0.3f, -0.1f, -0.9f), 0.1f, white));
    objects.push_back(new Cube(glm::vec3(-0.2f, -0.1f, -1.0f), 0.1f, white));
    objects.push_back(new Cube(glm::vec3(-0.1f, -0.1f, -1.1f), 0.1f, white));
    objects.push_back(new Cube(glm::vec3(0.0f, -0.1f, -1.2f), 0.1f, white));//esquina trasera
    objects.push_back(new Cube(glm::vec3(0.1f, -0.1f, -1.1f), 0.1f, white));
    objects.push_back(new Cube(glm::vec3(0.2f, -0.1f, -1.0f), 0.1f, white));
    objects.push_back(new Cube(glm::vec3(0.3f, -0.1f, -0.9f), 0.1f, white));
    objects.push_back(new Cube(glm::vec3(0.4f, -0.1f, -0.8f), 0.1f, white));
    objects.push_back(new Cube(glm::vec3(0.5f, -0.1f, -0.7f), 0.1f, white));
    objects.push_back(new Cube(glm::vec3(0.6f, -0.1f, -0.6f), 0.1f, white));//esquina derecha
    objects.push_back(new Cube(glm::vec3(0.6f, -0.1f, -0.5f), 0.1f, white));//esquina derecha
    objects.push_back(new Cube(glm::vec3(0.5f, -0.1f, -0.4f), 0.1f, white));
    objects.push_back(new Cube(glm::vec3(0.4f, -0.1f, -0.3f), 0.1f, white));
    objects.push_back(new Cube(glm::vec3(0.3f, -0.1f, -0.2f), 0.1f, white));
    objects.push_back(new Cube(glm::vec3(0.2f, -0.1f, -0.1f), 0.1f, white));
    objects.push_back(new Cube(glm::vec3(0.0f, -0.3f, -0.2f), 0.1f, white));//esquina frente
    objects.push_back(new Cube(glm::vec3(0.1f, -0.3f, -0.2f), 0.1f, white));//esquina frente
    objects.push_back(new Cube(glm::vec3(-0.1f, -0.3f, -0.2f), 0.1f, white));//esquina frente
    objects.push_back(new Cube(glm::vec3(-0.2f, -0.3f, -0.3f), 0.1f, white));//esquina
    objects.push_back(new Cube(glm::vec3(0.2f, -0.3f, -0.3f), 0.1f, white)); //esquina
    objects.push_back(new Cube(glm::vec3(0.0f, -0.4f, -0.3f), 0.1f, white));//esquina frente
    objects.push_back(new Cube(glm::vec3(0.1f, -0.4f, -0.3f), 0.1f, white));//esquina frente
    objects.push_back(new Cube(glm::vec3(-0.1f, -0.4f, -0.3f), 0.1f, white));//esquina frente
        // final de abajo
    objects.push_back(new Cube(glm::vec3(0.0f, -0.6f, -0.6f), 0.1f, white));//esquina frente
    objects.push_back(new Cube(glm::vec3(0.1f, -0.6f, -0.6f), 0.1f, white));//esquina frente
    objects.push_back(new Cube(glm::vec3(-0.1f, -0.6f, -0.6f), 0.1f, white));//esquina frente
        // penultimo
    objects.push_back(new Cube(glm::vec3(0.0f, -0.6f, -0.5f), 0.1f, white));//esquina frente
    objects.push_back(new Cube(glm::vec3(0.1f, -0.6f, -0.5f), 0.1f, white));//esquina frente
    objects.push_back(new Cube(glm::vec3(-0.1f, -0.6f, -0.5f), 0.1f, white));//esquina frente
    objects.push_back(new Cube(glm::vec3(0.0f, -0.5f, -0.7f), 0.1f, white));//esquina frente
    objects.push_back(new Cube(glm::vec3(0.1f, -0.5f, -0.7f), 0.1f, white));//esquina frente
    objects.push_back(new Cube(glm::vec3(-0.1f, -0.5f, -0.7f), 0.1f, white));//esquina frente
    objects.push_back(new Cube(glm::vec3(0.2f, -0.5f, -0.6f), 0.1f, white));//esquina derecha
    objects.push_back(new Cube(glm::vec3(0.2f, -0.5f, -0.5f), 0.1f, white));//esquina derecha
    objects.push_back(new Cube(glm::vec3(-0.2f, -0.5f, -0.5f), 0.1f, white));//esquina izquierda
    objects.push_back(new Cube(glm::vec3(-0.2f, -0.5f, -0.6f), 0.1f, white));//esquina izquierda
        //antepenultimo

    objects.push_back(new Cube(glm::vec3(0.2f,-0.4f, -0.4f), 0.1f, white));//esquina cruzada
    objects.push_back(new Cube(glm::vec3(0.0f, -0.5f, -0.4f), 0.1f, white));//esquina frente
    objects.push_back(new Cube(glm::vec3(0.1f, -0.5f, -0.4f), 0.1f, white));//esquina frente
    objects.push_back(new Cube(glm::vec3(-0.1f, -0.5f, -0.4f), 0.1f, white));//esquina frente
    objects.push_back(new Cube(glm::vec3(0.2f,-0.4f, -0.7f), 0.1f, white));//esquina cruzada
    objects.push_back(new Cube(glm::vec3(0.0f, -0.4f, -0.8f), 0.1f, white));//trasero
    objects.push_back(new Cube(glm::vec3(0.1f, -0.4f, -0.8f), 0.1f, white));//trasero
    objects.push_back(new Cube(glm::vec3(-0.1f, -0.4f, -0.8f), 0.1f, white));//trasero
    objects.push_back(new Cube(glm::vec3(-0.2f,-0.4f, -0.7f), 0.1f, white));//esquina cruzada
    objects.push_back(new Cube(glm::vec3(0.3f, -0.4f, -0.6f), 0.1f, white));//esquina derecha
    objects.push_back(new Cube(glm::vec3(0.3f, -0.4f, -0.5f), 0.1f, white));//esquina derecha
    objects.push_back(new Cube(glm::vec3(-0.2f,-0.4f, -0.4f), 0.1f, white));//esquina cruzada
    objects.push_back(new Cube(glm::vec3(-0.3f, -0.4f, -0.5f), 0.1f, white));//esquina izquierda
    objects.push_back(new Cube(glm::vec3(-0.3f, -0.4f, -0.6f), 0.1f, white));//esquina izquierda

        //anteantepenultimo
    objects.push_back(new Cube(glm::vec3(0.0f, -0.6f, -0.6f), 0.1f, white));//esquina frente
    objects.push_back(new Cube(glm::vec3(0.1f, -0.6f, -0.6f), 0.1f, white));//esquina frente
    objects.push_back(new Cube(glm::vec3(-0.1f, -0.6f, -0.6f), 0.1f, white));//esquina frente
    objects.push_back(new Cube(glm::vec3(-0.2f,-0.4f, -0.3f), 0.1f, white));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(0.2f,-0.4f, -0.3f), 0.1f, white));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(-0.3f,-0.4f, -0.4f), 0.1f, white));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(0.3f,-0.4f, -0.4f), 0.1f, white));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(0.4f, -0.3f, -0.6f), 0.1f, white));//esquina derecha
    objects.push_back(new Cube(glm::vec3(0.4f, -0.3f, -0.5f), 0.1f, white));//esquina derecha
    objects.push_back(new Cube(glm::vec3(0.0f, -0.3f, -0.9f), 0.1f, white));//esquina atras
    objects.push_back(new Cube(glm::vec3(0.1f, -0.3f, -0.9f), 0.1f, white));//esquina atras
    objects.push_back(new Cube(glm::vec3(-0.1f, -0.3f, -0.9f), 0.1f, white));//esquina atras
    objects.push_back(new Cube(glm::vec3(-0.2f,-0.4f, -0.4f), 0.1f, white));//esquina cruzada
    objects.push_back(new Cube(glm::vec3(-0.4f, -0.3f, -0.5f), 0.1f, white));//esquina izquierda
    objects.push_back(new Cube(glm::vec3(-0.4f, -0.3f, -0.6f), 0.1f, white));//esquina izquierda
    objects.push_back(new Cube(glm::vec3(-0.2f,-0.4f, -0.8f), 0.1f, white));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(0.2f,-0.4f, -0.8f), 0.1f, white));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(-0.3f,-0.4f, -0.7f), 0.1f, white));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(0.3f,-0.4f, -0.7f), 0.1f, white));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(0.2f,-0.3f, -0.3f), 0.1f, white));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(0.4f,-0.3f, -0.4f), 0.1f, white));//esquina cruzada adelante
        //primer circulo
    objects.push_back(new Cube(glm::vec3(0.0f, -0.5f, -0.4f), 0.1f, white));//esquina frente
    objects.push_back(new Cube(glm::vec3(0.1f, -0.5f, -0.4f), 0.1f, white));//esquina frente
    objects.push_back(new Cube(glm::vec3(-0.1f, -0.5f, -0.4f), 0.1f, white));//esquina frente
    objects.push_back(new Cube(glm::vec3(0.5f, -0.2f, -0.6f), 0.1f, white));//esquina derecha
    objects.push_back(new Cube(glm::vec3(0.5f, -0.2f, -0.5f), 0.1f, white));//esquina derecha
    objects.push_back(new Cube(glm::vec3(0.0f, -0.2f, -1.0f), 0.1f, white));//esquina atras
    objects.push_back(new Cube(glm::vec3(0.1f, -0.2f, -1.0f), 0.1f, white));//esquina atras
    objects.push_back(new Cube(glm::vec3(-0.1f, -0.2f, -1.0f), 0.1f, white));//esquina atras
    objects.push_back(new Cube(glm::vec3(-0.5f, -0.2f, -0.5f), 0.1f, white));//esquina izquierda
    objects.push_back(new Cube(glm::vec3(-0.5f, -0.2f, -0.6f), 0.1f, white));//esquina izquierda
    objects.push_back(new Cube(glm::vec3(-0.4f,-0.2f, -0.4f), 0.1f, white));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(0.4f,-0.2f, -0.4f), 0.1f, white));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(-0.3f,-0.2f, -0.3f), 0.1f, white));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(0.3f,-0.2f, -0.3f), 0.1f, white));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(-0.2f,-0.2f, -0.2f), 0.1f, white));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(0.2f,-0.2f, -0.2f), 0.1f, white));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(-0.3f,-0.3f, -0.3f), 0.1f, white));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(0.3f,-0.3f, -0.3f), 0.1f, white));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(-0.3f,-0.3f, -0.4f), 0.1f, white));//esquina cruzada adelante
    objects.push_back(new Cube(glm::vec3(0.3f,-0.3f, -0.4f), 0.1f, white));//esquina cruzada adelante




    Material grey = {
    Color(128, 128, 128),  // Color gris
    1.0,                    // Coeficiente de reflexión difusa
    0.5,                    // Coeficiente de reflexión especular
    9.0f,                   // Exponente especular
    0.0f,                   // Transmitancia
    0.5f                    // Índice de refracción
};
    objects.push_back(new Cube(glm::vec3(0.0f, 0.0f, 0.0f), 0.1f, grey));


}


void render() {
    float fov = 3.1415/3;
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            /*
            float random_value = static_cast<float>(std::rand())/static_cast<float>(RAND_MAX);
            if (random_value < 0.0) {
                continue;
            }
            */


            float screenX = (2.0f * (x + 0.5f)) / SCREEN_WIDTH - 1.0f;
            float screenY = -(2.0f * (y + 0.5f)) / SCREEN_HEIGHT + 1.0f;
            screenX *= ASPECT_RATIO;
            screenX *= tan(fov/2.0f);
            screenY *= tan(fov/2.0f);


            glm::vec3 cameraDir = glm::normalize(camera.target - camera.position);

            glm::vec3 cameraX = glm::normalize(glm::cross(cameraDir, camera.up));
            glm::vec3 cameraY = glm::normalize(glm::cross(cameraX, cameraDir));
            glm::vec3 rayDirection = glm::normalize(
                cameraDir + cameraX * screenX + cameraY * screenY
            );
           
            Color pixelColor = castRay(camera.position, rayDirection);
            /* Color pixelColor = castRay(glm::vec3(0,0,20), glm::normalize(glm::vec3(screenX, screenY, -1.0f))); */

            point(glm::vec2(x, y), pixelColor);
        }
    }
}

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    // Create a window
    SDL_Window* window = SDL_CreateWindow("Raycasting - FPS: 0", 
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                          SCREEN_WIDTH, SCREEN_HEIGHT, 
                                          SDL_WINDOW_SHOWN);

    if (!window) {
        SDL_Log("Unable to create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create a renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!renderer) {
        SDL_Log("Unable to create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;
    SDL_Event event;

    int frameCount = 0;
    Uint32 startTime = SDL_GetTicks();
    Uint32 currentTime = startTime;
    
    setUpPokeball();

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }

            if (event.type == SDL_KEYDOWN) {
                switch(event.key.keysym.sym) {
                    case SDLK_UP:
                        camera.move(1.0f);
                        break;
                    case SDLK_DOWN:
                        camera.move(-1.0f);
                        break;
                    case SDLK_LEFT:
                        camera.rotate(-1.0f, 0.0f);
                        break;
                    case SDLK_RIGHT:
                        camera.rotate(1.0f, 0.0f);
                        break;
                    case SDLK_RETURN: //enter
                        camera.rotate(0.0f, 1.0f);
                        break;
                    case SDLK_SPACE:
                        camera.rotate(0.0f, -1.0f);
                        break;
                }
            }


        }

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        render();

        // Present the renderer
        SDL_RenderPresent(renderer);

        frameCount++;

        // Calculate and display FPS
        if (SDL_GetTicks() - currentTime >= 1000) {
            currentTime = SDL_GetTicks();
            std::string title = "Raycasting  - FPS: " + std::to_string(frameCount);
            SDL_SetWindowTitle(window, title.c_str());
            frameCount = 0;
        }
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

