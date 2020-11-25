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

#include <docview.hpp>
#include <docview-c.h>

#include <vector>
#include <memory>
#include <functional>
#include <stdexcept>
#include <map>
#include <array>
#include <cstring>
#include <dlfcn.h>

// Class for libdl, automatically frees up memory on destruction
class dl_ptr
{
public:

    // Handle returned by dlopen
    void* handle = nullptr;

    // Path of extension
    std::filesystem::path path;

    // Pointer to extension object by this extension
    docview::extension* extension;
    
    dl_ptr(std::filesystem::path path, int flags = RTLD_NOW | RTLD_GLOBAL)
        : handle(dlopen(std::string(path).c_str(), flags)),
        path(path),
        extension(nullptr)
    {}

    // We don't want to copy it, so delete copy connstructor and define move contructor
    dl_ptr(const dl_ptr&) = delete;
    dl_ptr& operator = (const dl_ptr&) = delete;
    dl_ptr(dl_ptr&& other)
        : handle(other.handle),
        path(other.path),
        extension(other.extension)
    {
        other.handle = nullptr;
    }

    // Define destructor to deallocate memory used by libdl on destruction
    ~dl_ptr()
    {

        // If handle isn't nullptr, delete it
        if (handle)
            dlclose(handle);
    }
};

// Wrapper class for extensions written in C
class c_extension : public docview::extension
{
private:

    // Original functions defined by defined by extension
    std::function<docview_extension_applicability_level()> func_applicability_level;
    std::function<const docview_extension_doc_tree_node*(const char*)> func_get_docs_tree;
    std::function<docview_document(const docview_extension_doc_tree_node*)> func_get_doc;
    std::function<const char*(const docview_extension_doc_tree_node*)> func_get_brief;
    std::function<const char*(const docview_extension_doc_tree_node*)> func_get_details;
    std::function<const char*(const docview_extension_doc_tree_node*, const char*)> func_get_section;

    // All root nodes created by the this class
    std::vector<const docview::doc_tree_node*> root_nodes;

    // Map of original node created by extension, mapped with nodes created by this class
    std::map<const docview::doc_tree_node*, const docview_extension_doc_tree_node*> original_nodes;

    // Builds a doc_tree from a C doc_tree
    docview::doc_tree_node* build_doc_tree(
        const docview_extension_doc_tree_node* source,
        const docview::doc_tree_node* parent = nullptr
    )
    {

        // If source is a nullptr, return a nullptr
        if (!source) return nullptr;

        // Allocate memory 
        docview::doc_tree_node* node = new docview::doc_tree_node;

        // Set the parent
        node->parent = parent;

        // If there is no parents, add it to root nodes
        if (!parent) root_nodes.push_back(node);

        // Add it's information to original node map
        original_nodes.insert(std::make_pair(node, source));

        // Add all children
        for (unsigned long i = 0; source->children[i]; i++)
        {
            node->children.push_back(build_doc_tree(source->children[i], node));
        }

        // Return created node
        return node;
    }

    // Frees memory allocated for nodes created by this class, not by extensions
    void free_node(const docview::doc_tree_node* node)
    {

        // First delete all child nodes
        for (auto child : node->children)
        {
            free_node(child);
        }

        // Then delete the provided node
        delete node;
    }

public:

    // Constructs the object, throws on invalid input
    c_extension(docview_extension_funcs* functions)
        : func_applicability_level(functions->applicability_level),
        func_get_docs_tree(functions->func_get_docs_tree),
        func_get_doc(functions->get_doc),
        func_get_brief(functions->get_brief),
        func_get_details(functions->get_details),
        func_get_section(functions->get_section),
        root_nodes{},
        original_nodes{}
    {
        if (!func_applicability_level || !func_get_docs_tree || !func_get_doc)
            throw std::runtime_error("invalid functions pointers");
    }

    // Destructor, frees up all memory used
    virtual ~c_extension() noexcept
    {
        for (auto node : root_nodes)
            free_node(node);
    }

    // This function returns the applicability level of the extension
    applicability_level get_applicability_level() noexcept
    {
        return (applicability_level)func_applicability_level();
    }

    // This function returns a document tree of given path
    const docview::doc_tree_node* get_docs_tree(std::filesystem::path path) noexcept
    {

        // Convert C nodes to C++ nodes and return
        return build_doc_tree(func_get_docs_tree(std::string(path).c_str()));
    }

    // This function returns the content or URI of a document node
    std::pair<std::string, bool> get_doc(const docview::doc_tree_node* node) noexcept
    {
        docview_document doc = func_get_doc(original_nodes[node]);
        return std::make_pair(std::string(doc.content_or_uri), doc.is_uri);
    }

    // This function returns the brief of a document node
    std::string get_brief(const docview::doc_tree_node* node) noexcept
    {

        // If function is null, return empty string
        if (func_get_brief)
            return func_get_brief(original_nodes[node]);
        return std::string();
    }

    // This function returns details from a document node
    std::string get_details(const docview::doc_tree_node* node) noexcept
    {

        // If function is null, return empty string
        if (func_get_details)
            return func_get_details(original_nodes[node]);
        return std::string();
    }
    
    // This function returns a section from a document node
    std::string get_section(const docview::doc_tree_node* node, std::string section) noexcept
    {

        // If function is null, return empty string
        if (func_get_section)
            return func_get_section(original_nodes[node], section.c_str());
        return std::string();
    }
};

// Loaded library (aka extension)
static std::vector<dl_ptr> loaded_libs;

// Loaded extension objects
static std::vector<docview::extension*> loaded_extensions;

// Loaded C extension objects
static std::vector<std::shared_ptr<docview::extension>> loaded_c_extensions;

// All root nodes loaded till now, with the extension loaded it
std::vector<std::pair<const docview::doc_tree_node*, docview::extension*>> root_nodes;

const char* c_str(const std::string& string)
{
    char* str = new char[string.size() + 1];
    memcpy(str, string.data(), string.size());
    return str;
}

// Returns the owner extension of a node
docview::extension* get_extension(const docview::doc_tree_node* node)
{

    // Find the root of the node
    const docview::doc_tree_node* root = node->parent;
    while (root->parent)
        root = root->parent;

    // Find the corresponding extension
    docview::extension* ext = nullptr;
    for (auto& root_node : root_nodes)
        if (root_node.first == root)
            ext = root_node.second;
    
    // If no extension was found, it's a invalid node, so throw
    if (!ext)
        throw std::invalid_argument("invalid node provided");

    return ext;
}

std::vector<const docview::doc_tree_node*> search_node(const docview::doc_tree_node* node, std::string query)
{
    std::vector<const docview::doc_tree_node*> matches;

    for (auto& child_node : node->children)
    {
        std::vector<const docview::doc_tree_node*> found_matches = search_node(child_node, query);
        matches.insert(matches.end(), found_matches.begin(), found_matches.end());
    }

    return matches;
}

// All possible applicability level of an extension
std::array<docview::extension::applicability_level, 5> applicability_levels =
{
    docview::extension::applicability_level::tiny,
    docview::extension::applicability_level::small,
    docview::extension::applicability_level::medium,
    docview::extension::applicability_level::big,
    docview::extension::applicability_level::huge
};

namespace docview
{
    void load(std::filesystem::path path)
    {

        // If extension is already loaded, do nothing
        for (unsigned long i = 0; i < loaded_libs.size(); i++)
        {
            if (loaded_libs[i].path == path) return;
        }

        // If path is non-existant and path doesn't point to a file, throw exception
        if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path))
            throw std::runtime_error(std::string(path) + "is either non-existant or not a file");
        
        // Load the extension file into memory
        loaded_libs.emplace_back(path);

        // Check if extension file loading succeeded, throw otherwise
        if (!loaded_libs[loaded_libs.size() - 1].handle)
        {
            loaded_libs.pop_back();
            throw std::runtime_error(std::string(path) + "isn't a valid extension");
        }

        // Pointer to extension object
        docview::extension* extension;

        // Get the extension object
        extension = (docview::extension*)dlsym(
            loaded_libs[loaded_libs.size() - 1].handle, "extension_object"
        );

        // If it's not written in C++, check if it's written in C
        if (!extension)
        {

            std::shared_ptr<c_extension> c_ext;

            // Get extension functions
            docview_extension_funcs* ext_funcs = (docview_extension_funcs*)dlsym(
                loaded_libs[loaded_libs.size() - 1].handle, "extension_functions"
            );

            // If even "extension_functions" symbol isn't defined, it isn't a valid extension
            if (!ext_funcs)
                throw std::runtime_error(std::string(path) + "isn't a valid extension");
            
            // Try to construct the wrapper, if it fails because of invalid values, it's isn't a extension
            try
            {
                c_ext = std::make_shared<c_extension>(ext_funcs);
            }
            catch (std::runtime_error& exception)
            {
                throw std::runtime_error(std::string(path) + "isn't a valid extension");
            }

            loaded_c_extensions.push_back(c_ext);
            extension = c_ext.get();
        }

        // Add extension object to loaded extension
        loaded_extensions.push_back(extension);

        // Attach the extension with the corresponding extension file object
        loaded_libs[loaded_libs.size() - 1].extension = extension;
    }

    void unload(std::filesystem::path path)
    {

        // Find out the extension to unload
        dl_ptr* lib_to_unload = nullptr;
        for (auto& lib : loaded_libs)
            if (lib.path == path)
            {
                lib_to_unload = &lib;
                break;
            }

        // If not found, simply return
        if (lib_to_unload == nullptr)
            return;

        // Remove all root_nodes associated the extension
        for (unsigned long i = 0; i < root_nodes.size(); i++)
            if (root_nodes[i].second == lib_to_unload->extension)
                root_nodes.erase(root_nodes.begin() + i--);

        // Remove the extension
        for (unsigned int i = 0; i < loaded_extensions.size(); i++)
            if (loaded_extensions[i] == lib_to_unload->extension)
                loaded_extensions.erase(loaded_extensions.begin() + i--);
    }

    const doc_tree_node* get_docs_tree(std::filesystem::path path)
    {

        // Try to parse with extensions with applicability level from tiny to huge
        for (auto& applicability : applicability_levels)
        {
            for (auto& extension : loaded_extensions)
            {

                // Make sure the extension matches applicability level
                if (extension->get_applicability_level() != applicability) continue;

                const doc_tree_node* doc_tree = extension->get_docs_tree(path);
                if (doc_tree)
                {

                    // Add it to root nodes
                    root_nodes.push_back(std::make_pair(doc_tree, extension));
                    return doc_tree;
                }
            }
        }

        // Parsing failed, return nullptr
        return nullptr;
    }

    std::pair<std::string, bool> get_doc(const doc_tree_node* node)
    {
        docview::extension* ext = get_extension(node);
        return ext->get_doc(node);
    }

    std::string get_brief(const doc_tree_node* node)
    {
        docview::extension* ext = get_extension(node);
        return ext->get_brief(node);
    }

    std::string get_details(const doc_tree_node* node)
    {
        docview::extension* ext = get_extension(node);
        return ext->get_details(node);
    }
    
    std::string get_section(const doc_tree_node* node, std::string section)
    {
        docview::extension* ext = get_extension(node);
        return ext->get_section(node, section);
    }

    std::vector<const doc_tree_node*> search(std::string query)
    {
        std::vector<const doc_tree_node*> matches;

        for (auto& root_node : root_nodes)
        {
            std::vector<const doc_tree_node*> found_matches = search_node(root_node.first, query);
            matches.insert(matches.end(), found_matches.begin(), found_matches.end());
        }

        return matches;
    }
}

docview_doc_tree_node* docview_get_docs_tree(const char* path)
{
    return (docview_doc_tree_node*)docview::get_docs_tree(path);
}

docview_document docview_get_doc(docview_doc_tree_node* node)
{
    auto document = docview::get_doc((docview::doc_tree_node*)node);
    return {c_str(document.first), document.second};
}

const char* docview_get_brief(docview_doc_tree_node* node)
{
    return c_str(docview::get_brief((docview::doc_tree_node*)node));
}

const char* docview_get_details(docview_doc_tree_node* node)
{
    return c_str(docview::get_details((docview::doc_tree_node*)node));
}

const char* docview_get_section(docview_doc_tree_node* node, const char* section)
{
    return c_str(docview::get_section((docview::doc_tree_node*)node, section));
}

const docview_doc_tree_node* const* docview_search(const char* query)
{
    auto result = docview::search(query);
    const docview_doc_tree_node** return_value =
        (const docview_doc_tree_node**)new docview_doc_tree_node*[result.size() + 1];

    for (unsigned long i = 0; i < result.size(); i++)
        return_value[i] = (docview_doc_tree_node*)result[i];
    
    return_value[result.size()] = nullptr;
    
    return return_value;
}

docview_doc_tree_node* docview_doc_tree_node_parent(docview_doc_tree_node* node)
{
    return (docview_doc_tree_node*)((docview::doc_tree_node*)node)->parent;
}

const char* docview_doc_tree_node_title(docview_doc_tree_node* node)
{
    return c_str(((docview::doc_tree_node*)node)->title);
}

const char* const* docview_doc_tree_node_synonyms(docview_doc_tree_node* node)
{
    const char** synonyms = new const char*[((docview::doc_tree_node*)node)->synonyms.size() + 1];
    
    for (unsigned long i = 0; i < ((docview::doc_tree_node*)node)->synonyms.size(); i++)
        synonyms[i] = c_str(((docview::doc_tree_node*)node)->synonyms[i]);
    
    synonyms[((docview::doc_tree_node*)node)->synonyms.size()] = nullptr;

    return synonyms;
}

const docview_doc_tree_node* const* docview_doc_tree_node_children(docview_doc_tree_node* node)
{
    const docview_doc_tree_node** children = (const docview_doc_tree_node**)new docview_doc_tree_node*[
        ((docview::doc_tree_node*)node)->children.size() + 1
    ];

    for (unsigned long i = 0; i < ((docview::doc_tree_node*)node)->children.size(); i++)
        children[i] = (docview_doc_tree_node*)((docview::doc_tree_node*)node)->children[i];
    
    children[((docview::doc_tree_node*)node)->children.size()] = nullptr;
    
    return children;
}

docview::extension::extension() {}
docview::extension::~extension() {}

std::string docview::extension::get_brief(const docview::doc_tree_node*) noexcept
{
    return std::string();
}

std::string docview::extension::get_details(const docview::doc_tree_node*) noexcept
{
    return std::string();
}

std::string docview::extension::get_section(const docview::doc_tree_node*, std::string) noexcept
{
    return std::string();
}
