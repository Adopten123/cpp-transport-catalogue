#include "request_handler.h"    
#include "json_reader.h"        

#include <iostream>     
#include <fstream>
#include <sstream>
#include <string>


using namespace std::literals;

int main()
{
    transport::TransportCatalogue tc;
    transport::RequestHandler rh(tc);
    transport::reader::JSONreader jr;

    std::ifstream in ("input.txt"s);
    std::ofstream out("output.txt"s);

    jr.ProcessJSON(tc, rh, in, out);
}