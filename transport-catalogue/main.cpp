#include "request_handler.h"    
#include "json_reader.h"        

#include <iostream>         
#include <sstream>


int main()
{
    transport::TransportCatalogue tc;
    transport::RequestHandler rh(tc);
    transport::reader::JSONreader jr;
    jr.ProcessJSON(tc, rh, std::cin, std::cout);
}