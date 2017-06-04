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

#ifndef _ECONOMICAGENT_H
#define _ECONOMICAGENT_H

#include <string>

#include "types.h"

class Storage;
class Region;

enum EconomicAgentType { FINAL_DEMAND, PRODUCTION_SITE };

class EconomicAgent {
  public:
    Region* const region;
    EconomicAgentType const type;
    std::vector<Storage*> input_storages;

  public:
    EconomicAgent(Region* region, EconomicAgentType type);
    virtual ~EconomicAgent();
    virtual void iterate_consumption_and_production() = 0;
    void iterate_demand();
    Storage* find_input_storage(std::string sector_name);
    void remove_storage(Storage* storage);
};

#endif
