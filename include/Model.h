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

#ifndef _MODEL_H
#define _MODEL_H

#include <string>

#include "Output.h"
#include "types.h"

class Infrastructure;
class Sector;
class Region;
class ProductionSite;
class FinalDemand;

#define VERSION "1.1"

class Model {
  public:
    std::vector<Sector*> sectors__C;
    std::vector<Region*> regions__R;
    std::vector<Infrastructure*> infrastructure__G;
    Time time;
    unsigned char current_register;
    unsigned char other_register;
    Time delta_t;
    struct {
        bool demand;           // Default: false
        bool rewiring;         // Default: false
        Ratio history_weight;  // Default: 0
        Ratio rewiring_threshold;
        Ratio rewiring_establish_threshold;
        Ratio rewiring_abandon_threshold;
        Ratio rewiring_abandon_established_threshold;
        bool no_self_supply;  // Default: false
    } options;
    std::vector<Output*> outputs;

  public:
    Model();
    ~Model();
    void iterate();
    Region* find_region(std::string name);
    Sector* find_sector(std::string name);
    ProductionSite* find_production_site(std::string sector_name, std::string region_name);
    ProductionSite* find_production_site(Sector* sector, std::string region_name);
    FinalDemand* find_final_demand(Region* region);
    FinalDemand* find_final_demand(std::string region_name);
};

#endif
