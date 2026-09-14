# Buffer Storage and Updates

Study notes on the modern, canonical OpenGL 4.6 core model for buffer objects: immutable
storage, its flags, ranged content updates, mapping, and CPU-side data layout.

Scope: the immutable-storage (`glNamedBufferStorage`) model and Direct State Access (DSA)
operations only. Every operation below addresses the buffer by handle; no binding is involved.

## 1. Immutable storage

A buffer object is created with `glCreateBuffers` and its data store is established once with:

```
glNamedBufferStorage( buffer, size, data, flags )
```

- The store `size` is fixed at creation and never changes.
- `data` initializes the store; pass `nullptr` to leave it uninitialized.
- `flags` declares, at creation time, how the store may be accessed for the rest of its life.
- One buffer, one handle (`GLuint`).

Because the size never changes, holding a different number of elements means creating new
storage and deleting the old buffer.

## 2. Storage flags

- **`GL_DYNAMIC_STORAGE_BIT`** — permits the store contents to be rewritten after creation,
  through the ranged-update and copy commands of section 3. Without it the content is
  write-once: only the initial `data` ever lives in the store.
- **`GL_MAP_READ_BIT`** — permits the store to be mapped for CPU reading.
- **`GL_MAP_WRITE_BIT`** — permits the store to be mapped for CPU writing.
- **`GL_MAP_PERSISTENT_BIT`** — a mapping may remain active while the store is still used by
  drawing or compute. Requires `GL_MAP_READ_BIT` and/or `GL_MAP_WRITE_BIT`.
- **`GL_MAP_COHERENT_BIT`** — a persistent mapping is automatically coherent: CPU writes
  become visible to the GPU (and vice versa) without explicit flushing or barriers. Requires
  `GL_MAP_PERSISTENT_BIT`.
- **`GL_CLIENT_STORAGE_BIT`** — hint that the store should reside in client (CPU) memory
  rather than server (GPU) memory; intended for stores the CPU touches heavily.

Dependencies:

- Persistent mapping requires at least one mapping direction.
- Coherent mapping requires persistent mapping.
- Mapping at all requires the matching `GL_MAP_READ_BIT` / `GL_MAP_WRITE_BIT` at creation.

## 3. Ranged content updates

With `GL_DYNAMIC_STORAGE_BIT` set:

```
glNamedBufferSubData( buffer, offset, size, data )
```

replaces `size` bytes starting at byte `offset`. Bytes outside `[offset, offset + size)` are
untouched. The cost is proportional to the updated range, not to the whole store. This is the
canonical way to update a buffer's content.

Companions:

- `glClearNamedBufferSubData` — fill a range with a constant.
- `glCopyNamedBufferSubData` — copy a range between buffers.

## 4. One store for many objects

Lay the objects contiguously and address each by byte offset:

```
offset( i ) = i * stride          stride = byte size of one object
```

- Update object `i`: `glNamedBufferSubData( buffer, i * stride, stride, data )`. Only that
  object's bytes are transferred; the other objects are untouched.
- Draw object `i`: use the matching offset / first-vertex argument of the draw call
  (`glDrawArrays( mode, i * vertex_count, vertex_count )`, or an equivalent range).

The store must be sized up front for the maximum object count. Growing past that count requires
new storage.

## 5. Mapping

An alternative to ranged updates is to map the store into the CPU address space and write
directly:

```
glMapNamedBufferRange( buffer, offset, length, access )
...
glUnmapNamedBuffer( buffer )
```

Access flags passed to `glMapNamedBufferRange`:

- **`GL_MAP_READ_BIT` / `GL_MAP_WRITE_BIT`** — requested mapping direction; must be allowed by
  the creation flags.
- **`GL_MAP_INVALIDATE_RANGE_BIT`** — discard the previous contents of the mapped range.
- **`GL_MAP_INVALIDATE_BUFFER_BIT`** — discard the previous contents of the entire store.
- **`GL_MAP_FLUSH_EXPLICIT_BIT`** — CPU writes are not flushed automatically; call
  `glFlushMappedNamedBufferRange` on each written region before the GPU may read it.
- **`GL_MAP_UNSYNCHRONIZED_BIT`** — the caller promises no overlap with in-flight GPU
  accesses; no synchronization is performed.
- **`GL_MAP_PERSISTENT_BIT` / `GL_MAP_COHERENT_BIT`** — must have been set on the store.

Persistent-mapped buffers keep the map valid across draws, which suits continuous rewriting.
GPU reads are asynchronous, so without coordination the GPU may read a region while the CPU is
writing it; coherent mapping removes the explicit flush but not the need to schedule writes so
they do not race in-flight rendering (a ring of buffers, or fencing between writes and draws).

## 6. Interleaved (array-of-struct) vs split (struct-of-array)

**Array of struct** — all attributes of a vertex adjacent:

```
struct vertex { position; color; };
array< vertex > vertices;
```

An object's data is contiguous, so updating the whole object is a single ranged update, and
vertex fetch has good cache locality.

**Struct of array** — one contiguous region per attribute:

```
struct mesh { array< position > positions; array< color > colors; };
```

Updating only one attribute across objects is a single ranged update over that attribute's
region, with no bytes moved for the others.

Selection:

- Update all attributes of an object together → array of struct.
- Update one attribute across many objects while the others stay stable → struct of array.
- For GPU vertex fetch, the interleaved layout is generally preferred.

## 7. Selecting an update strategy

- **Occasional, input-driven changes** → `GL_DYNAMIC_STORAGE_BIT` plus
  `glNamedBufferSubData`, transferring only the changed range.
- **Continuous rewrite of large ranges** → consider mapping. Persistent mapping with explicit
  flushing (and a ring of buffers), or coherent mapping when measured acceptable, avoids
  repeated map/unmap and driver copies.
- **Many objects** → a single store addressed by offsets, or a few stores, reduces per-object
  overhead; the choice between array-of-struct and struct-of-array follows section 6.
