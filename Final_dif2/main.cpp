#include "rtweekend.h"
#include "camera.h"
#include "color.h"
#include "material.h"
#include "hittable_list.h"
#include "sphere.h"
#include "triangle.h"

#include <iostream>

class MoonWithHoles : public hittable {
public:
    MoonWithHoles(const point3& center, double radius, shared_ptr<material> material)
        : center(center), radius(radius), mat_ptr(material) {}

    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;

private:
    point3 center;
    double radius;
    shared_ptr<material> mat_ptr;
};

bool MoonWithHoles::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    vec3 oc = r.origin() - center;
    auto a = r.direction().length_squared();
    auto half_b = dot(oc, r.direction());
    auto c = oc.length_squared() - radius * radius;
    auto discriminant = half_b * half_b - a * c;

    if (discriminant > 0) {
        auto root = sqrt(discriminant);
        auto temp = (-half_b - root) / a;

        if (temp < t_max && temp > t_min) {
            rec.t = temp;
            rec.p = r.at(rec.t);
            rec.normal = (rec.p - center) / radius;
            rec.mat_ptr = mat_ptr;
            return true;
        }

        temp = (-half_b + root) / a;
        if (temp < t_max && temp > t_min) {
            rec.t = temp;
            rec.p = r.at(rec.t);
            rec.normal = (rec.p - center) / radius;
            rec.mat_ptr = mat_ptr;
            return true;
        }
    }

    return false;
}


hittable_list random_scene() {
    hittable_list world;

    // Green Lambertian floor
    auto ground_material = make_shared<lambertian>(color(0.1, 0.8, 0.3));
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

    // Add grass-like spheres
     int num_grass_spheres = 500;
    for (int i = 0; i < num_grass_spheres; i++) {
        auto albedo = color(1.0, 1.0, 0.0); // Bright yellow color
        auto fuzz = random_double(0, 0.2);
        auto sphere_material = make_shared<lambertian>(albedo);

        double radius = 0.05;
        double x = random_double(-15, 15);
        double z = random_double(-15, 15);
        double y = radius + 0.1 * random_double();
        world.add(make_shared<sphere>(point3(x, y, z), radius, sphere_material));
    }
    // Add emissive spheres (stars) in the sky
    int num_stars = 100;
    for (int i = 0; i < num_stars; i++) {
        double radius = 0.05;
        double x = random_double(-15, 15);
        double y = random_double(5, 20); // Adjust the height for the stars
        double z = random_double(-15, 15);
        color star_color = color(1.0, 1.0, 1.0) * random_double(2.0, 5.0); // Brightness of stars
        auto emissive_material = make_shared<lambertian>(star_color);
        world.add(make_shared<sphere>(point3(x, y, z), radius, emissive_material));
    }
    

     auto moon_albedo = color(0.8, 0.8, 0.8); // Gray color for moon
    auto moon_fuzz = random_double(0, 0.1);
    auto moon_material = make_shared<lambertian>(moon_albedo);
    world.add(make_shared<MoonWithHoles>(point3(0, 10, 0), 3, moon_material));



    // Your original spheres
    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(0, 1.0, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(make_shared<sphere>(point3(-1.5, 0.5, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(point3(1.5, 0.5, 0), 1.0, material3));

    return world;
}


color ray_color(const ray& r, const hittable& world, int depth) {
    hit_record rec;

    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
        return color(0, 0, 0);

    if (world.hit(r, 0.001, infinity, rec)) {
        ray scattered;
        color attenuation;
        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
            // Ambient lighting
            color ambient = color(0.1, 0.1, 0.1);
            return attenuation * (ambient + ray_color(scattered, world, depth - 1));
        }
        return color(0, 0, 0);
    }

    // Background: Black
    return color(0, 0, 0);
}


int main() {
    // Image

    const auto aspect_ratio = 3.0 / 2.0;
    const int image_width = 1200;
    const int image_height = static_cast<int>(image_width / aspect_ratio);
    const int samples_per_pixel = 20;
    const int max_depth = 50;

    // World
    auto world = random_scene();

    // Add grass-like spheres as stars
    int num_grass_spheres = 500;
    for (int i = 0; i < num_grass_spheres; i++) {
        auto albedo = color(1.0, 1.0, 0.0); // Bright yellow color for stars
        auto fuzz = random_double(0, 0.2);
        auto sphere_material = make_shared<lambertian>(albedo);

        double radius = 0.05;
        double x = random_double(-15, 15);
        double z = random_double(-15, 15);
        double y = radius + random_double(10, 50); // Raise stars higher in the sky
        world.add(make_shared<sphere>(point3(x, y, z), radius, sphere_material));
    }

    // Camera
    point3 lookfrom(13, 2, 3);
    point3 lookat(0, 0, 0);
    vec3 vup(0, 1, 0);
    auto dist_to_focus = 10.0;
    auto aperture = 0.1;

    camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus);
    // Render

    std::cout << "P3\n" << image_width << " " << image_height << "\n255\n";

    for (int j = image_height - 1; j >= 0; --j) {
        std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < image_width; ++i) {
            color pixel_color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel; ++s) {
                auto u = (i + random_double()) / (image_width - 1);
                auto v = (j + random_double()) / (image_height - 1);
                ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, world, max_depth);
            }
            write_color(std::cout, pixel_color, samples_per_pixel);
        }
    }

    std::cerr << "\nDone.\n";
}