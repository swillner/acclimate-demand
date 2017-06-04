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

#ifndef _MODELINITIALIZER_H
#define _MODELINITIALIZER_H

#include <string>
#include "Sector.h"
#include "XMLInput.h"
#include "types.h"

class ProductionSite;
class Model;
class Region;
class PurchasingManager;
class FinalDemand;
class EconomicAgent;

class ModelInitializer {
  private:
    // finds most specific parameter value (named property_name) for indicated regional site (region_name, sector_name)
    pugi::xml_node get_production_site_property(std::string sector_name, std::string region_name, std::string property_name);
    pugi::xml_node get_named_property(std::string tag_name, std::string node_name, std::string property_name);
    PurchasingManager* create_purchasing_manager(std::string sector_name, std::string region_name);
    pugi::xml_node get_parameter(std::string property_name, const char* parent_name = "parameters");

  protected:
    Model* const model;
    XMLInput* const settings;
    Sector* add_sector(std::string name);
    ProductionSite* add_production_site(Sector* sector, Region* region);
    FinalDemand* add_final_demand(Region* region);
    Flow flow_threshold;
    int initialize_connection(Sector* sector_from, Region* region_from, Sector* sector_to, Region* region_to, Flow flow);
    int initialize_connection(ProductionSite* production_site_from, EconomicAgent* economic_agent_to, Flow flow);
    int clean_network();

  public:
    ModelInitializer(Model* model, XMLInput* settings);
    virtual ~ModelInitializer() = 0;
    virtual int initialize() = 0;
};

#endif
