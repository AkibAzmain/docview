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
 * @brief Library header for C++
 * 
 */

#ifndef _DOCVIEW_HPP
#define _DOCVIEW_HPP

#include <vector>
#include <filesystem>
#include <utility>

#if !defined(__cplusplus) || __cplusplus < 201703L
#   error "Only C++17 and later supported, if you can't use C++17 or later, use the C bindings"
#endif

/**
 * @brief Namespace containing classes and function provided by libdocview
 * 
 */
namespace docview
{

    /**
     * @brief Loads an extension from given path
     * 
     * @details This function loads an extension from given path. If extension is
     * already loaded, there is no effects. If the provided path doesn't exist or
     * isn't a valid extension, std::runtime_error is thrown.
     * 
     * @param path path to extension
     * @throw std::runtime_error
     */
    void load_ext(std::filesystem::path path);

    /**
     * @brief Unloads extension with given path
     * 
     * @details This function unloads extension with given path. If extension isn't
     * loaded, there is no effects.
     * 
     * @param path path to extension
     */
    void unload_ext(std::filesystem::path path);

    /**
     * @brief Check whether extension at given path is loaded
     * 
     * @param path path to extension
     * @return whether extension at given path is loaded
     */
    bool is_loaded(std::filesystem::path path);

    /**
     * @brief Structure for holding a document tree
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
         */
        enum class applicability_level
        {

            /**
             * @brief The extension applies to a tiny amount of documentations
             * 
             */
            tiny = 0,

            /**
             * @brief The extension applies to a small amount of documentations, but not as small as tiny
             * 
             */
            small = 1,

            /**
             * @brief The extension applies to a bigger amount of documentations then small
             * 
             */
            medium = 2,

            /**
             * @brief The extension applies to a reasonable amount of documentations
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
         * @details Extensions must implement this method. This is used determine the
         * extension calling order. Returning wrong value might cause misbehaviour.
         * See documentation of enum applicability_level and it's members for more
         * information.
         * 
         * @return applicability level of extension
         */
        virtual applicability_level get_applicability_level() noexcept = 0;

        /**
         * @brief Returns a pointer to document tree of a path, nullptr on failure
         * 
         * @details Extensions must implement this method. Note that memory used by
         * the tree isn't managed by libdocview, it's extension's responsibility to
         * do and not do that.
         * 
         * @param path path to documents
         * 
         * @return pointer to document tree
         */
        virtual const doc_tree_node* get_doc_tree(std::filesystem::path path) noexcept = 0;

        /**
         * @brief Returns the URI or HTML content of document
         * 
         * @details Extensions must implement this method. This function will return
         * a pair of a std::string and a bool. The value bool will true of the value
         * of std::string is a URI, false if the value of std::string is HTML. The
         * pointer to document will be passed to parameter node.
         * 
         * @param node pointer to a node in document tree
         * 
         * @return URI or HTML content of document
         */
        virtual std::pair<std::string, bool> get_doc(const doc_tree_node* node) noexcept = 0;

        /**
         * @brief Returns the brief of from document
         * 
         * @details Extensions should override this method. This will return the brief
         * of the the document specified by parameter node. Return value should be in
         * plain text (although any type of text is allowed). The base class implemention
         * returns an empty string.
         * 
         * @param node pointer to a node in document tree
         * @return brief of from document
         */
        virtual std::string brief(const doc_tree_node* node) noexcept;

        /**
         * @brief Returns the details of from document
         * 
         * @details Extensions should override this method. This will return the details
         * from the document specified by parameter node. Return value should be in plain
         * text (although any type of text is allowed). The base class implemention
         * returns an empty string.
         * 
         * @param node pointer to a node in document tree
         * @return details of from document
         */
        virtual std::string details(const doc_tree_node* node) noexcept;

        /**
         * @brief Returns a section from document
         * 
         * @details Extensions should override this method. This will return a section
         * from the document specified by parameter node. Return value should be in plain
         * text (although any type of text is allowed). The base class implemention
         * returns an empty string.
         * 
         * @param node pointer to a node in document tree
         * @return details of from document
         */
        virtual std::string section(const doc_tree_node* node, std::string section) noexcept;
    };

    /**
     * @brief Returns a pointer to document tree of a path, nullptr on failure
     * 
     * @details This function returns a pointer to document tree of a path, nullptr
     * on failure. The pointer returned should not be managed by the application.
     * 
     * @param path path to documents
     * 
     * @return pointer to document tree
     */
    const doc_tree_node* get_doc_tree(std::filesystem::path path);

    /**
     * @brief Returns the URI or HTML content of document
     * 
     * @details This function will return a pair of a std::string and a bool.
     * The value bool will true of the value of std::string is a URI, false
     * if the value of std::string is HTML.
     * 
     * @param node pointer to a node in document tree
     * 
     * @return URI or HTML content of document
     */
    std::pair<std::string, bool> get_doc(const doc_tree_node* node);

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
    std::string brief(const doc_tree_node* node);

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
    std::string details(const doc_tree_node* node);

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
    std::string section(const doc_tree_node* node) noexcept;

    /**
     * @brief Searches through all loaded document tree
     * 
     * @param query the search query
     * @return vector with nodes of matched documents
     */
    std::vector<const doc_tree_node*> search(std::string query);

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
    bool validate(const doc_tree_node* node);
}

#endif
