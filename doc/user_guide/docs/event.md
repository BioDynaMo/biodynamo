---
title: "Event"
date: "2019-01-01"
path: "/docs/userguide/event/"
meta_title: "eve"
meta_description: "This is the event page."
toc: true
next:
    url:  "/docs/userguide/event/"
    title: "Event"
    description: "This is the event page."
sidebar: "userguide"
keywords:
  -event
---

If a new simulation object (e.g. Cell) is created during a simulation we denote
it as an Event.

An example is cell division: A mother cell splits into two daughter cells. By
our definition the already existing mother cell is modified to become the first
daughter cell. The second daughter cell must be created.

Further examples are:

  * Neurite elongation
  * Neurite branching
  * Neurite bifurcation
  * ...

## Stages
An event has two stages:

  1. Create new simulation object(s)
  2. Modify the simulation object which triggered the event (optional).

## Cell Division Example

Let‘s take the cell division event as an example and walk through these steps.

### Event trigger

A call to `cell.Divide()` triggers the event.

### Step 1: Create daughter 2

A new cell will be created using the following constructor
`Cell(const CellDivisionEvent& event, TMother* mother)`.

The class `CellDivisionEvent` contains the parameters to perform a cell division.
The second parameter is a pointer to the event trigger `cell` (aka mother) and
can be used to access its data members. (e.g. `volume_ = mother->GetVolume() * event.volume_ratio_`)

### Step 2: Modify event trigger

Currently the event trigger `cell` is still in the state before the division (= mother).
Now, we have to modify it to transform it into daughter 1.
For instance the original volume must be adjusted such that (`volume_mother = volume_daughter_1 + volume_daughter_2`).
This is performed in the `EventHandler` function. For cell division it has
the following signature:
`EventHandler(const CellDivisionEvent& event, TDaughter* daughter)`

<!-- TODO events and biology modules -->

## Extending Simulation Objects

This architecture is important to support extension of simulation objects.
Let's assume that you extend the Cell class to add a new data member
`my_new_data_member_`.

```cpp
class MyCell : public Cell {
  ...
  double my_new_data_member_ = {3.14};
  ...
}
```

Now you have to tell BioDynaMo what the value of `new_data_member_` should be
for daughter 1 and daughter 2 in case your cell divides. You can do that by
defining a constructor and event handler. Let's assume that `new_data_member_`
of the mother cell is divided between the daughters according to the volume
ratio defined in `CellDivisionEvent`.

```cpp
class MyCell : public Cell {
 public:
  MyCell(const Event& event, SimObject* other, uint64_t new_oid = 0)
       : Base(event, other, new_oid) {
    new_data_member_ = mother->new_data_member_ * event.volume_ratio;
  }

  void EventHandler((const Event& event, SimObject* other_1,
                    SimObject* other_2 = nullptr) override {
    if (auto* daughter_2 = dynamic_cast<MyCell*>(other_2)) {
      new_data_member_ -= daughter_2->new_data_member_;
    }
    Base::EventHandler(event, daughter_2);
  }
  ...
 private:  
  double my_new_data_member_ = {3.14};
  ...
};
```

The constructor initializes `new_data_member_` for daughter 2.
The event handler performs the transition from mother to daughter 1.

<a class="sbox" target="_blank" rel="noopener">
    <div class="sbox-content">
      <h4><b>CAUTION<b><h4>
      <p>Do not forget to forward the call to the constructor and event handler of the base class.
    </p>
    </div>
</a>

<!-- TODO explain default event handler and ctors -->

## Additional Notes

  * It is possible to create a default constructor and event handler that is
    called for every event. This is useful for example if you extend a simulation
    object, but do not add additional data members.
  * Events can create more than one simulation object. e.g. NeuriteBranchingEvent
  * The type of the simulation object that triggers the event and newly created
    objects can be different. e.g. NewNeuriteExtensionEvent
