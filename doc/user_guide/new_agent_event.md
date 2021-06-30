---
title: "NewAgentEvent"
date: "2019-01-01"
path: "/docs/userguide/new_agent_event/"
meta_title: "BioDynaMo User Guide"
meta_description: "This page explains NewAgentEvents."
toc: true
next:
    url:  "/docs/userguide/new-agent-event/"
    title: "Event"
    description: "This is the event page."
sidebar: "userguide"
keywords:
  -event
---

If a new agent (e.g. Cell) is created during a simulation we denote
it as a NewAgentEvent.

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

  1. Create new agent(s)
  2. Modify the agent which triggered the event; the existing agent.

## Cell Division Example

Let‘s take the cell division event as an example and walk through these steps.

### Event trigger

A call to `cell.Divide()` triggers the event.

### Step 1: Create daughter 2

A new cell will be created using the default constructor of `Cell`.
Afterwards, the `Initialize(const NewAgentEvent& event)` method is called for the new cell.

The class `CellDivisionEvent` contains the parameters to perform a cell division.
The base class `NewAgentEvent` contains pointers to the existing agent and to 
all new created agent. 
This information is sufficient to initialize all attributes of the new daughter cell.
```cpp
...
if (event.GetUid() == CellDivisionEvent::kUid) {
  auto* mother = bdm_static_cast<Cell*>(event.existing_agent);
  const auto& cde = static_cast<const CellDivisionEvent&>(event);
  volume_ = mother->GetVolume() * cde.volume_ratio`)
}
```

### Step 2: Modify existing agent

Currently the existing `cell` is still in the state before the division (= mother).
Now, we have to modify it to transform it into daughter 1.
For instance the original volume must be adjusted such that (`volume_mother = volume_daughter_1 + volume_daughter_2`).
This is performed in the `Update(const NewAgentEvent& event)` function.
It could look like the following:
```cpp
...
if (event.GetUid() == CellDivisionEvent::kUid) {
  auto* daughter_2 = static_cast<Cell*>(event.new_agents[0]);
  volume_ -= daughter_2->GetVolume();
}
```


<!-- TODO events and behaviors -->

## Extending Agents

This architecture is important to support extension of agents.
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
overriding the functions `void Initialize(const NewAgentEvent& event)` and 
`void Update(const NewAgentEvent& event)`. 
Let's assume that `new_data_member_` of the mother cell is divided between 
the daughters according to the volume ratio defined in `CellDivisionEvent`.

The function `Initialize` initializes `new_data_member_` for daughter 2.
The function `Update` performs the transition from mother to daughter 1.

```cpp
class MyCell : public Cell {
 public:  
  MyCell() {}
  
  void Initialize(const NewAgentEvent& event) override {
    Base::Initialize(event);

    if (event.GetUid() == CellDivisionEvent::kUid) {
      auto* mother = bdm_static_cast<Cell*>(event.existing_agent);
      const auto& cde = static_cast<const CellDivisionEvent&>(event);
      new_data_member_ -= mother->new_data_member_ * cde.volume_ratio;
    }
  }

  void Update(const NewAgentEvent& event) override {
    Base::Update(event);

    if (event.GetUid() == CellDivisionEvent::kUid) {
      auto* daughter_2 = bdm_static_cast<Cell*>(event.new_agents[0]);
      new_data_member_ -= daughter_2->new_data_member_;
    }
  }
  ...
 private:
  double my_new_data_member_ = {3.14};
  ...
};
```

<a class="sbox" target="_blank" rel="noopener">
    <div class="sbox-content">
      <h4><b>CAUTION</b></h4>
      <p>Do not forget to forward the call of Initialize and Update to the base class.
     Failing to do so will cause errors.
    </p>
    </div>
</a>

You can ommit the implementaiont of `Initialize` and/or `Update` if your
new data member, doesn't need this type of initialization.
<!-- TODO explain default event handler and ctors -->

## Additional Notes

  * NewAgentEvents can create more than one agent. e.g. NeuriteBranchingEvent
  * The type of the existing agent that triggers the event and the newly created
    agent can be different. e.g. NewNeuriteExtensionEvent
