
#include <assert.h>
#include <malloc.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ae.h>

static void transform(double *m, double x, double y, double z) {
  printf("%lf; %lf; %lf\n",
         m[ 0 + 0] * x +
         m[ 4 + 0] * y +
         m[ 8 + 0] * z + m[12 + 0],
         m[ 0 + 1] * x +
         m[ 4 + 1] * y +
         m[ 8 + 1] * z + m[12 + 1],
         m[ 0 + 2] * x +
         m[ 4 + 2] * y +
         m[ 8 + 2] * z + m[12 + 2]);
}

static GLuint list = -1;
#define CUBE list
#define SQUARE (list + 1)

static struct coor_t *ctr = NULL;

static void __attribute__ ((constructor)) create() {
  if (!corenv.gui) {
    fprintf(stderr, "only function under GUI mode\n");
    exit(1);
  }
}

static void __attribute__ ((destructor)) destroy() {
  if (ctr != NULL) {
    free(ctr);
    ctr = NULL;
  }
  if (list != -1u) {
    agDeleteLists(list, 2);
    list = -1u;
  }
}

static void cross
(double *x1, double *y1, double *z1, double x2, double y2, double z2)
{
  double dx = *y1 * z2 - *z1 * y2;
  double dy = *z1 * x2 - *x1 * z2;
  double dz = *x1 * y2 - *y1 * x2;
  *x1 = dx;
  *y1 = dy;
  *z1 = dz;
}

static bool backward(const struct coor_t *ctr, int *idx, double *dist,
                     struct coor_t *pos, double ofs)
{
  assert(*idx > 0);
  if (*dist < ofs) return false;
  int i = *idx - 1;
  struct coor_t to = *pos;
  double d = ofs;
  for (;;) {
    to.x -= ctr[i].x;
    to.y -= ctr[i].y;
    to.z -= ctr[i].z;
    double d2 = hypot(to.x, hypot(to.y, to.z));
    if (d2 >= d) {
      *idx = i + 1;
      *dist -= ofs;
      d2 = (d2 - d) / d2;
      pos->x = ctr[i].x + to.x * d2;
      pos->y = ctr[i].y + to.y * d2;
      pos->z = ctr[i].z + to.z * d2;
      return true;
    }
    else if (i == 0) return false;
    to = ctr[i];
    i -= 1;
    d -= d2;
  }
}

struct S {
  struct { // previous link
    struct link_t *link;
    struct coor_t *ctr; // center line
  } prev;
  struct coor_t *ctr; // center line of current link
  struct {
    struct {
      unsigned num;
      double wd;
      double ht;
      double dp;
    } carriage;
    double gap;
  } train;
};

static void (*defshape)(bool, bool, struct GConvey *, void *, const double[16]) = NULL;

void train
(bool tud, bool drv, struct GConvey *veh, void *info, const double view[16])
{
  assert(defshape != NULL);
  if (drv && !tud) return defshape(tud, drv, veh, info, view);

  assert(veh->sz >= sizeof(struct S));
  struct S *s = info;
  assert(list != -1u);

  struct coor_t *ctr = s->ctr;
  int idx = veh->prof_idx;
  assert(idx < veh->lane->link->prof_sz);
  double dist = ctr[idx].length;
  idx += 1;
  struct coor_t from = ctr[idx];

  /* veh->dist is based on the rail's right profile while dist is based on
   * the rail's center profile so sometimes dist may be lesser than
   * veh->dist though the difference is small. */
  bool success = backward(ctr, &idx, &dist, &from, fmax(dist - veh->dist, 0));
  assert(success); // don't enable it.

  unsigned num = s->train.carriage.num;
  for (;;) {
    struct coor_t to = from;

    void f(double ofs) {
      if (!backward(ctr, &idx, &dist, &to, ofs)) {
        if (!s->prev.ctr) {
          /* Some portion of the train will not be displayed as the
           * required information is missed. */
          to = ctr[0];
          num = 1;
          return;
        }
        assert(dist < ofs);
        ofs -= dist;
        ctr = s->prev.ctr;
        idx = s->prev.link->prof_sz;
        to = ctr[idx];
        dist = ctr[idx - 1].length;
        success = backward(ctr, &idx, &dist, &to, ofs);
        assert(success);
      }
    }

    f(s->train.carriage.dp);

    double ctrX = from.x - to.x, ctrY = from.y - to.y, ctrZ = from.z - to.z;
    double upX = ctrX, upY = ctrY, upZ = ctrZ;
    cross(&upX, &upY, &upZ, 0, 0, 1);
    cross(&upX, &upY, &upZ, ctrX, ctrY, ctrZ);
    double eyeX = (to.x + from.x) / 2;
    double eyeY = (to.y + from.y) / 2;
    double eyeZ = (to.z + from.z + s->train.carriage.ht) / 2;

    // draw one carriage
    //glMatrixMode(GL_MODELVIEW); // Usually it is unnecessary.
    glPushMatrix();
    double m[16];
    /* Using glLoadIdentity for initialization is wrong as each canvas
     * window has its own viewpoint represented by the passed `view'.
     * Thus the initialization is fulfilled by copying the matrix.
     * Note: this is very important! */
    memcpy(m, view, sizeof(m));
    aguLookAt(m, eyeX, eyeY, eyeZ, ctrX, ctrY, ctrZ, upX, upY, upZ);
    glLoadMatrixd(m);
    glScaled(s->train.carriage.wd, s->train.carriage.ht, s->train.carriage.dp);
    glCallList(CUBE);
    glPopMatrix();

    if (-- num == 0) break;
    if (s->train.gap > 0) f(s->train.gap);

    // draw the carriage connector?

    from = to;
  }
}

static unsigned train_carriage_num = 1; // number of carriages
static double train_carriage_wd = 2.4;
static double train_carriage_ht = 3.0;
static double train_carriage_dp = 16.366;
static double train_gap = 0.75;

static void rdgeom(const char *geom) {
  FILE *fptr = fopen(geom, "r");
  assert(fptr != NULL);
  rewind(fptr);
  while (!feof(fptr)) {
    char buf[256];
    fgets(buf, sizeof(buf), fptr);
    char *eq = strchr(buf, '=');
    if (eq != NULL) {
      *eq++ = '\0';
      if (!strcmp(buf, "train.carriage.num"))
        train_carriage_num = atoi(eq);
      else if (!strcmp(buf, "train.carriage.wd"))
        train_carriage_wd = atof(eq);
      else if (!strcmp(buf, "train.carriage.ht"))
        train_carriage_ht = atof(eq);
      else if (!strcmp(buf, "train.carriage.dp"))
        train_carriage_dp = atof(eq);
      else if (!strcmp(buf, "train.gap"))
        train_gap = atof(eq);
    }
  }
  fclose(fptr);
}

static void rdctr(const char *file) {
  FILE *fptr = fopen(file, "r");
  assert(fptr != NULL);
  char buf[256];
  fgets(buf, sizeof(buf), fptr);
  int i;
  sscanf(buf, "%i", &i);
  ctr = realloc(ctr, sizeof(*ctr) * i); // in case reentrant
  struct coor_t *ptr = &ctr[0];
  double d = 0;
  int j = 0;
  for (; j < i; ++ j, ++ ptr) {
    fgets(buf, sizeof(buf), fptr);
    sscanf(buf, "%lf,%lf,%lf", &ptr->x, &ptr->y, &ptr->z);
    if (j > 0) {
      /* Do not use the length information hold in struct link_t->prof
       * as some number mismatch would exist due to geometric
       * transposition operations. */
      d += hypot(ptr->x - ptr[-1].x, hypot(ptr->y - ptr[-1].y, ptr->z - ptr[-1].z));
      ptr[-1].length = d;
    }
  }
  fclose(fptr);
}

static void _I_transfer
(struct Machine *mach, struct convey_t *veh, void *info)
{
  /* As an exhibition, the backfill function is simple and static, that is
   * it does not consider the situation where trains may have different
   * geometries.  In real applications, this should be expanded. */
  struct S *s = info;
  s->prev.link = veh->udata;
  s->prev.ctr = s->prev.link ? s->prev.link->udata : NULL;
  s->ctr = ctr;
  s->train.carriage.num = train_carriage_num; // number of carriages
  s->train.carriage.wd = train_carriage_wd;
  s->train.carriage.ht = train_carriage_ht;
  s->train.carriage.dp = train_carriage_dp;
  s->train.gap = train_gap;
}

static void (*gltransfer(struct Machine *mach, struct convey_t *veh, unsigned *sz))
(struct Machine *, struct convey_t *, void *)
{
  /* Normally it is necessary to distinguish the vehicle's category.
   * But, as an exhibition, since only one train is assumed, the logic is
   * then simplified. */
  *sz = sizeof(struct S);
  return _I_transfer;
}

int g(struct Machine *mach, struct convey_t *veh, struct link_t *lnk) {
  /* called at the train's leave due to putv */
  veh->udata = lnk; // used to fill `struct S->prev'; see _I_transfer
}

int f(struct Machine *mach, const char *str, const char *geom) {
  if (list == -1u) // in case reentrant
    agCompile(({
          void f(void *dummy) {
            list = glGenLists(2);

            glNewList(CUBE, GL_COMPILE);
            glBegin(GL_QUADS);
            glColor3ub(0, 0, 0);

            glNormal3f(1, 0, 0);
            glVertex3f(0.5, 0.5, -0.5);
            glVertex3f(0.5, 0.5, 0.5);
            glVertex3f(0.5, -0.5, 0.5);
            glVertex3f(0.5, -0.5, -0.5);

            glNormal3f(-1, 0, 0);
            glVertex3f(-0.5, 0.5, 0.5);
            glVertex3f(-0.5, 0.5, -0.5); 
            glVertex3f(-0.5, -0.5, -0.5);
            glVertex3f(-0.5, -0.5, 0.5);

            glNormal3f(0, 0, 1);
            glVertex3f(0.5, 0.5, 0.5);
            glVertex3f(-0.5, 0.5, 0.5);
            glVertex3f(-0.5, -0.5, 0.5);
            glVertex3f(0.5, -0.5, 0.5);

            glNormal3f(0, 0, -1);
            glVertex3f(-0.5, 0.5, -0.5);
            glVertex3f(-0.5, -0.5, -0.5);
            glVertex3f(0.5, -0.5, -0.5);
            glVertex3f(0.5, 0.5, -0.5);

            glNormal3f(0, 1, 0);
            glVertex3f(0.5, 0.5, -0.5);
            glVertex3f(-0.5, 0.5, -0.5);
            glVertex3f(-0.5, 0.5, 0.5);
            glVertex3f(0.5, 0.5, 0.5);

            glNormal3f(0, -1, 0);
            glVertex3f(0.5, -0.5, 0.5);
            glVertex3f(-0.5, -0.5, 0.5);
            glVertex3f(-0.5, -0.5, -0.5);
            glVertex3f(0.5, -0.5, -0.5);

            glEnd();
            glEndList();

            glNewList(SQUARE, GL_COMPILE);
            glBegin(GL_POLYGON);

            glColor3ub(0, 0, 0);
            glNormal3f(0, 0, 1);
            glVertex3f(0.5, 0.5, -0.5);
            glVertex3f(-0.5, 0.5, -0.5);
            glVertex3f(-0.5, -0.5, -0.5);
            glVertex3f(0.5, -0.5, -0.5);

            glEnd();
            glEndList();
          }
          &f;
        }), NULL);
  rdctr(str); // read the center line
  rdgeom(geom); // read geometry
  /* install corresponding callbacks */
  mach->shared->evhuk.gltransfer = gltransfer;
  /* Again, the current implementation is simplified as it does not consider
   * the issue of shape of vehicles of different types.  In real situation,
   * this should be expanded too. */
  defshape = mach->shared->evhuk.glshape;
  mach->shared->evhuk.glshape = train;
  aeEnumLinks(mach->network, true, ({
        int f(struct link_t *lnk, void *dummy) {
          lnk->udata = ctr;
          return 0;
        }
        &f;
      }), NULL);
}
