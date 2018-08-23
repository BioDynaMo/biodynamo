
# Сancer growth

##### The idea is to visualise how from the only one mutated cell tumor occurs. 
>
>
This model creates single cancerous cell. This cell divides recursively. Cells that have many cells in their neighbourhood, divide slower, because of lack of nutrients. When the cell has certain numbers of neighborhood cells, it stops growing -- its growth_rate becomes 0 -- and changes its own cell_color.  

 ### Copy the demo code
To open a simulation write the following command `biodynamo demo cancer_growth`.
 ### Inspect the code
  Go into the `cancer_growth` directory and open the source file `src/cancer_growth.h` . Let`s go through the code:
 
### Creating cells
For our simulation we will extend general `Cell` by adding some extra features to `MyCell`
``` C++
BDM_SIM_OBJECT(MyCell, Cell) {  // our object extends the Cell object
                                // create the header with our new data member
  BDM_SIM_OBJECT_HEADER(MyCellExt, 1, cell_color_);
  public:
  MyCellExt() {}
  explicit MyCellExt(const std::array<double, 3>& position) : Base(position) {}
   // getter and setter for our new data member
    void SetCellColor(int cell_color) { cell_color_[kIdx] = cell_color; }
  int GetCellColor() const { return cell_color_[kIdx]; }
  private:
  // declare new data member and define their type
  // private data can only be accessed by public function and not directly
  vec<int> cell_color_;
};
```
Next, we will define the behavior of growth. Here we will create a cancerous cell that will grow and divide when it reaches a certain diameter. For this, we will define a new biology module structure GrowthModule that will be applied to cell elements, and we will make this GrowthModule copied into the cell daughter (so the daughter will also contain an instance of the biology module GrowthModule)
``` C++
struct GrowthModule : public BaseBiologyModule {
  GrowthModule() : BaseBiologyModule(gAllBmEvents) {}
  GrowthModule(double threshold, double growth_rate, double neighborhood_radius, 
						int maximum_of_neighbors) : 
				BaseBiologyModule(gAllBmEvents), 
				threshold_(threshold), 
				growth_rate_(growth_rate),
				neighborhood_radius_(neighborhood_radius),
				maximum_of_neighbors_(maximum_of_neighbors) {}

```
### Executing simulation
The template stimulation allows the code to be executed at each stimulation step
``` C++
template <typename T, typename TSimulation = Simulation<>>
  void Run(T* cell) {
    auto* grid = TSimulation::GetActive()->GetGrid();
```
Monitoring the number of neighbors of the cell in order to set the rate of dividing, because those cells that have more neighbors divide slower
``` C++
std::atomic<int> neighbors{0};
auto increment_neighbors = [&](auto&& lhs, SoHandle lhs_id){
	neighbors++;
};
grid->ForEachNeighborWithinRadius(increment_neighbors, *cell, cell->GetSoHandle(), (threshold_) * (threshold_));
```
Also we need to determine the volume of the cell, if it can grow further and the color of the cell says how hard the division and growth of this cell would be.  Shade of blue — there are not so many neighbors right now, the cell grows and divides quickly.
``` C++
	// if cell is bigger than or equal to growth threshold, then divide

	if (cell->GetDiameter() >= threshold_) {
		cell->Divide();
	} else {  
		// if cell is less than growth threshold
		if (neighbors != 0) {
			if (neighbors < maximum_of_neighbors_) {
				// volume of cell's growth is inversely proportional
				cell->ChangeVolume(growth_rate_ / neighbors);
			} else {
				// if there is more than maximum neigbors, cell would stop growing and repaint (in red)

				cell->ChangeVolume(0);
				cell->SetCellColor(cell->GetCellColor() + neighbors);

			}
		}
	}

}
```
We can change default values of variables as we want
``` C++
private:

double threshold_ = 30;
double growth_rate_ = 1500;
double neighborhood_radius_ = 30;
int maximum_of_neighbors_ = 5;
```
After creating our GrowthModule, we need to add this Biology module to the compile time parameter, to tell BioDynaMo to use this new BiologyModule
``` C++
template <typename Backend>
struct CompileTimeParam : public DefaultCompileTimeParam<Backend> {
	using BiologyModules = Variant<GrowthModule>; // add GrowthModule
	using AtomicTypes = VariadicTypedef<MyCell>; // use MyCell object
};
```
 Simulate is the main function of our code that is being executed.
 ``` C++
 inline int Simulate(int argc, const char** argv) {
	Simulation<> simulation(argc, argv);
```
 Define initial model, which is consisting of one cell at the beginning.
 ``` C++
 size_t cells_per_dim = 1;
auto construct = [](const std::array<double, 3>& position) {
	MyCell cell(position);
	cell.SetDiameter(30);
	cell.SetAdherence(0.4);
	cell.SetMass(1.0);
	cell.SetCellColor(8);
	// add GrowthModule
	cell.AddBiologyModule(GrowthModule()); // here by adding argument values inside GrowthModule(_) user can set variable of simulation

	return cell;

};
ModelInitializer::Grid3D(cells_per_dim, 1, construct);
```
Run simulation for N times
``` C++
	simulation.GetScheduler()->Simulate(1000); // time parameter in brackets

	return 0;
	}

} // namespace bdm

#endif // CELLS_MOVING_H_
```

### Configure the simulation
Create a `bdm.toml` file in the `cancer_growth` directory, and copy the following lines into it:
```C++
[visualization]
export = true
export_interval = 10

	[[visualize_sim_object]]
	name = "MyCell"
	additional_data_members = [ "cell_color_" ]
```

### Build and run the simulation
Run the following commands to build and run the simulation.
``` C++
biodynamo run
```