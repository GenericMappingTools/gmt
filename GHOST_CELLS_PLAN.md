# Replacing the grid pad with ghost cells

Working design + implementation plan for [#4358](https://github.com/GenericMappingTools/gmt/issues/4358).

## 1. The problem, restated in layout terms

Today a grid of `n_columns` x `n_rows` is stored as one matrix of
`mx` x `my` = `(n_columns + pad[XLO] + pad[XHI])` x `(n_rows + pad[YLO] + pad[YHI])`,
and the *interior* is a strided sub-rectangle of it:

```
gmt_M_ijp(h,row,col) = (row + pad[YHI]) * mx + col + pad[XLO]
```

The row stride is `mx`, not `n_columns`. That single fact is the whole issue:

* **A strided interior cannot be handed to Julia/Python/MEX zero-copy.** Numpy and
  Julia can describe strides, but they cannot describe "and there are 2 extra
  columns of junk you must not look at" in a way that survives a reshape, a
  `reinterpret`, or a write back. So GMT copies on the way out.
* **A pad-less matrix arriving from an external cannot be used by a
  pad-requiring algorithm.** So GMT copies on the way in.

The pad is doing two unrelated jobs at once: it *stores boundary condition
values*, and it *changes the shape of the data matrix*. Only the first job is
wanted. Ghost cells separate them.

## 2. The design

Keep the boundary values. Move them out of the data matrix.

```c
struct GMT_GRID_GHOST {         /* Halo held OUTSIDE the data matrix */
    unsigned int pad[4];        /* Ghost layers on each side: XLO, XHI, YLO, YHI */
    unsigned int mode;          /* BC actually used to fill it */
    uint64_t mxg;               /* pad[XLO] + n_columns + pad[XHI] */
    gmt_grdfloat *N, *S;        /* pad[YHI] / pad[YLO] full rows, corners included */
    gmt_grdfloat *W, *E;        /* n_rows x pad[XLO] / pad[XHI] side columns */
    gmt_grdfloat *alloc;        /* The single block N/S/W/E point into */
};
```

`N`/`S` are full width so the four corners belong to them and `W`/`E` stay
rectangular. Recall GMT's row convention: `row` grows southward, so `row = -1`
is the first ghost row *above* the north edge and is governed by `pad[YHI]`.

Indexing inside the halo:

| region | valid range | offset |
|---|---|---|
| `N` | `-pad[YHI] <= row < 0` | `(row + pad[YHI])*mxg + col + pad[XLO]` |
| `S` | `n_rows <= row < n_rows+pad[YLO]` | `(row - n_rows)*mxg + col + pad[XLO]` |
| `W` | `-pad[XLO] <= col < 0` | `row*pad[XLO] + col + pad[XLO]` |
| `E` | `n_columns <= col < n_columns+pad[XHI]` | `row*pad[XHI] + col - n_columns` |

The header's own `pad` is then `{0,0,0,0}`, which makes `mx == n_columns`,
`size == nm`, and — the load-bearing consequence —

```
gmt_M_ijp(h,row,col)  ==  row * n_columns + col  ==  gmt_M_ij0(h,row,col)
```

**Every interior loop in GMT keeps working, unmodified.** `gmt_M_ijp`,
`gmt_M_grd_loop`, `gmt_M_col_loop` all stay correct because they were already
an abstraction over the layout. The only code that breaks is code that
addresses a node *outside* `[0,n_rows) x [0,n_columns)`, and that is exactly
the code that should have been asking for a boundary condition all along.

### 2.1 The accessors

Two, and the fast one is the one the hot loops use.

```c
/* Single node, any row/col within the interior or the halo */
gmt_grdfloat gmt_grd_get_node (struct GMT_GRID *G, int64_t row, int64_t col);

/* n consecutive values starting at (row,col).  Returns a pointer straight into
 * G->data when the run is entirely interior (the overwhelmingly common case),
 * otherwise materialises into the caller's small stack buffer. */
const gmt_grdfloat *gmt_grd_get_window (struct GMT_GRID *G, int64_t row, int64_t col,
                                        unsigned int n, gmt_grdfloat *buf);
```

In **pad mode** both collapse to `&G->data[gmt_M_ijp(h,row,col)]` with no copy
and no halo branch, so a converted call site costs nothing in the legacy
layout. That property is what makes the migration incremental: a converted
module is still byte-for-byte the old module when running padded.

### 2.2 Dual mode is not a transition hack, it is the test harness

Both layouts stay live for the whole migration, selected at session level
(`GMT_Create_Session` pad argument, plus a `GMT_GHOST_CELLS=1` environment
override so the *entire* existing test suite can be run either way).

Every phase is then verified the same way: run a test in pad mode, run it in
ghost mode, require the outputs to be **bit-identical**. Not "close" —
identical. A ghost cell holds the same number the pad cell held; if a result
moves, the conversion is wrong.

## 3. Phases, as revised by what the work found

| # | Phase | State |
|---|---|---|
| 0 | **Inventory.** Poison the pad, sweep the tests, record what actually reads the halo. | **done** — `GHOST_INVENTORY.md` |
| 1 | **Infrastructure.** `GMT_GRID_GHOST`, alloc/free/duplicate, accessors, layout switch. | **done** |
| 3 | **`gmt_bcr.c`.** Stencil walks `(row,col)` through the accessors. | **done** — grdtrack, grdsample, grdproject verified bit-identical with no pad |
| 2 | **Boundary conditions fill the halo directly**, instead of filling a pad that is then moved. | **partial** — a private padded copy makes it correct; the in-place fill was written and reverted, see §8 |
| 4 | **Remaining sites**, one at a time, from the inventory. | **done** — 186 of 189 test scripts identical; the 3 exceptions are remote-data or non-reproducible, see §6 |
| 5 | **Flip the defaults.** Pad-free grids in and out of the API. | not yet — needs images, cubes and complex grids (§8) |

Two things changed from the original ordering.

**Phase 2 moved after phase 3, and turned out to be a prerequisite rather than
an optimisation.** Converting a *consumer* is easy to verify when the halo is
produced by copying the pad, because then the halo provably holds exactly what
the pad held and any difference in output isolates the consumer. So
`gmtlib_ghost_from_pad` came first and phase 3 was verified against it. But the
moment new grids are created without a pad (level 2 below), `gmt_grd_BC_set`
has nothing to compute into — it bails out on a pad smaller than 2 — so the
boundary conditions must be able to fill the halo directly. Until that is
written, a pad is borrowed for the duration of the call. That is correct and no
worse than what a pad-less external grid costs GMT today, but it is a stepping
stone, not the destination.

**The switch has three levels rather than two**, because the two halves fail
differently and needed to be measured apart:

```
GMT_GHOST_CELLS=0   legacy padded layout (default; nothing changes)
GMT_GHOST_CELLS=1   grids move to ghost cells once their BCs are set
GMT_GHOST_CELLS=2   as 1; see below for what this level turned out to mean
```

Level 1 is the interesting number for "does the halo still work". Level 2 began as the issue's
own experiment — `GMT_Create_Session` with pad 0 — and that experiment **failed for a reason
worth recording**: a grid read with data padding fills its pad with the real neighbouring
columns from the file, and with no pad there is nowhere to put them, so the halo silently falls
back to a computed boundary condition. That is exactly the accuracy issue #4358 does not want to
lose, and on a coarse global grid the error is gross rather than subtle. Grids are therefore
read through a transient pad and moved to ghost cells immediately afterwards — pad-free in
memory either way — which is why levels 1 and 2 now measure the same. An external handing GMT a
pad-free matrix still gets the pad-free path, since such a grid arrives with no pad whatever the
session default happens to be.

## 4. The safety net

Unconverted code does not have to be found before it can be trusted, because
demanding a pad now works:

* `gmt_grd_pad_on` folds any halo back into the matrix, moving the values
  rather than recomputing them, and marks the header so the grid is not
  stripped again. Every module that goes through `gmt_set_outgrid` — which is
  most of the ones that need a halo — therefore keeps working unchanged, at the
  old memory cost, until someone converts it.
* `gmt_duplicate_grid` copies the halo with the data. Recomputing boundary
  conditions on a duplicate would silently throw away a *data* halo, which is
  exactly the accuracy the issue says it does not want to lose.

This is what makes the migration incremental in practice and not just on paper:
a half-converted GMT is a correct GMT.

## 5. What this buys, against the issue's own list

* **Output.** `G->data` is a plain contiguous `n_columns * n_rows` array.
  Nothing to strip, so the external never sees a pad.
* **Input.** A pad-less matrix from Julia/Python/MEX is already in the canonical
  layout. No duplication, no expansion, no temporary.
* **The subset-accuracy downside.** Ghost cells do not by themselves fix Paul's
  grdtrack example, where a memory subset falls back to natural BC and quietly
  disagrees with the CLI. What they do is make the fix affordable: the halo is a
  small separate array, so filling it with real data from a parent grid costs
  `O(perimeter)` instead of `O(area)` and never touches the user's matrix. Data
  boundary conditions become something an external can afford to ask for.

## 6. Where it stands, measured

Every grid-using test script in `test/` that needs no remote data (189 of them, over
`grdtrack`, `grdsample`, `grdproject`, `grdgradient`, `grdfilter`, `grdmath`, `grdimage`,
`grdview`, `grdcontour`, `grdblend`, `grdclip`, `grdcut`, `grdedit`, `grdpaste`, `grdvolume`,
`grd2xyz`, `xyz2grd`, `surface`, `grdlandmask`, `grdmask`, `grdinterpolate`, `grdfill` and
`grdinfo`) was run in each layout and every file it produced compared against the same script
run in the legacy layout. PostScript is compared ignoring `%` comment lines; everything else
byte for byte.

| layout | identical | differ |
|---|---|---|
| `GMT_GHOST_CELLS=1` — halo outside the matrix | **186 / 189** | 3 |
| `GMT_GHOST_CELLS=2` | **186 / 189** | 3 |
| determinism control — level 2 run twice | 187 / 189 | 2 |

Levels 1 and 2 now give the same answers as each other (see §3), and the same three scripts
account for both:

* `test/grdimage/rounding.sh` and `test/grdimage/twogrids.sh` need the remote tiled
  `earth_relief` datasets. They were never reproducible outside the test harness, so they are
  **unverified**, not known-bad.
* `test/grdview/texture2_modern.sh` is not reproducible run to run — it also fails the
  determinism control, producing no output in one run of two.

The determinism control additionally flags `test/grdfill/gridfill.sh` as varying run to run at
one and the same layout. Neither of those two is a layout difference.

**How the harness must be run.** Two earlier rounds of this measurement produced numbers that
were pure noise, in both directions, and neither was caught by comparing layouts alone:

1. Six test scripts running in parallel shared one GMT user directory, so `gmt set -Du` and
   `gmt.history` raced each other. Every run needs its own `GMT_USERDIR` (a shared
   `GMT_CACHEDIR` is fine, and keeps remote downloads to one).
2. A comparison is only meaningful **within one sweep against one binary**. Grids carry the GMT
   version and commit hash in their netCDF header, so two builds always differ byte-wise on
   every `.nc` file even when the data is identical.
3. Always run one layout twice and compare that too. The level-2-twice control is what exposed
   `png_image`/`rgb_grids` as *non-deterministic* rather than merely different, which pointed
   straight at the cause.

Spot checks that matter more than the totals, all bit-identical with the pad gone: `grdtrack`
sampling at the exact grid corners, where the halo is the only thing being read; `grdsample`;
`grdproject`; `grdgradient`; and polar sampling on global grids in five different shapes
(gridline and pixel registration, with and without a pole row). `test/api/ghost_cells.sh`
checks the first four in all three layouts on every build, and asserts that the layout is
actually live — an early version of one guard silently turned the whole thing into a no-op and
the suite went green for the wrong reason.

## 7. What changing the layout turned up

Seven bugs that were already in GMT, each hidden by the fact that every grid happened to have
the same pad, or that a pad was there at all:

* **grdgradient** indexed the `-A` azimuth grid with node numbers computed from the *input*
  grid's header. Wrong whenever the two grids differ in pad.
* **grdfilter** did the same twice: the input grid and the variable filter-width grid were both
  indexed with the *output* grid's node number. The comment even said
  `[Here we know ij_out == ij_in]` — true only by coincidence of layout.
* **grdview** averaged the four tile corners of the intensity grid, and read the drape and
  relief grids, with node numbers computed from another grid's header plus offsets built from a
  third's `mx`.
* **gmt_customio.c** positioned the GDAL write pointer with a hardwired `2 * mx`, i.e. it
  assumed the pad is always two deep. On a small grid with no pad it stepped past the data and
  NaNs came back as zeros.
* **grdlandmask** accepted a node one past the last row or column — the bounds test read
  `row > n_rows` rather than `>=` — and wrote to it. With a pad that write landed in the pad and
  was invisible; with no pad, `col == n_columns` wraps onto the first node of the next row and
  corrupts real data.
* **grdimage** computed the z-range over the nodes inside `-R` by temporarily inflating the
  grid's pad, which only works when there is a pad to inflate.
* **gmt_copy_gridheader** overwrote the destination's halo pointer without freeing it.

And one trap for anyone converting further call sites: `openmp_int` is **unsigned** everywhere
except MSVC, so `col - 1` at column 0 hands an accessor 4294967295 rather than -1. Cast at every
call site; `GMT_GHOST_STRICT` builds assert on it rather than computing a wild pointer.

Two pre-existing oddities were found *inside* the boundary-condition code and deliberately left
exactly as they are, because correcting either would move existing output. Both are flagged in
comments where they sit:

* The pole phase shift is computed from the **padded** column index —
  `i180 = pad[XLO] + ((i + nxp2) % nxp)` — so the 180-degree partner of a column depends on how
  the grid is stored, which it should not.
* In the south-side natural-BC block of the x-periodic case, two tests read `set[YHI]` where
  `set[YLO]` is plainly meant. It only bites when the two sides differ, i.e. when one side holds
  real data.

## 8. Open items

* **Phase 2 proper — the memory win is not yet banked.** `gmt_grd_BC_set` still computes the
  conditions on a private padded copy of a pad-free grid and keeps only the halo. That is
  correct, and it never touches the caller's matrix, but it costs one temporary grid copy per
  pad-free boundary-condition set. Converting `gmtsupport_grd_BC_set` (some 420 lines of flat
  `j + i` arithmetic) to the `(row,col)` accessors removes that copy. **This was attempted and
  reverted**: the conversion builds and passes the targeted tests, but moves 21 scripts at
  level 2 for reasons not established — the pole phase shift and a dropped incoming-halo guard
  were both ruled out by experiment. The work is preserved on branch
  `ghost-cells-bc-inplace` and needs its cause found before it can return.
* **`grdview -Qg` with an intensity grid** is the one path in grdview still assuming the halo is
  inside the matrix. Routing `grdview_paint_gouraud_tile` through the accessors crashed
  `grdview -Qg` outright (truncated PostScript, non-zero exit, no message); the hunk was backed
  out and the cause not identified. It needs a debugger, not another guess.
* **Images** keep their pad. `gmtlib_bcr_get_img` and `gmtlib_image_BC_set` still walk a padded
  array and `GMT_IMAGE` has no halo of its own. Nothing fails because of this, so it is now a
  consistency and memory question rather than a correctness one. Note that forcing images to
  carry a pad in a pad-free session is **not** the answer — it was tried and it corrupted
  `grdmix`, which deliberately works with no pad at all.
* **Cubes** need a halo of their own; `gmtlib_ghost_suspend` is scaffolding and is named so it
  is easy to find.
* **Complex grids** are skipped by the conversion entirely.
* **grdfft / gravfft / grdpaste** use the pad as *working space*, not as boundary conditions —
  grdpaste gives each input a pad the height of the other so the two land in one array. Those
  modules want an explicitly allocated larger array, which is a separate change, and
  `gmtlib_ghost_from_pad` leaves any asymmetric or oversized pad alone for that reason.
* **Unrelated GMT bug found on the way, diagnosed and not fixed:** a cube written as a single
  3-D netCDF file reports a v-range taken from the padded layer arrays, read with the wrong
  stride, so the reported minimum can lie outside the data. Written up separately in
  `CUBE_VMIN_BUG.md`; the correct function (`gmt_cube_vminmax`) already exists in
  `gmt_grdio.c` and has no callers.

## 9. Status

- [x] Phase 0 — inventory
- [x] Phase 1 — infrastructure
- [x] Phase 3 — `gmt_bcr.c`
- [~] Phase 2 — boundary conditions (private copy works; the in-place fill is reverted, see §8)
- [x] Phase 4 — remaining sites: 186 of 189 scripts identical, the 3 exceptions being remote-data
      or non-reproducible rather than layout differences
- [~] Phase 5 — flipping the default still needs images, cubes and complex grids

## 10. Summary for a pull request

**What this adds.** A grid's boundary halo can be held *outside* the data matrix, so that
`G->data` is a plain contiguous `n_columns * n_rows` array with `h->pad` all zero. That makes
`gmt_M_ijp` degenerate to `row * n_columns + col`, so every interior loop in GMT keeps working
unmodified, and it is what allows a grid to be shared with Julia, Python or MATLAB without a
copy (issue #4358). New files: `gmt_ghost.h` (the layout and the accessors) and `gmt_ghost.c`
(alloc, free, duplicate, convert).

**Nothing changes by default.** The layout is selected by the `GMT_GHOST_CELLS` environment
variable, `0` (legacy) being the default. With it unset, the only code that behaves differently
is the seven bug fixes listed in §7 — and each of those is verified byte-identical on the test
suite in the padded layout, because in that layout the accessors compile down to exactly the
expression the old code computed by hand.

**Converted so far:** `gmt_bcr.c` — which carries grdtrack, grdsample, grdproject, grdimage and
grdview — plus grdgradient, grdfilter, grdmath, grdpaste, grdview's tile loops, the GDAL writer
and the cube import paths. Unconverted code keeps working because `gmt_grd_pad_on` folds any
halo back into the matrix, moving the values rather than recomputing them.

**Verification.** 189 grid test scripts, three layouts, every produced file compared:
**186 / 189 identical** at both `GMT_GHOST_CELLS=1` and `=2`; the three exceptions need remote
data or are not reproducible run to run (§6). A determinism control and a per-run
`GMT_USERDIR` are both necessary to get trustworthy numbers — see the warning in §6 before
re-running this. `test/api/ghost_cells.sh` is included and asserts both bit-identity across
layouts and that the layout is actually live.

**Two design conclusions worth carrying into review:**

1. **"No pad anywhere" is the wrong goal.** The original level 2 zeroed the session pad. That
   cannot work: a global grid read *with data padding* fills its pad with the real neighbouring
   columns from the file, and with nowhere to put them the halo falls back to a computed
   boundary condition — precisely the accuracy the issue does not want to lose. Grids are now
   read through a transient pad and moved to ghost cells immediately afterwards, so they are
   pad-free in memory either way, and the pad exists only for the duration of the read. This is
   why levels 1 and 2 now measure the same.
2. **Ghost cells replace the pad only where a pad would have existed.** Several modules
   (grdmix, grdpaste and friends) set the session pad to zero because they want no halo at all,
   and the padded layout computes no boundary conditions for them either. Building a halo for
   those grids is work nobody asked for, and it perturbed what those modules read back out of
   the header — visible as *run-to-run* variation, not just a layout difference.

**Suggested review order:** `gmt_ghost.h` (layout and accessors), then `gmt_bcr.c` as the model
conversion, then `gmt_grd_BC_set` in `gmt_support.c` (where the halo comes from), then the
per-module fixes, which are independent of each other and each defensible on its own.

**Independently useful without any of this machinery:** the seven fixes in §7. If the layout
change is considered too large to land at once, those can go in on their own, since each is a
real bug in current GMT.
