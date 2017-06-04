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

#ifndef _STORAGE_H
#define _STORAGE_H

#include "types.h"

class Sector;
class EconomicAgent;
class PurchasingManager;
class ProductionSite;

class Storage {
  protected:
    Flow input_flow__I[3];

  public:
    Forcing forcing__mu;
    Sector* const sector;
    EconomicAgent* const economic_agent;
    Quantity content__S;
    Quantity initial_content__S_star;
    Flow initial_input_flow__I_star;
    PurchasingManager* purchasing_manager;
    Flow used_flow__U;
    Flow desired_used_flow__U_tilde;

  public:
    Storage(Sector* sector, EconomicAgent* economic_agent, PurchasingManager* purchasing_manager);
    ~Storage();
    void set_desired_used_flow__U_tilde(Flow desired_used_flow__U_tilde);
    void use_content__S(Flow used_flow__U_current);
    Flow calc_possible_use__U_hat();
    void push_flow__Z(Flow flow__Z);
    Flow current_input_flow__I();
    Flow last_input_flow__I();
    Flow next_input_flow__I();
    void add_initial_flow__Z_star(Flow flow__Z_star);
    void iterate_consumption_and_production();

  protected:
    void calc_content__S();
};

#endif
