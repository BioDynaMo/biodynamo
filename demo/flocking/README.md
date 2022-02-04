
## Flocking Algorithm
This [Flocking](https://en.wikipedia.org/wiki/Flocking_(behavior)) algorithm builds on the framework established by R. Olfati-Saber in his paper ["Flocking for multi-agent dynamic systems: algorithms and theory"](https://ieeexplore.ieee.org/document/1605401?arnumber=1605401).  
Boids try to match their velocity with neighboring boids and keep a desired distance to them. A common group objective can be set towards which the boid should migrate.   
At each computational step an internal acceleration for each boid is computed and it's velocity and position updated accordingly. The acceleration consists of two terms that get added in a priority based scheme: the flocking force and a steering force towards the group objective. The flocking force is the (weighted) sum of three input terms:  
 - a term that tries to match the boid's velocity with neighboring boids  
 - a term that tries to keep a desired distance between a boid and its neighbors  
 - a term that improves the overall flock cohesion 

## Simulation
At the start of the simulation, all boids spawn uniformly at random inside a starting sphere centered at the domain's origin. All boids have no initial velocity and a random heading direction. By default the domain is an open torus.  
To visualize the simulation, import the output in paraview and set the boids' scale array to ```actual_diameter```.

## Simulation Specific Parameters
```n_boids:```  
 number of boids that are spawned in the simulation  
 
```starting_sphere_radius:```   
a boid's starting location is chosen uniformly at random inside a sphere with this radius  

```boid_perception_radius:```   
determining radius for the extended cohesion term  

```boid_interaction_radius:```  
boids try to match their velocity and keep a desired distance to boids within this radius

```perception_angle_deg:```     
a boid's field of view in degree (max. of 360°)

```neighbor_distance:```        
a boid's desired distance to neighboring boids  

```max_accel:```                
maximal value for the total internal acceleration of a boid

```max_speed:```                
maximal value for the speed of a boid  

```c_a_1:```                     
weight for the term that controls the velocity matching with neighboring boids  

```c_a_2:```                    
weight for the term that controls the desired distance to neighboring boids  

```c_a_3:```                   
weight for the extended cohesion term  

```c_y:```                    
weight for the navigational feedback term  

```d_t:```  
time between two computational steps. A smaller d_t corresponds to a higher update frequency  

```pos_gamma:```                  
location of the common group objective  

