#include "radar_view.h"
#include <unistd.h>
#include <champlain/champlain.h>
#include <nanomsg/nn.h>
#include <nanomsg/pair.h>

G_DEFINE_TYPE (PIRadarView, pi_radar_view, GTK_TYPE_DRAWING_AREA);

#define RADAR_VIEW_GET_PRIVATE(o) \
  ((PIRadarViewPrivate *) ((o)->priv))

enum {
  DUMMY_SIGNAL,
  LAST_SIGNAL
};

static guint radar_view_signals[LAST_SIGNAL] = { 0 };


// private member structor
struct _PIRadarViewPrivate
{
  double scale;
};

static void
pi_radar_view_class_init (PIRadarViewClass *class)
{

}

static void
pi_radar_view_init (PIRadarView *self)
{
  // self->priv = CHASSIS_ADAPTOR_PRIVATE (self);
  /* Allocate Private data structure */
  self->priv = \
      (PIRadarViewPrivate *) g_malloc0(sizeof(PIRadarViewPrivate));
  /* If correctly allocated, initialize parameters */
  if(self->priv != NULL)
  {
      PIRadarViewPrivate *priv = RADAR_VIEW_GET_PRIVATE(self);
      /* Initialize private data, if any */
  }  
}

static void
pi_radar_view_dispose(GObject *object)
{
    PIRadarView *self = (PIRadarView *)object;
    PIRadarViewPrivate *priv = RADAR_VIEW_GET_PRIVATE(self);
    /* Check if not NULL! To avoid calling dispose multiple times */
    if(priv != NULL)
    {
        /* Deallocate contents of the private data, if any */

        /* Deallocate private data structure */
        g_free(priv);
        /* And finally set the opaque pointer back to NULL, so that
         *  we don't deallocate it twice. */
        self->priv = NULL;
    }
}

// factory method to create new object
PIRadarView* pi_radar_view_new (void)
{
  return g_object_new (GTK_TYPE_RADAR_VIEW, NULL);
}
