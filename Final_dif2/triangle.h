#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "hittable.h"

class triangle : public hittable {
public:
    triangle() {}
    triangle(const point3& p0, const point3& p1, const point3& p2, const std::shared_ptr<material>& mat)
        : p0_(p0), p1_(p1), p2_(p2), mat_ptr_(mat) {}

    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;

public:
    point3 p0_, p1_, p2_;
    std::shared_ptr<material> mat_ptr_;
};

bool triangle::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    // Implementation of hit function for triangle
    // Add your code here for intersection calculation

    return false; // Replace this with actual intersection logic
}

#endif