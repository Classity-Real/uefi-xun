/** @file
  Volume-up key, ported from tb128fuPkg's Tb128fuKeys.h -- NOT YET WORKING
  FOR XUN, see below.

  tb128fu's board device tree puts volume up on TLMM GPIO 5, active low
  (`gpios = <&tlmm 5 1>`), read with a flat MMIO access to a TLMM GPIO_IN
  register. xun's own device tree (read directly off a real unit,
  2026-11-17) shows volume up is GPIO 5 too, but on a DIFFERENT
  controller: `qcom,spmi@1c40000/qcom,pm6125@0/pinctrl@c000` -- the PM6125
  PMIC's own GPIO block, reached over the SPMI PMIC arbiter bus (see
  PMIC ARB SPMI at 0x01C00000 in MemoryMapLib.c), not a TLMM MMIO tile at
  all. The two devices' GPIO numbering for this key only looks the same by
  coincidence (both "5") -- the access mechanism is entirely different.

  sk_open/sk_sample below still do tb128fu's flat TLMM MMIO read, which
  will read the WRONG register on xun (there is no key wired to TLMM GPIO 5
  here) and so will never report the volume-up press. This needs to be
  replaced with an SPMI PMIC arbiter transaction against PM6125's GPIO 5 --
  not implemented yet. This is Phase 5 ("buttons") work, not a Phase 1
  blocker: the UEFI menu can still render without a working Enter key, it
  just won't be navigable yet.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef XUN_KEYS_H_
#define XUN_KEYS_H_

#include <stdint.h>

enum {
  SK_KEY_POWER = 1u << 0,     // volume up, mapped to Enter by the driver
  SK_KEY_VOLDOWN = 1u << 1,
  SK_KEY_VOLUP = 1u << 2
};

#define XUN_TB128FU_INHERITED_TLMM_GPIO5_IN 0x00405004UL // WRONG peripheral for xun, see file header

struct sk_arbiter {
  int unused;
};

struct sk_keys {
  int gpio;
};

static inline int sk_open (const struct sk_arbiter *a, struct sk_keys *k)
{
  (void) a;
  k->gpio = 1;
  return 0;
}

static inline int sk_sample (const struct sk_arbiter *a, const struct sk_keys *k, uint8_t *pressed)
{
  (void) a;
  (void) k;
  uint32_t value = *(volatile uint32_t *)(uintptr_t)XUN_TB128FU_INHERITED_TLMM_GPIO5_IN; // TODO(Phase 5): replace with an SPMI read of PM6125 pinctrl@c000 GPIO 5
  // Active low: bit 0 clear while the key is held.
  *pressed = (value & 1u) ? 0 : SK_KEY_POWER;
  return 0;
}

// Debounce and auto-repeat, copied from the sheng key driver: a key counts after two
// identical samples; volume keys repeat while held.
struct sk_filter {
  uint8_t last, stable;
  uint16_t held[3];
};

static inline uint8_t sk_step (struct sk_filter *f, uint8_t sample, uint16_t delay, uint16_t period)
{
  uint8_t out = 0;
  if (sample == f->last) {
    uint8_t fresh = (uint8_t)(sample & ~f->stable);
    f->stable = sample;
    out = fresh;
    for (unsigned i = 0; i < 3; ++i) {
      uint8_t bit = (uint8_t)(1u << i);
      if (!(sample & bit)) { f->held[i] = 0; continue; }
      if (fresh & bit) { f->held[i] = 0; continue; }
      ++f->held[i];
      if (bit != SK_KEY_POWER && f->held[i] >= delay && (f->held[i] - delay) % period == 0) {
        out |= bit;
      }
    }
  }
  f->last = sample;
  return out;
}

#endif
