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

#ifndef _OUTPUT_H
#define _OUTPUT_H

#include <ctime>

#include "XMLInput.h"

class Model;
class Scenario;
class ProductionSite;
class Storage;
class Sector;
class Region;
class BusinessConnection;
class FinalDemand;
class EconomicAgent;

class Output {
  private:
    void write_production_site_parameters(ProductionSite* p, pugi::xml_node parameters);
    void write_input_storage_parameters(Storage* s, pugi::xml_node parameters);
    void write_input_storages(EconomicAgent* ea, pugi::xml_node input_storages_node);
    void write_outgoing_connections(ProductionSite* p, pugi::xml_node output_flows_node);
    void write_connection_parameters(BusinessConnection* b, pugi::xml_node outgoing_connection_node);
    void write_final_demand_parameters(FinalDemand* fd, pugi::xml_node parameters);
    void write_final_demand_connections(ProductionSite* p, pugi::xml_node outgoing_connections_node);

  protected:
    XMLInput* settings;
    pugi::xml_node output_node;
    time_t start_time;
    pugi::xml_node observables;
    virtual void internal_write_header(tm* timestamp, int max_threads, const char* version);
    virtual void internal_write_footer(tm* duration);
    virtual void internal_write_xml_input(XMLInput* input);
    virtual void internal_start();
    virtual void internal_iterate_begin();
    virtual void internal_iterate_end();
    virtual void internal_end();
    virtual void internal_write_int(const char* name, int v);
    virtual void internal_write_double(const char* name, double v);
    virtual void internal_start_target(const char* name, Sector* sector, Region* region);
    virtual void internal_start_target(const char* name, Sector* sector);
    virtual void internal_start_target(const char* name, Region* region);
    virtual void internal_start_target(const char* name);
    virtual void internal_end_target();

  public:
    Model* const model;
    Scenario* const scenario;
    Output(XMLInput* settings, Model* model, Scenario* scenario, pugi::xml_node output_node);
    virtual int initialize() = 0;
    void start();
    void iterate();
    void end();
    virtual ~Output();
};

#endif
