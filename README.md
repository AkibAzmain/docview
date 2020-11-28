# Docview

Docview is an extensible documentation viewer which can show documentation in a variety of formats.

Docview doesn't support any formats out of the box. You'll need to use extensions to use Docview. You can also write your own extension very easily (supported languages are C and C++).

## Writing own extensions

To extend Docview, you need extensions. libdocview is the backend of Docview. It manages all resources and extensions. To write extensions, you need to know how they work. When a program tells libdocview to parse documentations in a path, libdocview tells an extension to parse that. Then the extension tries to parse that and returns a `nullptr` or `NULL` on failure. On success, it returns a pointer to a document node, which holds a document tree. A document node holds it's title, synonyms of title, pointer to parent node (`nullptr` or `NULL` if the node has no parent), an array of pointers to it's children. See [docview.hpp](blob/master/src/libdocview/docview.hpp) and [docview.h](blob/master/src/libdocview/docview.h) for more, which are very documented headers.

## Notes

Docview is licensed under GNU GPLv3 (which can be found in the [LICENSE](blob/master/LICENSE) file), but the supporting library libdocview is licensed under GNU LGPLv3 (which can found at [src/libdocview/LICENSE](blob/master/src/libdocview/LICENSE)). All files under directory [src/libdocview](blob/master/src/libdocview) are part of libdocview and and license under LGPLv3. So feel free to link with libdocview dynamically.
