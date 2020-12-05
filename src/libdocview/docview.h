/*
    Copyright (C) 2020 Akib Azmain

    This file is part of libdocview.

    libdocview is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libdocview is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with libdocview.  If not, see <http://www.gnu.org/licenses/>.

*/

/**
 * @file docview.h
 * @details @rst
 * 
 * ``docview.h`` is the C header of libdocview. It provides all functions,
 * structures, enumerations required to interact with Docview. It mainly
 * consists of two parts. One part is for applications like documentation viewer
 * or browsers, IDEs for handling documentations. Another part is for
 * extensions. The first part provides functions required to load and unload
 * extensions, parse documents into :ref:`document trees <document-tree>` and
 * access them. The second part is extension related. It defines the most
 * important structure for extensions written in C,
 * :cpp:class:`docview_extension_functions`, which holds all required function
 * pointers of an extension. All symbols defined in this file is prefixed with
 * ``docview_``.
 * 
 * Include this file with::
 * 
 *      #include <docview.h>
 * 
 * @endrst
 * 
 */

#ifndef _DOCVIEW_C_H
#define _DOCVIEW_C_H

// Include stdbool.h to use bool
#ifndef __cplusplus
#   include <stdbool.h>
#endif

// Include stddef.h for NULL
#include <stddef.h>

// If it's being compiled by a C++ compiler, disable name mangling
#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Loads an extension from given path
 * 
 * @details @rst
 * 
 * This function loads an extension from given path. If extension is already
 * loaded, there is no effects. If extension loading fails, ``false`` is
 * returned.
 * 
 * @endrst
 * 
 * @param path path to extension
 * @return true on success, false on failure
 */
bool docview_load_ext(const char* path);

/**
 * @brief Unloads extension with given path
 * 
 * @details @rst
 * 
 * This function unloads extension with given path. If extension isn't
 * loaded, there is no effects.
 * 
 * @endrst
 * 
 * @param path path to extension
 */
void docview_unload_ext(const char* path);

/**
 * @brief Check whether extension at given path is loaded
 * 
 * @details @rst
 * 
 * This function checks whether an extension at given path is located.
 * 
 * @endrst
 * 
 * @param path path to extension
 * @return whether extension at given path is loaded
 */
bool docview_ext_is_loaded(std::filesystem::path path);

/**
 * @brief Structure for holding a document node
 * 
 * @details @rst
 * 
 * This structure holds all information about a document node. For more, see
 * ":ref:`document-node`".
 * 
 * @endrst
 * 
 */
struct docview_extension_doc_tree_node
{

    /**
     * @brief Pointer to parent, NULL if there is no parent
     * 
     */
    const docview_extension_doc_tree_node* parent = NULL;

    /**
     * @brief The title of document
     * 
     */
    const char* title;

    /**
     * @brief Synonyms of title, alternative search queries for finding this document, NULL terminated
     * 
     */
    const char* const* synonyms;

    /**
     * @brief Array of pointers to children, empty if there is no child, NULL terminated
     * 
     */
    const docview_extension_doc_tree_node* const* children;
};

/**
 * @brief Struct holding document content in HTML or URI
 * 
 */
struct docview_document
{

    /**
     * @brief Document content in HTML or URI
     * 
     */
    const char* content_or_uri;

    /**
     * @brief @rst
     * ``true`` if ``content_or_uri`` a is URI, ``false`` otherwise
     * @endrst
     * 
     */
    bool is_uri;
};

/**
 * @brief Enum holding all possible values of applicability level of an
 * extension
 * 
 * @details @rst
 * 
 * For more, see ":ref:`applicability-level`".
 * 
 * @endrst
 * 
 */
enum docview_extension_applicability_level
{

    /**
     * @brief The extension applies to a tiny amount of documentations
     * 
     */
    docview_extension_applicability_level_tiny = 0,

    /**
     * @brief The extension applies to a small amount of documentations, but not
     * as small as tiny
     * 
     */
    docview_extension_applicability_level_small = 1,

    /**
     * @brief The extension applies to a bigger amount of documentations then
     * small
     * 
     */
    docview_extension_applicability_level_medium = 2,

    /**
     * @brief The extension applies to a reasonable amount of documentations
     * 
     */
    docview_extension_applicability_level_big = 3,

    /**
     * @brief The extension applies to a huge amount of documentations
     * 
     */
    docview_extension_applicability_level_huge = 4
};

/**
 * @brief Structure for holding function pointers of an extension
 * 
 */
struct docview_extension_functions
{

    /**
     * @brief Returns the applicability level of extension
     * 
     * @details @rst
     * 
     * Extensions must implement this function. This is used determine the
     * extension calling order. Returning wrong value might cause misbehaviour.
     * See documentation of enum docview_extension_applicability_level and it's
     * members for more information.
     * 
     * @endrst
     * 
     * @return applicability level of extension
     */
    docview_extension_applicability_level(*applicability_level)();

    /**
     * @brief Returns a pointer to document tree of a path, nullptr on failure
     * 
     * @details @rst
     * 
     * Extensions must implement this function. Note that memory used by the
     * tree isn't managed by libdocview, it's extension's responsibility to do
     * and not do that.
     * 
     * @endrst
     * 
     * @param path path to documents
     * 
     * @return pointer to document tree
     */
    const docview_extension_doc_tree_node*(*func_get_docs_tree)(const char* path);

    /**
     * @brief Returns the URI or HTML content of document
     * 
     * @details @rst
     * 
     * Extensions must implement this method. This function will return a struct
     * with a ``const char*`` and a ``bool``. The value ``bool`` will ``true``
     * of the ``const char*`` points to a URI, ``false`` if the ``const char*``
     * points to HTML content. The pointer to document will be passed to
     * parameter node.
     * 
     * @endrst
     * 
     * @param node pointer to a node in document tree
     * 
     * @return URI or HTML content of document
     */
    docview_document(*get_doc)(const docview_extension_doc_tree_node* node);

    /**
     * @brief Returns the brief of from document
     * 
     * @details @rst
     * 
     * Extensions should implement this method. If an extension doesn't
     * implement this method, this member must be ``NULL``. This will return the
     * brief of the the document specified by parameter ``node``. Return value
     * should be in plain text (although any type of text is allowed). If not
     * implemented, an empty string will be assumed.
     * 
     * @endrst
     * 
     * @param node pointer to a node in document tree
     * @return brief of from document
     */
    const char*(*get_brief)(const docview_extension_doc_tree_node* node);

    /**
     * @brief Returns the details of from document
     * 
     * @details @rst
     * 
     * Extensions should implement this method. If an extension doesn't
     * implement this method, this member must be ``NULL``. This will return the
     * details for the document specified by parameter ``node``. Return value
     * should be in plain text (although any type of text is allowed). If not
     * implemented, an empty string will be assumed.
     * 
     * @endrst
     * 
     * @param node pointer to a node in document tree
     * @return details of from document
     */
    const char*(*get_details)(const docview_extension_doc_tree_node* node);

    /**
     * @brief Returns a section from document
     * 
     * @details @rst
     * 
     * Extensions should implement this method. If an extension doesn't
     * implement this method, this member must be ``NULL``. This will return a
     * section from the document specified by parameter ``node``. Return value
     * should be in plain text (although any type of text is allowed). If not
     * implemented, an
     * empty string will be assumed.
     * 
     * @endrst
     * 
     * @param node pointer to a node in document tree
     * @return details of from document
     */
    const char*(*get_section)(const docview_extension_doc_tree_node* node, const char*);
};

/**
 * @brief Define void as docview_doc_tree_node
 * 
 */
typedef void docview_doc_tree_node;

/**
 * @brief Returns a pointer to document tree of a path, ``NULL`` on failure
 * 
 * @details @rst
 * 
 * This function returns a pointer to document tree of a path, ``NULL`` on
 * failure. The pointer returned should not be freed by the application.
 * 
 * @endrst
 * 
 * @param path path to documents
 * 
 * @return pointer to document tree
 */
docview_doc_tree_node* docview_get_docs_tree(const char* path);

/**
 * @brief Returns the path or HTML content of document
 * 
 * @details @rst
 * 
 * This function will return a structure of a ``const char*`` and a ``bool``.
 * The value ``bool`` will ``true`` of the ``const char*`` points to a URI,
 * ``false`` if the ``const char*`` points to HTML content.
 * 
 * @endrst
 * 
 * @param node pointer to a node in document tree
 * 
 * @return path or HTML content of document
 */
docview_document docview_get_doc(docview_doc_tree_node* node);

/**
 * @brief Returns the brief of from document
 * 
 * @details @rst
 * 
 * This function will return the brief of the the document specified by
 * parameter ``node``. Return value should be in plain text (although any type
 * of text is allowed). May return empty string.
 * 
 * @endrst
 * 
 * @param node pointer to a node in document tree
 * @return brief of from document
 */
const char* docview_get_brief(docview_doc_tree_node* node);

/**
 * @brief Returns the details from document
 * 
 * @details @rst
 * 
 * This will will return the details from the document specified by parameter
 * ``node``. Return value should be in plain text (although any type of text is
 * allowed). May return empty string.
 * 
 * @endrst
 * 
 * @param node pointer to a node in document tree
 * @return details of from document
 */
const char* docview_get_details(docview_doc_tree_node* node);

/**
 * @brief Returns a section from document
 * 
 * @details @rst
 * 
 * This will return a section from the document specified by parameter ``node``.
 * Return value should be in plain text (although any type of text is allowed).
 * May return empty string.
 * 
 * @endrst
 * 
 * @param node pointer to a node in document tree
 * @return details of from document
 */
const char* docview_get_section(docview_doc_tree_node* node);

/**
 * @brief Searches through all loaded document tree
 * 
 * @details @rst
 * 
 * This function searches through the title and synonyms of all nodes of all
 * document trees (including root nodes). Matches occurs only when title or
 * any of synonyms follows or matches exactly (e.g. ``abc`` matches with
 * ``abc``, ``abcabc``, ``abcdef``, but not with ``acb``). Returned array does
 * not have any particular order.
 * 
 * @endrst
 * 
 * @param query the search query
 * @return NULL terminated array with nodes of matched documents
 */
const docview_doc_tree_node* const* docview_search(const char* query);

/**
 * @brief Checks whether a document node is still valid
 * 
 * @details @rst
 * 
 * This function checks whether a document node is still valid. A document may
 * be invalidate because of unloading an extension. It is recommended to
 * validate one node from every document node tree after unloading an extension.
 * 
 * @endrst
 * 
 * @param node the node to validate
 * @return whether a document node is valid
 */
bool docview_validate(const docview_doc_tree_node* node);

/**
 * @brief Returns the parent of a document node
 * 
 * @param node pointer to document node
 * @return parent of a document node
 */
docview_doc_tree_node* docview_doc_tree_node_parent(docview_doc_tree_node* node);

/**
 * @brief Returns the title of a document node
 * 
 * @param node pointer to document node
 * @return title of a document node
 */
const char* docview_doc_tree_node_title(docview_doc_tree_node* node);

/**
 * @brief Returns a NULL terminated array of synonyms of title
 * 
 * @details This function returns a NULL terminated array of synonyms of title. Here
 * synonyms means alternative title or search query to find this document.
 * 
 * @param node pointer to document node
 * @return NULL terminated array of synonyms of title
 */
const char* const* docview_doc_tree_node_synonyms(docview_doc_tree_node* node);

/**
 * @brief Returns a NULL terminated array of children of document node
 * 
 * @param node pointer to document node
 * @return NULL terminated array of children
 */
const docview_doc_tree_node* const* docview_doc_tree_node_children(docview_doc_tree_node* node);

// End of extern "C", only if compiler is a C++ complier
#ifdef __cplusplus
}
#endif

#endif
