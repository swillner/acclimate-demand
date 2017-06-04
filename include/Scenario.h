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

#ifndef _SCENARIO_H
#define _SCENARIO_H

#include "XMLInput.h"
#include "types.h"

class Model;
class ProductionSite;
class FinalDemand;

class Scenario {
  private:
    void set_production_site_property(ProductionSite* production_site, pugi::xml_node node);
    void set_final_demand_property(FinalDemand* final_demand, pugi::xml_node node);

  protected:
    pugi::xml_node scenario_node;

  public:
    Model* const model;
    Time start_time;
    Time stop_time;
    bool iterate();
    Scenario(XMLInput* scenario_settings, Model* model);
};

#endif
