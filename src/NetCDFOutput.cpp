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

#include "NetCDFOutput.h"

#include <string.h>
#include <sstream>
#include "Model.h"
#include "Region.h"
#include "Scenario.h"
#include "Sector.h"

using namespace netCDF;

NetCDFOutput::NetCDFOutput(XMLInput* settings, Model* model, Scenario* scenario, pugi::xml_node output_node) : Output(settings, model, scenario, output_node) {
    file = 0;
    timestep = 0;
    flush = true;
}

int NetCDFOutput::initialize() {
    if (output_node.attribute("file").empty()) {
        std::cerr << "ERROR: Output file name not given" << std::endl;
        return -1;
    }
    flush = output_node.attribute("flush").as_bool(true);
    filename = output_node.attribute("file").value();
    file = new NcFile(filename, NcFile::replace);
    if (!file) {
        std::cerr << "ERROR: Could not create output file " << filename << std::endl;
        return -1;
    }
    dim_time = file->addDim("time");
    dim_region = file->addDim("region", model->regions__R.size());
    dim_sector = file->addDim("sector", model->sectors__C.size());
    return 0;
}

void NetCDFOutput::internal_write_header(tm* timestamp, int max_threads, const char* version) {
    std::string str = asctime(timestamp);
    str.erase(str.end() - 1);
    file->putAtt("start_time", str);
    file->putAtt("max_threads", NcType::nc_INT, max_threads);
    file->putAtt("version", version);
}

void NetCDFOutput::internal_write_footer(tm* duration) { file->putAtt("duration", NcType::nc_INT, mktime(duration)); }

void NetCDFOutput::internal_write_xml_input(XMLInput* input) {
    std::stringstream ss;
    input->print(ss, "  ", pugi::format_indent | pugi::format_no_declaration, pugi::encoding_utf8, 0);
    file->putAtt("settings", ss.str());
}

void NetCDFOutput::internal_start() {
    NcVar var_sector_name = file->addVar("sector_name", NcType::nc_STRING, dim_sector);
    NcVar var_region_name = file->addVar("region_name", NcType::nc_STRING, dim_region);
    std::vector<size_t> index;
    index.push_back(0);
    std::cout << "Sectors:" << std::endl;
    for (size_t i = 0; i < model->sectors__C.size(); i++) {
        index[0] = i;
        var_sector_name.putVar(index, model->sectors__C[i]->name);
        std::cout << i << "," << model->sectors__C[i]->name << std::endl;
        sector_index.insert(std::make_pair(model->sectors__C[i], i));
    }
    std::cout << std::endl << "Regions:" << std::endl;
    for (size_t i = 0; i < model->regions__R.size(); i++) {
        index[0] = i;
        var_region_name.putVar(index, model->regions__R[i]->name);
        std::cout << i << "," << model->regions__R[i]->name << std::endl;
        region_index.insert(std::make_pair(model->regions__R[i], i));
    }
    for (pugi::xml_node_iterator it = observables.begin(); it != observables.end(); it++) {
        std::string name = it->name();
        if (name == "production_site") {
            for (pugi::xml_node observable = it->child("parameter"); observable; observable = observable.next_sibling("parameter")) {
                std::string name = std::string("production_site__") + observable.attribute("name").value();
                Variable v;
                std::vector<NcDim> dims;
                dims.push_back(dim_time);
                dims.push_back(dim_sector);
                dims.push_back(dim_region);
                v.nc_var = file->addVar(name, NcType::nc_DOUBLE, dims);
                v.dim = dims.size() - 1;
                variables.insert(std::make_pair(name, v));
            }
            for (pugi::xml_node input_storage_node = it->child("input_storage"); input_storage_node;
                 input_storage_node = input_storage_node.next_sibling("input_storage")) {
                for (pugi::xml_node observable = input_storage_node.child("parameter"); observable; observable = observable.next_sibling("parameter")) {
                    std::string name = std::string("input_storage__") + observable.attribute("name").value();
                    Variable v;
                    std::vector<NcDim> dims;
                    dims.push_back(dim_time);
                    dims.push_back(dim_sector);
                    dims.push_back(dim_region);
                    dims.push_back(dim_sector);
                    v.nc_var = file->addVar(name, NcType::nc_DOUBLE, dims);
                    v.dim = dims.size() - 1;
                    variables.insert(std::make_pair(name, v));
                }
            }
            for (pugi::xml_node outgoing_connection_node = it->child("outgoing_connection"); outgoing_connection_node;
                 outgoing_connection_node = outgoing_connection_node.next_sibling("outgoing_connection")) {
                for (pugi::xml_node observable = outgoing_connection_node.child("parameter"); observable; observable = observable.next_sibling("parameter")) {
                    std::string name = std::string("outgoing_connection__") + observable.attribute("name").value();
                    Variable v;
                    std::vector<NcDim> dims;
                    dims.push_back(dim_time);
                    dims.push_back(dim_sector);
                    dims.push_back(dim_region);
                    dims.push_back(dim_sector);
                    dims.push_back(dim_region);
                    v.nc_var = file->addVar(name, NcType::nc_DOUBLE, dims);
                    v.dim = dims.size() - 1;
                    variables.insert(std::make_pair(name, v));
                }
            }
            for (pugi::xml_node outgoing_connection_node = it->child("final_demand_connection"); outgoing_connection_node;
                 outgoing_connection_node = outgoing_connection_node.next_sibling("final_demand_connection")) {
                for (pugi::xml_node observable = outgoing_connection_node.child("parameter"); observable; observable = observable.next_sibling("parameter")) {
                    std::string name = std::string("final_demand_connection__") + observable.attribute("name").value();
                    Variable v;
                    std::vector<NcDim> dims;
                    dims.push_back(dim_time);
                    dims.push_back(dim_sector);
                    dims.push_back(dim_region);
                    dims.push_back(dim_region);
                    v.nc_var = file->addVar(name, NcType::nc_DOUBLE, dims);
                    v.dim = dims.size() - 1;
                    variables.insert(std::make_pair(name, v));
                }
            }
        } else if (name == "final_demand") {
            for (pugi::xml_node observable = it->child("parameter"); observable; observable = observable.next_sibling("parameter")) {
                std::string name = std::string("final_demand__") + observable.attribute("name").value();
                Variable v;
                std::vector<NcDim> dims;
                dims.push_back(dim_time);
                dims.push_back(dim_region);
                v.nc_var = file->addVar(name, NcType::nc_DOUBLE, dims);
                v.dim = dims.size() - 1;
                variables.insert(std::make_pair(name, v));
            }
            for (pugi::xml_node input_storage_node = it->child("input_storage"); input_storage_node;
                 input_storage_node = input_storage_node.next_sibling("input_storage")) {
                for (pugi::xml_node observable = input_storage_node.child("parameter"); observable; observable = observable.next_sibling("parameter")) {
                    std::string name = std::string("final_demand_input_storage__") + observable.attribute("name").value();
                    Variable v;
                    std::vector<NcDim> dims;
                    dims.push_back(dim_time);
                    dims.push_back(dim_region);
                    dims.push_back(dim_sector);
                    v.nc_var = file->addVar(name, NcType::nc_DOUBLE, dims);
                    v.dim = dims.size() - 1;
                    variables.insert(std::make_pair(name, v));
                }
            }
        } else if (name == "region") {
            for (pugi::xml_node observable = it->child("parameter"); observable; observable = observable.next_sibling("parameter")) {
                std::string name = std::string("region__") + observable.attribute("name").value();
                Variable v;
                std::vector<NcDim> dims;
                dims.push_back(dim_time);
                dims.push_back(dim_region);
                v.nc_var = file->addVar(name, NcType::nc_DOUBLE, dims);
                v.dim = dims.size() - 1;
                variables.insert(std::make_pair(name, v));
            }
        } else if (name == "world") {
            for (pugi::xml_node observable = it->child("parameter"); observable; observable = observable.next_sibling("parameter")) {
                std::string name = std::string("world__") + observable.attribute("name").value();
                Variable v;
                std::vector<NcDim> dims;
                dims.push_back(dim_time);
                v.nc_var = file->addVar(name, NcType::nc_DOUBLE, dims);
                v.dim = dims.size() - 1;
                variables.insert(std::make_pair(name, v));
            }
        }
    }
    timestep = 0;
}

void NetCDFOutput::internal_write_double(const char* name, double v) {
    Target t = stack.back();
    std::unordered_map<std::string, Variable>::const_iterator it = variables.find(t.name + "__" + name);
    if (it == variables.end()) {
        return;
    }
    Variable var = it->second;
    std::vector<size_t> index;
    index.push_back(timestep);
    if (t.sector >= 0) {
        if (t.region >= 0) {
            if (var.dim == 2) {
                index.push_back((size_t)t.sector);
                index.push_back((size_t)t.region);
            } else if (var.dim == 4) {
                Target t2 = stack[stack.size() - 2];
                index.push_back((size_t)t2.sector);
                index.push_back((size_t)t2.region);
                index.push_back((size_t)t.sector);
                index.push_back((size_t)t.region);
            }
        } else {
            if (var.dim == 1) {
                index.push_back((size_t)t.sector);
            } else if (var.dim == 2) {
                Target t2 = stack[stack.size() - 2];
                index.push_back((size_t)t2.region);
                index.push_back((size_t)t.sector);
            } else if (var.dim == 3) {
                Target t2 = stack[stack.size() - 2];
                index.push_back((size_t)t2.sector);
                index.push_back((size_t)t2.region);
                index.push_back((size_t)t.sector);
            }
        }
    } else {
        if (t.region >= 0) {
            if (var.dim == 1) {
                index.push_back((size_t)t.region);
            } else if (var.dim == 3) {
                Target t2 = stack[stack.size() - 2];
                index.push_back((size_t)t2.sector);
                index.push_back((size_t)t2.region);
                index.push_back((size_t)t.region);
            }
        }
    }
    var.nc_var.putVar(index, v);
}

void NetCDFOutput::internal_start_target(const char* name, Sector* sector, Region* region) {
    Target t;
    t.name = name;
    t.sector = sector_index[sector];
    t.region = region_index[region];
    stack.push_back(t);
}

void NetCDFOutput::internal_start_target(const char* name, Sector* sector) {
    Target t;
    t.name = name;
    t.sector = sector_index[sector];
    t.region = -1;
    stack.push_back(t);
}

void NetCDFOutput::internal_start_target(const char* name, Region* region) {
    Target t;
    t.name = name;
    t.sector = -1;
    t.region = region_index[region];
    stack.push_back(t);
}

void NetCDFOutput::internal_start_target(const char* name) {
    Target t;
    t.name = name;
    t.sector = -1;
    t.region = -1;
    stack.push_back(t);
}

void NetCDFOutput::internal_end_target() { stack.pop_back(); }

void NetCDFOutput::internal_iterate_end() {
    timestep++;
    if (flush) {
        delete file;
        file = new NcFile(filename, NcFile::write);
    }
}

void NetCDFOutput::internal_end() { delete file; }
