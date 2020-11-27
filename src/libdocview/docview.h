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
 * @brief C bindings for libdocview
 * 
 */

#ifndef _DOCVIEW_C_H
#define _DOCVIEW_C_H

// Include stdbool.h to use bool
#ifndef __cplusplus
#   include <stdbool.h>
#endif

// If it's being compiled by a C++ compiler, disable name mangling
#ifdef __cplusplus
extern "C"
{
#endif

// Define NULL if not defined
#ifndef NULL
#   define NULL 0
#endif

/**
 * @brief Structure for holding a document tree
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
     * @brief True if content_or_uri is URI, false otherwise
     * 
     */
    bool is_uri;
};

/**
 * @brief Enum holding all possible values of applicability level of an extension
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
     * @brief The extension applies to a small amount of documentations, but not as small as tiny
     * 
     */
    docview_extension_applicability_level_small = 1,

    /**
     * @brief The extension applies to a bigger amount of documentations then small
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
 * @brief Functions extension must implement
 * 
 */
struct docview_extension_funcs
{

    /**
     * @brief Returns the applicability level of extension
     * 
     * @details Extensions must implement this function. This is used determine the
     * extension calling order. Returning wrong value might cause misbehaviour. See
     * documentation of enum docview_extension_applicability_level and it's members
     * for more information.
     *
     * 
     * @return applicability level of extension
     */
    docview_extension_applicability_level(*applicability_level)();

    /**
     * @brief Returns a pointer to document tree of a path, nullptr on failure
     * 
     * @details Extensions must implement this function. Note that memory used by
     * the tree isn't managed by libdocview, it's extension's responsibility to
     * do and not do that.
     * 
     * @param path path to documents
     * 
     * @return pointer to document tree
     */
    const docview_extension_doc_tree_node*(*func_get_docs_tree)(const char* path);

    /**
     * @brief Returns the URI or HTML content of document
     * 
     * @details Extensions must implement this method. This function will return
     * a struct with a string and a bool. The value bool will true of the value
     * of the string is a URI, false if the value of the string is HTML. The
     * pointer to document will be passed to parameter node.
     * 
     * @param node pointer to a node in document tree
     * 
     * @return URI or HTML content of document
     */
    docview_document(*get_doc)(const docview_extension_doc_tree_node* node);

    /**
     * @brief Returns the brief of from document
     * 
     * @details Extensions should implement this method. If an extension doesn't
     * implement this method, this member must be NULL. This will return the
     * brief of the the document specified by parameter node. Return value should
     * be in plain text (although any type of text is allowed). If not implemented,
     * an empty string will be assumed.
     * 
     * @param node pointer to a node in document tree
     * @return brief of from document
     */
    const char*(*get_brief)(const docview_extension_doc_tree_node* node);

    /**
     * @brief Returns the details of from document
     * 
     * @details Extensions should implement this method. If an extension doesn't
     * implement this method, this member must be NULL. This will return the
     * details for the document specified by parameter node. Return value should
     * be in plain text (although any type of text is allowed). If not implemented,
     * an empty string will be assumed.
     * 
     * @param node pointer to a node in document tree
     * @return details of from document
     */
    const char*(*get_details)(const docview_extension_doc_tree_node* node);

    /**
     * @brief Returns a section from document
     * 
     * @details Extensions should implement this method. If an extension doesn't
     * implement this method, this member must be NULL. This will return a section
     * from the document specified by parameter node. Return value should be in
     * plain text (although any type of text is allowed).  If not implemented, an
     * empty string will be assumed.
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
 * @brief Returns a pointer to document tree of a path, NULL on failure
 * 
 * @details This function returns a pointer to document tree of a path, nullptr
 * on failure. The pointer returned should not be managed by the application.
 * 
 * @param path path to documents
 * 
 * @return pointer to document tree
 */
docview_doc_tree_node* docview_get_docs_tree(const char* path);

/**
 * @brief Returns the path or HTML content of document
 * 
 * @details This function will return a pair of a std::string and a bool.
 * The value bool will true of the value of std::string is a path, false
 * if the value of std::string is HTML.
 * 
 * @param node pointer to a node in document tree
 * 
 * @return path or HTML content of document
 */
docview_document docview_get_doc(docview_doc_tree_node* node);

/**
 * @brief Returns the brief of from document
 * 
 * @details This function will return the brief of the the document specified
 * by parameter node. Return value should be in plain text (although any type
 * of text is allowed). May return empty string.
 * 
 * @param node pointer to a node in document tree
 * @return brief of from document
 */
const char* docview_get_brief(docview_doc_tree_node* node);

/**
 * @brief Returns the details of from document
 * 
 * @details This will return the brief from the document specified by parameter
 * node. Return value should be in plain text (although any type of text is
 * allowed). May return empty string.
 * 
 * @param node pointer to a node in document tree
 * @return details of from document
 */
const char* docview_get_details(docview_doc_tree_node* node);

/**
 * @brief Returns a section from document
 * 
 * @details This will return a section from the document specified by parameter
 * node. Return value should be in plain text (although any type of text is
 * allowed). May return empty string.
 * 
 * @param node pointer to a node in document tree
 * @return details of from document
 */
const char* docview_get_section(docview_doc_tree_node* node);

/**
 * @brief Searches through all loaded document tree
 * 
 * @param query the search query
 * @return NULL terminated array with nodes of matched documents
 */
const docview_doc_tree_node* const* docview_search(const char* query);

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

/**
 * @brief Checks whether a document node is still valid
 * 
 * @details This function checks whether a document node is still valid. A document
 * may be invalidate because of unloading an extension. It is recommended to check
 * all root document node (or one node from a document node tree) after unloading an
 * extension.
 * 
 * @param node the node to validate
 * @return whether a document node is valid
 */
bool docview_validate(const docview_doc_tree_node* node);

// End of extern "C", only if compiler is a C++ complier
#ifdef __cplusplus
}
#endif

#endif
