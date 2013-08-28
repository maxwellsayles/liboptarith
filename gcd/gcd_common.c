#include "liboptarith/gcd/gcd_common.h"

#include "liboptarith/math32.h"
#include "liboptarith/math64.h"

int64_t xgcd_common_s64(xgcd_s64_f* core_fnc,
			int64_t* out_s, int64_t* out_t,
			const int64_t in_u, const int64_t in_v) {
  // Create bit masks for u and v (either -1 or 0).
  const uint64_t um = in_u >> 63;
  const uint64_t vm = in_v >> 63;

  // Let u and v be positive.
  const int64_t u = negate_using_mask_s64(um, in_u);
  const int64_t v = negate_using_mask_s64(vm, in_v);

  // Call core function to compute GCD.
  int64_t s;
  int64_t t;
  const int64_t g = core_fnc(&s, &t, u, v);

  // Special cases for if g divides one of the inputs.
  if (g == u) {
    *out_s = um | 1;  // Either -1 or 1.
    *out_t = 0;
  } else if (g == v) {
    *out_s = 0;
    *out_t = vm | 1;  // Either -1 or 1.
  } else {
    // Reduce u1 (mod b/u3) and u2 (mod a/u3)
    // and correct for sign.
    // TODO: Try single division and the multiply by q for both.
    int64_t q = s / v;
    *out_s = negate_using_mask_s64(um, s - q*v);
    *out_t = negate_using_mask_s64(vm, t + q*u);
  }
  return g;
}
