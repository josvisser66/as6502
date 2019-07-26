/* Definition of the O65 object file format */

#define STARTREC (1)
#define PUBLICREC (2)
#define RELOCREC (3)
#define CODEREC (4)
#define EXTREQREC (5)

struct obj65rec {
  int type;
  union {
    unsigned at_lc;
    unsigned len;
    struct {
      char id[IDLEN];
      unsigned at_lc;
    } sym;
  } rec;
};
