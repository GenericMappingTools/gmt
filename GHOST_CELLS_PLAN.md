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
| 2 | **Boundary conditions fill the halo directly**, instead of filling a pad that is then moved. | **partial** — a borrowed pad makes it correct; the native fill is still to write |
| 4 | **Remaining sites**, one at a time, from the inventory. | in progress — 24 test scripts still differ |
| 5 | **Flip the defaults.** No pad anywhere, in or out of the API. | reachable at `GMT_GHOST_CELLS=2` today |

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
GMT_GHOST_CELLS=2   as 1, and no grid in the session is created with a pad
```

Level 1 is the interesting number for "does the halo still work". Level 2 is
the issue's own experiment — `GMT_Create_Session` with pad 0 — and is what
externals would actually get.

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

Every grid-using test script in `test/` that needs no remote data (257 of them)
was run in each layout and every file it produced compared against the same
script run in the legacy layout. PostScript is compared ignoring `%` comment
lines; everything else byte for byte.

| layout | identical | differ | real failures |
|---|---|---|---|
| `GMT_GHOST_CELLS=1` — halo outside the matrix | **256–257** | 0–1 | **0** |
| `GMT_GHOST_CELLS=2` — and no pad created anywhere | 245 | 12 | 11 |

Level 1 is complete: with the pad gone from every grid that carries a boundary
halo, the whole suite is byte-identical. The one script that sometimes shows up
is `test/grdtrack/crosstrack_geo.sh`, which is not reproducible run to run in
*any* layout — established by running every script twice in the legacy layout —
so it is subtracted rather than counted, in both rows.

Level 2 — `GMT_Create_Session` with pad 0, the experiment the issue proposes —
is at 245 of 257, with 11 real failures. They are concentrated in the two areas
this work has not entered: cubes and images.

Spot checks that matter more than the totals, all bit-identical with the pad
gone: `grdtrack` sampling at the exact grid corners, where the halo is the only
thing being read; `grdsample`; `grdproject`; and every `grdgradient` test,
which is the module that reads the halo hardest. Poisoning the pad with a
sentinel still shows up in the ghost-layout answer, which proves the halo is
genuinely what those samplers read rather than a leftover pad.

## 7. What changing the layout turned up

Four bugs that were already in GMT, hidden by the fact that every grid happened
to have the same pad:

* **grdgradient** indexed the `-A` azimuth grid with node numbers computed from
  the *input* grid's header. Wrong whenever the two grids differ in pad.
* **grdfilter** did the same twice: the input grid and the variable filter-width
  grid were both indexed with the *output* grid's node number. The comment even
  said `[Here we know ij_out == ij_in]` — true only by coincidence of layout.
* **gmt_customio.c** positioned the GDAL write pointer with a hardwired
  `2 * mx`, i.e. it assumed the pad is always two deep. On a small grid with no
  pad it stepped past the data and NaNs came back as zeros.

And one trap for anyone converting further call sites: `openmp_int` is
**unsigned** everywhere except MSVC, so `col - 1` at column 0 hands an accessor
4294967295 rather than -1. Cast at every call site; `GMT_GHOST_STRICT` builds
assert on it rather than computing a wild pointer.

## 8. Open items

* **Level 2 only (11 scripts).** These pass at level 1, so the halo is not the
  problem; grids *created* without a pad by code that expects one are.
  - **Cubes (4).** Only the written cube's `v_min` moves; every plot made from
    the cube is identical, so the data agree and it is the reported range that
    differs. The legacy value is negative for a field whose true minimum is
    ~0.03, which suggests the padded run is picking up a boundary-condition node
    — worth a look on its own terms, since if so it is a bug that predates this
    work.
  - **grdimage (4)** and **grdmask, sph** — these run through grdmix and the
    image machinery, which still keeps its pad, so grids and images disagree
    about layout.
  - **surface/periodic** — the grids it writes are now identical; only the plot
    made from them differs.
* **Phase 2 proper** — `gmt_grd_BC_set` still borrows a pad instead of filling
  the halo slabs directly. That is the remaining memory win.
* **Cubes** need a halo of their own; `gmtlib_ghost_suspend` is scaffolding and
  is named so it is easy to find.
* **Images** keep their pad; `gmtlib_bcr_get_img` still walks the padded array.
* **Complex grids** are skipped by the conversion entirely.
* **grdfft / gravfft / grdpaste** use the pad as *working space*, not as boundary
  conditions — grdpaste gives each input a pad the height of the other so the two
  land in one array. `gmtlib_ghost_from_pad` leaves any asymmetric or oversized
  pad alone for that reason. Those modules want an explicitly allocated larger
  array, which is a separate change.

## 9. Status

- [x] Phase 0 — inventory
- [x] Phase 1 — infrastructure
- [x] Phase 3 — gmt_bcr.c
- [~] Phase 2 — boundary conditions (borrowed pad works; native fill pending)
- [x] Phase 4 — level 1 clean: no real failures in 257 test scripts
- [~] Phase 5 — level 2 at 245 of 257, 11 real failures
