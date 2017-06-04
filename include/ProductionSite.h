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

#ifndef _PRODUCTIONSITE_H
#define _PRODUCTIONSITE_H

#include "EconomicAgent.h"
#include "types.h"

class Region;
class Sector;
class OutputStorage;
class CapacityManager;
class Storage;
class SalesManager;
class PurchasingManager;

class ProductionSite : public EconomicAgent {
  public:
    Forcing forcing__lambda;
    Flow initial_production__X_star;
    Flow direct_damage__d;
    OutputStorage* const output_storage;
    CapacityManager* const capacity_manager;
    Flow production__X;

  protected:
    void produce__X();
    void damage__d();

  public:
    ProductionSite(Sector* sector, Region* region, Ratio possible_overcapacity_ratio__beta, SalesManager* sales_manager, PurchasingManager* purchasing_manager);
    ~ProductionSite();
    void iterate_consumption_and_production();
    void add_initial_production__X_star(Flow initial_production_flow__X_star);
};

#endif
