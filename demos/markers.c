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
#include <markers.h>
#include "great_circle_bearing.h"

typedef struct window_size_
{
  double width;
  double height;
}window_size;

ClutterActor *target_marker;
static ClutterActor *current_marker;
static ChamplainView *mainView;

static window_size getWindowViewSize(const ChamplainView *view)
{
  ChamplainBoundingBox *box = champlain_view_get_bounding_box(view);
  gdouble width = champlain_view_longitude_to_x(view, box->right);
  gdouble height = champlain_view_latitude_to_y(view, box->bottom);  
  champlain_bounding_box_free(box);

  return (window_size){.width=width, .height=height};
}

int
move_car(double lat, double lon)
{
  champlain_location_set_location (CHAMPLAIN_LOCATION (current_marker), lat, lon);
}

int
rotate_car(double toangle)
{
  gdouble c_lat, c_lon;
  window_size size = getWindowViewSize(mainView);

  c_lat = champlain_location_get_latitude (CHAMPLAIN_LOCATION (current_marker));
  c_lon = champlain_location_get_longitude (CHAMPLAIN_LOCATION (current_marker)); 

  gdouble c_x = champlain_view_longitude_to_x(mainView, c_lon);
  gdouble c_y = champlain_view_latitude_to_y(mainView, c_lat);
  clutter_actor_set_pivot_point(current_marker, c_x / size.width, c_y / size.height);
  
  clutter_actor_set_rotation_angle(current_marker, CLUTTER_Z_AXIS, toangle);

  return 0;
}


static void
custom_marker_move (ChamplainMarker *marker,
    gdouble dx,
    gdouble dy,
    ClutterEvent *event,
    ChamplainMarkerLayer *layer)
{
  gdouble t_lat, t_lon, c_lat, c_lon;

  t_lat = champlain_location_get_latitude (CHAMPLAIN_LOCATION (target_marker));
  t_lon = champlain_location_get_longitude (CHAMPLAIN_LOCATION (target_marker));

  c_lat = champlain_location_get_latitude (CHAMPLAIN_LOCATION (current_marker));
  c_lon = champlain_location_get_longitude (CHAMPLAIN_LOCATION (current_marker));  

  double bearing = gcb_initial(c_lat, c_lon, t_lat, t_lon) * 180 / 3.1415926;

  printf("target bearing: %f\n", bearing);
}

ChamplainMarkerLayer *
create_marker_layer (G_GNUC_UNUSED ChamplainView *view, ChamplainPathLayer **path)
{
  ChamplainMarkerLayer *layer;
  ClutterActor *layer_actor;
  ClutterColor orange = { 0xf3, 0x94, 0x07, 0xbb };
  GError *err = NULL;

  mainView = view;

  *path = champlain_path_layer_new ();
  layer = champlain_marker_layer_new_full (CHAMPLAIN_SELECTION_SINGLE);
  layer_actor = CLUTTER_ACTOR (layer);

  current_marker = champlain_label_new_from_file ("icons/car.png", &err);
  if (err != NULL)
  {
    // Report error to user, and free error
    fprintf (stderr, "Unable to create image label: %s\n", err->message);
    g_error_free (err);
  }

  gdouble c_lat = 22.5798001609, c_lon = 113.926382077;
  champlain_label_set_alignment(current_marker, PANGO_ALIGN_CENTER);
  champlain_label_set_draw_background (CHAMPLAIN_LABEL (current_marker), FALSE);
  champlain_location_set_location (CHAMPLAIN_LOCATION (current_marker), c_lat, c_lon);
  champlain_marker_layer_add_marker (layer, CHAMPLAIN_MARKER (current_marker));
  champlain_path_layer_add_node (*path, CHAMPLAIN_LOCATION (current_marker));
  champlain_marker_set_draggable(current_marker, FALSE);

  target_marker = champlain_label_new_from_file ("icons/target.png", &err);
  if (err != NULL)
  {
    // Report error to user, and free error
    fprintf (stderr, "Unable to create image label: %s\n", err->message);
    g_error_free (err);
  }
  champlain_label_set_alignment(target_marker, PANGO_ALIGN_CENTER);
  champlain_label_set_draw_background (CHAMPLAIN_LABEL (target_marker), FALSE);
  champlain_location_set_location (CHAMPLAIN_LOCATION (target_marker), c_lat, c_lon + 0.00001);
  champlain_marker_layer_add_marker (layer, CHAMPLAIN_MARKER (target_marker));
  champlain_path_layer_add_node (*path, CHAMPLAIN_LOCATION (target_marker));
  champlain_marker_set_draggable(target_marker, TRUE);

  // custom handler
  g_signal_connect (G_OBJECT (target_marker), "drag-motion",
      G_CALLBACK (custom_marker_move), layer);  

  clutter_actor_show (layer_actor);
  return layer;
}
