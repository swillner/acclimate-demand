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

#ifndef _TYPES_H
#define _TYPES_H

#include <algorithm>
#include <cmath>
#include <vector>

typedef double Basetype;
typedef Basetype Time;
typedef Basetype Quantity;
typedef Basetype Flow;   // Quantity/Time
typedef Basetype Ratio;  // in [0;1]
typedef Ratio Forcing;
typedef Flow Demand;
typedef Basetype Price;  // Demand/Quantity
typedef unsigned char TransportDelay;
typedef TransportDelay Distance;

inline Basetype round_(Basetype x) { return floor(x + 0.5); }

inline Flow MAKE_FLOW(Basetype a) { return round_(a * 1e3) * 1e-3; }

inline Demand MAKE_DEMAND(Basetype a) { return round_(a * 1e3) * 1e-3; }

inline Basetype MIN(Basetype a, Basetype b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}

inline Basetype MAX(Basetype a, Basetype b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

#endif
