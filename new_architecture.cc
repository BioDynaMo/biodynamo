
template <template <typename> class TDerived, typename TML = Scalar>
class CellDef : public SimulationObject<CellDef, TML> {

template <typename TML = Scalar>
class CellDef : public SimulationObject<TML> {}

// FIXME TDerived still needed?
BDM_SIM_OBJECT(Cell, SimulationObject) {
  // ForEachNeighbor(In) needs to call it for every mixin_;
  BDM_HEADER(CellDef);

  // friend DivideStrategy;
  // DivideStrategy* divide_strategy_; // stateless
  // what if CellDef is subclassed? how should DivideStrategy work?

  // friend each mixin in CellMixins?
  using Mixins = TCompileTimeParam::CellMixins;
  FixedSizeVector<Mixins, 2> mixins_; // mixins are created in ctor

  template<typename T>
  typename T::Self<SoaRef>& GetMixin() {
    uint64_t idx = ...;
    return (*std::get<T>(mixins_[idx]))[kIdx];
  }

  void DivideImpl( <parameter> ) {
    // copy all data members
    // position_, diamter_

    // mixins and
    DivideHandler handler;
    for (uint64_t i = 0; i < mixins_.size()) {
      handler.handle(mixins_[i][kIdx], daughter.mixins_[i][0], <parameter>);

      mixins_[i].handle<kCellDivision>(daughter.mixins_, parameter);
    }
    // biology modules
    // ...

    // forward to base class
    Base::DivideImpl(<parameter>);
  }
};

// -----------------------------------------------------------------------------
// CTParam needs to be template parameter since it is incomplete type
template <typename TCTParam = CompileTimeParam<>>
struct ColorMixin : public CopyOnAllEvents {  // StatelessMixin sizeof check
  BDM_HEADER(ColorMixin, 1, color_);

  vec<int> color_ = {{ 0 }};
  // FIXME kIdx ??
  int GetColor() { return color_[kIdx]; }
  void SetColor(int color) { color_[kIdx] = color; }

  template <typename T>
  void Run(T* sim_object) {}  // this would be relevant for GeneMixin

  // void handle(BmEvent event, ColorMixin*<> daughter, <parameters>) {
  //   daughter.SetColor(color_[kIdx]);
  // }

  template <BmEvent Event>
  void handle(ColorMixin*<> daughter, <parameters>) {

  }

  // how to get pointer of enclosing simulation object?
  // is this even required?
};

template <>
ColorMixin<>::handle<kCellDivision>(...) {

}

template <>
ColorMixin<>::handle<whatever>(...) {

}



template ...
struct MyColorMixin : public ColorMixin<..> {
  void handle(...) {

  }
}

// new event type -> new Visitor
//   not required to add new method
//
struct DivideHandler {
  // one function for each mixin / biology module
  void handle(ColorMixin<>* lhs, ColorMixin<>* rhs, double volume_ratio) {

  }

  // ... all other mixins, biology modules

  // default event handler doing nothing
  template <typename T, typename... Args>
  void handle(T t, Args... args) {
    // if sizeof T is <= 1 than it most likely does not contain state.
    if (sizeof(T) > 1) {
      // error missing divide event handler for type T
    }
  }
};

struct BranchingHandler {
  void handle(ColorMixin<>* lhs, ColorMixin<>* rhs, double volume_ratio) {}
};

struct TCompileTimeParam {
  using BiologyModules = VariantTypedef<GeneBM, Bar>;
  // use pointers to eliminate variant memory overhead
  using CellMixins = VariantTypedef< ColorMixin*, CustomDivideMixin*>;
  using NeuriteMixins = VariantTypedef<ColorMixin*>;
};

int Simulate() {
  Cell cell;

  // there should be "no" overhead using std::get, but let's check
  cell.GetMixin<ColorMixin>().SetColor(4);
  cell.GetBM<GeneBM>().GetSubstances()[0];

  // redefine predefined stuff
  // cell.SetDivideStrategy(CustomDivideStrategy);
  // cell.GetDivideStrategy().Divide();

  // -> mixin needs reference to neurite SO
  // access to private data members ??
}


template <typename TML = Scalar>
class NeuronSomaDef : public CellDef<TML> {};
