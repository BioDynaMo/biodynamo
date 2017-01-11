//
//  utils.hpp
//  TreeComp
//
//  Created by Victor Drobny on 18.10.16.
//  Copyright © 2016 Victor Drobny. All rights reserved.
//

#ifndef utils_hpp
#define utils_hpp

#include <cmath>
#include <utility>


using std::pair;
using std::make_pair;

double const eps = 1e-20;
double const inf = 1e20;

class point {
private:
    
    
public:
    double x, y, z;
    
    point(): x(0), y(0), z(0) {}
    point(double x, double y, double z): x(x), y(y), z(z) {}

    
    double length()
    {
        return sqrt(x * x + y * y + z * z);
    }

    void set(double x, double y, double z){
        this->x = x;
        this->y = y;
        this->z = z;
    }
    
//    void normalize()
//    {
//        double len = length();
//        x /= len;
//        y /= len;
//        z /= len;
//    }
    
    double distance(point p)
    {
        return (p + *this * (-1)).length();
    }
    
    double sqdist(point p)
    {
        double dx = x - p.x;
        double dy = y - p.y;
        double dz = z - p.z;
        
        return dx * dx + dy * dy + dz * dz;
    }
    
    double operator* (point p)
    {
        return x * p.x + y * p.y + z * p.z;
    }
    
    point operator* (double a)
    {
        return point(x * a, y * a, z * a);
    }
    
    point operator+ (point b)
    {
        return point(x + b.x, y + b.y, z + b.z);
    }
    
    void operator=(point b)
    {
        x = b.x;
        y = b.y;
        z = b.z;
    }
    
    bool equals(point b)
    {
        return fabs(x - b.x) < eps && fabs(y - b.y) < eps && fabs(z - b.z) < eps;
    }
};

struct bound
{
    // Bounds
    // far left bottom, near right top
    point flb, nrt;
    
    bound()
    {
        flb = point(0, 0, 0);
        nrt = point(1, 1, 1);
    }
    
    bound(double x1, double y1, double z1, double x2, double y2, double z2)
    {
        flb = point(x1, y1, z1);
        nrt = point(x2, y2, z2);
    }
    
    bound(point p1, point p2) : flb(p1), nrt(p2) {}
    
    double volume()
    {
        return (near() - far()) * (right() - left()) * (top() - bottom());
    }
    
    bool has(point p)
    {
        return is_between(p.x, far(), near()) && is_between(p.y, left(), right()) && is_between(p.z, bottom(), top());
    }
    
    bound add_point(point p)
    {
        bound new_bnd(fmin(p.x, far()), fmin(p.y, left()), fmin(p.z, bottom()),
                      fmax(p.x, near()), fmax(p.y, right()), fmax(p.z, top()));
        return new_bnd;
    }
    
    bound add_bound(bound b)
    {
        return bound(fmin(far(), b.far()),
                     fmin(left(), b.left()),
                     fmin(bottom(), b.bottom()),
                     fmax(near(), b.near()),
                     fmax(right(), b.right()),
                          fmax(top(), b.top()));
    }
    
    double diff_if_add_point(point p)
    {
        return add_point(p).volume() - volume();
    }
    
    double sqdist(pair<double, double> p1, pair<double, double> p2)
    {
        double dx = p1.first - p2.first;
        double dy = p1.second - p2.second;
        return dx * dx + dy * dy;
    }
    
    bool is_between(double x, double a, double b)
    {
        double min = fmin(a, b);
        double max = fmax(a, b);
        return (x >= min && x <= max);
    }
    
    double distance_(double x, double y, double a, double b)
    {
        double minxy = fmin(x, y);
        double maxxy = fmax(x, y);
        double minab = fmin(a, b);
        double maxab = fmax(a, b);
        
        if (minxy >= maxab)
            return minxy - maxab;
        return minab - maxxy;
    }
    
    double distance(bound b)
    {
        
        bool is_overlap_x;
        bool is_overlap_y;
        bool is_overlap_z;
        
        double bx[2][2] = {{far(), near()}, {b.far(), b.near()}};
        double by[2][2] = {{left(), right()}, {b.left(), b.right()}};
        double bz[2][2] = {{bottom(), top()}, {b.bottom(), b.top()}};
        
        
        is_overlap_x = (is_between(bx[0][0], bx[1][0], bx[1][1])) || (is_between(bx[0][1], bx[1][0], bx[1][1]))
                    || (is_between(bx[1][0], bx[0][0], bx[0][1])) || (is_between(bx[1][1], bx[0][0], bx[0][1]));
        is_overlap_y = (is_between(by[0][0], by[1][0], by[1][1])) || (is_between(by[0][1], by[1][0], by[1][1]))
                    || (is_between(by[1][0], by[0][0], by[0][1])) || (is_between(by[1][1], by[0][0], by[0][1]));
        is_overlap_z = (is_between(bz[0][0], bz[1][0], bz[1][1])) || (is_between(bz[0][1], bz[1][0], bz[1][1]))
                    || (is_between(bz[1][0], bz[0][0], bz[0][1])) || (is_between(bz[1][1], bz[0][0], bz[0][1]));
        
        double dx = 0, dy = 0, dz = 0;
        
        if (!is_overlap_x)
            dx = distance_(bx[0][0], bx[0][1], bx[1][0], bx[1][1]);
        
        if (!is_overlap_y)
            dy = distance_(by[0][0], by[0][1], by[1][0], by[1][1]);
        
        if (!is_overlap_z)
            dz = distance_(bz[0][0], bz[0][1], bz[1][0], bz[1][1]);
        
        return dx * dx + dy * dy + dz * dz;
        
    }
    
    point center()
    {
        return (flb + nrt) * 0.5;
    }
    
    double near()
    {
        return nrt.x;
    }
    
    double far()
    {
        return flb.x;
    }
    
    double left()
    {
        return flb.y;
    }
    
    double right()
    {
        return nrt.y;
    }
    
    double top()
    {
        return nrt.z;
    }
    
    double bottom()
    {
        return flb.z;
    }

    double length()
    {
        return nrt.x-flb.x;
    }

    double width()
    {
        return nrt.y-flb.y;
    }

    double height()
    {
        return nrt.z-flb.z;
    }

    double half_surface_area()
    {
        return width()*length()+height()*length()+width()*height();
    }
};


#endif /* utils_hpp */
