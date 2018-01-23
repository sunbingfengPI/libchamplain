/*
 * Copyright (C) 2008 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <champlain/champlain.h>
#include <math.h>

#ifndef GREATECRICLEBEARING
#define GREATECRICLEBEARING

static double _gcb_bearing(double lat1, double long1, double lat2, double long2)
{
    double pi = acos(-1);
    double degToRad = pi / 180.0;

    double phi1 = lat1 * degToRad;
    double phi2 = lat2 * degToRad;
    double lam1 = long1 * degToRad;
    double lam2 = long2 * degToRad;

    // positive when clockwize
    return atan2(sin(lam2-lam1)*cos(phi2),\
        cos(phi1)*sin(phi2) - sin(phi1)*cos(phi2)*cos(lam2-lam1));
}

static double gcb_initial (double lat1, double long1, double lat2, double long2)
{
    double pi = acos(-1);
    return fmod((_gcb_bearing(lat1, long1, lat2, long2) + pi * 2), pi * 2);
}
static double gcb_final(double lat1, double long1, double lat2, double long2)
{
    double pi = acos(-1);
    return fmod((_gcb_bearing(lat1, long1, lat2, long2) + pi), pi * 2);
}

static double gcb_distance(double lat1, double long1, double lat2, double long2)
{
    double pi = acos(-1);
    double degToRad = pi / 180.0;
    // Convert coordinates to radians (rad = deg * (pi / 2))
    double lat_1_rad, lon_1_rad, lat_2_rad, lon_2_rad;
    lat_1_rad = lat1 * degToRad;
    lon_1_rad = long1 * degToRad;
    lat_2_rad = lat2 * degToRad;
    lon_2_rad = long2 * degToRad;

    // Determine latitude and longitude deltas
    double delta_lat, delta_lon;
    delta_lat = lat_1_rad - lat_2_rad;
    delta_lon = lon_1_rad - lon_2_rad;

    // Calculate sin^2 (delta / 2) for both lat and long
    double sdlat = pow(sin(delta_lat / 2), 2);
    double sdlon = pow(sin(delta_lon / 2), 2);

    // Radius of the Earth (approximate)
    const double radius_earth_km = 6378;

    // http://en.wikipedia/org/wiki/Haversine_formula
    // d=2r*asin(sqrt(sin^2((lat1-lat2)/2)+cos(l1)cos(l2)sin^2((lon2-lon1)/2)))
    //  if t = sqrt(sin^2((lat1-lat2)/2)+cos(l1)cos(l2)sin^2((lon2-lon1)/2)
    //  -> d = 2 * radius_earth * asin (t)  
    double t = sqrt(sdlat + (cos(lat_1_rad) * cos(lat_2_rad) * sdlon));
    double distance_km = 2 * radius_earth_km * asin(t);

    return distance_km * 1000;  // return value on meters
}

#endif
