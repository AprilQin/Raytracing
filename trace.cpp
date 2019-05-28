 #include <stdio.h>
#include <GL/glut.h>
#include <math.h>
#include "global.h"
#include "sphere.h"

//
// Global variables
//
extern int win_width;
extern int win_height;

extern GLfloat frame[WIN_HEIGHT][WIN_WIDTH][3];  

extern float image_width;
extern float image_height;

extern Point eye_pos;
extern float image_plane;
extern RGB_float background_clr;
extern RGB_float null_clr;

extern Spheres *scene;

// light 1 position and color
extern Point light1;
extern float light1_ambient[3];
extern float light1_diffuse[3];
extern float light1_specular[3];

// global am  bient term
extern float global_ambient[3];

// light decay parameters
extern float decay_a;
extern float decay_b;
extern float decay_c;

extern int shadow_on;
extern int reflection_on;
extern int refraction_on;
extern int stochastic_on;
extern int supersampling_on;
extern int chessboard_on;

extern int step_max; 


Vector board_norm = {0, 1, 0};
Point board_center = {0, -2, -6};
/////////////////////////////////////////////////////////////////////



/*********************************************************************
 * Define chessboard
 *********************************************************************/

bool intersect_chessboard(Point o, Vector v, Point *hit)
{
    Vector vec;
    vec.x = board_center.x - o.x;
    vec.y = board_center.y - o.y;
    vec.z = board_center.z - o.z;

    float t = vec_dot( vec, board_norm ) / vec_dot (v, board_norm); // (board_center_p - eye_pos) * board_normal / current_ray * board_normal

    if (t > 0) {

        int x = (int) floor( o.x + t * v.x );
        int z = (int) floor( o.z + t * v.z );
        
        if (x < 4 && x >= -4 && z < -2 && z >= -10  ) {
            hit->x = o.x + t * v.x;
            hit->y = o.y + t * v.y;
            hit->z = o.z + t * v.z;
            return true;
        }
 
    }

    return false;
}

RGB_float get_board_color_at(Point p)
{
    RGB_float color;
    int i = (int) floor(p.x);
    int j = (int) floor(p.z);

    //assign colors
    if ( (i % 2 == 0 && j % 2 ==0) || (i%2 !=0 && j%2 != 0) ) 
        color = {1, 1, 0};
    else 
        color = {0, 0, 0};
    
    return color;
}


/*********************************************************************
 * Phong illumination - you need to implement this!
 *********************************************************************/
RGB_float phong(Point q, Vector v, Vector surf_norm, Spheres *sph) {
//
// do your thing here
//
    RGB_float color = null_clr;   
    Vector l = get_vec(q, light1); //it's the vector between hit point and light source
    normalize( &l);

    //add global ambient light
    color.r += global_ambient[0] * sph->mat_ambient[0];
    color.g += global_ambient[1] * sph->mat_ambient[1];
    color.b += global_ambient[2] * sph->mat_ambient[2];

    //check if the area is not in shadow, we add diffuse and specular light
    if (!shadow_on || !sphere_shadow(q, l, scene)) //check if the ray from hit point to the light source is blocked by any objects
    {
            //add light1 ambient light  
        color.r += light1_ambient[0] * sph->mat_ambient[0]; 
        color.g += light1_ambient[1] * sph->mat_ambient[1];
        color.b += light1_ambient[2] * sph->mat_ambient[2];

        Vector r = vec_minus(l, vec_scale( surf_norm, 2 * (vec_dot(l, surf_norm)))); //refl = l - 2(l dot n)*n
        normalize(&r); //normalize reflection vec for calculating specular light     
        float d = vec_len(get_vec(q, light1)); //calculate the common scale for diffuse and specular light
        float scale = 1 / (decay_a + decay_b * d + decay_c * pow (d, 2));  //1/ (a + bd + cd^2) where d is the distance between the light source and the point on the object & a, b, c is the decay value

        //diffuse light = coef * Id * Kd
        float  ndotl  = vec_dot(surf_norm, l);
        color.r += scale * light1_diffuse[0] * sph->mat_diffuse[0] * ndotl; //diffuse light r component
        color.g += scale * light1_diffuse[1] * sph->mat_diffuse[1] * ndotl; //diffuse light g component
        color.b += scale * light1_diffuse[2] * sph->mat_diffuse[2] * ndotl; //diffuse light b component

        //specular light = coef * Is * Id
        float N = sph->mat_shineness;
        float rdotv = vec_dot(r, v);
        color.r += scale * (light1_specular[0] * sph->mat_specular[0] * pow(rdotv, N));
        color.g += scale * (light1_specular[1] * sph->mat_specular[1] * pow(rdotv, N));
        color.b += scale * (light1_specular[2] * sph->mat_specular[2] * pow(rdotv, N));
    }
   
  	return color;
}

/************************************************************************
 * This is the recursive ray tracer - you need to implement this!
 * You should decide what arguments to use.
 ************************************************************************/
RGB_float recursive_ray_trace(Point o, Vector v, int step) { //v is the vector from eye to intersection point on the sphere object
//
// do your thing here
//
    RGB_float color = background_clr; //initilize the color of the current pixel to black
    Point hit;
    Spheres *first_hit_sph = intersect_scene(o, v, scene, &hit, 0); //check if the ray hits any object in the scene and store the result in sphere. NULL if nothing found, 
                                                               //pointer of the sphere if found


    //illuminate board
    Point board_hit;
    if (chessboard_on && intersect_chessboard(o,v, &board_hit))
    {
        //printf("drawing my chessboard\n");
        Vector l = get_vec(board_hit, light1);
        normalize(&l);
        color = get_board_color_at(board_hit);

        if (shadow_on  && sphere_shadow(board_hit, l, scene)) {
            color = clr_scale(color, 0.4);
        }

        if (reflection_on && step < step_max) {
            Vector reflected_light = vec_plus(vec_scale(board_norm, -2 * vec_dot(board_norm, v)), v);
            normalize(&reflected_light);
            RGB_float reflected_color = recursive_ray_trace(board_hit, reflected_light, step + 1);
            color = clr_add(color, clr_scale(reflected_color, 0.3));

            // if (stochastic_on)
            // {
                
            // }
        }
    }

    //illuminate spheres
    if (first_hit_sph) 
    {
        Vector n = sphere_normal(hit, first_hit_sph); //n is the surface normal and already normalized  
        color = phong(hit, v, n, first_hit_sph); 

        //accumumated color from reflected ray. I = I phong + reflectance * I reflect
        if (reflection_on && step < step_max) 
        {
          Vector reflected_light = vec_minus(v, vec_scale(n, 2 * vec_dot(n, v)));
          normalize(&reflected_light);
          RGB_float reflected_color = recursive_ray_trace( hit, reflected_light, step + 1);
          color = clr_add(color, clr_scale(reflected_color, first_hit_sph->reflectance));

          // if (stochastic_on)
          // {
                
          // }
        } 

        // if(refraction_on)
        // {
        //   //make spheres transparent 
        // }
    }





  	return color;
}

/*********************************************************************
 * This function traverses all the pixels and cast rays. It calls the
 * recursive ray tracer and assign return color to frame
 *
 * You should not need to change it except for the call to the recursive
 * ray tracer. Feel free to change other parts of the function however,
 * if you must.
 *********************************************************************/
void ray_trace() {
  int i, j;
  float x_grid_size = image_width / float(win_width);
  float y_grid_size = image_height / float(win_height);
  float x_start = -0.5 * image_width;
  float y_start = -0.5 * image_height;
  RGB_float ret_color;
  Point cur_pixel_pos;
  Vector ray;

  // ray is cast through center of pixel
  cur_pixel_pos.x = x_start + 0.5 * x_grid_size;
  cur_pixel_pos.y = y_start + 0.5 * y_grid_size;
  cur_pixel_pos.z = image_plane;

  for (i=0; i<win_height; i++) {
    for (j=0; j<win_width; j++) {
      ray = get_vec(eye_pos, cur_pixel_pos); //define a ray from eye to center of the current pixel
      normalize( &ray );
      //
      // You need to change this!!!
      //
      //ret_color = background_clr; // just background for now

      // Parallel rays can be cast instead using below
      //
      // ray.x = ray.y = 0;
      // ray.z = -1.0;
      // ret_color = recursive_ray_trace(cur_pixel_pos, ray, 1);

      // Checkboard for testing
      //RGB_float clr = {float(i/32), 0, float(j/32)};
      //ret_color = clr;
      ret_color = recursive_ray_trace(eye_pos, ray, 1);

      if (supersampling_on) {
          int n = 4;
          float x = x_grid_size/n;
          float y = y_grid_size/n;
          Point sampling_pos = cur_pixel_pos;
          Vector sampling_ray;

          //two points on the left
          sampling_pos.x = cur_pixel_pos.x - x;
          sampling_pos.y = cur_pixel_pos.y + y;
          sampling_ray = get_vec(eye_pos, sampling_pos);
          normalize( &sampling_ray);
          ret_color = clr_add(ret_color, recursive_ray_trace(eye_pos, sampling_ray, 1));

          sampling_pos.y = cur_pixel_pos.y - y;
          sampling_ray = get_vec(eye_pos, sampling_pos);
          normalize( &sampling_ray);
          ret_color = clr_add(ret_color, recursive_ray_trace(eye_pos, sampling_ray, 1));

          //two points on the right
          sampling_pos.x = cur_pixel_pos.x + x;
          sampling_pos.y = cur_pixel_pos.y + y;
          sampling_ray = get_vec(eye_pos, sampling_pos);
          normalize( &sampling_ray);
          ret_color = clr_add(ret_color, recursive_ray_trace(eye_pos, sampling_ray, 1));

          sampling_pos.y = cur_pixel_pos.y - y;
          sampling_ray = get_vec(eye_pos, sampling_pos);
          normalize( &sampling_ray);
          ret_color = clr_add(ret_color, recursive_ray_trace(eye_pos, sampling_ray, 1));

          //take average
          ret_color = clr_scale(ret_color, 1.0 / 5);
      }

      frame[i][j][0] = ret_color.r;
      frame[i][j][1] = ret_color.g;
      frame[i][j][2] = ret_color.b;

      cur_pixel_pos.x += x_grid_size;
    }

    cur_pixel_pos.y += y_grid_size;
    cur_pixel_pos.x = x_start;
  }
}
