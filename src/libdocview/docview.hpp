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
 * @file docview.hpp
 * @details @rst
 * 
 * ``docview.hpp`` is the C++ header of libdocview. It provides all functions,
 * classes, enumerations required to interact with Docview. It mainly consists
 * of two parts. One part is for applications like documentation viewer or
 * browsers, IDEs for handling documentations. Another part is for extensions.
 * The first part provides functions required to load and unload extensions,
 * parse documents into :ref:`document trees <document-tree>` and access them.
 * The second part is extension related. It defines the most important class for
 * extensions, :cpp:class:`docview::extension`, which is the base class for all
 * extensions.
 * 
 * .. note:: This header is compatible only with C++17 and above. It recommended
 *      to switch to C++17 or above if using old standards, as those are old and
 *      C++17 many new features, required by this header. If you are unable to
 *      upgrade, please consider using the :ref:`C header <libdocview-c-api>`
 *      instead.
 * 
 * Include this file with::
 * 
 *      #include <docview.hpp>
 * 
 * @endrst
 * 
 */

#ifndef _DOCVIEW_HPP
#define _DOCVIEW_HPP

#include <vector>
#include <filesystem>
#include <utility>

#if !defined(__cplusplus) || __cplusplus < 201703L
#   error "Only C++17 and later supported, if you can't use C++17 or later, use \
    the C header instead"
#endif

/**
 * @brief Namespace containing classes and function provided by libdocview
 * 
 * @details @rst
 * 
 * Namespace :cpp:type:`docview` contains all symbols (e.g. functions, classes,
 * enumerations) exposed by :ref:`libdocview <libdocview>`. It includes symbols
 * required by both applications and extensions.
 * 
 * @endrst
 * 
 */
namespace docview
{

    /**
     * @brief Loads an extension from given path
     * 
     * @details @rst
     * 
     * This function loads an extension from given path. If extension is already
     * loaded, there is no effects. If the provided path doesn't exist or it's
     * target isn't valid, an instance of ``std::runtime_error`` is thrown. If
     * the path doesn't point to a valid extension, an instance of
     * ``std::invalid_argument`` is thrown.
     * 
     * @endrst
     * 
     * @param path path to extension
     * 
     * @throw std::runtime_error if given path or it's target is invalid
     * 
     * @throw std::invalid_argument if the path doesn't point to a valid
     * extension
     */
    void load_ext(std::filesystem::path path);

    /**
     * @brief Unloads extension with given path
     * 
     * @details @rst
     * 
     * This function unloads extension with given path. If extension at given
     * path isn't loaded, there are no effects. This function never throws
     * exceptions unless an internal error occurred.
     * 
     * @endrst
     * 
     * @param path path to extension
     */
    void unload_ext(std::filesystem::path path);

    /**
     * @brief Checks whether extension at given path is loaded
     * 
     * @details @rst
     * 
     * This function checks whether an extension at given path is located. This
     * function never throws exceptions unless an internal error occurred.
     * 
     * @endrst
     * 
     * @param path path to extension
     * @return whether extension at given path is loaded
     */
    bool is_loaded(std::filesystem::path path);

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
    struct doc_tree_node
    {

        /**
         * @brief Pointer to parent, nullptr if there is no parent
         * 
         */
        const doc_tree_node* parent = nullptr;

        /**
         * @brief The title of document
         * 
         */
        std::string title;

        /**
         * @brief Synonyms of title, alternative search queries for finding this document
         * 
         */
        std::vector<std::string> synonyms;

        /**
         * @brief Vector of pointers to children, empty if there is no child
         * 
         */
        std::vector<const doc_tree_node*> children;
    };

    /**
     * @brief Base class for extension, which defines the structure of an extension object
     * 
     */
    class extension
    {
    public:

        /**
         * @brief Enum holding all possible values of applicability level
         * 
         * @details @rst
         * 
         * For more, see ":ref:`applicability-level`".
         * 
         * @endrst
         * 
         */
        enum class applicability_level
        {

            /**
             * @brief The extension applies to a tiny amount of documentations
             * 
             */
            tiny = 0,

            /**
             * @brief The extension applies to a small amount of documentations,
             * but not as small as tiny
             * 
             */
            small = 1,

            /**
             * @brief The extension applies to a bigger amount of documentations
             * then small
             * 
             */
            medium = 2,

            /**
             * @brief The extension applies to a reasonable amount of
             * documentations
             * 
             */
            big = 3,

            /**
             * @brief The extension applies to a huge amount of documentations
             * 
             */
            huge = 4
        };

        /**
         * @brief Constructs a new extension object
         * 
         */
        extension();

        /**
         * @brief Destroys the extension object
         * 
         */
        virtual ~extension() = 0;

        /**
         * @brief Returns the applicability level of extension
         * 
         * @details @rst
         * 
         * Extensions must implement this method. This is used determine the
         * extension calling order. For more, see ":ref:`applicability-level`".
         * 
         * @endrst
         * 
         * @return applicability level of extension
         */
        virtual applicability_level get_applicability_level() noexcept = 0;

        /**
         * @brief Returns a pointer to document tree of a path, nullptr on failure
         * 
         * @details @rst
         * 
         * Extensions must implement this method. Note that memory used by
         * the tree isn't managed by libdocview, it's extension's responsibility to
         * do and not do that.
         * 
         * @endrst
         * 
         * @param path path to documents
         * 
         * @return pointer to document tree
         */
        virtual const doc_tree_node* get_doc_tree(std::filesystem::path path) noexcept = 0;

        /**
         * @brief Returns the URI or HTML content of document
         * 
         * @details @rst
         * 
         * Extensions must implement this method. This function will return a
         * pair of a ``std::string`` and a ``bool``. The value ``bool`` will be
         * ``true`` if the value of ``std::string`` is a URI, ``false`` if the
         * value of ``std::string`` is HTML.
         * 
         * @endrst
         * 
         * @param node pointer to a node in document tree
         * 
         * @return URI or HTML content of document pointed by node
         */
        virtual std::pair<std::string, bool> get_doc(const doc_tree_node* node) noexcept = 0;

        /**
         * @brief Returns the brief of from document
         * 
         * @details @rst
         * 
         * Extensions should override this method. This will return the brief of
         * the document specified by parameter ``node``. Return value should be
         * in plain text (although any type of text is allowed). The base class
         * implemention returns an empty string.
         * 
         * @endrst
         * 
         * @param node pointer to a node in document tree
         * @return brief of from document
         */
        virtual std::string brief(const doc_tree_node* node) noexcept;

        /**
         * @brief Returns the details of from document
         * 
         * @details @rst
         * 
         * Extensions should override this method. This function will return the
         * details from the document specified by parameter ``node``. Return
         * value should be in plain text (although any type of text is allowed).
         * The base class implemention returns an empty string.
         * 
         * @endrst
         * 
         * @param node pointer to a node in document tree
         * @return details of from document
         */
        virtual std::string details(const doc_tree_node* node) noexcept;

        /**
         * @brief Returns a section from document
         * 
         * @details @rst
         * 
         * Extensions should override this method. This will return a section
         * from the document specified by parameter ``node``. Return value
         * should be in plain text (although any type of text is allowed). The
         * base class implemention returns an empty string.
         * 
         * @endrst
         * 
         * @param node pointer to a node in document tree
         * @param section the name of section
         * @return details of from document
         */
        virtual std::string section(const doc_tree_node* node, std::string section) noexcept;
    };

    /**
     * @brief Returns a pointer to document tree of a path, nullptr on failure
     * 
     * @details @rst
     * 
     * This function returns a pointer to document tree of a path, ``nullptr``
     * on failure. The pointer returned should not be managed by the
     * application.
     * 
     * @endrst
     * 
     * @param path path to documents
     * 
     * @return pointer to document tree
     */
    const doc_tree_node* get_doc_tree(std::filesystem::path path);

    /**
     * @brief Returns the URI or HTML content of document
     * 
     * @details @rst
     * 
     * This function will return a pair of a ``std::string`` and a ``bool``. The
     * value ``bool`` will ``true`` of the value of ``std::string`` is a URI,
     * ``false`` if the value of ``std::string`` is HTML.
     * 
     * @endrst
     * 
     * @param node pointer to a node in document tree
     * 
     * @return URI or HTML content of document
     */
    std::pair<std::string, bool> get_doc(const doc_tree_node* node);

    /**
     * @brief Returns the brief of from document
     * 
     * @details @rst
     * 
     * This function will return the brief of the the document specified by
     * parameter ``node``. Return value should be in plain text without any
     * markup (however it's not impossible to contains markup or special
     * characters). May return empty string.
     * 
     * @endrst
     * 
     * @param node pointer to a node in document tree
     * @return brief of from document
     */
    std::string brief(const doc_tree_node* node);

    /**
     * @brief Returns the details of from document
     * 
     * @details @rst
     * 
     * This will return the brief from the document specified by parameter
     * ``node``. Return value should be in plain text without any markup
     * (however it's not impossible to contains markup or special characters).
     * May return empty string.
     * 
     * @endrst
     * 
     * @param node pointer to a node in document tree
     * @return details of from document
     */
    std::string details(const doc_tree_node* node);

    /**
     * @brief Returns a section from document
     * 
     * @details @rst
     * 
     * This will return a section from the document specified by parameter
     * ``node``. Return value should be in plain text without any markup
     * (however it's not impossible to contains markup or special characters).
     * May return empty string.
     * 
     * @endrst
     * 
     * @param node pointer to a node in document tree
     * @return details of from document
     */
    std::string section(const doc_tree_node* node) noexcept;

    /**
     * @brief Searches through all loaded document trees
     * 
     * @details @rst
     * 
     * This function searches through the title and synonyms of all nodes of all
     * document trees (including root nodes). Matches occurs only when title or
     * any of synonyms follows or matches exactly (e.g. ``abc`` matches with
     * ``abc``, ``abcabc``, ``abcdef``, but not with ``acb``). Returned vector
     * does not have any particular order.
     * 
     * @endrst
     * 
     * @param query the search query
     * @return vector with nodes of matched document nodes
     */
    std::vector<const doc_tree_node*> search(std::string query);

    /**
     * @brief Searches through given document trees
     * 
     * @details @rst
     * 
     * This function searches through the title and synonyms of all nodes of
     * given document trees (including root nodes). Matches occurs only when
     * title or any of synonyms follows or matches exactly (e.g. ``abc`` matches
     * with ``abc``, ``abcabc``, ``abcdef``, but not with ``acb``). Returned
     * vector does not have any particular order.
     * 
     * .. warning:: This function is deprecated. This was added due to an issue
     *      during search. This function will be removed eventually.
     * 
     * @endrst
     * 
     * @param query the search query
     * @return vector with nodes of matched document nodes
     */
    std::vector<const doc_tree_node*> search(
        std::string query,
        std::vector<std::pair<const docview::doc_tree_node*, std::filesystem::path>> document_roots
    );

    /**
     * @brief Checks whether a document node is still valid
     * 
     * @details @rst
     * 
     * This function checks whether a document node is still valid. A document
     * may be invalidate because of unloading an extension. It is recommended to
     * validate one node from every document node tree after unloading an
     * extension.
     * 
     * @endrst
     * 
     * @param node the node to validate
     * @return whether a document node is valid
     */
    bool validate(const doc_tree_node* node);
}

#endif
