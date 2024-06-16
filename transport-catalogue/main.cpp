#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main() {
    transport::TransportCatalogue catalogue;

    transport::reader::FillCatalogue(catalogue);

    transport::reader::ShowCatalogue(catalogue);
}