# Data Formats

## Tensor file format

This is the binary format used on disk to store a tensor. It's what Python produces and what the simulator reads.

```
order
shape[0]
shape[1]
...
shape[order-1]
number of entries total
entries[0]
entries[1]
...
entries[num-1]
number of fiber pointers + 1
fiber_starts[0]
fiber_starts[1]
...
fiber_starts[num-1]
end of last fiber + 1
```

These are all `pointer_type` (32-bit unsigned int) except for the entries, which are each a 32-bit unsigned int (index) followed by a float (which is also 32-bit).

Note that the pointers for the starts of fibers begin at 0 since they index into the `entries` array. Therefore, the address in main memory of the first fiber start is added to all of these pointers when they're loaded into metadata memory.

This also creates a dependency where the fiber entries must be loaded first so that we know where the fiber pointers must point to, particularly if we ever needed to support a fragmented situation where fibers are stored in main memory non-contiguously. Therefore, to simplify the loading procedure so the file can be read linearly, the entries are stored on disk before the pointers to them.

## Tensor accelerator in-memory format

This is the format of a tensor once it has been loaded into the accelerator. The accelerator has two memory banks: the actual tensor elements (as pairs of fiber index and value) and the metadata storage for pointing to ranges of these elements (each range is a fiber).

The format in main storage is just all of the fibers concatenated into one long list. That's it.

Here's the format in metadata storage:

```
order (the tensor pointer points here)
shape[0]
shape[1]
...
shape[order-1]
pointer to fiber[0,0,:] in main mem
pointer to fiber[0,1,:] in main mem
...
pointer to fiber[1,0,:] in main mem
pointer to fiber[1,1,:] in main mem
...
pointer to end of last fiber + 1
```

We need the last pointer to point right after the end of the last fiber since processing elements receive memory ranges to compute on, and these memory ranges are specified with a pointer to the inclusive start element and the exclusive end element.
