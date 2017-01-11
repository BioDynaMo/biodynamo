//
//  octree.hpp
//  TreeComp
//
//  Created by Victor Drobny on 18.10.16.
//  Copyright © 2016 Victor Drobny. All rights reserved.
//

#ifndef octree_hpp
#define octree_hpp

#include <vector>
#include <cstdio>

#include "spatial_tree.hpp"

using std::vector;
using std::make_pair;

template <typename T>
class octree_node: public spatial_tree_node<T>
{
private:
    bool is_leaf_node;
    octree_node<T> * children[8];
    vector<pair<point, T> > * objects;
    int max_depth;
    int max_amount_of_objects;
    
    void split();
    int get_child_id(point p);
    
    virtual spatial_tree_node<T> ** get_children();
    virtual vector<pair<point, T> > * get_objects();
    virtual int get_children_size();
    
    
public:
    octree_node();
    octree_node(bound bnd, int max_depth, int max_amount_of_objects);
    octree_node(int max_depth, int max_amount_of_objects);
    ~octree_node();
    virtual bool is_leaf();
    virtual void put(point p, T obj);
    T at(point p);
    int size();
};


template <typename T>
octree_node<T>::octree_node()
{
    octree_node<T>(bound(0, 0, 0, 1, 1, 1), 10, 1000);
}

template <typename T>
octree_node<T>::octree_node(int max_depth, int max_amount_of_objects)
{
    this->bnd = bound(-inf, -inf, -inf, inf, inf, inf);
    this->is_leaf_node = true;
    this->max_depth = max_depth;
    this->max_amount_of_objects = max_amount_of_objects;
    for (int i = 0; i < 8; i++)
        children[i] = nullptr;
    objects = new vector<pair<point, T> >();
    is_leaf_node = true;
}

template <typename T>
octree_node<T>::octree_node(bound bnd, int max_depth, int max_amount_of_objects)
{
    this->bnd = bnd;
    this->is_leaf_node = true;
    this->max_depth = max_depth;
    this->max_amount_of_objects = max_amount_of_objects;
    for (int i = 0; i < 8; i++)
        children[i] = nullptr;
    objects = new vector<pair<point, T> >();
    is_leaf_node = true;
}


template <typename T>
octree_node<T>::~octree_node()
{
    if (!is_leaf())
    {
        for (int i = 0; i < 8; i++)
            if (children[i] != nullptr)
            {
                delete children[i];
                children[i] = nullptr;
            }
    }
    if (objects != nullptr)
    {
        delete objects;
        objects = nullptr;
    }
}

template <typename T>
bool octree_node<T>::is_leaf()
{
    return is_leaf_node;
}


template <typename T>
void octree_node<T>::put(point p, T obj)
{
    if (is_leaf_node)
    {
        // Insert
        if (objects->size() < max_amount_of_objects || (max_depth == 0))
        {
            objects->push_back(make_pair(p, obj));
        } else {
            // Split
            split();
            put(p, obj);
        }
    } else {
        int idx = get_child_id(p);
        children[idx]->put(p, obj);
    }

}


template <typename T>
void octree_node<T>::split()
{
    if (!this->is_leaf_node)
        return;
    point center = this->bnd.center();
    bound bnd = this->bnd;
    point p[8] = {
        point(center.x, center.y, center.z),
        point(center.x, bnd.left(), center.z),
        point(center.x, bnd.left(), bnd.bottom()),
        point(center.x, center.y, bnd.bottom()),
        point(bnd.far(), center.y, center.z),
        point(bnd.far(), bnd.left(), center.z),
        point(bnd.far(), bnd.left(), bnd.bottom()),
        point(bnd.far(), center.y, bnd.bottom())
    };
    point e[8] = {
        point(bnd.near(), bnd.right(), bnd.top()),
        point(bnd.near(), center.y, bnd.top()),
        point(bnd.near(), center.y, center.z),
        point(bnd.near(), bnd.right(), center.z),
        point(center.x, bnd.right(), bnd.top()),
        point(center.x, center.y, bnd.top()),
        point(center.x, center.y, center.z),
        point(center.x, bnd.right(), center.z)
    };
    
    for (int i = 0; i < 8; i++)
    {
        children[i] = new octree_node<T>(bound(p[i], e[i]), max_depth - 1, max_amount_of_objects);
    }
    
    for (int i = 0; i < objects->size(); i++)
    {
        int idx = get_child_id(objects->at(i).first);
        children[idx]->put(objects->at(i).first, objects->at(i).second);
    }
    delete objects;
    objects = nullptr;
    this->is_leaf_node = false;
}


template <typename T>
int octree_node<T>::get_child_id(point p)
{
    point center = this->bnd.center();
    int result = 0;
    if (p.y >= center.y)
    {
        if (p.z >= center.z)
        {
            result = 0;
        } else {
            result = 3;
        }
    } else {
        if (p.z >= center.z)
        {
            result = 1;
        } else {
            result = 2;
        }
    }
    if (p.x < center.x)
    {
        result += 4;
    }
    return result;
}


template <typename T>
T octree_node<T>::at(point p)
{
    if (this->is_leaf_node)
    {
        for (int i = 0; i < objects->size(); i++)
            if (p.equals(objects->at(i).first))
                return objects->at(i).second;
    }
    else
    {
        int idx = get_child_id(p);
        return children[idx]->at(p);
    }
    return NULL;
}

template <typename T>
int octree_node<T>::size()
{
    if (this->is_leaf_node)
    {
        return (int) objects->size();
    }
    else
    {
        int sum = 0;
        for (int i = 0; i < 8; i++)
        {
            sum += children[i]->size();
        }
        return sum;
    }
    return -1;
}


template <typename T>
spatial_tree_node<T> ** octree_node<T>::get_children()
{
    return (spatial_tree_node<T> **) children;
}


template <typename T>
vector<pair<point, T> > * octree_node<T>::get_objects()
{
    return this->objects;
}


template <typename T>
int octree_node<T>::get_children_size()
{
    if (!is_leaf())
        return 8;
    return 0;
}

#endif /* octree_hpp */
