/**********************************************************************
 * Some stuff to handle spheres
 **********************************************************************/
#include"include/vec.h"

typedef struct sphere {
  int index;               // identifies a sphere; must be greater than 0

  vec3 center;
  float radius;

  vec3 mat_ambient;    // material property used in Phong model
  vec3 mat_diffuse;
  vec3 mat_specular;
  vec3 mat_shineness;

  float reflectance;       // this number [0,1] determines how much 
                           // reflected light contributes to the color
                           // of a pixel
  struct sphere *next;
} Spheres;   // a list of spheres

// intersect ray with sphere
Spheres *intersect_scene(vec3, vec3, Spheres *, vec3 *);
// return the unit normal at a point on sphere
vec3 sphere_normal(vec3, Spheres *);
// add a sphere to the sphere list
Spheres *add_sphere(Spheres *, vec3, float, vec3 , vec3, vec3, float, float, int);

