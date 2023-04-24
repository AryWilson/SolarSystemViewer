//--------------------------------------------------
// Author: Ary Wilson
// Date: 2/18/23
// Description: Loads PLY files in ASCII format
//--------------------------------------------------
#define _USE_MATH_DEFINES
#include <cmath>
#include <string>
#include <vector>
#include "agl/window.h"
#include "plymesh.h"
#include "osutils.h"

using namespace std;
using namespace glm;
using namespace agl;

struct Material
{
   float ka;
   float kd;
   float ks;
   float alpha;
};

struct Light
{
   vec3 pos;
   vec3 col;
};

struct Particle
{
   glm::vec3 pos;
   glm::vec4 color;
   float rot;
   float size;
};

struct Planet
{
   // position, velocity, redius, texture
   float size;
   float radius;
   float vel;
   vec3 pos;
   string texture;
   vector<Particle> trail;
};

const int PLANET_COUNT = 6;
const int PARTICLE_COUNT = 10;
const float ORBIT = 12;
const float STAR_SIZE = 1.5;

class Viewer : public Window
{
public:
   Viewer() : Window()
   {
      eyePos = vec3(7, 0, 0);
      lookPos = vec3(0, 0, 0);
      upDir = vec3(0, 1, 0);
      mesh = PLYMesh("../models/planet.ply");
      shaders = {"unlit", "phong-texture"};

      radius = 10;
      azimuth = 0;
      elevation = 0;
      update_time = 0;
      material = {0.1f, 0.6f, 0.8f, 15.0f};
      light = {lookPos, vec3(1.0f, 1.0f, 1.0f)};
      single_planet = 0;
      single = false;
      bbCentx = 0;
      bbCenty = 0;
      bbCentz = 0;
      d = 1;
   }

   void setup()
   {
      for (string s : shaders)
      {
         renderer.loadShader(s, "../shaders/" + s + ".vs", "../shaders/" + s + ".fs");
      }
      // renderer.loadCubeMap();
      textures = GetFilenamesInDir("../textures", "png");

      renderer.loadCubemap("cubemap", "../textures/cubemap", 0);

      for (int i = 0; i < textures.size(); i++)
      {
         renderer.loadTexture(textures[i], "../textures/" + textures[i], i + 1);
      }
      renderer.loadTexture("particle", "../textures/particle/particle.png", textures.size() + 1);


      initPlanets();
      setMeshDim(mesh);
   }

   void mouseMotion(int x, int y, int dx, int dy)
   {
      if (mouseIsDown(GLFW_MOUSE_BUTTON_LEFT))
      {
         azimuth -= dx * (0.02f);
         azimuth = fmod(azimuth, 2 * M_PI);

         elevation += dy * (0.02f);
         // elevation += M_PI_2;
         // elevation = fmod(elevation,M_PI);
         // elevation -= M_PI_2;

         if (elevation >= M_PI_2)
         {
            elevation = M_PI_2 - 0.01f;
         }
         if (elevation <= -1 * M_PI_2)
         {
            elevation = -1 * M_PI_2 + 0.01f;
         }
      }
   }

   void initPlanets()
   {
      // vector<float> radii;
      // for(int i = 0; i< PLANET_COUNT; i++){
      //    radii[i] = agl::random(STAR_SIZE+1,ORBIT);
      // }
      // sort(radii.begin(), radii.end());

      // for(int i = 0; i< PLANET_COUNT; i++){
      //    Planet toAdd;
      //    toAdd.size = agl::random(0.1,0.7);
      //    toAdd.radius = radii[0];
      //    toAdd.vel = agl::random(0.1,0.5) * (ORBIT + 1 - radii[0]);
      //    int t = (int) agl::random( 0, textures.size());
      //    toAdd.texture = textures[t];
      //    planets.push_back(toAdd);
      // }
      
      Planet a, b, c, d, e, f;
      a.size = 1.0f / 6;
      b.size = 1.0f / 9;
      c.size = 1.0f / 7;
      d.size = 1.0f / 3;
      e.size = 1.0f / 6;
      f.size = 1.0f / 9;
      a.radius = 3.0f;
      b.radius = 4.0f;
      c.radius = 5.0f;
      d.radius = 8.0f;
      e.radius = 10.0f;
      f.radius = 11.0f;

      a.vel = 0.5f;
      b.vel = 0.4f;
      c.vel = 0.2f;
      d.vel = 0.5f;
      e.vel = 0.3f;
      f.vel = 0.9f;
      a.texture = "crater";
      b.texture = "smoke";
      c.texture = "jupiter";
      d.texture = "gas";
      e.texture = "swirl1";
      f.texture = "swirl2";
      planets.push_back(a);
      planets.push_back(b);
      planets.push_back(c);
      planets.push_back(d);
      planets.push_back(e);
      planets.push_back(f);

      Particle particle;
      particle.color = vec4(1.0f, 1.0f, .8f, 1.0f);
      particle.size = 0.1;
      particle.pos = vec3(0, 0, 0);
      for (int i = 0; i < PLANET_COUNT * PARTICLE_COUNT; i++)
      {
         planets[i / PARTICLE_COUNT].trail.push_back(particle);
      }
   }
   
   void setMeshDim(PLYMesh mesh)
   {
      // find bounding box
      vec3 bbMin = mesh.minBounds();
      vec3 bbMax = mesh.maxBounds();
      float bbCentx = (bbMin.x + bbMax.x) / 2.0f;
      float bbCenty = (bbMin.y + bbMax.y) / 2.0f;
      float bbCentz = (bbMin.z + bbMax.z) / 2.0f;
      // translate bounding box to 0,0,0
      float bbXlen = abs(bbMax.x - bbMin.x);
      float bbYlen = abs(bbMax.y - bbMin.y);
      float bbZlen = abs(bbMax.z - bbMin.z);
      float d = std::max(bbXlen, std::max(bbYlen, bbZlen));
   }

vec3 screenToWorld(const vec2& screen){
    vec4 screenPos = vec4(screen,1,1);

    //flip y coord 
    screenPos.y = height() - screenPos.y;

    // convert to canonical view coords
    screenPos.x = 2.0f*((screenPos.x / width()) - 0.5);
    screenPos.y = 2.0f*((screenPos.y / height()) - 0.5);

    //convert the particle position to screen coords
    mat4 projection = renderer.projectionMatrix();
    mat4 view = renderer.viewMatrix();
    
    vec4 worldPos = inverse(projection * view) * screenPos; 

    // convert from homogeneous to ordinary coord
    worldPos.x /= worldPos.w;
    worldPos.y /= worldPos.w;
    worldPos.z /= worldPos.w;
    return vec3(worldPos.x, worldPos.y, worldPos.z);
}

   bool sphereIntercetionBool(vec3 p0, vec3 v, vec3 c, float r){
      vec3 l = c - p0;
      float s = dot(l,normalize(v));
      if (s < 0){ // sphere is behind us
         return false;
      }
      float m2 = pow(length(l),2) - pow(s,2);
      if(m2 > pow(r,2)){
         return false;
      }

      float q = sqrt(pow(r,2) - m2);
      
      float t = s - q;
      vec3 far_a = v*(s + q);
      
      if(pow(length(l),2) > pow(r,2)){
         return false;
      } else {
         return true;
      }
      return false;
   }

   float sphereIntercetion(vec3 p0, vec3 v, vec3 c, float r){
      vec3 l = c - p0;
      float s = dot(l,normalize(v));
      if (s < 0){ // sphere is behind us
         return -1;
      }
      float m2 = pow(length(l),2) - pow(s,2);
      if(m2 > pow(r,2)){
         return -1;
      }

      float q = sqrt(pow(r,2) - m2);
      
      float t = s - q;
      vec3 far_a = v*(s + q);
      
      if(pow(length(l),2) > pow(r,2)){
         return -1;
      } else {
         return s;
      }
      return -1;
   }

   void mouseDown(int button, int mods){
      if(!single){
         vec2 mousePos = mousePosition();
         vec3 worldPos = screenToWorld(mousePos);
         vec3 rayDir = normalize(worldPos - eyePos);
         // sort planets by depth?
         for (int i = 0; i< planets.size(); i++){
            if(sphereIntercetionBool(eyePos, rayDir, planets[i].pos, planets[i].size)){
               single = true;
               single_planet = i;
               break;
            }
         }
      }
   }

   void _mouseDown(int button, int mods){
      if(!single){
         vec2 mousePos = mousePosition();
         vec3 worldPos = screenToWorld(mousePos);
         vec3 rayDir = normalize(worldPos - eyePos);
         float dists[PLANET_COUNT];
         memset(dists, -1, sizeof(dists));
         float min = ORBIT*4;

         for (int i = 0; i< planets.size(); i++){
            float dist = sphereIntercetion(eyePos, rayDir, planets[i].pos, planets[i].size);
            if(dist >= 0){
               single = true;
               // single_planet = i;
               if (dist < min){
                  min = dist;

               }
            }
         }
         if(single){
            for(int i = 0; i< PLANET_COUNT; i++){
               if(dists[i] >= 0 && dists[i] <= min + 0.0001){
                  single_planet = i;
                  break;
               }
            }
         }
      }
   }


   void scroll(float dx, float dy)
   {
      radius += dy;
      if (radius <= 0)
      {
         radius = 1;
      }
   }

   void keyUp(int key, int mods)
   {
      if (key >= 49 && key <= 49 + 6)
      {
         single = true;
         single_planet = key - 49;
      }
      else if (key == 27)
      {
         single = false;
      }
   }

   void update()
   {

      float x = radius * sin(azimuth) * cos(elevation);
      float y = radius * sin(elevation);
      float z = radius * cos(azimuth) * cos(elevation);
      eyePos = vec3(x, y, z);
   }

   // lookAt() changes the camera position and orientation
   // translate() applies a translation transformation
   // rotate() applies a rotation transformation
   // scale() applies a scale transformation
   // loockAt(vec3(0,0,0),lookPos,upDir);

   void updateTrail(float dt, vector<Particle> mParticles, vec3 position)
   {
      bool one = agl::random() > 0.5;

      for (int i = 0; i < mParticles.size(); i++)
      {

         if (one && mParticles[i].color.w <= 0)
         {
            // one new particle
            mParticles[i].pos = position;
            mParticles[i].color = vec4(1.0, 1.0, 0.8, 1.0);
            one = false;
         }
         else
         {
            // updates the opacity
            mParticles[i].color.w -= dt;
         }
      }
   }

   // render all sprites in pool
   void drawTrail(vector<Particle> mParticles)
   {
      renderer.texture("image", "particle");
      for (int i = 0; i < mParticles.size(); i++)
      {
         Particle particle = mParticles[i];
         renderer.sprite(particle.pos, particle.color, particle.size);
      }
   }

   void drawSingle(Planet planet)
   {
      // ---SINGLE PLANET
      renderer.beginShader("phong-texture"); // activates shader with given name
      float aspect = ((float)width()) / height();
      renderer.texture("diffuseTexture", planet.texture);

      float theta = elapsedTime();
      float v = planet.vel;
      float s = planet.size;

      renderer.rotate(v * theta, vec3(0, 1, 0));
      renderer.scale(vec3(s / d, s / d, s / d));
      renderer.translate(vec3(-1 * bbCentx, -1 * bbCenty, -1 * bbCentz));
      renderer.mesh(mesh);

      renderer.setUniform("ProjMatrix", renderer.projectionMatrix());
      renderer.setUniform("material.kd", material.kd);
      renderer.setUniform("material.ks", material.ks);
      renderer.setUniform("material.ka", material.ka);
      renderer.setUniform("material.alpha", material.alpha);
      renderer.setUniform("light.pos", eyePos);
      renderer.setUniform("light.col", light.col);
      renderer.lookAt(eyePos, lookPos, upDir);
      renderer.perspective(glm::radians(60.0f), aspect, 0.1f, 50.0f);

      renderer.endShader();
      return;
   }

   void drawSpace(vector<Planet> planets)
   {
      // ---STAR---
      renderer.beginShader("unlit"); // activates shader with given name
      float aspect = ((float)width()) / height();
      renderer.push(); // push identity
      renderer.scale(vec3(STAR_SIZE));
      renderer.push(); // push star scale
      
      renderer.scale(vec3(1.0f / d, 1.0f / d, 1.0f / d));
      renderer.translate(vec3(-1 * bbCentx, -1 * bbCenty, -1 * bbCentz));
      renderer.push(); // push mesh matrix
     
      

      renderer.mesh(mesh);
      renderer.endShader();

      // ---PLANETS---
      renderer.pop(); // get matrix for star
      renderer.beginShader("phong-texture");

      float theta = elapsedTime();
      float delta = dt();
      update_time += dt();
      bool update = false;
      if (update_time >= 5)
      {
         update_time = 0;
         update = true;
      }

      for (int i = 0; i < planets.size(); i++)
      {
         renderer.push();// save matrix for star
         renderer.texture("diffuseTexture", planets[i].texture); //?
         float r = planets[i].radius;
         float v = planets[i].vel;
         float s = planets[i].size;
         vec3 pos = vec3(r * cos(v * theta), 0, r * sin(v * theta));
         planets[i].pos = pos;
         renderer.translate(pos);
         renderer.scale(vec3(s, s, s));

         if (update)
         {
            updateTrail(delta, planets[i].trail, pos);
         }

         // renderer.sphere();
         renderer.mesh(mesh);
         renderer.pop(); // reset to mesh matrix
      }

      renderer.pop(); // remove star size
      renderer.pop(); // reset to identity
      for (int i = 0; i < planets.size(); i++)
      {
         drawTrail(planets[i].trail);
      }

      renderer.setUniform("ProjMatrix", renderer.projectionMatrix());
      renderer.setUniform("material.kd", material.kd);
      renderer.setUniform("material.ks", material.ks);
      renderer.setUniform("material.ka", material.ka);
      renderer.setUniform("material.alpha", material.alpha);
      renderer.setUniform("light.pos", light.pos);
      renderer.setUniform("light.col", light.col);
      renderer.lookAt(eyePos, lookPos, upDir);
      renderer.perspective(glm::radians(60.0f), aspect, 0.1f, 50.0f);

      renderer.endShader();
      return;
   }
   // load in mesh
   void fnExit() { mesh.clear(); }

   void draw()
   {
      // update campos
      update();
      renderer.beginShader("cubemap");
      renderer.skybox(ORBIT + 1);
      renderer.endShader();

      if (single)
      {
         // ---SINGLE PLANET
         drawSingle(planets[single_planet]);
      }
      else
      {
         // ---STAR AND PLANETS
         drawSpace(planets);
      }

      // renderer.setUniform("ViewMatrix", renderer.viewMatrix());
      // renderer.setUniform("ProjMatrix", renderer.projectionMatrix());
      // renderer.setUniform("HasUV", mesh.hasUV());
      // renderer.setUniform("ModelViewMatrix", renderer.);
      // renderer.setUniform("NormalMatrix", renderer.);
      // renderer.setUniform("eyePos", eyePos);
      // renderer.setUniform("material.kd", material.kd);
      // renderer.setUniform("material.ks", material.ks);
      // renderer.setUniform("material.ka", material.ka);
      // renderer.setUniform("material.alpha", material.alpha);
      // renderer.setUniform("light.pos", light.pos);
      // renderer.setUniform("light.col", light.col);
      // renderer.lookAt(eyePos,lookPos,upDir);
      // renderer.perspective(glm::radians(60.0f), aspect, 0.1f, 50.0f);
   }

protected:
   PLYMesh mesh;
   vec3 eyePos;
   vec3 lookPos;
   vec3 upDir;
   float radius;
   float azimuth;   // in [0, 360]
   float elevation; // in [-90, 90]
   Material material;
   Light light;
   vector<string> textures;
   vector<Planet> planets;
   vector<string> shaders;

   float update_time;
   bool single;
   int single_planet;
   float bbCentx;
   float bbCenty;
   float bbCentz;
   float d;
};

// void fnExit(){ }

int main(int argc, char **argv)
{
   // atexit (fnExit);

   Viewer viewer;
   viewer.run();
   return 0;
}
