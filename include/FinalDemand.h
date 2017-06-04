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

#ifndef _FINALDEMAND_H
#define _FINALDEMAND_H

#include "EconomicAgent.h"
#include "types.h"

class Region;

class FinalDemand : public EconomicAgent {
  public:
    Forcing forcing__kappa;

  public:
    FinalDemand(Region* region);
    Ratio calc_consumption_rate__c();
    void iterate_consumption_and_production();
};

#endif
