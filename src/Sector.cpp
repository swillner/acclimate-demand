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

#include "Sector.h"

#include "Model.h"
#include "OutputStorage.h"
#include "ProductionSite.h"
#include "SalesManager.h"

Sector::Sector(Model* model, std::string name, Time storage_refill_enforcement__gamma, Ratio upper_storage_limit__omega, Time initial_storage_fill_factor__psi)
    : model(model),
      name(name),
      storage_refill_enforcement__gamma(storage_refill_enforcement__gamma),
      upper_storage_limit__omega(upper_storage_limit__omega),
      initial_storage_fill_factor__psi(initial_storage_fill_factor__psi) {
    total_demand__D = 0;
    total_production__X = 0;
}

void Sector::add_demand_request__D(Demand demand_request__D) {
#pragma omp atomic
    total_demand__D += demand_request__D;
}

void Sector::add_production__X(Flow production__X) {
#pragma omp atomic
    total_production__X += production__X;
}

Price Sector::calc_world_market_price__P() { return (Price)total_demand__D / (Price)total_production__X; }

void Sector::iterate_consumption_and_production() {
    total_demand__D = 0;
    total_production__X = 0;
}
