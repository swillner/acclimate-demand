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

#ifndef _SECTOR_H
#define _SECTOR_H

#include <string>
#include "types.h"

class Model;
class ProductionSite;

class Sector {
  public:
    Ratio const upper_storage_limit__omega;
    Time const initial_storage_fill_factor__psi;
    Time const storage_refill_enforcement__gamma;
    Demand total_demand__D;
    Flow total_production__X;
    std::string name;
    std::vector<ProductionSite*> production_sites__N;
    Model* const model;

  public:
    Sector(Model* model, std::string name, Time storage_refill_enforcement__gamma, Ratio upper_storage_limit__omega, Time initial_storage_fill_factor__psi);
    void add_demand_request__D(Demand demand_request__D);
    void add_production__X(Flow output_flow__chi);
    Price calc_world_market_price__P();
    void iterate_consumption_and_production();
};

#endif
