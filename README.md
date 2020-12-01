# Docview

Docview is an extensible documentation viewer which can show documentation in a
variety of formats. It is extensible with extensions, so it's possible to view
almost any type of documentation in a single application.

## Why use Docview

As Docview is extensible, it isn't limited to only a few number of formats like
most documentation viewer. With the help of extensions, it can show almost any
type of documentation. So it isn't required to wait for new releases to add
support for a particular format, anyone can write an extension adding support
for that format. So you don't need switch from one documentation viewer to
another (e.g Terminal for man and info pages, Devhelp for GNOME documentations),
Docview is an all-in-one solution.

### Comparison with other documentation viewers

There are many popular documentation viewer available (both free and paid). The
advantage of Docview is it's ability to be extended which is an unavailable
feature of most other documentation viewer. The following is a comparison of
Docview (with proper extensions enabled) some other documentation viewers (not
to ):

 Feature | Docview | man | Devhelp
---------|---------|-----|---------
Extensible | Yes | No  | No
Supports man pages | Yes | Yes | No
Supports Devhelp documentations | Yes | No | Yes

## Writing own extensions

To extend Docview, you need extensions. libdocview is the backend of Docview. It
manages all resources and extensions. You can see a list of extensions
[here](extensions.md) (not complete).

To write extensions, you need to know how they work. When a program tells
libdocview to parse documentations in a path, libdocview tells an extension to
parse that. Then the extension tries to parse that and returns a `nullptr` or
`NULL` on failure. On success, it returns a pointer to a document node, which
holds a document tree. A document node holds it's title, synonyms of title,
pointer to parent node (`nullptr` or `NULL` if the node has no parent), an
array of pointers to it's children. Afterwards, the program calls a function
with that or any of it's child (or grandchild and even deeper) node to do an
operation (e.g. get the document, get the brief, get a section). See
[docview.hpp](src/libdocview/docview.hpp) and
[docview.h](src/libdocview/docview.h) for language specific information, which
are very documented headers.

After you've written your very own extension, you can share it here.

## Notes

Docview is licensed under GNU GPLv3 (which can be found in the
[LICENSE](blob/master/LICENSE) file), but the supporting library libdocview is
licensed under GNU LGPLv3 (which can found at
[src/libdocview/LICENSE](blob/master/src/libdocview/LICENSE)). All files under
directory [src/libdocview](blob/master/src/libdocview) are part of libdocview
and license under LGPLv3. So feel free to link with libdocview dynamically.
