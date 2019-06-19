# Operation

Operations are functions that are executed for each simulation object.
To add execute a function for specific simulation objects have a look at
biology modules.

To support multi-scale simulations, operations have a data member frequency.
If it is set to one it means that this function will be executed for every
time step. If this member is set to two it will be executed every second time
step, and so on.

Here the link for the complete API documentation for [Operation](TODO insert link)

## Usage examples

### Add a user-defined operation

Below you can find a code example to add an operation that prints the position
of all simulation objects to stdout.

``` C++
... C++
auto* scheduler = simulation.GetScheduler();
Operation print_position_op("print position op", [](SimObject* so){
    const auto& p = so->GetPosition();
    std::cout << "SimObject " << so->GetUid() << ": "
              << p[0] << ", " << p[1] << ", " << p[2] << std::endl;
   });
scheduler.AddOperation(print_position_op);
...
```

### Change the execution frequency of an operation

Let's assume that we want to output all position every 100 timesteps instead of
every.

``` C++
...
auto* scheduler = simulation.GetScheduler();
scheduler.GetOperation("print position op")->frequency_ = 100;
...
```
