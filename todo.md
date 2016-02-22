## Windows/Linux coding notes
### dllexport / dllimport
When making a DLL on Windows, the public functions and classes need to be marked with dllexport.

Apparently in GCC, you can just use 'declspec(dllexport)', instead of attribute (dllexport)),
and it will do the right thing.  When it finds a dllexport, it also sets the visibility to
"default".

Note that on GCC, all inlined functions are dllexport by default, so it is best not to
inline any functions in a DLL to be built with GCC.

### 

## TODO
### Environment Variables
Vanilla dash has a hand-coded singly-linked list to store the environment variables.
It has a manual memory management scheme.

POSIX expects the environ variables names to be in a 7-bit portable character set,
with the values in 8-bit-clean character set of unspecified encoding.

Windows has environment variables encoded in wchar_t.

Plan: store env vars in a map<wstring,...>.  For POSIX, convert
locale encoding to wchar_t using codecvt.
