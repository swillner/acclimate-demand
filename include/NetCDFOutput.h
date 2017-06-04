/*
  Copyright (C) 2013-2014 Sven Willner <sven.willner@pik-potsdam.de>

  This file is part of Acclimate (basic and demand version).

  Acclimate is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of
  the License, or (at your option) any later version.

  Acclimate is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with Acclimate.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _NETCDFOUTPUT_H
#define _NETCDFOUTPUT_H

#include "Output.h"
#include "XMLInput.h"
#include <netcdf>
#include <string>
#include <unordered_map>
#include <vector>

class Model;
class Scenario;

using namespace netCDF;

class NetCDFOutput : public Output {
  private:
    struct Target {
        std::string name;
        int region;
        int sector;
    };
    struct Variable {
        NcVar nc_var;
        unsigned char dim;
        double* data;
    };
    std::string filename;
    std::unordered_map<std::string, Variable> variables;
    std::unordered_map<Region*, int> region_index;
    std::unordered_map<Sector*, int> sector_index;
    std::vector<Target> stack;
    NcFile* file;
    NcDim dim_time;
    NcDim dim_region;
    NcDim dim_sector;
    NcDim dim_string;
    size_t timestep;
    bool flush;

  protected:
    void internal_write_header(tm* timestamp, int max_threads, const char* version);
    void internal_write_footer(tm* duration);
    void internal_write_xml_input(XMLInput* input);
    void internal_start();
    void internal_iterate_end();
    void internal_end();
    void internal_write_double(const char* name, double v);
    void internal_start_target(const char* name, Sector* sector, Region* region);
    void internal_start_target(const char* name, Sector* sector);
    void internal_start_target(const char* name, Region* region);
    void internal_start_target(const char* name);
    void internal_end_target();

  public:
    NetCDFOutput(XMLInput* settings, Model* model, Scenario* scenario, pugi::xml_node output_node);
    int initialize();
};

#endif
