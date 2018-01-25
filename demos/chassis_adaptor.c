#include "chassis_adaptor.h"
#include <champlain/champlain.h>
#include <nanomsg/nn.h>
#include <nanomsg/pair.h>

G_DEFINE_TYPE (PIChassisAdaptor, pi_chassis_adaptor, G_TYPE_OBJECT);

// #define CHASSIS_ADAPTOR_GET_PRIVATE(o) \
//   (G_TYPE_INSTANCE_GET_PRIVATE ((o), GTK_TYPE_CHASSIS_ADAPTOR, PIChassisAdaptorPrivate))
#define CHASSIS_ADAPTOR_GET_PRIVATE(o) \
  ((PIChassisAdaptorPrivate *) ((o)->priv))

enum {
  UPDATED,
  LAST_SIGNAL
};

static guint chassis_adaptor_signals[LAST_SIGNAL] = { 0 };

static int chassis_fd = -1;
static gboolean on_listen = FALSE;
static GThread * listen_thread;

// private member structor
struct _PIChassisAdaptorPrivate
{
  gboolean connected;
  chassis_pos latest_pos;
};

static void
pi_chassis_adaptor_class_init (PIChassisAdaptorClass *class)
{
  GObjectClass *gobject_class;
  gobject_class = G_OBJECT_CLASS (class);

  chassis_adaptor_signals[UPDATED] =
    g_signal_new ("updated",
      G_OBJECT_CLASS_TYPE (gobject_class),
      G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (PIChassisAdaptorClass, updated),
      NULL, NULL,
      NULL,
      G_TYPE_NONE, 0);
}

static void
pi_chassis_adaptor_init (PIChassisAdaptor *self)
{
  // self->priv = CHASSIS_ADAPTOR_PRIVATE (self);
  /* Allocate Private data structure */
  self->priv = \
      (PIChassisAdaptorPrivate *) g_malloc0(sizeof(PIChassisAdaptorPrivate));
  /* If correctly allocated, initialize parameters */
  if(self->priv != NULL)
  {
      PIChassisAdaptorPrivate *priv = CHASSIS_ADAPTOR_GET_PRIVATE(self);
      /* Initialize private data, if any */
      priv->connected = FALSE;
      priv->latest_pos.lat = 23.0;
      priv->latest_pos.lon = 112.0;
      priv->latest_pos.heading = 20;
  }  
}

static void
pi_chassis_adaptor_dispose(GObject *object)
{
    PIChassisAdaptor *self = (PIChassisAdaptor *)object;
    PIChassisAdaptorPrivate *priv = CHASSIS_ADAPTOR_GET_PRIVATE(self);
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
PIChassisAdaptor* pi_chassis_adaptor_new (void)
{
  return g_object_new (GTK_TYPE_CHASSIS_ADAPTOR, NULL);
}

void pi_chassis_adaptor_set_target (PIChassisAdaptor *self, chassis_pos target)
{
  uint8_t msg[sizeof (chassis_pos)];
  int rc;

  PIChassisAdaptorPrivate *priv = CHASSIS_ADAPTOR_GET_PRIVATE(self);
  if(priv->connected)
  {
    // only allowed connected.
    memcpy (msg, &target, sizeof (chassis_pos));

    rc = nn_send (chassis_fd, msg, sizeof (msg), 0);    
  }
}

chassis_pos pi_chassis_adaptor_get_pos (PIChassisAdaptor *self)
{
  PIChassisAdaptorPrivate *priv;
  priv = self->priv;

  printf("priv address:%f, %f, %f\n", priv->latest_pos.lat, priv->latest_pos.lon, priv->latest_pos.heading);

  return priv->latest_pos;
}

static void pi_chassis_adaptor_set_pos (PIChassisAdaptor *self, chassis_pos pos)
{
  PIChassisAdaptorPrivate *priv = CHASSIS_ADAPTOR_GET_PRIVATE(self);

  priv->latest_pos.lat = pos.lat;
  priv->latest_pos.lon = pos.lon;
  priv->latest_pos.heading = pos.heading;
}

static void 
pi_chassis_adaptor_update(PIChassisAdaptor *self)
{
  int rc;

  chassis_pos pos;
  for (;on_listen;) {
      uint8_t msg[sizeof (chassis_pos)];

      rc = nn_recv (chassis_fd, msg, sizeof (msg), 0);
      if (rc < 0) {
          fprintf (stderr, "nn_recv: %s\n", nn_strerror (nn_errno ()));
          break;
      }
      if (rc != sizeof (msg)) {
          fprintf (stderr, "nn_recv: got %d bytes, wanted %d\n",
              rc, (int)sizeof (msg));
           break;
      }
      memcpy (&pos, msg, sizeof (chassis_pos));
      pi_chassis_adaptor_set_pos(self, pos);

      g_signal_emit (self, chassis_adaptor_signals[UPDATED], 0);

      printf ("latest position reached, lat: %f, lon: %f, heading: %f\n", pos.lat, pos.lon, pos.heading);
  }

  printf("thread exit\n");
}

/*  The client runs in a loop, displaying the content. */
int 
pi_chassis_adaptor_subscribe(PIChassisAdaptor *self, const char *url)
{
    int fd;
    int rc;

    fd = nn_socket (AF_SP, NN_PAIR);
    if (fd < 0) {
        fprintf (stderr, "nn_socket: %s\n", nn_strerror (nn_errno ()));
        return (-1);
    }

    if (nn_connect (fd, url) < 0) {
        fprintf (stderr, "nn_socket: %s\n", nn_strerror (nn_errno ()));
        nn_close (fd);
        return (-1);        
    }

    on_listen = TRUE;
    listen_thread = g_thread_new("sub_chassis", &pi_chassis_adaptor_update, self);
    chassis_fd = fd;

    PIChassisAdaptorPrivate *priv = CHASSIS_ADAPTOR_GET_PRIVATE(self);
    priv->connected = TRUE;

    return (0);
}

int 
pi_chassis_adaptor_unsubscribe(PIChassisAdaptor *self)
{
  g_thread_unref(listen_thread);
  on_listen = FALSE;

  if(chassis_fd > 0)
  {
    nn_close (chassis_fd);
  }

  chassis_fd = -1;

  PIChassisAdaptorPrivate *priv = CHASSIS_ADAPTOR_GET_PRIVATE(self);
  priv->connected = FALSE;

  return 0;
}
