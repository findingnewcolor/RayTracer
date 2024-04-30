#include "rtweekend.h"

#include "box.h"
#include "bvh.h"
#include "camera.h"
#include "color.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "texture.h"

#include <iostream>

#include <chrono>
#include <ctime>    
#include <limits.h>
#include <fstream>


color ray_color(const ray& r, const color& background, const hittable& world, int depth) {
    hit_record rec;

    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
        return color(0,0,0);

    // If the ray hits nothing, return the background color.
    if (!world.hit(r, 0.001, infinity, rec))
        return background;

    ray scattered;
    color attenuation;
    color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);

    if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
        return emitted;

    return emitted + attenuation * ray_color(scattered, background, world, depth-1);
}



hittable_list myRTscape2() {
    hittable_list objects;

    // auto pertext = make_shared<solid_color>(0.3, 0.8, 0.1);//I have no clue what this color will be!
    auto material1 = make_shared<lambertian>(color(0.686,0.2,0.039));
    objects.add(make_shared<sphere>(point3(0,-1000,0), 1000, material1));
    //center ball
    auto material2 = make_shared<metal>(color(0.78,0.84,0.835), 0.1);
    objects.add(make_shared<sphere>(point3(-0.5,1.5,-1), 1.5, material2));

    //little balls submerged in surface
    objects.add(make_shared<sphere>(point3(4.8,0.066,2), 0.3, material2));//front left
    objects.add(make_shared<sphere>(point3(-7,0.4,0.6), 0.3, material2));//back left behind center ball
    objects.add(make_shared<sphere>(point3(0,0.066,0.3), 0.3, material2));//near left of center ball
    objects.add(make_shared<sphere>(point3(-0.3,0.066,4), 0.3, material2));//left and behind camera
    //bigger
    objects.add(make_shared<sphere>(point3(4,0.2,-0.5), 0.5, material2));

    //glass ball in front of doorway and red blob to defract light
    auto material3 = make_shared<dielectric>(0.4);
    objects.add(make_shared<sphere>(point3(3.5,2,-5), 0.4, material3));
    objects.add(make_shared<sphere>(point3(4.1,1.1,-4.5), 0.4, material3));

    //red blob
    // auto material4 = make_shared<diffuse_light>(color(4,4,4));
    auto material4 = make_shared<diffuse_light>(color(9,0,0));
    objects.add(make_shared<sphere>(point3(4,2,-5), 0.6, material4));
    objects.add(make_shared<sphere>(point3(3.3, 0.67, -3.9), 0.2, material4));

    // auto material5 = make_shared<diffuse_light>(color(4,4,4));
    auto material5 = make_shared<diffuse_light>(color(9,0,0));
    objects.add(make_shared<sphere>(point3(3.85,1.33,-4.9), 0.4, material5));
    objects.add(make_shared<sphere>(point3(4.1, 0.43, -5.11), 0.8, material5));

    //red black floaty ball
    auto material6 = make_shared<lambertian>(color(0.22, 0.016, 0.016));
    objects.add(make_shared<sphere>(point3(3.65,0.82,-1.5), 0.25, material6));
    objects.add(make_shared<sphere>(point3(-3.5,1.2,3.5), 1, material6));

    //black speck in front of doorway
    objects.add(make_shared<sphere>(point3(5.1,2.47,-4.83), 0.45, material6));

    // sun
    auto material7 = make_shared<diffuse_light>(color(20,0,0));
    objects.add(make_shared<sphere>(point3(-20,10,-50), 4.5, material7));
    auto material8 = make_shared<dielectric>(20.0);
    objects.add(make_shared<sphere>(point3(-20,10,-50), 5, material8));

    //original doorway
    auto difflightW = make_shared<diffuse_light>(color(4,4,4));
    objects.add(make_shared<xy_rect>(3, 5, 0.25, 3, -5, difflightW));

    //more doors
    auto difflightOffW = make_shared<diffuse_light>(color(20,2,4));
    objects.add(make_shared<yz_rect>(1, 3, -2, 0, -5, difflightOffW));
    objects.add(make_shared<yz_rect>(1, 3, 0, 2, -5, difflightOffW));
    objects.add(make_shared<yz_rect>(1, 3, 3, 5, -5, difflightOffW));
    objects.add(make_shared<yz_rect>(1, 3, 6, 8, -5, difflightOffW));
    objects.add(make_shared<yz_rect>(1, 3, 9, 11, -5, difflightOffW));
    objects.add(make_shared<yz_rect>(1, 3, 12, 14, -5, difflightOffW));

    //cube of cubes
    auto blue   = make_shared<lambertian>(color(.15, .05, .65));
    auto metalB   = make_shared<metal>(color(0.78,0.84,0.835), 0.1);
    shared_ptr<hittable> box1 = make_shared<box>(point3(1,0,1), point3(2,1,2), metalB);
    box1 = make_shared<rotate_y>(box1, 11);
    objects.add(box1);

    shared_ptr<hittable> box2 = make_shared<box>(point3(1,0,1), point3(2,1,2), blue);
    box2 = make_shared<rotate_y>(box2, 11);
    box2 = make_shared<translate>(box2, vec3(1.2,0,1.2));
    objects.add(box2);

    shared_ptr<hittable> box3 = make_shared<box>(point3(1,0,1), point3(2,1,2), metalB);
    box3 = make_shared<rotate_y>(box3, 11);
    box3 = make_shared<translate>(box3, vec3(1.7,0.9,1.3));
    objects.add(box3);

    return objects;
}


int main() {

    // Image

    auto aspect_ratio = 16.0 / 9.0;
    int image_width = 1200;
    // int image_width = 400;
    int samples_per_pixel = 200;
    // int samples_per_pixel = 50;
    int max_depth = 25;

    // World

    hittable_list world = myRTscape2();
    // color background(0.18,0.286,0.19);
    color background(0,0,0);

    // Camera

    point3 lookfrom = point3(0,2,10);
    point3 lookat = point3(0,1,0);
    auto vfov = 30.0;
    auto aperture = 0.1;

    const vec3 vup(0,1,0);
    const auto dist_to_focus = 10.0;
    const int image_height = static_cast<int>(image_width / aspect_ratio);

    camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);

    // Render


    auto start = std::chrono::system_clock::now();


    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    for (int j = image_height-1; j >= 0; --j) {
        std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < image_width; ++i) {
            color pixel_color(0,0,0);
            for (int s = 0; s < samples_per_pixel; ++s) {
                auto u = (i + random_double()) / (image_width-1);
                auto v = (j + random_double()) / (image_height-1);
                ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, background, world, max_depth);
            }
            write_color(std::cout, pixel_color, samples_per_pixel);
        }
    }

    std::cerr << "\nDone.\n";


    std::fstream fout("full_res_elapsed.txt", std::fstream::out);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
    fout << "finished computation at " << std::ctime(&end_time)
              << "elapsed time: " << elapsed_seconds.count() << "s"
              << std::endl;
}
