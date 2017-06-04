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

#include "Storage.h"

#include "BusinessConnection.h"
#include "EconomicAgent.h"
#include "Model.h"
#include "ProductionSite.h"
#include "PurchasingManager.h"
#include "Sector.h"

Storage::Storage(Sector* sector, EconomicAgent* economic_agent, PurchasingManager* purchasing_manager)
    : sector(sector), economic_agent(economic_agent), purchasing_manager(purchasing_manager) {
    forcing__mu = 1;
    used_flow__U = 0;
    input_flow__I[0] = 0;
    input_flow__I[1] = 0;
    input_flow__I[2] = 0;
    initial_input_flow__I_star = 0;
    initial_content__S_star = 0;
    desired_used_flow__U_tilde = 0;
    content__S = 0;
    purchasing_manager->storage = this;
}

Storage::~Storage() { delete purchasing_manager; }

void Storage::iterate_consumption_and_production() {
    calc_content__S();
    input_flow__I[2] = input_flow__I[sector->model->other_register];
    input_flow__I[sector->model->other_register] = 0;
    purchasing_manager->iterate_consumption_and_production();
}

Flow Storage::calc_possible_use__U_hat() { return content__S / sector->model->delta_t + current_input_flow__I(); }

Flow Storage::current_input_flow__I() { return input_flow__I[sector->model->other_register]; }

Flow Storage::last_input_flow__I() { return input_flow__I[2]; }

void Storage::calc_content__S() {
    content__S = MAX(MIN(content__S + sector->model->delta_t * (current_input_flow__I() - used_flow__U),  // old I used here
                         forcing__mu * initial_content__S_star * sector->upper_storage_limit__omega),
                     (Basetype)0);
}

void Storage::use_content__S(Flow used_flow__U_current) { used_flow__U = used_flow__U_current; }

void Storage::set_desired_used_flow__U_tilde(Flow desired_used_flow__U_tilde) { this->desired_used_flow__U_tilde = desired_used_flow__U_tilde; }

void Storage::push_flow__Z(Flow flow__Z) {
#pragma omp atomic
    input_flow__I[sector->model->current_register] += flow__Z;
}

Flow Storage::next_input_flow__I() { return input_flow__I[sector->model->current_register]; }

void Storage::add_initial_flow__Z_star(Flow flow__Z_star) {
    input_flow__I[1] += flow__Z_star;
    input_flow__I[2] += flow__Z_star;
    initial_input_flow__I_star += flow__Z_star;
    initial_content__S_star += flow__Z_star * sector->initial_storage_fill_factor__psi;
    content__S += flow__Z_star * sector->initial_storage_fill_factor__psi;
    purchasing_manager->demand__D += flow__Z_star;
}
