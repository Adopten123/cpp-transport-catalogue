#include "json_reader.h"   
#include "map_renderer.h"
#include "request_handler.h"    

#include <fstream>
#include <iostream>     
#include <sstream>
#include <string>


using namespace std::literals;

int main()
{
    std::ifstream in ("s10_final_opentest_1.txt"s);
    std::ofstream out("output_s10_final_opentest_1.txt"s);

    transport::TransportCatalogue tc;
    transport::renderer::MapRenderer mr;
    transport::reader::JsonReader jr;


    jr.ProcessJSON(tc, mr, std::cin, std::cout);
    //rh.RenderMap().Render(out);
    //rh.RenderMap().Render(std::cout);
}