#include "zutil.h"
#include "deflate.h"

#define DIST_CODE_LEN  512
// see definition of array dist_code below

// Local data. These are initialized only once.
struct z_tree
{
	uch dist_code[DIST_CODE_LEN];
	// Distance codes. The first 256 values correspond to the distances
	// 3 .. 258, the last 256 values correspond to the top 8 bits of
	// the 15 bit distances.

	uch length_code[MAX_MATCH - MIN_MATCH + 1];
	// length code for each normalized match length (0 == MIN_MATCH)

	ct_data ltree[L_CODES + 2];
	// The literal tree. Since the bit lengths are imposed, there is no
	// need for the L_CODES extra codes used during heap construction. However
	// The codes 286 and 287 are needed to build a canonical tree (see _tr_init
	// below).

	ct_data dtree[D_CODES];
	// The distance tree. (Actually a trivial tree since all codes use 5 bits.

	int base_length[LENGTH_CODES];
	// First normalized length for each code (0 = MIN_MATCH)

	int base_dist[D_CODES];
	// First normalized distance for each code (0 = distance of 1)

	// extra bits for each length code
	int extra_lbits[LENGTH_CODES];
	//= { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0 };

	// extra bits for each distance code
	int extra_dbits[D_CODES];
	//= { 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13 };

	// extra bits for each bit length code
	int extra_blbits[BL_CODES];
	//= { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 7 };

	// The lengths of the bit length codes are sent in order of decreasing
	// probability, to avoid transmitting the lengths for unused bit length codes.
	uch bl_order[BL_CODES];
	//= { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };
};

// permutation of code lengths
void ztree_order_initialize(unsigned char *order);

void zlib_tree_init(struct z_tree *pzt);

