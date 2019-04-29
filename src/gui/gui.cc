#include <gui/gui.h>
#include <iostream>

int main() {
    std::cout << "In gui.cc main()" << '\n';
    bdm::Gui & g = bdm::Gui::GetInstance(); 
    g.Init();
    std::cout << "Exiting gui.cc main()" << '\n';
    return 0;
}