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

#include "Infrastructure.h"

#include "GeographicEntity.h"
#include "TransportChainLink.h"

Infrastructure::Infrastructure(Distance distance) : GeographicEntity(INFRASTRUCTURE), distance(distance) { forcing__nu = 1; }

void Infrastructure::set_forcing__nu(Forcing forcing__nu) {
    for (int i = 0; i < transport_chain_links.size(); i++) {
        transport_chain_links[i]->set_forcing__nu(forcing__nu);
    }
    this->forcing__nu = forcing__nu;
}

void Infrastructure::remove_transport_chain_link(TransportChainLink* transport_chain_link) {
    for (std::vector<TransportChainLink*>::iterator it = transport_chain_links.begin(); it != transport_chain_links.end(); it++) {
        if ((void*)(*it) == (void*)(transport_chain_link)) {
            transport_chain_links.erase(it);
            break;
        }
    }
}
