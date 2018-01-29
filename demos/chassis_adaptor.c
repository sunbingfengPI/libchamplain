#include "chassis_adaptor.h"
#include <unistd.h>
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

  int path_len;
  gps_2d *path;
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
      G_TYPE_NONE, 1, G_TYPE_INT);
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

      priv->path_len = 0;
      priv->path = NULL;
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
        if(priv->path)
        {
          free(priv->path);
        }

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
  pi_system_command cmd;
  uint8_t msg[sizeof (pi_system_command)];
  int rc, retry = 3;

  PIChassisAdaptorPrivate *priv = CHASSIS_ADAPTOR_GET_PRIVATE(self);
  if(priv->connected)
  {
    // only allowed when connected.
    cmd.type = PI_COMMAND_SET_TARGET;
    memcpy(cmd.body, &target, sizeof(chassis_pos));
    memcpy (msg, &cmd, sizeof (pi_system_command));
    rc = nn_send (chassis_fd, msg, sizeof (msg), NN_DONTWAIT);   

    while(rc == EAGAIN && retry-- > 0) 
    {
      usleep(1000);
      rc = nn_send (chassis_fd, msg, sizeof (msg), NN_DONTWAIT); 
    }
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

gboolean pi_chassis_adaptor_get_path(PIChassisAdaptor *self, gps_2d **path, int *len)
{
  if(!path || !len)
  {
    return FALSE;
  }

  PIChassisAdaptorPrivate *priv = CHASSIS_ADAPTOR_GET_PRIVATE(self);
  *path = priv->path;
  *len = priv->path_len;

  return TRUE;
}

static void pi_chassis_adaptor_set_path(PIChassisAdaptor *self, gps_2d *nodes, int size)
{
  PIChassisAdaptorPrivate *priv = CHASSIS_ADAPTOR_GET_PRIVATE(self);

  if(!priv->path)
  {
    priv->path = (gps_2d *)malloc(size * sizeof(gps_2d));
  }
  else
  {
    priv->path = (gps_2d *)realloc(priv->path, size * sizeof(gps_2d));
  }

  memcpy((void *)priv->path, (void *)nodes, size * sizeof(gps_2d));
  priv->path_len = size;
}

static gboolean pi_chassis_adaptor_recv_check(pi_chassis_status_common *pcom, int r_size)
{
  int true_size = 0;
  gboolean ret = FALSE;

  switch(pcom->type)
  {
    case PI_CHASSIS_STATUS_TYPE_POS:
      true_size = sizeof(pi_chassis_status_common) - sizeof(void *) + sizeof(chassis_pos);
      ret = (true_size == r_size);

      printf("expected: %d, recv: %d\n", true_size, r_size);
      break;
    case PI_CHASSIS_STATUS_TYPE_PATH:
      true_size = sizeof(pi_chassis_status_common) - sizeof(void *) + pcom->length * sizeof(gps_2d);
      ret = (true_size == r_size);

      break;
    default:
      break;
  }

  return ret;
}

static void pi_chassis_adaptor_recv_handle(int type, int len, void * data)
{

}

static void 
pi_chassis_adaptor_listen(PIChassisAdaptor *self)
{
  int rc;

  for (;on_listen;) {
      uint8_t *buf = NULL;

      rc = nn_recv (chassis_fd, &buf, NN_MSG, 0);
      if (rc > 0) {
        pi_chassis_status_common st_common;
        memcpy(&st_common, buf, sizeof(pi_chassis_status_common));

        if(!pi_chassis_adaptor_recv_check(&st_common, rc))
        {
          // recv frame wrong, abort!
          continue;
        }

        printf("type: %d\n", st_common.type);
        switch(st_common.type)
        {
          case PI_CHASSIS_STATUS_TYPE_POS:
            pi_chassis_adaptor_set_pos(self, *((chassis_pos *)(buf + offsetof(pi_chassis_status_common, data))));          
            break;
          case PI_CHASSIS_STATUS_TYPE_PATH:
            pi_chassis_adaptor_set_path(self, (chassis_pos *)(buf + offsetof(pi_chassis_status_common, data)), st_common.length);
            break;
          default:
            break;
        }
        
        g_signal_emit (self, chassis_adaptor_signals[UPDATED], 0, st_common.type);   
        nn_freemsg(buf);
      }
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
    listen_thread = g_thread_new("sub_chassis", &pi_chassis_adaptor_listen, self);
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
