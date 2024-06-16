#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main() {
    transport::TransportCatalogue catalogue;

    transport::reader::FillCatalogue(std::cin, catalogue);

    transport::reader::ShowCatalogue(std::cin, std::cout, catalogue);
}