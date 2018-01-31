#ifndef RADAR_VIEW_H
#define RADAR_VIEW_H

#include <glib-object.h>
#include <gtk/gtk.h>
#include <champlain/champlain.h>
#include <champlain-gtk/champlain-gtk.h>
#include <clutter-gtk/clutter-gtk.h>
#include "chassis_com.h"

G_BEGIN_DECLS

#define GTK_TYPE_RADAR_VIEW                  (pi_radar_view_get_type ())

typedef struct _PIRadarView PIRadarView;
typedef struct _PIRadarViewClass PIRadarViewClass;
typedef struct _PIRadarViewPrivate PIRadarViewPrivate;

struct _PIRadarView
{
	GtkDrawingArea parent;

	PIRadarViewPrivate *priv;
};

struct _PIRadarViewClass
{
	GtkDrawingAreaClass parent_class;

	void (* updated) (PIRadarView *self, int type);
	// void (* path_updated) (PIRadarView *self);
};

GType pi_radar_view_get_type (void) G_GNUC_CONST;

PIRadarView* pi_radar_view_new (void);

G_END_DECLS

#endif
