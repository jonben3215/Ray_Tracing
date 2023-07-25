#ifndef MATERIAL_H
#define MATERIAL_H

#include "rtweekend.h"
#include "hittable_list.h"

struct hit_record;

class material
{
    public:
       virtual bool scatter(
            const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
        ) const = 0;
};

//For light scattering  and reflectance
class lambertian : public material {
    public:
        lambertian(const color& a) : albedo(a) {}

        virtual bool scatter(
            const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
        ) const override {
            auto scatter_direction = rec.normal + random_unit_vector();

            // Catch degenerate scatter direction
            if (scatter_direction.near_zero())
                scatter_direction = rec.normal;

            scattered = ray(rec.p, scatter_direction);
            attenuation = albedo;
            return true;
        }
    public:
        color albedo;
};

class swirl : public material {
public:
    swirl(const color& a, double frequency) : albedo(a), freq(frequency) {}

    virtual bool scatter(
        const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
    ) const override {
        // Swirl effect - Modify the position and normal of the hit point
        vec3 p = freq * rec.p;
        double scale = sin(p.x()) * sin(p.y()) * sin(p.z());
        vec3 new_position = vec3(rec.p.x() * scale, rec.p.y() * scale, rec.p.z() * scale);
        vec3 new_normal = unit_vector(new_position - rec.p);

        scattered = ray(new_position, new_normal); // Use the modified position and normal
        attenuation = albedo;
        return true;
    }

public:
    color albedo;
    double freq;
};
//Formula for reflecting rays: See 9.4: Mirroed Light Reflection
class metal : public material {
    public:
        metal(const color& a) : albedo(a) {}
        metal(const color& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}//For reflecting on fuzzy images

        virtual bool scatter(
            const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
        ) const override {
            vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
            scattered = ray(rec.p, reflected);
            attenuation = albedo;
            return (dot(scattered.direction(), rec.normal) > 0);
        }

    public:
        color albedo;
        double fuzz;
};

//Class to help defract
class dielectric : public material {
    public:
        dielectric(double index_of_refraction) : ir(index_of_refraction) {}

        virtual bool scatter(
            const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
        ) const override {
            attenuation = color(1.0, 1.0, 1.0);
            double refraction_ratio = rec.front_face ? (1.0/ir) : ir;

            vec3 unit_direction = unit_vector(r_in.direction());
            double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
            double sin_theta = sqrt(1.0 - cos_theta*cos_theta);

            bool cannot_refract = refraction_ratio * sin_theta > 1.0;
            vec3 direction;

           if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double())
                direction = reflect(unit_direction, rec.normal);
            else
                direction = refract(unit_direction, rec.normal, refraction_ratio);

            scattered = ray(rec.p, direction);
            return true;
        }

    public:
        double ir; // Index of Refraction
    
    private:
        static double reflectance(double cosine, double ref_idx) {
            // Use Schlick's approximation for reflectance.
            auto r0 = (1-ref_idx) / (1+ref_idx);
            r0 = r0*r0;
            return r0 + (1-r0)*pow((1 - cosine),5);
        }
};
#endif