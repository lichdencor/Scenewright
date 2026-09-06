set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_PROVIDED_FORTRAN ON)

# Distinct triplet name (per docs/adr/0010) so vcpkg builds its own copy of
# every dependency with /fsanitize=address instead of reusing the regular
# x64-windows-static-md binary cache — MSVC's ASan container annotations
# (annotate_string/annotate_vector) are ABI-incompatible between binaries
# built with and without this flag, and mixing them is a link error
# (LNK2038), not a runtime one.
set(VCPKG_CXX_FLAGS "/fsanitize=address")
set(VCPKG_C_FLAGS "/fsanitize=address")
