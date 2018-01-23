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

static ClutterActor *target_marker;
static ClutterActor *current_marker;
static ChamplainView *mainView;

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

  double x = champlain_view_longitude_to_x(mainView, t_lon);

  ChamplainBoundingBox *box = champlain_view_get_bounding_box(mainView);
  gdouble width = champlain_view_longitude_to_x(mainView, box->right);
  gdouble height = champlain_view_latitude_to_y(mainView, box->bottom);

  gdouble c_x = champlain_view_longitude_to_x(mainView, c_lon);
  gdouble c_y = champlain_view_latitude_to_y(mainView, c_lat);
  clutter_actor_set_pivot_point(current_marker, c_x / width, c_y / height);

  champlain_bounding_box_free(box);

  double bearing = gcb_initial(c_lat, c_lon, t_lat, t_lon) * 180 / 3.1415926;
  clutter_actor_set_rotation_angle(current_marker, CLUTTER_Z_AXIS, bearing);

  fprintf (stderr, "%f, %f, %f, %f\n", width, height, c_x, c_y);
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

  target_marker = champlain_label_new_with_text ("Target", "Serif 14", NULL, NULL);
  champlain_label_set_draw_shadow(target_marker, FALSE);
  champlain_label_set_color (CHAMPLAIN_LABEL (target_marker), &orange);
  champlain_location_set_location (CHAMPLAIN_LOCATION (target_marker), 22.5798011609, 113.926382077);
  champlain_marker_layer_add_marker (layer, CHAMPLAIN_MARKER (target_marker));
  champlain_path_layer_add_node (*path, CHAMPLAIN_LOCATION (target_marker));
  champlain_marker_set_draggable(target_marker, TRUE);

  current_marker = champlain_label_new_from_file ("icons/car.png", &err);
  if (err != NULL)
  {
    // Report error to user, and free error
    fprintf (stderr, "Unable to create image label: %s\n", err->message);
    g_error_free (err);
  }

  gdouble c_lat = 22.5798001609, c_lon = 113.926382077;
  champlain_label_set_alignment(current_marker, PANGO_ALIGN_CENTER);
  champlain_label_set_color (CHAMPLAIN_LABEL (current_marker), &orange);
  champlain_label_set_draw_background (CHAMPLAIN_LABEL (current_marker), FALSE);
  champlain_location_set_location (CHAMPLAIN_LOCATION (current_marker), c_lat, c_lon);
  champlain_marker_layer_add_marker (layer, CHAMPLAIN_MARKER (current_marker));
  champlain_path_layer_add_node (*path, CHAMPLAIN_LOCATION (current_marker));
  champlain_marker_set_draggable(current_marker, FALSE);

  // custom handler
  g_signal_connect (G_OBJECT (target_marker), "drag-motion",
      G_CALLBACK (custom_marker_move), layer);  

  clutter_actor_show (layer_actor);
  return layer;
}
