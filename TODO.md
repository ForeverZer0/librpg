# TODO

## High-Priority

- [x] Tilemap rendering
- [x] Only render visible tilemap, or draw instanced
- [x] Maintain track of current bound texture, only switch when needed (texture handles instead?)
- [x] Maintain track of current blend factors, only switch when needed (or only switch if changed, then switch back?)
- [ ] Multiple tilesets for each layer?
- [ ] Use MSAA in viewport/image FBOs
- [ ] Fix inverted viewport projection

## Normal-Priority

- [ ] Remove all the redundant "unbindings" of buffers and vertex arrays
- [ ] Remove NULL checks in every API call
- [ ] Use callback driven error reporting instead of return value?

## Before-Release

- [ ] Documentation
- [ ] Format, code cleanup
- [ ] Double-check naming conventions
