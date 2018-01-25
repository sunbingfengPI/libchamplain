#ifndef CHASSIS_ADAPTOR_H
#define CHASSIS_ADAPTOR_H

#include <glib-object.h>
#include <champlain/champlain.h>
#include "chassis_com.h"

G_BEGIN_DECLS

#define GTK_TYPE_CHASSIS_ADAPTOR                  (pi_chassis_adaptor_get_type ())

typedef struct _PIChassisAdaptor PIChassisAdaptor;
typedef struct _PIChassisAdaptorClass PIChassisAdaptorClass;
typedef struct _PIChassisAdaptorPrivate PIChassisAdaptorPrivate;

struct _PIChassisAdaptor
{
	GObject parent;

	PIChassisAdaptorPrivate *priv;
};

struct _PIChassisAdaptorClass
{
	GObjectClass parent_class;

	void (* updated) (PIChassisAdaptor *self);
};

GType pi_chassis_adaptor_get_type (void) G_GNUC_CONST;

PIChassisAdaptor* pi_chassis_adaptor_new (void);
chassis_pos pi_chassis_adaptor_get_pos (PIChassisAdaptor *self);
void pi_chassis_adaptor_set_target (PIChassisAdaptor *self, chassis_pos target);

int pi_chassis_adaptor_subscribe(PIChassisAdaptor *self, const char *url);
int pi_chassis_adaptor_unsubscribe(PIChassisAdaptor *self);

G_END_DECLS

#endif
