#include "sphere.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <float.h>

/**********************************************************************
 * This function intersects a ray with a given sphere 'sph'. You should
 * use the parametric representation of a line and do the intersection.
 * The function should return the parameter value for the intersection, 
 * which will be compared with others to determine which intersection
 * is closest. The value -1.0 is returned if there is no intersection
 *
 * If there is an intersection, the point of intersection should be
 * stored in the "hit" variable
 **********************************************************************/

bool sphere_shadow(Point o, Vector v, Spheres *sph)
{
  bool inShadow = false;
  Point hit;
  Spheres * hit_sph = intersect_scene(o, v, sph, &hit, 0);
  if (hit_sph)
    inShadow = true;
  
  return inShadow;
}

float intersect_sphere(Point o, Vector v, Spheres *sph, Point *hit) {
  //geometric solution of line-sphere intersection
  
  //solve the first triangle 
  Vector L = get_vec(o, sph->center); //L is the vector between sphere center (C) and eye position (O) = C - O
  float hypotenuse = vec_len(L); 
  float ajacent = vec_dot(L, v); //a is the projection of L onto the ray v
  
  //check cases when there's no intersection
  if(ajacent < 0)  
    return FLT_MAX; 
  float opposite_square = hypotenuse * hypotenuse - ajacent * ajacent; 
  if (opposite_square > pow(sph->radius, 2) ) 
    return FLT_MAX; 
  
  //solve the second triangle to get t
  float t1c = sqrt( pow(sph->radius, 2) - opposite_square ); //t1c is the opposite of the second/inner triangle
  float t1 = ajacent - t1c;

  return t1;
}


/*********************************************************************
 * This function returns a pointer to the sphere object that the
 * ray intersects first; NULL if no intersection. You should decide
 * which arguments to use for the function. For exmaple, note that you
 * should return the point of intersection to the calling function.
 **********************************************************************/
Spheres *intersect_scene(Point o, Vector v, Spheres *sph, Point *hit, int i) {
//
// do your thing here
//
  Spheres * first_hit_sph = NULL;
  float distance = FLT_MAX;  

  while(sph)
  {
    // normalize(&v);
    float t1 = intersect_sphere(o, v, sph, hit);
    if ( t1 < distance) 
    {
      distance = t1;
      first_hit_sph = sph;
      hit->x = o.x + v.x * distance;
      hit->y = o.y + v.y * distance;
      hit->z = o.z + v.z * distance;
    }
    
    sph = sph -> next;
  }

	return first_hit_sph;
}

/*****************************************************
 * This function adds a sphere into the sphere list
 *
 * You need not change this.
 *****************************************************/
Spheres *add_sphere(Spheres *slist, Point ctr, float rad, float amb[],
		    float dif[], float spe[], float shine, 
		    float refl, int sindex) {
  Spheres *new_sphere;

  new_sphere = (Spheres *)malloc(sizeof(Spheres));
  new_sphere->index = sindex;
  new_sphere->center = ctr;
  new_sphere->radius = rad;
  (new_sphere->mat_ambient)[0] = amb[0];
  (new_sphere->mat_ambient)[1] = amb[1];
  (new_sphere->mat_ambient)[2] = amb[2];
  (new_sphere->mat_diffuse)[0] = dif[0];
  (new_sphere->mat_diffuse)[1] = dif[1];
  (new_sphere->mat_diffuse)[2] = dif[2];
  (new_sphere->mat_specular)[0] = spe[0];
  (new_sphere->mat_specular)[1] = spe[1];
  (new_sphere->mat_specular)[2] = spe[2];
  new_sphere->mat_shineness = shine;
  new_sphere->reflectance = refl;
  new_sphere->next = NULL;

  if (slist == NULL) { // first object
    slist = new_sphere;
  } else { // insert at the beginning
    new_sphere->next = slist;
    slist = new_sphere;
  }

  return slist;
}

/******************************************
 * computes a sphere normal - done for you
 ******************************************/
Vector sphere_normal(Point q, Spheres *sph) {
  Vector rc;

  rc = get_vec(sph->center, q);
  normalize(&rc);
  return rc;
}
