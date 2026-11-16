// Ported from tb128fuPkg, which uses no board raw files: its Mu-only driver
// set reads no SecParti.cfg, panel XML or charger configuration, because
// the stock TB128FU ABL carries a single statically linked LinuxLoader PE
// instead of separable vendor DXE modules.
//
// NOT YET VERIFIED for xun: whether its ABL is built the same way (no
// extractable vendor DXE, hence no Binaries/xun). Xiaomi's Khaje/bengal ABL
// may differ from Lenovo's. Confirm during Phase 0 by inspecting a real
// xun `abl` partition dump before assuming this file should stay empty.
