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
    //std::ifstream in ("input.txt"s);
    //std::ofstream out("output.svg"s);

    transport::TransportCatalogue tc;
    transport::renderer::MapRenderer mr;
    transport::RequestHandler rh(tc, mr);
    transport::reader::JsonReader jr;
    jr.ProcessJSON(tc, rh, mr, std::cin, std::cout);
    //rh.RenderMap().Render(out);
    //rh.RenderMap().Render(std::cout);
}