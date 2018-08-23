# Сells moving along the gradient of a substance

##### The idea is to visualise how cells migrate in the space by sensing the gradient and move towards higher concentrations. 
>
>
This model creates random number of cells from (15 to 25) at random positions of simulation space. Cells  are sensing the extracellular gradient and moving according to it. The gradient belongs to the substance that is concentrated on the plane with equation By + Cz = 0 (passes through x-axis). Due to the fact that diffusion is occuring during the simulation (otherwise there would no be gradient) the substance spreads in the space, so its concentartion will be slightly higher near the origin (0;0;0), than at peripheral regions. That's why cells continue moving even after reachig the scope of substance.

 ### Copy the demo code
To open a simulation write the following command `biodynamo demo cells_moving`.
 ### Inspect the code
 Go into the `cells_moving` directory and open the source file `src/cells_moving.h` . Let`s go through the code:
 

 1. **List the extracellular substances**
 The extracellular substances that will be used in the simulation are listed in an `enum` data structure. According to our C++ coding style we will prepend the substance's name with the letter "k".
``` C++
 enum Substances { kSubstance };
 ```
 2. **Define displacement behavior.** 
The base biology module enables the program to execute the chemotaxis functions.
``` C++
struct Chemotaxis : public BaseBiologyModule {
	Chemotaxis() : BaseBiologyModule(gAllBmEvents) {}
``` 
3. **The template stimulation** allows the code to be executed at each stimulation step, so that the cells would follow the pattern and be able to move
``` C++
template <typename T, typename TSimulation = Simulation<>>
void Run(T* cell) {
auto* sim = TSimulation::GetActive();
auto* rm = sim->GetResourceManager();
``` 
4. **Getting Diffusion parameters**
``` C++
static auto* kDg = rm->GetDiffusionGrid(kSubstance);
kDg->SetConcentrationThreshold(1e15);
``` 
5.  **Determining the gradient of a substance and updating cell`s position**
``` C++
		auto& position = cell->GetPosition();
		std::array<double, 3> gradient;

		// Compute gradient of the substance at particular position
		kDg->GetGradient(position, &gradient);
		gradient[0] *= 0.5;
		gradient[1] *= 0.5;
		gradient[2] *= 0.5;

		cell->UpdatePosition(gradient);
	}
	ClassDefNV(Chemotaxis, 1);
};
```
6. **Enabling BioDynaMo to use the biology module by adding it to compile time parameter **
``` C++
template <typename Backend>
struct CompileTimeParam : public DefaultCompileTimeParam<Backend> {
	using BiologyModules = Variant<Chemotaxis>; // add Chemotaxis
};
```
7. **Main function**
``` C++
inline int Simulate(int argc, const char** argv) {
	Simulation<> simulation(argc, argv);
	auto* param = simulation.GetParam();
	auto* random = simulation.GetActive()->GetRandom();
```
8. **Defining initial model**
We limit the space where the simmulation will take place, 
``` C++
// Create an artificial bounds for the simulation space
param->bound_space_ = true;
param->min_bound_ = -100;
param->max_bound_ = 100;

// random number of cells used in simulation
size_t nb_of_cells = random->Uniform(15, 25);

// create nb_of_cells cells at random positions
auto construct = [](const std::array<double, 3>& position) {
	Cell cell(position);
	cell.SetDiameter(10);
	cell.SetAdherence(0.4);
	cell.SetMass(1.0);
	cell.AddBiologyModule(Chemotaxis());

	return cell;

};
```
9.  **Initialising the model**
``` C++
// creating cells randomly
ModelInitializer::CreateCellsRandom(param->min_bound_, param->max_bound_, nb_of_cells, construct); 

// defining the substance towards which cells will be moving

ModelInitializer::DefineSubstance(kSubstance, "Substance", 0.1, 0, 20);

ModelInitializer::InitializeSubstance(kSubstance, "Substance", GaussianBand(0, 5, Axis::kXAxis));
```
10.  **Run simulation for N times** 
``` C++
	simulation.GetScheduler()->Simulate(1000); // time parameter in brackets

	return 0;
	}

} // namespace bdm

#endif // CELLS_MOVING_H_
```

### Configure the simulation
Create a `bdm.toml` file in the `cells_moving` directory, and copy the following lines into it:
```C++
[visualization]
export = true
export_interval = 10

	[[visualize_sim_object]]
	name = "Cell"
	additional_data_members = [ "diameter_" ]
	
	[[visualize_diffusion]]
	name = "Substance"
	gradient = false
```

### Build and run the simulation
Run the following commands to build and run the simulation.
``` C++
biodynamo run
```
