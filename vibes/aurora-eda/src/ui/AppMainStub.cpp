#include <iostream>

int main() {
  std::cerr << "aurora-eda was built without Qt 6 Widgets support.\n"
            << "Install Qt 6 and rebuild with AURORA_BUILD_UI=ON to run the GUI.\n";
  return 1;
}
