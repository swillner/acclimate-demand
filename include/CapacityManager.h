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

#ifndef _CAPACITYMANAGER_H
#define _CAPACITYMANAGER_H

#include "types.h"

class Sector;
class EconomicAgent;
class ProductionSite;
class Storage;
class Region;
class PurchasingManager;

class CapacityManager {
  public:
    ProductionSite* const production_site;
    Ratio const possible_overcapacity_ratio__beta;
    Ratio production_capacity__p;
    Ratio desired_production_capacity__p_tilde;
    Ratio possible_production_capacity__p_hat;

  protected:
    void calc_desired_production_capacity__p_tilde();
    void calc_possible_production_capacity__p_hat();

  public:
    CapacityManager(ProductionSite* production_site, Ratio possible_overcapacity_ratio__beta);
    void calc_production_capacity__p();
};

#endif
