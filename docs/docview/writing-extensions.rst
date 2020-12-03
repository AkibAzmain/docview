.. _writing-extensions:

Writing extensions
==================

Docview is an **extensible** documentation viewer. And it uses extensions which
are responsible parsing documents into trees shown in Docview's sidebar.

Theory behind extensions
------------------------

The theory behind extensions are simple. An extension adds support for a (or
several format) format to Docview. Docview make API calls to libdocview, and
libdocview calls extension to parse documents. On success, the extension returns
a document tree, which is made of document nodes. On failure, it does nothing
but notifies libdocview about the incident somehow, depending on the language
being used. After parse documents into document tree, the program requests the
actual document pointed by a node. libdocview redirects the request to the
extension that owns the node and the extension returns the content or URI of
document.

A document tree::

                --------
                | Root |
                --------
               /        \
        -----------   -----------
        | Child 1 |   | Child 2 |
        -----------   -----------
          /     \       /     \
        ....   ....   ....   ....

Results in Docview sidebar::

        v Root
        |-- v Child 1
        |   |-- ....
        |   `-- ....
        `-- v Child 2
            |-- ....
            `-- ....

Every extension must have two things. They are:

* Ability to parse documents
* Applicability level of the extension


What is a document node?
++++++++++++++++++++++++

A document node is an object or structure holding some information about the
document it points and the document tree it is part of. A node holds four data,
these are:

* Title of the document it points
* Alternative search queries (aka synonyms)
* Reference to it's parent node
* Reference to all children nodes of the node


.. _what-is-applicability-level:

What is applicability level?
++++++++++++++++++++++++++++

Applicability level is a value returned by a dedicated extension function (or
similar depending on language). Applicability level has some fixed possible
values, describing the format parsed by the extension is how much frequently
used. The value return by this method will be used to determine which extension
should be called before by libdocview.


Writing an extension
--------------------

To write extensions, you must choose a language which is supported (directly or
indirectly) by libdocview. libdocview supports C++ (C++17 and above) and C
languages directly.


Writing extension in C++
++++++++++++++++++++++++

C++ is an general purpose object-oriented language. So it's very easy to write
an extension in C++. This tutorial assumes that the reader much knowledge about
C++.

In accordance with the ancient tradition of computer science, introducing a
hello world extension for Docview::

    #include <docview.hpp>

    class hello : public docview::extension
    {
    public:
        applicability_level get_applicability_level() noexcept
        {
            return applicability_level::tiny;
        }

        const docview::doc_tree_node* get_doc_tree(std::filesystem::path path) noexcept
        {
            root = new docview::doc_tree_node;
            root->title = "Hello World!";
            root->parent = nullptr;
            return root;
        }

        std::pair<std::string, bool> get_doc(const docview::doc_tree_node* node) noexcept
        {
            return std::make_pair("file:///path/to/your/doc.html", true);
        }
    };

    extern "C"
    {
        hello extension_object;
    }

Let's explain what's going on here. The program starts with::

    #include <docview.hpp>

``docview.hpp`` is the C++ header file of libdocview. Then::

    class hello : public docview::extension

We declare our class ``hello``, which inherits ``docview::extension``.
``docview::extension`` is the base class for all extension classes. Class
``hello`` will contain all required symbols (e.g. methods, variables) as an
extension. Further ahead::

    applicability_level get_applicability_level() noexcept
    {
        return applicability_level::tiny;
    }

Every extension must define the method ``get_applicability_level``. It would
return a enumerator named ``applicability_level``. This enumerator contains
several member (e.g. tiny, small, medium, big, huge). See
":ref:`what-is-applicability-level`" for more. In this example, it returns
``applicability_level::tiny``. The next method::

    const docview::doc_tree_node* get_doc_tree(std::filesystem::path path) noexcept
    {
        root = new docview::doc_tree_node;
        root->title = "Hello World!";
        root->parent = nullptr;
        return root;
    }

This method also must be implemented. This method's task is to parse a document
into a document tree. It returns a pointer to the root node of an document tree
on success, and a ``nullptr`` on failure or unsupported format. This method must
not throw exceptions. In this example, it creates a new root node, whose title
"Hello World!" and parent is ``nullptr``, meaning it's the root. The last
method::

    std::pair<std::string, bool> get_doc(const docview::doc_tree_node* node) noexcept
    {
        return std::make_pair("file:///path/to/your/doc.html", true);
    }

This method must be implemented too. This function returns the document contains
in html or URI specified by parameter ``node``. This method must not throw
exceptions too. It returns a pair of ``std::string`` and ``bool``. If the
``bool`` true, the value of ``std::string`` is a URI, otherwise it's plain HTML.
Finally::

    extern "C"
    {
        hello extension_object;
    }

It's the most important one. libdocview accesses the extension with this global
object. Without this, all the above things are are useless. It's inside
``extern "C"`` to prevent name mangling.


Writing extension in C
++++++++++++++++++++++

C is a general purpose procedural language. So you need to use functions and
structures to build an extension.

In accordance with the ancient tradition of computer science, introducing a
hello world extension for Docview::

    #include <docview.h>
    #include <stdlib.h> // For malloc

    enum applicability_level get_applicability_level()
    {
        return applicability_level::tiny;
    }

    const struct docview_extension_doc_tree_node* get_doc_tree(const char* path)
    {
        root = malloc(sizeof(docview_extension_doc_tree_node));
        root->title = "Hello World!";
        root->parent = NULL;
        root->synonyms = malloc(sizeof(struct docview_extension_doc_tree_node*));
        *root->synonyms = NULL;
        root->children = malloc(sizeof(struct docview_extension_doc_tree_node*));
        *root->children = NULL;
        return root;
    }

    struct docview_document get_doc(const struct docview_extension_doc_tree_node* node)
    {
        return {"file:///path/to/your/doc.html", true};
    }

    struct docview_extension_functions extension_functions =
    {
        get_applicability_level,
        get_doc_tree,
        get_doc
    };

Let's see what's going on here. The very first line::

    #include <docview.h>

``docview.h`` is the C header of libdocview. In the next line, ``stdlib.h`` is
included for ``malloc``. Then::

    enum applicability_level get_applicability_level()
    {
        return applicability_level::tiny;
    }

Every extension must define this function. This function return the
applicability level of the extension. See ":ref:`what-is-applicability-level`"
for more. Then::

    const struct docview_extension_doc_tree_node* get_doc_tree(const char* path)
    {
        root = malloc(sizeof(docview_extension_doc_tree_node));
        root->title = "Hello World!";
        root->parent = NULL;
        root->synonyms = malloc(sizeof(struct docview_extension_doc_tree_node*));
        *root->synonyms = NULL;
        root->children = malloc(sizeof(struct docview_extension_doc_tree_node*));
        *root->children = NULL;
        return root;
    }

This function also must be implemented. In this example, it creates the root
node, whose title is "Hello World!". Then it sets it's parent to NULL, meaning
it's the root. As this is a simple example, we won't set any synonyms, but as
array is NULL-terminated, we set it's first element to NULL. We're not going to
have children of this node, so set the first element as NULL, as this array is
NULL-terminated too. Further ahead::

    struct docview_document get_doc(const struct docview_extension_doc_tree_node* node)
    {
        return {"file:///path/to/your/doc.html", true};
    }

This function is mandatory too. This function returns the document contains in
html or URI specified by parameter ``node``. This method must not throw
exceptions too. It returns a structure of ``const char*`` and ``bool``. If the
``bool`` true, the value of ``const char*`` is a URI, otherwise it's plain HTML.
Finally::

    struct docview_extension_functions extension_functions =
    {
        get_applicability_level,
        get_doc_tree,
        get_doc
    };

It's the most important one. libdocview will access extension function via this
structure. Without this, all the above functions are useless.


See also
--------

* :ref:`libdocview`
* :ref:`libdocview-cpp-api`
* :ref:`libdocview-c-api`
