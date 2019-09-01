# TODO

## High-Priority

- [x] Tilemap rendering
- [ ] Maintain track of current bound texture, only switch when needed (texture handles instead?)
- [ ] Maintain track of current blend factors, only switch when needed (or only switch if changed, then switch back?)

## Normal-Priority

- [ ] Use VBO for all model matrices, use glBufferSubData
- [ ] Remove all the redundant "unbindings" of buffers and vertex arrays
- [ ] Remove NULL checks in every API call
- [ ] Use callback driven error reporting instead of return value?

## Before-Release

- [ ] Documentation
- [ ] Format, code cleanup
- [ ] Double-check naming conventions
